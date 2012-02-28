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
static T* QSort_Partition(S& context, T* left, T* right, T* pivot,
        bool (*lessThan)(S&, const T, const T))
{
    T pivotValue = *pivot;
    SkTSwap(*pivot, *right);
    T* newPivot = left;
    while (left < right) {
        if (lessThan(context, *left, pivotValue)) {
            SkTSwap(*left, *newPivot);
            newPivot += 1;
        }
        left += 1;
    }
    SkTSwap(*newPivot, *right);
    return newPivot;
}

template <typename S, typename T>
void QSort(S& context, T* left, T* right,
        bool (*lessThan)(S& , const T, const T))
{
    if (left >= right) {
        return;
    }
    T* pivot = left + (right - left >> 1);
    pivot = QSort_Partition(context, left, right, pivot, lessThan);
    QSort(context, left, pivot - 1, lessThan);
    QSort(context, pivot + 1, right, lessThan);
}
