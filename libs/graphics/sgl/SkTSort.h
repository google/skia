#ifndef SkTSort_DEFINED
#define SkTSort_DEFINED

#include "SkTypes.h"

template <typename T>
void SkTHeapSort_SiftDown(T array[], int root, int bottom)
{
	int	root2 = root << 1;

	while (root2 <= bottom)
	{
		int	maxChild;

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
