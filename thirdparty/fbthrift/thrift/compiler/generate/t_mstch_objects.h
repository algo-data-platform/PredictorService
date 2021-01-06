/*
 * Copyright 2004-present Facebook, Inc.
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
#pragma once

#include <iomanip>
#include <unordered_map>

#include <mstch/mstch.hpp>

#include <thrift/compiler/generate/t_generator.h>

class mstch_base;
class mstch_generators;

enum ELEMENT_POSITION {
  NONE = 0,
  FIRST = 1,
  LAST = 2,
  FIRST_AND_LAST = 3,
};

struct mstch_cache {
  std::map<std::string, std::string> parsed_options_;
  std::unordered_map<std::string, std::shared_ptr<mstch_base>> enums_;
  std::unordered_map<std::string, std::shared_ptr<mstch_base>> structs_;
  std::unordered_map<std::string, std::shared_ptr<mstch_base>> services_;
  std::unordered_map<std::string, std::shared_ptr<mstch_base>> programs_;
};

class enum_value_generator {
 public:
  virtual ~enum_value_generator() = default;
  virtual std::shared_ptr<mstch_base> generate(
      t_enum_value const* enum_value,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos = ELEMENT_POSITION::NONE,
      int32_t /*index*/ = 0) const;
};

class enum_generator {
 public:
  virtual ~enum_generator() = default;
  virtual std::shared_ptr<mstch_base> generate(
      t_enum const* enm,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos = ELEMENT_POSITION::NONE,
      int32_t /*index*/ = 0) const;
};

class const_value_generator {
 public:
  const_value_generator() = default;
  virtual ~const_value_generator() = default;
  virtual std::shared_ptr<mstch_base> generate(
      t_const_value const* const_value,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos = ELEMENT_POSITION::NONE,
      int32_t index = 0,
      t_const const* current_const = nullptr,
      t_type const* expected_type = nullptr) const;
  virtual std::shared_ptr<mstch_base> generate(
      std::pair<t_const_value*, t_const_value*> const& value_pair,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos = ELEMENT_POSITION::NONE,
      int32_t index = 0,
      t_const const* current_const = nullptr,
      std::pair<t_type*, t_type*> const& expected_types =
          std::pair<t_type*, t_type*>(nullptr, nullptr)) const;
};

class type_generator {
 public:
  type_generator() = default;
  virtual ~type_generator() = default;
  virtual std::shared_ptr<mstch_base> generate(
      t_type const* type,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos = ELEMENT_POSITION::NONE,
      int32_t /*index*/ = 0) const;
};

class field_generator {
 public:
  field_generator() = default;
  virtual ~field_generator() = default;
  virtual std::shared_ptr<mstch_base> generate(
      t_field const* field,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos = ELEMENT_POSITION::NONE,
      int32_t index = 0) const;
};

class struct_generator {
 public:
  struct_generator() = default;
  virtual ~struct_generator() = default;
  virtual std::shared_ptr<mstch_base> generate(
      t_struct const* strct,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos = ELEMENT_POSITION::NONE,
      int32_t /*index*/ = 0) const;
};

class function_generator {
 public:
  function_generator() = default;
  virtual ~function_generator() = default;
  virtual std::shared_ptr<mstch_base> generate(
      t_function const* function,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos = ELEMENT_POSITION::NONE,
      int32_t /*index*/ = 0) const;
};

class service_generator {
 public:
  service_generator() = default;
  virtual ~service_generator() = default;
  virtual std::shared_ptr<mstch_base> generate(
      t_service const* service,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos = ELEMENT_POSITION::NONE,
      int32_t /*index*/ = 0) const;
};

class typedef_generator {
 public:
  typedef_generator() = default;
  virtual ~typedef_generator() = default;
  virtual std::shared_ptr<mstch_base> generate(
      t_typedef const* typedf,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos = ELEMENT_POSITION::NONE,
      int32_t /*index*/ = 0) const;
};

class const_generator {
 public:
  const_generator() = default;
  virtual ~const_generator() = default;
  virtual std::shared_ptr<mstch_base> generate(
      t_const const* cnst,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos = ELEMENT_POSITION::NONE,
      int32_t index = 0,
      t_const const* current_const = nullptr,
      t_type const* expected_type = nullptr) const;
};

class program_generator {
 public:
  program_generator() = default;
  virtual ~program_generator() = default;
  virtual std::shared_ptr<mstch_base> generate(
      t_program const* program,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos = ELEMENT_POSITION::NONE,
      int32_t /*index*/ = 0) const;
};

class mstch_generators {
 public:
  mstch_generators()
      : enum_value_generator_(std::make_unique<enum_value_generator>()),
        enum_generator_(std::make_unique<enum_generator>()),
        const_value_generator_(std::make_unique<const_value_generator>()),
        type_generator_(std::make_unique<type_generator>()),
        field_generator_(std::make_unique<field_generator>()),
        struct_generator_(std::make_unique<struct_generator>()),
        function_generator_(std::make_unique<function_generator>()),
        service_generator_(std::make_unique<service_generator>()),
        typedef_generator_(std::make_unique<typedef_generator>()),
        const_generator_(std::make_unique<const_generator>()),
        program_generator_(std::make_unique<program_generator>()) {}
  ~mstch_generators() = default;

  void set_enum_value_generator(std::unique_ptr<enum_value_generator> g) {
    enum_value_generator_ = std::move(g);
  }

  void set_enum_generator(std::unique_ptr<enum_generator> g) {
    enum_generator_ = std::move(g);
  }

  void set_const_value_generator(std::unique_ptr<const_value_generator> g) {
    const_value_generator_ = std::move(g);
  }

  void set_type_generator(std::unique_ptr<type_generator> g) {
    type_generator_ = std::move(g);
  }

  void set_field_generator(std::unique_ptr<field_generator> g) {
    field_generator_ = std::move(g);
  }

  void set_struct_generator(std::unique_ptr<struct_generator> g) {
    struct_generator_ = std::move(g);
  }

  void set_function_generator(std::unique_ptr<function_generator> g) {
    function_generator_ = std::move(g);
  }

  void set_service_generator(std::unique_ptr<service_generator> g) {
    service_generator_ = std::move(g);
  }

  void set_typedef_generator(std::unique_ptr<typedef_generator> g) {
    typedef_generator_ = std::move(g);
  }

  void set_const_generator(std::unique_ptr<const_generator> g) {
    const_generator_ = std::move(g);
  }

  void set_program_generator(std::unique_ptr<program_generator> g) {
    program_generator_ = std::move(g);
  }

  std::unique_ptr<enum_value_generator> enum_value_generator_;
  std::unique_ptr<enum_generator> enum_generator_;
  std::unique_ptr<const_value_generator> const_value_generator_;
  std::unique_ptr<type_generator> type_generator_;
  std::unique_ptr<field_generator> field_generator_;
  std::unique_ptr<struct_generator> struct_generator_;
  std::unique_ptr<function_generator> function_generator_;
  std::unique_ptr<service_generator> service_generator_;
  std::unique_ptr<typedef_generator> typedef_generator_;
  std::unique_ptr<const_generator> const_generator_;
  std::unique_ptr<program_generator> program_generator_;
};

class mstch_base : public mstch::object {
 public:
  mstch_base(
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION const pos)
      : generators_(generators), cache_(cache), pos_(pos) {
    register_methods(
        this,
        {
            {"first?", &mstch_base::first},
            {"last?", &mstch_base::last},
        });
  }
  virtual ~mstch_base() = default;

  mstch::node first() {
    return pos_ == ELEMENT_POSITION::FIRST ||
        pos_ == ELEMENT_POSITION::FIRST_AND_LAST;
  }
  mstch::node last() {
    return pos_ == ELEMENT_POSITION::LAST ||
        pos_ == ELEMENT_POSITION::FIRST_AND_LAST;
  }

  static t_type const* resolve_typedef(t_type const* type) {
    while (type->is_typedef()) {
      type = dynamic_cast<t_typedef const*>(type)->get_type();
    }
    return type;
  }

  static ELEMENT_POSITION element_position(size_t index, size_t length) {
    ELEMENT_POSITION pos = ELEMENT_POSITION::NONE;
    if (index == 0) {
      pos = ELEMENT_POSITION::FIRST;
    }
    if (index == length - 1) {
      pos = ELEMENT_POSITION::LAST;
    }
    if (length == 1) {
      pos = ELEMENT_POSITION::FIRST_AND_LAST;
    }
    return pos;
  }

  template <typename Container, typename Generator, typename... Args>
  static mstch::array generate_elements(
      Container const& container,
      Generator const* generator,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      Args const&... args) {
    mstch::array a{};
    for (size_t i = 0; i < container.size(); ++i) {
      auto pos = element_position(i, container.size());
      a.push_back(generator->generate(
          container[i], generators, cache, pos, i, args...));
    }
    return a;
  }

  template <typename Container, typename Generator, typename Cache>
  static mstch::array generate_elements_cached(
      Container const& container,
      Generator const* generator,
      Cache& c,
      std::string const& id,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache) {
    mstch::array a{};
    for (size_t i = 0; i < container.size(); ++i) {
      auto pos = element_position(i, container.size());
      std::string elem_id = id + container[i]->get_name();
      if (!c.count(elem_id)) {
        c[elem_id] =
            generator->generate(container[i], generators, cache, pos, i);
      }
      a.push_back(c[elem_id]);
    }
    return a;
  }

 protected:
  std::shared_ptr<mstch_generators const> generators_;
  std::shared_ptr<mstch_cache> cache_;
  ELEMENT_POSITION const pos_;
};

class mstch_enum_value : public mstch_base {
 public:
  using node_type = t_enum_value;
  mstch_enum_value(
      t_enum_value const* enm_value,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos)
      : mstch_base(generators, cache, pos), enm_value_(enm_value) {
    register_methods(
        this,
        {
            {"enumValue:name", &mstch_enum_value::name},
            {"enumValue:value", &mstch_enum_value::value},
        });
  }
  mstch::node name() {
    return enm_value_->get_name();
  }
  mstch::node value() {
    return std::to_string(enm_value_->get_value());
  }

 protected:
  t_enum_value const* enm_value_;
};

class mstch_enum : public mstch_base {
 public:
  using node_type = t_enum;
  mstch_enum(
      t_enum const* enm,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos)
      : mstch_base(generators, cache, pos), enm_(enm) {
    register_methods(
        this,
        {
            {"enum:name", &mstch_enum::name},
            {"enum:values", &mstch_enum::values},
        });
  }

  mstch::node name() {
    return enm_->get_name();
  }
  mstch::node values();

 protected:
  t_enum const* enm_;
};

class mstch_const_value : public mstch_base {
 public:
  using cv = t_const_value::t_const_value_type;
  mstch_const_value(
      t_const_value const* const_value,
      t_const const* current_const,
      t_type const* expected_type,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos,
      int32_t index)
      : mstch_base(generators, cache, pos),
        const_value_(const_value),
        current_const_(current_const),
        expected_type_(expected_type),
        type_(const_value->get_type()),
        index_(index) {
    register_methods(
        this,
        {
            {"value:index_plus_one", &mstch_const_value::index_plus_one},
            {"value:bool?", &mstch_const_value::is_bool},
            {"value:double?", &mstch_const_value::is_double},
            {"value:integer?", &mstch_const_value::is_integer},
            {"value:enum?", &mstch_const_value::is_enum},
            {"value:enum_value?", &mstch_const_value::has_enum_value},
            {"value:string?", &mstch_const_value::is_string},
            {"value:base?", &mstch_const_value::is_base},
            {"value:map?", &mstch_const_value::is_map},
            {"value:list?", &mstch_const_value::is_list},
            {"value:container?", &mstch_const_value::is_container},
            {"value:empty_container?", &mstch_const_value::is_empty_container},
            {"value:value", &mstch_const_value::value},
            {"value:integerValue", &mstch_const_value::integer_value},
            {"value:doubleValue", &mstch_const_value::double_value},
            {"value:boolValue", &mstch_const_value::bool_value},
            {"value:nonzero?", &mstch_const_value::is_non_zero},
            {"value:enum_name", &mstch_const_value::enum_name},
            {"value:enum_value_name", &mstch_const_value::enum_value_name},
            {"value:stringValue", &mstch_const_value::string_value},
            {"value:listElements", &mstch_const_value::list_elems},
            {"value:mapElements", &mstch_const_value::map_elems},
            {"value:const_struct", &mstch_const_value::const_struct},
            {"value:referenceable?", &mstch_const_value::referenceable},
            {"value:owning_const", &mstch_const_value::owning_const},
            {"value:enable_referencing",
             &mstch_const_value::enable_referencing},
        });
  }

  std::string format_double_string(const double d) {
    std::ostringstream oss;
    oss << std::setprecision(std::numeric_limits<double>::digits10) << d;
    return oss.str();
  }
  mstch::node index_plus_one() {
    return std::to_string(index_ + 1);
  }
  mstch::node is_bool() {
    return type_ == cv::CV_BOOL;
  }
  mstch::node is_double() {
    return type_ == cv::CV_DOUBLE;
  }
  mstch::node is_integer() {
    return type_ == cv::CV_INTEGER && !const_value_->is_enum();
  }
  mstch::node is_enum() {
    return type_ == cv::CV_INTEGER && const_value_->is_enum();
  }
  mstch::node has_enum_value() {
    return const_value_->get_enum_value() != nullptr;
  }
  mstch::node is_string() {
    return type_ == cv::CV_STRING;
  }
  mstch::node is_base() {
    return type_ == cv::CV_BOOL || type_ == cv::CV_DOUBLE ||
        type_ == cv::CV_INTEGER || type_ == cv::CV_STRING;
  }
  mstch::node is_map() {
    return type_ == cv::CV_MAP;
  }
  mstch::node is_list() {
    return type_ == cv::CV_LIST;
  }
  mstch::node is_container() {
    return type_ == cv::CV_MAP || type_ == cv::CV_LIST;
  }
  mstch::node is_empty_container() {
    return (type_ == cv::CV_MAP && const_value_->get_map().empty()) ||
        (type_ == cv::CV_LIST && const_value_->get_list().empty());
  }
  mstch::node value();
  mstch::node integer_value();
  mstch::node double_value();
  mstch::node bool_value();
  mstch::node is_non_zero();
  mstch::node enum_name();
  mstch::node enum_value_name();
  mstch::node string_value();
  mstch::node list_elems();
  mstch::node map_elems();
  mstch::node const_struct();
  mstch::node referenceable() {
    return current_const_ && const_value_->get_owner() &&
        current_const_ != const_value_->get_owner() && same_type_as_expected();
  }
  mstch::node owning_const();
  mstch::node enable_referencing() {
    return mstch::map{{"value:enable_referencing?", true}};
  }

 protected:
  t_const_value const* const_value_;
  t_const const* current_const_;
  t_type const* expected_type_;
  cv const type_;
  int32_t index_;

  virtual bool same_type_as_expected() const {
    return false;
  }
};

class mstch_const_value_key_mapped_pair : public mstch_base {
 public:
  mstch_const_value_key_mapped_pair(
      std::pair<t_const_value*, t_const_value*> const& pair_values,
      t_const const* current_const,
      std::pair<t_type*, t_type*> const& expected_types,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos,
      int32_t index)
      : mstch_base(generators, cache, pos),
        pair_(pair_values),
        current_const_(current_const),
        expected_types_(expected_types),
        index_(index) {
    register_methods(
        this,
        {
            {"element:key", &mstch_const_value_key_mapped_pair::element_key},
            {"element:value",
             &mstch_const_value_key_mapped_pair::element_value},
        });
  }
  mstch::node element_key();
  mstch::node element_value();

 protected:
  std::pair<t_const_value*, t_const_value*> const pair_;
  t_const const* current_const_;
  std::pair<t_type*, t_type*> const expected_types_;
  int32_t index_;
};

class mstch_type : public mstch_base {
 public:
  mstch_type(
      t_type const* type,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos)
      : mstch_base(generators, cache, pos),
        type_(type),
        resolved_type_(resolve_typedef(type)) {
    register_methods(
        this,
        {
            {"type:name", &mstch_type::name},
            {"type:void?", &mstch_type::is_void},
            {"type:string?", &mstch_type::is_string},
            {"type:binary?", &mstch_type::is_binary},
            {"type:bool?", &mstch_type::is_bool},
            {"type:byte?", &mstch_type::is_byte},
            {"type:i16?", &mstch_type::is_i16},
            {"type:i32?", &mstch_type::is_i32},
            {"type:i64?", &mstch_type::is_i64},
            {"type:double?", &mstch_type::is_double},
            {"type:float?", &mstch_type::is_float},
            {"type:struct?", &mstch_type::is_struct},
            {"type:enum?", &mstch_type::is_enum},
            {"type:stream?", &mstch_type::is_stream},
            {"type:streamresponse?", &mstch_type::is_streamresponse},
            {"type:extratype?", &mstch_type::has_extratype},
            {"type:deprecated_stream?", &mstch_type::is_deprecated_stream},
            {"type:service?", &mstch_type::is_service},
            {"type:base?", &mstch_type::is_base},
            {"type:container?", &mstch_type::is_container},
            {"type:list?", &mstch_type::is_list},
            {"type:set?", &mstch_type::is_set},
            {"type:map?", &mstch_type::is_map},
            {"type:typedef?", &mstch_type::is_typedef},
            {"type:struct", &mstch_type::get_struct},
            {"type:enum", &mstch_type::get_enum},
            {"type:listElemType", &mstch_type::get_list_type},
            {"type:setElemType", &mstch_type::get_set_type},
            {"type:streamElemType", &mstch_type::get_stream_elem_type},
            {"type:streamResponseType", &mstch_type::get_stream_response_type},
            {"type:keyType", &mstch_type::get_key_type},
            {"type:valueType", &mstch_type::get_value_type},
            {"type:typedefType", &mstch_type::get_typedef_type},
        });
  }

  mstch::node name() {
    return type_->get_name();
  }
  mstch::node is_void() {
    return resolved_type_->is_void();
  }
  mstch::node is_string() {
    return resolved_type_->is_string() && !resolved_type_->is_binary();
  }
  mstch::node is_binary() {
    return resolved_type_->is_string() && resolved_type_->is_binary();
  }
  mstch::node is_bool() {
    return resolved_type_->is_bool();
  }
  mstch::node is_byte() {
    return resolved_type_->is_byte();
  }
  mstch::node is_i16() {
    return resolved_type_->is_i16();
  }
  mstch::node is_i32() {
    return resolved_type_->is_i32();
  }
  mstch::node is_i64() {
    return resolved_type_->is_i64();
  }
  mstch::node is_double() {
    return resolved_type_->is_double();
  }
  mstch::node is_float() {
    return resolved_type_->is_float();
  }
  mstch::node is_struct() {
    return resolved_type_->is_struct() || resolved_type_->is_xception();
  }
  mstch::node is_enum() {
    return resolved_type_->is_enum();
  }
  mstch::node is_stream() {
    return resolved_type_->is_pubsub_stream();
  }
  mstch::node is_streamresponse() {
    return resolved_type_->is_streamresponse();
  }
  mstch::node has_extratype() {
    return resolved_type_->has_extratype();
  }
  mstch::node is_deprecated_stream() {
    return resolved_type_->is_stream();
  }
  mstch::node is_service() {
    return resolved_type_->is_service();
  }
  mstch::node is_base() {
    return resolved_type_->is_base_type();
  }
  mstch::node is_container() {
    return resolved_type_->is_container();
  }
  mstch::node is_list() {
    return resolved_type_->is_list();
  }
  mstch::node is_set() {
    return resolved_type_->is_set();
  }
  mstch::node is_map() {
    return resolved_type_->is_map();
  }
  mstch::node is_typedef() {
    return type_->is_typedef();
  }
  virtual std::string get_type_namespace(t_program const*) {
    return "";
  }
  mstch::node get_struct();
  mstch::node get_enum();
  mstch::node get_list_type();
  mstch::node get_set_type();
  mstch::node get_key_type();
  mstch::node get_value_type();
  mstch::node get_typedef_type();
  mstch::node get_stream_elem_type();
  mstch::node get_stream_response_type();

 protected:
  t_type const* type_;
  t_type const* resolved_type_;
};

class mstch_field : public mstch_base {
 public:
  mstch_field(
      t_field const* field,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos,
      int32_t index)
      : mstch_base(generators, cache, pos), field_(field), index_(index) {
    register_methods(
        this,
        {
            {"field:name", &mstch_field::name},
            {"field:key", &mstch_field::key},
            {"field:value", &mstch_field::value},
            {"field:type", &mstch_field::type},
            {"field:next_field_key", &mstch_field::next_field_key},
            {"field:next_field_type", &mstch_field::next_field_type},
            {"field:index", &mstch_field::index},
            {"field:index_plus_one", &mstch_field::index_plus_one},
            {"field:required?", &mstch_field::is_required},
            {"field:optional?", &mstch_field::is_optional},
            {"field:optInReqOut?", &mstch_field::is_optInReqOut},
        });
  }
  bool has_annotation(std::string const& name) {
    return field_->annotations_.count(name);
  }
  std::string get_annotation(std::string const& name) {
    if (has_annotation(name)) {
      return field_->annotations_.at(name);
    }
    return std::string();
  }
  mstch::node name() {
    return field_->get_name();
  }
  mstch::node key() {
    return std::to_string(field_->get_key());
  }
  mstch::node value();
  mstch::node type();
  mstch::node next_field_key() {
    return std::to_string(field_->get_next()->get_key());
  }
  mstch::node next_field_type();
  mstch::node index() {
    return std::to_string(index_);
  }
  mstch::node index_plus_one() {
    return std::to_string(index_ + 1);
  }
  mstch::node is_required() {
    return field_->get_req() == t_field::e_req::T_REQUIRED;
  }
  mstch::node is_optional() {
    return field_->get_req() == t_field::e_req::T_OPTIONAL;
  }
  mstch::node is_optInReqOut() {
    return field_->get_req() == t_field::e_req::T_OPT_IN_REQ_OUT;
  }

 protected:
  t_field const* field_;
  int32_t index_;
};

class mstch_struct : public mstch_base {
 public:
  mstch_struct(
      t_struct const* strct,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos)
      : mstch_base(generators, cache, pos), strct_(strct) {
    register_methods(
        this,
        {
            {"struct:name", &mstch_struct::name},
            {"struct:fields?", &mstch_struct::has_fields},
            {"struct:fields", &mstch_struct::fields},
            {"struct:exception?", &mstch_struct::is_exception},
            {"struct:union?", &mstch_struct::is_union},
            {"struct:plain?", &mstch_struct::is_plain},
        });
  }
  mstch::node name() {
    return strct_->get_name();
  }
  mstch::node has_fields() {
    return !strct_->get_members().empty();
  }
  mstch::node fields();
  mstch::node is_exception() {
    return strct_->is_xception();
  }
  mstch::node is_union() {
    return strct_->is_union();
  }
  mstch::node is_plain() {
    return !strct_->is_xception() && !strct_->is_union();
  }

 protected:
  t_struct const* strct_;
};

class mstch_function : public mstch_base {
 public:
  mstch_function(
      t_function const* function,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos)
      : mstch_base(generators, cache, pos), function_(function) {
    register_methods(
        this,
        {
            {"function:name", &mstch_function::name},
            {"function:oneway?", &mstch_function::oneway},
            {"function:returnType", &mstch_function::return_type},
            {"function:takenStreamType", &mstch_function::taken_stream_type},
            {"function:exceptions", &mstch_function::exceptions},
            {"function:stream_exceptions", &mstch_function::stream_exceptions},
            {"function:exceptions?", &mstch_function::has_exceptions},
            {"function:stream_exceptions?",
             &mstch_function::has_streamexceptions},
            {"function:args", &mstch_function::arg_list},
            {"function:comma", &mstch_function::has_args},
            {"function:eb", &mstch_function::event_based},
            {"function:priority", &mstch_function::priority},
            {"function:args_without_streams",
             &mstch_function::arg_list_without_streams},
            {"function:any_streams?", &mstch_function::any_streams},
            {"function:returns_stream?", &mstch_function::returns_stream},
            {"function:takes_stream?", &mstch_function::takes_stream},
        });
  }
  mstch::node taken_stream_type();

  mstch::node name() {
    return function_->get_name();
  }
  mstch::node oneway() {
    return function_->is_oneway();
  }
  mstch::node has_exceptions() {
    return !function_->get_xceptions()->get_members().empty();
  }
  mstch::node has_streamexceptions() {
    return !function_->get_stream_xceptions()->get_members().empty();
  }
  mstch::node has_args() {
    if (!function_->get_arglist()->get_members().empty()) {
      return std::string(", ");
    }
    return std::string();
  }
  mstch::node event_based() {
    auto const* strct = function_->get_annotations();
    if (strct && strct->annotations_.count("thread") &&
        strct->annotations_.at("thread") == "eb") {
      return true;
    }
    if (cache_->parsed_options_.count("process_in_event_base") != 0) {
      return true;
    }
    return false;
  }
  mstch::node priority() {
    auto const* strct = function_->get_annotations();
    if (strct && strct->annotations_.count("priority")) {
      return strct->annotations_.at("priority");
    }
    return std::string("NORMAL");
  }

  mstch::node return_type();
  mstch::node exceptions();
  mstch::node stream_exceptions();
  mstch::node arg_list();
  mstch::node arg_list_without_streams();
  mstch::node any_streams();
  mstch::node returns_stream();
  mstch::node takes_stream();

 protected:
  t_function const* function_;
};

class mstch_service : public mstch_base {
 public:
  mstch_service(
      t_service const* service,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos)
      : mstch_base(generators, cache, pos), service_(service) {
    register_methods(
        this,
        {
            {"service:name", &mstch_service::name},
            {"service:functions", &mstch_service::functions},
            {"service:functions?", &mstch_service::has_functions},
            {"service:extends", &mstch_service::extends},
            {"service:extends?", &mstch_service::has_extends},
            {"service:any_streams?", &mstch_service::any_streams},
        });
  }

  virtual std::string get_service_namespace(t_program const*) {
    return "";
  }

  mstch::node name() {
    return service_->get_name();
  }
  mstch::node has_functions() {
    return !service_->get_functions().empty();
  }
  mstch::node has_extends() {
    return service_->get_extends() != nullptr;
  }
  mstch::node functions();
  mstch::node extends();

  mstch::node any_streams() {
    auto& funcs = service_->get_functions();
    return std::any_of(funcs.cbegin(), funcs.cend(), [](auto const& func) {
      return func->any_streams();
    });
  }

 protected:
  t_service const* service_;
};

class mstch_typedef : public mstch_base {
 public:
  mstch_typedef(
      t_typedef const* typedf,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos)
      : mstch_base(generators, cache, pos), typedf_(typedf) {
    register_methods(
        this,
        {
            {"typedef:type", &mstch_typedef::type},
            {"typedef:is_same_type", &mstch_typedef::is_same_type},
            {"typedef:symbolic", &mstch_typedef::symbolic},
        });
  }
  mstch::node type();
  mstch::node symbolic() {
    return typedf_->get_symbolic();
  }
  mstch::node is_same_type() {
    return typedf_->get_name() == typedf_->get_type()->get_name();
  }

 protected:
  t_typedef const* typedf_;
};

class mstch_const : public mstch_base {
 public:
  mstch_const(
      t_const const* cnst,
      t_const const* current_const,
      t_type const* expected_type,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos,
      int32_t index)
      : mstch_base(generators, cache, pos),
        cnst_(cnst),
        current_const_(current_const),
        expected_type_(expected_type),
        index_(index) {
    register_methods(
        this,
        {
            {"constant:name", &mstch_const::name},
            {"constant:index", &mstch_const::index},
            {"constant:type", &mstch_const::type},
            {"constant:value", &mstch_const::value},
            {"constant:program", &mstch_const::program},
        });
  }
  mstch::node name() {
    return cnst_->get_name();
  }
  mstch::node index() {
    return index_;
  }
  mstch::node type();
  mstch::node value();
  mstch::node program();

 protected:
  t_const const* cnst_;
  t_const const* current_const_;
  t_type const* expected_type_;
  int32_t index_;
};

class mstch_program : public mstch_base {
 public:
  mstch_program(
      t_program const* program,
      std::shared_ptr<mstch_generators const> generators,
      std::shared_ptr<mstch_cache> cache,
      ELEMENT_POSITION pos)
      : mstch_base(generators, cache, pos), program_(program) {
    register_methods(
        this,
        {
            {"program:name", &mstch_program::name},
            {"program:includePrefix", &mstch_program::include_prefix},
            {"program:structs", &mstch_program::structs},
            {"program:enums", &mstch_program::enums},
            {"program:services", &mstch_program::services},
            {"program:typedefs", &mstch_program::typedefs},
            {"program:constants", &mstch_program::constants},
            {"program:enums?", &mstch_program::has_enums},
            {"program:structs?", &mstch_program::has_structs},
            {"program:services?", &mstch_program::has_services},
            {"program:typedefs?", &mstch_program::has_typedefs},
            {"program:constants?", &mstch_program::has_constants},
        });
  }

  virtual std::string get_program_namespace(t_program const*) {
    return "";
  }

  mstch::node name() {
    return program_->get_name();
  }
  mstch::node include_prefix() {
    return program_->get_include_prefix();
  }
  mstch::node has_enums() {
    return !program_->get_enums().empty();
  }
  mstch::node has_structs() {
    return !program_->get_structs().empty() ||
        !program_->get_xceptions().empty();
  }
  mstch::node has_services() {
    return !program_->get_services().empty();
  }
  mstch::node has_typedefs() {
    return !program_->get_typedefs().empty();
  }
  mstch::node has_constants() {
    return !program_->get_consts().empty();
  }

  mstch::node structs();
  mstch::node enums();
  mstch::node services();
  mstch::node typedefs();
  mstch::node constants();

 protected:
  t_program const* program_;
};
