/*
 *  Copyright (c) 2015-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#pragma once

#include <initializer_list>
#include <proxygen/lib/http/codec/SettingsId.h>
#include <vector>

/*
 * HTTPSettings are stored in an underlying vector, presumably to limit the
 * amount of memory required.  Access various settings thus generally involves
 * iterating over available entries.  Removing entries does not require any
 * shifts.
 */

namespace proxygen {

struct HTTPSetting {
  HTTPSetting(SettingsId i, uint32_t v)
    : id(i), value(v) {}

  SettingsId id;
  uint32_t value;
};

class HTTPSettings {
 public:
  // HTTP/2 Defaults
  HTTPSettings() :
      settings_(
        {{ SettingsId::HEADER_TABLE_SIZE, 4096 },
         { SettingsId::ENABLE_PUSH, 1 },
         { SettingsId::MAX_FRAME_SIZE, 16384 }}) { }
  explicit HTTPSettings(
      const std::initializer_list<SettingPair>& initialSettings) {
    settings_.reserve(initialSettings.size());
    // TODO: setSetting involves looping over all settings so the below actually
    // models as O(n^2) which is pretty bad.  Can't change it outright without
    // making sure we handle duplicates (same setting id)
    for (auto& setting: initialSettings) {
      setSetting(setting.first, setting.second);
    }
  }
  void setSetting(SettingsId id, uint32_t val);
  void unsetSetting(SettingsId id);
  const HTTPSetting* getSetting(SettingsId id) const;
  uint32_t getSetting(SettingsId id, uint32_t defaultVal) const;
  // Note: this does not count disabled settings
  std::size_t getNumSettings() const { return settings_.size(); }
  const std::vector<HTTPSetting>& getAllSettings() { return settings_; }
  void clearSettings() {
    settings_.clear();
  }
 private:
  // Returns the specified type of iterator to the setting associated with the
  // specified id
  std::vector<HTTPSetting>::iterator getSettingIter(SettingsId id);
  std::vector<HTTPSetting>::const_iterator getSettingConstIter(
    SettingsId id) const;

  // TODO: evaluate whether using a list or default initializing vector with
  // all settings so that size == capacity (else on push_backs capacity is
  // likely to be > size)
  std::vector<HTTPSetting> settings_;
};

using SettingsList = std::vector<HTTPSetting>;

}
