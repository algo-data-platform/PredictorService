/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#pragma once

#include "src/gen-cpp2/MyServiceFast.h"

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/GeneratedCodeHelper.h>
#include <thrift/lib/cpp2/GeneratedSerializationCodeHelper.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>

namespace cpp2 {
typedef apache::thrift::ThriftPresult<false, apache::thrift::FieldData<1, apache::thrift::protocol::T_I64, int64_t*>> MyServiceFast_hasDataById_pargs;
typedef apache::thrift::ThriftPresult<true, apache::thrift::FieldData<0, apache::thrift::protocol::T_BOOL, bool*>> MyServiceFast_hasDataById_presult;
typedef apache::thrift::ThriftPresult<false, apache::thrift::FieldData<1, apache::thrift::protocol::T_I64, int64_t*>> MyServiceFast_getDataById_pargs;
typedef apache::thrift::ThriftPresult<true, apache::thrift::FieldData<0, apache::thrift::protocol::T_STRING, std::string*>> MyServiceFast_getDataById_presult;
typedef apache::thrift::ThriftPresult<false, apache::thrift::FieldData<1, apache::thrift::protocol::T_I64, int64_t*>, apache::thrift::FieldData<2, apache::thrift::protocol::T_STRING, std::string*>> MyServiceFast_putDataById_pargs;
typedef apache::thrift::ThriftPresult<true> MyServiceFast_putDataById_presult;
typedef apache::thrift::ThriftPresult<false, apache::thrift::FieldData<1, apache::thrift::protocol::T_I64, int64_t*>, apache::thrift::FieldData<2, apache::thrift::protocol::T_STRING, std::string*>> MyServiceFast_lobDataById_pargs;
template <typename ProtocolIn_, typename ProtocolOut_>
void MyServiceFastAsyncProcessor::process_hasDataById(std::unique_ptr<apache::thrift::ResponseChannel::Request> req, std::unique_ptr<folly::IOBuf> buf, std::unique_ptr<ProtocolIn_> iprot,apache::thrift::Cpp2RequestContext* ctx,folly::EventBase* eb, apache::thrift::concurrency::ThreadManager* tm) {
  // make sure getConnectionContext is null
  // so async calls don't accidentally use it
  iface_->setConnectionContext(nullptr);
  MyServiceFast_hasDataById_pargs args;
  int64_t uarg_id{0};
  args.get<0>().value = &uarg_id;
  std::unique_ptr<apache::thrift::ContextStack> ctxStack(this->getContextStack(this->getServiceName(), "MyServiceFast.hasDataById", ctx));
  try {
    deserializeRequest(args, buf.get(), iprot.get(), ctxStack.get());
  }
  catch (const std::exception& ex) {
    ProtocolOut_ prot;
    if (req) {
      LOG(ERROR) << ex.what() << " in function hasDataById";
      apache::thrift::TApplicationException x(apache::thrift::TApplicationException::TApplicationExceptionType::PROTOCOL_ERROR, ex.what());
      folly::IOBufQueue queue = serializeException("hasDataById", &prot, ctx->getProtoSeqId(), nullptr, x);
      queue.append(apache::thrift::transport::THeader::transform(queue.move(), ctx->getHeader()->getWriteTransforms(), ctx->getHeader()->getMinCompressBytes()));
      eb->runInEventBaseThread([queue = std::move(queue), req = std::move(req)]() mutable {
        if (req->isStream()) {
          req->sendStreamReply({queue.move(), {}});
        } else {
          req->sendReply(queue.move());
        }
      }
      );
      return;
    }
    else {
      LOG(ERROR) << ex.what() << " in oneway function hasDataById";
    }
  }
  auto callback = std::make_unique<apache::thrift::HandlerCallback<bool>>(std::move(req), std::move(ctxStack), return_hasDataById<ProtocolIn_,ProtocolOut_>, throw_wrapped_hasDataById<ProtocolIn_, ProtocolOut_>, ctx->getProtoSeqId(), eb, tm, ctx);
  ctx->setStartedProcessing();
  iface_->async_eb_hasDataById(std::move(callback), args.get<0>().ref());
}

template <class ProtocolIn_, class ProtocolOut_>
folly::IOBufQueue MyServiceFastAsyncProcessor::return_hasDataById(int32_t protoSeqId, apache::thrift::ContextStack* ctx, bool const& _return) {
  ProtocolOut_ prot;
  MyServiceFast_hasDataById_presult result;
  result.get<0>().value = const_cast<bool*>(&_return);
  result.setIsSet(0, true);
  return serializeResponse("hasDataById", &prot, protoSeqId, ctx, result);
}

template <class ProtocolIn_, class ProtocolOut_>
void MyServiceFastAsyncProcessor::throw_wrapped_hasDataById(std::unique_ptr<apache::thrift::ResponseChannel::Request> req,int32_t protoSeqId,apache::thrift::ContextStack* ctx,folly::exception_wrapper ew,apache::thrift::Cpp2RequestContext* reqCtx) {
  if (!ew) {
    return;
  }
  ProtocolOut_ prot;
   {
    if (req) {
      LOG(ERROR) << ew << " in function hasDataById";
      apache::thrift::TApplicationException x(ew.what().toStdString());
      ctx->userExceptionWrapped(false, ew);
      ctx->handlerErrorWrapped(ew);
      folly::IOBufQueue queue = serializeException("hasDataById", &prot, protoSeqId, ctx, x);
      queue.append(apache::thrift::transport::THeader::transform(queue.move(), reqCtx->getHeader()->getWriteTransforms(), reqCtx->getHeader()->getMinCompressBytes()));
      req->sendReply(queue.move());
      return;
    }
    else {
      LOG(ERROR) << ew << " in oneway function hasDataById";
    }
  }
}

template <typename ProtocolIn_, typename ProtocolOut_>
void MyServiceFastAsyncProcessor::process_getDataById(std::unique_ptr<apache::thrift::ResponseChannel::Request> req, std::unique_ptr<folly::IOBuf> buf, std::unique_ptr<ProtocolIn_> iprot,apache::thrift::Cpp2RequestContext* ctx,folly::EventBase* eb, apache::thrift::concurrency::ThreadManager* tm) {
  // make sure getConnectionContext is null
  // so async calls don't accidentally use it
  iface_->setConnectionContext(nullptr);
  MyServiceFast_getDataById_pargs args;
  int64_t uarg_id{0};
  args.get<0>().value = &uarg_id;
  std::unique_ptr<apache::thrift::ContextStack> ctxStack(this->getContextStack(this->getServiceName(), "MyServiceFast.getDataById", ctx));
  try {
    deserializeRequest(args, buf.get(), iprot.get(), ctxStack.get());
  }
  catch (const std::exception& ex) {
    ProtocolOut_ prot;
    if (req) {
      LOG(ERROR) << ex.what() << " in function getDataById";
      apache::thrift::TApplicationException x(apache::thrift::TApplicationException::TApplicationExceptionType::PROTOCOL_ERROR, ex.what());
      folly::IOBufQueue queue = serializeException("getDataById", &prot, ctx->getProtoSeqId(), nullptr, x);
      queue.append(apache::thrift::transport::THeader::transform(queue.move(), ctx->getHeader()->getWriteTransforms(), ctx->getHeader()->getMinCompressBytes()));
      eb->runInEventBaseThread([queue = std::move(queue), req = std::move(req)]() mutable {
        if (req->isStream()) {
          req->sendStreamReply({queue.move(), {}});
        } else {
          req->sendReply(queue.move());
        }
      }
      );
      return;
    }
    else {
      LOG(ERROR) << ex.what() << " in oneway function getDataById";
    }
  }
  auto callback = std::make_unique<apache::thrift::HandlerCallback<std::string>>(std::move(req), std::move(ctxStack), return_getDataById<ProtocolIn_,ProtocolOut_>, throw_wrapped_getDataById<ProtocolIn_, ProtocolOut_>, ctx->getProtoSeqId(), eb, tm, ctx);
  ctx->setStartedProcessing();
  iface_->async_eb_getDataById(std::move(callback), args.get<0>().ref());
}

template <class ProtocolIn_, class ProtocolOut_>
folly::IOBufQueue MyServiceFastAsyncProcessor::return_getDataById(int32_t protoSeqId, apache::thrift::ContextStack* ctx, std::string const& _return) {
  ProtocolOut_ prot;
  MyServiceFast_getDataById_presult result;
  result.get<0>().value = const_cast<std::string*>(&_return);
  result.setIsSet(0, true);
  return serializeResponse("getDataById", &prot, protoSeqId, ctx, result);
}

template <class ProtocolIn_, class ProtocolOut_>
void MyServiceFastAsyncProcessor::throw_wrapped_getDataById(std::unique_ptr<apache::thrift::ResponseChannel::Request> req,int32_t protoSeqId,apache::thrift::ContextStack* ctx,folly::exception_wrapper ew,apache::thrift::Cpp2RequestContext* reqCtx) {
  if (!ew) {
    return;
  }
  ProtocolOut_ prot;
   {
    if (req) {
      LOG(ERROR) << ew << " in function getDataById";
      apache::thrift::TApplicationException x(ew.what().toStdString());
      ctx->userExceptionWrapped(false, ew);
      ctx->handlerErrorWrapped(ew);
      folly::IOBufQueue queue = serializeException("getDataById", &prot, protoSeqId, ctx, x);
      queue.append(apache::thrift::transport::THeader::transform(queue.move(), reqCtx->getHeader()->getWriteTransforms(), reqCtx->getHeader()->getMinCompressBytes()));
      req->sendReply(queue.move());
      return;
    }
    else {
      LOG(ERROR) << ew << " in oneway function getDataById";
    }
  }
}

template <typename ProtocolIn_, typename ProtocolOut_>
void MyServiceFastAsyncProcessor::process_putDataById(std::unique_ptr<apache::thrift::ResponseChannel::Request> req, std::unique_ptr<folly::IOBuf> buf, std::unique_ptr<ProtocolIn_> iprot,apache::thrift::Cpp2RequestContext* ctx,folly::EventBase* eb, apache::thrift::concurrency::ThreadManager* tm) {
  // make sure getConnectionContext is null
  // so async calls don't accidentally use it
  iface_->setConnectionContext(nullptr);
  MyServiceFast_putDataById_pargs args;
  int64_t uarg_id{0};
  args.get<0>().value = &uarg_id;
  auto uarg_data = std::make_unique<std::string>();
  args.get<1>().value = uarg_data.get();
  std::unique_ptr<apache::thrift::ContextStack> ctxStack(this->getContextStack(this->getServiceName(), "MyServiceFast.putDataById", ctx));
  try {
    deserializeRequest(args, buf.get(), iprot.get(), ctxStack.get());
  }
  catch (const std::exception& ex) {
    ProtocolOut_ prot;
    if (req) {
      LOG(ERROR) << ex.what() << " in function putDataById";
      apache::thrift::TApplicationException x(apache::thrift::TApplicationException::TApplicationExceptionType::PROTOCOL_ERROR, ex.what());
      folly::IOBufQueue queue = serializeException("putDataById", &prot, ctx->getProtoSeqId(), nullptr, x);
      queue.append(apache::thrift::transport::THeader::transform(queue.move(), ctx->getHeader()->getWriteTransforms(), ctx->getHeader()->getMinCompressBytes()));
      eb->runInEventBaseThread([queue = std::move(queue), req = std::move(req)]() mutable {
        if (req->isStream()) {
          req->sendStreamReply({queue.move(), {}});
        } else {
          req->sendReply(queue.move());
        }
      }
      );
      return;
    }
    else {
      LOG(ERROR) << ex.what() << " in oneway function putDataById";
    }
  }
  auto callback = std::make_unique<apache::thrift::HandlerCallback<void>>(std::move(req), std::move(ctxStack), return_putDataById<ProtocolIn_,ProtocolOut_>, throw_wrapped_putDataById<ProtocolIn_, ProtocolOut_>, ctx->getProtoSeqId(), eb, tm, ctx);
  ctx->setStartedProcessing();
  iface_->async_eb_putDataById(std::move(callback), args.get<0>().ref(), args.get<1>().ref());
}

template <class ProtocolIn_, class ProtocolOut_>
folly::IOBufQueue MyServiceFastAsyncProcessor::return_putDataById(int32_t protoSeqId, apache::thrift::ContextStack* ctx) {
  ProtocolOut_ prot;
  MyServiceFast_putDataById_presult result;
  return serializeResponse("putDataById", &prot, protoSeqId, ctx, result);
}

template <class ProtocolIn_, class ProtocolOut_>
void MyServiceFastAsyncProcessor::throw_wrapped_putDataById(std::unique_ptr<apache::thrift::ResponseChannel::Request> req,int32_t protoSeqId,apache::thrift::ContextStack* ctx,folly::exception_wrapper ew,apache::thrift::Cpp2RequestContext* reqCtx) {
  if (!ew) {
    return;
  }
  ProtocolOut_ prot;
   {
    if (req) {
      LOG(ERROR) << ew << " in function putDataById";
      apache::thrift::TApplicationException x(ew.what().toStdString());
      ctx->userExceptionWrapped(false, ew);
      ctx->handlerErrorWrapped(ew);
      folly::IOBufQueue queue = serializeException("putDataById", &prot, protoSeqId, ctx, x);
      queue.append(apache::thrift::transport::THeader::transform(queue.move(), reqCtx->getHeader()->getWriteTransforms(), reqCtx->getHeader()->getMinCompressBytes()));
      req->sendReply(queue.move());
      return;
    }
    else {
      LOG(ERROR) << ew << " in oneway function putDataById";
    }
  }
}

template <typename ProtocolIn_, typename ProtocolOut_>
void MyServiceFastAsyncProcessor::process_lobDataById(std::unique_ptr<apache::thrift::ResponseChannel::Request> req, std::unique_ptr<folly::IOBuf> buf, std::unique_ptr<ProtocolIn_> iprot,apache::thrift::Cpp2RequestContext* ctx,folly::EventBase* eb, apache::thrift::concurrency::ThreadManager* tm) {
  if (!req->isOneway()) {
    if (req->isStream()) {
      req->sendStreamReply({std::unique_ptr<folly::IOBuf>(), {}});
    } else {
      req->sendReply(std::unique_ptr<folly::IOBuf>());
    }
  }
  // make sure getConnectionContext is null
  // so async calls don't accidentally use it
  iface_->setConnectionContext(nullptr);
  MyServiceFast_lobDataById_pargs args;
  int64_t uarg_id{0};
  args.get<0>().value = &uarg_id;
  auto uarg_data = std::make_unique<std::string>();
  args.get<1>().value = uarg_data.get();
  std::unique_ptr<apache::thrift::ContextStack> ctxStack(this->getContextStack(this->getServiceName(), "MyServiceFast.lobDataById", ctx));
  try {
    deserializeRequest(args, buf.get(), iprot.get(), ctxStack.get());
  }
  catch (const std::exception& ex) {
    LOG(ERROR) << ex.what() << " in function lobDataById";
    return;
  }
  auto callback = std::make_unique<apache::thrift::HandlerCallbackBase>(std::move(req), std::move(ctxStack), nullptr, eb, tm, ctx);
  ctx->setStartedProcessing();
  iface_->async_eb_lobDataById(std::move(callback), args.get<0>().ref(), args.get<1>().ref());
}

} // cpp2
namespace apache { namespace thrift {

}} // apache::thrift