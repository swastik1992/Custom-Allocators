#pragma once
#include <stdlib.h>
#define  Log(msg)	std::cout<< msg << std::endl

namespace List
{
	template <class T>
	struct Node
	{
		T data;
		Node* next;
	};

	template <class T>
	class LinkedList
	{
	public:

		LinkedList();

		void Add(Node<T>* currentNode, Node<T>* prevNode);
		void Delete(Node<T>* currentNode, Node<T>* prevNode);

		Node<T>* head;
	};

}


struct AllocationHeader
{
	size_t size;
	size_t padding;
};

struct AvailableMemoryHeader
{
	size_t size;
};

class LinkedListAllocator
{
	
public:
	LinkedListAllocator(const size_t size);
	~LinkedListAllocator();

	void InitializeAllocator();

	void* CMalloc(const size_t size, const size_t alignment);

	void CFree(void* memory);
 

private:

	void* startLocation;

	List::LinkedList<AvailableMemoryHeader> availableMemories;

	size_t allocationSize;

	size_t usedSize;

	size_t peakSize;

	void DebugPrintAvailableList();
};

