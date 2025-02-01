#include "Bucket.h"
#include <cassert>
#include <filesystem>

Bucket::Bucket() {
    data.resize(BucketConfig::BUCKET_SIZE * BlockConfig::BLOCK_SIZE);
    this->id = -1;
    this->bucketSize = BucketConfig::BUCKET_SIZE;
    this->realBlockCapacity = BucketConfig::BUCKET_REAL_BLOCK_CAPACITY;
    this->dummyBlockCapacity = BucketConfig::BUCKET_DUMMY_BLOCK_CAPACITY;
    this->blockSize = BlockConfig::BLOCK_SIZE;
    // this->num_threads = (BucketConfig::TYPE_THREAD_NUM)std::thread::hardware_concurrency();
    this->num_threads = ServerConfig::num_threads;
    elgamal = ElGamal_parallel_ntl(this->num_threads, this->blockSize);
    this->blockData.resize(this->blockSize);
    this->dummyBlock.reserve(this->blockSize);
    this->block.GenData(this->blockSize, false, -1, this->dummyBlock);
    this->elgamal.ParallelEncrypt(this->dummyBlock, this->dummyBlockCiphertexts);
    this->ciphertextsData.reserve(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
    this->ciphertextsData.resize(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
    this->logger = DurationLogger(LogConfig::LOG_DIR + LogConfig::LOG_FILE);
}

Bucket::Bucket(BucketConfig::TYPE_BUCKET_ID id, 
               BucketConfig::TYPE_BUCKET_SIZE bucketSize,
               BucketConfig::TYPE_BUCKET_SIZE realBlockCapacity,
               BucketConfig::TYPE_BUCKET_SIZE dummyBlockCapacity,
                BlockConfig::TYPE_BLOCK_SIZE blockSize,
                BucketConfig::TYPE_THREAD_NUM num_threads
               ) : id(id), 
               bucketSize(bucketSize),
               realBlockCapacity(realBlockCapacity),
               dummyBlockCapacity(dummyBlockCapacity),
               blockSize(blockSize),
               num_threads(num_threads),
               elgamal(num_threads, blockSize),
               blockData(blockSize) {
    data.resize(bucketSize * this->blockSize);
    this->dummyBlock.reserve(this->blockSize);
    this->block.GenData(this->blockSize, false, -1, this->dummyBlock);
    this->elgamal.ParallelEncrypt(this->dummyBlock, this->dummyBlockCiphertexts);
    this->logger = DurationLogger(LogConfig::LOG_DIR + LogConfig::LOG_FILE);
    
   
}

Bucket::~Bucket() {
    // data.clear();
}

void Bucket::SetBucketID(BucketConfig::TYPE_BUCKET_ID id) {
    this->id = id;
}

void Bucket::AddRealBlock(BlockConfig::TYPE_BLOCK_ID blockID,
                          PathConfig::TYPE_PATH_ID leafID,
                          std::vector<char>& blockData) {
    // assert(md.nextRealIndex < realBlockCapacity && "Error: Real block capacity exceeded");
    // check the blockData size with the BlockConfig::BLOCK_SIZE
    // assert(blockData.size() == BlockConfig::BLOCK_SIZE && "Error: Block data size mismatch");
    md.AddRealBlock(blockID, leafID);
    std::copy(blockData.begin(), blockData.end(), data.begin() + md.offsets[md.nextRealIndex - 1] * this->blockSize);
    // test above copy method
    // BlockConfig::TYPE_BLOCK_ID blockID_test;
    // std::memcpy(&blockID_test, data.data() + md.offsets[md.nextRealIndex - 1] * this->blockSize, sizeof(blockID_test));
    // assert(blockID == blockID_test && "Error: Block ID mismatch");
}

void Bucket::AddDummyBlock(std::vector<char>& blockData) {
    // assert(md.nextDummyIndex < this->bucketSize && "Error: Dummy block capacity exceeded");
    // check the blockData size with the BlockConfig::BLOCK_SIZE
    // assert(blockData.size() == BlockConfig::BLOCK_SIZE && "Error: Block data size mismatch");
    std::copy(blockData.begin(), blockData.end(), data.begin() + md.offsets[md.nextDummyIndex] * this->blockSize);
    md.nextDummyIndex++;
}

void Bucket::PrintBlockIDs() {
    for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < this->realBlockCapacity; i++) {
        BlockConfig::TYPE_BLOCK_ID blockID;
        std::memcpy(&blockID, data.data() + md.offsets[i] * this->blockSize, sizeof(blockID));
        std::cout << blockID << " ";
    }
    std::cout << std::endl;
}
/**
 * @brief Save the encrypted bucket data to disk
 */
void Bucket::SaveData2Disk(const std::string& dirPath,
                           const std::string& fileName) {
    // Check if the directory exists
    std::filesystem::path path(dirPath);
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path); 
    }
    // Save the this->data to the file namely fileName
    std::ofstream bucketFile(dirPath + "/" + fileName, std::ios::binary);
    for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < this->bucketSize; i++) {
        // Encrypt the block data before saving to disk via ElGamal_parallel_ntl
        std::copy(data.begin() + i * this->blockSize, data.begin() + (i + 1) * this->blockSize, this->blockData.begin());
        // BlockConfig::TYPE_BLOCK_ID blockID;
        // std::memcpy(&blockID, this->blockData.data(), sizeof(blockID));
        // std::cout << "blockID: " << blockID << std::endl;
        // elgamal.ParallelEncrypt(this->blockData, ciphertexts);
        elgamal.ParallelEncrypt(this->blockData, this->ciphertexts_ZZ_p);
        // elgamal.SerializeCiphertexts(ciphertexts, this->ciphertextsData);
        elgamal.SerializeCiphertexts(this->ciphertexts_ZZ_p, this->ciphertextsData);
        // this->ciphertextsDataSize = this->ciphertextsData.size();
        // bucketFile.write(reinterpret_cast<const char*>(&this->ciphertextsDataSize), sizeof(this->ciphertextsDataSize));
        bucketFile.write(this->ciphertextsData.data(), this->ciphertextsData.size());
        // std::vector<std::pair<ZZ_p, ZZ_p>> ciphertexts_test;
        // elgamal.DeserializeCiphertexts(this->ciphertextsData, ciphertexts_test);
        // std::vector<char> decrypted_data;
        // elgamal.ParallelDecrypt(ciphertexts_test, decrypted_data);
        // BlockConfig::TYPE_BLOCK_ID blockID_test;
        // std::memcpy(&blockID_test, decrypted_data.data(), sizeof(blockID_test));
        // std::cout << "blockID_test: " << blockID_test << std::endl;
    }
    bucketFile.close();
}
/**
 * @brief Load the encrypted bucket data from disk
 */

void Bucket::LoadDataFromDisk(const std::string& dirPath,
                      const std::string& fileName,
                      std::vector<std::vector<std::pair<ZZ, ZZ>>>& bucketCiphertexts) {
    FileConfig::bucketFileLoad.open(dirPath + "/" + fileName, std::ios::binary);
    // data.clear();
    // data.shrink_to_fit();
    for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < this->bucketSize; i++) {
        FileConfig::bucketFileLoad.read(reinterpret_cast<char*>(&this->ciphertextsDataSize), sizeof(this->ciphertextsDataSize));
        this->ciphertextsData.resize(this->ciphertextsDataSize);
        FileConfig::bucketFileLoad.read(this->ciphertextsData.data(), this->ciphertextsDataSize);
        elgamal.DeserializeCiphertexts(this->ciphertextsData, this->ciphertexts);
        // bucketCiphertexts.emplace_back(this->ciphertexts);
        bucketCiphertexts.emplace_back(std::vector<std::pair<ZZ, ZZ>>(this->ciphertexts));
        // Decrypt the block data after loading from disk via ElGamal_parallel_ntl
        // elgamal.ParallelDecrypt(this->ciphertexts, this->blockData);
        // data.insert(data.end(), this->blockData.begin(), this->blockData.end());
    }
    FileConfig::bucketFileLoad.close();
}

/**
 * @brief Load the encrypted bucket data from disk with ZZ_p ciphertexts
 */
void Bucket::LoadDataFromDisk(const std::string& dirPath,
                      const std::string&fileName,
                      std::vector<std::vector<std::pair<ZZ_p, ZZ_p>>>& bucketCiphertexts) {
    // bucketCiphertexts.resize(this->bucketSize);
    #if USE_ASSERT
    assert(bucketCiphertexts.size() == this->bucketSize && "Error: bucketCiphertexts size mismatch");
    #endif
    FileConfig::bucketFileLoad.open(dirPath + "/" + fileName, std::ios::binary);
    for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < this->bucketSize; i++) {
        // bucketFile.read(reinterpret_cast<char*>(&this->ciphertextsDataSize), sizeof(this->ciphertextsDataSize));    
        // this->ciphertextsData.resize(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
        #if LOG_EVICT_BREAKDOWN_COST_SERVER
        this->logger.startTiming(this->LogEvictionLoadPathBucketsFromDiskScheme2);
        #endif
        FileConfig::bucketFileLoad.read(this->ciphertextsData.data(), ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
        #if LOG_EVICT_BREAKDOWN_COST_SERVER
        this->logger.stopTiming(this->LogEvictionLoadPathBucketsFromDiskScheme2);
        this->logger.writeToFile();
        #endif
        elgamal.DeserializeCiphertexts(this->ciphertextsData, this->ciphertexts_ZZ_p);
        // bucketCiphertexts.emplace_back(std::vector<std::pair<ZZ_p, ZZ_p>>(this->ciphertexts_ZZ_p));
        // bucketCiphertexts[i] = std::vector<std::pair<ZZ_p, ZZ_p>>(this->ciphertexts_ZZ_p);
        // use std::move to avoid copy
        bucketCiphertexts[i] = std::move(this->ciphertexts_ZZ_p);
    }
    FileConfig::bucketFileLoad.close();
}

void Bucket::LoadDataFromDiskSupportMT(const std::string& dirPath,
                                       const std::string& fileName,
                                       std::vector<std::vector<std::pair<ZZ_p, ZZ_p>>>& bucketCiphertexts) {
    bucketCiphertexts.resize(this->bucketSize);
    FileConfig::bucketFileLoad.open(dirPath + "/" + fileName, std::ios::binary);
    for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < this->bucketSize; i++) {
        // this->ciphertextsData.resize(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
        std::vector<char> ciphertextsData(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
        FileConfig::bucketFileLoad.read(ciphertextsData.data(), ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
        std::vector<std::pair<ZZ_p, ZZ_p>> ciphertexts_ZZ_p;
        elgamal.DeserializeCiphertexts(ciphertextsData, ciphertexts_ZZ_p);
        bucketCiphertexts[i] = std::move(ciphertexts_ZZ_p);
    }
    FileConfig::bucketFileLoad.close();
}

void Bucket::LoadDataFromDiskWithOffset(const std::string& dirPath,
                                        const std::string& fileName,
                                        std::vector<std::vector<std::pair<ZZ, ZZ>>>& bucketCiphertexts,
                                        std::vector<BucketConfig::TYPE_SLOT_ID>& offsets) {
    bucketCiphertexts.resize(this->bucketSize);
    std::ifstream bucketFile(dirPath + "/" + fileName, std::ios::binary);
    for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < this->bucketSize; i++) {
        bucketFile.read(reinterpret_cast<char*>(&this->ciphertextsDataSize), sizeof(this->ciphertextsDataSize));
        this->ciphertextsData.resize(this->ciphertextsDataSize);
        bucketFile.read(this->ciphertextsData.data(), this->ciphertextsDataSize);
        elgamal.DeserializeCiphertexts(this->ciphertextsData, this->ciphertexts);
        bucketCiphertexts[offsets[i]] = std::vector<std::pair<ZZ, ZZ>>(this->ciphertexts);
    }

}


/*
* @brief Load the encrypted bucket data from disk with ZZ_p ciphertexts and offsets
*/

void Bucket::LoadDataFromDiskWithOffset(const std::string& dirPath,
                                        const std::string& fileName,
                                        std::vector<std::vector<std::pair<ZZ_p, ZZ_p>>>& bucketCiphertexts,
                                        std::vector<BucketConfig::TYPE_SLOT_ID>& offsets) {
    bucketCiphertexts.resize(this->bucketSize);
    std::ifstream bucketFile(dirPath + "/" + fileName, std::ios::binary);
    for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < this->bucketSize; i++) {
        // std::cout << "The offset: " << offsets[i] << std::endl;
        // bucketFile.read(reinterpret_cast<char*>(&this->ciphertextsDataSize), sizeof(this->ciphertextsDataSize));
        this->ciphertextsData.resize(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
         #if LOG_BREAKDOWN_COST
        this->logger.startTiming(this->LogEarlyReshuffleLoadBucketFromDiskScheme2);
        #endif
        bucketFile.read(this->ciphertextsData.data(), ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
        #if LOG_BREAKDOWN_COST
        this->logger.stopTiming(this->LogEarlyReshuffleLoadBucketFromDiskScheme2);
        this->logger.writeToFile();
        #endif
        elgamal.DeserializeCiphertexts(this->ciphertextsData, this->ciphertexts_ZZ_p);
        bucketCiphertexts[offsets[i]] = std::vector<std::pair<ZZ_p, ZZ_p>>(this->ciphertexts_ZZ_p);
    }
    bucketFile.close();
}

void Bucket::LoadTripletDataFromDiskWithOffset(const std::string& dirPath,
                                            const std::string& fileName,
                                            std::vector<BucketConfig::TYPE_BUCKET_ID>& tripletBucketIDs,
                                            std::vector<std::vector<std::pair<ZZ_p, ZZ_p>>>& bucketCiphertexts,
                                            std::vector<BucketConfig::TYPE_SLOT_ID>& offsets) {
    for (const auto& id : tripletBucketIDs) {
        std::ifstream bucketFile(dirPath + "/" + fileName + std::to_string(id), std::ios::binary);
        for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < this->bucketSize; ++i) {
            bucketFile.read(this->ciphertextsData.data(), ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
            this->elgamal.DeserializeCiphertexts(this->ciphertextsData, this->ciphertexts_ZZ_p);
            bucketCiphertexts[offsets[i]] = std::vector<std::pair<ZZ_p, ZZ_p>>(this->ciphertexts_ZZ_p);
        }
        bucketFile.close();
    }
}

void Bucket::LoadSingleBlockFromDisk(const std::string& dirPath,
                                     const std::string& fileName,
                                     const BucketConfig::TYPE_SLOT_ID& blockIndex,
                                     std::vector<std::pair<ZZ, ZZ>>& blockCiphertexts) {
    std::ifstream bucketFile(dirPath + "/" + fileName, std::ios::binary);
    size_t ciphertextsDataSize;
    for (BucketConfig::TYPE_SLOT_ID i = 0; i < blockIndex; i++) {
        bucketFile.read(reinterpret_cast<char*>(&ciphertextsDataSize), sizeof(ciphertextsDataSize));
        bucketFile.seekg(ciphertextsDataSize, std::ios::cur);
    }
    bucketFile.read(reinterpret_cast<char*>(&ciphertextsDataSize), sizeof(ciphertextsDataSize));
    std::vector<char> ciphertextsData(ciphertextsDataSize);
    bucketFile.read(ciphertextsData.data(), ciphertextsDataSize);
    elgamal.DeserializeCiphertexts(ciphertextsData, blockCiphertexts);
    bucketFile.close();
}

/*
* @brief Load the encrypted block data from disk with ZZ_p ciphertexts
*/

void Bucket::LoadSingleBlockFromDisk(const std::string& dirPath,
                                     const std::string& fileName,
                                     const BucketConfig::TYPE_SLOT_ID& blockIndex,
                                     std::vector<std::pair<ZZ_p, ZZ_p>>& blockCiphertexts) {
    std::ifstream bucketFile(dirPath + "/" + fileName, std::ios::binary);
    // std::cout << "The blockIndex: " << blockIndex << std::endl;
    // ClientConfig::TYPE_CHAR_SIZE ciphertextsDataSize;
    for (BucketConfig::TYPE_SLOT_ID i = 0; i < blockIndex; i++) {
        bucketFile.seekg(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS, std::ios::cur);
    }
    std::vector<char> ciphertextsData(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
    bucketFile.read(ciphertextsData.data(), ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
    // std::cout << "The size of the ciphertextsData: " << ciphertextsData.size() << std::endl;
    elgamal.DeserializeCiphertexts(ciphertextsData, blockCiphertexts);
    // std::cout << "The size of the blockCiphertexts: " << blockCiphertexts.size() << std::endl;
}

/*
* @brief Load the encrypted block data from disk with ZZ_p ciphertexts and update the blockCiphertexts in dummy
*/

void Bucket::LoadSingleBlockFromDiskWithUpdate(const std::string& dirPath,
                                              const std::string& fileName,
                                              const BucketConfig::TYPE_SLOT_ID& blockIndex,
                                              std::vector<std::pair<ZZ_p, ZZ_p>>& blockCiphertexts) {
    std::fstream bucketFile(dirPath + "/" + fileName, std::ios::in | std::ios::out | std::ios::binary);
    std::streampos blockPos = blockIndex * ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS;
    bucketFile.seekg(blockPos);
    std::vector<char> ciphertextsData(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
    #if LOG_BREAKDOWN_COST
    this->logger.startTiming(this->LogReadPathLoadSingleBlock);
    #endif
    bucketFile.read(ciphertextsData.data(), ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
    #if LOG_BREAKDOWN_COST
    this->logger.stopTiming(this->LogReadPathLoadSingleBlock);
    this->logger.writeToFile();
    #endif
    elgamal.DeserializeCiphertexts(ciphertextsData, blockCiphertexts);
    // go back to the beginning of the block and create a new dummy block ciphertexts to cover it
    bucketFile.seekg(-ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS, std::ios::cur);
    std::vector<std::pair<ZZ_p, ZZ_p>> dummyBlockCiphertexts_copy(this->dummyBlockCiphertexts);
    elgamal.ParallelRerandomize(dummyBlockCiphertexts_copy);
    std::vector<char> dummyBlockCiphertextsData;
    elgamal.SerializeCiphertexts(dummyBlockCiphertexts_copy, dummyBlockCiphertextsData);
    #if LOG_BREAKDOWN_COST
    this->logger.startTiming(this->LogReadPathWrtieBackBlock);
    #endif
    bucketFile.write(dummyBlockCiphertextsData.data(), dummyBlockCiphertextsData.size());
    #if LOG_BREAKDOWN_COST
    this->logger.stopTiming(this->LogReadPathWrtieBackBlock);
    this->logger.writeToFile();
    #endif
    bucketFile.close();
}