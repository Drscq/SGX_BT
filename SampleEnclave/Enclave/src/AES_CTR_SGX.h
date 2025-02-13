#pragma once

#include "sgx_tcrypto.h"
#include <cstdint>

class AES_CTR_SGX {
public:
    /**
     * Constructor expects a 128-bit key (16 bytes).
     * You can adapt for 256 bits if desired, but the Tcrypto library
     * specifically uses sgx_aes_ctr_128bit_key_t for 128-bit keys.
     */
    AES_CTR_SGX(const uint8_t *key);

    ~AES_CTR_SGX() = default;

    /**
     * Encrypts `input` of length `inputSize` in AES-CTR mode using the provided IV.
     * - `input` : pointer to plaintext
     * - `inputSize` : size in bytes
     * - `output` : pointer to buffer for ciphertext (in-place is allowed but be careful)
     * - `iv` : 128-bit counter (16 bytes)
     */
    void encrypt(const uint8_t* input, uint32_t inputSize, uint8_t* output, const uint8_t* iv);

    /**
     * Decrypts `input` of length `inputSize` in AES-CTR mode using the provided IV.
     * - `input` : pointer to ciphertext
     * - `inputSize` : size in bytes
     * - `output` : pointer to buffer for plaintext
     * - `iv` : 128-bit counter (16 bytes)
     */
    void decrypt(const uint8_t* input, uint32_t inputSize, uint8_t* output, const uint8_t* iv);

private:
    /**
     * Intel SGX expects a 128-bit key in sgx_aes_ctr_128bit_key_t.
     */
    sgx_aes_ctr_128bit_key_t m_key;
};
