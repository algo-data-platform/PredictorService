#pragma once

#include "thrift/lib/cpp2/protocol/Serializer.h"

namespace common {

// mutual conversion between obj and compact by compact
template <class T>
bool serializeThriftObjToCompact(std::string *str, const T &obj) {
  try {
    apache::thrift::CompactSerializer::serialize(obj, str);
  }
  catch (std::exception e) {
    return false;
  }
  return true;
}

template <class T>
bool deserializeThriftObjFromCompact(T *obj, const std::string &str) {
  try {
    apache::thrift::CompactSerializer::deserialize(str, *obj);
  }
  catch (std::exception e) {
    return false;
  }
  return true;
}

// mutual conversion between obj and compact by binary
template <class T>
bool serializeThriftObjToBinary(std::string *str, const T &obj) {
  try {
    apache::thrift::BinarySerializer::serialize(obj, str);
  }
  catch (std::exception e) {
    return false;
  }
  return true;
}

template <class T>
bool deserializeThriftObjFromBinary(T *obj, const std::string &str) {
  try {
    apache::thrift::BinarySerializer::deserialize(str, *obj);
  }
  catch (std::exception e) {
    return false;
  }
  return true;
}

// mutual conversion between obj and compact by json
template <class T>
bool serializeThriftObjToJson(std::string *str, const T &obj) {
  try {
    apache::thrift::JSONSerializer::serialize(obj, str);
  }
  catch (std::exception e) {
    return false;
  }
  return true;
}

template <class T>
bool deserializeThriftObjFromJson(T *obj, const std::string &str) {
  try {
    apache::thrift::JSONSerializer::deserialize(str, *obj);
  }
  catch (std::exception e) {
    return false;
  }
  return true;
}

// mutual conversion between obj and compact by json
template <class T>
bool serializeThriftObjToSimpleJson(std::string *str, const T &obj) {
  try {
    apache::thrift::SimpleJSONSerializer::serialize(obj, str);
  }
  catch (std::exception e) {
    return false;
  }
  return true;
}

template <class T>
bool deserializeThriftObjFromSimpleJson(T *obj, const std::string &str) {
  try {
    apache::thrift::SimpleJSONSerializer::deserialize(str, *obj);
  }
  catch (std::exception e) {
    return false;
  }
  return true;
}

}  // namespace common

