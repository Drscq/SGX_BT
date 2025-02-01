#include "Tree.h"
#include "Bucket.h" 

Tree::Tree(PathConfig::TYPE_PATH_SIZE h,
           BucketConfig::TYPE_BUCKET_SIZE tnb,
           BucketConfig::TYPE_BUCKET_SIZE tnnlb,
           BucketConfig::TYPE_BUCKET_SIZE tnlb,
           BlockConfig::TYPE_BLOCK_SIZE rbn) :
           height(h),
           total_num_buckets(tnb),
           total_num_non_leaf_buckets(tnnlb),
           total_num_leaf_buckets(tnlb),
           real_block_num(rbn) {

    this->pathIDs.reserve(this->real_block_num);
    this->pathIDs.resize(this->real_block_num);
    // std::random_device rd;
    // std::mt19937 g(rd());
    for (BlockConfig::TYPE_BLOCK_SIZE i = 0; i < this->real_block_num; ++i) {
        this->pathIDs[i] = i % this->total_num_leaf_buckets;
    }
    this->block = Block();
    this->blockDummyData.reserve(BlockConfig::BLOCK_SIZE);
    this->block.GenData(BlockConfig::BLOCK_SIZE, false, -1, this->blockDummyData);
    this->SerializedDataMD.resize(BucketConfig::META_DATA_SIZE);
    this->blockEncryptedData.reserve(BlockConfig::BLOCK_SIZE);
    this->blockEncryptedData.resize(BlockConfig::BLOCK_SIZE);
    this->blockRealData.reserve(BlockConfig::BLOCK_SIZE);
    std::filesystem::path path(BucketConfig::DATADIR);
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path); 
    }
    this->bucketDataScheme1.reserve(BucketConfig::BUCKET_SIZE * BlockConfig::BLOCK_SIZE + BucketConfig::META_DATA_SIZE);
    this->bucketDataScheme1.resize(BucketConfig::BUCKET_SIZE * BlockConfig::BLOCK_SIZE + BucketConfig::META_DATA_SIZE);
    this->bucketPartialMDData.reserve(encryptedMDSize);
    this->bucketPartialMDData.resize(encryptedMDSize);

};

Tree::~Tree() {

};

void Tree::GenTree(std::vector<BucketConfig::META_DATA>& metaDatas,
              bool isClient = false) {   
    metaDatas.clear();    
    if (isClient) {
        for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < this->total_num_non_leaf_buckets; ++i) {
            BucketConfig::META_DATA md;
            md.shiftNextDummyIndex2End();
            metaDatas.emplace_back(md);
        }
        for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < this->total_num_leaf_buckets; ++i) {
            BucketConfig::META_DATA md;
            this->blockID = i;
            while (this->blockID < this->real_block_num) {
                md.AddRealBlock(this->blockID, this->pathIDs[this->blockID]);
                this->blockID = (this->blockID + this->total_num_leaf_buckets);
            }
            md.shiftNextDummyIndex2End();
            metaDatas.emplace_back(md);
        }
    } else {
        for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < this->total_num_non_leaf_buckets; ++i) {
            Bucket bucket(i, BucketConfig::BUCKET_SIZE,
                          BucketConfig::BUCKET_REAL_BLOCK_CAPACITY,
                          BucketConfig::BUCKET_DUMMY_BLOCK_CAPACITY,
                          BlockConfig::BLOCK_SIZE, ServerConfig::num_threads);
            while (bucket.md.nextDummyIndex < BucketConfig::BUCKET_SIZE) {
                this->block.GenData(BlockConfig::BLOCK_SIZE, false, -1, this->blockData);
                bucket.AddDummyBlock(blockData);
            }
            bucket.SaveData2Disk(BucketConfig::DATADIR, BucketConfig::BUCKETPREFIX + std::to_string(i));
        }
        for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < this->total_num_leaf_buckets; ++i) {
            Bucket bucket(i + this->total_num_non_leaf_buckets, BucketConfig::BUCKET_SIZE,
                          BucketConfig::BUCKET_REAL_BLOCK_CAPACITY,
                          BucketConfig::BUCKET_DUMMY_BLOCK_CAPACITY,
                          BlockConfig::BLOCK_SIZE, ServerConfig::num_threads);
            this->blockID = i;
            while (this->blockID < this->real_block_num) {
                this->block.GenData(BlockConfig::BLOCK_SIZE, true, this->blockID, this->blockData);
                bucket.AddRealBlock(this->blockID, this->pathIDs[this->blockID], this->blockData);
                this->blockID = this->blockID + this->total_num_leaf_buckets;
            }
            while (bucket.md.nextDummyIndex < BucketConfig::BUCKET_SIZE) {
                this->block.GenData(BlockConfig::BLOCK_SIZE, false, -1, this->blockData);
                bucket.AddDummyBlock(this->blockData);
            }
            bucket.SaveData2Disk(BucketConfig::DATADIR, BucketConfig::BUCKETPREFIX + std::to_string(i + this->total_num_non_leaf_buckets));
        }
    }

}

void Tree::GenTree(std::unordered_map<BucketConfig::TYPE_BUCKET_ID, BucketConfig::META_DATA>& metaDatas,
                bool isClient) {
    metaDatas.clear();
    if (isClient) {
        for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < this->total_num_non_leaf_buckets; ++i) {
            BucketConfig::META_DATA md;
            md.shiftNextDummyIndex2End();
            metaDatas[i] = md;
        }
        for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < this->total_num_leaf_buckets; ++i) {
            BucketConfig::META_DATA md;
            this->blockID = i;
            while (this->blockID < this->real_block_num) {
                md.AddRealBlock(this->blockID, this->pathIDs[this->blockID]);
                this->blockID = (this->blockID + this->total_num_leaf_buckets);
            }
            md.shiftNextDummyIndex2End();
            metaDatas[i + this->total_num_non_leaf_buckets] = md;
        }
    }
}

void Tree::GenPathMDs(std::unordered_map<BucketConfig::TYPE_BUCKET_ID, BucketConfig::META_DATA>& metaDatas,
                  PathConfig::TYPE_PATH_ID pathID) {
    this->path = Path(pathID, this->height);
    for (PathConfig::TYPE_PATH_SIZE i = 0; i < this->height - 1; ++i) {
        BucketConfig::META_DATA md;
        md.shiftNextDummyIndex2End();
        metaDatas[this->path.bIDs[i]] = md;
    }
    BucketConfig::META_DATA md;
    this->blockID = 0;
    md.AddRealBlock(this->blockID, 0);
    md.shiftNextDummyIndex2End();
    metaDatas[this->path.bIDs[this->height - 1]] = md;
}

void Tree::GenEvictPathMDs(std::unordered_map<BucketConfig::TYPE_BUCKET_ID, BucketConfig::META_DATA>& metaDatas,
                       PathConfig::TYPE_PATH_ID pathID) {
    this->path = Path(pathID, this->height);
    BucketConfig::META_DATA rootMD;
    metaDatas[0] = rootMD;
    for (const auto& bID : this->path.bIDs) {
        for (BucketConfig::TYPE_SMALL_INDEX_U i = 0; i < 2; ++i) {
            BucketConfig::META_DATA md;
            md.shiftNextDummyIndex2End();
            metaDatas[bID * 2 + i + 1] = md;
        }
    }
}

void Tree::GenPathwithMDs(PathConfig::TYPE_PATH_ID pathID) {
    this->path = Path(pathID, this->height);
    for (PathConfig::TYPE_PATH_SIZE i = 0; i < this->height - 1; ++i) {
        this->md.shiftNextDummyIndex2End();
        this->md.Serialize(this->SerializedDataMD);
        this->EncryptMD(this->SerializedDataMD, this->path.bIDs[i]);
        this->ofs.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(this->path.bIDs[i]), std::ios::binary);
        this->ofs.write(this->SerializedDataMD.data(), BucketConfig::META_DATA_SIZE);
        for (BucketConfig::TYPE_BUCKET_SIZE j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
            std::memset(iv, this->path.bIDs[i] + j, AES_BLOCK_SIZE);
            this->aes.encrypt(reinterpret_cast<const unsigned char*>(this->blockDummyData.data()),
                                BlockConfig::BLOCK_SIZE,
                                reinterpret_cast<unsigned char*>(this->blockEncryptedData.data()),
                                iv);
            this->ofs.write(this->blockEncryptedData.data(), BlockConfig::BLOCK_SIZE);
        }
        this->ofs.close();
    }
    this->md.Initialize();
    this->blockID = 0;
    this->md.AddRealBlock(this->blockID, 0);
    this->md.shiftNextDummyIndex2End();
    this->md.Serialize(this->SerializedDataMD);
    this->EncryptMD(this->SerializedDataMD, this->path.bIDs[this->height - 1]);
    this->ofs.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(this->path.bIDs[this->height - 1]), std::ios::binary);
    this->ofs.write(this->SerializedDataMD.data(), BucketConfig::META_DATA_SIZE);
    for (BucketConfig::TYPE_BUCKET_SIZE j = 0; j < this->md.nextRealIndex; ++j) {
        std::memset(iv, this->path.bIDs[this->height - 1] + this->md.offsets[j], AES_BLOCK_SIZE);
        this->block.GenData(BlockConfig::BLOCK_SIZE, true, this->md.addrs[j], this->blockRealData);
        this->aes.encrypt(reinterpret_cast<const unsigned char*>(this->blockRealData.data()),
                            BlockConfig::BLOCK_SIZE,
                            reinterpret_cast<unsigned char*>(this->blockEncryptedData.data()),
                            iv);
        this->ofs.seekp(BucketConfig::META_DATA_SIZE + this->md.offsets[j] * BlockConfig::BLOCK_SIZE, std::ios::beg);
        this->ofs.write(this->blockEncryptedData.data(), BlockConfig::BLOCK_SIZE);
    }
    for (BucketConfig::TYPE_BUCKET_SIZE j = this->md.nextRealIndex; j < BucketConfig::BUCKET_SIZE; ++j) {
        std::memset(iv, this->path.bIDs[this->height - 1] + this->md.offsets[j], AES_BLOCK_SIZE);
        this->aes.encrypt(reinterpret_cast<const unsigned char*>(this->blockDummyData.data()),
                            BlockConfig::BLOCK_SIZE,
                            reinterpret_cast<unsigned char*>(this->blockEncryptedData.data()),
                            iv);
        this->ofs.seekp(BucketConfig::META_DATA_SIZE + this->md.offsets[j] * BlockConfig::BLOCK_SIZE, std::ios::beg);
        this->ofs.write(this->blockEncryptedData.data(), BlockConfig::BLOCK_SIZE);
    }
    this->ofs.close();
}

void Tree::GenEvictPathWithMDs(PathConfig::TYPE_PATH_ID pathID) {
    BucketConfig::TYPE_BUCKET_ID rootBucketID = 0;
    FileConfig::fileWriteScheme1.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(rootBucketID), std::ios::binary);
    this->md.shiftNextDummyIndex2End();
    this->md.Serialize(this->SerializedDataMD);
    this->EncryptMD(this->SerializedDataMD, 0);
    FileConfig::fileWriteScheme1.write(this->SerializedDataMD.data(), BucketConfig::META_DATA_SIZE);
    for (BucketConfig::TYPE_SLOT_ID j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
        std::memset(iv, j, AES_BLOCK_SIZE);
        this->aes.encrypt(reinterpret_cast<const unsigned char*>(this->blockDummyData.data()),
                            BlockConfig::BLOCK_SIZE,
                            reinterpret_cast<unsigned char*>(this->blockEncryptedData.data()),
                            iv);
        FileConfig::fileWriteScheme1.write(this->blockEncryptedData.data(), BlockConfig::BLOCK_SIZE);
    }
    FileConfig::fileWriteScheme1.close();
    this->path = Path(pathID, this->height);
    for (PathConfig::TYPE_PATH_SIZE i = 0; i < this->height - 1; ++i) {
        FileConfig::fileWriteScheme1.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(this->path.bIDs[i] * 2 + 1), std::ios::binary);
        this->md.shiftNextDummyIndex2End();
        this->md.Serialize(this->SerializedDataMD);
        this->EncryptMD(this->SerializedDataMD, this->path.bIDs[i] * 2 + 1);
        FileConfig::fileWriteScheme1.write(this->SerializedDataMD.data(), BucketConfig::META_DATA_SIZE);
        for (BucketConfig::TYPE_SLOT_ID j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
            std::memset(iv, this->path.bIDs[i] * 2 + 1 + j, AES_BLOCK_SIZE);
            this->aes.encrypt(reinterpret_cast<const unsigned char*>(this->blockDummyData.data()),
                                BlockConfig::BLOCK_SIZE,
                                reinterpret_cast<unsigned char*>(this->blockEncryptedData.data()),
                                iv);
            FileConfig::fileWriteScheme1.write(this->blockEncryptedData.data(), BlockConfig::BLOCK_SIZE);
        }
        FileConfig::fileWriteScheme1.close();
        FileConfig::fileWriteScheme1.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(this->path.bIDs[i] * 2 + 2), std::ios::binary);
        this->md.shiftNextDummyIndex2End();
        this->md.Serialize(this->SerializedDataMD);
        this->EncryptMD(this->SerializedDataMD, this->path.bIDs[i] * 2 + 2);
        FileConfig::fileWriteScheme1.write(this->SerializedDataMD.data(), BucketConfig::META_DATA_SIZE);
        for (BucketConfig::TYPE_SLOT_ID j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
            std::memset(iv, this->path.bIDs[i] * 2 + 2 + j, AES_BLOCK_SIZE);
            this->aes.encrypt(reinterpret_cast<const unsigned char*>(this->blockDummyData.data()),
                                BlockConfig::BLOCK_SIZE,
                                reinterpret_cast<unsigned char*>(this->blockEncryptedData.data()),
                                iv);
            FileConfig::fileWriteScheme1.write(this->blockEncryptedData.data(), BlockConfig::BLOCK_SIZE);
        }
        FileConfig::fileWriteScheme1.close();
    }
}

void Tree::GenSingleBucketWithMD(BucketConfig::TYPE_BUCKET_ID bucketID) {
    FileConfig::fileWriteScheme1.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(bucketID), std::ios::binary);
    this->md.shiftNextDummyIndex2End();
    this->md.Serialize(this->SerializedDataMD);
    this->EncryptMD(this->SerializedDataMD, bucketID);
    FileConfig::fileWriteScheme1.write(this->SerializedDataMD.data(), BucketConfig::META_DATA_SIZE);
    for (BucketConfig::TYPE_SLOT_ID j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
        std::memset(iv, bucketID + j, AES_BLOCK_SIZE);
        this->aes.encrypt(reinterpret_cast<const unsigned char*>(this->blockDummyData.data()),
                            BlockConfig::BLOCK_SIZE,
                            reinterpret_cast<unsigned char*>(this->blockEncryptedData.data()),
                            iv);
        FileConfig::fileWriteScheme1.write(this->blockEncryptedData.data(), BlockConfig::BLOCK_SIZE);
    }
    FileConfig::fileWriteScheme1.close();
}

void Tree::GenTreeWithMDs() {
    // For non-leaf buckets, fill them up with dummy blocks
    for (BucketConfig::TYPE_BUCKET_ID i = 0; i < this->total_num_non_leaf_buckets; ++i) {
        this->md.shiftNextDummyIndex2End();
        this->md.Serialize(this->SerializedDataMD);
        this->EncryptMD(this->SerializedDataMD, i);
        // this->DecryptMD(this->SerializedDataMD, i);
        this->ofs.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(i), std::ios::binary);
        this->ofs.write(this->SerializedDataMD.data(), BucketConfig::META_DATA_SIZE);
        for (BucketConfig::TYPE_BUCKET_SIZE j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
            std::memset(iv, i + j, AES_BLOCK_SIZE);
            this->aes.encrypt(reinterpret_cast<const unsigned char*>(this->blockDummyData.data()),
                                BlockConfig::BLOCK_SIZE,
                                reinterpret_cast<unsigned char*>(this->blockEncryptedData.data()),
                                iv);
            this->ofs.write(this->blockEncryptedData.data(), BlockConfig::BLOCK_SIZE);
        }
        this->ofs.close();
    }
    // For leaf buckets, fill them up with corresponding real blocks and dummy blocks
    for (BucketConfig::TYPE_BUCKET_ID i = 0; i < this->total_num_leaf_buckets; ++i) {
        this->blockID = i;
        this->md.Initialize();
        while (this->blockID < this->real_block_num) {
            this->md.AddRealBlock(this->blockID, i);
            this->blockID = this->blockID + this->total_num_leaf_buckets;
        }
        this->md.shiftNextDummyIndex2End();
        this->md.Serialize(this->SerializedDataMD);
        this->EncryptMD(this->SerializedDataMD, i + this->total_num_non_leaf_buckets);
        this->ofs.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(i + this->total_num_non_leaf_buckets), std::ios::binary);
        this->ofs.write(this->SerializedDataMD.data(), BucketConfig::META_DATA_SIZE);
        for (BucketConfig::TYPE_SLOT_ID j = 0; j < this->md.nextRealIndex; ++j) {
            std::memset(iv, i + this->md.offsets[j] + this->total_num_non_leaf_buckets, AES_BLOCK_SIZE);
            this->block.GenData(BlockConfig::BLOCK_SIZE, true, this->md.addrs[j], this->blockRealData);
            this->aes.encrypt(reinterpret_cast<const unsigned char*>(this->blockRealData.data()),
                                BlockConfig::BLOCK_SIZE,
                                reinterpret_cast<unsigned char*>(this->blockEncryptedData.data()),
                                iv);
            // move the file pointer to the position of the current + this->md.offsets[j] block
            this->ofs.seekp(BucketConfig::META_DATA_SIZE + this->md.offsets[j] * BlockConfig::BLOCK_SIZE, std::ios::beg);
            this->ofs.write(this->blockEncryptedData.data(), BlockConfig::BLOCK_SIZE);
        }

        for (BucketConfig::TYPE_SLOT_ID j = this->md.nextRealIndex; j < BucketConfig::BUCKET_SIZE; ++j) {
            std::memset(iv, i + this->md.offsets[j] + this->total_num_non_leaf_buckets, AES_BLOCK_SIZE);
            this->aes.encrypt(reinterpret_cast<const unsigned char*>(this->blockDummyData.data()),
                                BlockConfig::BLOCK_SIZE,
                                reinterpret_cast<unsigned char*>(this->blockEncryptedData.data()),
                                iv);
            // move the file pointer to the position of the current + this->md.offsets[j] block
            this->ofs.seekp(BucketConfig::META_DATA_SIZE + this->md.offsets[j] * BlockConfig::BLOCK_SIZE, std::ios::beg);
            this->ofs.write(this->blockEncryptedData.data(), BlockConfig::BLOCK_SIZE);
        }
        this->ofs.close();
        // This version is not efficient, but it is easier to understand.
        // The efficient version is to use a buffer to store the data and write them to the file at once.
    }
}


void Tree::EncryptMD(std::vector<char>& serializedMDData, BucketConfig::TYPE_BUCKET_ID bucketID) {
    std::memset(iv, bucketID, AES_BLOCK_SIZE);
    std::memcpy(this->bucketPartialMDData.data(), serializedMDData.data() + this->plainMDSize, this->encryptedMDSize);
    this->aes.encrypt(reinterpret_cast<const unsigned char*>(this->bucketPartialMDData.data()),
                        this->encryptedMDSize,
                        reinterpret_cast<unsigned char*>(serializedMDData.data() + this->plainMDSize),
                        iv);
}

void Tree::EncryptMD(void* serializedMDData, BucketConfig::TYPE_BUCKET_ID bucketID) {
    // convert the void* to char* via static_cast<char*>(serializedMDData)
    // static_cast<char*>(serializedMDData);
    std::memset(iv, bucketID, AES_BLOCK_SIZE);
    std::memcpy(this->bucketPartialMDData.data(), static_cast<char*>(serializedMDData) + this->plainMDSize, this->encryptedMDSize);
    this->aes.encrypt(reinterpret_cast<const unsigned char*>(this->bucketPartialMDData.data()),
                        this->encryptedMDSize,
                        reinterpret_cast<unsigned char*>(static_cast<char*>(serializedMDData) + this->plainMDSize),
                        iv);
}


void Tree::DecryptMD(std::vector<char>& serializedMDData, BucketConfig::TYPE_BUCKET_ID bucketID) {
    std::memset(iv, bucketID, AES_BLOCK_SIZE);
    this->aes.decrypt(reinterpret_cast<const unsigned char*>(serializedMDData.data() + this->plainMDSize),
                        this->encryptedMDSize,
                        reinterpret_cast<unsigned char*>(this->bucketPartialMDData.data()),
                        iv);
    std::memcpy(serializedMDData.data() + this->plainMDSize, this->bucketPartialMDData.data(), this->encryptedMDSize);
    
}

void Tree::DecryptMD(void* serializedMDData, BucketConfig::TYPE_BUCKET_ID bucketID) {
    // convert the void* to char* via static_cast<char*>(serializedMDData)
    static_cast<char*>(serializedMDData);
    std::memset(iv, bucketID, AES_BLOCK_SIZE);
    this->aes.decrypt(reinterpret_cast<const unsigned char*>(static_cast<char*>(serializedMDData) + this->plainMDSize),
                        this->encryptedMDSize,
                        reinterpret_cast<unsigned char*>(this->bucketPartialMDData.data()),
                        iv);
    std::memcpy(static_cast<char*>(serializedMDData) + this->plainMDSize, this->bucketPartialMDData.data(), this->encryptedMDSize);
}
