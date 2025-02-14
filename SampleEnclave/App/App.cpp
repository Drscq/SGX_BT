/*
 * Copyright (C) 2011-2021 Intel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Intel Corporation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <iostream> 
# include <unistd.h>
# include <pwd.h>
# define MAX_PATH FILENAME_MAX

#include "sgx_urts.h"
#include "App.h"
#include "Enclave_u.h"
#include <chrono>
#include <vector>

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

typedef struct _sgx_errlist_t {
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
} sgx_errlist_t;

/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] = {
    {
        SGX_ERROR_UNEXPECTED,
        "Unexpected error occurred.",
        NULL
    },
    {
        SGX_ERROR_INVALID_PARAMETER,
        "Invalid parameter.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_MEMORY,
        "Out of memory.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_LOST,
        "Power transition occurred.",
        "Please refer to the sample \"PowerTransition\" for details."
    },
    {
        SGX_ERROR_INVALID_ENCLAVE,
        "Invalid enclave image.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ENCLAVE_ID,
        "Invalid enclave identification.",
        NULL
    },
    {
        SGX_ERROR_INVALID_SIGNATURE,
        "Invalid enclave signature.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_EPC,
        "Out of EPC memory.",
        NULL
    },
    {
        SGX_ERROR_NO_DEVICE,
        "Invalid SGX device.",
        "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards."
    },
    {
        SGX_ERROR_MEMORY_MAP_CONFLICT,
        "Memory map conflicted.",
        NULL
    },
    {
        SGX_ERROR_INVALID_METADATA,
        "Invalid enclave metadata.",
        NULL
    },
    {
        SGX_ERROR_DEVICE_BUSY,
        "SGX device was busy.",
        NULL
    },
    {
        SGX_ERROR_INVALID_VERSION,
        "Enclave version was invalid.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ATTRIBUTE,
        "Enclave was not authorized.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_FILE_ACCESS,
        "Can't open enclave file.",
        NULL
    },
    {
        SGX_ERROR_MEMORY_MAP_FAILURE,
        "Failed to reserve memory for the enclave.",
        NULL
    },
};

/* Check error conditions for loading enclave */
void print_error_message(sgx_status_t ret)
{
    size_t idx = 0;
    size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++) {
        if(ret == sgx_errlist[idx].err) {
            if(NULL != sgx_errlist[idx].sug)
                printf("Info: %s\n", sgx_errlist[idx].sug);
            printf("Error: %s\n", sgx_errlist[idx].msg);
            break;
        }
    }
    
    if (idx == ttl)
    	printf("Error code is 0x%X. Please refer to the \"Intel SGX SDK Developer Reference\" for more details.\n", ret);
}

/* Initialize the enclave:
 *   Call sgx_create_enclave to initialize an enclave instance
 */
int initialize_enclave(void)
{
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    
    /* Call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */
    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, NULL, NULL, &global_eid, NULL);
    if (ret != SGX_SUCCESS) {
        print_error_message(ret);
        return -1;
    }

    return 0;
}

/* OCall functions */
void ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate 
     * the input string to prevent buffer overflow. 
     */
    printf("%s", str);
}


/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);


    /* Initialize the enclave */
    if(initialize_enclave() < 0){
        printf("Enter a character before exit ...\n");
        getchar();
        return -1; 
    }
    // Example: 1MB buffer
    size_t data_len = 1024 * 1024 * 4;
    // std::vector<uint8_t> host_buffer(data_len, 0xAB); // Fill with 0xAB
    uint8_t* host_buffer = (uint8_t*)malloc(data_len);
    // -- Start timing: transfer host->enclave
    volatile uint8_t sum = 0;
    for (size_t i = 0; i < data_len; ++i) {
        sum ^= host_buffer[i];
    }
    auto start1 = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < data_len; ++i) {
        sum ^= host_buffer[i];
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    double elapsedSec1 = std::chrono::duration_cast<std::chrono::duration<double>>(end1 - start1).count();
    double mbps1 = (static_cast<double>(data_len) / 1024 / 1024) / elapsedSec1;
    printf("Host->Host: %zu bytes in %.6f seconds (%.2f MB/s)\n", data_len, elapsedSec1, mbps1);
    std::cout << "Sum: " << (int)sum << std::endl;
    start1 = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < data_len; ++i) {
        host_buffer[i] = static_cast<uint8_t>(i & 0xFF);
    }
    end1 = std::chrono::high_resolution_clock::now();
    elapsedSec1 = std::chrono::duration_cast<std::chrono::duration<double>>(end1 - start1).count();
    mbps1 = (static_cast<double>(data_len) / 1024 / 1024) / elapsedSec1;
    printf("Host->Host: %zu bytes in %.6f seconds (%.2f MB/s)\n", data_len, elapsedSec1, mbps1);
    // data_len = 1024;
    auto start = std::chrono::high_resolution_clock::now();
    sgx_status_t ret = ecall_bandwidth_test(global_eid, host_buffer, data_len);
    auto end = std::chrono::high_resolution_clock::now();

    if (ret != SGX_SUCCESS) {
        printf("Error: ecall_bandwidth_test returned %d\n", ret);
        return -1;
    } else {
        double elapsedSec = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
        double mbps = (static_cast<double>(data_len) / 1024 / 1024) / elapsedSec;
        printf("Host->Enclave: %zu bytes in %.6f seconds (%.2f MB/s)\n", data_len, elapsedSec, mbps);
    }
    
    // -- Next, measure enclave->host
    // We will call ecall_write_to_untrusted, which writes data into the buffer
    start = std::chrono::high_resolution_clock::now();
    ret = ecall_write_to_untrusted(global_eid, host_buffer, data_len);
    end = std::chrono::high_resolution_clock::now();

    if (ret != SGX_SUCCESS) {
        printf("Error: ecall_write_to_untrusted returned %d\n", ret);
        return -1;
    } else {
        double elapsedSec = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count();
        double mbps = (static_cast<double>(data_len) / 1024 / 1024) / elapsedSec;
        printf("Enclave->Host: %zu bytes in %.6f seconds (%.2f MB/s)\n", data_len, elapsedSec, mbps);
    }
 
    /* Utilize edger8r attributes */
    edger8r_array_attributes();
    edger8r_pointer_attributes();
    edger8r_type_attributes();
    edger8r_function_attributes();
    
    /* Utilize trusted libraries */
    ecall_libc_functions();
    ecall_libcxx_functions();
    ecall_thread_functions();

    /* Destroy the enclave */
    sgx_destroy_enclave(global_eid);
    delete host_buffer;
    
    printf("Info: SampleEnclave successfully returned.\n");

    const char* message = "Hello, from the untrusted side!\n";
    
    ocall_print_string(message);

    ecall_print_hello_world(global_eid, message);

    // printf("Enter a character before exit ...\n");
    // getchar();
    return 0;
}

