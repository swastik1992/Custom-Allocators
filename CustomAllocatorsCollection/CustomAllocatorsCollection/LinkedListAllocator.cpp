#include "pch.h"
#include "LinkedListAllocator.h"
#include <iostream>
#include <algorithm>

namespace List
{
	template <class T>
	LinkedList<T>::LinkedList()
	{

	}

	template <class T>
	void LinkedList<T>::Add(Node<T>* currentNode, Node<T>* prevNode)
	{
		if (prevNode != nullptr) //if prev is valid, 
		{
			if (prevNode->next == nullptr) // prevNode is the first element.
			{
				prevNode->next = currentNode;
				currentNode->next = nullptr;
			}
			else 
			{
				//point current node's next to whatever prevNode's next was pointing to.
				currentNode->next = prevNode->next;

				prevNode->next = currentNode;
			}
		}
		else //if no prev node, it should be the start of the list. 
		{
			if (head != nullptr) //add the element before head if head is valid.
			{
				currentNode->next = head;
			}
			else 
			{
				currentNode->next = nullptr;
			}

			// make the current node head.
			head = currentNode;
		}
	}

	template <class T>
	void LinkedList<T>::Delete(Node<T>* currentNode, Node<T>* prevNode)
	{
		if (prevNode == nullptr) 
		{
			if (currentNode->next == nullptr)
			{
				head = nullptr;
			}
			else
			{
				head = currentNode->next;
			}
		}
		else
		{
			prevNode->next = currentNode->next;
		}
	}

}


LinkedListAllocator::LinkedListAllocator(const size_t size)
{
	allocationSize = size;

	//Log ("[LinkedList Allocation] Allocation size :" +  allocationSize);
}

LinkedListAllocator::~LinkedListAllocator()
{
	free(startLocation);
	startLocation = nullptr;
}

void LinkedListAllocator::InitializeAllocator()
{
	if (startLocation != nullptr)
	{
		free(startLocation);
		startLocation = nullptr;
	}

	//allocate the memory of max allocated size in the beginning.
	startLocation = malloc(allocationSize);

	usedSize = 0;
	peakSize = 0;
	List::Node<size_t>* firstElem = (List::Node<size_t>*) startLocation;
	firstElem->next = nullptr;
	
	availableMemories.head = nullptr;
	availableMemories.Add(firstElem, nullptr);

	//Log ("[LinkedList Allocation] Allocating memory of size :" + allocationSize);
}

void* LinkedListAllocator::CMalloc(const size_t size, const size_t alignment)
{
	//We would need to allocate memory for desired size +  padding size to make sure alignment is being followed 
	
	//iterate through free memory list to find the best available memory for required size.
	List::Node<size_t>* bestNode = availableMemories.head;
	List::Node<size_t>* prevNode = nullptr;
	size_t padding;

	while (bestNode != nullptr)
	{
		//calculate padding.
		size_t headerSize = sizeof(AllocationHeader);
		size_t multiplier = ((size_t)bestNode / alignment) + 1; //number of alignment required for allocation size.
		padding = (multiplier * alignment) - size; //size needed for padding(empty memory) after actual allocation size.

		if (padding < headerSize) // if padding not sufficient enough to store header.
		{
			size_t requiredSize = headerSize - padding;
		
			if (requiredSize % alignment > 0) // mod is greater then zero or zero will imply that a new alignment size allocation (could be more then one) is required to store header.
			{
				padding += alignment * ((requiredSize / alignment) + 1);
			}
			else //(requiredSize / alignment) is the multiple of alignment size which is required to store header.
			{
				padding += alignment * (requiredSize / alignment);
			}
		}

		if (bestNode->data >= padding + size) //check if current node can contain required allocation size.
		{
			//Log ("[LinkedList Allocation] Best node for allocating memory for header size: " + headerSize );

			//Log ("[LinkedList Allocation] Required padding: " + padding);
			
			break;
		}

		prevNode = bestNode;
		bestNode = bestNode->next;
	}

	if (bestNode == nullptr)
	{
		//Log ("[LinkedList Allocation] List is out of free memory.");
		return nullptr;
	}

	if (bestNode->data > (size + padding))
	{
		size_t unusedMemory = bestNode->data - (size + padding);

		//need to free unused memory from best node and add it as a new node in available memory list.
		List::Node<size_t>* unusedMemNode = (List::Node<size_t>*) (bestNode->data + (size + padding));
		unusedMemNode->data = (size_t)unusedMemory;
		availableMemories.Add(unusedMemNode, bestNode);

		//Log ("[LinkedList Allocation] Total unused memory for alignment: " + unusedMemory);
	}

	// remove the free memory needed by the function from available memory list.
	availableMemories.Delete(bestNode, prevNode);

	//location of header to be placed. Needs to be at the start of the memory + extra memory for alignment from padding. 
	size_t headerAddr = (size_t)bestNode + (padding - sizeof(AllocationHeader));
	//Log ("[LinkedList Allocation] Header address: " + headerAddr);

	//location of actual memory for user will be placed: [location of the best node + extra memory for alignment + memory for header ]
	size_t userMemAddr = headerAddr + sizeof(AllocationHeader);
	//Log ("[LinkedList Allocation] Actual memory for user: " + userMemAddr);

	((AllocationHeader*)headerAddr)->size = size + padding;
	((AllocationHeader*)headerAddr)->padding = (padding - sizeof(AllocationHeader));

	usedSize += size + padding;

	peakSize = std::max(peakSize, usedSize);

	//Log ("[LinkedList Allocation] Total used memory: " + usedSize);
	
	return (void*)userMemAddr;
}

void LinkedListAllocator::CFree(void* memory)
{

}

