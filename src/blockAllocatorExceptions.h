#ifndef _BLOCK_ALLOCATOR_EXCEPTIONS_H
#define _BLOCK_ALLOCATOR_EXCEPTIONS_H


//! \addtogroup BlockAllocator
//! @{
#include <exception>
#include <string>

namespace BlockAllocatorExceptions
{
//! \brief The basic exception interface.

//! Used to implement other BlockAllocatorExceptions exceptions.
class IException : public std::exception
{
public:
	//! \brief Method to get exception's message.

	//! \return Returns a C-string with message.
	const char* what() const noexcept final;

private:
	//! \brief Holds exception's message.
    std::string message;

protected:
    //! \brief Simple constructor.
    IException();
    //! \brief Simple constructor with message.
    //! \param[in] msg Message inside the exception.
    IException(std::string msg);
    //! \brief Pure virtual destructor.
    virtual ~IException() = 0;
};

//! \brief The out of system memory exception.

//! Thrown when an allocator can't get enough system memory from the OS to construct a memory pool.
class OutOfSystemMemoryException : public IException
{
public:
	//! \brief The constructor.
	OutOfSystemMemoryException();
	//! \brief The default destructor.
	~OutOfSystemMemoryException() = default;
};


//! \brief The invalid constructor parameters exception.

//! Thrown when invalid constructor BlockAllocator(size_t size, size_t blocks, void* pool) parameters are passed.
class InvalidConstructorParametersException : public IException
{
public:
	//! \brief The constructor.
	InvalidConstructorParametersException();
	//! \brief The default destructor.
	~InvalidConstructorParametersException() = default;
};

//! \brief The out of allocatable memory exception.

//! Thrown when allocator's memory pool becomes totally filled. It's not possible to allocate more blocks.
class OutOfAllocatableMemoryException : public IException
{
public:
	//! \brief The constructor.
	OutOfAllocatableMemoryException();
	//! \brief The default destructor.
	~OutOfAllocatableMemoryException() = default;
};

//! \brief The invalid block address exception.

//! Thrown when invalid block address is passed to be deallocated.
class InvalidBlockAddressException : public IException
{
public:
	//! \brief The constructor.
	InvalidBlockAddressException();
	//! \brief The default destructor.
	~InvalidBlockAddressException() = default;
};

}

//! @}

#endif
