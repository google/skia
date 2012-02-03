#include "SkTypes.h"

// FIXME: Move this templated version into SKTSearch.h

template <typename T>
static void QSort_Partition(T** first, T** last)
{
    T**   left = first;
    T**   rite = last;
    T**   pivot = left;

    while (left <= rite) {
        while (left < last && **left < **pivot)
            left += 1;
        while (first < rite && **pivot < **rite)
            rite -= 1;
        if (left <= rite) {
            if (left < rite) {
                SkTSwap(*left, *rite);
            }
            left += 1;
            rite -= 1;
        }
    }
    if (first < rite)
        QSort_Partition(first, rite);
    if (left < last)
        QSort_Partition(left, last);
}

template <typename T>
void QSort(T** base, size_t count)
{
    SkASSERT(base);

    if (count <= 1) {
        return;
    }
    QSort_Partition(base, base + (count - 1));
}

template <typename T>
static void QSort_Partition(T* first, T* last, bool (*lessThan)(const T*, const T*))
{
    T*   left = first;
    T*   rite = last;
    T*   pivot = left;

    while (left <= rite) {
        while (left < last && lessThan(left, pivot) < 0)
            left += 1;
        while (first < rite && lessThan(rite, pivot) > 0)
            rite -= 1;
        if (left <= rite) {
            if (left < rite) {
                SkTSwap(*left, *rite);
            }
            left += 1;
            rite -= 1;
        }
    }
    if (first < rite)
        QSort_Partition(first, rite, lessThan);
    if (left < last)
        QSort_Partition(left, last, lessThan);
}

template <typename T>
void QSort(T* base, size_t count, bool (*lessThan)(const T*, const T*))
{
    SkASSERT(base);
    SkASSERT(lessThan);

    if (count <= 1) {
        return;
    }
    QSort_Partition(base, base + (count - 1), lessThan);
}
