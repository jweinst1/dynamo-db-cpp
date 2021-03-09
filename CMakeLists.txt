cmake_minimum_required(VERSION 3.8)

project(dynamo-db-cpp)

include(ExternalProject)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

# Must be set for compiling aws sdk
set(CMAKE_CXX_STANDARD 11)

set(BUILD_DEPS_DIR ${PROJECT_BINARY_DIR}/deps)
set(AWS_SDK_INSTALL_DIR ${PROJECT_BINARY_DIR}/awssdk)

file(MAKE_DIRECTORY ${BUILD_DEPS_DIR})
file(MAKE_DIRECTORY ${AWS_SDK_INSTALL_DIR})

set(AWS_COMPONENTS
    dynamodb
)


ExternalProject_Add(aws-sdk-cpp
       PREFIX ${BUILD_DEPS_DIR}
       GIT_REPOSITORY "https://github.com/aws/aws-sdk-cpp.git"
       BUILD_IN_SOURCE 0
       CMAKE_ARGS
       -DENABLE_TESTING=OFF
       -DAUTORUN_UNIT_TESTS=OFF
       -DBUILD_ONLY=${AWS_COMPONENTS}
       -DENABLE_UNITY_BUILD=ON
       -DBUILD_SHARED_LIBS=OFF
)

# Need to set these explicitly due to using external project
set(AWSSDK_LINK_LIBRARIES
    aws-cpp-sdk-dynamodb
    aws-cpp-sdk-core
    aws-c-event-stream
    aws-checksums
    aws-c-common
    pthread
    curl
)

# Source files go here.
set(DYANMO_CPP_SOURCE
    main.cpp
    Logger.cpp
    TableMonitor.cpp
)

# We must let the compiler know its OK to link or include from /usr/local
link_directories(${CMAKE_INSTALL_PREFIX}/lib)
include_directories(${CMAKE_INSTALL_PREFIX}/include)

add_executable(dynamo_cpp "${DYANMO_CPP_SOURCE}")
target_link_libraries(dynamo_cpp "${AWSSDK_LINK_LIBRARIES}")
add_dependencies(dynamo_cpp aws-sdk-cpp)
