/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */

#pragma once
#include <src/gen-cpp2/EmptyService.h>
#include <src/gen-cpp2/ReturnService.h>
#include <src/gen-cpp2/ParamService.h>

#include <folly/futures/Future.h>
#include <folly/futures/Promise.h>
#include <folly/Unit.h>
#include <thrift/lib/py3/clientcallbacks.h>

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace some {
namespace valid {
namespace ns {

class EmptyServiceClientWrapper {
  protected:
    std::shared_ptr<some::valid::ns::EmptyServiceAsyncClient> async_client;
    std::shared_ptr<apache::thrift::RequestChannel> channel_;
  public:
    explicit EmptyServiceClientWrapper(
      std::shared_ptr<some::valid::ns::EmptyServiceAsyncClient> async_client,
      std::shared_ptr<apache::thrift::RequestChannel> channel);
    virtual ~EmptyServiceClientWrapper();

    folly::Future<folly::Unit> disconnect();
    void disconnectInLoop();
    void setPersistentHeader(const std::string& key, const std::string& value);

};


class ReturnServiceClientWrapper {
  protected:
    std::shared_ptr<some::valid::ns::ReturnServiceAsyncClient> async_client;
    std::shared_ptr<apache::thrift::RequestChannel> channel_;
  public:
    explicit ReturnServiceClientWrapper(
      std::shared_ptr<some::valid::ns::ReturnServiceAsyncClient> async_client,
      std::shared_ptr<apache::thrift::RequestChannel> channel);
    virtual ~ReturnServiceClientWrapper();

    folly::Future<folly::Unit> disconnect();
    void disconnectInLoop();
    void setPersistentHeader(const std::string& key, const std::string& value);

    folly::Future<folly::Unit> noReturn(
      apache::thrift::RpcOptions& rpcOptions);
    folly::Future<bool> boolReturn(
      apache::thrift::RpcOptions& rpcOptions);
    folly::Future<int16_t> i16Return(
      apache::thrift::RpcOptions& rpcOptions);
    folly::Future<int32_t> i32Return(
      apache::thrift::RpcOptions& rpcOptions);
    folly::Future<int64_t> i64Return(
      apache::thrift::RpcOptions& rpcOptions);
    folly::Future<float> floatReturn(
      apache::thrift::RpcOptions& rpcOptions);
    folly::Future<double> doubleReturn(
      apache::thrift::RpcOptions& rpcOptions);
    folly::Future<std::string> stringReturn(
      apache::thrift::RpcOptions& rpcOptions);
    folly::Future<std::string> binaryReturn(
      apache::thrift::RpcOptions& rpcOptions);
    folly::Future<std::map<std::string,int64_t>> mapReturn(
      apache::thrift::RpcOptions& rpcOptions);
    folly::Future<int32_t> simpleTypedefReturn(
      apache::thrift::RpcOptions& rpcOptions);
    folly::Future<std::vector<std::map<some::valid::ns::Empty,some::valid::ns::MyStruct>>> complexTypedefReturn(
      apache::thrift::RpcOptions& rpcOptions);
    folly::Future<std::vector<std::vector<std::vector<std::map<some::valid::ns::Empty,some::valid::ns::MyStruct>>>>> list_mostComplexTypedefReturn(
      apache::thrift::RpcOptions& rpcOptions);
    folly::Future<some::valid::ns::MyEnumA> enumReturn(
      apache::thrift::RpcOptions& rpcOptions);
    folly::Future<std::vector<some::valid::ns::MyEnumA>> list_EnumReturn(
      apache::thrift::RpcOptions& rpcOptions);
    folly::Future<some::valid::ns::MyStruct> structReturn(
      apache::thrift::RpcOptions& rpcOptions);
    folly::Future<std::set<some::valid::ns::MyStruct>> set_StructReturn(
      apache::thrift::RpcOptions& rpcOptions);
    folly::Future<some::valid::ns::ComplexUnion> unionReturn(
      apache::thrift::RpcOptions& rpcOptions);
    folly::Future<std::vector<some::valid::ns::ComplexUnion>> list_UnionReturn(
      apache::thrift::RpcOptions& rpcOptions);
    folly::Future<folly::IOBuf> readDataEb(
      apache::thrift::RpcOptions& rpcOptions,
      int64_t arg_size);
    folly::Future<std::unique_ptr<folly::IOBuf>> readData(
      apache::thrift::RpcOptions& rpcOptions,
      int64_t arg_size);
};


class ParamServiceClientWrapper {
  protected:
    std::shared_ptr<some::valid::ns::ParamServiceAsyncClient> async_client;
    std::shared_ptr<apache::thrift::RequestChannel> channel_;
  public:
    explicit ParamServiceClientWrapper(
      std::shared_ptr<some::valid::ns::ParamServiceAsyncClient> async_client,
      std::shared_ptr<apache::thrift::RequestChannel> channel);
    virtual ~ParamServiceClientWrapper();

    folly::Future<folly::Unit> disconnect();
    void disconnectInLoop();
    void setPersistentHeader(const std::string& key, const std::string& value);

    folly::Future<folly::Unit> void_ret_i16_param(
      apache::thrift::RpcOptions& rpcOptions,
      int16_t arg_param1);
    folly::Future<folly::Unit> void_ret_byte_i16_param(
      apache::thrift::RpcOptions& rpcOptions,
      int8_t arg_param1,
      int16_t arg_param2);
    folly::Future<folly::Unit> void_ret_map_param(
      apache::thrift::RpcOptions& rpcOptions,
      std::map<std::string,int64_t> arg_param1);
    folly::Future<folly::Unit> void_ret_map_setlist_param(
      apache::thrift::RpcOptions& rpcOptions,
      std::map<std::string,int64_t> arg_param1,
      std::set<std::vector<std::string>> arg_param2);
    folly::Future<folly::Unit> void_ret_map_typedef_param(
      apache::thrift::RpcOptions& rpcOptions,
      int32_t arg_param1);
    folly::Future<folly::Unit> void_ret_enum_param(
      apache::thrift::RpcOptions& rpcOptions,
      some::valid::ns::MyEnumA arg_param1);
    folly::Future<folly::Unit> void_ret_struct_param(
      apache::thrift::RpcOptions& rpcOptions,
      some::valid::ns::MyStruct arg_param1);
    folly::Future<folly::Unit> void_ret_listunion_param(
      apache::thrift::RpcOptions& rpcOptions,
      std::vector<some::valid::ns::ComplexUnion> arg_param1);
    folly::Future<bool> bool_ret_i32_i64_param(
      apache::thrift::RpcOptions& rpcOptions,
      int32_t arg_param1,
      int64_t arg_param2);
    folly::Future<bool> bool_ret_map_param(
      apache::thrift::RpcOptions& rpcOptions,
      std::map<std::string,int64_t> arg_param1);
    folly::Future<bool> bool_ret_union_param(
      apache::thrift::RpcOptions& rpcOptions,
      some::valid::ns::ComplexUnion arg_param1);
    folly::Future<int64_t> i64_ret_float_double_param(
      apache::thrift::RpcOptions& rpcOptions,
      float arg_param1,
      double arg_param2);
    folly::Future<int64_t> i64_ret_string_typedef_param(
      apache::thrift::RpcOptions& rpcOptions,
      std::string arg_param1,
      std::set<std::vector<std::vector<std::map<some::valid::ns::Empty,some::valid::ns::MyStruct>>>> arg_param2);
    folly::Future<int64_t> i64_ret_i32_i32_i32_i32_i32_param(
      apache::thrift::RpcOptions& rpcOptions,
      int32_t arg_param1,
      int32_t arg_param2,
      int32_t arg_param3,
      int32_t arg_param4,
      int32_t arg_param5);
    folly::Future<double> double_ret_setstruct_param(
      apache::thrift::RpcOptions& rpcOptions,
      std::set<some::valid::ns::MyStruct> arg_param1);
    folly::Future<std::string> string_ret_string_param(
      apache::thrift::RpcOptions& rpcOptions,
      std::string arg_param1);
    folly::Future<std::string> binary_ret_binary_param(
      apache::thrift::RpcOptions& rpcOptions,
      std::string arg_param1);
    folly::Future<std::map<std::string,int64_t>> map_ret_bool_param(
      apache::thrift::RpcOptions& rpcOptions,
      bool arg_param1);
    folly::Future<std::vector<bool>> list_ret_map_setlist_param(
      apache::thrift::RpcOptions& rpcOptions,
      std::map<int32_t,std::vector<std::string>> arg_param1,
      std::vector<std::string> arg_param2);
    folly::Future<std::map<std::set<std::vector<int32_t>>,std::map<std::vector<std::set<std::string>>,std::string>>> mapsetlistmapliststring_ret_listlistlist_param(
      apache::thrift::RpcOptions& rpcOptions,
      std::vector<std::vector<std::vector<std::vector<int32_t>>>> arg_param1);
    folly::Future<int32_t> typedef_ret_i32_param(
      apache::thrift::RpcOptions& rpcOptions,
      int32_t arg_param1);
    folly::Future<std::vector<int32_t>> listtypedef_ret_typedef_param(
      apache::thrift::RpcOptions& rpcOptions,
      std::vector<std::map<some::valid::ns::Empty,some::valid::ns::MyStruct>> arg_param1);
    folly::Future<some::valid::ns::MyEnumA> enum_ret_double_param(
      apache::thrift::RpcOptions& rpcOptions,
      double arg_param1);
    folly::Future<some::valid::ns::MyEnumA> enum_ret_double_enum_param(
      apache::thrift::RpcOptions& rpcOptions,
      double arg_param1,
      some::valid::ns::MyEnumA arg_param2);
    folly::Future<std::vector<some::valid::ns::MyEnumA>> listenum_ret_map_param(
      apache::thrift::RpcOptions& rpcOptions,
      std::map<std::string,int64_t> arg_param1);
    folly::Future<some::valid::ns::MyStruct> struct_ret_i16_param(
      apache::thrift::RpcOptions& rpcOptions,
      int16_t arg_param1);
    folly::Future<std::set<some::valid::ns::MyStruct>> setstruct_ret_set_param(
      apache::thrift::RpcOptions& rpcOptions,
      std::set<std::string> arg_param1);
    folly::Future<some::valid::ns::ComplexUnion> union_ret_i32_i32_param(
      apache::thrift::RpcOptions& rpcOptions,
      int32_t arg_param1,
      int32_t arg_param2);
    folly::Future<std::vector<some::valid::ns::ComplexUnion>> listunion_string_param(
      apache::thrift::RpcOptions& rpcOptions,
      std::string arg_param1);
};


} // namespace some
} // namespace valid
} // namespace ns
