#ifndef TREE_H
#define TREE_H
#include "config.h"
#include "Block.h"
#include "AES_CTR.h"    
#include "Path.h"

class Tree {
public:
    Tree(PathConfig::TYPE_PATH_SIZE h,
         BucketConfig::TYPE_BUCKET_SIZE tnb,
         BucketConfig::TYPE_BUCKET_SIZE tnnlb,
         BucketConfig::TYPE_BUCKET_SIZE tnlb,
         BlockConfig::TYPE_BLOCK_SIZE rbn);
    ~Tree();
    void GenTree(std::vector<BucketConfig::META_DATA>& metaDatas,
            bool isClient);
    void GenTree(std::unordered_map<BucketConfig::TYPE_BUCKET_ID, BucketConfig::META_DATA>& metaDatas,
            bool isClient);
    void GenPathMDs(std::unordered_map<BucketConfig::TYPE_BUCKET_ID, BucketConfig::META_DATA>& metaDatas,
            PathConfig::TYPE_PATH_ID pathID);
    void GenEvictPathMDs(std::unordered_map<BucketConfig::TYPE_BUCKET_ID, BucketConfig::META_DATA>& metaDatas,
            PathConfig::TYPE_PATH_ID pathID);

    void GenTreeWithMDs();
    void GenPathwithMDs(PathConfig::TYPE_PATH_ID pathID);
    void GenSingleBucketWithMD(BucketConfig::TYPE_BUCKET_ID bucketID);
    void GenEvictPathWithMDs(PathConfig::TYPE_PATH_ID pathID);
    void EncryptMD(std::vector<char>& serializedMDData, BucketConfig::TYPE_BUCKET_ID bucketID);
    void EncryptMD(void* serializedMDData, BucketConfig::TYPE_BUCKET_ID bucketID);
    void DecryptMD(std::vector<char>& serializedMDData, BucketConfig::TYPE_BUCKET_ID bucketID);
    void DecryptMD(void* serializedMDData, BucketConfig::TYPE_BUCKET_ID bucketID);
    ClientConfig::TYPE_CHAR_SIZE encryptedMDSize = BucketConfig::META_DATA_SIZE - sizeof(size_t) - BucketConfig::BUCKET_SIZE;
    ClientConfig::TYPE_CHAR_SIZE plainMDSize = sizeof(size_t) + BucketConfig::BUCKET_SIZE;


private:
    PathConfig::TYPE_PATH_SIZE height;
    BucketConfig::TYPE_BUCKET_SIZE total_num_buckets;
    BucketConfig::TYPE_BUCKET_SIZE total_num_non_leaf_buckets;
    BucketConfig::TYPE_BUCKET_SIZE total_num_leaf_buckets;
    BlockConfig::TYPE_BLOCK_SIZE real_block_num;
    std::vector<PathConfig::TYPE_PATH_ID> pathIDs;
    BlockConfig::TYPE_BLOCK_ID blockID;
    Block block;
    std::vector<char> blockData;
    std::vector<char> blockDummyData;
    std::vector<char> blockEncryptedData;
    std::vector<char> blockRealData;

    BucketConfig::META_DATA md;
    std::vector<char> SerializedDataMD;
    AES_CTR aes;
    unsigned char iv[AES_BLOCK_SIZE];

    std::ofstream ofs;
    std::vector<char> bucketDataScheme1;


    std::vector<char> bucketPartialMDData;
    Path path;

};

#endif // TREE_H