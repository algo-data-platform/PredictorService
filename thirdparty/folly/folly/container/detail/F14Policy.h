/*
 * Copyright 2017-present Facebook, Inc.
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

#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#include <folly/Memory.h>
#include <folly/Unit.h>
#include <folly/container/detail/F14Table.h>
#include <folly/hash/Hash.h>
#include <folly/lang/Align.h>
#include <folly/lang/SafeAssert.h>
#include <folly/memory/Malloc.h>

#if FOLLY_F14_VECTOR_INTRINSICS_AVAILABLE

namespace folly {
namespace f14 {
namespace detail {

template <typename Ptr>
using NonConstPtr = typename std::pointer_traits<Ptr>::template rebind<
    std::remove_const_t<typename std::pointer_traits<Ptr>::element_type>>;

template <typename KeyType, typename MappedType>
using MapValueType = std::pair<KeyType const, MappedType>;

template <typename KeyType, typename MappedTypeOrVoid>
using SetOrMapValueType = std::conditional_t<
    std::is_same<MappedTypeOrVoid, void>::value,
    KeyType,
    MapValueType<KeyType, MappedTypeOrVoid>>;

// Policy provides the functionality of hasher, key_equal, and
// allocator_type.  In addition, it can add indirection to the values
// contained in the base table by defining a non-trivial value() method.
//
// To facilitate stateful implementations it is guaranteed that there
// will be a 1:1 relationship between BaseTable and Policy instance:
// policies will only be copied when their owning table is copied, and
// they will only be moved when their owning table is moved.
//
// Key equality will have the user-supplied search key as its first
// argument and the table contents as its second.  Heterogeneous lookup
// should be handled on the first argument.
//
// Item is the data stored inline in the hash table's chunks.  The policy
// controls how this is mapped to the corresponding Value.
//
// The policies defined in this file work for either set or map types.
// Most of the functionality is identical. A few methods detect the
// collection type by checking to see if MappedType is void, and then use
// SFINAE to select the appropriate implementation.
template <
    typename KeyType,
    typename MappedTypeOrVoid,
    typename HasherOrVoid,
    typename KeyEqualOrVoid,
    typename AllocOrVoid,
    typename ItemType>
struct BasePolicy
    : std::tuple<
          Defaulted<HasherOrVoid, DefaultHasher<KeyType>>,
          Defaulted<KeyEqualOrVoid, DefaultKeyEqual<KeyType>>,
          Defaulted<
              AllocOrVoid,
              DefaultAlloc<SetOrMapValueType<KeyType, MappedTypeOrVoid>>>> {
  //////// user-supplied types

  using Key = KeyType;
  using Mapped = MappedTypeOrVoid;
  using Value = SetOrMapValueType<Key, Mapped>;
  using Item = ItemType;
  using Hasher = Defaulted<HasherOrVoid, DefaultHasher<Key>>;
  using KeyEqual = Defaulted<KeyEqualOrVoid, DefaultKeyEqual<Key>>;
  using Alloc = Defaulted<AllocOrVoid, DefaultAlloc<Value>>;
  using AllocTraits = std::allocator_traits<Alloc>;

  using ByteAlloc = typename AllocTraits::template rebind_alloc<uint8_t>;
  using ByteAllocTraits = typename std::allocator_traits<ByteAlloc>;
  using BytePtr = typename ByteAllocTraits::pointer;

  //////// info about user-supplied types

  static_assert(
      std::is_same<typename AllocTraits::value_type, Value>::value,
      "wrong allocator value_type");

 private:
  // emulate c++17's std::allocator_traits<A>::is_always_equal

  template <typename A, typename = void>
  struct AllocIsAlwaysEqual : std::is_empty<A> {};

  template <typename A>
  struct AllocIsAlwaysEqual<A, typename A::is_always_equal>
      : A::is_always_equal {};

  // emulate c++17 has std::is_nothrow_swappable
  template <typename T>
  static constexpr bool isNothrowSwap() {
    using std::swap;
    return noexcept(swap(std::declval<T&>(), std::declval<T&>()));
  }

 public:
  static constexpr bool kAllocIsAlwaysEqual = AllocIsAlwaysEqual<Alloc>::value;

  static constexpr bool kDefaultConstructIsNoexcept =
      std::is_nothrow_default_constructible<Hasher>::value &&
      std::is_nothrow_default_constructible<KeyEqual>::value &&
      std::is_nothrow_default_constructible<Alloc>::value;

  static constexpr bool kSwapIsNoexcept = kAllocIsAlwaysEqual &&
      isNothrowSwap<Hasher>() && isNothrowSwap<KeyEqual>();

  static constexpr bool isAvalanchingHasher() {
    return IsAvalanchingHasher<Hasher, Key>::value;
  }

  //////// internal types and constants

  using InternalSizeType = std::size_t;

  using Super = std::tuple<Hasher, KeyEqual, Alloc>;

  // if false, F14Table will be smaller but F14Table::begin() won't work
  static constexpr bool kEnableItemIteration = true;

  using Chunk = F14Chunk<Item>;
  using ChunkPtr = typename std::pointer_traits<
      typename AllocTraits::pointer>::template rebind<Chunk>;
  using ItemIter = F14ItemIter<ChunkPtr>;

  static constexpr bool kIsMap = !std::is_same<Key, Value>::value;
  static_assert(
      kIsMap == !std::is_void<MappedTypeOrVoid>::value,
      "Assumption for the kIsMap check violated.");

  using MappedOrBool = std::conditional_t<kIsMap, Mapped, bool>;

  //////// methods

  BasePolicy(Hasher const& hasher, KeyEqual const& keyEqual, Alloc const& alloc)
      : Super{hasher, keyEqual, alloc} {}

  BasePolicy(BasePolicy const& rhs)
      : Super{rhs.hasher(),
              rhs.keyEqual(),
              AllocTraits::select_on_container_copy_construction(rhs.alloc())} {
  }

  BasePolicy(BasePolicy const& rhs, Alloc const& alloc)
      : Super{rhs.hasher(), rhs.keyEqual(), alloc} {}

  BasePolicy(BasePolicy&& rhs) noexcept
      : Super{std::move(rhs.hasher()),
              std::move(rhs.keyEqual()),
              std::move(rhs.alloc())} {}

  BasePolicy(BasePolicy&& rhs, Alloc const& alloc) noexcept
      : Super{std::move(rhs.hasher()), std::move(rhs.keyEqual()), alloc} {}

  BasePolicy& operator=(BasePolicy const& rhs) {
    hasher() = rhs.hasher();
    keyEqual() = rhs.keyEqual();
    if (AllocTraits::propagate_on_container_copy_assignment::value) {
      alloc() = rhs.alloc();
    }
    return *this;
  }

  BasePolicy& operator=(BasePolicy&& rhs) noexcept {
    hasher() = std::move(rhs.hasher());
    keyEqual() = std::move(rhs.keyEqual());
    if (AllocTraits::propagate_on_container_move_assignment::value) {
      alloc() = std::move(rhs.alloc());
    }
    return *this;
  }

  void swapBasePolicy(BasePolicy& rhs) {
    using std::swap;
    swap(hasher(), rhs.hasher());
    swap(keyEqual(), rhs.keyEqual());
    if (AllocTraits::propagate_on_container_swap::value) {
      swap(alloc(), rhs.alloc());
    }
  }

  Hasher& hasher() {
    return std::get<0>(*this);
  }
  Hasher const& hasher() const {
    return std::get<0>(*this);
  }
  KeyEqual& keyEqual() {
    return std::get<1>(*this);
  }
  KeyEqual const& keyEqual() const {
    return std::get<1>(*this);
  }
  Alloc& alloc() {
    return std::get<2>(*this);
  }
  Alloc const& alloc() const {
    return std::get<2>(*this);
  }

  template <typename K>
  std::size_t computeKeyHash(K const& key) const {
    static_assert(
        isAvalanchingHasher() == IsAvalanchingHasher<Hasher, K>::value, "");
    static_assert(
        !isAvalanchingHasher() ||
            sizeof(decltype(hasher()(key))) >= sizeof(std::size_t),
        "hasher is not avalanching if it doesn't return enough bits");
    return hasher()(key);
  }

  Key const& keyForValue(Key const& v) const {
    return v;
  }
  Key const& keyForValue(std::pair<Key const, MappedOrBool> const& p) const {
    return p.first;
  }
  Key const& keyForValue(std::pair<Key&&, MappedOrBool&&> const& p) const {
    return p.first;
  }

  // map's choice of pair<K const, T> as value_type is unfortunate,
  // because it means we either need a proxy iterator, a pointless key
  // copy when moving items during rehash, or some sort of UB hack.
  //
  // This code implements the hack.  Use moveValue(v) instead of
  // std::move(v) as the source of a move construction.  enable_if_t is
  // used so that this works for maps while being a no-op for sets.
  template <typename Dummy = int>
  static std::pair<Key&&, MappedOrBool&&> moveValue(
      std::pair<Key const, MappedOrBool>& value,
      std::enable_if_t<kIsMap, Dummy> = 0) {
    return {std::move(const_cast<Key&>(value.first)), std::move(value.second)};
  }

  template <typename Dummy = int>
  static Value&& moveValue(Value& value, std::enable_if_t<!kIsMap, Dummy> = 0) {
    return std::move(value);
  }

  template <typename P>
  bool
  beforeBuild(std::size_t /*size*/, std::size_t /*capacity*/, P&& /*rhs*/) {
    return false;
  }

  template <typename P>
  void afterBuild(
      bool /*undoState*/,
      bool /*success*/,
      std::size_t /*size*/,
      std::size_t /*capacity*/,
      P&& /*rhs*/) {}

  std::size_t alignedAllocSize(std::size_t n) const {
    if (kRequiredVectorAlignment <= alignof(max_align_t) ||
        std::is_same<ByteAlloc, std::allocator<uint8_t>>::value) {
      return n;
    } else {
      return n + kRequiredVectorAlignment;
    }
  }

  bool beforeRehash(
      std::size_t /*size*/,
      std::size_t /*oldCapacity*/,
      std::size_t /*newCapacity*/,
      std::size_t chunkAllocSize,
      BytePtr& outChunkAllocation) {
    outChunkAllocation =
        allocateOverAligned<ByteAlloc, kRequiredVectorAlignment>(
            ByteAlloc{alloc()}, chunkAllocSize);
    return false;
  }

  void afterRehash(
      bool /*undoState*/,
      bool /*success*/,
      std::size_t /*size*/,
      std::size_t /*oldCapacity*/,
      std::size_t /*newCapacity*/,
      BytePtr chunkAllocation,
      std::size_t chunkAllocSize) {
    // on success, this will be the old allocation, on failure the new one
    if (chunkAllocation != nullptr) {
      deallocateOverAligned<ByteAlloc, kRequiredVectorAlignment>(
          ByteAlloc{alloc()}, chunkAllocation, chunkAllocSize);
    }
  }

  void beforeClear(std::size_t /*size*/, std::size_t /*capacity*/) {}

  void afterClear(std::size_t /*size*/, std::size_t /*capacity*/) {}

  void beforeReset(std::size_t /*size*/, std::size_t /*capacity*/) {}

  void afterReset(
      std::size_t /*size*/,
      std::size_t /*capacity*/,
      BytePtr chunkAllocation,
      std::size_t chunkAllocSize) {
    deallocateOverAligned<ByteAlloc, kRequiredVectorAlignment>(
        ByteAlloc{alloc()}, chunkAllocation, chunkAllocSize);
  }

  void prefetchValue(Item const&) const {
    // Subclass should disable with prefetchBeforeRehash(),
    // prefetchBeforeCopy(), and prefetchBeforeDestroy().  if they don't
    // override this method, because neither gcc nor clang can figure
    // out that DenseMaskIter with an empty body can be elided.
    FOLLY_SAFE_DCHECK(false, "should be disabled");
  }
};

// BaseIter is a convenience for concrete set and map implementations
template <typename ValuePtr, typename Item>
class BaseIter : public std::iterator<
                     std::forward_iterator_tag,
                     std::remove_const_t<
                         typename std::pointer_traits<ValuePtr>::element_type>,
                     std::ptrdiff_t,
                     ValuePtr,
                     decltype(*std::declval<ValuePtr>())> {
 protected:
  using Chunk = F14Chunk<Item>;
  using ChunkPtr =
      typename std::pointer_traits<ValuePtr>::template rebind<Chunk>;
  using ItemIter = F14ItemIter<ChunkPtr>;

  using ValueConstPtr = typename std::pointer_traits<ValuePtr>::template rebind<
      std::add_const_t<typename std::pointer_traits<ValuePtr>::element_type>>;
};

//////// ValueContainer

template <
    typename Key,
    typename Mapped,
    typename HasherOrVoid,
    typename KeyEqualOrVoid,
    typename AllocOrVoid>
class ValueContainerPolicy;

template <typename ValuePtr>
using ValueContainerIteratorBase = BaseIter<
    ValuePtr,
    std::remove_const_t<typename std::pointer_traits<ValuePtr>::element_type>>;

template <typename ValuePtr>
class ValueContainerIterator : public ValueContainerIteratorBase<ValuePtr> {
  using Super = ValueContainerIteratorBase<ValuePtr>;
  using typename Super::ItemIter;
  using typename Super::ValueConstPtr;

 public:
  using typename Super::pointer;
  using typename Super::reference;
  using typename Super::value_type;

  ValueContainerIterator() = default;
  ValueContainerIterator(ValueContainerIterator const&) = default;
  ValueContainerIterator(ValueContainerIterator&&) = default;
  ValueContainerIterator& operator=(ValueContainerIterator const&) = default;
  ValueContainerIterator& operator=(ValueContainerIterator&&) = default;
  ~ValueContainerIterator() = default;

  /*implicit*/ operator ValueContainerIterator<ValueConstPtr>() const {
    return ValueContainerIterator<ValueConstPtr>{underlying_};
  }

  reference operator*() const {
    return underlying_.item();
  }

  pointer operator->() const {
    return std::pointer_traits<pointer>::pointer_to(**this);
  }

  ValueContainerIterator& operator++() {
    underlying_.advance();
    return *this;
  }

  ValueContainerIterator operator++(int) {
    auto cur = *this;
    ++*this;
    return cur;
  }

  bool operator==(ValueContainerIterator<ValueConstPtr> const& rhs) const {
    return underlying_ == rhs.underlying_;
  }
  bool operator!=(ValueContainerIterator<ValueConstPtr> const& rhs) const {
    return !(*this == rhs);
  }

 private:
  ItemIter underlying_;

  explicit ValueContainerIterator(ItemIter const& underlying)
      : underlying_{underlying} {}

  template <typename K, typename M, typename H, typename E, typename A>
  friend class ValueContainerPolicy;

  template <typename P>
  friend class ValueContainerIterator;
};

template <
    typename Key,
    typename MappedTypeOrVoid,
    typename HasherOrVoid,
    typename KeyEqualOrVoid,
    typename AllocOrVoid>
class ValueContainerPolicy : public BasePolicy<
                                 Key,
                                 MappedTypeOrVoid,
                                 HasherOrVoid,
                                 KeyEqualOrVoid,
                                 AllocOrVoid,
                                 SetOrMapValueType<Key, MappedTypeOrVoid>> {
 public:
  using Super = BasePolicy<
      Key,
      MappedTypeOrVoid,
      HasherOrVoid,
      KeyEqualOrVoid,
      AllocOrVoid,
      SetOrMapValueType<Key, MappedTypeOrVoid>>;
  using typename Super::Alloc;
  using typename Super::AllocTraits;
  using typename Super::Item;
  using typename Super::ItemIter;
  using typename Super::Value;

 private:
  using typename Super::ByteAlloc;

  using Super::kIsMap;

 public:
  using ConstIter = ValueContainerIterator<typename AllocTraits::const_pointer>;
  using Iter = std::conditional_t<
      kIsMap,
      ValueContainerIterator<typename AllocTraits::pointer>,
      ConstIter>;

  //////// F14Table policy

  static constexpr bool prefetchBeforeRehash() {
    return false;
  }

  static constexpr bool prefetchBeforeCopy() {
    return false;
  }

  static constexpr bool prefetchBeforeDestroy() {
    return false;
  }

  static constexpr bool destroyItemOnClear() {
    return !std::is_trivially_destructible<Item>::value ||
        !folly::AllocatorHasDefaultObjectDestroy<Alloc, Item>::value;
  }

  // inherit constructors
  using Super::Super;

  void swapPolicy(ValueContainerPolicy& rhs) {
    this->swapBasePolicy(rhs);
  }

  using Super::keyForValue;
  static_assert(
      std::is_same<Item, Value>::value,
      "Item and Value should be the same type for ValueContainerPolicy.");

  std::size_t computeItemHash(Item const& item) const {
    return this->computeKeyHash(keyForValue(item));
  }

  template <typename K>
  bool keyMatchesItem(K const& key, Item const& item) const {
    return this->keyEqual()(key, keyForValue(item));
  }

  Value const& buildArgForItem(Item const& item) const& {
    return item;
  }

  // buildArgForItem(Item&)&& is used when moving between unequal allocators
  decltype(auto) buildArgForItem(Item& item) && {
    return Super::moveValue(item);
  }

  Value&& valueAtItemForExtract(Item& item) {
    return std::move(item);
  }

  template <typename... Args>
  void
  constructValueAtItem(std::size_t /*size*/, Item* itemAddr, Args&&... args) {
    Alloc& a = this->alloc();
    folly::assume(itemAddr != nullptr);
    AllocTraits::construct(a, itemAddr, std::forward<Args>(args)...);
  }

  template <typename T>
  std::enable_if_t<std::is_nothrow_move_constructible<T>::value>
  complainUnlessNothrowMove() {}

  template <typename T>
  [[deprecated(
      "use F14NodeMap/Set or mark key and mapped type move constructor nothrow")]] std::
      enable_if_t<!std::is_nothrow_move_constructible<T>::value>
      complainUnlessNothrowMove() {}

  void moveItemDuringRehash(Item* itemAddr, Item& src) {
    complainUnlessNothrowMove<Key>();
    complainUnlessNothrowMove<lift_unit_t<MappedTypeOrVoid>>();

    constructValueAtItem(0, itemAddr, Super::moveValue(src));
    if (destroyItemOnClear()) {
      if (kIsMap) {
        // Laundering in the standard is only described as a solution
        // for changes to const fields due to the creation of a new
        // object lifetime (destroy and then placement new in the same
        // location), but it seems highly likely that it will also cause
        // the compiler to drop such assumptions that are violated due
        // to our UB const_cast in moveValue.
        destroyItem(*folly::launder(std::addressof(src)));
      } else {
        destroyItem(src);
      }
    }
  }

  void destroyItem(Item& item) {
    Alloc& a = this->alloc();
    AllocTraits::destroy(a, std::addressof(item));
  }

  template <typename V>
  void visitPolicyAllocationClasses(
      std::size_t chunkAllocSize,
      std::size_t /*size*/,
      std::size_t /*capacity*/,
      V&& visitor) const {
    if (chunkAllocSize > 0) {
      visitor(
          allocationBytesForOverAligned<ByteAlloc, kRequiredVectorAlignment>(
              chunkAllocSize),
          1);
    }
  }

  //////// F14BasicMap/Set policy

  Iter makeIter(ItemIter const& underlying) const {
    return Iter{underlying};
  }
  ConstIter makeConstIter(ItemIter const& underlying) const {
    return ConstIter{underlying};
  }
  ItemIter const& unwrapIter(ConstIter const& iter) const {
    return iter.underlying_;
  }
};

//////// NodeContainer

template <
    typename Key,
    typename Mapped,
    typename HasherOrVoid,
    typename KeyEqualOrVoid,
    typename AllocOrVoid>
class NodeContainerPolicy;

template <typename ValuePtr>
class NodeContainerIterator : public BaseIter<ValuePtr, NonConstPtr<ValuePtr>> {
  using Super = BaseIter<ValuePtr, NonConstPtr<ValuePtr>>;
  using typename Super::ItemIter;
  using typename Super::ValueConstPtr;

 public:
  using typename Super::pointer;
  using typename Super::reference;
  using typename Super::value_type;

  NodeContainerIterator() = default;
  NodeContainerIterator(NodeContainerIterator const&) = default;
  NodeContainerIterator(NodeContainerIterator&&) = default;
  NodeContainerIterator& operator=(NodeContainerIterator const&) = default;
  NodeContainerIterator& operator=(NodeContainerIterator&&) = default;
  ~NodeContainerIterator() = default;

  /*implicit*/ operator NodeContainerIterator<ValueConstPtr>() const {
    return NodeContainerIterator<ValueConstPtr>{underlying_};
  }

  reference operator*() const {
    return *underlying_.item();
  }

  pointer operator->() const {
    return std::pointer_traits<pointer>::pointer_to(**this);
  }

  NodeContainerIterator& operator++() {
    underlying_.advance();
    return *this;
  }

  NodeContainerIterator operator++(int) {
    auto cur = *this;
    ++*this;
    return cur;
  }

  bool operator==(NodeContainerIterator<ValueConstPtr> const& rhs) const {
    return underlying_ == rhs.underlying_;
  }
  bool operator!=(NodeContainerIterator<ValueConstPtr> const& rhs) const {
    return !(*this == rhs);
  }

 private:
  ItemIter underlying_;

  explicit NodeContainerIterator(ItemIter const& underlying)
      : underlying_{underlying} {}

  template <typename K, typename M, typename H, typename E, typename A>
  friend class NodeContainerPolicy;

  template <typename P>
  friend class NodeContainerIterator;
};

template <
    typename Key,
    typename MappedTypeOrVoid,
    typename HasherOrVoid,
    typename KeyEqualOrVoid,
    typename AllocOrVoid>
class NodeContainerPolicy
    : public BasePolicy<
          Key,
          MappedTypeOrVoid,
          HasherOrVoid,
          KeyEqualOrVoid,
          AllocOrVoid,
          typename std::allocator_traits<Defaulted<
              AllocOrVoid,
              DefaultAlloc<std::conditional_t<
                  std::is_void<MappedTypeOrVoid>::value,
                  Key,
                  MapValueType<Key, MappedTypeOrVoid>>>>>::pointer> {
 public:
  using Super = BasePolicy<
      Key,
      MappedTypeOrVoid,
      HasherOrVoid,
      KeyEqualOrVoid,
      AllocOrVoid,
      typename std::allocator_traits<Defaulted<
          AllocOrVoid,
          DefaultAlloc<std::conditional_t<
              std::is_void<MappedTypeOrVoid>::value,
              Key,
              MapValueType<Key, MappedTypeOrVoid>>>>>::pointer>;
  using typename Super::Alloc;
  using typename Super::AllocTraits;
  using typename Super::Item;
  using typename Super::ItemIter;
  using typename Super::Value;

 private:
  using typename Super::ByteAlloc;

  using Super::kIsMap;

 public:
  using ConstIter = NodeContainerIterator<typename AllocTraits::const_pointer>;
  using Iter = std::conditional_t<
      kIsMap,
      NodeContainerIterator<typename AllocTraits::pointer>,
      ConstIter>;

  //////// F14Table policy

  static constexpr bool prefetchBeforeRehash() {
    return true;
  }

  static constexpr bool prefetchBeforeCopy() {
    return true;
  }

  static constexpr bool prefetchBeforeDestroy() {
    return !std::is_trivially_destructible<Value>::value;
  }

  static constexpr bool destroyItemOnClear() {
    return true;
  }

  // inherit constructors
  using Super::Super;

  void swapPolicy(NodeContainerPolicy& rhs) {
    this->swapBasePolicy(rhs);
  }

  using Super::keyForValue;

  std::size_t computeItemHash(Item const& item) const {
    return this->computeKeyHash(keyForValue(*item));
  }

  template <typename K>
  bool keyMatchesItem(K const& key, Item const& item) const {
    return this->keyEqual()(key, keyForValue(*item));
  }

  Value const& buildArgForItem(Item const& item) const& {
    return *item;
  }

  // buildArgForItem(Item&)&& is used when moving between unequal allocators
  decltype(auto) buildArgForItem(Item& item) && {
    return Super::moveValue(*item);
  }

  Value&& valueAtItemForExtract(Item& item) {
    return std::move(*item);
  }

  template <typename... Args>
  void
  constructValueAtItem(std::size_t /*size*/, Item* itemAddr, Args&&... args) {
    Alloc& a = this->alloc();
    folly::assume(itemAddr != nullptr);
    new (itemAddr) Item{AllocTraits::allocate(a, 1)};
    auto p = std::addressof(**itemAddr);
    folly::assume(p != nullptr);
    AllocTraits::construct(a, p, std::forward<Args>(args)...);
  }

  void moveItemDuringRehash(Item* itemAddr, Item& src) {
    // This is basically *itemAddr = src; src = nullptr, but allowing
    // for fancy pointers.
    folly::assume(itemAddr != nullptr);
    new (itemAddr) Item{std::move(src)};
    src = nullptr;
    src.~Item();
  }

  void prefetchValue(Item const& item) const {
    prefetchAddr(std::addressof(*item));
  }

  void destroyItem(Item& item) {
    if (item != nullptr) {
      Alloc& a = this->alloc();
      AllocTraits::destroy(a, std::addressof(*item));
      AllocTraits::deallocate(a, item, 1);
    }
    item.~Item();
  }

  template <typename V>
  void visitPolicyAllocationClasses(
      std::size_t chunkAllocSize,
      std::size_t size,
      std::size_t /*capacity*/,
      V&& visitor) const {
    if (chunkAllocSize > 0) {
      visitor(
          allocationBytesForOverAligned<ByteAlloc, kRequiredVectorAlignment>(
              chunkAllocSize),
          1);
    }
    if (size > 0) {
      visitor(sizeof(Value), size);
    }
  }

  //////// F14BasicMap/Set policy

  Iter makeIter(ItemIter const& underlying) const {
    return Iter{underlying};
  }
  ConstIter makeConstIter(ItemIter const& underlying) const {
    return Iter{underlying};
  }
  ItemIter const& unwrapIter(ConstIter const& iter) const {
    return iter.underlying_;
  }
};

//////// VectorContainer

template <
    typename Key,
    typename MappedTypeOrVoid,
    typename HasherOrVoid,
    typename KeyEqualOrVoid,
    typename AllocOrVoid>
class VectorContainerPolicy;

template <typename ValuePtr>
class VectorContainerIterator : public BaseIter<ValuePtr, uint32_t> {
  using Super = BaseIter<ValuePtr, uint32_t>;
  using typename Super::ValueConstPtr;

 public:
  using typename Super::pointer;
  using typename Super::reference;
  using typename Super::value_type;

  VectorContainerIterator() = default;
  VectorContainerIterator(VectorContainerIterator const&) = default;
  VectorContainerIterator(VectorContainerIterator&&) = default;
  VectorContainerIterator& operator=(VectorContainerIterator const&) = default;
  VectorContainerIterator& operator=(VectorContainerIterator&&) = default;
  ~VectorContainerIterator() = default;

  /*implicit*/ operator VectorContainerIterator<ValueConstPtr>() const {
    return VectorContainerIterator<ValueConstPtr>{current_, lowest_};
  }

  reference operator*() const {
    return *current_;
  }

  pointer operator->() const {
    return current_;
  }

  VectorContainerIterator& operator++() {
    if (UNLIKELY(current_ == lowest_)) {
      current_ = nullptr;
    } else {
      --current_;
    }
    return *this;
  }

  VectorContainerIterator operator++(int) {
    auto cur = *this;
    ++*this;
    return cur;
  }

  bool operator==(VectorContainerIterator<ValueConstPtr> const& rhs) const {
    return current_ == rhs.current_;
  }
  bool operator!=(VectorContainerIterator<ValueConstPtr> const& rhs) const {
    return !(*this == rhs);
  }

 private:
  ValuePtr current_;
  ValuePtr lowest_;

  explicit VectorContainerIterator(ValuePtr current, ValuePtr lowest)
      : current_(current), lowest_(lowest) {}

  std::size_t index() const {
    return current_ - lowest_;
  }

  template <typename K, typename M, typename H, typename E, typename A>
  friend class VectorContainerPolicy;

  template <typename P>
  friend class VectorContainerIterator;
};

struct VectorContainerIndexSearch {
  uint32_t index_;
};

template <
    typename Key,
    typename MappedTypeOrVoid,
    typename HasherOrVoid,
    typename KeyEqualOrVoid,
    typename AllocOrVoid>
class VectorContainerPolicy : public BasePolicy<
                                  Key,
                                  MappedTypeOrVoid,
                                  HasherOrVoid,
                                  KeyEqualOrVoid,
                                  AllocOrVoid,
                                  uint32_t> {
 public:
  using Super = BasePolicy<
      Key,
      MappedTypeOrVoid,
      HasherOrVoid,
      KeyEqualOrVoid,
      AllocOrVoid,
      uint32_t>;
  using typename Super::Alloc;
  using typename Super::AllocTraits;
  using typename Super::ByteAlloc;
  using typename Super::ByteAllocTraits;
  using typename Super::BytePtr;
  using typename Super::Hasher;
  using typename Super::Item;
  using typename Super::ItemIter;
  using typename Super::KeyEqual;
  using typename Super::Value;

  using Super::kAllocIsAlwaysEqual;

 private:
  using Super::kIsMap;

 public:
  static constexpr bool kEnableItemIteration = false;

  using InternalSizeType = Item;

  using ConstIter =
      VectorContainerIterator<typename AllocTraits::const_pointer>;
  using Iter = std::conditional_t<
      kIsMap,
      VectorContainerIterator<typename AllocTraits::pointer>,
      ConstIter>;
  using ConstReverseIter = typename AllocTraits::const_pointer;
  using ReverseIter = std::
      conditional_t<kIsMap, typename AllocTraits::pointer, ConstReverseIter>;

  using ValuePtr = typename AllocTraits::pointer;

  //////// F14Table policy

  static constexpr bool prefetchBeforeRehash() {
    return true;
  }

  static constexpr bool prefetchBeforeCopy() {
    return false;
  }

  static constexpr bool prefetchBeforeDestroy() {
    return false;
  }

  static constexpr bool destroyItemOnClear() {
    return false;
  }

 private:
  static constexpr bool valueIsTriviallyCopyable() {
    return folly::AllocatorHasDefaultObjectConstruct<Alloc, Value, Value>::
               value &&
        folly::AllocatorHasDefaultObjectDestroy<Alloc, Value>::value &&
        folly::is_trivially_copyable<Value>::value;
  }

 public:
  VectorContainerPolicy(
      Hasher const& hasher,
      KeyEqual const& keyEqual,
      Alloc const& alloc)
      : Super{hasher, keyEqual, alloc} {}

  VectorContainerPolicy(VectorContainerPolicy const& rhs) : Super{rhs} {
    // values_ will get allocated later to do the copy
  }

  VectorContainerPolicy(VectorContainerPolicy const& rhs, Alloc const& alloc)
      : Super{rhs, alloc} {
    // values_ will get allocated later to do the copy
  }

  VectorContainerPolicy(VectorContainerPolicy&& rhs) noexcept
      : Super{std::move(rhs)}, values_{rhs.values_} {
    rhs.values_ = nullptr;
  }

  VectorContainerPolicy(
      VectorContainerPolicy&& rhs,
      Alloc const& alloc) noexcept
      : Super{std::move(rhs), alloc} {
    if (kAllocIsAlwaysEqual || this->alloc() == rhs.alloc()) {
      // common case
      values_ = rhs.values_;
      rhs.values_ = nullptr;
    } else {
      // table must be constructed in new memory
      values_ = nullptr;
    }
  }

  VectorContainerPolicy& operator=(VectorContainerPolicy const& rhs) {
    if (this != &rhs) {
      FOLLY_SAFE_DCHECK(values_ == nullptr, "");
      Super::operator=(rhs);
    }
    return *this;
  }

  VectorContainerPolicy& operator=(VectorContainerPolicy&& rhs) noexcept {
    if (this != &rhs) {
      FOLLY_SAFE_DCHECK(values_ == nullptr, "");
      bool transfer =
          AllocTraits::propagate_on_container_move_assignment::value ||
          kAllocIsAlwaysEqual || this->alloc() == rhs.alloc();
      Super::operator=(std::move(rhs));
      if (transfer) {
        values_ = rhs.values_;
        rhs.values_ = nullptr;
      }
    }
    return *this;
  }

  void swapPolicy(VectorContainerPolicy& rhs) {
    using std::swap;
    this->swapBasePolicy(rhs);
    swap(values_, rhs.values_);
  }

  template <typename K>
  std::size_t computeKeyHash(K const& key) const {
    static_assert(
        Super::isAvalanchingHasher() ==
            IsAvalanchingHasher<typename Super::Hasher, K>::value,
        "");
    return this->hasher()(key);
  }

  std::size_t computeKeyHash(VectorContainerIndexSearch const& key) const {
    return computeItemHash(key.index_);
  }

  using Super::keyForValue;

  std::size_t computeItemHash(Item const& item) const {
    return this->computeKeyHash(keyForValue(values_[item]));
  }

  bool keyMatchesItem(VectorContainerIndexSearch const& key, Item const& item)
      const {
    return key.index_ == item;
  }

  template <typename K>
  bool keyMatchesItem(K const& key, Item const& item) const {
    return this->keyEqual()(key, keyForValue(values_[item]));
  }

  Key const& keyForValue(VectorContainerIndexSearch const& arg) const {
    return keyForValue(values_[arg.index_]);
  }

  VectorContainerIndexSearch buildArgForItem(Item const& item) const {
    return {item};
  }

  Value&& valueAtItemForExtract(Item& item) {
    return std::move(values_[item]);
  }

  void constructValueAtItem(
      std::size_t /*size*/,
      Item* itemAddr,
      VectorContainerIndexSearch arg) {
    *itemAddr = arg.index_;
  }

  template <typename... Args>
  void constructValueAtItem(std::size_t size, Item* itemAddr, Args&&... args) {
    Alloc& a = this->alloc();
    *itemAddr = size;
    AllocTraits::construct(
        a, std::addressof(values_[size]), std::forward<Args>(args)...);
  }

  void moveItemDuringRehash(Item* itemAddr, Item& src) {
    *itemAddr = src;
  }

  void prefetchValue(Item const& item) const {
    prefetchAddr(std::addressof(values_[item]));
  }

  void destroyItem(Item&) {}

  template <typename T>
  std::enable_if_t<std::is_nothrow_move_constructible<T>::value>
  complainUnlessNothrowMove() {}

  template <typename T>
  [[deprecated(
      "use F14NodeMap/Set or mark key and mapped type move constructor nothrow")]] std::
      enable_if_t<!std::is_nothrow_move_constructible<T>::value>
      complainUnlessNothrowMove() {}

  void transfer(Alloc& a, Value* src, Value* dst, std::size_t n) {
    complainUnlessNothrowMove<Key>();
    complainUnlessNothrowMove<lift_unit_t<MappedTypeOrVoid>>();

    if (valueIsTriviallyCopyable()) {
      std::memcpy(dst, src, n * sizeof(Value));
    } else {
      for (std::size_t i = 0; i < n; ++i, ++src, ++dst) {
        folly::assume(dst != nullptr);
        AllocTraits::construct(a, dst, Super::moveValue(*src));
        if (kIsMap) {
          AllocTraits::destroy(a, folly::launder(src));
        } else {
          AllocTraits::destroy(a, src);
        }
      }
    }
  }

  template <typename P, typename V>
  bool beforeBuildImpl(std::size_t size, P&& rhs, V const& constructorArgFor) {
    Alloc& a = this->alloc();

    FOLLY_SAFE_DCHECK(values_ != nullptr, "");

    auto src = std::addressof(rhs.values_[0]);
    Value* dst = std::addressof(values_[0]);

    if (valueIsTriviallyCopyable()) {
      std::memcpy(dst, src, size * sizeof(Value));
    } else {
      for (std::size_t i = 0; i < size; ++i, ++src, ++dst) {
        try {
          folly::assume(dst != nullptr);
          AllocTraits::construct(a, dst, constructorArgFor(*src));
        } catch (...) {
          for (Value* cleanup = std::addressof(values_[0]); cleanup != dst;
               ++cleanup) {
            AllocTraits::destroy(a, cleanup);
          }
          throw;
        }
      }
    }
    return true;
  }

  bool beforeBuild(
      std::size_t size,
      std::size_t /*capacity*/,
      VectorContainerPolicy const& rhs) {
    return beforeBuildImpl(size, rhs, [](Value const& v) { return v; });
  }

  bool beforeBuild(
      std::size_t size,
      std::size_t /*capacity*/,
      VectorContainerPolicy&& rhs) {
    return beforeBuildImpl(
        size, rhs, [](Value& v) { return Super::moveValue(v); });
  }

  template <typename P>
  void afterBuild(
      bool /*undoState*/,
      bool success,
      std::size_t /*size*/,
      std::size_t /*capacity*/,
      P&& /*rhs*/) {
    // buildArgForItem can be used to construct a new item trivially,
    // so no failure between beforeBuild and afterBuild should be possible
    FOLLY_SAFE_DCHECK(success, "");
  }

 private:
  // Returns the byte offset of the first Value in a unified allocation
  // that first holds prefixBytes of data, where prefixBytes comes from
  // Chunk storage and hence must be at least 8-byte aligned (sub-Chunk
  // allocations always have an even capacity and sizeof(Item) == 4).
  static std::size_t valuesOffset(std::size_t prefixBytes) {
    FOLLY_SAFE_DCHECK((prefixBytes % 8) == 0, "");
    if (alignof(Value) > 8) {
      prefixBytes = -(-prefixBytes & ~(alignof(Value) - 1));
    }
    FOLLY_SAFE_DCHECK((prefixBytes % alignof(Value)) == 0, "");
    return prefixBytes;
  }

  // Returns the total number of bytes that should be allocated to store
  // prefixBytes of Chunks and valueCapacity values.
  static std::size_t allocSize(
      std::size_t prefixBytes,
      std::size_t valueCapacity) {
    return valuesOffset(prefixBytes) + sizeof(Value) * valueCapacity;
  }

 public:
  ValuePtr beforeRehash(
      std::size_t size,
      std::size_t oldCapacity,
      std::size_t newCapacity,
      std::size_t chunkAllocSize,
      BytePtr& outChunkAllocation) {
    FOLLY_SAFE_DCHECK(
        size <= oldCapacity && ((values_ == nullptr) == (oldCapacity == 0)) &&
            newCapacity > 0 &&
            newCapacity <= (std::numeric_limits<Item>::max)(),
        "");

    outChunkAllocation =
        allocateOverAligned<ByteAlloc, kRequiredVectorAlignment>(
            ByteAlloc{Super::alloc()}, allocSize(chunkAllocSize, newCapacity));

    ValuePtr before = values_;
    ValuePtr after = std::pointer_traits<ValuePtr>::pointer_to(
        *static_cast<Value*>(static_cast<void*>(
            &*outChunkAllocation + valuesOffset(chunkAllocSize))));

    if (size > 0) {
      Alloc& a{this->alloc()};
      transfer(a, std::addressof(before[0]), std::addressof(after[0]), size);
    }

    values_ = after;
    return before;
  }

  FOLLY_NOINLINE void afterFailedRehash(ValuePtr state, std::size_t size) {
    // state holds the old storage
    Alloc& a = this->alloc();
    if (size > 0) {
      transfer(a, std::addressof(values_[0]), std::addressof(state[0]), size);
    }
    values_ = state;
  }

  void afterRehash(
      ValuePtr state,
      bool success,
      std::size_t size,
      std::size_t oldCapacity,
      std::size_t newCapacity,
      BytePtr chunkAllocation,
      std::size_t chunkAllocSize) {
    if (!success) {
      afterFailedRehash(state, size);
    }

    // on success, chunkAllocation is the old allocation, on failure it is the
    // new one
    if (chunkAllocation != nullptr) {
      deallocateOverAligned<ByteAlloc, kRequiredVectorAlignment>(
          ByteAlloc{Super::alloc()},
          chunkAllocation,
          allocSize(chunkAllocSize, (success ? oldCapacity : newCapacity)));
    }
  }

  void beforeClear(std::size_t size, std::size_t capacity) {
    FOLLY_SAFE_DCHECK(
        size <= capacity && ((values_ == nullptr) == (capacity == 0)), "");
    Alloc& a = this->alloc();
    for (std::size_t i = 0; i < size; ++i) {
      AllocTraits::destroy(a, std::addressof(values_[i]));
    }
  }

  void beforeReset(std::size_t size, std::size_t capacity) {
    beforeClear(size, capacity);
  }

  void afterReset(
      std::size_t /*size*/,
      std::size_t capacity,
      BytePtr chunkAllocation,
      std::size_t chunkAllocSize) {
    if (chunkAllocation != nullptr) {
      deallocateOverAligned<ByteAlloc, kRequiredVectorAlignment>(
          ByteAlloc{Super::alloc()},
          chunkAllocation,
          allocSize(chunkAllocSize, capacity));
      values_ = nullptr;
    }
  }

  template <typename V>
  void visitPolicyAllocationClasses(
      std::size_t chunkAllocSize,
      std::size_t /*size*/,
      std::size_t capacity,
      V&& visitor) const {
    FOLLY_SAFE_DCHECK((chunkAllocSize == 0) == (capacity == 0), "");
    if (chunkAllocSize > 0) {
      visitor(
          allocationBytesForOverAligned<ByteAlloc, kRequiredVectorAlignment>(
              allocSize(chunkAllocSize, capacity)),
          1);
    }
  }

  // Iterator stuff

  Iter linearBegin(std::size_t size) const {
    return Iter{(size > 0 ? values_ + size - 1 : nullptr), values_};
  }

  Iter linearEnd() const {
    return Iter{nullptr, nullptr};
  }

  //////// F14BasicMap/Set policy

  Iter makeIter(ItemIter const& underlying) const {
    if (underlying.atEnd()) {
      return linearEnd();
    } else {
      folly::assume(values_ + underlying.item() != nullptr);
      folly::assume(values_ != nullptr);
      return Iter{values_ + underlying.item(), values_};
    }
  }

  ConstIter makeConstIter(ItemIter const& underlying) const {
    return makeIter(underlying);
  }

  Item iterToIndex(ConstIter const& iter) const {
    auto n = iter.index();
    folly::assume(n <= std::numeric_limits<Item>::max());
    return static_cast<Item>(n);
  }

  Iter indexToIter(Item index) const {
    return Iter{values_ + index, values_};
  }

  Iter iter(ReverseIter it) {
    return Iter{it, values_};
  }

  ConstIter iter(ConstReverseIter it) const {
    return ConstIter{it, values_};
  }

  ReverseIter riter(Iter it) {
    return it.current_;
  }

  ConstReverseIter riter(ConstIter it) const {
    return it.current_;
  }

  ValuePtr values_{nullptr};
};

template <
    template <typename, typename, typename, typename, typename> class Policy,
    typename Key,
    typename Mapped,
    typename Hasher,
    typename KeyEqual,
    typename Alloc>
using MapPolicyWithDefaults = Policy<
    Key,
    Mapped,
    VoidDefault<Hasher, DefaultHasher<Key>>,
    VoidDefault<KeyEqual, DefaultKeyEqual<Key>>,
    VoidDefault<Alloc, DefaultAlloc<std::pair<Key const, Mapped>>>>;

template <
    template <typename, typename, typename, typename, typename> class Policy,
    typename Key,
    typename Hasher,
    typename KeyEqual,
    typename Alloc>
using SetPolicyWithDefaults = Policy<
    Key,
    void,
    VoidDefault<Hasher, DefaultHasher<Key>>,
    VoidDefault<KeyEqual, DefaultKeyEqual<Key>>,
    VoidDefault<Alloc, DefaultAlloc<Key>>>;

} // namespace detail
} // namespace f14
} // namespace folly

#endif // FOLLY_F14_VECTOR_INTRINSICS_AVAILABLE