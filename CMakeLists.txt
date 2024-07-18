cmake_minimum_required(VERSION 3.16)

project(blockAllocatorModule)

#Setup Doxygen API docs generation
find_package(Doxygen)
if (DOXYGEN_FOUND)
	set(DOXYGEN_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile )
	
	add_custom_target(doxyDocs ALL
			COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_CONFIG}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMENT "Generating API documentation with Doxygen"
			VERBATIM)
else (DOXYGEN_FOUND)
	message("Can't find Doxygen in the system, please install Doxygen to generate documentation")
endif (DOXYGEN_FOUND)

set(TEST_EXE_NAME tests)

add_subdirectory(src)
add_subdirectory(test)

