#ifndef PATH_H
#define PATH_H  
#include "config.h"
#include "Bucket.h"
#include "Block.h"
class Path {
public:
    Path();
    Path(PathConfig::TYPE_PATH_ID id, PathConfig::TYPE_PATH_SIZE height); // The realBlockNum is less than the BucketConfig::BUCKET_REAL_BLOCK_CAPACITY * height
    ~Path();

    // properties
    PathConfig::TYPE_PATH_ID id;
    PathConfig::TYPE_PATH_SIZE height;
    std::vector<BucketConfig::TYPE_BUCKET_ID> bIDs;
    PathConfig::TYPE_PATH_SIZE leafNum;

    // Other classes
    Block block;
    Bucket bucket_eviction;
    

    // methods
    void GenPath(PathConfig::TYPE_PATH_SIZE realBlockNum, std::vector<BucketConfig::META_DATA>& metaDatas, bool isClient = false);
    void ConvertPID2BIDs(PathConfig::TYPE_PATH_ID pid, PathConfig::TYPE_PATH_SIZE height, std::vector<BucketConfig::TYPE_BUCKET_ID>& bIDs);
    void GenRootBucket(BucketConfig::TYPE_BUCKET_ID rootBucketID, BucketConfig::META_DATA& rootBucketMD, BucketConfig::TYPE_BUCKET_SIZE realBlockNum, bool isClient = false);
    void GenTripletBuckets(std::vector<BucketConfig::TYPE_BUCKET_ID>& tripletBukcetIDs, std::vector<BucketConfig::META_DATA>& tripletBukcetMDs, BucketConfig::TYPE_BUCKET_SIZE realBlockNum, bool isClient = false);
    void GenEvictPath(PathConfig::TYPE_PATH_ID path_id, PathConfig::TYPE_PATH_SIZE height);

    // variables
    std::vector<char> blockDataEviction;
    PathConfig::TYPE_PATH_ID leafIDEviction;
    std::vector<std::vector<PathConfig::TYPE_PATH_ID>> leafIDsTripletEviction;
    BlockConfig::TYPE_BLOCK_ID blockID = 0;
};

#endif // PATH_H