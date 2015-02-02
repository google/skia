#ifndef SkAtomics_atomic_DEFINED
#define SkAtomics_atomic_DEFINED

template <typename T>
T sk_atomic_load(const T* ptr, sk_memory_order mo) {
    return __atomic_load_n(ptr, mo);
}

template <typename T>
void sk_atomic_store(T* ptr, T val, sk_memory_order mo) {
    __atomic_store_n(ptr, val, mo);
}

template <typename T>
T sk_atomic_fetch_add(T* ptr, T val, sk_memory_order mo) {
    return __atomic_fetch_add(ptr, val, mo);
}

template <typename T>
bool sk_atomic_compare_exchange(T* ptr, T* expected, T desired,
                                sk_memory_order success,
                                sk_memory_order failure) {
    return __atomic_compare_exchange_n(ptr, expected, desired, false/*weak?*/, success, failure);
}

#endif//SkAtomics_atomic_DEFINED
