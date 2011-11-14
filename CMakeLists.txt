cmake_minimum_required(VERSION 2.8)
if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of
    build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug
    Release RelWithDebInfo MinSizeRel.")
endif()
project(libsentences)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(re2c)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(libs "")

find_package(Unistring REQUIRED)
find_package(Boost)

set(libs "${libs};${UNISTRING_LIBRARY}")
include_directories("${UNISTRING_INCLUDE_DIR}")
include_directories(${Boost_INCLUDE_DIRS})

if(UNIX OR MINGW)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/libsentences")
re2c("${CMAKE_CURRENT_SOURCE_DIR}/libsentences/standard_tokenizer.rr.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/libsentences/standard_tokenizer.cpp")

add_library(sentences 
    libsentences/utf8_slice.cpp
    libsentences/memory_pool.cpp
    libsentences/utf8_iterator.cpp
    libsentences/text_sentences.cpp
    libsentences/quotes_detector.cpp
    libsentences/sbd_model.cpp
    libsentences/standard_tokenizer.cpp)

add_executable(sentence_splitter sentence_splitter.cpp)
target_link_libraries(sentence_splitter sentences ${libs})

add_executable(sbd_util sbd_util.cpp)
target_link_libraries(sbd_util sentences ${libs})

add_executable(eval_sbd eval_sbd.cpp)
target_link_libraries(eval_sbd sentences ${libs})
