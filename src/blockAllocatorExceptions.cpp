#include "blockAllocatorExceptions.h"

using namespace BlockAllocatorExceptions;

IException::IException() :
		message(" ")
{}

IException::IException(std::string msg) :
		message(msg)
{}

IException::~IException() = default;

const char* IException::what() const noexcept
{
	return message.c_str();
}

OutOfSystemMemoryException::OutOfSystemMemoryException() :
		IException("Can't acquire enough memory from the system!")
{}


InvalidConstructorParametersException::InvalidConstructorParametersException() :
		IException("Invalid constructor parameters passed!")
{}

OutOfAllocatableMemoryException::OutOfAllocatableMemoryException() :
		IException("Out of free memory at pool exception!")
{}

InvalidBlockAddressException::InvalidBlockAddressException() :
		IException("Invalid block address exception!")
{}
