#include <limits>
#include <mutex>

#include "blockAllocator.h"

using namespace BlockAllocatorExceptions;

BlockAllocator::BlockAllocator(size_t size, size_t blocks, void* memoryPool) :
		blockSize(size), headerSize(sizeof(Block*)), maxBlocks(blocks)
{
	if (blockSize == 0 || maxBlocks == 0)
		throw InvalidConstructorParametersException();

	if (!isSizeCorrect(blockSize, maxBlocks))
		throw InvalidConstructorParametersException();

	blockWithHeaderSize = blockSize + headerSize;

	// Task doesn't specify how the memoryPool is set
	// if external pool isn't provided let's create a new one from the system
	if (memoryPool == NULL)
	{
		poolType = Internal;
		startHeader = (char*)malloc(blockWithHeaderSize * maxBlocks);

		if(startHeader == NULL)
			throw OutOfSystemMemoryException();
	}
	else
	{
		poolType = External;
		startHeader = (char*)memoryPool;
	}
	// It is assumed that a memory address of 0x1 can't be used by a user in any real system.
	// Let's use it as a flag to indicate that a block is currently in use.
	// Otherwise an independent bit flag can be used in the header.
	blockInUseFlag = (Block*)1;

	endHeader = startHeader + blockWithHeaderSize * (maxBlocks - 1);
	headHeader = (Block*)startHeader;

	buildBlocksList();
}

bool BlockAllocator::isSizeCorrect(size_t blockByteSize, size_t numOfBlocks) const noexcept
{
	size_t maxBlockWithHeaderSize = std::numeric_limits<size_t>::max() / numOfBlocks;

	if (maxBlockWithHeaderSize < getHeaderSize())
		return false;

	return blockByteSize <= maxBlockWithHeaderSize - headerSize;
}

void BlockAllocator::buildBlocksList()
{
	Block* block;

	for (char* i = startHeader; i < endHeader; i += blockWithHeaderSize)
	{
		block = (Block*)i;
		block->next = (Block*)(i + blockWithHeaderSize);
	}
	block = (Block*)endHeader;
	block->next = NULL;
}

// Task doesn't specify if we need to allocate multiple blocks at once.
// Let's choose not to allocate more then one block at once.
// Otherwise we'll need to hold used block size somehow.
// This will increase minimum block size if header is kept inside the block.
void* BlockAllocator::allocate()
{
	std::lock_guard<std::mutex> lock(mutex);
	if (headHeader == NULL)
	{
		throw OutOfAllocatableMemoryException();
	}

	Block* freeBlock = headHeader;
	headHeader = headHeader->next;
	freeBlock->next = blockInUseFlag;

	return (char*)freeBlock + getHeaderSize();
}

size_t BlockAllocator::getHeaderSize() noexcept
{
	return sizeof(Block*);
}

size_t BlockAllocator::getBlockSize() const noexcept
{
	return blockSize;
}

void BlockAllocator::deallocate(void* block)
{
	std::lock_guard<std::mutex> lock(mutex);
	if (!isBlockInUse(block))
	{
		throw InvalidBlockAddressException();
	}

	Block* header = (Block*)((char*)block - headerSize);

	header->next = headHeader;

	headHeader = header;
}

bool BlockAllocator::isBlockInUse(void* block) const noexcept
{
	if (!isBlockAddress(block))
		return false;

	Block* header = (Block*)((char*)block - headerSize);
	if (header->next == blockInUseFlag)
		return true;

	return false;
}

bool BlockAllocator::isBlockAddress(void* block) const noexcept
{
	char* header = (char*)block - headerSize;

	if (block == NULL || header > endHeader || header < startHeader)
		return false;

	size_t reminder = (size_t)(header - startHeader) % blockWithHeaderSize;

	if (reminder == 0)
		return true;

	return false;
}

BlockAllocator::~BlockAllocator()
{
	if (poolType == Internal && startHeader != NULL)
	{
		std::free(startHeader);
	}
}

BlockAllocator::MemoryPoolType BlockAllocator::getPoolType() const noexcept
{
	return poolType;
}
