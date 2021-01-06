/**
 * Autogenerated by Thrift
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#pragma once

#include <thrift/lib/cpp2/GeneratedHeaderHelper.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>


// BEGIN declare_enums

// END declare_enums
// BEGIN struct_indirection

// END struct_indirection
// BEGIN forward_declare
namespace cpp2 {
class OldStructure;
class NewStructure;
class NewStructure2;
class NewStructureNested;
class NewStructureNestedField;
} // cpp2
// END forward_declare
// BEGIN typedefs
namespace cpp2 {
typedef std::map<int16_t, float> FloatFeatures;
typedef std::map<int64_t, double> DoubleMapType;
typedef std::map<int16_t,  ::cpp2::DoubleMapType> OldMapMap;
typedef std::map<int32_t,  ::cpp2::DoubleMapType> NewMapMap;
typedef std::map<int16_t, std::vector<float>> OldMapList;
typedef std::map<int32_t, std::vector<float>> NewMapList;

} // cpp2
// END typedefs
// BEGIN hash_and_equal_to
// END hash_and_equal_to
namespace cpp2 {
class OldStructure final : private apache::thrift::detail::st::ComparisonOperators<OldStructure> {
 public:

  OldStructure() {}
  // FragileConstructor for use in initialization lists only.
  OldStructure(apache::thrift::FragileConstructor, std::map<int16_t, double> features__arg);
  template <typename T__ThriftWrappedArgument__Ctor, typename... Args__ThriftWrappedArgument__Ctor>
  OldStructure(::apache::thrift::detail::argument_wrapper<1, T__ThriftWrappedArgument__Ctor> arg, Args__ThriftWrappedArgument__Ctor&&... args):
    OldStructure(std::forward<Args__ThriftWrappedArgument__Ctor>(args)...)
  {
    features = arg.move();
    __isset.features = true;
  }

  OldStructure(OldStructure&&) = default;

  OldStructure(const OldStructure&) = default;

  OldStructure& operator=(OldStructure&&) = default;

  OldStructure& operator=(const OldStructure&) = default;
  void __clear();
  std::map<int16_t, double> features;

  struct __isset {
    bool features;
  } __isset = {};
  bool operator==(const OldStructure& rhs) const;
  bool operator<(const OldStructure& rhs) const;
  const std::map<int16_t, double>& get_features() const&;
  std::map<int16_t, double> get_features() &&;

  template <typename T_OldStructure_features_struct_setter = std::map<int16_t, double>>
  std::map<int16_t, double>& set_features(T_OldStructure_features_struct_setter&& features_) {
    features = std::forward<T_OldStructure_features_struct_setter>(features_);
    __isset.features = true;
    return features;
  }

  template <class Protocol_>
  uint32_t read(Protocol_* iprot);
  template <class Protocol_>
  uint32_t serializedSize(Protocol_ const* prot_) const;
  template <class Protocol_>
  uint32_t serializedSizeZC(Protocol_ const* prot_) const;
  template <class Protocol_>
  uint32_t write(Protocol_* prot_) const;

 private:
  static void translateFieldName(FOLLY_MAYBE_UNUSED folly::StringPiece _fname, FOLLY_MAYBE_UNUSED int16_t& fid, FOLLY_MAYBE_UNUSED apache::thrift::protocol::TType& _ftype);

  template <class Protocol_>
  void readNoXfer(Protocol_* iprot);

  friend class ::apache::thrift::Cpp2Ops< OldStructure >;
};

void swap(OldStructure& a, OldStructure& b);
extern template void OldStructure::readNoXfer<>(apache::thrift::BinaryProtocolReader*);
extern template uint32_t OldStructure::write<>(apache::thrift::BinaryProtocolWriter*) const;
extern template uint32_t OldStructure::serializedSize<>(apache::thrift::BinaryProtocolWriter const*) const;
extern template uint32_t OldStructure::serializedSizeZC<>(apache::thrift::BinaryProtocolWriter const*) const;
extern template void OldStructure::readNoXfer<>(apache::thrift::CompactProtocolReader*);
extern template uint32_t OldStructure::write<>(apache::thrift::CompactProtocolWriter*) const;
extern template uint32_t OldStructure::serializedSize<>(apache::thrift::CompactProtocolWriter const*) const;
extern template uint32_t OldStructure::serializedSizeZC<>(apache::thrift::CompactProtocolWriter const*) const;
extern template void OldStructure::readNoXfer<>(apache::thrift::SimpleJSONProtocolReader*);
extern template uint32_t OldStructure::write<>(apache::thrift::SimpleJSONProtocolWriter*) const;
extern template uint32_t OldStructure::serializedSize<>(apache::thrift::SimpleJSONProtocolWriter const*) const;
extern template uint32_t OldStructure::serializedSizeZC<>(apache::thrift::SimpleJSONProtocolWriter const*) const;

template <class Protocol_>
uint32_t OldStructure::read(Protocol_* iprot) {
  auto _xferStart = iprot->getCurrentPosition().getCurrentPosition();
  readNoXfer(iprot);
  return iprot->getCurrentPosition().getCurrentPosition() - _xferStart;
}

} // cpp2
namespace apache { namespace thrift {

template <> inline void Cpp2Ops< ::cpp2::OldStructure>::clear( ::cpp2::OldStructure* obj) {
  return obj->__clear();
}

template <> inline constexpr apache::thrift::protocol::TType Cpp2Ops< ::cpp2::OldStructure>::thriftType() {
  return apache::thrift::protocol::T_STRUCT;
}

template <> template <class Protocol> uint32_t Cpp2Ops< ::cpp2::OldStructure>::write(Protocol* proto,  ::cpp2::OldStructure const* obj) {
  return obj->write(proto);
}

template <> template <class Protocol> void Cpp2Ops< ::cpp2::OldStructure>::read(Protocol* proto,  ::cpp2::OldStructure* obj) {
  return obj->readNoXfer(proto);
}

template <> template <class Protocol> uint32_t Cpp2Ops< ::cpp2::OldStructure>::serializedSize(Protocol const* proto,  ::cpp2::OldStructure const* obj) {
  return obj->serializedSize(proto);
}

template <> template <class Protocol> uint32_t Cpp2Ops< ::cpp2::OldStructure>::serializedSizeZC(Protocol const* proto,  ::cpp2::OldStructure const* obj) {
  return obj->serializedSizeZC(proto);
}

}} // apache::thrift
namespace cpp2 {
class NewStructure final : private apache::thrift::detail::st::ComparisonOperators<NewStructure> {
 public:

  NewStructure() {}
  // FragileConstructor for use in initialization lists only.
  NewStructure(apache::thrift::FragileConstructor, std::map<int16_t, double> features__arg);
  template <typename T__ThriftWrappedArgument__Ctor, typename... Args__ThriftWrappedArgument__Ctor>
  NewStructure(::apache::thrift::detail::argument_wrapper<1, T__ThriftWrappedArgument__Ctor> arg, Args__ThriftWrappedArgument__Ctor&&... args):
    NewStructure(std::forward<Args__ThriftWrappedArgument__Ctor>(args)...)
  {
    features = arg.move();
    __isset.features = true;
  }

  NewStructure(NewStructure&&) = default;

  NewStructure(const NewStructure&) = default;

  NewStructure& operator=(NewStructure&&) = default;

  NewStructure& operator=(const NewStructure&) = default;
  void __clear();
  std::map<int16_t, double> features;

  struct __isset {
    bool features;
  } __isset = {};
  bool operator==(const NewStructure& rhs) const;
  bool operator<(const NewStructure& rhs) const;
  const std::map<int16_t, double>& get_features() const&;
  std::map<int16_t, double> get_features() &&;

  template <typename T_NewStructure_features_struct_setter = std::map<int16_t, double>>
  std::map<int16_t, double>& set_features(T_NewStructure_features_struct_setter&& features_) {
    features = std::forward<T_NewStructure_features_struct_setter>(features_);
    __isset.features = true;
    return features;
  }

  template <class Protocol_>
  uint32_t read(Protocol_* iprot);
  template <class Protocol_>
  uint32_t serializedSize(Protocol_ const* prot_) const;
  template <class Protocol_>
  uint32_t serializedSizeZC(Protocol_ const* prot_) const;
  template <class Protocol_>
  uint32_t write(Protocol_* prot_) const;

 private:
  static void translateFieldName(FOLLY_MAYBE_UNUSED folly::StringPiece _fname, FOLLY_MAYBE_UNUSED int16_t& fid, FOLLY_MAYBE_UNUSED apache::thrift::protocol::TType& _ftype);

  template <class Protocol_>
  void readNoXfer(Protocol_* iprot);

  friend class ::apache::thrift::Cpp2Ops< NewStructure >;
};

void swap(NewStructure& a, NewStructure& b);
extern template void NewStructure::readNoXfer<>(apache::thrift::BinaryProtocolReader*);
extern template uint32_t NewStructure::write<>(apache::thrift::BinaryProtocolWriter*) const;
extern template uint32_t NewStructure::serializedSize<>(apache::thrift::BinaryProtocolWriter const*) const;
extern template uint32_t NewStructure::serializedSizeZC<>(apache::thrift::BinaryProtocolWriter const*) const;
extern template void NewStructure::readNoXfer<>(apache::thrift::CompactProtocolReader*);
extern template uint32_t NewStructure::write<>(apache::thrift::CompactProtocolWriter*) const;
extern template uint32_t NewStructure::serializedSize<>(apache::thrift::CompactProtocolWriter const*) const;
extern template uint32_t NewStructure::serializedSizeZC<>(apache::thrift::CompactProtocolWriter const*) const;
extern template void NewStructure::readNoXfer<>(apache::thrift::SimpleJSONProtocolReader*);
extern template uint32_t NewStructure::write<>(apache::thrift::SimpleJSONProtocolWriter*) const;
extern template uint32_t NewStructure::serializedSize<>(apache::thrift::SimpleJSONProtocolWriter const*) const;
extern template uint32_t NewStructure::serializedSizeZC<>(apache::thrift::SimpleJSONProtocolWriter const*) const;

template <class Protocol_>
uint32_t NewStructure::read(Protocol_* iprot) {
  auto _xferStart = iprot->getCurrentPosition().getCurrentPosition();
  readNoXfer(iprot);
  return iprot->getCurrentPosition().getCurrentPosition() - _xferStart;
}

} // cpp2
namespace apache { namespace thrift {

template <> inline void Cpp2Ops< ::cpp2::NewStructure>::clear( ::cpp2::NewStructure* obj) {
  return obj->__clear();
}

template <> inline constexpr apache::thrift::protocol::TType Cpp2Ops< ::cpp2::NewStructure>::thriftType() {
  return apache::thrift::protocol::T_STRUCT;
}

template <> template <class Protocol> uint32_t Cpp2Ops< ::cpp2::NewStructure>::write(Protocol* proto,  ::cpp2::NewStructure const* obj) {
  return obj->write(proto);
}

template <> template <class Protocol> void Cpp2Ops< ::cpp2::NewStructure>::read(Protocol* proto,  ::cpp2::NewStructure* obj) {
  return obj->readNoXfer(proto);
}

template <> template <class Protocol> uint32_t Cpp2Ops< ::cpp2::NewStructure>::serializedSize(Protocol const* proto,  ::cpp2::NewStructure const* obj) {
  return obj->serializedSize(proto);
}

template <> template <class Protocol> uint32_t Cpp2Ops< ::cpp2::NewStructure>::serializedSizeZC(Protocol const* proto,  ::cpp2::NewStructure const* obj) {
  return obj->serializedSizeZC(proto);
}

}} // apache::thrift
namespace cpp2 {
class NewStructure2 final : private apache::thrift::detail::st::ComparisonOperators<NewStructure2> {
 public:

  NewStructure2() {}
  // FragileConstructor for use in initialization lists only.
  NewStructure2(apache::thrift::FragileConstructor,  ::cpp2::FloatFeatures features__arg);
  template <typename T__ThriftWrappedArgument__Ctor, typename... Args__ThriftWrappedArgument__Ctor>
  NewStructure2(::apache::thrift::detail::argument_wrapper<1, T__ThriftWrappedArgument__Ctor> arg, Args__ThriftWrappedArgument__Ctor&&... args):
    NewStructure2(std::forward<Args__ThriftWrappedArgument__Ctor>(args)...)
  {
    features = arg.move();
    __isset.features = true;
  }

  NewStructure2(NewStructure2&&) = default;

  NewStructure2(const NewStructure2&) = default;

  NewStructure2& operator=(NewStructure2&&) = default;

  NewStructure2& operator=(const NewStructure2&) = default;
  void __clear();
   ::cpp2::FloatFeatures features;

  struct __isset {
    bool features;
  } __isset = {};
  bool operator==(const NewStructure2& rhs) const;
  bool operator<(const NewStructure2& rhs) const;
  const  ::cpp2::FloatFeatures& get_features() const&;
   ::cpp2::FloatFeatures get_features() &&;

  template <typename T_NewStructure2_features_struct_setter =  ::cpp2::FloatFeatures>
   ::cpp2::FloatFeatures& set_features(T_NewStructure2_features_struct_setter&& features_) {
    features = std::forward<T_NewStructure2_features_struct_setter>(features_);
    __isset.features = true;
    return features;
  }

  template <class Protocol_>
  uint32_t read(Protocol_* iprot);
  template <class Protocol_>
  uint32_t serializedSize(Protocol_ const* prot_) const;
  template <class Protocol_>
  uint32_t serializedSizeZC(Protocol_ const* prot_) const;
  template <class Protocol_>
  uint32_t write(Protocol_* prot_) const;

 private:
  static void translateFieldName(FOLLY_MAYBE_UNUSED folly::StringPiece _fname, FOLLY_MAYBE_UNUSED int16_t& fid, FOLLY_MAYBE_UNUSED apache::thrift::protocol::TType& _ftype);

  template <class Protocol_>
  void readNoXfer(Protocol_* iprot);

  friend class ::apache::thrift::Cpp2Ops< NewStructure2 >;
};

void swap(NewStructure2& a, NewStructure2& b);
extern template void NewStructure2::readNoXfer<>(apache::thrift::BinaryProtocolReader*);
extern template uint32_t NewStructure2::write<>(apache::thrift::BinaryProtocolWriter*) const;
extern template uint32_t NewStructure2::serializedSize<>(apache::thrift::BinaryProtocolWriter const*) const;
extern template uint32_t NewStructure2::serializedSizeZC<>(apache::thrift::BinaryProtocolWriter const*) const;
extern template void NewStructure2::readNoXfer<>(apache::thrift::CompactProtocolReader*);
extern template uint32_t NewStructure2::write<>(apache::thrift::CompactProtocolWriter*) const;
extern template uint32_t NewStructure2::serializedSize<>(apache::thrift::CompactProtocolWriter const*) const;
extern template uint32_t NewStructure2::serializedSizeZC<>(apache::thrift::CompactProtocolWriter const*) const;
extern template void NewStructure2::readNoXfer<>(apache::thrift::SimpleJSONProtocolReader*);
extern template uint32_t NewStructure2::write<>(apache::thrift::SimpleJSONProtocolWriter*) const;
extern template uint32_t NewStructure2::serializedSize<>(apache::thrift::SimpleJSONProtocolWriter const*) const;
extern template uint32_t NewStructure2::serializedSizeZC<>(apache::thrift::SimpleJSONProtocolWriter const*) const;

template <class Protocol_>
uint32_t NewStructure2::read(Protocol_* iprot) {
  auto _xferStart = iprot->getCurrentPosition().getCurrentPosition();
  readNoXfer(iprot);
  return iprot->getCurrentPosition().getCurrentPosition() - _xferStart;
}

} // cpp2
namespace apache { namespace thrift {

template <> inline void Cpp2Ops< ::cpp2::NewStructure2>::clear( ::cpp2::NewStructure2* obj) {
  return obj->__clear();
}

template <> inline constexpr apache::thrift::protocol::TType Cpp2Ops< ::cpp2::NewStructure2>::thriftType() {
  return apache::thrift::protocol::T_STRUCT;
}

template <> template <class Protocol> uint32_t Cpp2Ops< ::cpp2::NewStructure2>::write(Protocol* proto,  ::cpp2::NewStructure2 const* obj) {
  return obj->write(proto);
}

template <> template <class Protocol> void Cpp2Ops< ::cpp2::NewStructure2>::read(Protocol* proto,  ::cpp2::NewStructure2* obj) {
  return obj->readNoXfer(proto);
}

template <> template <class Protocol> uint32_t Cpp2Ops< ::cpp2::NewStructure2>::serializedSize(Protocol const* proto,  ::cpp2::NewStructure2 const* obj) {
  return obj->serializedSize(proto);
}

template <> template <class Protocol> uint32_t Cpp2Ops< ::cpp2::NewStructure2>::serializedSizeZC(Protocol const* proto,  ::cpp2::NewStructure2 const* obj) {
  return obj->serializedSizeZC(proto);
}

}} // apache::thrift
namespace cpp2 {
class NewStructureNested final : private apache::thrift::detail::st::ComparisonOperators<NewStructureNested> {
 public:

  NewStructureNested() {}
  // FragileConstructor for use in initialization lists only.
  NewStructureNested(apache::thrift::FragileConstructor, std::vector< ::cpp2::FloatFeatures> lst__arg, std::map<int16_t,  ::cpp2::FloatFeatures> mp__arg, std::set< ::cpp2::FloatFeatures> s__arg);
  template <typename T__ThriftWrappedArgument__Ctor, typename... Args__ThriftWrappedArgument__Ctor>
  NewStructureNested(::apache::thrift::detail::argument_wrapper<1, T__ThriftWrappedArgument__Ctor> arg, Args__ThriftWrappedArgument__Ctor&&... args):
    NewStructureNested(std::forward<Args__ThriftWrappedArgument__Ctor>(args)...)
  {
    lst = arg.move();
    __isset.lst = true;
  }
  template <typename T__ThriftWrappedArgument__Ctor, typename... Args__ThriftWrappedArgument__Ctor>
  NewStructureNested(::apache::thrift::detail::argument_wrapper<2, T__ThriftWrappedArgument__Ctor> arg, Args__ThriftWrappedArgument__Ctor&&... args):
    NewStructureNested(std::forward<Args__ThriftWrappedArgument__Ctor>(args)...)
  {
    mp = arg.move();
    __isset.mp = true;
  }
  template <typename T__ThriftWrappedArgument__Ctor, typename... Args__ThriftWrappedArgument__Ctor>
  NewStructureNested(::apache::thrift::detail::argument_wrapper<3, T__ThriftWrappedArgument__Ctor> arg, Args__ThriftWrappedArgument__Ctor&&... args):
    NewStructureNested(std::forward<Args__ThriftWrappedArgument__Ctor>(args)...)
  {
    s = arg.move();
    __isset.s = true;
  }

  NewStructureNested(NewStructureNested&&) = default;

  NewStructureNested(const NewStructureNested&) = default;

  NewStructureNested& operator=(NewStructureNested&&) = default;

  NewStructureNested& operator=(const NewStructureNested&) = default;
  void __clear();
  std::vector< ::cpp2::FloatFeatures> lst;
  std::map<int16_t,  ::cpp2::FloatFeatures> mp;
  std::set< ::cpp2::FloatFeatures> s;

  struct __isset {
    bool lst;
    bool mp;
    bool s;
  } __isset = {};
  bool operator==(const NewStructureNested& rhs) const;
  bool operator<(const NewStructureNested& rhs) const;
  const std::vector< ::cpp2::FloatFeatures>& get_lst() const&;
  std::vector< ::cpp2::FloatFeatures> get_lst() &&;

  template <typename T_NewStructureNested_lst_struct_setter = std::vector< ::cpp2::FloatFeatures>>
  std::vector< ::cpp2::FloatFeatures>& set_lst(T_NewStructureNested_lst_struct_setter&& lst_) {
    lst = std::forward<T_NewStructureNested_lst_struct_setter>(lst_);
    __isset.lst = true;
    return lst;
  }
  const std::map<int16_t,  ::cpp2::FloatFeatures>& get_mp() const&;
  std::map<int16_t,  ::cpp2::FloatFeatures> get_mp() &&;

  template <typename T_NewStructureNested_mp_struct_setter = std::map<int16_t,  ::cpp2::FloatFeatures>>
  std::map<int16_t,  ::cpp2::FloatFeatures>& set_mp(T_NewStructureNested_mp_struct_setter&& mp_) {
    mp = std::forward<T_NewStructureNested_mp_struct_setter>(mp_);
    __isset.mp = true;
    return mp;
  }
  const std::set< ::cpp2::FloatFeatures>& get_s() const&;
  std::set< ::cpp2::FloatFeatures> get_s() &&;

  template <typename T_NewStructureNested_s_struct_setter = std::set< ::cpp2::FloatFeatures>>
  std::set< ::cpp2::FloatFeatures>& set_s(T_NewStructureNested_s_struct_setter&& s_) {
    s = std::forward<T_NewStructureNested_s_struct_setter>(s_);
    __isset.s = true;
    return s;
  }

  template <class Protocol_>
  uint32_t read(Protocol_* iprot);
  template <class Protocol_>
  uint32_t serializedSize(Protocol_ const* prot_) const;
  template <class Protocol_>
  uint32_t serializedSizeZC(Protocol_ const* prot_) const;
  template <class Protocol_>
  uint32_t write(Protocol_* prot_) const;

 private:
  static void translateFieldName(FOLLY_MAYBE_UNUSED folly::StringPiece _fname, FOLLY_MAYBE_UNUSED int16_t& fid, FOLLY_MAYBE_UNUSED apache::thrift::protocol::TType& _ftype);

  template <class Protocol_>
  void readNoXfer(Protocol_* iprot);

  friend class ::apache::thrift::Cpp2Ops< NewStructureNested >;
};

void swap(NewStructureNested& a, NewStructureNested& b);
extern template void NewStructureNested::readNoXfer<>(apache::thrift::BinaryProtocolReader*);
extern template uint32_t NewStructureNested::write<>(apache::thrift::BinaryProtocolWriter*) const;
extern template uint32_t NewStructureNested::serializedSize<>(apache::thrift::BinaryProtocolWriter const*) const;
extern template uint32_t NewStructureNested::serializedSizeZC<>(apache::thrift::BinaryProtocolWriter const*) const;
extern template void NewStructureNested::readNoXfer<>(apache::thrift::CompactProtocolReader*);
extern template uint32_t NewStructureNested::write<>(apache::thrift::CompactProtocolWriter*) const;
extern template uint32_t NewStructureNested::serializedSize<>(apache::thrift::CompactProtocolWriter const*) const;
extern template uint32_t NewStructureNested::serializedSizeZC<>(apache::thrift::CompactProtocolWriter const*) const;
extern template void NewStructureNested::readNoXfer<>(apache::thrift::SimpleJSONProtocolReader*);
extern template uint32_t NewStructureNested::write<>(apache::thrift::SimpleJSONProtocolWriter*) const;
extern template uint32_t NewStructureNested::serializedSize<>(apache::thrift::SimpleJSONProtocolWriter const*) const;
extern template uint32_t NewStructureNested::serializedSizeZC<>(apache::thrift::SimpleJSONProtocolWriter const*) const;

template <class Protocol_>
uint32_t NewStructureNested::read(Protocol_* iprot) {
  auto _xferStart = iprot->getCurrentPosition().getCurrentPosition();
  readNoXfer(iprot);
  return iprot->getCurrentPosition().getCurrentPosition() - _xferStart;
}

} // cpp2
namespace apache { namespace thrift {

template <> inline void Cpp2Ops< ::cpp2::NewStructureNested>::clear( ::cpp2::NewStructureNested* obj) {
  return obj->__clear();
}

template <> inline constexpr apache::thrift::protocol::TType Cpp2Ops< ::cpp2::NewStructureNested>::thriftType() {
  return apache::thrift::protocol::T_STRUCT;
}

template <> template <class Protocol> uint32_t Cpp2Ops< ::cpp2::NewStructureNested>::write(Protocol* proto,  ::cpp2::NewStructureNested const* obj) {
  return obj->write(proto);
}

template <> template <class Protocol> void Cpp2Ops< ::cpp2::NewStructureNested>::read(Protocol* proto,  ::cpp2::NewStructureNested* obj) {
  return obj->readNoXfer(proto);
}

template <> template <class Protocol> uint32_t Cpp2Ops< ::cpp2::NewStructureNested>::serializedSize(Protocol const* proto,  ::cpp2::NewStructureNested const* obj) {
  return obj->serializedSize(proto);
}

template <> template <class Protocol> uint32_t Cpp2Ops< ::cpp2::NewStructureNested>::serializedSizeZC(Protocol const* proto,  ::cpp2::NewStructureNested const* obj) {
  return obj->serializedSizeZC(proto);
}

}} // apache::thrift
namespace cpp2 {
class NewStructureNestedField final : private apache::thrift::detail::st::ComparisonOperators<NewStructureNestedField> {
 public:

  NewStructureNestedField() {}
  // FragileConstructor for use in initialization lists only.
  NewStructureNestedField(apache::thrift::FragileConstructor,  ::cpp2::NewStructureNested f__arg);
  template <typename T__ThriftWrappedArgument__Ctor, typename... Args__ThriftWrappedArgument__Ctor>
  NewStructureNestedField(::apache::thrift::detail::argument_wrapper<1, T__ThriftWrappedArgument__Ctor> arg, Args__ThriftWrappedArgument__Ctor&&... args):
    NewStructureNestedField(std::forward<Args__ThriftWrappedArgument__Ctor>(args)...)
  {
    f = arg.move();
    __isset.f = true;
  }

  NewStructureNestedField(NewStructureNestedField&&) = default;

  NewStructureNestedField(const NewStructureNestedField&) = default;

  NewStructureNestedField& operator=(NewStructureNestedField&&) = default;

  NewStructureNestedField& operator=(const NewStructureNestedField&) = default;
  void __clear();
   ::cpp2::NewStructureNested f;

  struct __isset {
    bool f;
  } __isset = {};
  bool operator==(const NewStructureNestedField& rhs) const;
  bool operator<(const NewStructureNestedField& rhs) const;
  const  ::cpp2::NewStructureNested& get_f() const&;
   ::cpp2::NewStructureNested get_f() &&;

  template <typename T_NewStructureNestedField_f_struct_setter =  ::cpp2::NewStructureNested>
   ::cpp2::NewStructureNested& set_f(T_NewStructureNestedField_f_struct_setter&& f_) {
    f = std::forward<T_NewStructureNestedField_f_struct_setter>(f_);
    __isset.f = true;
    return f;
  }

  template <class Protocol_>
  uint32_t read(Protocol_* iprot);
  template <class Protocol_>
  uint32_t serializedSize(Protocol_ const* prot_) const;
  template <class Protocol_>
  uint32_t serializedSizeZC(Protocol_ const* prot_) const;
  template <class Protocol_>
  uint32_t write(Protocol_* prot_) const;

 private:
  static void translateFieldName(FOLLY_MAYBE_UNUSED folly::StringPiece _fname, FOLLY_MAYBE_UNUSED int16_t& fid, FOLLY_MAYBE_UNUSED apache::thrift::protocol::TType& _ftype);

  template <class Protocol_>
  void readNoXfer(Protocol_* iprot);

  friend class ::apache::thrift::Cpp2Ops< NewStructureNestedField >;
};

void swap(NewStructureNestedField& a, NewStructureNestedField& b);
extern template void NewStructureNestedField::readNoXfer<>(apache::thrift::BinaryProtocolReader*);
extern template uint32_t NewStructureNestedField::write<>(apache::thrift::BinaryProtocolWriter*) const;
extern template uint32_t NewStructureNestedField::serializedSize<>(apache::thrift::BinaryProtocolWriter const*) const;
extern template uint32_t NewStructureNestedField::serializedSizeZC<>(apache::thrift::BinaryProtocolWriter const*) const;
extern template void NewStructureNestedField::readNoXfer<>(apache::thrift::CompactProtocolReader*);
extern template uint32_t NewStructureNestedField::write<>(apache::thrift::CompactProtocolWriter*) const;
extern template uint32_t NewStructureNestedField::serializedSize<>(apache::thrift::CompactProtocolWriter const*) const;
extern template uint32_t NewStructureNestedField::serializedSizeZC<>(apache::thrift::CompactProtocolWriter const*) const;
extern template void NewStructureNestedField::readNoXfer<>(apache::thrift::SimpleJSONProtocolReader*);
extern template uint32_t NewStructureNestedField::write<>(apache::thrift::SimpleJSONProtocolWriter*) const;
extern template uint32_t NewStructureNestedField::serializedSize<>(apache::thrift::SimpleJSONProtocolWriter const*) const;
extern template uint32_t NewStructureNestedField::serializedSizeZC<>(apache::thrift::SimpleJSONProtocolWriter const*) const;

template <class Protocol_>
uint32_t NewStructureNestedField::read(Protocol_* iprot) {
  auto _xferStart = iprot->getCurrentPosition().getCurrentPosition();
  readNoXfer(iprot);
  return iprot->getCurrentPosition().getCurrentPosition() - _xferStart;
}

} // cpp2
namespace apache { namespace thrift {

template <> inline void Cpp2Ops< ::cpp2::NewStructureNestedField>::clear( ::cpp2::NewStructureNestedField* obj) {
  return obj->__clear();
}

template <> inline constexpr apache::thrift::protocol::TType Cpp2Ops< ::cpp2::NewStructureNestedField>::thriftType() {
  return apache::thrift::protocol::T_STRUCT;
}

template <> template <class Protocol> uint32_t Cpp2Ops< ::cpp2::NewStructureNestedField>::write(Protocol* proto,  ::cpp2::NewStructureNestedField const* obj) {
  return obj->write(proto);
}

template <> template <class Protocol> void Cpp2Ops< ::cpp2::NewStructureNestedField>::read(Protocol* proto,  ::cpp2::NewStructureNestedField* obj) {
  return obj->readNoXfer(proto);
}

template <> template <class Protocol> uint32_t Cpp2Ops< ::cpp2::NewStructureNestedField>::serializedSize(Protocol const* proto,  ::cpp2::NewStructureNestedField const* obj) {
  return obj->serializedSize(proto);
}

template <> template <class Protocol> uint32_t Cpp2Ops< ::cpp2::NewStructureNestedField>::serializedSizeZC(Protocol const* proto,  ::cpp2::NewStructureNestedField const* obj) {
  return obj->serializedSizeZC(proto);
}

}} // apache::thrift
