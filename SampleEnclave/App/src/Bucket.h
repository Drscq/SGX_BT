// #include <new>
#ifndef BUCKET_H
#define BUCKET_H
#include "config.h"
#include "ElGamal_parallel_ntl.h"
#include "Block.h"
#include <fstream>
#include "DurationLogger.h"
class Bucket {
public:
    Bucket(); // Default constructor
    Bucket(BucketConfig::TYPE_BUCKET_ID id, 
            BucketConfig::TYPE_BUCKET_SIZE bucketSize,
            BucketConfig::TYPE_BUCKET_SIZE realBlockCapacity, 
            BucketConfig::TYPE_BUCKET_SIZE dummyBlockCapacity, 
            BlockConfig::TYPE_BLOCK_SIZE blockSize,
            BucketConfig::TYPE_THREAD_NUM num_threads
            );
    ~Bucket();
    // Methods
    void SetBucketID(BucketConfig::TYPE_BUCKET_ID id);
    void AddRealBlock(BlockConfig::TYPE_BLOCK_ID blockID, 
                      PathConfig::TYPE_PATH_ID leafID,
                      std::vector<char>& blockData);
    void AddDummyBlock(std::vector<char>& blockData);
    void SaveData2Disk(const std::string& dirPath,
                       const std::string& fileName);
    void LoadDataFromDisk(const std::string& dirPath,
                          const std::string& fileName,
                          std::vector<std::vector<std::pair<ZZ, ZZ>>>& bucketCiphertexts);
    void LoadDataFromDisk(const std::string& dirPath,
                          const std::string& fileName,
                          std::vector<std::vector<std::pair<ZZ_p, ZZ_p>>>& bucketCiphertexts);
    void LoadDataFromDiskSupportMT(const std::string& dirPath,
                                   const std::string& fileName,
                                   std::vector<std::vector<std::pair<ZZ_p, ZZ_p>>>& bucketCiphertexts);
    void LoadDataFromDiskWithOffset(const std::string& dirPath,
                                    const std::string& fileName,
                                    std::vector<std::vector<std::pair<ZZ, ZZ>>>& bucketCiphertexts,
                                    std::vector<BucketConfig::TYPE_SLOT_ID>& offsets);
    void LoadDataFromDiskWithOffset(const std::string& dirPath,
                                    const std::string& fileName,
                                    std::vector<std::vector<std::pair<ZZ_p, ZZ_p>>>& bucketCiphertexts,
                                    std::vector<BucketConfig::TYPE_SLOT_ID>& offsets);
    void LoadTripletDataFromDiskWithOffset(const std::string& dirPath,
                                            const std::string& fileName,
                                            std::vector<BucketConfig::TYPE_BUCKET_ID>& tripletBucketIDs,
                                            std::vector<std::vector<std::pair<ZZ_p, ZZ_p>>>& bucketCiphertexts,
                                            std::vector<BucketConfig::TYPE_SLOT_ID>& offsets);

    void LoadSingleBlockFromDisk(const std::string& dirPath,
                                 const std::string& fileName,
                                 const BucketConfig::TYPE_SLOT_ID& blockIndex,
                                 std::vector<std::pair<ZZ, ZZ>>& blockCiphertexts);
    void LoadSingleBlockFromDisk(const std::string& dirPath,
                                const std::string& fileName,
                                const BucketConfig::TYPE_SLOT_ID& blockIndex,
                                std::vector<std::pair<ZZ_p, ZZ_p>>& blockCiphertexts);
    void LoadSingleBlockFromDiskWithUpdate(const std::string& dirPath,
                                           const std::string& fileName,
                                           const BucketConfig::TYPE_SLOT_ID& blockIndex,
                                           std::vector<std::pair<ZZ_p, ZZ_p>>& blockCiphertexts);

    void PrintBlockIDs();

    // Properties
    BucketConfig::TYPE_BUCKET_ID id;
    BucketConfig::META_DATA md;
    BucketConfig::TYPE_BUCKET_SIZE bucketSize;
    BucketConfig::TYPE_BUCKET_SIZE realBlockCapacity;
    BucketConfig::TYPE_BUCKET_SIZE dummyBlockCapacity;
    BlockConfig::TYPE_BLOCK_SIZE blockSize;
    BucketConfig::TYPE_THREAD_NUM num_threads;
    std::vector<char> data;
    std::vector<char> blockData;
    std::vector<char> ciphertextsData;
    ClientConfig::TYPE_CHAR_SIZE ciphertextsDataSize;
    std::vector<std::pair<ZZ, ZZ>> ciphertexts;
    std::vector<std::pair<ZZ_p, ZZ_p>> ciphertexts_ZZ_p;
    ElGamal_parallel_ntl elgamal;

    // Data structures
    Block block;
    std::vector<char> dummyBlock;
    std::vector<std::pair<ZZ_p, ZZ_p>> dummyBlockCiphertexts;
    // std::ifstream bucketFile;

    DurationLogger logger;
    std::string LogReadPathLoadSingleBlock = "ServerIOReadPathLoadSingleBlock";
    std::string LogReadPathRerandomizeBlock = "ServerComputationReadPathRerandomizeBlock";
    std::string LogReadPathWrtieBackBlock = "ServerIOReadPathWrtieBackBlock";
     std::string LogEvictionLoadPathBucketsFromDiskScheme2 = "ServerIOEvictionLoadPathBucketsFromDiskScheme2";

     std::string LogEarlyReshuffleLoadBucketFromDiskScheme2 = "ServerIOEarlyReshuffleLoadBucketFromDiskScheme2";
};


#endif // BUCKET_H