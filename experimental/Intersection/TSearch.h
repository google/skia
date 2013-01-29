/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef TSearch_DEFINED
#define TSearch_DEFINED

// FIXME: Move this templated version into SKTSearch.h

template <typename T>
static T* QSort_Partition(T* left, T* right, T* pivot)
{
    T pivotValue = *pivot;
    SkTSwap(*pivot, *right);
    T* newPivot = left;
    while (left < right) {
        if (*left < pivotValue) {
            SkTSwap(*left, *newPivot);
            newPivot += 1;
        }
        left += 1;
    }
    SkTSwap(*newPivot, *right);
    return newPivot;
}

template <typename T>
void QSort(T* left, T* right)
{
    if (left >= right) {
        return;
    }
    T* pivot = left + (right - left >> 1);
    pivot = QSort_Partition(left, right, pivot);
    QSort(left, pivot - 1);
    QSort(pivot + 1, right);
}

template <typename T>
static T** QSort_Partition(T** left, T** right, T** pivot)
{
    T* pivotValue = *pivot;
    SkTSwap(*pivot, *right);
    T** newPivot = left;
    while (left < right) {
        if (**left < *pivotValue) {
            SkTSwap(*left, *newPivot);
            newPivot += 1;
        }
        left += 1;
    }
    SkTSwap(*newPivot, *right);
    return newPivot;
}

template <typename T>
void QSort(T** left, T** right)
{
    if (left >= right) {
        return;
    }
    T** pivot = left + (right - left >> 1);
    pivot = QSort_Partition(left, right, pivot);
    QSort(left, pivot - 1);
    QSort(pivot + 1, right);
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

#endif
