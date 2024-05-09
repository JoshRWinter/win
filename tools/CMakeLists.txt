cmake_minimum_required(VERSION 3.0)
project(wintools)

set(CMAKE_CXX_STANDARD 17)

add_executable(roll roll/roll.cpp roll/roll.hpp roll/recipe/recipe.cpp roll/recipe/recipe.hpp roll/recipe/recipeparser.cpp roll/recipe/recipeparser.hpp)
add_executable(roll-recipe-tests roll/recipe/tests/test.hpp roll/recipe/tests/parsertests.cpp roll/recipe/recipeparser.cpp roll/recipe/recipeparser.hpp)

add_executable(atlasizer atlasizer/atlasizer.hpp atlasizer/main.cpp atlasizer/gui.cpp atlasizer/layoutexporter.cpp atlasizer/layoutexporter.hpp atlasizer/compileatlas.cpp)

if (NOGUI)
	add_compile_definitions(NOGUI)
else()
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../win ${CMAKE_CURRENT_BINARY_DIR}/win)
	target_link_libraries(atlasizer win)
endif()


if(WIN32)
	target_include_directories(roll PRIVATE ../win/include/zlib)
	target_link_libraries(roll PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../win/lib/zlib.lib)
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W1" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	endif()
else()
	find_package(ZLIB REQUIRED)

	target_link_libraries(roll ${ZLIB_LIBRARIES})
endif()

if (USE_ASAN)
	message("Enabling ASAN")

	target_compile_options(roll PRIVATE -fsanitize=address)
	target_link_options(roll PRIVATE -fsanitize=address)

	target_compile_options(roll-recipe-tests PRIVATE -fsanitize=address)
	target_link_options(roll-recipe-tests PRIVATE -fsanitize=address)

	target_compile_options(atlasizer PRIVATE -fsanitize=address)
	target_link_options(atlasizer PRIVATE -fsanitize=address)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		target_compile_options(roll PRIVATE -fsanitize=undefined)
		target_link_options(roll PRIVATE -fsanitize=undefined)

		target_compile_options(roll-recipe-tests PRIVATE -fsanitize=undefined)
		target_link_options(roll-recipe-tests PRIVATE -fsanitize=undefined)

		target_compile_options(atlasizer PRIVATE -fsanitize=undefined)
		target_link_options(atlasizer PRIVATE -fsanitize=undefined)
	endif()
endif()
