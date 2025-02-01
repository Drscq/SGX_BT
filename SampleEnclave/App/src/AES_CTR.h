// AES_CTR.h
#ifndef AES_CTR_H
#define AES_CTR_H

#include <openssl/aes.h>
#include <openssl/evp.h> 
#include <stdexcept>
#include <cstring>
#include "config.h"

class AES_CTR {
public:
    AES_CTR();
    ~AES_CTR();
    void encrypt(const unsigned char* input, BlockConfig::TYPE_BLOCK_SIZE inputSize, unsigned char* output, const unsigned char* iv);
    void decrypt(const unsigned char* input, BlockConfig::TYPE_BLOCK_SIZE inputSize, unsigned char* output, const unsigned char* iv);

private:
    EVP_CIPHER_CTX* encrypt_ctx;
    EVP_CIPHER_CTX* decrypt_ctx;
    int len; // Moved from function scope to class member
    int outputLength; // Moved from function scope to class member
};

#endif // AES_CTR_H