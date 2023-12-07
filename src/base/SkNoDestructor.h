/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkNoDestructor_DEFINED
#define SkNoDestructor_DEFINED

#include <cstddef>
#include <new>
#include <type_traits>  // IWYU pragma: keep
#include <utility>

// Helper type to create a function-local static variable of type `T` when `T`
// has a non-trivial destructor. Storing a `T` in a `SkNoDestructor<T>` will
// prevent `~T()` from running, even when the variable goes out of scope. This
// code is adapted from `base::NoDestructor<T>` in Chromium.
//
// Useful when a variable has static storage duration but its type has a
// non-trivial destructor. Chromium (and transitively, Skia) bans global
// constructors and destructors: using a function-local static variable prevents
// the former, while using `SkNoDestructor<T>` prevents the latter.
//
// ## Caveats
//
// - Must not be used for locals or fields; by definition, this does not run
//   destructors, and this will likely lead to memory leaks and other
//   surprising and undesirable behaviour.
//
// - If `T` is not constexpr constructible, must be a function-local static
//   variable, since a global `NoDestructor<T>` will still generate a static
//   initializer.
//
// - If `T` is constinit constructible, may be used as a global, but mark the
//   global `constinit` (once C++20 is available)
//
// - If the data is rarely used, consider creating it on demand rather than
//   caching it for the lifetime of the program. Though `SkNoDestructor<T>`
//   does not heap allocate, the compiler still reserves space in bss for
//   storing `T`, which costs memory at runtime.
//
// - If `T` is trivially destructible, do not use `SkNoDestructor<T>`:
//
//     const uint64_t GetUnstableSessionSeed() {
//       // No need to use `SkNoDestructor<T>` as `uint64_t` is trivially
//       // destructible and does not require a global destructor.
//       static const uint64_t kSessionSeed = GetRandUint64();
//       return kSessionSeed;
//     }
//
// ## Example Usage
//
// const std::string& GetDefaultText() {
//   // Required since `static const std::string` requires a global destructor.
//   static const SkNoDestructor<std::string> s("Hello world!");
//   return *s;
// }
//
// More complex initialization using a lambda:
//
// const std::string& GetRandomNonce() {
//   // `nonce` is initialized with random data the first time this function is
//   // called, but its value is fixed thereafter.
//   static const SkNoDestructor<std::string> nonce([] {
//     std::string s(16);
//     GetRandString(s.data(), s.size());
//     return s;
//   }());
//   return *nonce;
// }
//
// ## Thread safety
//
// Initialization of function-local static variables is thread-safe since C++11.
// The standard guarantees that:
//
// - function-local static variables will be initialised the first time
//   execution passes through the declaration.
//
// - if another thread's execution concurrently passes through the declaration
//   in the middle of initialisation, that thread will wait for the in-progress
//   initialisation to complete.
template <typename T> class SkNoDestructor {
public:
    static_assert(!(std::is_trivially_constructible_v<T> && std::is_trivially_destructible_v<T>),
                  "T is trivially constructible and destructible; please use a constinit object of "
                  "type T directly instead");

    static_assert(!std::is_trivially_destructible_v<T>,
                  "T is trivially destructible; please use a function-local static of type T "
                  "directly instead");

    // Not constexpr; just write static constexpr T x = ...; if the value should be a constexpr.
    template <typename... Args> explicit SkNoDestructor(Args&&... args) {
        new (fStorage) T(std::forward<Args>(args)...);
    }

    // Allows copy and move construction of the contained type, to allow construction from an
    // initializer list, e.g. for std::vector.
    explicit SkNoDestructor(const T& x) { new (fStorage) T(x); }
    explicit SkNoDestructor(T&& x) { new (fStorage) T(std::move(x)); }

    SkNoDestructor(const SkNoDestructor&) = delete;
    SkNoDestructor& operator=(const SkNoDestructor&) = delete;

    ~SkNoDestructor() = default;

    const T& operator*() const { return *get(); }
    T& operator*() { return *get(); }

    const T* operator->() const { return get(); }
    T* operator->() { return get(); }

    const T* get() const { return reinterpret_cast<const T*>(fStorage); }
    T* get() { return reinterpret_cast<T*>(fStorage); }

private:
    alignas(T) std::byte fStorage[sizeof(T)];

#if defined(__clang__) && defined(__has_feature)
#if __has_feature(leak_sanitizer) || __has_feature(address_sanitizer)
    // TODO(https://crbug.com/812277): This is a hack to work around the fact that LSan doesn't seem
    // to treat SkNoDestructor as a root for reachability analysis. This means that code like this:
    //     static SkNoDestructor<std::vector<int>> v({1, 2, 3});
    // is considered a leak. Using the standard leak sanitizer annotations to suppress leaks doesn't
    // work: std::vector is implicitly constructed before calling the SkNoDestructor constructor.
    //
    // Unfortunately, I haven't been able to demonstrate this issue in simpler reproductions: until
    // that's resolved, hold an explicit pointer to the placement-new'd object in leak sanitizer
    // mode to help LSan realize that objects allocated by the contained type are still reachable.
    T* fStoragePtr = reinterpret_cast<T*>(fStorage);
#endif  // leak_sanitizer/address_sanitizer
#endif  // __has_feature
};

#endif  // SkNoDestructor_DEFINED
