#ifndef CONFIGSGX_H
#define CONFIGSGX_H
typedef long long TYPE_BLOCK_ID_SGX;
typedef unsigned long TYPE_UNSIGNED_SIZE_SGX;
typedef long long TYPE_PATH_ID_SGX;
typedef unsigned short TYPE_SLOT_ID_SGX;
const TYPE_UNSIGNED_SIZE_SGX BUCKET_REAL_BLOCK_CAPACITY_SGX = 30;
const TYPE_UNSIGNED_SIZE_SGX BUCKET_SIZE_SGX = 73;
TYPE_UNSIGNED_SIZE_SGX META_DATA_SIZE_SGX = sizeof(size_t) + BUCKET_SIZE_SGX * (sizeof(bool)) +
                                            sizeof(size_t) + BUCKET_REAL_BLOCK_CAPACITY_SGX * sizeof(TYPE_BLOCK_ID_SGX) +
                                            BUCKET_REAL_BLOCK_CAPACITY_SGX * sizeof(TYPE_BLOCK_ID_SGX) +
                                            BUCKET_SIZE_SGX * sizeof(TYPE_PATH_ID_SGX) +
                                            BUCKET_SIZE_SGX * sizeof(TYPE_SLOT_ID_SGX) +
                                            sizeof(TYPE_SLOT_ID_SGX) * 2;

#endif // CONFIGSGX_H