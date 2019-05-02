# Custom-Allocators
---------------------------

This is a work in progress example of simple free list (linked list based) custom memory allocator.

Simple idea behind the list allocator is to keep a list of all the free/available memory in either a binary tree or linked list and give out the location of best possible node from the available list once an allocation is requested from the allocator.

## Steps in making the allocator:

* Start by creating a big allocation on heap using C malloc and storing it as the first element in the available list. 
* Each node in the available tree would have an header which would contain the size of the node.
* When user requests an allocation from the list allocator, simply calculate the size that would be needed aside from the allocation that is requested. 
* The total size needed would be the size requested, padding size for the alignment if required and the size of allocation header. (the total size should be a multiple of alignment size, if alignemnt is required.)
* Allocation header would contain meta data about the allocation made such as size of the allocation, size of the padding etc.   
* Final location for the allocation : [Location of best node + Extra padding memory to fulfill the alignment + memory for header]
* When the deallocation is required, we can trace back to the actual memeory location using header.
* When freeing the memory, We can merge any contigous memory that is left in the available memory list.


![Screen Capture](https://raw.githubusercontent.com/swastik1992/Custom-Allocators/master/Images/Capture.PNG)
