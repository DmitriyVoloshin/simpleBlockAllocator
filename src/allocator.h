#ifndef ALLOCATOR_H
#define ALLOCATOR_H


class Allocator
{
public:
	Allocator(int blockSize, int blocksNumber);

	void* allocate(int size);
	void free(void* pointer);



private:
	void* memoryStart;



};






#endif
