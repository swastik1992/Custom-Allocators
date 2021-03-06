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

	Log ("[LinkedList Allocation] Allocation size :" <<  allocationSize << std::endl);
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
	List::Node<AvailableMemoryHeader>* firstElem = (List::Node<AvailableMemoryHeader>*) startLocation;
	firstElem->data.size = allocationSize;
	firstElem->next = nullptr;
	
	availableMemories.head = nullptr;
	availableMemories.Add(firstElem, nullptr);

	Log ("[LinkedList Allocation] InitializeAllocator() Allocating memory of size :" << allocationSize << std::endl);
}

void* LinkedListAllocator::CMalloc(const size_t size, const size_t alignment)
{
	Log("[LinkedList Allocation] Memory left in the free list before allocation :");
	DebugPrintAvailableList();

	Log(std::endl << "[LinkedList Allocation] CMalloc() for size: " << size << " with alignment: " << alignment);

	//We would need to allocate memory for desired size +  padding size to make sure alignment is being followed 
	
	//iterate through free memory list to find the best available memory for required size.
	List::Node<AvailableMemoryHeader>* bestNode = availableMemories.head;
	List::Node<AvailableMemoryHeader>* prevNode = nullptr;
	size_t padding;

	while (bestNode != nullptr)
	{
		//calculate padding.
		size_t headerSize = sizeof(AllocationHeader);
		size_t multiplier = (size / alignment) + 1; //number of alignment required for allocation size.
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

		if (bestNode->data.size >= padding + size) //check if current node can contain required allocation size.
		{
			Log("[LinkedList Allocation] CMalloc() Allocation header size: " << sizeof(AllocationHeader));

			Log ("[LinkedList Allocation] CMalloc() Required padding (memory for header + memory required for alignemnt): " << padding);
			
			break;
		}

		prevNode = bestNode;
		bestNode = bestNode->next;
	}

	if (bestNode == nullptr)
	{
		Log ("[LinkedList Allocation] CMalloc() List is out of free memory.");
		return nullptr;
	}

	if (bestNode->data.size > (size + padding))
	{
		size_t unusedMemory = bestNode->data.size - (size + padding);

		//need to free unused memory from best node and add it as a new node in available memory list.
		List::Node<AvailableMemoryHeader>* unusedMemNode = (List::Node<AvailableMemoryHeader>*) ((size_t)bestNode + (size + padding));
		unusedMemNode->data.size = (size_t)unusedMemory;

		availableMemories.Add(unusedMemNode, bestNode);

		Log ("[LinkedList Allocation] CMalloc() Total remaining memory left from block after (allocation size + padding) : " << unusedMemory);
	}

	// remove the free memory needed by the function from available memory list.
	availableMemories.Delete(bestNode, prevNode);

	Log("[LinkedList Allocation] CMalloc() Location of found memory node : " << (size_t)bestNode);

	//location of header to be placed. Needs to be at the start of the memory + extra memory for alignment from padding. 
	size_t headerAddr = (size_t)bestNode + (padding - sizeof(AllocationHeader));
	Log ("[LinkedList Allocation] CMalloc() Header address: " << headerAddr);

	//location of actual memory for user will be placed: [location of the best node + extra memory for alignment + memory for header ]
	size_t userMemAddr = headerAddr + sizeof(AllocationHeader);
	Log ("[LinkedList Allocation] CMalloc() Actual memory for user: " << userMemAddr);

	((AllocationHeader*)headerAddr)->size = size + padding;
	((AllocationHeader*)headerAddr)->padding = (padding - sizeof(AllocationHeader));
	Log("[LinkedList Allocation] CMalloc() Final block size in saved in header: " << ((AllocationHeader*)headerAddr)->size);
	Log("[LinkedList Allocation] CMalloc() Final padding size saved in header: " << (size_t) ((AllocationHeader*)headerAddr)->padding);

	usedSize += size + padding;

	peakSize = std::max(peakSize, usedSize);

	Log ("[LinkedList Allocation] CMalloc() Total used memory: " << usedSize << std::endl);
	
	Log("[LinkedList Allocation] Memory left in the free list after allocation :");
	DebugPrintAvailableList();

	return (void*)userMemAddr;
}

void LinkedListAllocator::CFree(void* memory)
{
	Log("[LinkedList Allocation] Memory left in the free list before deallocation :");
	DebugPrintAvailableList();


	size_t memoryAddrs = (size_t)memory;
	Log(std::endl <<"[LinkedList Allocation] CFree() Location of address to be freed: " << memoryAddrs);

	size_t headerAddrs = memoryAddrs - sizeof(AllocationHeader);
	Log("[LinkedList Allocation] CFree() Size of header: " << sizeof(AllocationHeader));
	Log("[LinkedList Allocation] CFree() Location of header: " << headerAddrs);

	AllocationHeader * allocationHeader{ (AllocationHeader *)headerAddrs };
	Log("[LinkedList Allocation] CFree() Size of block to be freed: " << allocationHeader->size);
	Log("[LinkedList Allocation] CFree() Size of padding : " << (size_t)allocationHeader->padding);

	List::Node<AvailableMemoryHeader>* node = (List::Node<AvailableMemoryHeader>*)(headerAddrs - allocationHeader->padding);
	node->data.size = allocationHeader->size;
	node->next = nullptr;

	Log("[LinkedList Allocation] CMalloc() Location of final free node : " << (size_t) node);

	List::Node<AvailableMemoryHeader>* iterator = availableMemories.head;
	List::Node<AvailableMemoryHeader>* prevNode = nullptr;

	while (iterator != nullptr)
	{
		if (memory < iterator)
		{
			availableMemories.Add(node, prevNode);
			break;
		}
		prevNode = iterator;
		iterator = iterator->next;
	}

	usedSize -= node->data.size;
	Log("[LinkedList Allocation] CMalloc() Total used memory: " << usedSize);
	
	Log(std::endl << "[LinkedList Allocation] Memory left in the free list after deallocation :");
	DebugPrintAvailableList();

	if (node->next != nullptr && (size_t)node + node->data.size == (size_t)node->next)
	{
		node->data.size += node->next->data.size;
		availableMemories.Delete(node->next, node);
	}

	if (prevNode != nullptr && (size_t)prevNode + prevNode->data.size == (size_t)node)
	{
		prevNode->data.size += node->data.size;
		availableMemories.Delete(node, prevNode);
	}
	
	Log(std::endl << "[LinkedList Allocation] Memory left in the free list after merging contigous memory :");
	DebugPrintAvailableList();
}

void LinkedListAllocator::DebugPrintAvailableList()
{
	List::Node<AvailableMemoryHeader>* it = availableMemories.head;
	int index = 0;
	while (it != nullptr)
	{
		Log("Memory at index: " << index << " , Size of the free memory: " << it->data.size << ", Location of memory: " << (size_t)it);
		index++;

		it = it->next;
	}
}



