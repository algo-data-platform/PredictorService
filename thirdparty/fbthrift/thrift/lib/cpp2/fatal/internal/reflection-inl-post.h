/*
 * Copyright 2016-present Facebook, Inc.
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
#ifndef THRIFT_FATAL_REFLECTION_INL_POST_H_
#define THRIFT_FATAL_REFLECTION_INL_POST_H_ 1

#if !defined THRIFT_FATAL_REFLECTION_H_
#error "This file must be included from reflection.h"
#endif

namespace folly {
template <class Value>
class Optional;
}

namespace apache {
namespace thrift {
namespace detail {
namespace reflection_impl {

template <typename T>
struct is_optional : std::false_type {};
template <typename T>
struct is_optional<folly::Optional<T>> : std::true_type {};

template <typename, typename T>
struct has_isset_ : std::false_type {};
template <typename T>
struct has_isset_<folly::void_t<decltype(std::declval<T>().__isset)>, T>
    : std::true_type {};
template <typename T>
using has_isset = has_isset_<void, T>;

template <typename Owner, typename Getter, bool HasIsSet>
struct isset {
  template <int I>
  struct kind {};

  template <typename T>
  using kind_of = std::conditional_t<
      is_optional<
          std::decay_t<decltype(Getter::ref(std::declval<T const&>()))>>::value,
      kind<0>,
      std::conditional_t<HasIsSet && has_isset<T>::value, kind<1>, kind<2>>>;

  template <typename T>
  static constexpr bool check(kind<0>, T const& owner) {
    return Getter::ref(owner).has_value();
  }
  template <typename T>
  static constexpr bool check(kind<1>, T const& owner) {
    return Getter::copy(owner.__isset);
  }
  template <typename T>
  static constexpr bool check(kind<2>, T const&) {
    return true;
  }
  template <typename T>
  static constexpr bool check(T const& owner) {
    return check(kind_of<T>{}, owner);
  }

  template <typename T>
  static constexpr bool mark(kind<0>, T& owner, bool set) {
    auto& field = Getter::ref(owner);
    if (set && !field.has_value()) {
      field.emplace();
    } else if (!set && field.has_value()) {
      field.clear();
    }
    return set;
  }
  template <typename T>
  static constexpr bool mark(kind<1>, T& owner, bool set) {
    return Getter::ref(owner.__isset) = set;
  }
  template <typename T>
  static constexpr bool mark(kind<2>, T&, bool set) {
    return set;
  }
  template <typename T>
  static constexpr bool mark(T& owner, bool set) {
    return mark(kind_of<T>{}, owner, set);
  }
};

} // namespace reflection_impl

template <typename...>
struct chained_data_member_getter;

template <typename OuterGetter, typename... Getters>
struct chained_data_member_getter<OuterGetter, Getters...>
    : fatal::chained_data_member_getter<OuterGetter, Getters...> {
  using head = OuterGetter;
  using tail = chained_data_member_getter<Getters...>;
};
template <>
struct chained_data_member_getter<> : fatal::chained_data_member_getter<> {
  using head = void;
  using tail = void;
};

template <typename Impl>
struct reflection_indirection_getter {
  template <typename T>
  using type = decltype(Impl::val(std::declval<T&&>()));

  template <typename T>
  using reference = decltype(Impl::ref(std::declval<T&&>()));

  template <typename T>
  static inline auto&& ref(T&& arg) {
    return Impl::ref(std::forward<T>(arg));
  }
};

template <typename Module, typename Annotations, legacy_type_id_t LegacyTypeId>
struct type_common_metadata_impl {
  using module = Module;
  using annotations = Annotations;
  using legacy_id = std::integral_constant<legacy_type_id_t, LegacyTypeId>;
};

template <
    typename T,
    bool = fatal::is_complete<thrift_list_traits<T>>::value,
    bool = fatal::is_complete<thrift_map_traits<T>>::value,
    bool = fatal::is_complete<thrift_set_traits<T>>::value>
struct reflect_container_type_class_impl {
  using type = type_class::unknown;
};

template <typename T>
struct reflect_container_type_class_impl<T, true, false, false> {
  using type = type_class::list<
      reflect_type_class<typename thrift_list_traits<T>::value_type>>;
};

template <typename T>
struct reflect_container_type_class_impl<T, false, true, false> {
  using type = type_class::map<
      reflect_type_class<typename thrift_map_traits<T>::key_type>,
      reflect_type_class<typename thrift_map_traits<T>::mapped_type>>;
};

template <typename T>
struct reflect_container_type_class_impl<T, false, false, true> {
  using type = type_class::set<
      reflect_type_class<typename thrift_set_traits<T>::value_type>>;
};

template <typename T>
struct reflect_type_class_impl {
  using type = fatal::conditional<
      is_reflectable_enum<T>::value,
      type_class::enumeration,
      fatal::conditional<
          is_reflectable_union<T>::value,
          type_class::variant,
          fatal::conditional<
              is_reflectable_struct<T>::value,
              type_class::structure,
              fatal::conditional<
                  std::is_floating_point<T>::value,
                  type_class::floating_point,
                  fatal::conditional<
                      std::is_integral<T>::value,
                      type_class::integral,
                      fatal::conditional<
                          std::is_same<void, T>::value,
                          type_class::nothing,
                          fatal::conditional<
                              fatal::is_complete<
                                  thrift_string_traits<T>>::value,
                              type_class::string,
                              typename reflect_container_type_class_impl<
                                  T>::type>>>>>>>;
};

template <typename T, bool IsTry>
struct reflect_module_tag_selector<type_class::enumeration, T, IsTry> {
  using type = typename fatal::enum_traits<T>::metadata::module;
};

template <typename T, bool IsTry>
struct reflect_module_tag_selector<type_class::variant, T, IsTry> {
  using type = typename fatal::variant_traits<T>::metadata::module;
};

template <typename T, bool IsTry>
struct reflect_module_tag_selector<type_class::structure, T, IsTry> {
  using type = typename reflect_struct<T>::module;
};

} // namespace detail

template <>
struct reflected_annotations<void> {
  struct keys {};
  struct values {};
  using map = fatal::list<>;
};

} // namespace thrift
} // namespace apache

#endif // THRIFT_FATAL_REFLECTION_INL_POST_H_
