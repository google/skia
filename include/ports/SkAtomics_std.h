#ifndef SkAtomics_std_DEFINED
#define SkAtomics_std_DEFINED

// We try not to depend on the C++ standard library,
// but these uses of <atomic> should all inline, so we don't feel to bad here.
#include <atomic>

template <typename T>
T sk_atomic_load(const T* ptr, sk_memory_order mo) {
    const std::atomic<T>* ap = reinterpret_cast<const std::atomic<T>*>(ptr);
    return std::atomic_load_explicit(ap, (std::memory_order)mo);
}

template <typename T>
void sk_atomic_store(T* ptr, T val, sk_memory_order mo) {
    std::atomic<T>* ap = reinterpret_cast<std::atomic<T>*>(ptr);
    return std::atomic_store_explicit(ap, val, (std::memory_order)mo);
}

template <typename T>
T sk_atomic_fetch_add(T* ptr, T val, sk_memory_order mo) {
    std::atomic<T>* ap = reinterpret_cast<std::atomic<T>*>(ptr);
    return std::atomic_fetch_add_explicit(ap, val, (std::memory_order)mo);
}

template <typename T>
bool sk_atomic_compare_exchange(T* ptr, T* expected, T desired,
                                sk_memory_order success,
                                sk_memory_order failure) {
    std::atomic<T>* ap = reinterpret_cast<std::atomic<T>*>(ptr);
    return std::atomic_compare_exchange_strong_explicit(ap, expected, desired,
                                                        (std::memory_order)success,
                                                        (std::memory_order)failure);
}

#endif//SkAtomics_std_DEFINED
