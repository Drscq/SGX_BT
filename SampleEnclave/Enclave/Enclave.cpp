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

void ecall_print_hello_world(const char* /*str*/) {
    printf("Hello from inside the enclave!\n");
    AES_CTR_SGX aes(reinterpret_cast<const uint8_t*>(key));
    // std::cout << "Hello from inside the enclave!" << std::endl;
}

void ecall_sort_array(int* arr, size_t arr_len) {
    std::sort(arr, arr + arr_len);
}