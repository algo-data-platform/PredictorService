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
#include <node.h>
#include <node_buffer.h>
#include <v8.h>

#include <folly/Singleton.h>
#include <folly/synchronization/CallOnce.h>
#include <thrift/lib/cpp/concurrency/FunctionRunner.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>

using namespace v8;
std::unique_ptr<folly::EventBase> integrated_uv_event_base;

void run_loop(uv_async_t* /* handle */, int /* status */) {
  integrated_uv_event_base->loop();
}

uv_async_t async;

std::string getStringAttrSafe(Local<Object>& obj, const std::string& key) {
  auto keyStr = String::New(key.c_str());
  if (!obj->Has(keyStr)) {
    return "";
  }
  auto value = obj->Get(keyStr);
  if (!value->IsString()) {
    return "";
  }
  return *String::Utf8Value(obj->Get(keyStr)->ToString());
}

std::list<std::string> getStringListSafe(
    Local<Object>& obj,
    const std::string& key) {
  std::list<std::string> result;
  auto keyStr = String::New(key.c_str());
  if (!obj->Has(keyStr)) {
    return result;
  }
  auto value = obj->Get(keyStr);
  if (!value->IsArray()) {
    return result;
  }
  auto jsArray = Local<Array>::Cast(value);
  auto len = jsArray->Length();
  for (size_t i = 0; i < len; ++i) {
    auto item = jsArray->Get(i);
    if (item->IsString()) {
      result.push_back(*String::Utf8Value(item->ToString()));
    }
  }
  return result;
}

folly::SSLContext::SSLVerifyPeerEnum getSSLVerify(Local<Object>& cfg) {
  auto result = folly::SSLContext::SSLVerifyPeerEnum::VERIFY;
  auto attr = getStringAttrSafe(cfg, "verify");
  if (attr == "verify_required") {
    result = folly::SSLContext::SSLVerifyPeerEnum::VERIFY_REQ_CLIENT_CERT;
  } else if (attr == "no_verify") {
    result = folly::SSLContext::SSLVerifyPeerEnum::NO_VERIFY;
  }
  return result;
}

class ThriftServerCallback : public node::ObjectWrap {
 public:
  static v8::Persistent<v8::Function> constructor;

  static void Init(v8::Handle<v8::Object> exports) {
    Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    tpl->SetClassName(String::NewSymbol("ThriftServerCallback"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    tpl->PrototypeTemplate()->Set(
      String::NewSymbol("sendReply"),
      FunctionTemplate::New(ThriftServerCallback::sendReply)->GetFunction());

    constructor = Persistent<Function>::New(tpl->GetFunction());
    exports->Set(String::NewSymbol("ThriftServerCallback"), constructor);
  }

  static v8::Handle<v8::Value> New(const v8::Arguments& args) {
    HandleScope scope;

    if (args.IsConstructCall()) {
      auto s = new ThriftServerCallback;
      s->Wrap(args.This());
      return args.This();
    } else {
      const int argc = 0;
      Local<Value> argv[argc];
      return scope.Close(constructor->NewInstance(argc, argv));
    }
  }
  static Local<Object> NewInstance() {
    const unsigned argc = 0;
    Handle<Value> argv[argc] = {};
    Local<Object> instance = constructor->NewInstance(argc, argv);

    return instance;
  }

  static Handle<Value> sendReply(const Arguments& args) {
    auto obj = ObjectWrap::Unwrap<ThriftServerCallback>(args.This());
    Local<Object> bufferObj = args[0]->ToObject();
    char* bufferData = node::Buffer::Data(bufferObj);
    size_t bufferLen = node::Buffer::Length(bufferObj);
    auto iobuf =
      apache::thrift::transport::THeader::transform(
        folly::IOBuf::copyBuffer(bufferData, bufferLen),
        obj->reqCtx_->getHeader()->getWriteTransforms(),
        obj->reqCtx_->getHeader()->getMinCompressBytes());
    obj->eb_->runInEventBaseThread(
      [r = std::move(obj->req_), iobuf = std::move(iobuf)]() mutable {
        r->sendReply(std::move(iobuf));
    });
    return args.This();
  }

  static void setRequest(
      Local<Object> arg,
      std::unique_ptr<apache::thrift::ResponseChannel::Request> req,
      apache::thrift::Cpp2RequestContext* reqCtx,
      folly::EventBase* base) {
    auto obj = ObjectWrap::Unwrap<ThriftServerCallback>(arg);
    obj->req_ = std::move(req);
    obj->eb_ = base;
    obj->reqCtx_ = reqCtx;
  }

 private:
  std::unique_ptr<apache::thrift::ResponseChannel::Request> req_;
  folly::EventBase* eb_;
  apache::thrift::Cpp2RequestContext* reqCtx_;
};

Persistent<Function> ThriftServerCallback::constructor;

// Processor to forward to Node.js processor
class NodeProcessor : public apache::thrift::AsyncProcessor {
 public:
  explicit NodeProcessor(Persistent<Object>& server,
                         Persistent<Function>& iface)
      : server_(server)
      , iface_(iface) {}

  void process(
      std::unique_ptr<apache::thrift::ResponseChannel::Request> req,
      std::unique_ptr<folly::IOBuf> buf,
      apache::thrift::protocol::PROTOCOL_TYPES,
      apache::thrift::Cpp2RequestContext* context,
      folly::EventBase* eb,
      apache::thrift::concurrency::ThreadManager*) override {
    integrated_uv_event_base->runInEventBaseThread(
      [=, req = std::move(req), buf = std::move(buf)]() mutable {
        HandleScope scope;
        buf->coalesce();
        uint64_t resp;
        char* data;
        void *user_data = NULL;
        assert(buf->length() > 0);
        node::Buffer* inBuffer = node::Buffer::New(buf->length());
        assert(inBuffer);
        memcpy(
          node::Buffer::Data(inBuffer), buf->data(), buf->length());

        Local<Object> globalObj = Context::GetCurrent()->Global();
        Local<Function> bufferConstructor = Local<Function>::Cast(
          globalObj->Get(String::New("Buffer")));
        std::array<Handle<class v8::Value>, 2> constructorArgs = {
            {inBuffer->handle_, v8::Integer::New(buf->length())}};

        Local<Object> bufin =
            bufferConstructor->NewInstance(2, constructorArgs.data());

        auto callback = ThriftServerCallback::NewInstance();
        ThriftServerCallback::setRequest(
          callback,
          std::move(req),
          context,
          eb);

        const unsigned argc = 3;
        Local<Value> argv[argc] = {
          Local<Object>::New(server_), callback, bufin};
        iface_->Call(server_, argc, argv);

        // Delete objects created since scope creation on stack (i.e. inBuffer)
        scope.Close(Undefined());
      });
    uv_async_send(&async);
  }

  bool isOnewayMethod(
      const folly::IOBuf*,
      const apache::thrift::transport::THeader*) override {
    return false;
  }
 private:
  Persistent<Object> server_;
  Persistent<Function> iface_;
};


class NodeServerInterface : public apache::thrift::ServerInterface {
 public:
  explicit NodeServerInterface(Persistent<Object> server,
    Persistent<Function>& iface)
      : server_(server)
      , iface_(iface) {}

  std::unique_ptr<apache::thrift::AsyncProcessor> getProcessor() override {
    return std::unique_ptr<apache::thrift::AsyncProcessor>(
      new NodeProcessor(server_, iface_));
  }
 private:
  Persistent<Object> server_;
  Persistent<Function> iface_;
};

// Wrapped NodeJS class
class CppThriftServer : public node::ObjectWrap {
 public:

  static Handle<Value> setPort(const Arguments& args) {
    auto obj = ObjectWrap::Unwrap<CppThriftServer>(args.This());
    obj->server_.setPort(args[0]->NumberValue());

    return args.This();
  }

  static Handle<Value> setTimeout(const Arguments& args) {
    auto obj = ObjectWrap::Unwrap<CppThriftServer>(args.This());
    obj->server_.setTaskExpireTime(
      std::chrono::milliseconds(args[0]->ToNumber()->ToInt32()->Value())
    );

    return args.This();
  }

  static Handle<Value> setInterface(const Arguments& args) {
    auto obj = ObjectWrap::Unwrap<CppThriftServer>(args.This());
    auto localproc = Local<Function>::Cast(args[0]);
    auto proc = Persistent<Function>::New(localproc);
    auto srv = Persistent<Object>::New(args.This());
    auto interface = std::make_shared<NodeServerInterface>(srv, proc);
    obj->server_.setInterface(interface);

    return srv;
  }

  static Handle<Value> serve(const Arguments& args) {
    auto obj = ObjectWrap::Unwrap<CppThriftServer>(args.This());
    std::thread([=](){
        obj->server_.serve();
      }).detach();

    return args.This();
  }

  static Handle<Value> setSSLConfigJs(const Arguments& args) {
    auto obj = ObjectWrap::Unwrap<CppThriftServer>(args.This());
    auto sslConfig = args[0]->ToObject();

    auto certPath = getStringAttrSafe(sslConfig, "certPath");
    auto keyPath = getStringAttrSafe(sslConfig, "keyPath");
    if (certPath.empty() ^ keyPath.empty()) {
      return ThrowException(
          String::New("certPath and keyPath must both be populated"));
    }
    auto cfg = std::make_shared<wangle::SSLContextConfig>();
    cfg->clientCAFile = getStringAttrSafe(sslConfig, "clientCaPath");
    if (!certPath.empty()) {
      auto keyPwPath = getStringAttrSafe(sslConfig, "keyPwPath");
      cfg->setCertificate(certPath, keyPath, keyPwPath);
    }

    cfg->clientVerification = getSSLVerify(sslConfig);
    auto eccCurve = getStringAttrSafe(sslConfig, "eccCurveName");
    if (!eccCurve.empty()) {
      cfg->eccCurveName = eccCurve;
    }
    auto alpnProtocols = getStringListSafe(sslConfig, "alpnProtocols");
    cfg->setNextProtocols(alpnProtocols);

    auto sessionContext = getStringAttrSafe(sslConfig, "sessionContext");
    if (!sessionContext.empty()) {
      cfg->sessionContext = sessionContext;
    }

    obj->server_.setSSLConfig(cfg);

    auto policyAttr = getStringAttrSafe(sslConfig, "policy");
    auto policy = apache::thrift::SSLPolicy::PERMITTED;
    if (policyAttr == "required") {
      policy = apache::thrift::SSLPolicy::REQUIRED;
    }
    obj->server_.setSSLPolicy(policy);

    auto ticketFilePath = getStringAttrSafe(sslConfig, "ticketFilePath");
    if (!ticketFilePath.empty()) {
      obj->server_.watchTicketPathForChanges(ticketFilePath, true);
    }

    return args.This();
  }

  static void Init(v8::Handle<v8::Object> exports) {
    Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    tpl->SetClassName(String::NewSymbol("CppThriftServer"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    tpl->PrototypeTemplate()->Set(
      String::NewSymbol("setPort"),
      FunctionTemplate::New(CppThriftServer::setPort)->GetFunction());
    tpl->PrototypeTemplate()->Set(
      String::NewSymbol("setTimeout"),
      FunctionTemplate::New(CppThriftServer::setTimeout)->GetFunction());
    tpl->PrototypeTemplate()->Set(
      String::NewSymbol("serve"),
      FunctionTemplate::New(CppThriftServer::serve)->GetFunction());
    tpl->PrototypeTemplate()->Set(
      String::NewSymbol("setInterface"),
      FunctionTemplate::New(CppThriftServer::setInterface)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("setSSLConfig"),
        FunctionTemplate::New(CppThriftServer::setSSLConfigJs)->GetFunction());
    constructor = Persistent<Function>::New(tpl->GetFunction());
    exports->Set(String::NewSymbol("CppThriftServer"), constructor);
  }

 private:
  explicit CppThriftServer() {}
  ~CppThriftServer() override {}

  static v8::Handle<v8::Value> New(const v8::Arguments& args) {
    HandleScope scope;

    if (args.IsConstructCall()) {
      auto s = new CppThriftServer;
      s->Wrap(args.This());
      return args.This();
    } else {
      const int argc = 0;
      Local<Value> argv[argc];
      return scope.Close(constructor->NewInstance(argc, argv));
    }
  }
  static v8::Persistent<v8::Function> constructor;
 public:
  apache::thrift::ThriftServer server_;
};

Persistent<Function> CppThriftServer::constructor;

void init(Handle<Object> exports) {
  static folly::once_flag flag;
  folly::call_once(
      flag, [] { folly::SingletonVault::singleton()->registrationComplete(); });
  integrated_uv_event_base.reset(new folly::EventBase());
  CppThriftServer::Init(exports);
  ThriftServerCallback::Init(exports);
  uv_async_init(uv_default_loop(), &async, run_loop);
}

NODE_MODULE(CppThriftServer, init)
