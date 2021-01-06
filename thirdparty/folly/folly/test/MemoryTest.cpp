/*
 * Copyright 2013-present Facebook, Inc.
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

#include <folly/Memory.h>

#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

#include <glog/logging.h>

#include <folly/ConstexprMath.h>
#include <folly/String.h>
#include <folly/memory/Arena.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

using namespace folly;

static constexpr std::size_t kTooBig = folly::constexpr_max(
    std::size_t{std::numeric_limits<uint32_t>::max()},
    std::size_t{1} << (8 * sizeof(std::size_t) - 14));

TEST(aligned_malloc, examples) {
  auto trial = [](size_t align) {
    auto const ptr = aligned_malloc(1, align);
    return (aligned_free(ptr), uintptr_t(ptr));
  };

  if (!kIsSanitize) { // asan allocator raises SIGABRT instead
    EXPECT_EQ(EINVAL, (trial(2), errno)) << "too small";
    EXPECT_EQ(EINVAL, (trial(513), errno)) << "not power of two";
  }

  EXPECT_EQ(0, trial(512) % 512);
  EXPECT_EQ(0, trial(8192) % 8192);
}

TEST(make_unique, compatible_with_std_make_unique) {
  //  HACK: To enforce that `folly::` is imported here.
  to_shared_ptr(std::unique_ptr<std::string>());

  using namespace std;
  make_unique<string>("hello, world");
}

TEST(to_weak_ptr, example) {
  auto s = std::make_shared<int>(17);
  EXPECT_EQ(1, s.use_count());
  EXPECT_EQ(2, (to_weak_ptr(s).lock(), s.use_count())) << "lvalue";
  EXPECT_EQ(3, (to_weak_ptr(decltype(s)(s)).lock(), s.use_count())) << "rvalue";
}

TEST(SysAllocator, equality) {
  using Alloc = SysAllocator<float>;
  Alloc const a, b;
  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a != b);
}

TEST(SysAllocator, allocate_unique) {
  using Alloc = SysAllocator<float>;
  Alloc const alloc;
  auto ptr = allocate_unique<float>(alloc, 3.);
  EXPECT_EQ(3., *ptr);
}

TEST(SysAllocator, vector) {
  using Alloc = SysAllocator<float>;
  Alloc const alloc;
  std::vector<float, Alloc> nums(alloc);
  nums.push_back(3.);
  nums.push_back(5.);
  EXPECT_THAT(nums, testing::ElementsAreArray({3., 5.}));
}

TEST(SysAllocator, bad_alloc) {
  using Alloc = SysAllocator<float>;
  Alloc const alloc;
  std::vector<float, Alloc> nums(alloc);
  if (!kIsSanitize) {
    EXPECT_THROW(nums.reserve(kTooBig), std::bad_alloc);
  }
}

TEST(AlignedSysAllocator, equality_fixed) {
  using Alloc = AlignedSysAllocator<float, FixedAlign<1024>>;
  Alloc const a, b;
  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a != b);
}

TEST(AlignedSysAllocator, allocate_unique_fixed) {
  using Alloc = AlignedSysAllocator<float, FixedAlign<1024>>;
  Alloc const alloc;
  auto ptr = allocate_unique<float>(alloc, 3.);
  EXPECT_EQ(3., *ptr);
  EXPECT_EQ(0, std::uintptr_t(ptr.get()) % 1024);
}

TEST(AlignedSysAllocator, vector_fixed) {
  using Alloc = AlignedSysAllocator<float, FixedAlign<1024>>;
  Alloc const alloc;
  std::vector<float, Alloc> nums(alloc);
  nums.push_back(3.);
  nums.push_back(5.);
  EXPECT_THAT(nums, testing::ElementsAreArray({3., 5.}));
  EXPECT_EQ(0, std::uintptr_t(nums.data()) % 1024);
}

TEST(AlignedSysAllocator, bad_alloc_fixed) {
  using Alloc = AlignedSysAllocator<float, FixedAlign<1024>>;
  Alloc const alloc;
  std::vector<float, Alloc> nums(alloc);
  if (!kIsSanitize) {
    EXPECT_THROW(nums.reserve(kTooBig), std::bad_alloc);
  }
}

TEST(AlignedSysAllocator, equality_default) {
  using Alloc = AlignedSysAllocator<float>;
  Alloc const a(1024), b(1024), c(512);
  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a != b);
  EXPECT_FALSE(a == c);
  EXPECT_TRUE(a != c);
}

TEST(AlignedSysAllocator, allocate_unique_default) {
  using Alloc = AlignedSysAllocator<float>;
  Alloc const alloc(1024);
  auto ptr = allocate_unique<float>(alloc, 3.);
  EXPECT_EQ(3., *ptr);
  EXPECT_EQ(0, std::uintptr_t(ptr.get()) % 1024);
}

TEST(AlignedSysAllocator, vector_default) {
  using Alloc = AlignedSysAllocator<float>;
  Alloc const alloc(1024);
  std::vector<float, Alloc> nums(alloc);
  nums.push_back(3.);
  nums.push_back(5.);
  EXPECT_THAT(nums, testing::ElementsAreArray({3., 5.}));
  EXPECT_EQ(0, std::uintptr_t(nums.data()) % 1024);
}

TEST(AlignedSysAllocator, bad_alloc_default) {
  using Alloc = AlignedSysAllocator<float>;
  Alloc const alloc(1024);
  std::vector<float, Alloc> nums(alloc);
  if (!kIsSanitize) {
    EXPECT_THROW(nums.reserve(kTooBig), std::bad_alloc);
  }
}

TEST(allocate_sys_buffer, compiles) {
  auto buf = allocate_sys_buffer(256);
  //  Freed at the end of the scope.
}

namespace {
template <typename T>
struct TestAlloc1 : SysAllocator<T> {
  template <typename U, typename... Args>
  void construct(U* p, Args&&... args) {
    ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
  }
};

template <typename T>
struct TestAlloc2 : TestAlloc1<T> {
  template <typename U>
  void destroy(U* p) {
    p->~U();
  }
};

template <typename T>
struct TestAlloc3 : TestAlloc2<T> {
  using folly_has_default_object_construct = std::true_type;
};

template <typename T>
struct TestAlloc4 : TestAlloc3<T> {
  using folly_has_default_object_destroy = std::true_type;
};

template <typename T>
struct TestAlloc5 : SysAllocator<T> {
  using folly_has_default_object_construct = std::true_type;
  using folly_has_default_object_destroy = std::false_type;
};
} // namespace

TEST(AllocatorObjectLifecycleTraits, compiles) {
  using A = std::allocator<int>;
  using S = std::string;

  static_assert(
      folly::AllocatorHasDefaultObjectConstruct<A, int, int>::value, "");
  static_assert(folly::AllocatorHasDefaultObjectConstruct<A, S, S>::value, "");

  static_assert(folly::AllocatorHasDefaultObjectDestroy<A, int>::value, "");
  static_assert(folly::AllocatorHasDefaultObjectDestroy<A, S>::value, "");

  static_assert(
      folly::AllocatorHasDefaultObjectConstruct<
          folly::AlignedSysAllocator<int>,
          int,
          int>::value,
      "");
  static_assert(
      folly::AllocatorHasDefaultObjectConstruct<
          folly::AlignedSysAllocator<int>,
          S,
          S>::value,
      "");

  static_assert(
      folly::AllocatorHasDefaultObjectDestroy<
          folly::AlignedSysAllocator<int>,
          int>::value,
      "");
  static_assert(
      folly::AllocatorHasDefaultObjectDestroy<
          folly::AlignedSysAllocator<int>,
          S>::value,
      "");

  static_assert(
      !folly::AllocatorHasDefaultObjectConstruct<TestAlloc1<S>, S, S>::value,
      "");
  static_assert(
      folly::AllocatorHasDefaultObjectDestroy<TestAlloc1<S>, S>::value, "");

  static_assert(
      !folly::AllocatorHasDefaultObjectConstruct<TestAlloc2<S>, S, S>::value,
      "");
  static_assert(
      !folly::AllocatorHasDefaultObjectDestroy<TestAlloc2<S>, S>::value, "");

  static_assert(
      folly::AllocatorHasDefaultObjectConstruct<TestAlloc3<S>, S, S>::value,
      "");
  static_assert(
      !folly::AllocatorHasDefaultObjectDestroy<TestAlloc3<S>, S>::value, "");

  static_assert(
      folly::AllocatorHasDefaultObjectConstruct<TestAlloc4<S>, S, S>::value,
      "");
  static_assert(
      folly::AllocatorHasDefaultObjectDestroy<TestAlloc4<S>, S>::value, "");

  static_assert(
      folly::AllocatorHasDefaultObjectConstruct<TestAlloc5<S>, S, S>::value,
      "");
  static_assert(
      !folly::AllocatorHasDefaultObjectDestroy<TestAlloc5<S>, S>::value, "");
}

template <typename C>
static void test_enable_shared_from_this(std::shared_ptr<C> sp) {
  ASSERT_EQ(1l, sp.use_count());

  // Test shared_from_this().
  std::shared_ptr<C> sp2 = sp->shared_from_this();
  ASSERT_EQ(sp, sp2);

  // Test weak_from_this().
  std::weak_ptr<C> wp = sp->weak_from_this();
  ASSERT_EQ(sp, wp.lock());
  sp.reset();
  sp2.reset();
  ASSERT_EQ(nullptr, wp.lock());

  // Test shared_from_this() and weak_from_this() on object not owned by a
  // shared_ptr. Undefined in C++14 but well-defined in C++17. Also known to
  // work with libstdc++ >= 20150123. Feel free to add other standard library
  // versions where the behavior is known.
#if __cplusplus >= 201700L || __GLIBCXX__ >= 20150123L
  C stack_resident;
  ASSERT_THROW(stack_resident.shared_from_this(), std::bad_weak_ptr);
  ASSERT_TRUE(stack_resident.weak_from_this().expired());
#endif
}

TEST(enable_shared_from_this, compatible_with_std_enable_shared_from_this) {
  // Compile-time compatibility.
  class C_std : public std::enable_shared_from_this<C_std> {};
  class C_folly : public folly::enable_shared_from_this<C_folly> {};
  static_assert(
      noexcept(std::declval<C_std>().shared_from_this()) ==
          noexcept(std::declval<C_folly>().shared_from_this()),
      "");
  static_assert(
      noexcept(std::declval<C_std const>().shared_from_this()) ==
          noexcept(std::declval<C_folly const>().shared_from_this()),
      "");
  static_assert(noexcept(std::declval<C_folly>().weak_from_this()), "");
  static_assert(noexcept(std::declval<C_folly const>().weak_from_this()), "");

  // Runtime compatibility.
  test_enable_shared_from_this(std::make_shared<C_folly>());
  test_enable_shared_from_this(std::make_shared<C_folly const>());
}

template <typename T>
struct ExpectingAlloc {
  using value_type = T;

  ExpectingAlloc(std::size_t expectedSize, std::size_t expectedCount)
      : expectedSize_{expectedSize}, expectedCount_{expectedCount} {}

  template <class U>
  explicit ExpectingAlloc(ExpectingAlloc<U> const& other) noexcept
      : expectedSize_{other.expectedSize_},
        expectedCount_{other.expectedCount_} {}

  T* allocate(std::size_t n) {
    EXPECT_EQ(expectedSize_, sizeof(T));
    EXPECT_EQ(expectedSize_, alignof(T));
    EXPECT_EQ(expectedCount_, n);
    return std::allocator<T>{}.allocate(n);
  }

  void deallocate(T* p, std::size_t n) {
    std::allocator<T>{}.deallocate(p, n);
  }

  std::size_t expectedSize_;
  std::size_t expectedCount_;
};

struct alignas(64) OverAlignedType {
  std::array<char, 64> raw_;
};

TEST(allocateOverAligned, notActuallyOver) {
  // allocates 6 bytes with alignment 4, should get turned into an
  // allocation of 2 elements of something of size and alignment 4
  ExpectingAlloc<char> a(4, 2);
  auto p = folly::allocateOverAligned<decltype(a), 4>(a, 6);
  EXPECT_EQ((reinterpret_cast<uintptr_t>(p) % 4), 0);
  folly::deallocateOverAligned<decltype(a), 4>(a, p, 6);
  EXPECT_EQ((folly::allocationBytesForOverAligned<decltype(a), 4>(6)), 8);
}

TEST(allocateOverAligned, manualOverStdAlloc) {
  // allocates 6 bytes with alignment 64 using std::allocator, which will
  // result in a call to aligned_malloc underneath.  We free one directly
  // to check that this is not the padding path
  std::allocator<short> a;
  auto p = folly::allocateOverAligned<decltype(a), 64>(a, 3);
  auto p2 = folly::allocateOverAligned<decltype(a), 64>(a, 3);
  EXPECT_EQ((reinterpret_cast<uintptr_t>(p) % 64), 0);
  folly::deallocateOverAligned<decltype(a), 64>(a, p, 3);
  aligned_free(p2);
  EXPECT_EQ((folly::allocationBytesForOverAligned<decltype(a), 64>(3)), 6);
}

TEST(allocateOverAligned, manualOverCustomAlloc) {
  // allocates 6 byte with alignment 64 using non-standard allocator, which
  // will result in an allocation of 64 + alignof(max_align_t) underneath.
  ExpectingAlloc<short> a(
      alignof(folly::max_align_t), 64 / alignof(folly::max_align_t) + 1);
  auto p = folly::allocateOverAligned<decltype(a), 64>(a, 3);
  EXPECT_EQ((reinterpret_cast<uintptr_t>(p) % 64), 0);
  folly::deallocateOverAligned<decltype(a), 64>(a, p, 3);
  EXPECT_EQ(
      (folly::allocationBytesForOverAligned<decltype(a), 64>(3)),
      64 + alignof(folly::max_align_t));
}

TEST(allocateOverAligned, defaultOverCustomAlloc) {
  ExpectingAlloc<OverAlignedType> a(
      alignof(folly::max_align_t), 128 / alignof(folly::max_align_t));
  auto p = folly::allocateOverAligned(a, 1);
  EXPECT_EQ((reinterpret_cast<uintptr_t>(p) % 64), 0);
  folly::deallocateOverAligned(a, p, 1);
  EXPECT_EQ(folly::allocationBytesForOverAligned<decltype(a)>(1), 128);
}
