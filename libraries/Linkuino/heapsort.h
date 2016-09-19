#ifndef __Linkuino_heapsort_h
#define __Linkuino_heapsort_h

template<typename ElemT>
static inline void siftDown(ElemT numbers[], int root, int bottom)
{
  int maxChild = root * 2 + 1;
 
  // Find the biggest child
  if(maxChild < bottom) {
	int otherChild = maxChild + 1;
	// Reversed for stability
	maxChild = (numbers[otherChild] > numbers[maxChild])?otherChild:maxChild;
  } else {
	// Don't overflow
	if(maxChild > bottom) return;
  }
 
  // If we have the correct ordering, we are done.
  if(numbers[root] >= numbers[maxChild]) return;
 
  // Swap
  {
	ElemT temp = numbers[root];
	numbers[root] = numbers[maxChild];
	numbers[maxChild] = temp;
  }
 
  // Tail queue recursion. Will be compiled as a loop with correct compiler switches.
  siftDown<ElemT>(numbers, maxChild, bottom);
}

template<typename ElemT, int16_t array_size>
static inline void heapSort(ElemT numbers[])
{	 
  for (int i = (array_size / 2); i >= 0; i--) {
	siftDown<ElemT>(numbers, i, array_size - 1);
  }
 
  for (int i = array_size-1; i >= 1; i--) {
	// Swap
	ElemT temp = numbers[0];
	numbers[0] = numbers[i];
	numbers[i] = temp;
 
	siftDown<ElemT>(numbers, 0, i-1);
  }
}

#endif
