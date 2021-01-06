/*
 * Copyright 2014 Facebook, Inc.
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
namespace apache {
namespace thrift {
namespace frozen {
namespace detail {

/**
 * Layout specialization for Optional<T>, which is used in the codegen for
 * optional fields. Just a boolean and a value.
 */
template <class T>
struct OptionalLayout : public LayoutBase {
  typedef LayoutBase Base;
  Field<bool> issetField;
  Field<T> valueField;

  OptionalLayout()
      : LayoutBase(typeid(T)), issetField(1, "isset"), valueField(2, "value") {}

  FieldPosition maximize() {
    FieldPosition pos = startFieldPosition();
    FROZEN_MAXIMIZE_FIELD(isset);
    FROZEN_MAXIMIZE_FIELD(value);
    return pos;
  }

  FieldPosition
  layout(LayoutRoot& root, const folly::Optional<T>& o, LayoutPosition self) {
    FieldPosition pos = startFieldPosition();
    pos = root.layoutField(self, pos, issetField, o.hasValue());
    if (o) {
      pos = root.layoutField(self, pos, valueField, o.value());
    }
    return pos;
  }

  FieldPosition layout(LayoutRoot& root, const T& o, LayoutPosition self) {
    FieldPosition pos = startFieldPosition();
    pos = root.layoutField(self, pos, issetField, true);
    pos = root.layoutField(self, pos, valueField, o);
    return pos;
  }

  void freeze(
      FreezeRoot& root,
      const folly::Optional<T>& o,
      FreezePosition self) const {
    root.freezeField(self, issetField, o.hasValue());
    if (o) {
      root.freezeField(self, valueField, o.value());
    }
  }

  void freeze(FreezeRoot& root, const T& o, FreezePosition self) const {
    root.freezeField(self, issetField, true);
    root.freezeField(self, valueField, o);
  }

  void thaw(ViewPosition self, folly::Optional<T>& out) const {
    bool set;
    thawField(self, issetField, set);
    if (set) {
      out.emplace();
      thawField(self, valueField, out.value());
    }
  }

  typedef folly::Optional<typename Layout<T>::View> View;

  View view(ViewPosition self) const {
    View v;
    bool set;
    thawField(self, issetField, set);
    if (set) {
      v.assign(valueField.layout.view(self(valueField.pos)));
    }
    return v;
  }

  void print(std::ostream& os, int level) const final {
    LayoutBase::print(os, level);
    os << "optional " << folly::demangle(type.name());
    issetField.print(os, level + 1);
    valueField.print(os, level + 1);
  }

  void clear() final {
    issetField.clear();
    valueField.clear();
  }

  FROZEN_SAVE_INLINE(FROZEN_SAVE_FIELD(isset) FROZEN_SAVE_FIELD(value))

  FROZEN_LOAD_INLINE(FROZEN_LOAD_FIELD(isset, 1) FROZEN_LOAD_FIELD(value, 2))
};
} // namespace detail

template <class T>
struct Layout<folly::Optional<T>> : public detail::OptionalLayout<T> {};

} // namespace frozen
} // namespace thrift
} // namespace apache
