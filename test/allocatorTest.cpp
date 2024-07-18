#include "CppUTest/TestHarness.h"

#include <limits>
#include <thread>
#include <iostream>
#include <list>
#include <vector>
#include <algorithm>
#include <atomic>

#include "../src/blockAllocator.h"

using namespace BlockAllocatorExceptions;

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
TEST_GROUP(AllocatorCreation)
{
	int numOfBlocks = 10;

    void setup()
    {
    }
    void teardown()
    {
	}
};

static void createAllocator(size_t size, size_t blocks)
{
	BlockAllocator ba {size, blocks};
}

TEST(AllocatorCreation, zeroBlockCreationThrowsInvalidParams)
{
	CHECK_THROWS(InvalidConstructorParametersException, createAllocator(1, 0));
}

TEST(AllocatorCreation, invalidParamsMessageEquals)
{
	std::string actual;
	try
	{
		createAllocator(1, 0);
	}
	catch (const InvalidConstructorParametersException& e)
	{
		actual = e.what();
	}
	STRCMP_EQUAL("Invalid constructor parameters passed!", actual.c_str());
}

TEST(AllocatorCreation, zeroSizeCreationThrowsInvalidParams)
{
	CHECK_THROWS(InvalidConstructorParametersException,	createAllocator(0, 1));
}

TEST(AllocatorCreation, zeroBlocksAndSizeCreationThrowsInvalidParams)
{
	CHECK_THROWS(InvalidConstructorParametersException, createAllocator(0, 0));
}

TEST(AllocatorCreation, tooBigBlocksSizeThrowsInvalidParams)
{
	CHECK_THROWS(InvalidConstructorParametersException, createAllocator(std::numeric_limits<size_t>::max(), 2));
}

TEST(AllocatorCreation, tooBigNumOfBlocksThrowsInvalidParams)
{
	CHECK_THROWS(InvalidConstructorParametersException, createAllocator(2, std::numeric_limits<size_t>::max()));
}

TEST(AllocatorCreation, requestOverSystemMemoryAvailableThrowsOutOfSystemMemoryException)
{
	CHECK_THROWS(OutOfSystemMemoryException, createAllocator(std::numeric_limits<size_t>::max() - 1000, 1));
}

TEST(AllocatorCreation, OutOfSystemMemoryMessageEquals)
{
	std::string actual;
	try
	{
		createAllocator(std::numeric_limits<size_t>::max() - 1000, 1);
	}
	catch (const OutOfSystemMemoryException& e)
	{
		actual = e.what();
	}
	STRCMP_EQUAL("Can't acquire enough memory from the system!", actual.c_str());
}

TEST(AllocatorCreation, ifNoMemoryPoolIsSpecifiedCreatesInternalAllocator)
{
	BlockAllocator ba {2, 2};

	LONGS_EQUAL(BlockAllocator::Internal, ba.getPoolType());
}

TEST(AllocatorCreation, canGetBlockSize)
{
	size_t blockSize = 32;
	BlockAllocator ba {blockSize, 2};

	LONGS_EQUAL(blockSize, ba.getBlockSize());
}


//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
TEST_GROUP(Allocation)
{
	size_t numOfBlocks = 4;

	const char minChar = std::numeric_limits<char>::min();
	const char maxChar = std::numeric_limits<char>::max();

    void setup()
    {
    }
    void teardown()
    {
    }
};

TEST(Allocation, canAllocateOneByte)
{
	BlockAllocator ba {sizeof(char), 1};

	char expected = 125;

	char* actual = (char*)ba.allocate();
	*actual = expected;

	LONGS_EQUAL(expected, *actual);
}

TEST(Allocation, canAllocateTwoBytes)
{
	BlockAllocator ba {sizeof(char), numOfBlocks};

	char* first = (char*)ba.allocate();
	char* second = (char*)ba.allocate();

	*first = minChar;
	*second = maxChar;

	LONGS_EQUAL(minChar, *first);
	LONGS_EQUAL(maxChar, *second);
}

TEST(Allocation, addressRangeBetweenTwoBytesEqualsBlockPlusHeaderSize)
{
	size_t blockSize = sizeof(char);
	BlockAllocator ba {blockSize, numOfBlocks};

	char* first = (char*)ba.allocate();
	char* second = (char*)ba.allocate();

	size_t expected = blockSize + ba.getHeaderSize();

	LONGS_EQUAL(expected, second - first);
}


TEST(Allocation, multipleAllocationsAddressDifferenceEqualsBlockSize)
{
	size_t blockSize = 64;
	BlockAllocator ba {blockSize, numOfBlocks};

	size_t expected = blockSize + ba.getHeaderSize();

	char* first = (char*)ba.allocate();
	char* second = (char*)ba.allocate();
	char* third = (char*)ba.allocate();

	LONGS_EQUAL(expected, third - second);
	LONGS_EQUAL(expected, second - first);
}

TEST(Allocation, lastBlockAddressIsCorrect)
{
	size_t blockSize = 32;
	BlockAllocator ba {blockSize, numOfBlocks};

	size_t expected = blockSize + ba.getHeaderSize();

	char* first = (char*)ba.allocate();
	ba.allocate();
	ba.allocate();
	char* fourth = (char*)ba.allocate();

	LONGS_EQUAL(first + expected * (numOfBlocks - 1), fourth);
}


TEST(Allocation, canUseWithMoreThenOneByteSizeVariablesBoundsCheck)
{
	BlockAllocator ba {sizeof(unsigned long), numOfBlocks};

	unsigned long* first = (unsigned long*)ba.allocate();
	unsigned long* second = (unsigned long*)ba.allocate();
	unsigned long* third = (unsigned long*)ba.allocate();

	*first = 0x7FFFFFFFFFFFFFFE;
	*second = 0xFFFFFFFFFFFFFFFF;
	*third = 0x7FFFFFFFFFFFFFFE;

	UNSIGNED_LONGS_EQUAL(*first, 0x7FFFFFFFFFFFFFFE);
	UNSIGNED_LONGS_EQUAL(*second, 0xFFFFFFFFFFFFFFFF);
	UNSIGNED_LONGS_EQUAL(*third, 0x7FFFFFFFFFFFFFFE);
}



//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
TEST_GROUP(AllocationException)
{
	const size_t numOfBlocks = 4;
	const size_t blockSize = 20;

    void setup()
    {
    }
    void teardown()
    {
	}
};

static void FillAllocator(BlockAllocator& ba, size_t numOfBlocks)
{
	for(size_t i = 0; i < numOfBlocks; i++)
	{
		ba.allocate();
	}
}

TEST(AllocationException, ifAllMemoryWasAllocatedThrowOutOfAllocatableMemoryException)
{
	BlockAllocator ba {blockSize, numOfBlocks};

	FillAllocator(ba, numOfBlocks);

	CHECK_THROWS(OutOfAllocatableMemoryException, ba.allocate());
}


//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
class AllocatorSpy : public BlockAllocator
{
public:
	AllocatorSpy(size_t blockByteSize, size_t numOfBlocks) :
		BlockAllocator(blockByteSize, numOfBlocks)
	{}
	~AllocatorSpy() = default;

	void* getStart()
	{
		return startHeader;
	}

	void* getEnd()
	{
		return endHeader;
	}

	void* getFirstBlock()
	{
		return (char*)startHeader + headerSize;
	}

	void* getLastBlock()
	{
		return (char*)endHeader + headerSize;
	}

	bool isUsed(void* block)
	{
		return isBlockInUse(block);
	}
};

TEST_GROUP(Deallocation)
{
	size_t numOfBlocks = 4;
	size_t blockSize = 16;

	BlockAllocator* ba;
	AllocatorSpy* as;
	void* start = NULL;
	void* end = NULL;
	void* firstBlock = NULL;
	void* lastBlock = NULL;

    void setup()
    {
    	ba = new AllocatorSpy(blockSize, numOfBlocks);

    	as = dynamic_cast<AllocatorSpy*>(ba);
    	start = as->getStart();
    	end = as->getEnd();
    	firstBlock = as->getFirstBlock();
		lastBlock = as->getLastBlock();
    }
    void teardown()
    {
    	as = nullptr;
    	delete ba;
	}
};

TEST(Deallocation, nullAddressBelowTheRangeThrowsInvalidBlockStartException)
{
	CHECK_THROWS(InvalidBlockAddressException, ba->deallocate(NULL));
}

TEST(Deallocation, afterAllocatorIsFilledNullDeallocationThrowsAnException)
{
	ba->allocate();
	ba->allocate();

	CHECK_THROWS(InvalidBlockAddressException, ba->deallocate(NULL));
}

TEST(Deallocation, invalidAddressRightAfterStartPlusHeaderThrows)
{
	char* invalidBlock = (char*)firstBlock + 1;
	CHECK_THROWS(InvalidBlockAddressException, ba->deallocate(invalidBlock));
}

TEST(Deallocation, invalidAddressRightBeforeStartPlusHeaderThrows)
{
	char* invalidBlock = (char*)firstBlock - 1;
	CHECK_THROWS(InvalidBlockAddressException, ba->deallocate(invalidBlock));
}

TEST(Deallocation, invalidAddressRightBeforeEndPlusHeaderThrows)
{
	char* invalidBlock = (char*)lastBlock - 1;
	CHECK_THROWS(InvalidBlockAddressException, ba->deallocate(invalidBlock));
}

TEST(Deallocation, invalidAddressRightAfterEndPlusHeaderThrows)
{
	char* invalidBlock = (char*)lastBlock + 1;
	CHECK_THROWS(InvalidBlockAddressException, ba->deallocate(invalidBlock));
}

TEST(Deallocation, invalidAddressFromValidRangeIsNotBlockAddress)
{
	char* invalidBlock = (char*)firstBlock + blockSize - 1;
	CHECK_FALSE(ba->isBlockAddress(invalidBlock));
}

TEST(Deallocation, startAddressPlusHeaderIsABlockAddress)
{
	CHECK_TRUE(ba->isBlockAddress(firstBlock));
}

TEST(Deallocation, endPlusHeaderIsABlockAddress)
{
	CHECK_TRUE(ba->isBlockAddress(lastBlock));
}

TEST(Deallocation, validAdressIsABlockAddress)
{
	char* validBlock = (char*)end - ba->getBlockSize();
	CHECK_TRUE(ba->isBlockAddress(validBlock));
}

TEST(Deallocation, allBlocksInRangeAreCorrectBlocks)
{
	for (char* i = (char*)firstBlock; i <= (char*)lastBlock; i+= blockSize + ba->getHeaderSize())
	{
		CHECK_TRUE(ba->isBlockAddress(i));
	}
}

TEST(Deallocation, unusedBlockIsNotInUse)
{
	CHECK_FALSE(as->isUsed(firstBlock));
}

TEST(Deallocation, canCheckIfBlockIsInUse)
{
	void* first = ba->allocate();

	CHECK_TRUE(as->isUsed(first));
}

TEST(Deallocation, invalidBlockIsNotInUse)
{
	char* invalidBlock = (char*)firstBlock + blockSize + 1;
	CHECK_FALSE(as->isUsed(invalidBlock));
}

TEST(Deallocation, validAddressTwiceThrows)
{
	char* block = (char*)ba->allocate();
	ba->deallocate(block);
	CHECK_THROWS(InvalidBlockAddressException, ba->deallocate(block));
}

TEST(Deallocation, deallocatedBlockCanBeReallocated)
{
	void* first = ba->allocate();

	ba->deallocate(first);

	void* second = ba->allocate();

	LONGS_EQUAL(first, second);
}

TEST(Deallocation, allocationReturnsPreviouslyDeallocatedBlock)
{
	void* first = ba->allocate();
	ba->allocate();

	ba->deallocate(first);

	void* newFirst = ba->allocate();

	LONGS_EQUAL(first, newFirst);
}

TEST(Deallocation, fillFreeAndGetTheLastBlock)
{
	FillAllocator(*ba, numOfBlocks);

	for (char* i = (char*)firstBlock; i <= (char*)lastBlock; i += blockSize + ba->getHeaderSize())
	{
		ba->deallocate(i);
	}

	void* last = ba->allocate();

	LONGS_EQUAL((char*)lastBlock, last);
}

TEST(Deallocation, fillFreeInReverseAndGetTheFirstBlock)
{
	FillAllocator(*ba, numOfBlocks);

	for (char* i = (char*)lastBlock ; i >= (char*)firstBlock; i -= blockSize + ba->getHeaderSize())
	{
		ba->deallocate(i);
	}

	void* first = ba->allocate();

	LONGS_EQUAL(firstBlock, first);
}



//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
TEST_GROUP(ExternalPool)
{
	size_t numOfBlocks = 2;
	size_t blockSize = 32;

	size_t fullBlockSize = blockSize + BlockAllocator::getHeaderSize();

	char* pool;

    void setup()
    {
    	pool = (char*)malloc(fullBlockSize * numOfBlocks);
    }
    void teardown()
    {
    	free(pool);
	}
};

TEST(ExternalPool, ifMemoryPoolIsSpecifiedCreatesAnExternalAllocator)
{
	BlockAllocator ba {blockSize, numOfBlocks, pool};
	BlockAllocator::MemoryPoolType actual = ba.getPoolType();

	LONGS_EQUAL(BlockAllocator::External, actual);
}

TEST(ExternalPool, destructorDoesntTryToDeleteExternalMemory)
{
	BlockAllocator* ba = new BlockAllocator(blockSize, numOfBlocks, pool);

	*pool = 20;

	delete ba;

	char actual = *pool;

	LONGS_EQUAL(20, actual);
}

TEST(ExternalPool, canAllocateExternalPool)
{
	BlockAllocator ba(blockSize, numOfBlocks, pool);

	void* actual1 = ba.allocate();
	void* actual2 = ba.allocate();

	char* expected1 = pool + ba.getHeaderSize();
	char* expected2 = expected1 + fullBlockSize;

	LONGS_EQUAL(expected1, actual1);
	LONGS_EQUAL(expected2, actual2);
}


//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
static std::atomic<int> threadsComplete;

TEST_GROUP(ThreadSafety)
{
	size_t numOfBlocks = 10;
	size_t blockSize = 64;
	size_t fullBlockSize = 0;

	BlockAllocator* ba;
	AllocatorSpy* as;

	void* start = NULL;
	void* end = NULL;
	void* firstBlock = NULL;
	void* lastBlock = NULL;

	std::list<void*> aquired1Bocks;
	std::list<void*> aquired2Bocks;
	std::list<void*> aquired3Bocks;

	void* block1;
	void* block2;
	void* block3;
	void* block4;

    void setup()
    {
    	block1 = (void*)0;
    	block2 = (void*)0;
    	block3 = (void*)0;
    	block4 = (void*)0;

    	ba = new AllocatorSpy(blockSize, numOfBlocks);
    	as = dynamic_cast<AllocatorSpy*>(ba);

    	fullBlockSize = blockSize + ba->getHeaderSize();

    	start = as->getStart();
    	end = as->getEnd();

    	firstBlock = as->getFirstBlock();
		lastBlock = as->getLastBlock();

    	aquired1Bocks.clear();
    	aquired2Bocks.clear();
    	aquired3Bocks.clear();
    	threadsComplete.store(0);
    }
    void teardown()
    {
    	as = nullptr;
    	delete ba;
	}
};


static void getBlock(BlockAllocator* ba, void** block)
{
	*block = ba->allocate();
	++threadsComplete;
}

TEST(ThreadSafety, twoThreadsCanGetAnAddressSimultaneously)
{
	std::thread th1(getBlock, ba, &block1);
	std::thread th2(getBlock, ba, &block2);

	th1.join();
	th2.join();

	void* expected1 = firstBlock;
	void* expected2 = (char*)firstBlock + fullBlockSize;

	while (threadsComplete.load() != 2)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}
	CHECK_FALSE(block1 == block2);
	CHECK_TRUE(block1 == expected1 || block2 == expected1);
	CHECK_TRUE(block1 == expected2 || block2 == expected2);
}



static void get2Blocks(BlockAllocator* ba, void** block1, void** block2, const char* msg)
{
	*block1 = ba->allocate();
	*block2 = ba->allocate();
}

static void release2Blocks(BlockAllocator* ba, void** block1, void** block2, const char* msg)
{
	ba->deallocate(*block1);
	ba->deallocate(*block2);
}

static void multipleAlloctionsAndDeallocations(int num, BlockAllocator* ba, void** block1, void** block2)
{
	for (int i = 0; i < num; i++)
	{
		get2Blocks(ba, block1, block2, "");
		std::this_thread::sleep_for(std::chrono::microseconds(100));
		release2Blocks(ba, block1, block2, "");
	}
	++threadsComplete;
}

static void makeASafeException(BlockAllocator* ba)
{
	for (int i = 0; i < 40; i++)
	{
		try
		{
			ba->deallocate(NULL);
		}
		catch(const InvalidBlockAddressException& e)
		{
			//Do nothing
		}
		std::this_thread::sleep_for(std::chrono::microseconds(100));
	}
	++threadsComplete;
}

TEST(ThreadSafety, catchingAnExceptionDoesntLockTheAllocator)
{
	threadsComplete.store(0, std::memory_order_seq_cst);

	std::thread th1(multipleAlloctionsAndDeallocations, 100, ba, &block1, &block2);
	std::thread th2(multipleAlloctionsAndDeallocations, 100, ba, &block3, &block4);
	std::thread th3(makeASafeException, ba);

	th1.join();
	th3.join();
	th2.join();

	while (threadsComplete.load() != 3)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(1000));
	}
}


//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
TEST_GROUP(MultithreadWork)
{
	size_t numOfBlocks = 258;
	size_t blockSize = 64;

	std::list<void*> aquired1Blocks;
	std::list<void*> aquired2Blocks;
	std::list<void*> aquired3Blocks;
	std::list<void*> aquired4Blocks;
	std::list<void*> aquired5Blocks;
	std::list<void*> aquired6Blocks;

	std::vector<void*> sum;

    void setup()
    {
    }
    void teardown()
    {
	}

    bool isDuplicateFound()
    {
    	sum.insert(sum.end(), aquired1Blocks.begin(), aquired1Blocks.end());
		sum.insert(sum.end(), aquired2Blocks.begin(), aquired2Blocks.end());
		sum.insert(sum.end(), aquired3Blocks.begin(), aquired3Blocks.end());
		sum.insert(sum.end(), aquired4Blocks.begin(), aquired4Blocks.end());
		sum.insert(sum.end(), aquired5Blocks.begin(), aquired5Blocks.end());
		sum.insert(sum.end(), aquired6Blocks.begin(), aquired6Blocks.end());

    	std::sort(sum.begin(), sum.end());

    	const auto duplicateIterator = std::adjacent_find(sum.begin(), sum.end());

    	return  duplicateIterator != sum.end();
    }

    bool isSumSizeCorrect()
    {
    	return sum.size() == numOfBlocks;
    }
};

    void getBlocks(BlockAllocator* ba, std::list<void*>* blocks, int num)
    {
    	while(blocks->size() != static_cast<size_t>(num))
    	{
    		try
    		{
    			void* block = ba->allocate();
				blocks->push_back(block);
    		}
    		catch(const OutOfAllocatableMemoryException& e)
    		{
    			std::cout << e.what();
    		}

    		std::this_thread::sleep_for(std::chrono::microseconds(100));
    	}
    }

    void releaseBlocks(BlockAllocator* ba, std::list<void*>* blocks)
    {
    	while (!blocks->empty())
    	{
    		try
    		{
    			void* blockToDeallocate = blocks->front();
    			ba->deallocate(blockToDeallocate);
    			blocks->pop_front();
    		}
    		catch(const InvalidBlockAddressException& e)
    		{
    			std::cout << e.what();
    		}

    		std::this_thread::sleep_for(std::chrono::microseconds(10));
    	}
    }

    void getAndRealeasBlocks(BlockAllocator* ba, std::list<void*>* blocks, int numOfBlocks)
    {
		getBlocks(ba, blocks, numOfBlocks);
		releaseBlocks(ba, blocks);

    	threadsComplete++;

    	while (threadsComplete.load() != 6)
    	{
    		std::this_thread::sleep_for(std::chrono::microseconds(100));
    	}
    	getBlocks(ba, blocks, numOfBlocks);
    	threadsComplete++;
    }


// This is some kind of integration test, should not be really used in real environment.
// This test can take quite some time usually less then 1 second. Rarely near 2 seconds.
TEST(MultithreadWork, canFillAllocatorCallAllocationExceptionReleaseAndAllocateAgain)
{
	BlockAllocator* ba = new BlockAllocator(blockSize, numOfBlocks);

	threadsComplete.store(0, std::memory_order_seq_cst);

	int numOfBlocks = 43;

	std::thread th1(getAndRealeasBlocks, ba, &aquired1Blocks, numOfBlocks);
	std::thread th2(getAndRealeasBlocks, ba, &aquired2Blocks, numOfBlocks);
	std::thread th3(getAndRealeasBlocks, ba, &aquired3Blocks, numOfBlocks);
	std::thread th4(getAndRealeasBlocks, ba, &aquired4Blocks, numOfBlocks);
	std::thread th5(getAndRealeasBlocks, ba, &aquired5Blocks, numOfBlocks);
	std::thread th6(getAndRealeasBlocks, ba, &aquired6Blocks, numOfBlocks);

	th1.join();
	th2.join();
	th3.join();
	th4.join();
	th5.join();
	th6.join();

	while (threadsComplete.load() != 12)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(100));
	}

	CHECK_FALSE(isDuplicateFound());
	CHECK_TRUE(isSumSizeCorrect());

	delete ba;
}
