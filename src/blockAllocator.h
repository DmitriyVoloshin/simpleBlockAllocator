#ifndef _BLOCK_ALLOCATOR_H
#define _BLOCK_ALLOCATOR_H

//! \defgroup BlockAllocator
//! \brief Simple block allocator implementation.

//! @{
#include <stdint.h>
#include <mutex>

#include "blockAllocatorExceptions.h"

//! This class implements a simple thread-safe block memory allocator.
class BlockAllocator
{
protected:

	//! \brief The memory block implementation.
	struct Block
	{
		//! \brief Holds a pointer to the next block.
		Block* next;
	};

	//! \brief Allocatable block size, each allocated block has this size in bytes. Set in constructor.
	size_t blockSize = 0;

	//! \brief Block header size.
	size_t headerSize = 0;

	//! \brief The maximum number of blocks holds maximum blocks of memory the allocator can provide. Set in constructor.
	size_t maxBlocks = 0;

	//! \brief Allocator's first block header.
	char* startHeader = NULL;

	//! \brief Allocator's last block header.
	char* endHeader = NULL;

	//! \brief The pointer to currently free block. Allocation request returns this address.
	//! \sa Block
	Block* headHeader = NULL;

	//! \brief Checks if passed block address was allocated.
	//! \param[in] block The address of the block of interest.
	//! \return Returns true if the block was previously allocated.
	bool isBlockInUse(void* block) const noexcept;

	//! \brief Checks if passed block size and number of blocks can be allocated.
	//! \param[in] blockByteSize The block size in bytes.
	//! \param[in] numOfBlocks The number of blocks required.
	//! \return Returns true if the product of size and number of blocks is correct.
	bool isSizeCorrect(size_t blockByteSize, size_t numOfBlocks) const noexcept;

public:
	//! \brief Represents a memory pool category type.
	enum MemoryPoolType
	{
		//! Allocator uses internal memory pool.
		Internal,
		//! Allocator uses external memory pool.
		External
	};

	//! \brief BlockAllocator constructor.

	//! If invalid parameters were passed e.g. numOfBlocks=0 or size=0 the constructor will throw BlockAllocatorExceptions::InvalidConstructorParametersException.
	//! If no memoryPool is passed, will be created an internal memory pool inside the allocator.
	//! If internal poll exceeds allocatable from the system memory, the constructor will throw BlockAllocatorExceptions::OutOfSystemMemoryException.
	//! Trying to allocate invalid external memory will terminate the program without an exception.
	//! \warning If external memory pool address provided there's is no internal (in constructor or elsewhere) check if external poll can be modified(!).
	//! \warning Using external memory pool will require to prepare [(block size) + (allocator header size)] * (number of blocks) memory pool, a user must take header size into account.
	//! \param[in] blockByteSize A selected block size in bytes, must be greater than 0. Will be equated to BlockAllocator::minBlockSize() if less then it.
	//! \param[in] numOfBlocks A desired quantity of blocks, must be greater than 0.
	//! \param[in] memoryPool An address of an external memory pool.
	//! \throw BlockAllocatorExceptions::InvalidConstructorParametersException If invalid constructor parameters were passed.
	//! \throw BlockAllocatorExceptions::OutOfSystemMemoryException If no memory poll pointer was passed and system can't provide enough memory.
	//! ### Example
	//! ~~~~~~~~~~~~~~~~~~~~~~~.cpp
	//! size_t blockByteSize = 32;
	//!
	//! size_t numOfBlocks = 64;
	//!
	//! BlockAllocator ba {blockSize, numOfBlocks};
	//! ~~~~~~~~~~~~~~~~~~~~~~~
	BlockAllocator(size_t blockByteSize, size_t numOfBlocks, void* memoryPool = NULL);

	//! \brief Deleted copy constructor
	BlockAllocator(const BlockAllocator&) = delete;

	//! \brief Virtual destructor for inheritance purposes.
	virtual ~BlockAllocator();

	//! \brief Deleted move constructor.
	BlockAllocator(BlockAllocator&&) = delete;

	//! \brief Deleted assignment operator.
	BlockAllocator& operator=(BlockAllocator&) = delete;

	//! \brief Returns first free block address.

	//! \return Returns a pointer to a new block.
	//! \throw BlockAllocatorExceptions::OutOfSystemMemoryException Thrown if no more empty blocks are available.
	//! \attention A user should place try-catch block.

	//! ### Example
	//! ~~~~~~~~~~~~~~~~~~~~~~~.cpp
	//!
	//! void* block;
	//!
    //! try
	//! {
	//!		block = ba.allocate();
	//! }
	//! catch (const OutOfSystemMemoryException& e)
	//! {
	//!		...
	//! }
	//! ~~~~~~~~~~~~~~~~~~~~~~~
	void* allocate();

	//! \brief Tries to deallocate a block with passed address.

	//! \param[in] Block's address to deallocate.
	//! \throw BlockAllocatorExceptions::InvalidBlockAddressException Thrown if invalid block address is passed.
	//! \attention A user should place try-catch block.

	//! ### Example
	//! ~~~~~~~~~~~~~~~~~~~~~~~.cpp
	//! ...
	//!
	//! try
	//! {
	//! 	ba.deallocate(block);
	//! }
	//! catch (const InvalidBlockAddressException& e)
	//! {
	//! 	...
	//! }
	//! ~~~~~~~~~~~~~~~~~~~~~~~
	void deallocate(void* block);

	//! \brief Returns current block size.
	//! \return Allocators block size in bytes.
	size_t getBlockSize() const noexcept;

	//! \brief Returns current header size.
	//! \return Allocators header size in bytes.
	static size_t getHeaderSize() noexcept;

	//! \brief Checks passed block address.
	//! \param[in] block a pointer to the block of interest.
	//! \return Returns true if passed address is really this allocator's block address.
	bool isBlockAddress(void* block) const noexcept;

	//! \brief Gets current working pool type.
	//! \return Returns current working pool type as type of MemoryPoolType
	//! \sa MemoryPoolType
	MemoryPoolType getPoolType() const noexcept;

private:
	//! \brief Mutex instance used to synchronize multithread operations.
	std::mutex mutex;

	//! \brief Builds linked list of free blocks.
	void buildBlocksList();

	//! \brief Holds current working memory pool, set in the constructor.
	//! \sa MemoryPoolType
	MemoryPoolType poolType;

	//! \brief Holds a value to indicate a block is in use.
	//! \sa Block
	Block* blockInUseFlag;

	//! \brief Block with header size in bytes.
	size_t blockWithHeaderSize = 0;
};

//! @}
#endif

