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
# include <unistd.h>
# include <pwd.h>
# define MAX_PATH FILENAME_MAX

#include "sgx_urts.h"
#include "App.h"
#include "Enclave_u.h"
#include <chrono>
#include <vector>
#include <iostream>
#include <random>
#include <pthread.h>
#include <vector>
#include <fstream>
#include <sys/stat.h>
#include <chrono>
#include <time.h>
#include <cassert>
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/ec.h>
#include "src/config.h"
#include "src/Bucket.h"
#include "src/Block.h"
#include "src/Path.h"
#include "src/Server.h"
#include "src/Tree.h"
#include "src/AES_CTR.h"  
#include "src/ElGamal_parallel_ntl.h"
#include "src/DurationLogger.h"  
using namespace std::chrono;

void InitializeElGamalParams() {
    std::cout << "Initializing ElGamal Parameters..." << std::endl;
    
    // Set seed for random number generation
    SetSeed(ElGamalNTLConfig::SEED);
    
    // Generate prime P
    GenPrime(ElGamalNTLConfig::P, ElGamalNTLConfig::KEY_SIZE);
    ZZ_p::init(ElGamalNTLConfig::P);
//     std::cout << "P: " << ElGamalNTLConfig::P << std::endl;
    
    // Convert G to G_p
    ElGamalNTLConfig::G_p = conv<ZZ_p>(ElGamalNTLConfig::G);
//     std::cout << "G_p: " << ElGamalNTLConfig::G_p << std::endl;
    
    // Generate private key X and convert to X_p
    ElGamalNTLConfig::X = RandomLen_ZZ(ElGamalNTLConfig::RANDOM_SIZE);
    ElGamalNTLConfig::X_p = conv<ZZ_p>(ElGamalNTLConfig::X);
//     std::cout << "X_p: " << ElGamalNTLConfig::X_p << std::endl;
    
    // Calculate public key Y and convert to Y_p
    ElGamalNTLConfig::Y = PowerMod(ElGamalNTLConfig::G, ElGamalNTLConfig::X, ElGamalNTLConfig::P);
    ElGamalNTLConfig::Y_p = conv<ZZ_p>(ElGamalNTLConfig::Y);
//     std::cout << "Y_p: " << ElGamalNTLConfig::Y_p << std::endl;
    
    // Generate random K and convert to K_p
    ElGamalNTLConfig::K = RandomLen_ZZ(ElGamalNTLConfig::RANDOM_SIZE);
    ElGamalNTLConfig::K_p = conv<ZZ_p>(ElGamalNTLConfig::K);
//     std::cout << "K_p: " << ElGamalNTLConfig::K_p << std::endl;
     ElGamalNTLConfig::GPowK = power(ElGamalNTLConfig::G_p, ElGamalNTLConfig::K);
     ElGamalNTLConfig::YPowK = power(ElGamalNTLConfig::Y_p, ElGamalNTLConfig::K);
}

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
// Configuration: how many integers in the array?
static const size_t ARRAY_LEN = 10;
// Shared data between threads
static int* g_array = NULL;

// Thread synchronization
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_cond_array = PTHREAD_COND_INITIALIZER;
static pthread_cond_t g_cond_sorted = PTHREAD_COND_INITIALIZER;

// Flags
static bool g_array_ready = false;
static bool g_array_sorted = false;
/*
* Thread B (Enclave Worker)
* Waits until the array is ready, calls ecall_sort_array, then signals Thread A
*/

void* enclave_thread_func() {
    // Lock the mutex to wait for the array to be ready
    pthread_mutex_lock(&g_mutex);

    while (!g_array_ready) {
        pthread_cond_wait(&g_cond_array, &g_mutex);
    }
    // Now, the server thread has created & filled g_array
    // Let's call ecall_sort_array inside the enclave.
    if (g_array != NULL) {
        sgx_status_t status = ecall_sort_array(global_eid, g_array, ARRAY_LEN);
        if (status != SGX_SUCCESS) {
            printf("[Enclave Thread] ecall_sort_array failed: %d\n", status);
        } else {
            printf("[Enclave Thread] ecall_sort_array succeeded\n");
        }
    }
    // Signal that we are done sorting
    g_array_sorted = true;
    pthread_cond_signal(&g_cond_sorted);
    pthread_mutex_unlock(&g_mutex);

    return NULL;
}
/*
* Thread A (Server Thread)
* Generates random array, signals enclave thread,
* waits for sort completion, sum even indices.
*/
// void* server_thread_func(void* arg) {
void* server_thread_func() {
    // Allocate the array
    g_array = new int[ARRAY_LEN];
    if (!g_array) {
        printf("[Server Thread] Failed to allocate array\n");
        return NULL;
    }
    // Fill the array with random integers
    srand((unsigned int)time(NULL));
    for (size_t i = 0; i < ARRAY_LEN; ++i) {
        g_array[i] = rand() % 100;
    }
    // Display the unsorted array
    printf("[Server Thread] Unsorted array: \n");
    for (size_t i = 0; i < ARRAY_LEN; ++i) {
        printf("%d ", g_array[i]);
    }
    printf("\n");
    // Signal the enclave thread that the array is ready
    pthread_mutex_lock(&g_mutex);
    g_array_ready = true;
    pthread_cond_signal(&g_cond_array);
    // Wait for the enclave thread to finish sorting
    while (!g_array_sorted) {
        pthread_cond_wait(&g_cond_sorted, &g_mutex);
    }
    pthread_mutex_unlock(&g_mutex);
    // At this point, the array is sorted
    printf("[Server Thread] Sorted array: \n");
    for (size_t i = 0; i < ARRAY_LEN; ++i) {
        printf("%d ", g_array[i]);
    }
    printf("\n");
    // Now sum the elements at even indices
    size_t sum_even_indices = 0;
    for (size_t i = 0; i < ARRAY_LEN; i += 2) {
        sum_even_indices += g_array[i];
    }
    printf("[Server Thread] Sum of elements at even indices: %zu\n", sum_even_indices);

    // Clean up
    free(g_array);
    g_array = NULL;
    return NULL;
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
    LogConfig::CheckLogDir();
    InitializeElGamalParams();
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    } else if (argv[1] == std::string("server")) {
        Server server(ServerConfig::PORT);
        server.SgxEarlyReshuffleScheme1(global_eid, 0);
        server.Start();
    }
    // std::cout << "Initializing Tree..." << std::endl;
    // // Create threads
    // pthread_t serverThread, enclaveThread;
    // pthread_create(&serverThread, NULL, server_thread_func, NULL);
    // pthread_create(&enclaveThread, NULL, enclave_thread_func, NULL);

    // // Wait for threads to finish
    // pthread_join(serverThread, NULL);
    // pthread_join(enclaveThread, NULL);

    

    /* Destroy the enclave */
    sgx_destroy_enclave(global_eid);
    return 0;
}

