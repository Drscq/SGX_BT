// AES_CTR.cpp
#include "AES_CTR.h"
#include <chrono>
#include <iostream>

AES_CTR::AES_CTR() {
    // Create and initialize the context
    encrypt_ctx = EVP_CIPHER_CTX_new();
    decrypt_ctx = EVP_CIPHER_CTX_new();
    if (!encrypt_ctx || !decrypt_ctx) {
        throw std::runtime_error("Failed to create EVP_CIPHER_CTX");
    }
     // Initialize the contexts with cipher and key
    if (1 != EVP_EncryptInit_ex(encrypt_ctx, EVP_aes_128_ctr(), nullptr, AESConfig::key, nullptr)) {
        throw std::runtime_error("Encryption initialization failed");
    }

    if (1 != EVP_DecryptInit_ex(decrypt_ctx, EVP_aes_128_ctr(), nullptr, AESConfig::key, nullptr)) {
        throw std::runtime_error("Decryption initialization failed");
    }

    len = 0;
    outputLength = 0;
}

AES_CTR::~AES_CTR() {
    // Clean up the context
   if (encrypt_ctx) {
        EVP_CIPHER_CTX_free(encrypt_ctx);
    }
    if (decrypt_ctx) {
        EVP_CIPHER_CTX_free(decrypt_ctx);
    }
}

void AES_CTR::encrypt(const unsigned char* input, BlockConfig::TYPE_BLOCK_SIZE inputSize, unsigned char* output, const unsigned char* iv) {
    outputLength = 0;
    #if LOG_AES_CTR_ENCRYPTION_DECRIPTION
    auto start = std::chrono::high_resolution_clock::now();
    #endif
    // Set the IV for encryption operation
    // EVP_CIPHER_CTX* encrypt_ctx = EVP_CIPHER_CTX_new();
    if (1 != EVP_EncryptInit_ex(encrypt_ctx, nullptr, nullptr, nullptr, iv)) {
        throw std::runtime_error("Encryption IV setup failed");
    }

    // Provide the message to be encrypted, and obtain the encrypted output
    if (1 != EVP_EncryptUpdate(encrypt_ctx, output, &len, input, inputSize)) {
        throw std::runtime_error("Encryption failed");
    }
    outputLength += len;

    // Finalize encryption
    if (1 != EVP_EncryptFinal_ex(encrypt_ctx, output + outputLength, &len)) {
        throw std::runtime_error("Final encryption step failed");
    }
    #if LOG_AES_CTR_ENCRYPTION_DECRIPTION
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<long long, std::nano> duration = end - start;
    std::cout << "Encryption took: " << duration.count() << " nanoseconds" << std::endl;
    #endif
    // free the context
    // EVP_CIPHER_CTX_free(encrypt_ctx);
}

void AES_CTR::decrypt(const unsigned char* input, BlockConfig::TYPE_BLOCK_SIZE inputSize, unsigned char* output, const unsigned char* iv) {
    outputLength = 0;
    #if LOG_AES_CTR_ENCRYPTION_DECRIPTION
    auto start = std::chrono::high_resolution_clock::now();
    #endif
    // Set the IV for decryption operation
    // EVP_CIPHER_CTX* decrypt_ctx = EVP_CIPHER_CTX_new();
    if (1 != EVP_DecryptInit_ex(decrypt_ctx, nullptr, nullptr, nullptr, iv)) {
        throw std::runtime_error("Decryption IV setup failed");
    }

    // Provide the message to be decrypted, and obtain the decrypted output
    if (1 != EVP_DecryptUpdate(decrypt_ctx, output, &len, input, inputSize)) {
        throw std::runtime_error("Decryption failed");
    }
    outputLength += len;

    // Finalize decryption
    if (1 != EVP_DecryptFinal_ex(decrypt_ctx, output + outputLength, &len)) {
        throw std::runtime_error("Final decryption step failed");
    }
    #if LOG_AES_CTR_ENCRYPTION_DECRIPTION
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<long long, std::nano> duration = end - start;
    std::cout << "Decryption took: " << duration.count() << " nanoseconds" << std::endl;
    #endif
    // free the context
    // EVP_CIPHER_CTX_free(decrypt_ctx);
}
