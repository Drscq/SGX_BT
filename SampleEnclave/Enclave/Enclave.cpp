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

#include "Enclave.h"
#include "Enclave_t.h" /* print_string */
#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include <string.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <unordered_map>
// #define AES_BLOCK_SIZE 16
// static const uint32_t CTR_INC_BITS = 128;  // Full 128-bit counter increments
#include "src/AES_CTR_SGX.h"
unsigned char key[AES_BLOCK_SIZE] = {
    0x2b, 0x7e, 0x15, 0x16,
    0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88,
    0x09, 0xcf, 0x4f, 0x3c
};
/* 
 * printf: 
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
int printf(const char* fmt, ...)
{
    char buf[BUFSIZ] = { '\0' };
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
    return (int)strnlen(buf, BUFSIZ - 1) + 1;
}

void ecall_bandwidth_test(uint8_t* data, size_t data_len) {
    // This function can do something with the incoming buffer,
    // e.g., read it, parse it, etc. For a simple test, maybe just 
    // do a quick pass to simulate some work:
    volatile uint8_t sum = 0;
    for (size_t i = 0; i < data_len; ++i) {
        sum ^= data[i];
    }
    // sum is volatile to ensure the loop doesn't get optimized out
}


void ecall_write_to_untrusted(uint8_t* data, size_t data_len) {
    // For demonstration, fill the enclave buffer with some values
    for (size_t i = 0; i < data_len; ++i) {
        data[i] = static_cast<uint8_t>(i & 0xFF);
    }
    // Upon return, SGX copies 'data' back out to untrusted memory
}
#include "../App/src/configSgx.h"
META_DATA_SGX meta_data_sgx;
std::vector<TYPE_SLOT_ID_SGX> realBlocksOffsetEarlyReshuffle1_sgx(BUCKET_REAL_BLOCK_CAPACITY_SGX, 0);
std::unordered_map<TYPE_BLOCK_ID_SGX, std::vector<char>> stash_sgx;
std::vector<char> decrypted_block_data_sgx(BLOCK_SIZE_SGX, 0);
std::vector<char> dummy_block_data_sgx(BLOCK_SIZE_SGX, 0);
TYPE_BUCKET_ID_SGX bucket_id_sgx = 0;
AES_CTR_SGX  aes_sgx(reinterpret_cast<const uint8_t*>(key));
uint8_t iv[AES_BLOCK_SIZE] = {0};
void ecall_early_reshuffle_1(char* buffer, uint8_t* flag) {
    // printf("The test value is: %d\n", META_DATA_SIZE_SGX);
    // printf("Hello from inside the enclave!\n");
    // printf("The flag value is: %d\n", *flag);
    while (!flag[0]) {
        // Wait for the buffer to be ready
        __asm__ __volatile__("pause");
    }
    aes_sgx.decrypt(reinterpret_cast<const uint8_t*>(buffer) + PLAINMDSIZE_SGX,
                    META_DATA_SIZE_SGX - PLAINMDSIZE_SGX, 
                    reinterpret_cast<uint8_t*>(buffer) + PLAINMDSIZE_SGX, iv); 
    meta_data_sgx.Deserialize(reinterpret_cast<const char*>(buffer));
    TYPE_SLOT_ID_SGX curEmptySlotIndexEarlyReshuffle1_sgx = 0;
    TYPE_SLOT_ID_SGX curProcessSlotIndexEarlyReshuffle1_sgx = 0;
    while (curEmptySlotIndexEarlyReshuffle1_sgx < BUCKET_REAL_BLOCK_CAPACITY_SGX && 
            curProcessSlotIndexEarlyReshuffle1_sgx < BUCKET_SIZE_SGX) {
        if (meta_data_sgx.valids[meta_data_sgx.offsets[curProcessSlotIndexEarlyReshuffle1_sgx]]) {
            realBlocksOffsetEarlyReshuffle1_sgx[curEmptySlotIndexEarlyReshuffle1_sgx] = meta_data_sgx.offsets[curProcessSlotIndexEarlyReshuffle1_sgx];
            curEmptySlotIndexEarlyReshuffle1_sgx++;
        }
        curProcessSlotIndexEarlyReshuffle1_sgx++;
    }
    #if defined(UNIT_TEST_SGX)
    // Check the values in the realBlocksOffsetEarlyReshuffle1_sgx vector
    for (size_t i = 0; i < realBlocksOffsetEarlyReshuffle1_sgx.size(); ++i) {
        printf("realBlocksOffsetEarlyReshuffle1_sgx[%d] = %d\n", i, realBlocksOffsetEarlyReshuffle1_sgx[i]);
    }
    #endif
    // copy the realBlocksOffsetEarlyReshuffle1_sgx vector to the buffer
    memcpy(buffer, realBlocksOffsetEarlyReshuffle1_sgx.data(), realBlocksOffsetEarlyReshuffle1_sgx.size() * sizeof(TYPE_SLOT_ID_SGX));
    flag[1] = 1;
    // Store the real blocks into the stash
    while(!flag[2]) {
        __asm__ __volatile__("pause");
    }
    auto it = reinterpret_cast<char*>(buffer);
    for (TYPE_SLOT_ID_SGX i = 0; i < meta_data_sgx.nextRealIndex; ++i) {
        std::memset(iv, bucket_id_sgx + meta_data_sgx.offsets[i], AES_BLOCK_SIZE);
        aes_sgx.decrypt(reinterpret_cast<const uint8_t*>(it),
                        BLOCK_SIZE_SGX,
                        reinterpret_cast<uint8_t*>(decrypted_block_data_sgx.data()),
                        iv);
        stash_sgx[meta_data_sgx.addrs[i]] = decrypted_block_data_sgx;
        it += BLOCK_SIZE_SGX;
    }
    // reset the meta_data_sgx
    meta_data_sgx.ResetEarlyReshuffle1();
    // Re-construction the bucket data
        // Step 1 write the real blocks to the this->bucketDataEarlyReshuffle1
        for (TYPE_SLOT_ID_S_SGX i = 0; i < meta_data_sgx.nextRealIndex; ++i) {
            it = reinterpret_cast<char*>(reinterpret_cast<uint8_t*>(buffer) + META_DATA_SIZE_SGX) +
                    meta_data_sgx.offsets[i] * BLOCK_SIZE_SGX;
            std::memset(iv, bucket_id_sgx +
                        meta_data_sgx.offsets[i], AES_BLOCK_SIZE);
            aes_sgx.encrypt(reinterpret_cast<const uint8_t*>(stash_sgx[meta_data_sgx.addrs[i]].data()),
                            BLOCK_SIZE_SGX,
                            reinterpret_cast<uint8_t*>(it),
                            iv);
        }
        // Step 2 write the dummy blocks to the this->bucketDataEarlyReshuffle1
        for (TYPE_SLOT_ID_SGX i = meta_data_sgx.nextRealIndex; i < BUCKET_SIZE_SGX; ++i) {
            it = buffer + META_DATA_SIZE_SGX +
                meta_data_sgx.offsets[i] * BLOCK_SIZE_SGX;
            std::memset(iv, bucket_id_sgx +
                        meta_data_sgx.offsets[i], AES_BLOCK_SIZE);
            aes_sgx.encrypt(reinterpret_cast<const uint8_t*>(dummy_block_data_sgx.data()),
                                BLOCK_SIZE_SGX,
                                reinterpret_cast<uint8_t*>(it),
                                iv);
        }
        // Step 3 write the meta data to the this->bucketDataEarlyReshuffle1
        // Step 3.1 serialize the bucketMD to the bucketMDataEarlyReshuffle1
        it = buffer;
        meta_data_sgx.Serialize(it);
        // Step 3.2 encrypt the bucketMDataEarlyReshuffle1
        std::memset(iv, bucket_id_sgx, AES_BLOCK_SIZE);
        aes_sgx.encrypt(reinterpret_cast<const uint8_t*>(it) + PLAINMDSIZE_SGX,
                        META_DATA_SIZE_SGX - PLAINMDSIZE_SGX,
                        reinterpret_cast<uint8_t*>(it) + PLAINMDSIZE_SGX,
                        iv);
        flag[3] = 1;
}

void ecall_sort_array(int* arr, size_t arr_len) {
    std::sort(arr, arr + arr_len);
}