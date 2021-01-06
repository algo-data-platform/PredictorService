/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "src/gen-cpp2/module_types.h"
#include "src/gen-cpp2/module_types.tcc"

#include <algorithm>
#include <folly/Indestructible.h>

#include "src/gen-cpp2/module_data.h"

namespace cpp2 {

Banal::Banal(apache::thrift::FragileConstructor) {}

void Banal::__clear() {
  // clear all fields
}

bool Banal::operator==(const Banal& rhs) const {
  (void)rhs;
  auto& lhs = *this;
  return true;
}

bool Banal::operator<(const Banal& rhs) const {
  (void)rhs;
  auto& lhs = *this;
  return false;
}

void Banal::translateFieldName(FOLLY_MAYBE_UNUSED folly::StringPiece _fname, FOLLY_MAYBE_UNUSED int16_t& fid, FOLLY_MAYBE_UNUSED apache::thrift::protocol::TType& _ftype) {
  if (false) {}
}

void swap(Banal& a, Banal& b) {
  using ::std::swap;
  (void)a;
  (void)b;
}

template void Banal::readNoXfer<>(apache::thrift::BinaryProtocolReader*);
template uint32_t Banal::write<>(apache::thrift::BinaryProtocolWriter*) const;
template uint32_t Banal::serializedSize<>(apache::thrift::BinaryProtocolWriter const*) const;
template uint32_t Banal::serializedSizeZC<>(apache::thrift::BinaryProtocolWriter const*) const;
template void Banal::readNoXfer<>(apache::thrift::CompactProtocolReader*);
template uint32_t Banal::write<>(apache::thrift::CompactProtocolWriter*) const;
template uint32_t Banal::serializedSize<>(apache::thrift::CompactProtocolWriter const*) const;
template uint32_t Banal::serializedSizeZC<>(apache::thrift::CompactProtocolWriter const*) const;

} // cpp2
namespace cpp2 {

Fiery::Fiery(apache::thrift::FragileConstructor, std::string message__arg) :
    message(std::move(message__arg)) {}

void Fiery::__clear() {
  // clear all fields
  message = apache::thrift::StringTraits< std::string>::fromStringLiteral("");
}

bool Fiery::operator==(const Fiery& rhs) const {
  (void)rhs;
  auto& lhs = *this;
  if (!(lhs.message == rhs.message)) {
    return false;
  }
  return true;
}

bool Fiery::operator<(const Fiery& rhs) const {
  (void)rhs;
  auto& lhs = *this;
  if (!(lhs.message == rhs.message)) {
    return lhs.message < rhs.message;
  }
  return false;
}

void Fiery::translateFieldName(FOLLY_MAYBE_UNUSED folly::StringPiece _fname, FOLLY_MAYBE_UNUSED int16_t& fid, FOLLY_MAYBE_UNUSED apache::thrift::protocol::TType& _ftype) {
  if (false) {}
  else if (_fname == "message") {
    fid = 1;
    _ftype = apache::thrift::protocol::T_STRING;
  }
}

void swap(Fiery& a, Fiery& b) {
  using ::std::swap;
  swap(a.message, b.message);
}

template void Fiery::readNoXfer<>(apache::thrift::BinaryProtocolReader*);
template uint32_t Fiery::write<>(apache::thrift::BinaryProtocolWriter*) const;
template uint32_t Fiery::serializedSize<>(apache::thrift::BinaryProtocolWriter const*) const;
template uint32_t Fiery::serializedSizeZC<>(apache::thrift::BinaryProtocolWriter const*) const;
template void Fiery::readNoXfer<>(apache::thrift::CompactProtocolReader*);
template uint32_t Fiery::write<>(apache::thrift::CompactProtocolWriter*) const;
template uint32_t Fiery::serializedSize<>(apache::thrift::CompactProtocolWriter const*) const;
template uint32_t Fiery::serializedSizeZC<>(apache::thrift::CompactProtocolWriter const*) const;

} // cpp2