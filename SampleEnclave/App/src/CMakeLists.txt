cmake_minimum_required(VERSION 3.10)

project(CRTGamal)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the optimization level
add_compile_options(-O3)
# Add any additional flags that might be beneficial
add_compile_options(-march=native)  # Optimize for the local machine

# Find OpenSSL
find_package(OpenSSL REQUIRED)
# Set the local installation paths
set(LOCAL_PREFIX $ENV{HOME}/local)

# Include directories
include_directories(${LOCAL_PREFIX}/include)

# Include OpenSSL headers
include_directories(${OPENSSL_INCLUDE_DIR})
add_executable(crtgamal
    main.cpp
    DurationLogger.cpp
    ElGamal_parallel_ntl.cpp
    Bucket.cpp
    Block.cpp
    Path.cpp
    SocketCommunicator.cpp
    Server.cpp
    Client.cpp
    Tree.cpp
    AES_CTR.cpp
)

target_link_libraries(crtgamal
    OpenSSL::Crypto
    OpenSSL::SSL
    ${LOCAL_PREFIX}/lib/libntl.a
    ${LOCAL_PREFIX}/lib/libgmp.a
    stdc++fs
    pthread
)

set_property(TARGET crtgamal PROPERTY CXX_STANDARD 17)

####################################
############### Tests ##############
####################################
# Define a macro for the output directory
set(TEST_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/output/test")

# Create the output/test directory
file(MAKE_DIRECTORY ${TEST_OUTPUT_DIR})

add_executable(test_openssl_ntl
    Test/test_openssl_ntl.cpp
)

target_link_libraries(test_openssl_ntl
    OpenSSL::Crypto
    ${LOCAL_PREFIX}/lib/libntl.a
    ${LOCAL_PREFIX}/lib/libgmp.a
    pthread
)

set_property(TARGET test_openssl_ntl PROPERTY CXX_STANDARD 17)

# Set the output directory for the test executable
set_target_properties(test_openssl_ntl PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIR}
)

add_executable(test_openssl_ntl_parallel
    Test/test_openssl_ntl_parallel.cpp
    ElGamal_parallel_ntl.cpp
    ElGamal_parallel_ntl.h
    DurationLogger.cpp
    DurationLogger.h
    config.h
)

target_link_libraries(test_openssl_ntl_parallel
    OpenSSL::Crypto
    ${LOCAL_PREFIX}/lib/libntl.a
    ${LOCAL_PREFIX}/lib/libgmp.a
    pthread
)

set_property(TARGET test_openssl_ntl_parallel PROPERTY CXX_STANDARD 17)
# Set the output directory for the test executable
set_target_properties(test_openssl_ntl_parallel PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIR}
)

add_executable(test_generate_evict_path
    Test/test_generate_evict_path.cpp
    ElGamal_parallel_ntl.cpp
    ElGamal_parallel_ntl.h
    DurationLogger.cpp
    DurationLogger.h
    config.h
    Path.cpp
    Path.h
    Bucket.cpp
    Bucket.h
    Block.cpp
    Block.h
)

target_link_libraries(test_generate_evict_path
    OpenSSL::Crypto
    ${LOCAL_PREFIX}/lib/libntl.a
    ${LOCAL_PREFIX}/lib/libgmp.a
    pthread
)

set_property(TARGET test_generate_evict_path PROPERTY CXX_STANDARD 17)
# Set the output directory for the test executable
set_target_properties(test_generate_evict_path PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIR}
)
