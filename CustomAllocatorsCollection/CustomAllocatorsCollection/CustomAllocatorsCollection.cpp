// CustomAllocatorsCollection.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

///////////////////////////////////////////
//	Linear Allocator				[]
//	Stack Allocator					[]
//  Double Ended Stack Allocator	[]
//	Pool Allocator					[]
//	List Allocator				    []
//	Single Buffered Allocator		[]
//	Double Buffered Allocator		[]
/////////////////////////////////////////


#include "pch.h"
#include <iostream>
#include "LinkedListAllocator.h"

int main()
{
	LinkedListAllocator* listAllocator = new LinkedListAllocator(1e8);
	listAllocator->InitializeAllocator();

	void* ptr = listAllocator->CMalloc(1014, 8);

	Log("---------------------------------------------------------------");

	listAllocator->CFree(ptr);

	delete listAllocator;
}


