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

// Function to test BIGNUM operation inside the enclave
void test_bignum_in_enclave() {
    char result[256] = { 0 };
    
    printf("[App] Calling ecall_test_bignum to perform crypto operations in the enclave...\n");
    
    sgx_status_t status = ecall_test_bignum(global_eid, result, sizeof(result));
    if (status != SGX_SUCCESS) {
        printf("[App] ecall_test_bignum failed: %d\n", status);
        return;
    }
    
    printf("[App] Result from enclave: %s\n", result);
    
    // Optional: Verify the result
    // The expected result should be 123456789 + 987654321 = 1111111110
    printf("[App] Expected result should be: 1111111110\n");
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
    // const char* str = "Hello, Enclave!";
    // ecall_early_reshuffle_1(global_eid, str);
    LogConfig::CheckLogDir();
    DurationLogger durationLogger(LogConfig::LOG_DIR + LogConfig::LOG_FILE);
    InitializeElGamalParams();
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    } else if (argv[1] == std::string("earlyReshuffle1")) {
        Server server(ServerConfig::PORT);
        server.SgxEarlyReshuffleScheme1Init(0);
        std::string logMessage = "EarlyReshuffleScheme1";
        durationLogger.startTiming(logMessage);
        server.SgxEarlyReshuffleScheme1(global_eid, 0);
        durationLogger.stopTiming(logMessage);
        durationLogger.writeToFile();
        
    } else if (argv[1] == std::string("eviction1")) {
        Server server(ServerConfig::PORT);
        // Initialize the path
        PathConfig::TYPE_PATH_ID pathID = 0;
        server.tree.GenEvictPathWithMDs(pathID);
        std::string logMessage = "EvictionScheme1";
        durationLogger.startTiming(logMessage);
        server.SgxEvictScheme1(global_eid, pathID);
        durationLogger.stopTiming(logMessage);
        durationLogger.writeToFile();
    } else if (argv[1] == std::string("server")) {
        Server server(ServerConfig::PORT);
        server.Start();
    } else if (strcmp(argv[1], "test_bignum") == 0) {
        // Test BIGNUM functionality
        test_bignum_in_enclave();
        size_t num_threads = 2;
        size_t data_size = 2;
        ElGamal_parallel_ntl elgamal(num_threads, data_size);
        // Test ElGamal encryption and decryption for the chunk level
        BIGNUM* bn_message = BN_new();
            BIGNUM* c1 = BN_new();
            BIGNUM* c2 = BN_new();
            BIGNUM* message_decrypted = BN_new();
            std::cout << "Testing ElGamal encryption and decryption for the chunk level..." << std::endl;
            for (int i = 0; i < 10; ++i) {
                BN_set_word(bn_message, 100 + i);
                elgamal.EncryptBlock(bn_message, c1, c2);
                elgamal.DecryptBlock(c1, c2, message_decrypted);
                std::cout << "The decrypted message is: " << BN_bn2dec(message_decrypted) << std::endl;
            }
            BN_free(bn_message);
            BN_free(c1);
            BN_free(c2);
            BN_free(message_decrypted);
        // Test the ElGamalConfigSGX::generate_identity_data
        std::cout << "Testing ElGamalConfigSGX::generate_identity_data..." << std::endl;
        std::vector<char> identity_data;
        ElGamalConfig::generate_identity_data(BlockConfig::BLOCK_SIZE, identity_data);
        ElGamalConfig::test_generate_identity_data(identity_data);
        // Test the ElGamal parallel encryption and decryption
        std::cout << "Testing ElGamal parallel encryption and decryption..." << std::endl;
        int num_of_chunks = ElGamalNTLConfig::BLOCK_CHUNK_SIZE;
        std::vector<std::vector<BIGNUM*>> ciphertexts(2, std::vector<BIGNUM*>(num_of_chunks));
        for (int i = 0; i < num_of_chunks; ++i) {
            ciphertexts[0][i] = BN_new();
            ciphertexts[1][i] = BN_new();
        }
        elgamal.ParallelEncrypt(identity_data, ciphertexts);
        elgamal.ParallelDecrypt(ciphertexts, identity_data);
        ElGamalConfig::test_generate_identity_data(identity_data);

        // Test the ElGamal_parallel_ntl::ConvertVecChar2VecBN
        std::cout << "Testing ElGamal_parallel_ntl::ConvertVecChar2VecBN..." << std::endl;
        std::vector<BIGNUM*> bn_data_vec(num_of_chunks);
        for (int i = 0; i < num_of_chunks; ++i) {
            bn_data_vec[i] = BN_new();
        }
        elgamal.ConvertVecChar2VecBN(identity_data, bn_data_vec);
        elgamal.ParallelEncrypt(bn_data_vec, ciphertexts[0], ciphertexts[1]);
        std::vector<char> decrypted_data_identity;
        elgamal.ParallelDecrypt(ciphertexts, decrypted_data_identity);
        ElGamalConfig::test_generate_identity_data(decrypted_data_identity);
        std::cout << "Testing ElGamal_parallel_ntl::ConvertVecBNCipher2VecChar..." << std::endl;
        std::vector<char> ciphertext_data(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
        elgamal.ConvertVecBNCipher2VecChar(ciphertexts[0], ciphertexts[1], ciphertext_data);

        // ElGamalConfig::test_generate_identity_data(identity_data);
        for (int i = 0; i < num_of_chunks; ++i) {
            BN_free(ciphertexts[0][i]);
            BN_free(ciphertexts[1][i]);
            BN_free(bn_data_vec[i]);
        }
    } else {
        std::cout << "Usage: " << argv[0] << " [earlyReshuffle1|eviction1|server|test_bignum]" << std::endl;
    }
    

    /* Destroy the enclave */
    sgx_destroy_enclave(global_eid);
    return 0;
}

