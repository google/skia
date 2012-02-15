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

template <typename S, typename T>
static void QSort_Partition(S& context, T* first, T* last,
        bool (*lessThan)(S&, const T*, const T*))
{
    T*   left = first;
    T*   rite = last;
    T*   pivot = left;

    while (left <= rite) {
        while (left < last && lessThan(context, left, pivot))
            left += 1;
        while (first < rite && lessThan(context, pivot, rite))
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
        QSort_Partition(context, first, rite, lessThan);
    if (left < last)
        QSort_Partition(context, left, last, lessThan);
}

template <typename S, typename T>
void QSort(S& context, T* base, size_t count,
        bool (*lessThan)(S& , const T*, const T*))
{
    SkASSERT(base);
    SkASSERT(lessThan);

    if (count <= 1) {
        return;
    }
    QSort_Partition(context, base, base + (count - 1), lessThan);
}
