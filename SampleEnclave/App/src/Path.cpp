#include "Path.h"
#include <bitset>
#include <cassert>
Path::Path() {}
Path::Path(PathConfig::TYPE_PATH_ID id,
           PathConfig::TYPE_PATH_SIZE height) : id(id), height(height) {

            this->bIDs.reserve(height);
            ConvertPID2BIDs(id, height, this->bIDs);
            this->leafNum = std::pow(2, height - 1);
            this->block = Block();
            this->leafIDsTripletEviction.resize(3);
            for (int i = 0; i < 3; ++i) {
                this->leafIDsTripletEviction[i].resize(this->leafNum);
            }
}

Path::~Path() {}

void Path::ConvertPID2BIDs(PathConfig::TYPE_PATH_ID pid,
                           PathConfig::TYPE_PATH_SIZE height,
                           std::vector<BucketConfig::TYPE_BUCKET_ID>& bIDs) {
    bIDs.clear();
    bIDs.emplace_back(0);
    for (PathConfig::TYPE_PATH_SIZE i = 0; i < height - 1; ++i) {
        if (pid >> (height - 2 - i) & 1) {
            bIDs.emplace_back(2 * bIDs.back() + 2);
        } else {
            bIDs.emplace_back(2 * bIDs.back() + 1);
        }
    }
}

void Path::GenPath(PathConfig::TYPE_PATH_SIZE realBlockNum,
                   std::vector<BucketConfig::META_DATA>& metaDatas,
                   bool isClient) {
    
    // make sure the value of realBlockNum is less than the BucketConfig::BUCKET_REAL_BLOCK_CAPACITY * height
    // assert(realBlockNum < BucketConfig::BUCKET_REAL_BLOCK_CAPACITY * height && "The value of realBlockNum is out of range");
    metaDatas.clear();
    std::random_device rd;
    std::mt19937 g(rd());
    if (isClient) {
        // Generate the path for the client
        for (PathConfig::TYPE_PATH_SIZE i = 0; i < height; ++i) {
            BucketConfig::META_DATA md;
            BucketConfig::TYPE_BUCKET_ID j = i;
            while (j < realBlockNum) {
                // Generate a random leafID via a uniform distribution
                std::uniform_int_distribution<PathConfig::TYPE_PATH_ID> dis(0, leafNum - 1);
                PathConfig::TYPE_PATH_ID leafID = dis(g);
                md.AddRealBlock(j, leafID);
                j += height;
            }
            md.nextDummyIndex = BucketConfig::BUCKET_SIZE;
            metaDatas.emplace_back(md);
        }
    } else {
        // Generate the path for the server
        for (PathConfig::TYPE_PATH_SIZE i = 0; i < height; ++i) {
           Bucket bucket(this->bIDs[i], BucketConfig::BUCKET_SIZE, 
                        BucketConfig::BUCKET_REAL_BLOCK_CAPACITY, 
                        BucketConfig::BUCKET_DUMMY_BLOCK_CAPACITY,
                        BlockConfig::BLOCK_SIZE, ServerConfig::num_threads);
            #if READ_PATH_SIMULATION
                BucketConfig::TYPE_BUCKET_ID j = i;
                while (j < realBlockNum) {
                    std::vector<char> blockData;
                    this->block.GenData(BlockConfig::BLOCK_SIZE, true, j, blockData);
                    std::uniform_int_distribution<PathConfig::TYPE_PATH_ID> dis(0, leafNum - 1);
                    PathConfig::TYPE_PATH_ID leafID = dis(g);
                    bucket.AddRealBlock(j, leafID, blockData);
                    j += height;
                }
            #else
                if (i == height - 1) {
                    std::vector<char> blockData;
                    this->block.GenData(BlockConfig::BLOCK_SIZE, true, 0, blockData);
                    bucket.AddRealBlock(0, 0, blockData);
                }
            #endif

            while (bucket.md.nextDummyIndex < BucketConfig::BUCKET_SIZE) {
                std::vector<char> blockData;
                this->block.GenData(BlockConfig::BLOCK_SIZE, false, -1, blockData);
                bucket.AddDummyBlock(blockData);
            }
            metaDatas.emplace_back(bucket.md);
            bucket.SaveData2Disk(BucketConfig::DATADIR, BucketConfig::BUCKETPREFIX + std::to_string(this->bIDs[i]));
        }
    }

}


void Path::GenRootBucket(BucketConfig::TYPE_BUCKET_ID rootBucketID,
                         BucketConfig::META_DATA& rootBucketMD,
                         BucketConfig::TYPE_BUCKET_SIZE realBlockNum,
                         bool isClient) {
    // assert(realBlockNum < BucketConfig::BUCKET_REAL_BLOCK_CAPACITY && "The value of realBlockNum is out of range");
    std::random_device rd;
    std::mt19937 g(rd());
    if (isClient) {
        BucketConfig::META_DATA md;
        BlockConfig::TYPE_BLOCK_ID j = 0;
        while (j < realBlockNum) {
            std::uniform_int_distribution<PathConfig::TYPE_PATH_ID> dis(0, leafNum - 1);
            PathConfig::TYPE_PATH_ID leafID = dis(g);
            md.AddRealBlock(j, leafID);
            j++;
        }
        md.nextDummyIndex = BucketConfig::BUCKET_SIZE;
        rootBucketMD = std::move(md);

    } else {
        Bucket bucket(rootBucketID, BucketConfig::BUCKET_SIZE,
                  BucketConfig::BUCKET_REAL_BLOCK_CAPACITY,
                  BucketConfig::BUCKET_DUMMY_BLOCK_CAPACITY,
                  BlockConfig::BLOCK_SIZE, ServerConfig::num_threads);
        for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < realBlockNum; ++i) {
            std::vector<char> blockData;
            BlockConfig::TYPE_BLOCK_ID j = i;
            this->block.GenData(BlockConfig::BLOCK_SIZE, true, j, blockData);
            std::uniform_int_distribution<PathConfig::TYPE_PATH_ID> dis(0, leafNum - 1);
            PathConfig::TYPE_PATH_ID leafID = dis(g);
            bucket.AddRealBlock(j, leafID, blockData);
        }
        while (bucket.md.nextDummyIndex < BucketConfig::BUCKET_SIZE) {
            std::vector<char> blockData;
            this->block.GenData(BlockConfig::BLOCK_SIZE, false, -1, blockData);
            bucket.AddDummyBlock(blockData);
        }
        rootBucketMD = std::move(bucket.md);
        bucket.SaveData2Disk(BucketConfig::DATADIR, BucketConfig::BUCKETPREFIX + std::to_string(rootBucketID));
    }

    
}

void Path::GenTripletBuckets(std::vector<BucketConfig::TYPE_BUCKET_ID>& tripletBucketIDs,
                      std::vector<BucketConfig::META_DATA>& tripletBucketMDs,
                      BucketConfig::TYPE_BUCKET_SIZE realBlockNum,
                      bool isClient) {
    assert(tripletBucketIDs.size() == 3 && "The size of tripletBucketIDs is not equal to 3");
    std::random_device rd;
    std::mt19937 g(rd());
    for (auto& leafIDs : this->leafIDsTripletEviction) {
        for (auto& leafID : leafIDs) {
            std::uniform_int_distribution<PathConfig::TYPE_PATH_ID> dis(0, leafNum - 1);
            leafID = dis(g);
        }
    }
    if (isClient) {
        for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < 3; ++i) {
            BucketConfig::META_DATA md;
            BlockConfig::TYPE_BLOCK_ID j = 0;
            while (j < realBlockNum) {
                md.AddRealBlock(j + i * realBlockNum, this->leafIDsTripletEviction[i][j]);
                j++;
            }
            md.nextDummyIndex = BucketConfig::BUCKET_SIZE;
            tripletBucketMDs.emplace_back(md);
        }
    } else {
        for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < 3; ++i) {
            Bucket bucket(tripletBucketIDs[i], BucketConfig::BUCKET_SIZE,
                          BucketConfig::BUCKET_REAL_BLOCK_CAPACITY,
                          BucketConfig::BUCKET_DUMMY_BLOCK_CAPACITY,
                          BlockConfig::BLOCK_SIZE, ServerConfig::num_threads);
            for (BucketConfig::TYPE_BUCKET_SIZE j = 0; j < realBlockNum; ++j) {
                this->block.GenData(BlockConfig::BLOCK_SIZE, true, j + i * realBlockNum, this->blockDataEviction);
                bucket.AddRealBlock(j, this->leafIDsTripletEviction[i][j], this->blockDataEviction);
            }
            while (bucket.md.nextDummyIndex < BucketConfig::BUCKET_SIZE) {
                this->block.GenData(BlockConfig::BLOCK_SIZE, false, -1, this->blockDataEviction);
                bucket.AddDummyBlock(this->blockDataEviction);
            }
            bucket.SaveData2Disk(BucketConfig::DATADIR, BucketConfig::BUCKETPREFIX + std::to_string(tripletBucketIDs[i]));
        }
        
    }

}

void Path::GenEvictPath(PathConfig::TYPE_PATH_ID path_id, PathConfig::TYPE_PATH_SIZE height) {
    for (const auto& bID : this->bIDs) {
        Bucket bucket(bID, BucketConfig::BUCKET_SIZE,
                      BucketConfig::BUCKET_REAL_BLOCK_CAPACITY,
                      BucketConfig::BUCKET_DUMMY_BLOCK_CAPACITY,
                      BlockConfig::BLOCK_SIZE, ServerConfig::num_threads);
        while (bucket.md.nextDummyIndex < BucketConfig::BUCKET_SIZE) {
            this->block.GenData(BlockConfig::BLOCK_SIZE, false, -1, this->blockDataEviction);
            bucket.AddDummyBlock(this->blockDataEviction);
        }
        bucket.SaveData2Disk(BucketConfig::DATADIR, BucketConfig::BUCKETPREFIX + std::to_string(bID));
    }
}