#ifndef SkTSort_DEFINED
#define SkTSort_DEFINED

template <typename T, typename C> void SkTQSort(T* left, T* right, C lessThan);
template <typename T> void SkTQSort(T* left, T* right);
template <typename T> void SkTQSort(T** left, T** right);

#endif
