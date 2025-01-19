cmake_minimum_required(VERSION 3.25)

project(FuzzyPSI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(-O0)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif ()

if (CMAKE_BUILD_TYPE MATCHES Release)
    add_compile_options(-O2)
endif ()

add_compile_options(-Wall)
add_compile_options(-g -ggdb)

file(GLOB SOURCES "src/*/*.cpp" "src/*/*.hpp" "src/*.cpp" "src/*.hpp")
add_executable(fuzzypsi ${SOURCES})


find_package(libOTe REQUIRED
        PATHS ./libOTe
        NO_DEFAULT_PATH
        NO_CMAKE_ENVIRONMENT_PATH
        NO_CMAKE_PATH
        NO_SYSTEM_ENVIRONMENT_PATH
        NO_CMAKE_SYSTEM_PATH
        NO_CMAKE_FIND_ROOT_PATH)
target_link_libraries(fuzzypsi oc::libOTe)


find_package(OpenSSL REQUIRED)

if (OPENSSL_FOUND)
    message(STATUS "OpenSSL Version: ${OPENSSL_VERSION}")
    message(STATUS "OpenSSL Include Dir: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL Libraries: ${OPENSSL_LIBRARIES}")
    include_directories(${OPENSSL_INCLUDE_DIR})
    link_directories(${OPENSSL_LIBRARIES})
endif ()
target_link_libraries(fuzzypsi ${OPENSSL_LIBRARIES})
