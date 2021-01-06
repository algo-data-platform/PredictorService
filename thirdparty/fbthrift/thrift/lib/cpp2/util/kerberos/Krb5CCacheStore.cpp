/*
 * Copyright 2014-present Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/util/kerberos/Krb5CCacheStore.h>

#include <chrono>
#include <glog/logging.h>
#include <memory>
#include <set>
#include <stdio.h>

#include <folly/stats/BucketedTimeSeries-defs.h>
#include <folly/Memory.h>
#include <folly/ScopeGuard.h>
#include <folly/String.h>

// In older version of krb, this constant is not defined. Define it
#ifndef KRB5_GC_NO_STORE
#define KRB5_GC_NO_STORE     8  /**< Do not store in credential cache */
#endif

namespace apache { namespace thrift { namespace krb5 {
using namespace std;

const int Krb5CCacheStore::SERVICE_HISTOGRAM_NUM_BUCKETS = 10;
const int Krb5CCacheStore::SERVICE_HISTOGRAM_PERIOD = 600;
/**
 * Don't use the tickets if they're about to expire within 5 minutes.
 * This is to prevent handshake failures where the tickets expire during the
 * handshake. This is also to make sure clock skew issues are minimized.
 */
const uint32_t Krb5CCacheStore::EXPIRATION_THRESHOLD_SEC = 300;

static bool serviceCountCompare (
    const pair<string, uint64_t>& i,
    const pair<string, uint64_t>& j) {
  return (i.second > j.second);
}

Krb5CCacheStore::ServiceData::ServiceData() :
  timeSeries(
    Krb5CCacheStore::SERVICE_HISTOGRAM_NUM_BUCKETS,
    std::chrono::seconds(
      Krb5CCacheStore::SERVICE_HISTOGRAM_PERIOD)) {
}

void Krb5CCacheStore::ServiceData::bumpCount() {
  folly::SharedMutex::WriteHolder guard(lockTimeSeries);
  time_t now = time(nullptr);
  timeSeries.addValue(std::chrono::seconds(now), 1);
}

uint64_t Krb5CCacheStore::ServiceData::getCount() {
  folly::SharedMutex::ReadHolder guard(lockTimeSeries);
  // Note that we don't have a need to call timeSeries_.update(<time>)
  // here because we don't care to have the exact count at the current
  // time. We're ok with grabbing the count at the last update.
  return timeSeries.count();
}

std::shared_ptr<Krb5CCache> Krb5CCacheStore::waitForCache(
    const Krb5Principal& service,
    SecurityLogger* logger) {
  std::shared_ptr<ServiceData> dataPtr = getServiceDataPtr(service);

  uint64_t curtime = chrono::duration_cast<chrono::seconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();

  // Bump that we've used the principal
  dataPtr->bumpCount();

  if (logger) {
    logger->logStart("get_prepared_cache", folly::to<string>(service));
  }

  {
    folly::SharedMutex::ReadHolder readLock(dataPtr->lockCache);
    // If there is a cache, just return it. Try with a read lock first
    // for performance reasons.
    if (dataPtr->cache) {
      // First check that the service principal in the cache isn't about
      // to expire.
      if (dataPtr->expires > curtime + EXPIRATION_THRESHOLD_SEC) {
        if (logger) {
          logger->logEnd("get_prepared_cache");
        }
        return dataPtr->cache;
      }
    }
  }

  // If we didn't find a cache, initialize a local, temporary cache for the
  // service. We might throw this away if some other thread initialized the
  // cache meanwhile.
  if (logger) {
    logger->logStart("init_cache_for_service", folly::to<string>(service));
  }
  uint64_t expires{0};
  auto tempCache = initCacheForService(service, nullptr, logger, expires);
  if (logger) {
    logger->logEnd("init_cache_for_service");
  }

  // Upgrade to a write lock, and initialize the cache. Overwrite if already
  // initialized by some other thread.
  folly::SharedMutex::WriteHolder writeLock(dataPtr->lockCache);
  dataPtr->cache = std::move(tempCache);
  dataPtr->expires = expires;

  return dataPtr->cache;
}

std::shared_ptr<Krb5CCacheStore::ServiceData>
    Krb5CCacheStore::getServiceDataPtr(const Krb5Principal& service) {

  // If ccache is disabled, just return a fresh ServiceData obj
  if (maxCacheSize_ == 0) {
    return std::make_shared<ServiceData>();
  }

  string service_name = folly::to<string>(service);
  {
    folly::SharedMutex::ReadHolder readLock(serviceDataMapLock_);
    auto found = serviceDataMap_.find(service_name);
    if (found != serviceDataMap_.end()) {
      // Get the element from the map if it exists
      return found->second;
    }
  }

  // Not found, we need to create it
  folly::SharedMutex::WriteHolder writeLock(serviceDataMapLock_);
  auto found = serviceDataMap_.find(service_name);
  if (found == serviceDataMap_.end()) {
    // If we reached the limit, then we need to free some room
    if (maxCacheSize_ > 0 &&
        cacheItemQueue_.size() >= static_cast<size_t>(maxCacheSize_)) {
      serviceDataMap_.erase(cacheItemQueue_.front());
      cacheItemQueue_.pop();
    }
    serviceDataMap_[service_name] = std::make_shared<ServiceData>();
    cacheItemQueue_.push(service_name);
  }
  return serviceDataMap_[service_name];
}

std::unique_ptr<Krb5CCache> Krb5CCacheStore::initCacheForService(
    const Krb5Principal& service,
    const krb5_creds* creds,
    SecurityLogger* logger,
    uint64_t& expires) {
  expires = 0;
  if (logger) {
    logger->logStart("wait_for_tgt");
  }
  std::shared_ptr<const Krb5Credentials> tgt;
  auto client_princ = tgts_.getClientPrincipal();
  // Get a cross-realm tgt if we need
  string srealm = service.getRealm();
  if (srealm != tgts_.getClientPrincipal().getRealm()) {
    tgt = tgts_.getTgtForRealm(srealm);
  } else {
    tgt = tgts_.getTgt();
  }
  if (logger) {
    logger->logEnd("wait_for_tgt");
  }

  if (logger) {
    logger->logStart("init_barebones_ccache");
  }
  // Make a new memory cache.
  auto mem = std::make_unique<Krb5CCache>(
    Krb5CCache::makeNewUnique("MEMORY"));
  // Initialize the new CC
  mem->setDestroyOnClose();
  mem->initialize(client_princ.get());
  mem->storeCred(tgt->get());
  if (logger) {
    logger->logEnd("init_barebones_ccache");
  }

  if (creds != nullptr) {
    mem->storeCred(*creds);
    expires = creds->times.endtime;
  } else {
    if (logger) {
      logger->logStart("get_credential_from_kdc");
    }
    Krb5Credentials cred = mem->getCredentials(service.get());
    expires = cred.get().times.endtime;
    if (logger) {
      logger->logEnd("get_credential_from_kdc");
    }
  }

  return mem;
}

void Krb5CCacheStore::importCache(
    Krb5CCache& file_cache) {
  logger_->logStart("import_tgts");
  // Split out tgts and service principals
  std::vector<Krb5Credentials> tgts;
  std::vector<Krb5Credentials> services;
  uint64_t count = 0;
  for (auto& tmp_cred : file_cache) {
    Krb5Credentials cred(std::move(tmp_cred));
    Krb5Principal server = Krb5Principal::copyPrincipal(
      ctx_.get(), cred.get().server);
    if (server.isTgt()) {
      tgts.push_back(std::move(cred));
      count++;
    } else if (!server.getRealm().empty()) {
      // Sometimes, somehow, principals for host/{hg.vvv,svn.vip} have no
      // realm!!  Why?  No idea, but they exist and they blow up the entire
      // cache reading thread, so defend against them for now by ignoring that
      // entry (example: #7808411).
      services.push_back(std::move(cred));
    } else {
      logger_->log("Realm empty, so service ignored");
    }

  }

  // Import TGTs
  Krb5Tgts tgts_obj;
  Krb5Principal client_principal = file_cache.getClientPrincipal();
  tgts_obj.setClientPrincipal(client_principal);
  for (auto& tgt : tgts) {
    Krb5Principal server_principal = Krb5Principal::copyPrincipal(
      ctx_.get(), tgt.get().server);
    if (server_principal.getComponent(1) == client_principal.getRealm()) {
      tgts_obj.setTgt(std::make_unique<Krb5Credentials>(std::move(tgt)));
    } else {
      tgts_obj.setTgtForRealm(
        server_principal.getComponent(1),
        std::make_unique<Krb5Credentials>(std::move(tgt)));
    }
  }
  tgts_ = std::move(tgts_obj);
  logger_->logEnd("import_tgts", folly::to<std::string>(count));

  // Import service creds
  DataMapType new_data_map;
  std::queue<std::string> new_data_queue;
  logger_->logStart("import_service_creds");
  count = 0;
  for (auto& service : services) {
    if (maxCacheSize_ >= 0 && count >= static_cast<size_t>(maxCacheSize_)) {
      break;
    }
    Krb5Principal princ = Krb5Principal::copyPrincipal(
      ctx_.get(), service.get().server);
    uint64_t expires{0};
    auto mem = initCacheForService(princ, &service.get(), nullptr, expires);

    auto data = std::make_shared<ServiceData>();
    std::string name = folly::to<string>(princ);
    new_data_map[name] = data;
    new_data_queue.push(name);
    data->cache = std::move(mem);
    data->expires = expires;
    data->bumpCount();
    count++;
  }

  folly::SharedMutex::WriteHolder service_data_lock(serviceDataMapLock_);
  serviceDataMap_ = std::move(new_data_map);
  cacheItemQueue_ = std::move(new_data_queue);
  logger_->logEnd("import_service_creds", folly::to<std::string>(count));
}

std::vector<Krb5Principal> Krb5CCacheStore::getServicePrincipalList() {
  std::vector<Krb5Principal> services;
  folly::SharedMutex::ReadHolder lock(serviceDataMapLock_);
  for (auto& data : serviceDataMap_) {
    folly::SharedMutex::ReadHolder cache_lock(data.second->lockCache);
    if (!data.second->cache) {
      continue;
    }
    auto princ_list = data.second->cache->getServicePrincipalList();
    if (princ_list.size() < 1) {
      throw std::runtime_error("Principal list too small in ccache");
    }
    auto princ = std::move(princ_list[0]);
    services.push_back(std::move(princ));
  }
  return services;
}

uint64_t Krb5CCacheStore::renewCreds() {
  auto curTgt = tgts_.getTgt();
  auto client_princ = tgts_.getClientPrincipal();
  auto realms = tgts_.getValidRealms();
  uint64_t renewCount = 0;

  // Renew TGTs
  Krb5Tgts tgts;
  tgts.kInit(client_princ);
  for (const auto& realm : realms) {
    tgts.getTgtForRealm(realm);
  }
  tgts_ = std::move(tgts);

  struct CCachesWithExpiration {
    std::unique_ptr<Krb5CCache> ccache;
    uint64_t expires;
  };

  // Renew service creds, and store the renewed creds in a temporary map
  std::unordered_map<string, CCachesWithExpiration> renewed_map;
  for (auto& service : getServicePrincipalList()) {
    try {
      uint64_t expires{0};
      auto mem = initCacheForService(service, nullptr, nullptr, expires);
      renewed_map[folly::to<string>(service)] = {std::move(mem), expires};
      renewCount++;
    } catch (const std::runtime_error& e) {
      VLOG(4) << "Failed to renew cred for service: "
              << folly::to<string>(service) << " "
              << e.what();
    }
  }

  // Update the main service data map with the renewed creds.
  // If the creds failed to be renewed and the old ones are stale, then
  // just delete the old ones.
  folly::SharedMutex::WriteHolder service_data_lock(serviceDataMapLock_);
  for (auto& entry : serviceDataMap_) {
    auto renewed_entry = renewed_map.find(entry.first);
    folly::SharedMutex::WriteHolder lock(entry.second->lockCache);
    if (renewed_entry != renewed_map.end()) {
      entry.second->cache = std::move(renewed_entry->second.ccache);
      entry.second->expires = renewed_entry->second.expires;
    } else if (entry.second->cache) {
      // Not found, see if it's a new cred or old one
      auto lifetime = entry.second->cache->getLifetime();
      auto tgt_lifetime = tgts_.getLifetime();
      if (lifetime.second < tgt_lifetime.second) {
        // Delete old credential
        entry.second->cache.reset();
      }
    } // else
    // It's possible this thread wins the race to hit a service in the
    // data map that hasn't yet obtained a ccache. In this case, just leave
    // it alone. It will get the correct ccache.
  }
  return renewCount;
}

std::set<std::string> Krb5CCacheStore::getTopServices(size_t limit) {
  // Put 'limit' number of most frequently used credentials into the
  // top_services set.
  folly::SharedMutex::ReadHolder readLock(serviceDataMapLock_);
  vector<pair<string, uint64_t>> count_vector;
  for (auto& element : serviceDataMap_) {
    count_vector.emplace_back(element.first, element.second->getCount());
  }

  sort(count_vector.begin(), count_vector.end(), serviceCountCompare);

  std::set<std::string> top_services;
  size_t count = 0;
  for (auto& element : count_vector) {
    if (count >= limit) {
      break;
    }
    top_services.insert(element.first);
    count++;
  }
  return top_services;
}

std::unique_ptr<Krb5CCache> Krb5CCacheStore::exportCache(size_t limit) {
  Krb5Principal client_principal = tgts_.getClientPrincipal();

  // Make a new memory cache.
  auto temp_cache = std::make_unique<Krb5CCache>(
    Krb5CCache::makeNewUnique("MEMORY"));
  // Initialize the new CC
  temp_cache->initialize(client_principal.get());

  {
    folly::SharedMutex::ReadHolder readLock(serviceDataMapLock_);
    const auto top_services = getTopServices(limit);

    for (auto& data : serviceDataMap_) {
      folly::SharedMutex::ReadHolder lock(data.second->lockCache);
      if (!data.second->cache) {
        continue;
      }
      auto princ_list = data.second->cache->getServicePrincipalList();
      if (princ_list.size() < 1) {
        throw std::runtime_error("Principal list too small in ccache");
      }
      auto princ = std::move(princ_list[0]);
      if (top_services.count(folly::to<string>(princ)) == 0) {
        continue;
      }
      Krb5Credentials service_cred = data.second->cache->retrieveCred(
        princ.get());
      temp_cache->storeCred(service_cred.get());
    }
  }

  // Store the TGTs in the cache.
  temp_cache->storeCred(tgts_.getTgt()->get());
  auto realms = tgts_.getValidRealms();
  for (const auto& realm : realms) {
    auto cred = tgts_.getTgtForRealm(realm);
    temp_cache->storeCred(cred->get());
  }
  return temp_cache;
}

bool Krb5CCacheStore::isInitialized() {
  return tgts_.isInitialized();
}

Krb5Lifetime Krb5CCacheStore::getLifetime() {
  return tgts_.getLifetime();
}

Krb5Principal Krb5CCacheStore::getClientPrincipal() {
  return tgts_.getClientPrincipal();
}

void Krb5CCacheStore::kInit(const Krb5Principal& client) {
  tgts_.kInit(client);
  folly::SharedMutex::WriteHolder service_data_lock(serviceDataMapLock_);
  serviceDataMap_.clear();
  std::queue<std::string> emptyQueue;
  cacheItemQueue_ = std::move(emptyQueue);
}

void Krb5CCacheStore::notifyOfError(const std::string& error) {
  tgts_.notifyOfError(error);
}

std::map<std::string, Krb5Lifetime>
Krb5CCacheStore::getServicePrincipalLifetimes(size_t limit) {
  folly::SharedMutex::ReadHolder readLock(serviceDataMapLock_);
  const auto top_services = getTopServices(limit);

  std::map<std::string, Krb5Lifetime> services;
  for (auto& data : serviceDataMap_) {
    folly::SharedMutex::ReadHolder cache_lock(data.second->lockCache);
    if (!data.second->cache) {
      continue;
    }
    const auto service = getLifetimeOfFirstServicePrincipal(data.second->cache);
    if (top_services.count(service.first) == 0) {
      continue;
    }
    services[service.first] = service.second;
  }
  return services;
}

std::map<std::string, Krb5Lifetime> Krb5CCacheStore::getTgtLifetimes() {
  return tgts_.getLifetimes();
}

std::pair<std::string, Krb5Lifetime>
Krb5CCacheStore::getLifetimeOfFirstServicePrincipal(
    const std::shared_ptr<Krb5CCache>& cache) {
  auto princ_list = cache->getServicePrincipalList();
  if (princ_list.size() < 1) {
    throw std::runtime_error("Principal list too small in ccache");
  }
  auto princ = std::move(princ_list[0]);
  auto princStr = folly::to<string>(princ);
  auto creds = cache->retrieveCred(princ.get());
  auto lifetime =
      std::make_pair(creds.get().times.starttime, creds.get().times.endtime);
  return std::make_pair(folly::to<string>(princ), std::move(lifetime));
}
}}}
