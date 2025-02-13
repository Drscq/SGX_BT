#include "AES_CTR_SGX.h"
#include <cstring>     // for memcpy
#include <stdexcept>   // for std::runtime_error
#include <sstream>     // Added to support string conversion for status codes

// SGX CTR mode uses a 128-bit key (16 bytes).
// The "counter" is also 128 bits, which you typically pass as your IV buffer.
// The last parameter "ctr_inc_bits" generally should be 128.

static const uint32_t CTR_INC_BITS = 128;  // Full 128-bit counter increments

AES_CTR_SGX::AES_CTR_SGX(const uint8_t *key)
{
    // Copy the 16-byte key into the sgx_aes_ctr_128bit_key_t structure
    // sgx_aes_ctr_128bit_key_t is just a typedef for uint8_t[16].
    memcpy(m_key, key, sizeof(sgx_aes_ctr_128bit_key_t));
}

void AES_CTR_SGX::encrypt(const uint8_t* input, uint32_t inputSize,
                          uint8_t* output, const uint8_t* iv)
{
    // sgx_aes_ctr_encrypt(...) does AES-128 in CTR mode.
    // The key is m_key (128 bits).
    // `iv` must be 16 bytes. `ctr_inc_bits` is how many bits to increment in the CTR.
    sgx_status_t ret = sgx_aes_ctr_encrypt(
        &m_key,          // 128-bit key
        input,           // plaintext
        inputSize,       // plaintext length
        const_cast<uint8_t*>(iv), // 16-byte IV as the initial counter
        CTR_INC_BITS,    // 128 bits
        output           // ciphertext output buffer
    );

    if (ret != SGX_SUCCESS) {
        std::ostringstream oss;
        oss << "sgx_aes_ctr_encrypt failed with status code " << ret;
        throw std::runtime_error(oss.str());
    }
}

void AES_CTR_SGX::decrypt(const uint8_t* input, uint32_t inputSize,
                          uint8_t* output, const uint8_t* iv)
{
    // sgx_aes_ctr_decrypt(...) does AES-128 in CTR mode (the inverse operation).
    sgx_status_t ret = sgx_aes_ctr_decrypt(
        &m_key,
        input,
        inputSize,
        const_cast<uint8_t*>(iv),
        CTR_INC_BITS,
        output
    );

    if (ret != SGX_SUCCESS) {
        std::ostringstream oss;
        oss << "sgx_aes_ctr_decrypt failed with status code " << ret;
        throw std::runtime_error(oss.str());
    }
}
