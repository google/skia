/* libs/corecg/SkTSort.h
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#ifndef SkTSort_DEFINED
#define SkTSort_DEFINED

#include "SkTypes.h"

template <typename T>
void SkTHeapSort_SiftDown(T array[], int root, int bottom)
{
    int root2 = root << 1;

    while (root2 <= bottom)
    {
        int maxChild;

        if (root2 == bottom)
            maxChild = root2;
        else if (array[root2] > array[root2 + 1])
            maxChild = root2;
        else
            maxChild = root2 + 1;

        if (array[root] < array[maxChild])
        {
            SkTSwap<T>(array[root], array[maxChild]);
            root = maxChild;
            root2 = root << 1;
        }
        else
            break;
    }
}

template <typename T>
void SkTHeapSort(T array[], int count)
{
    int i;

    for (i = count/2 - 1; i >= 0; --i)
        SkTHeapSort_SiftDown<T>(array, i, count);

    for (i = count - 2; i >= 0; --i)
    {
        SkTSwap<T>(array[0], array[i + 1]);
        SkTHeapSort_SiftDown<T>(array, 0, i);
    }
}

#endif
