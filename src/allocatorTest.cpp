#include "CppUTest/TestHarness.h"

#include <exception>
#include <stdexcept>

#include "allocator.h"

TEST_GROUP(AllocatorTest)
{
	void setup()
	{
	}

	void teardown()
	{
	}
};

TEST(AllocatorTest, canCreateAnEmptyAllocator)
{
	CHECK_THROWS(float, throw 4);

	/*CHECK_THROWS({
	    try {
	    	throw std::runtime_error("testing...");
	    }
	    catch(std::out_of_range const & err) {
	        EXPECT_EQ(err.what(),std::string("Out of range"));
	    }
	    catch(...) {
	        FAIL() << "Expected std::out_of_range";
	    }


	}

	);*/

};
