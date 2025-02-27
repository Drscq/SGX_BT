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
// Eviction Scheme 1
void SgxDecryptMD(void* serializedMDData, TYPE_BUCKET_ID_SGX bucketID) {
    static_cast<char*>(serializedMDData);
    std::memset(iv, bucketID, AES_BLOCK_SIZE);
    aes_sgx.decrypt(reinterpret_cast<const uint8_t*>(serializedMDData) + PLAINMDSIZE_SGX,
                    META_DATA_SIZE_SGX - PLAINMDSIZE_SGX,
                    reinterpret_cast<uint8_t*>(serializedMDData) + PLAINMDSIZE_SGX,
                    iv);
}
std::vector<TYPE_BUCKET_ID_SGX> tripletBucketIDsEviction1(3, 0);
TYPE_PATH_SIZE_SGX curLevelEviction1 = 0;
META_DATA_SGX tripletBucketMDs[3];
std::vector<TYPE_SLOT_ID_SGX> tripletBucketRealBlocksOffsetEviction1(3 * BUCKET_REAL_BLOCK_CAPACITY_SGX, 0);
TYPE_UNSIGNED_SIZE_SGX offset_base_eviction1 = BUCKET_REAL_BLOCK_CAPACITY_SGX * BLOCK_SIZE_SGX; 
TYPE_UNSIGNED_SIZE_SGX bucketSizeEviction1 = META_DATA_SIZE_SGX + BUCKET_SIZE_SGX * BLOCK_SIZE_SGX;
std::unordered_map<TYPE_BLOCK_ID_SGX, std::pair<TYPE_PATH_ID_SGX, std::vector<char>>> stashEviction1_sgx;
std::vector<TYPE_BLOCK_ID_SGX> blockIDsDeletedFromStashEviction1;
std::vector<char> dummyBlockData1(BLOCK_SIZE_SGX, 0);
int offset_base_flag = 4;
void ecall_evict_1(char* buffer, uint8_t* flags) {
    for(TYPE_SLOT_ID_SGX k = 0; k < TreeConfigSgx::HEIGHT - 1; ++k) {
        // Wait for the buffer to be ready
        while (!flags[k * offset_base_flag]) {
            __asm__ __volatile__("pause");
        }
        // Decrypt the meta data
        auto it = buffer;
        std::memcpy(tripletBucketIDsEviction1.data(), it, sizeof(TYPE_BUCKET_ID_SGX));
        for (TYPE_SMALL_INDEX_U_SGX i = 1; i < 3; ++i) {
            tripletBucketIDsEviction1[i] = 2 * tripletBucketIDsEviction1[0] + i;
        }
        #if defined(UNIT_TEST_SGX)
        for (TYPE_SMALL_INDEX_U_SGX i = 0; i < 3; ++i) {
            printf("tripletBucketIDsEviction1[%d] = %d\n", i, tripletBucketIDsEviction1[i]);
        }
        #endif
        it += sizeof(TYPE_BUCKET_ID_SGX);
        std::memcpy(&curLevelEviction1, it, sizeof(TYPE_PATH_SIZE_SGX));
        #if defined(UNIT_TEST_SGX)
        printf("curLevelEviction1 = %d\n", curLevelEviction1);
        #endif
        it += sizeof(TYPE_PATH_SIZE_SGX);
        for (TYPE_SMALL_INDEX_U_SGX i = 0; i < 3; ++i) {
            SgxDecryptMD(it, tripletBucketIDsEviction1[i]);
            tripletBucketMDs[i].Deserialize(it);
            TYPE_SLOT_ID_SGX curEmptySlotIndexEarlyReshuffle1 = 0;
            TYPE_SLOT_ID_SGX curProcessSlotIndexEarlyReshuffle1 = 0;
            while (curEmptySlotIndexEarlyReshuffle1 < BUCKET_REAL_BLOCK_CAPACITY_SGX && 
                    curProcessSlotIndexEarlyReshuffle1 < BUCKET_SIZE_SGX) {
                if (tripletBucketMDs[i].valids[tripletBucketMDs[i].offsets[curProcessSlotIndexEarlyReshuffle1]]) {
                    tripletBucketRealBlocksOffsetEviction1[i * BUCKET_REAL_BLOCK_CAPACITY_SGX + curEmptySlotIndexEarlyReshuffle1] = tripletBucketMDs[i].offsets[curProcessSlotIndexEarlyReshuffle1];
                    curEmptySlotIndexEarlyReshuffle1++;
                }
                curProcessSlotIndexEarlyReshuffle1++;
            }
            #if USE_COUT
            this->tripletBucketMDs[i].print();
            #endif
            it += META_DATA_SIZE_SGX;
        }
        #if defined(UNIT_TEST_SGX)
        // check the values in the tripletBucketRealBlocksOffsetEviction1 vector
        for (size_t i = 0; i < tripletBucketRealBlocksOffsetEviction1.size(); ++i) {
            printf("tripletBucketRealBlocksOffsetEviction1[%d] = %d\n", i, tripletBucketRealBlocksOffsetEviction1[i]);
        }
        #endif
        // copy the tripletBucketRealBlocksOffsetEviction1 vector to the buffer
        memcpy(buffer, tripletBucketRealBlocksOffsetEviction1.data(), tripletBucketRealBlocksOffsetEviction1.size() * sizeof(TYPE_SLOT_ID_SGX));
        flags[k * offset_base_flag + 1] = 1;
        // wait for the server to send the real blocks
        while (!flags[k * offset_base_flag + 2]) {
            __asm__ __volatile__("pause");
        }
        // Move the real blocks from the buffer to the stash_sgx
        for (TYPE_SMALL_INDEX_U_SGX i = 0; i < 3; ++i) {
            it = buffer + i * offset_base_eviction1;
            for (TYPE_SLOT_ID_SGX j = 0; j < tripletBucketMDs[i].nextRealIndex; ++j) {
                std::vector<char> decryptedBlockData(BLOCK_SIZE_SGX, 0);
                std::memset(iv, tripletBucketIDsEviction1[i] + tripletBucketRealBlocksOffsetEviction1[i * BUCKET_REAL_BLOCK_CAPACITY_SGX + j], AES_BLOCK_SIZE);
                aes_sgx.decrypt(reinterpret_cast<uint8_t*>(it),
                                    BLOCK_SIZE_SGX,
                                    reinterpret_cast<uint8_t*>(decryptedBlockData.data()),
                                    iv);
                stashEviction1_sgx[tripletBucketMDs[i].addrs[j]].first = tripletBucketMDs[i].leaves[j];
                stashEviction1_sgx[tripletBucketMDs[i].addrs[j]].second = std::move(decryptedBlockData);
                it += BLOCK_SIZE_SGX;
            }
        }

        blockIDsDeletedFromStashEviction1.clear();
        for(TYPE_SMALL_INDEX_U_SGX i = 3; i > 0; --i) {
            // Reset the tripletBucketMDs
            tripletBucketMDs[i - 1].ResetEviction1();
            // Re-construction the bucket meta data and data
            for (const auto& block : stashEviction1_sgx) {
                if (TreeConfigSgx::CanBlockGoThroughBucket(block.second.first, 
                                                        TreeConfigSgx::HEIGHT,
                                                        tripletBucketIDsEviction1[i - 1],
                                                        curLevelEviction1)) {
                    tripletBucketMDs[i - 1].AddRealBlock(block.first, block.second.first);
                    blockIDsDeletedFromStashEviction1.emplace_back(block.first);
                }
                                                            
            }
            it = buffer + (i - 1) * bucketSizeEviction1;
            for (TYPE_SLOT_ID_SGX j = 0; j < tripletBucketMDs[i - 1].nextRealIndex; ++j) {
                std::memset(iv, tripletBucketIDsEviction1[i - 1] + tripletBucketMDs[i - 1].offsets[j], AES_BLOCK_SIZE);
                aes_sgx.encrypt(reinterpret_cast<const uint8_t*>(stashEviction1_sgx[tripletBucketMDs[i - 1].addrs[j]].second.data()),
                                    BLOCK_SIZE_SGX,
                                    reinterpret_cast<uint8_t*>(it + META_DATA_SIZE_SGX + tripletBucketMDs[i - 1].offsets[j] * BLOCK_SIZE_SGX),
                                    iv);       
            }
            while (tripletBucketMDs[i - 1].nextDummyIndex < BUCKET_SIZE_SGX) {
                std::memset(iv, tripletBucketIDsEviction1[i - 1] + tripletBucketMDs[i - 1].offsets[tripletBucketMDs[i - 1].nextDummyIndex], AES_BLOCK_SIZE);
                aes_sgx.encrypt(reinterpret_cast<const uint8_t*>(dummyBlockData1.data()),
                                    BLOCK_SIZE_SGX,
                                    reinterpret_cast<uint8_t*>(it + META_DATA_SIZE_SGX + tripletBucketMDs[i - 1].offsets[tripletBucketMDs[i - 1].nextDummyIndex] * BLOCK_SIZE_SGX),
                                    iv);
                tripletBucketMDs[i - 1].AddDummyBlock();
            }
            tripletBucketMDs[i - 1].Serialize(it);
            SgxDecryptMD(it, tripletBucketIDsEviction1[i - 1]);
        }
        // Delete the blocks from the stashEviction1
        for (const auto& blockID : blockIDsDeletedFromStashEviction1) {
            stashEviction1_sgx.erase(blockID);
        }
        flags[k * offset_base_flag + 3] = 1;
    }
}

std::vector<TYPE_SLOT_ID_SGX> perm1EarlyReshuffleComplete_sgx(BUCKET_SIZE_SGX, 1);
// The enclave functions for the second scheme
void ecall_early_reshuffle_2(char* buffer, uint8_t* flags) {
    while (!flags[0]) {
        // Wait for the buffer to be ready
        __asm__ __volatile__("pause");
    }
    printf("The values of flags[0] = %d\n", flags[0]);
    printf("The content of the buffer\n");
    std::memcpy(perm1EarlyReshuffleComplete_sgx.data(), buffer, perm1EarlyReshuffleComplete_sgx.size() * sizeof(TYPE_SLOT_ID_SGX));
    // #if defined(UNIT_TEST_SGX)
    printf("The content of the perm1EarlyReshuffleComplete_sgx vector\n");
    // Check the values in the perm1EarlyReshuffleComplete_sgx vector
    for (size_t i = 0; i < perm1EarlyReshuffleComplete_sgx.size(); ++i) {
        printf("perm1EarlyReshuffleComplete_sgx[%d] = %d\n", i, perm1EarlyReshuffleComplete_sgx[i]);
    }
    // #endif
}

void ecall_sort_array(int* arr, size_t arr_len) {
    std::sort(arr, arr + arr_len);
}