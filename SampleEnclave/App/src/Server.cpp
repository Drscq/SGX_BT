clear
#include "Server.h"
#include <unistd.h>
#include "Bucket.h"
#include <future>
#include "DurationLogger.h"
#include <cassert>
#include "Path.h"
#include <thread>
Server::Server(ServerConfig::TYPE_PORT_NUM port) : port(port),
                                                  tripletBucketIDs{0, 1, 2},
                                                  realBlockNumForEviction(BucketConfig::BUCKET_REAL_BLOCK_CAPACITY),
                                                  tree(TreeConfig::HEIGHT, TreeConfig::TOTAL_NUM_BUCKETS, 
                                                       TreeConfig::TOTAL_NUM_NON_LEAF_BUCKETS, TreeConfig::TOTAL_NUM_LEAF_BUCKETS, 
                                                       TreeConfig::REAL_BLOCK_NUM),
                                                       path(0, TreeConfig::HEIGHT) {
    // std::cout << "Server constructor" << std::endl;
    this->pathIDChars.resize(sizeof(PathConfig::TYPE_PATH_ID));
    this->pathIDChars.reserve(sizeof(PathConfig::TYPE_PATH_ID));
    this->blockIDChars.resize(sizeof(BlockConfig::TYPE_BLOCK_ID));
    this->blockIDChars.reserve(sizeof(BlockConfig::TYPE_BLOCK_ID));
    this->offsets.resize(PathConfig::HEIGHT);
    this->offsetsCharsSize = PathConfig::HEIGHT * sizeof(BucketConfig::TYPE_SLOT_ID);
    this->offsetsChars.resize(this->offsetsCharsSize);
    Block block;
    std::vector<char> blockData;
    block.GenData(BlockConfig::BLOCK_SIZE, false, -1, blockData);
    this->elgamal = ElGamal_parallel_ntl(ServerConfig::num_threads, BlockConfig::BLOCK_SIZE);
    this->elgamal.ParallelEncrypt(blockData, this->targetBlockCiphertexts);
    this->originalTargetBlockCiphertexts = this->targetBlockCiphertexts;
    this->resultCiphertexts.reserve(this->targetBlockCiphertexts.size());
    this->permDataSize = BucketConfig::BUCKET_SIZE * sizeof(BucketConfig::TYPE_SLOT_ID);
    this->permData2.resize(this->permDataSize);
    this->perm2.resize(BucketConfig::BUCKET_SIZE);
    #if !LOG_READ_PATH_TOTAL_DELAY
    this->InitConnectThirdParty();
    #endif
    // std::cout << "The BLOCK_CIPHERTEXT_NUM_CHARS: " << ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS << std::endl;
    this->blockCiphertextsSerializedData.reserve(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
    this->blockCiphertextsSerializedData.resize(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
    // std::cout << "The BUCKET_CIPHERTEXT_NUM_CHARS: " << ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS << std::endl;
    this->bucketCiphertextsSerializedData.reserve(ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
    this->bucketCiphertextsSerializedData.resize(ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
    this->rootBucketData.resize(ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
    this->triplet_evict_perm2.resize(3 * BucketConfig::BUCKET_SIZE);
    // this->triplet_evict_perm_size = 3 * BucketConfig::BUCKET_SIZE * sizeof(BucketConfig::TYPE_SLOT_ID);
    this->path_evict_bucketCiphertexts_complete.reserve((TreeConfig::HEIGHT - 1) * 2 * BucketConfig::BUCKET_SIZE);
    this->path_evict_bucketCiphertexts_complete.resize((TreeConfig::HEIGHT - 1) * 2 * BucketConfig::BUCKET_SIZE);
    for (auto& inner_vector : this->path_evict_bucketCiphertexts_complete) {
        inner_vector.reserve(ElGamalNTLConfig::BLOCK_CHUNK_SIZE);
    }
    this->triplet_evict_bucketCiphertexts_flat.reserve(3 * BucketConfig::BUCKET_SIZE * ElGamalNTLConfig::BLOCK_CHUNK_SIZE);
    this->bucketCiphertexts_flat.reserve(BucketConfig::BUCKET_SIZE * ElGamalNTLConfig::BLOCK_CHUNK_SIZE);
    this->triplet_evict_bucketCiphertextsSerializedData.reserve(3 * ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
    this->triplet_evict_bucketCiphertextsSerializedData.resize(3 * ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
    this->triplet_evict_bucketCiphertextsSerializedDataSize = 3 * ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS;
    this->pathCompleteOffsets.reserve(TreeConfig::HEIGHT);
    this->pathCompleteOffsets.resize(TreeConfig::HEIGHT);
    this->pathCompleteOffsetsSize = TreeConfig::HEIGHT * sizeof(BucketConfig::TYPE_SLOT_ID);
    this->triplet_evict_perm_size = 3 * BucketConfig::BUCKET_SIZE * sizeof(BucketConfig::TYPE_SLOT_ID);
    this->triplet_evict_permData2.reserve(this->triplet_evict_perm_size);
    this->tripletEvictPermComplete.reserve(3 * BucketConfig::BUCKET_SIZE);
    this->tripletEvictPermComplete.resize(3 * BucketConfig::BUCKET_SIZE);
    // this->perm1EarlyReshuffleComplete.reserve(BucketConfig::BUCKET_SIZE);
    // this->perm1EarlyReshuffleComplete.resize(BucketConfig::BUCKET_SIZE);
    this->perm2EarlyReshuffleComplete.reserve(BucketConfig::BUCKET_SIZE);
    this->perm2EarlyReshuffleComplete.resize(BucketConfig::BUCKET_SIZE);
    this->bucketCiphertexts.resize(BucketConfig::BUCKET_SIZE);
    this->rootBucketDataEvictComplete.reserve(ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
    this->rootBucketDataEvictComplete.resize(ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
    this->bucketCiphertextsEvictComplete.reserve(BucketConfig::BUCKET_SIZE);
    this->bucketCiphertextsEvictComplete.resize(BucketConfig::BUCKET_SIZE);
    this->tripletBucketIDsComplete.reserve(3);
    this->tripletBucketIDsComplete.resize(3);
    this->tripletBucketIDsCompleteCharNum = 3 * sizeof(BucketConfig::TYPE_BUCKET_ID);
    this->evictPathBucketIDsComplete.reserve(TreeConfig::HEIGHT - 1);
    this->evictPathBucketIDsComplete.resize(TreeConfig::HEIGHT - 1);
    this->targetBlockCiphertextsSerializedData.reserve(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
    this->targetBlockCiphertextsSerializedData.resize(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);

    this->logger = DurationLogger(LogConfig::LOG_DIR + LogConfig::LOG_FILE);

    this->pathBucketMDDataSize1 = BucketConfig::META_DATA_SIZE * TreeConfig::HEIGHT;
    this->pathBucketMDData1.resize(this->pathBucketMDDataSize1);
    this->pathBlocksDataSize1 = TreeConfig::HEIGHT * BlockConfig::BLOCK_SIZE;
    this->pathBlocksData1.resize(this->pathBlocksDataSize1);
    this->targetBlockCipherData1.resize(BlockConfig::BLOCK_SIZE);
    this->bucketMDataEarlyReshuffle1.resize(BucketConfig::META_DATA_SIZE);
    this->bucketRealDataEarlyReshuffle1.resize(BucketConfig::BUCKET_REAL_BLOCK_CAPACITY
                                                * BlockConfig::BLOCK_SIZE);
    this->realBlocksOffsetEarlyReshuffle1Size = BucketConfig::BUCKET_REAL_BLOCK_CAPACITY
                                                * sizeof(BucketConfig::TYPE_SLOT_ID);
    this->realBlocksOffsetEarlyReshuffle1.resize(BucketConfig::BUCKET_REAL_BLOCK_CAPACITY); 
    this->realBlocksDataEarlyReshuffle1.resize(BucketConfig::BUCKET_REAL_BLOCK_CAPACITY
                                                * BlockConfig::BLOCK_SIZE);
    this->bucketCipherDataEarlyReshuffle1Size = BucketConfig::META_DATA_SIZE
                                                + BucketConfig::BUCKET_SIZE * BlockConfig::BLOCK_SIZE;
    this->bucketCipherDataEarlyReshuffle1.resize(this->bucketCipherDataEarlyReshuffle1Size);
    this->pathBucketsDataEviction1Size = (2 * TreeConfig::HEIGHT - 1) * (BucketConfig::META_DATA_SIZE
                                        + BucketConfig::BUCKET_SIZE * BlockConfig::BLOCK_SIZE);
    this->pathBucketsDataEviction1.resize(this->pathBucketsDataEviction1Size);
    this->pathBucketsDataEviction1.reserve(this->pathBucketsDataEviction1Size);
    this->bucketSizeEviction1 = BucketConfig::META_DATA_SIZE + BucketConfig::BUCKET_SIZE * BlockConfig::BLOCK_SIZE;
    this->tripletBucketMDsDataEviction1Size = 3 * BucketConfig::META_DATA_SIZE + sizeof(BucketConfig::TYPE_BUCKET_ID) + sizeof(PathConfig::TYPE_PATH_SIZE);
    this->tripletBucketMDsDataEviction1.resize(this->tripletBucketMDsDataEviction1Size);
    this->tripletBucketRealBlocksOffsetEviction1.resize(3 * BucketConfig::BUCKET_REAL_BLOCK_CAPACITY);
    this->tripletBucketRealBlocksOffsetEviction1Size = 3 * BucketConfig::BUCKET_REAL_BLOCK_CAPACITY * sizeof(BucketConfig::TYPE_SLOT_ID);
    this->tripletBucketsRealBlocksDataEviction1Size = 3 * BucketConfig::BUCKET_REAL_BLOCK_CAPACITY * BlockConfig::BLOCK_SIZE;
    this->tripletBucketsRealBlocksDataEviction1.resize(this->tripletBucketsRealBlocksDataEviction1Size);
    this->tripletBucketsDataEviction1Size = 3 * this->bucketSizeEviction1;
    this->tripletBucketsDataEviction1.resize(this->tripletBucketsDataEviction1Size);
    this->bucketCiphertextsNumPairs = ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS / (2 * ElGamalNTLConfig::PER_CIPHERTEXT_SIZE);
    this->single_bucketCiphertexts_flat.resize(this->bucketCiphertextsNumPairs);
    this->single_bucketCiphertexts_flat.reserve(this->bucketCiphertextsNumPairs);
    this->blockCiphertextsNumPairs = ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS / (2 * ElGamalNTLConfig::PER_CIPHERTEXT_SIZE);
    this->triplet_evict_bucketCiphertexts.resize(3 * BucketConfig::BUCKET_SIZE);
    for (auto& inner_vector : this->triplet_evict_bucketCiphertexts) {
        inner_vector.resize(blockCiphertextsNumPairs);
        inner_vector.reserve(blockCiphertextsNumPairs);
    }
}
Server::~Server() {
    // std::cout << "Server destructor" << std::endl;
}

void Server::Start() {
    if (!communicator.listenOnPort(port)) {
        std::cerr << "Error: Server failed to start" << std::endl;
        return;
    }
    std::cout << "Server started on port " << port << std::endl;
    while (true) {
        auto clientSockfd = communicator.acceptConnection();
        if (clientSockfd < 0) {
            std::cerr << "Error: Server failed to accept connection" << std::endl;
            return;
        }
        handleClient(clientSockfd);
    }
    this->communicator.~SocketCommunicator();
}

void Server::handleClient(int clientSockfd) {
    while (true) {
        if (!this->communicator.receiveCommand(clientSockfd, this->command)) {
            std::cerr << "Error: Server failed to receive command" << std::endl;
            return;
        } else {
            if (this->command == ServerConfig::CMD_CREATE_DB) {
                this->communicator.receiveData(clientSockfd, this->pathIDChars, sizeof(PathConfig::TYPE_PATH_ID));
                std::memcpy(&this->pathID, this->pathIDChars.data(), sizeof(PathConfig::TYPE_PATH_ID));
                std::vector<BucketConfig::META_DATA> metaDatas;
                this->path = Path(this->pathID, PathConfig::HEIGHT);
                path.GenPath(PathConfig::REAL_BLOCK_NUM, metaDatas, false);
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
            } else if (this->command == ServerConfig::CMD_READ_PATH) {
                this->communicator.receiveData(clientSockfd, reinterpret_cast<char*>(&this->pathID), sizeof(this->pathID));
                this->communicator.receiveData(clientSockfd,
                                              this->offsets.data(),
                                                this->offsetsCharsSize);
                #if USE_COUT
                    std::cout << "Offset elements: ";
                    for (auto& offset : this->offsets) {
                        std::cout << offset << " ";
                    }
                    std::cout << std::endl;
                #endif
                // this->targetBlockCiphertexts = this->originalTargetBlockCiphertexts;
                // std::vector<char> blockData;
                // this->elgamal.ParallelDecrypt(this->targetBlockCiphertexts, blockData);
                // BlockConfig::TYPE_BLOCK_ID blockID;
                // std::memcpy(&blockID, blockData.data(), sizeof(BlockConfig::TYPE_BLOCK_ID));
                // std::cout << "Block ID: " << blockID << std::endl;
                // The following codes is without the multi-threading technique
                // for (PathConfig::TYPE_PATH_SIZE i = 0; i < PathConfig::HEIGHT; i++) {
                //     std::vector<std::pair<ZZ_p, ZZ_p>> blockCiphertexts;
                //     // Hoestly this part can be accelerated by the multi-threading technique
                //     bucket.LoadSingleBlockFromDisk(BucketConfig::DATADIR,
                //                                     BucketConfig::BUCKETPREFIX + std::to_string(this->path.bIDs[i]),
                //                                     this->offsets[i],
                //                                     blockCiphertexts);
                //     // std::vector<char> blockData;
                //     // this->elgamal.ParallelDecrypt(blockCiphertexts, blockData);
                //     // BlockConfig::TYPE_BLOCK_ID blockID;
                //     // std::memcpy(&blockID, blockData.data(), sizeof(blockID));
                //     // std::cout << "Block ID: " << blockID << std::endl;
                //     elgamal.ParallelMultiplyCiphertexts(blockCiphertexts, this->targetBlockCiphertexts, resultCiphertexts);
                //     this->targetBlockCiphertexts = std::move(resultCiphertexts);
                //     // this->elgamal.SerializeCiphertexts(this->targetBlockCiphertexts, this->targetBlockCiphertextsSerializedData);
                //     // std::vector<char> blockData;
                //     // this->elgamal.ParallelDecrypt(this->targetBlockCiphertexts, blockData);
                //     // BlockConfig::TYPE_BLOCK_ID blockID;
                //     // std::memcpy(&blockID, blockData.data(), sizeof(BlockConfig::TYPE_BLOCK_ID));
                //     // std::cout << "Block ID: " << blockID << std::endl;
                // }
                // The following codes is with the multi-threading technique
                this->targetBlockCiphertexts = this->originalTargetBlockCiphertexts;
                std::vector<std::future<std::vector<std::pair<ZZ_p, ZZ_p>>>> futures;
                for (PathConfig::TYPE_PATH_SIZE i = 0; i < PathConfig::HEIGHT; i++) {
                    // Launch async task for each block load, returning blockCiphertexts
                    futures.push_back(
                        std::async(
                            std::launch::async, [&, i]() -> std::vector<std::pair<ZZ_p, ZZ_p>> {
                                ZZ_p::init(ElGamalNTLConfig::P);  // Initialize ZZ_p
                                // std::vector<std::pair<ZZ, ZZ>> blockCiphertexts;
                                std::vector<std::pair<ZZ_p, ZZ_p>> blockCiphertexts;
                                bucket.LoadSingleBlockFromDisk(BucketConfig::DATADIR,
                                                                BucketConfig::BUCKETPREFIX + std::to_string(this->path.bIDs[i]),
                                                                this->offsets[i],
                                                                blockCiphertexts);
                                return blockCiphertexts;
                            }
                        )
                    );

                    // Ensure the number of futures does not exceed the number of threads in ServerConfig::num_threads
                    if (futures.size() >= ServerConfig::num_threads) {
                        auto blockCiphertexts = futures.front().get();
                        elgamal.ParallelMultiplyCiphertexts(blockCiphertexts, this->targetBlockCiphertexts, resultCiphertexts);
                        this->targetBlockCiphertexts = std::move(resultCiphertexts);
                        futures.erase(futures.begin());
                    }
                }

                // Process any remaining futures
                for (auto& future : futures) {
                    auto blockCiphertexts = future.get();
                    elgamal.ParallelMultiplyCiphertexts(blockCiphertexts, this->targetBlockCiphertexts, resultCiphertexts);
                    this->targetBlockCiphertexts = std::move(resultCiphertexts);
                }
                this->elgamal.SerializeCiphertexts(this->targetBlockCiphertexts, this->targetBlockCiphertextsSerializedData);
                // this->communicator.sendDataWithoutKnownSize(clientSockfd, this->targetBlockCiphertextsSerializedData);
                this->communicator.sendData(clientSockfd, this->targetBlockCiphertextsSerializedData.data(), this->targetBlockCiphertextsSerializedData.size());

            } else if (this->command == ServerConfig::CMD_EARLY_RESHUFFLE_INIT) {
                // std::cout << "Received command: CMD_EARLY_RESHUFFLE_INIT" << std::endl;
                this->path.GenRootBucket(0, this->rootBucketMD, BucketConfig::BUCKET_REAL_BLOCK_CAPACITY - 1, false);
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
            } else if (this->command == ServerConfig::CMD_EARLY_RESHUFFLE) {
                // std::cout << "Received command: CMD_EARLY_RESHUFFLE" << std::endl;
                this->communicator.receiveData(clientSockfd, this->perm2.data(), this->permDataSize);
                this->bucket.LoadDataFromDiskWithOffset(BucketConfig::DATADIR,
                                                        BucketConfig::BUCKETPREFIX + std::to_string(0),
                                                        this->bucketCiphertexts,
                                                        this->perm2);
                // Re-randomize the elements in the bucketCiphertexts
                for (auto& blockCiphertexts : this->bucketCiphertexts) {
                    this->elgamal.ParallelRerandomize(blockCiphertexts);
                }
                size_t offset = 0;
                for (auto &blockCiphertexts : this->bucketCiphertexts) {
                    this->elgamal.SerializeCiphertexts(blockCiphertexts, this->blockCiphertextsSerializedData);
                    std::memcpy(this->bucketCiphertextsSerializedData.data() + offset,
                                this->blockCiphertextsSerializedData.data(),
                                ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
                    offset += ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS;
                }
                this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(),
                                                          ServerConfig::CMD_EARLY_RESHUFFLE_SERVER_THRID_PARTY);
                // this->communicator2ThirdParty.sendDataWithoutKnownSize(this->communicator2ThirdParty.getSockfd(),
                                                                    // this->bucketCiphertextsSerializedData);
                this->communicator2ThirdParty.sendData(this->communicator2ThirdParty.getSockfd(),
                                                        this->bucketCiphertextsSerializedData.data(),
                                                        this->bucketCiphertextsSerializedData.size());
                // std::cout << "The size of the bucketCiphertextsSerializedData: " << this->bucketCiphertextsSerializedData.size() << std::endl;
                // this->communicator2ThirdParty.receiveDataWithoutKnownSize(this->communicator2ThirdParty.getSockfd(),
                //                                                     this->bucketCiphertextsSerializedData);
                this->communicator2ThirdParty.receiveData(this->communicator2ThirdParty.getSockfd(),
                                                        this->bucketCiphertextsSerializedData.data(),
                                                        ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
                // std::cout << "Received bucketCiphertextsSerializedData: " << this->bucketCiphertextsSerializedData.size() << std::endl;
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
            } else if (this->command == ServerConfig::CMD_EVICT_SERVER_INIT) {
                #if USE_COUT
                std::cout << "Received command: CMD_EVICT_SERVER" << std::endl;
                #endif
                this->path = Path(0, PathConfig::HEIGHT);
                #if EVICT_SIMULATION
                this->path.GenTripletBuckets(this->tripletBucketIDs, this->tripletBucketMDs, this->realBlockNumForEviction, false);
                // open the file of BukcetConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(0)
                // read the data into the rootBucketData
                std::ifstream ifs(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(0), std::ios::binary);
                ifs.read(this->rootBucketData.data(), ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
                ifs.close();
                #else
                this->path.GenEvictPath(0, PathConfig::HEIGHT);
                 // read the root bucket data to the rootBucketDataEvictComplete
                std::ifstream ifs(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(0), std::ios::binary);
                ifs.read(this->rootBucketDataEvictComplete.data(), ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
                ifs.close();
                #endif
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
            } else if (this->command == ServerConfig::CMD_EVICT_SERVER_OPERATION) {
                #if USE_COUT
                    std::cout << "Received command: CMD_EVICT_SERVER_OPERATION" << std::endl;
                #endif
                this->communicator.receiveData(clientSockfd, this->triplet_evict_perm2.data(), this->triplet_evict_perm_size);
                #if USE_COUT
                    std::cout << "Received triplet_evict_perm2: ";
                    for (auto& p : this->triplet_evict_perm2) {
                        std::cout << p << " ";
                    }
                    std::cout << std::endl;
                #endif
                // Load the triplet buckets data from disk with this->triplet_evict_perm2
                // DurationLogger logger(LogConfig::LOG_DIR + LogConfig::LOG_FILE);
                // logger.startTiming("ServerDiskIO:LoadTripletDataFromDiskWithOffset");
                this->bucket.LoadTripletDataFromDiskWithOffset(BucketConfig::DATADIR,
                                                        BucketConfig::BUCKETPREFIX,
                                                        this->tripletBucketIDs,
                                                        this->triplet_evict_bucketCiphertexts,
                                                        this->triplet_evict_perm2);
                // logger.stopTiming("ServerDiskIO:LoadTripletDataFromDiskWithOffset");
                // logger.writeToFile();
                // Re-randomize the elements in the triplet_evict_bucketCiphertexts
                // logger.startTiming("ServerComputation:ParallelReRandomizePure");
                for (auto& blockCiphertexts : this->triplet_evict_bucketCiphertexts) {
                    this->elgamal.ParallelRerandomize(blockCiphertexts);
                }
                // logger.stopTiming("ServerComputation:ParallelReRandomizePure");
                // logger.writeToFile();
                // Serialize the triplet_evict_bucketCiphertexts into triplet_evict_bucketCiphertextsSerializedData
                // logger.startTiming("ServerComputation:SerializeCiphertextsPure");
                size_t offset = 0;
                for (auto &blockCiphertexts : this->triplet_evict_bucketCiphertexts) {
                    this->elgamal.SerializeCiphertexts(blockCiphertexts, this->blockCiphertextsSerializedData);
                    std::memcpy(this->triplet_evict_bucketCiphertextsSerializedData.data() + offset,
                                this->blockCiphertextsSerializedData.data(),
                                ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
                    offset += ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS;
                }
                // logger.stopTiming("ServerComputation:SerializeCiphertextsPure");
                // logger.writeToFile();
                // std::cout << "The size of the triplet_evict_bucketCiphertexts: " << this->triplet_evict_bucketCiphertexts.size() << std::endl;
                // Send the triplet_evict_bucketCiphertextsSerializedData to the third party
                this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(),
                                                          ServerConfig::CMD_EVICT_FROM_SERVER_TO_THIRD_PARTY);
                // DurationLogger logger(LogConfig::LOG_DIR + LogConfig::LOG_FILE);
                // logger.startTiming("ServerComputation:SendTripletData2ThirdParty");
                this->communicator2ThirdParty.sendData(this->communicator2ThirdParty.getSockfd(),
                                                        this->triplet_evict_bucketCiphertextsSerializedData.data(),
                                                        this->triplet_evict_bucketCiphertextsSerializedData.size());
                this->communicator2ThirdParty.receiveCommand(this->communicator2ThirdParty.getSockfd(), this->command);
                // logger.stopTiming("ServerComputation:SendTripletData2ThirdParty");
                // std::cout << "The size of the triplet_evict_bucketCiphertextsSerializedData: " << this->triplet_evict_bucketCiphertextsSerializedData.size() << std::endl;
                this->communicator2ThirdParty.receiveData(this->communicator2ThirdParty.getSockfd(), this->triplet_evict_bucketCiphertextsSerializedData,
                                                this->triplet_evict_bucketCiphertextsSerializedDataSize);
                // std::cout << "Received triplet_evict_bucketCiphertextsSerializedData: " << this->triplet_evict_bucketCiphertextsSerializedData.size() << std::endl;
                // logger.startTiming("ServerDiskIO:WriteTripletData2Disk");
                // Write the triplet_evict_bucketCiphertextsSerializedData to the disk
                // for (int i = 0; i < 3; ++i) {
                //     std::ofstream ofs(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(this->tripletBucketIDs[i]), std::ios::binary);
                //     ofs.write(this->triplet_evict_bucketCiphertextsSerializedData.data() + i * ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS, ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
                //     ofs.close();
                // }
                //  Muti-theading version for above function
                std::vector<std::thread> threads;
                for (int i = 0; i < 3; ++i) {
                    threads.emplace_back([this, i]() {
                        std::ofstream ofs(
                            BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(this->tripletBucketIDs[i]),
                            std::ios::binary
                        );
                        ofs.write(
                            this->triplet_evict_bucketCiphertextsSerializedData.data() + i * ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS,
                            ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS
                        );
                        // No need to explicitly close; destructor will handle it.
                    });
                }

                // Wait for all threads to complete
                for (auto& thread : threads) {
                    thread.join();
                }
                // logger.stopTiming("ServerDiskIO:WriteTripletData2Disk");
                // logger.writeToFile();
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
            } else if (this->command == ServerConfig::CMD_EVICT_COMMUNICATION_ROOT_BUCKET) {
                // std::cout << "Received command: CMD_EVICT_COMMUNICATION_ROOT_BUCKET" << std::endl;
                this->communicator.sendData(clientSockfd, this->rootBucketData.data(), ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
                this->communicator.receiveData(clientSockfd, this->rootBucketData.data(), ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
                // this->communicator.sendData(clientSockfd, this->rootBucketData);
                // this->communicator.receiveData(clientSockfd, this->rootBucketData, ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
            } else if (this->command == ServerConfig::CMD_COMPLETE_CREATE_BINARY_TREE) {
                // std::cout << "Received command: CMD_COMPLETE_CREATE_BINARY_TREE" << std::endl;
                this->tree.GenTree(this->treeMetaDatas, false);
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
            } else if (this->command == ServerConfig::CMD_COMPLETE_READ_PATH) {
                // std::cout << "Received command: CMD_COMPLETE_READ_PATH" << std::endl;
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                this->communicator.receiveData(clientSockfd, 
                                                reinterpret_cast<char*>(&this->pathIDComplete),
                                                sizeof(this->pathIDComplete));
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                #if LOG_BREAKDOWN_COST
                this->logger.startTiming(this->LogReadPathGenBucketIDsScheme2);
                #endif
                this->path.ConvertPID2BIDs(this->pathIDComplete, TreeConfig::HEIGHT, this->bucketIDOffsets);
                #if LOG_BREAKDOWN_COST
               this->logger.stopTiming(this->LogReadPathGenBucketIDsScheme2);
               this->logger.writeToFile();
                #endif
                // get the bucket IDs of the pathIDComplete
                this->communicator.receiveData(clientSockfd, 
                                                this->pathCompleteOffsets.data(),
                                                this->pathCompleteOffsetsSize);
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                // #if LOG_BREAKDOWN_COST
                // start = std::chrono::high_resolution_clock::now();
                // #endif
                this->targetBlockCiphertexts = this->originalTargetBlockCiphertexts;
                #if MULTI_THREAD_SWITCH 
                std::vector<std::future<std::vector<std::pair<ZZ_p, ZZ_p>>>> futures;
                for (PathConfig::TYPE_PATH_SIZE i = 0; i < TreeConfig::HEIGHT; i++) {
                    // Launch async task for each block load, returning blockCiphertexts
                    futures.push_back(
                        std::async(
                            std::launch::async, [&, i]() -> std::vector<std::pair<ZZ_p, ZZ_p>> {
                                ZZ_p::init(ElGamalNTLConfig::P);  // Initialize ZZ_p
                                std::vector<std::pair<ZZ_p, ZZ_p>> blockCiphertexts;
                                bucket.LoadSingleBlockFromDiskWithUpdate(BucketConfig::DATADIR,
                                                                BucketConfig::BUCKETPREFIX + std::to_string(this->bucketIDOffsets[i]),
                                                                this->pathCompleteOffsets[i],
                                                                blockCiphertexts);
                                return blockCiphertexts;
                            }
                        )
                    );

                    // Ensure the number of futures does not exceed the number of threads in ServerConfig::num_threads
                    if (futures.size() >= ServerConfig::num_threads) {
                        auto blockCiphertexts = futures.front().get();
                        elgamal.ParallelMultiplyCiphertexts(blockCiphertexts, this->targetBlockCiphertexts, resultCiphertexts);
                        this->targetBlockCiphertexts = std::move(resultCiphertexts);
                        futures.erase(futures.begin());
                    }
                }

                // Process any remaining futures
                for (auto& future : futures) {
                    auto blockCiphertexts = future.get();
                    elgamal.ParallelMultiplyCiphertexts(blockCiphertexts, this->targetBlockCiphertexts, resultCiphertexts);
                    this->targetBlockCiphertexts = std::move(resultCiphertexts);
                }
                #else
                for (PathConfig::TYPE_PATH_SIZE i = 0; i < TreeConfig::HEIGHT; ++i) {
                    // std::vector<std::pair<ZZ_p, ZZ_p>> blockCiphertexts;
                    bucket.LoadSingleBlockFromDiskWithUpdate(BucketConfig::DATADIR,
                                                            BucketConfig::BUCKETPREFIX + std::to_string(this->bucketIDOffsets[i]),
                                                            this->pathCompleteOffsets[i],
                                                            this->blockCiphertextsScheme2);
                    #if LOG_BREAKDOWN_COST
                    this->logger.startTiming(this->LogReadPathMultiplyCiphertextsScheme2);
                    #endif
                    elgamal.ParallelMultiplyCiphertexts(this->blockCiphertextsScheme2, this->targetBlockCiphertexts, resultCiphertexts);
                    #if LOG_BREAKDOWN_COST
                    this->logger.stopTiming(this->LogReadPathMultiplyCiphertextsScheme2);
                    this->logger.writeToFile();
                    #endif
                    this->targetBlockCiphertexts = std::move(resultCiphertexts);
                }
                #endif
               
                this->elgamal.SerializeCiphertexts(this->targetBlockCiphertexts, this->targetBlockCiphertextsSerializedData);
                #if LOG_BREAKDOWN_COST
                this->logger.startTiming(this->LogReadPathSendCiphertextScheme2);
                #endif
                this->communicator.sendData(clientSockfd,
                                            this->targetBlockCiphertextsSerializedData.data(),
                                            ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
                this->communicator.receiveCommand(clientSockfd, this->cmd1);
                #if LOG_BREAKDOWN_COST
                this->logger.stopTiming(this->LogReadPathSendCiphertextScheme2);
                this->logger.writeToFile();
                #endif
                
            } else if (this->command == ServerConfig::CMD_COMPLETE_EARLY_RESHUFFLE) {
                // std::cout << "Received command: CMD_COMPLETE_EARLY_RESHUFFLE" << std::endl;
                this->communicator.receiveData(clientSockfd,
                                                this->perm2EarlyReshuffleComplete.data(),
                                                this->permDataSize);
                this->communicator.receiveData(clientSockfd,
                                                reinterpret_cast<char*>(&this->bucketIDEarlyReshuffleComplete),
                                                sizeof(this->bucketIDEarlyReshuffleComplete));
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                this->bucket.LoadDataFromDiskWithOffset(BucketConfig::DATADIR,
                                                        BucketConfig::BUCKETPREFIX + std::to_string(this->bucketIDEarlyReshuffleComplete),
                                                        this->bucketCiphertexts,
                                                        this->perm2EarlyReshuffleComplete);

                // Alternative way to load the data from disk
                // this->bucket.LoadDataFromDisk(BucketConfig::DATADIR,
                //                             BucketConfig::BUCKETPREFIX + std::to_string(this->bucketIDEarlyReshuffleComplete),
                //                             this->bucketCiphertexts);
                // BucketConfig::ApplyPerm(this->bucketCiphertexts, this->perm2EarlyReshuffleComplete); 
                #if USE_COUT
                    // recover the blocks from the bucketCiphertexts
                    std::cout << "Get it from disk" << std::endl;
                    for (auto &blockCiphertexts : this->bucketCiphertexts) {
                        std::vector<char> blockData;
                        this->elgamal.ParallelDecrypt(blockCiphertexts, blockData);
                        BlockConfig::TYPE_BLOCK_ID blockID;
                        std::memcpy(&blockID, blockData.data(), sizeof(BlockConfig::TYPE_BLOCK_ID));
                        std::cout << "Block ID: " << blockID << std::endl;
                    }
                    std::cout << "The size of the bucketCiphertexts: " << this->bucketCiphertexts.size() << std::endl;
                #endif
                this->bucketCiphertexts_flat.clear();
                for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < BucketConfig::BUCKET_SIZE; ++i) {
                    this->bucketCiphertexts_flat.insert(this->bucketCiphertexts_flat.end(),
                                                        this->bucketCiphertexts[i].begin(),
                                                        this->bucketCiphertexts[i].end());
                }
                // Re-randomize the elements in the bucketCiphertexts
                // for (auto& blockCiphertexts : this->bucketCiphertexts) {
                //     this->elgamal.ParallelRerandomize(blockCiphertexts);
                // }
                #if LOG_BREAKDOWN_COST
                this->logger.startTiming(this->LogEarlyReshuffleRerandomizeandSerializeScheme2);
                #endif
                this->elgamal.ParallelRerandomize(this->bucketCiphertexts_flat);
                #if LOG_BREAKDOWN_COST
                this->logger.stopTiming(this->LogEarlyReshuffleRerandomizeandSerializeScheme2);
                this->logger.writeToFile();
                #endif
                for (auto& inner_vector : this->bucketCiphertexts) {
                    inner_vector.clear();
                }
                // Move all data back to the original bucketCiphertexts
                auto flat_iter = this->bucketCiphertexts_flat.begin();
                for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < BucketConfig::BUCKET_SIZE; ++i) {
                    this->bucketCiphertexts[i].insert(
                        this->bucketCiphertexts[i].begin(),
                        std::move_iterator(flat_iter),
                        std::move_iterator(flat_iter + ElGamalNTLConfig::BLOCK_CHUNK_SIZE)
                    );
                    flat_iter += ElGamalNTLConfig::BLOCK_CHUNK_SIZE;
                }
                this->offsetEarlyReshuffleComplete = 0;
                for (auto &blockCiphertexts : this->bucketCiphertexts) {
                    this->elgamal.SerializeCiphertexts(blockCiphertexts, this->blockCiphertextsSerializedData);
                    std::memcpy(this->bucketCiphertextsSerializedData.data() + this->offsetEarlyReshuffleComplete,
                                this->blockCiphertextsSerializedData.data(),
                                ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
                    this->offsetEarlyReshuffleComplete += ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS;
                }
                #if USE_COUT
                    // // Test above Serialization
                    this->offsetEarlyReshuffleComplete = 0;
                    for (auto &blockCiphertexts : this->bucketCiphertexts) {
                        std::memcpy(this->blockCiphertextsSerializedData.data(),
                                    this->bucketCiphertextsSerializedData.data() + this->offsetEarlyReshuffleComplete,
                                    ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
                        this->elgamal.DeserializeCiphertexts(this->blockCiphertextsSerializedData, blockCiphertexts);
                        std::vector<char> blockData;
                        this->elgamal.ParallelDecrypt(blockCiphertexts, blockData);
                        BlockConfig::TYPE_BLOCK_ID blockID;
                        std::memcpy(&blockID, blockData.data(), sizeof(BlockConfig::TYPE_BLOCK_ID));
                        std::cout << "Block ID: " << blockID << std::endl;
                        this->offsetEarlyReshuffleComplete += ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS;
                    }
                #endif
                this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(),
                                                          ServerConfig::CMD_COMPLETE_EARLY_RESHUFFLE_SERVER_THIRD_PARTY);
                #if LOG_BREAKDOWN_COST
                this->logger.startTiming(this->LogEarlyReshuffleSendBucketToThirdPartyScheme2);
                #endif
                this->communicator2ThirdParty.sendData(this->communicator2ThirdParty.getSockfd(),
                                                        this->bucketCiphertextsSerializedData.data(),
                                                        this->bucketCiphertextsSerializedData.size());
                this->communicator2ThirdParty.receiveCommand(this->communicator2ThirdParty.getSockfd(), this->command);
                #if LOG_BREAKDOWN_COST
                this->logger.stopTiming(this->LogEarlyReshuffleSendBucketToThirdPartyScheme2);
                this->logger.writeToFile();
                #endif
                this->communicator2ThirdParty.receiveData(this->communicator2ThirdParty.getSockfd(),
                                                        this->bucketCiphertextsSerializedData.data(),
                                                        ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
                this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(), ServerConfig::CMD_SUCCESS);
                #if USE_COUT
                // recovert the bucketCiphertexts from the bucketCiphertextsSerializedData
                this->offsetEarlyReshuffleComplete = 0;
                for (auto &blockCiphertexts : this->bucketCiphertexts) {
                    std::memcpy(this->blockCiphertextsSerializedData.data(),
                                this->bucketCiphertextsSerializedData.data() + this->offsetEarlyReshuffleComplete,
                                ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
                    this->elgamal.DeserializeCiphertexts(this->blockCiphertextsSerializedData, blockCiphertexts);
                    std::vector<char> blockData;
                    this->elgamal.ParallelDecrypt(blockCiphertexts, blockData);
                    BlockConfig::TYPE_BLOCK_ID blockID;
                    std::memcpy(&blockID, blockData.data(), sizeof(BlockConfig::TYPE_BLOCK_ID));
                    std::cout << "Block ID: " << blockID << std::endl;
                    this->offsetEarlyReshuffleComplete += ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS;
                }
                #endif
                // open the file of BukcetConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(0)
                #if LOG_BREAKDOWN_COST
                this->logger.startTiming(this->LogEarlyReshuffleWriteBucketBackToDiskScheme2);
                #endif
                ofs_early_reshuffle.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(this->bucketIDEarlyReshuffleComplete), std::ios::binary);
                ofs_early_reshuffle.write(this->bucketCiphertextsSerializedData.data(), ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
                ofs_early_reshuffle.close();
                #if LOG_BREAKDOWN_COST
                this->logger.stopTiming(this->LogEarlyReshuffleWriteBucketBackToDiskScheme2);
                this->logger.writeToFile();
                #endif
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
            } else if (this->command == ServerConfig::CMD_COMPLETE_EVICT_CLIENT_TO_SERVER) {
                #if LOG_EVICT_BREAKDOWN_COST_SERVER
                this->logger.startTiming(this->LogEvictionGenPathBucketIDsScheme2);
                #endif
                TreeConfig::GenPathBucketIDsInReverseOrder(TreeConfig::ACCESS_COUNT_EVICTION_COMPLETE,
                                                        TreeConfig::HEIGHT,
                                                        this->evictPathBucketIDsComplete);
                #if LOG_EVICT_BREAKDOWN_COST_SERVER
                this->logger.stopTiming(this->LogEvictionGenPathBucketIDsScheme2);
                this->logger.writeToFile();
                #endif
                for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < TreeConfig::HEIGHT - 1; ++i) {
                    this->bucket.LoadDataFromDisk(BucketConfig::DATADIR,
                                                BucketConfig::BUCKETPREFIX + std::to_string(2 * this->evictPathBucketIDsComplete[i] + 1),
                                                this->bucketCiphertextsEvictComplete);
                    // Wrtie the this->bucketCiphertextsEvictComplete to this->path_evict_bucketCiphertexts_complete
                    for (BucketConfig::TYPE_BUCKET_SIZE j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
                        // this->path_evict_bucketCiphertexts_complete[ 2 * i * BucketConfig::BUCKET_SIZE + j] = this->bucketCiphertextsEvictComplete[j];
                        // Use std::move to avoid the copy
                        this->path_evict_bucketCiphertexts_complete[ 2 * i * BucketConfig::BUCKET_SIZE + j] = std::move(this->bucketCiphertextsEvictComplete[j]);
                    }
                    this->bucket.LoadDataFromDisk(BucketConfig::DATADIR,
                                                BucketConfig::BUCKETPREFIX + std::to_string(2 * this->evictPathBucketIDsComplete[i] + 2),
                                                this->bucketCiphertextsEvictComplete);
                    for (BucketConfig::TYPE_BUCKET_SIZE j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
                        // this->path_evict_bucketCiphertexts_complete[ (2 * i + 1) * BucketConfig::BUCKET_SIZE + j] = this->bucketCiphertextsEvictComplete[j];
                        // Use std::move to avoid the copy
                        this->path_evict_bucketCiphertexts_complete[ (2 * i + 1) * BucketConfig::BUCKET_SIZE + j] = std::move(this->bucketCiphertextsEvictComplete[j]);
                    }
                }
                ++TreeConfig::ACCESS_COUNT_EVICTION_COMPLETE;
                // read the root bucket data to the rootBucketDataEvictComplete
                #if LOG_EVICT_BREAKDOWN_COST_SERVER
                logger.startTiming(this->LogEvictionLoadRootBucketFromDiskScheme2);
                #endif
                ifs_evict.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(0), std::ios::binary);
                    ifs_evict.read(this->rootBucketDataEvictComplete.data(), ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
                ifs_evict.close();
                #if LOG_EVICT_BREAKDOWN_COST_SERVER
                logger.stopTiming(this->LogEvictionLoadRootBucketFromDiskScheme2);
                logger.writeToFile();
                #endif
                #if USE_COUT
                std::cout << "Received command: CMD_COMPLETE_EVICT_CLIENT_TO_SERVER" << std::endl;
                #endif
                #if LOG_EVICT_BREAKDOWN_COST_SERVER
                this->logger.startTiming(this->LogEvictionSendRootBucketToClientScheme2);
                #endif
                this->communicator.sendData(clientSockfd,
                                            this->rootBucketDataEvictComplete.data(),
                                            ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
                this->communicator.receiveCommand(clientSockfd, this->cmd1);
                #if LOG_EVICT_BREAKDOWN_COST_SERVER
                this->logger.stopTiming(this->LogEvictionSendRootBucketToClientScheme2);
                this->logger.writeToFile();
                #endif
                this->communicator.receiveData(clientSockfd,
                                                this->rootBucketDataEvictComplete.data(),
                                                ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < BucketConfig::BUCKET_SIZE; ++i) {
                    // std::memcpy(this->blockCiphertextsSerializedData.data(),
                    //             this->rootBucketDataEvictComplete.data() + i * ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS,
                    //             ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
                    // this->elgamal.DeserializeCiphertexts(this->blockCiphertextsSerializedData, this->triplet_evict_bucketCiphertexts[i]);
                    this->elgamal.DeserializeCiphertexts(reinterpret_cast<const unsigned char*>(this->rootBucketDataEvictComplete.data() + i * ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS),
                                                        this->triplet_evict_bucketCiphertexts[i]);
                    #if USE_COUT
                        std::cout << "Block ID: for the root bucket: " << i << std::endl;
                        std::vector<char> blockData;
                        this->elgamal.ParallelDecrypt(this->triplet_evict_bucketCiphertexts[i], blockData);
                        BlockConfig::TYPE_BLOCK_ID blockID;
                        std::memcpy(&blockID, blockData.data(), sizeof(BlockConfig::TYPE_BLOCK_ID));
                        std::cout << "Block ID: " << blockID << std::endl;
                    #endif
                }
                // Multi-threading version of above code
                // this->elgamal.DeserializeCiphertexts(reinterpret_cast<const unsigned char*>(this->rootBucketDataEvictComplete.data()),
                //                                     this->single_bucketCiphertexts_flat,
                //                                     this->bucketCiphertextsNumPairs);

                // // Compare the this->single_bucketCiphertexts_flat with the this->triplet_evict_bucketCiphertexts[0]
                // for (size_t i = 0; i < BucketConfig::BUCKET_SIZE; ++i) {
                //     this->triplet_evict_bucketCiphertexts[i].resize(this->blockCiphertextsNumPairs);
                //     for (size_t j = 0; j < this->blockCiphertextsNumPairs; ++j) {
                //         // if (this->single_bucketCiphertexts_flat[i * this->blockCiphertextsNumPairs + j] != this->triplet_evict_bucketCiphertexts[i][j]) {
                //         //     std::cerr << "[Eviction]Error: The single_bucketCiphertexts_flat is not equal to the triplet_evict_bucketCiphertexts[0]" << std::endl;
                //         // }
                //         this->triplet_evict_bucketCiphertexts[i][j] = std::move(this->single_bucketCiphertexts_flat[i * this->blockCiphertextsNumPairs + j]);
                //     }
                // }
                
                                    
                // this->triplet_evict_bucketCiphertexts[0] = std::move(this->single_bucketCiphertexts_flat);
                // for (size_t i = 1; i < 3; ++i) {
                //     // Wrtie the this->bucketCiphertextsEvictComplete to this->triplet_evict_bucketCiphertexts
                //     for (BucketConfig::TYPE_BUCKET_SIZE j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
                //         // this->triplet_evict_bucketCiphertexts[i * BucketConfig::BUCKET_SIZE + j] = this->path_evict_bucketCiphertexts_complete[(i - 1) * BucketConfig::BUCKET_SIZE + j];
                //         // Use std::move to avoid the copy
                //         this->triplet_evict_bucketCiphertexts[i * BucketConfig::BUCKET_SIZE + j] = std::move(this->path_evict_bucketCiphertexts_complete[(i - 1) * BucketConfig::BUCKET_SIZE + j]);
                //     }
                // }
                this->triplet_evict_bucketCiphertexts_flat.clear();
                for (BucketConfig::TYPE__SMALL_INDEX i = 0; i < 3; ++i) {
                    for (BucketConfig::TYPE_BUCKET_SIZE j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
                        if (i == 0) {
                            this->triplet_evict_bucketCiphertexts_flat.insert(
                                this->triplet_evict_bucketCiphertexts_flat.end(),
                                std::make_move_iterator(this->triplet_evict_bucketCiphertexts[j].begin()),
                                std::make_move_iterator(this->triplet_evict_bucketCiphertexts[j].end())
                            );
                        } else {
                            this->triplet_evict_bucketCiphertexts_flat.insert(
                                this->triplet_evict_bucketCiphertexts_flat.end(),
                                std::make_move_iterator(this->path_evict_bucketCiphertexts_complete[(i - 1) * BucketConfig::BUCKET_SIZE + j].begin()),
                                std::make_move_iterator(this->path_evict_bucketCiphertexts_complete[(i - 1) * BucketConfig::BUCKET_SIZE + j].end())
                            );
                        }
                    }
                }
                #if LOG_EVICT_BREAKDOWN_COST_SERVER
                logger.startTiming(this->LogEvictionRerandomizationTripletBucketsScheme2);
                #endif
                // rerandomize the triplet_evict_bucketCiphertexts
                // for (auto& blockCiphertexts : this->triplet_evict_bucketCiphertexts) {
                //     this->elgamal.ParallelRerandomize(blockCiphertexts);
                // }
                this->elgamal.ParallelRerandomize(this->triplet_evict_bucketCiphertexts_flat);
                #if LOG_EVICT_BREAKDOWN_COST_SERVER
                logger.stopTiming(this->LogEvictionRerandomizationTripletBucketsScheme2);
                logger.writeToFile();
                #endif
                // Clear existing data in the triplet_evict_bucketCiphertexts before refilling
                for (auto& inner_vector : this->triplet_evict_bucketCiphertexts) {
                    inner_vector.clear();
                }
                // Move all data from the triplet_evict_bucketCiphertexts_flat to the triplet_evict_bucketCiphertexts
                auto flat_iter = this->triplet_evict_bucketCiphertexts_flat.begin();
                for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < 3 * BucketConfig::BUCKET_SIZE; ++i) {
                    if (flat_iter == this->triplet_evict_bucketCiphertexts_flat.end()) {
                        std::cerr << "[Eviction]Error: Not enough data in triplet_evict_bucketCiphertexts_flat" << std::endl;
                        break;
                    }
                    this->triplet_evict_bucketCiphertexts[i].insert(
                        this->triplet_evict_bucketCiphertexts[i].begin(),
                        std::make_move_iterator(flat_iter),
                        std::make_move_iterator(flat_iter + ElGamalNTLConfig::BLOCK_CHUNK_SIZE)
                    );
                    // Advance the iterator by the number of elements moved
                    flat_iter += ElGamalNTLConfig::BLOCK_CHUNK_SIZE;
                }
                this->communicator.receiveData(clientSockfd,
                                                this->tripletEvictPermComplete.data(),
                                                this->triplet_evict_perm_size);
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                #if LOG_EVICT_BREAKDOWN_COST_SERVER
                logger.startTiming(this->LogEvictPermutateFirstTripletBuckets);
                #endif
                // Apply the permutation to the triplet_evict_bucketCiphertexts
                BucketConfig::ApplyPermInPlace(this->triplet_evict_bucketCiphertexts, this->tripletEvictPermComplete);
                #if LOG_EVICT_BREAKDOWN_COST_SERVER
                logger.stopTiming(this->LogEvictPermutateFirstTripletBuckets);
                logger.writeToFile();
                #endif
                // Serialize the triplet_evict_bucketCiphertexts into triplet_evict_bucketCiphertextsSerializedData
                size_t offset = 0;
                for (auto &blockCiphertexts : this->triplet_evict_bucketCiphertexts) {
                    this->elgamal.SerializeCiphertexts(blockCiphertexts, this->blockCiphertextsSerializedData);
                    std::memcpy(this->triplet_evict_bucketCiphertextsSerializedData.data() + offset,
                                this->blockCiphertextsSerializedData.data(),
                                ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
                    offset += ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS;
                }
                // Send the triplet_evict_bucketCiphertextsSerializedData to the third party
                this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(),
                                                          ServerConfig::CMD_COMPLETE_EVICT_SERVER_TO_THIRD_PARTY);
                #if LOG_EVICT_BREAKDOWN_COST_SERVER
                logger.startTiming(this->LogEvictionSendTripletBucketsToThirdPartyScheme2);
                #endif
                this->communicator2ThirdParty.sendData(this->communicator2ThirdParty.getSockfd(),
                                                       this->triplet_evict_bucketCiphertextsSerializedData.data(),
                                                         this->triplet_evict_bucketCiphertextsSerializedData.size());
                this->communicator2ThirdParty.receiveCommand(this->communicator2ThirdParty.getSockfd(), this->command);
                #if LOG_EVICT_BREAKDOWN_COST_SERVER
                this->logger.stopTiming(this->LogEvictionSendTripletBucketsToThirdPartyScheme2);
                this->logger.writeToFile();
                #endif
                this->communicator2ThirdParty.receiveData(this->communicator2ThirdParty.getSockfd(),
                                                        this->triplet_evict_bucketCiphertextsSerializedData.data(),
                                                        this->triplet_evict_bucketCiphertextsSerializedDataSize);
                this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(), ServerConfig::CMD_SUCCESS);
                #if USE_COUT
                // Test the correctness of the triplet_evict_bucketCiphertextsSerializedData
                offset = 0;
                for (auto &blockCiphertexts : this->triplet_evict_bucketCiphertexts) {
                    std::memcpy(this->blockCiphertextsSerializedData.data(),
                                this->triplet_evict_bucketCiphertextsSerializedData.data() + offset,
                                ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
                    this->elgamal.DeserializeCiphertexts(this->blockCiphertextsSerializedData, blockCiphertexts);
                    std::vector<char> blockData;
                    this->elgamal.ParallelDecrypt(blockCiphertexts, blockData);
                    BlockConfig::TYPE_BLOCK_ID blockID;
                    std::memcpy(&blockID, blockData.data(), sizeof(BlockConfig::TYPE_BLOCK_ID));
                    std::cout << "Block ID: " << blockID << std::endl;
                    offset += ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS;
                }
                #endif
                #if LOG_EVICT_BREAKDOWN_COST_SERVER
                this->logger.startTiming(this->LogEvictionWriteRootBucketBackToDiskScheme2);
                #endif
                std::thread([this]() {
                    this->ofs_evict_root.open(
                        BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(0),
                        std::ios::binary
                    );
                    this->ofs_evict_root.write(
                        this->triplet_evict_bucketCiphertextsSerializedData.data(),
                        ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS
                    );
                    this->ofs_evict_root.close();
                    
                }).detach();
                #if LOG_EVICT_BREAKDOWN_COST_SERVER
                this->logger.stopTiming(this->LogEvictionWriteRootBucketBackToDiskScheme2);
                this->logger.writeToFile();
                #endif
                // Deserialize the triplet_evict_bucketCiphertextsSerializedData into this->path_evict_bucketCiphertexts_complete
                for (BucketConfig::TYPE_SMALL_INDEX_U i = 1; i < 3; ++i) {
                    for (BucketConfig::TYPE_BUCKET_SIZE j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
                        // std::memcpy(this->blockCiphertextsSerializedData.data(),
                        //             this->triplet_evict_bucketCiphertextsSerializedData.data() + i * ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS + j * ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS,
                        //             ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
                        // this->elgamal.DeserializeCiphertexts(this->blockCiphertextsSerializedData, this->path_evict_bucketCiphertexts_complete[(i - 1) * BucketConfig::BUCKET_SIZE + j]);
                        this->elgamal.DeserializeCiphertexts(reinterpret_cast<const unsigned char*>(this->triplet_evict_bucketCiphertextsSerializedData.data() + i * ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS + j * ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS),
                                                            this->path_evict_bucketCiphertexts_complete[(i - 1) * BucketConfig::BUCKET_SIZE + j]);
                    }
                }

                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                
                for (PathConfig::TYPE_PATH_SIZE i = 1; i < TreeConfig::HEIGHT - 1; ++i) {
                    #if USE_COUT
                        std::cout << "i: " << i << std::endl;
                        std::cout << "The tripletBucketIDsComplete: ";
                        for (auto& id : this->tripletBucketIDsComplete) {
                            std::cout << id << " ";
                        }
                        std::cout << std::endl;
                    #endif
                    this->triplet_evict_bucketCiphertexts_flat.clear();
                    for (BucketConfig::TYPE_SMALL_INDEX_U ii = 0; ii < 3; ++ii) {
                        if (ii == 0 && this->evictPathBucketIDsComplete[i] % 2 == 1) {
                            for (BucketConfig::TYPE_BUCKET_SIZE j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
                                // this->triplet_evict_bucketCiphertexts[ii * BucketConfig::BUCKET_SIZE + j] = this->path_evict_bucketCiphertexts_complete[(2 * (i - 1)) * BucketConfig::BUCKET_SIZE + j];
                                // Use std::move to avoid the copy
                                // this->triplet_evict_bucketCiphertexts[ii * BucketConfig::BUCKET_SIZE + j] = std::move(this->path_evict_bucketCiphertexts_complete[(2 * (i - 1)) * BucketConfig::BUCKET_SIZE + j]);
                                this->triplet_evict_bucketCiphertexts_flat.insert(
                                    this->triplet_evict_bucketCiphertexts_flat.end(),
                                    std::make_move_iterator(this->path_evict_bucketCiphertexts_complete[(2 * (i - 1)) * BucketConfig::BUCKET_SIZE + j].begin()),
                                    std::make_move_iterator(this->path_evict_bucketCiphertexts_complete[(2 * (i - 1)) * BucketConfig::BUCKET_SIZE + j].end())
                                );
                            }
                        } else if (ii == 0 && this->evictPathBucketIDsComplete[i] % 2 == 0) {
                            for (BucketConfig::TYPE_BUCKET_SIZE j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
                                // this->triplet_evict_bucketCiphertexts[ii * BucketConfig::BUCKET_SIZE + j] = this->path_evict_bucketCiphertexts_complete[(2 * i - 1) * BucketConfig::BUCKET_SIZE + j];
                                // Use std::move to avoid the copy
                                // this->triplet_evict_bucketCiphertexts[ii * BucketConfig::BUCKET_SIZE + j] = std::move(this->path_evict_bucketCiphertexts_complete[(2 * i - 1) * BucketConfig::BUCKET_SIZE + j]);
                                this->triplet_evict_bucketCiphertexts_flat.insert(
                                    this->triplet_evict_bucketCiphertexts_flat.end(),
                                    std::make_move_iterator(this->path_evict_bucketCiphertexts_complete[(2 * i - 1) * BucketConfig::BUCKET_SIZE + j].begin()),
                                    std::make_move_iterator(this->path_evict_bucketCiphertexts_complete[(2 * i - 1) * BucketConfig::BUCKET_SIZE + j].end())
                                );
                            }
                        } else {
                            for (BucketConfig::TYPE_BUCKET_SIZE j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
                                // this->triplet_evict_bucketCiphertexts[ii * BucketConfig::BUCKET_SIZE + j] = this->path_evict_bucketCiphertexts_complete[(2 * i + ii - 1) * BucketConfig::BUCKET_SIZE + j];
                                // Use std::move to avoid the copy
                                // this->triplet_evict_bucketCiphertexts[ii * BucketConfig::BUCKET_SIZE + j] = std::move(this->path_evict_bucketCiphertexts_complete[(2 * i + ii - 1) * BucketConfig::BUCKET_SIZE + j]);
                                this->triplet_evict_bucketCiphertexts_flat.insert(
                                    this->triplet_evict_bucketCiphertexts_flat.end(),
                                    std::make_move_iterator(this->path_evict_bucketCiphertexts_complete[(2 * i + ii - 1) * BucketConfig::BUCKET_SIZE + j].begin()),
                                    std::make_move_iterator(this->path_evict_bucketCiphertexts_complete[(2 * i + ii - 1) * BucketConfig::BUCKET_SIZE + j].end())
                                );
                            }
                        }
                    }
                    // rerandomize the triplet_evict_bucketCiphertexts
                    // for (auto& blockCiphertexts : this->triplet_evict_bucketCiphertexts) {
                    //     this->elgamal.ParallelRerandomize(blockCiphertexts);
                    // }
                     #if LOG_EVICT_BREAKDOWN_COST_SERVER
                    logger.startTiming(this->LogEvictionRerandomizationTripletBucketsScheme2);
                    #endif
                    this->elgamal.ParallelRerandomize(this->triplet_evict_bucketCiphertexts_flat);
                    #if LOG_EVICT_BREAKDOWN_COST_SERVER
                    logger.stopTiming(this->LogEvictionRerandomizationTripletBucketsScheme2);
                    logger.writeToFile();
                    #endif
                    // Write the data in the triplet_evict_bucketCiphertexts_flat back to the triplet_evict_bucketCiphertexts
                    flat_iter = this->triplet_evict_bucketCiphertexts_flat.begin();
                    for (BucketConfig::TYPE_BUCKET_SIZE j = 0; j < 3 * BucketConfig::BUCKET_SIZE; ++j) {
                        if (flat_iter == this->triplet_evict_bucketCiphertexts_flat.end()) {
                            std::cerr << "[Eviction]Error: Not enough data in triplet_evict_bucketCiphertexts_flat" << std::endl;
                            break;
                        }
                        this->triplet_evict_bucketCiphertexts[j].clear();
                        this->triplet_evict_bucketCiphertexts[j].insert(
                            this->triplet_evict_bucketCiphertexts[j].begin(),
                            std::make_move_iterator(flat_iter),
                            std::make_move_iterator(flat_iter + ElGamalNTLConfig::BLOCK_CHUNK_SIZE)
                        );
                        // Advance the iterator by the number of elements moved
                        flat_iter += ElGamalNTLConfig::BLOCK_CHUNK_SIZE;
                    }
                    this->communicator.receiveData(clientSockfd,
                                                    this->tripletEvictPermComplete.data(),
                                                    this->triplet_evict_perm_size);
                    this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                    // Apply the permutation to the triplet_evict_bucketCiphertexts
                    #if LOG_EVICT_BREAKDOWN_COST_SERVER
                    this->logger.startTiming(this->LogEvictionRerandomizationTripletBucketsScheme2);
                    #endif
                    BucketConfig::ApplyPermInPlace(this->triplet_evict_bucketCiphertexts, this->tripletEvictPermComplete);
                     #if LOG_EVICT_BREAKDOWN_COST_SERVER
                    this->logger.stopTiming(this->LogEvictionRerandomizationTripletBucketsScheme2);
                    this->logger.writeToFile();
                    #endif
                    // Serialize the triplet_evict_bucketCiphertexts into triplet_evict_bucketCiphertextsSerializedData
                    size_t offset = 0;
                    for (auto &blockCiphertexts : this->triplet_evict_bucketCiphertexts) {
                        this->elgamal.SerializeCiphertexts(blockCiphertexts, this->blockCiphertextsSerializedData);
                        std::memcpy(this->triplet_evict_bucketCiphertextsSerializedData.data() + offset,
                                    this->blockCiphertextsSerializedData.data(),
                                    ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
                        offset += ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS;
                    }
                    // Send the triplet_evict_bucketCiphertextsSerializedData to the third party
                    this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(),
                                                            ServerConfig::CMD_COMPLETE_EVICT_SERVER_TO_THIRD_PARTY);
                    #if LOG_EVICT_BREAKDOWN_COST_SERVER
                    this->logger.startTiming(this->LogEvictionSendTripletBucketsToThirdPartyScheme2);
                    #endif
                    this->communicator2ThirdParty.sendData(this->communicator2ThirdParty.getSockfd(),
                                                        this->triplet_evict_bucketCiphertextsSerializedData.data(),
                                                        this->triplet_evict_bucketCiphertextsSerializedData.size());
                    this->communicator2ThirdParty.receiveCommand(this->communicator2ThirdParty.getSockfd(), this->command);
                    #if LOG_EVICT_BREAKDOWN_COST_SERVER
                    this->logger.stopTiming(this->LogEvictionSendTripletBucketsToThirdPartyScheme2);
                    this->logger.writeToFile();
                    #endif
                    this->communicator2ThirdParty.receiveData(this->communicator2ThirdParty.getSockfd(),
                                                            this->triplet_evict_bucketCiphertextsSerializedData.data(),
                                                            this->triplet_evict_bucketCiphertextsSerializedDataSize);
                    this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(), ServerConfig::CMD_SUCCESS);
                    // Deserialize the triplet_evict_bucketCiphertextsSerializedData into this->path_evict_bucketCiphertexts_complete
                    for (BucketConfig::TYPE_SMALL_INDEX_U ii = 0; ii < 3; ++ii) {
                        if (ii == 0 && this->evictPathBucketIDsComplete[i] % 2 == 1) {
                            for (BucketConfig::TYPE_BUCKET_SIZE j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
                                // std::memcpy(this->blockCiphertextsSerializedData.data(),
                                //             this->triplet_evict_bucketCiphertextsSerializedData.data() + ii * ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS + j * ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS,
                                //             ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
                                // this->elgamal.DeserializeCiphertexts(this->blockCiphertextsSerializedData, this->path_evict_bucketCiphertexts_complete[(2 * (i - 1)) * BucketConfig::BUCKET_SIZE + j]);
                                this->elgamal.DeserializeCiphertexts(reinterpret_cast<const unsigned char*>(this->triplet_evict_bucketCiphertextsSerializedData.data() + ii * ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS + j * ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS),
                                                                    this->path_evict_bucketCiphertexts_complete[(2 * (i - 1)) * BucketConfig::BUCKET_SIZE + j]);
                            }
                        } else if (ii == 0 && this->evictPathBucketIDsComplete[i] % 2 == 0) {
                            for (BucketConfig::TYPE_BUCKET_SIZE j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
                                // std::memcpy(this->blockCiphertextsSerializedData.data(),
                                //             this->triplet_evict_bucketCiphertextsSerializedData.data() + ii * ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS + j * ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS,
                                //             ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
                                // this->elgamal.DeserializeCiphertexts(this->blockCiphertextsSerializedData, this->path_evict_bucketCiphertexts_complete[(2 * i - 1) * BucketConfig::BUCKET_SIZE + j]);
                                this->elgamal.DeserializeCiphertexts(reinterpret_cast<const unsigned char*>(this->triplet_evict_bucketCiphertextsSerializedData.data() + ii * ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS + j * ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS),
                                                                    this->path_evict_bucketCiphertexts_complete[(2 * i - 1) * BucketConfig::BUCKET_SIZE + j]);
                            }
                        } else {
                            for (BucketConfig::TYPE_BUCKET_SIZE j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
                                // std::memcpy(this->blockCiphertextsSerializedData.data(),
                                //             this->triplet_evict_bucketCiphertextsSerializedData.data() + ii * ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS + j * ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS,
                                //             ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
                                // this->elgamal.DeserializeCiphertexts(this->blockCiphertextsSerializedData, this->path_evict_bucketCiphertexts_complete[(2 * i + ii - 1) * BucketConfig::BUCKET_SIZE + j]);
                                this->elgamal.DeserializeCiphertexts(reinterpret_cast<const unsigned char*>(this->triplet_evict_bucketCiphertextsSerializedData.data() + ii * ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS + j * ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS),
                                                                    this->path_evict_bucketCiphertexts_complete[(2 * i + ii - 1) * BucketConfig::BUCKET_SIZE + j]);
                            }
                        }
                    }
                    this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                }
                // Write the this->path_evict_bucketCiphertexts_complete to the disk
                for (PathConfig::TYPE_PATH_SIZE i = 0; i < TreeConfig::HEIGHT - 1; ++i) {
                    this->ofs_evict.open(
                        BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(this->evictPathBucketIDsComplete[i] * 2 + 1),
                        std::ios::binary
                    );
                    for (BucketConfig::TYPE_BUCKET_SIZE j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
                        this->elgamal.SerializeCiphertexts(this->path_evict_bucketCiphertexts_complete[i * 2 * BucketConfig::BUCKET_SIZE + j], this->blockCiphertextsSerializedData);
                        #if LOG_EVICT_BREAKDOWN_COST_SERVER
                            this->logger.startTiming(this->LogEvictionWritePathBucketsBackToDiskScheme2);
                        #endif
                         this->ofs_evict.write(
                            this->blockCiphertextsSerializedData.data(),
                            ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS
                        );
                         #if LOG_EVICT_BREAKDOWN_COST_SERVER
                            this->logger.stopTiming(this->LogEvictionWritePathBucketsBackToDiskScheme2);
                            this->logger.writeToFile();
                        #endif
                    }
                    this->ofs_evict.close();
                    this->ofs_evict.open(
                        BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(this->evictPathBucketIDsComplete[i] * 2 + 2),
                        std::ios::binary
                    );
                    for (BucketConfig::TYPE_BUCKET_SIZE j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
                        this->elgamal.SerializeCiphertexts(this->path_evict_bucketCiphertexts_complete[(i * 2 + 1) * BucketConfig::BUCKET_SIZE + j], this->blockCiphertextsSerializedData);
                        #if LOG_EVICT_BREAKDOWN_COST_SERVER
                            this->logger.startTiming(this->LogEvictionWritePathBucketsBackToDiskScheme2);
                        #endif
                         this->ofs_evict.write(
                            this->blockCiphertextsSerializedData.data(),
                            ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS
                        );
                        #if LOG_EVICT_BREAKDOWN_COST_SERVER
                            this->logger.stopTiming(this->LogEvictionWritePathBucketsBackToDiskScheme2);
                            this->logger.writeToFile();
                        #endif
                    }
                     this->ofs_evict.close();
                }
                // #if LOG_EVICT_BREAKDOWN_COST_SERVER
                // this->logger.startTiming(this->LogEvictionWritePathBucketsBackToDiskScheme2);
                // #endif
                // std::vector<std::future<void>> futures;

                // for (PathConfig::TYPE_PATH_SIZE i = 0; i < TreeConfig::HEIGHT - 1; ++i) {
                //     futures.push_back(std::async(std::launch::async, [this, i]() {
                //         // Write to File A and B in parallel
                //         std::vector<std::future<void>> inner_futures;

                //         // File A
                //         inner_futures.push_back(std::async(std::launch::async, [this, i]() {
                //             std::ofstream ofs_evict(
                //                 BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(this->evictPathBucketIDsComplete[i] * 2 + 1),
                //                 std::ios::binary
                //             );
                //             for (BucketConfig::TYPE_BUCKET_SIZE j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
                //                 std::vector<char> serializedData; // Use local variable
                //                 serializedData.resize(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
                //                 this->elgamal.SerializeCiphertexts(
                //                     this->path_evict_bucketCiphertexts_complete[i * 2 * BucketConfig::BUCKET_SIZE + j], 
                //                     serializedData
                //                 );
                //                 ofs_evict.write(
                //                     serializedData.data(),
                //                     ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS
                //                 );
                //             }
                //         }));

                //         // File B
                //         inner_futures.push_back(std::async(std::launch::async, [this, i]() {
                //             std::ofstream ofs_evict(
                //                 BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(this->evictPathBucketIDsComplete[i] * 2 + 2),
                //                 std::ios::binary
                //             );
                //             for (BucketConfig::TYPE_BUCKET_SIZE j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
                //                 std::vector<char> serializedData; // Use local variable
                //                 serializedData.resize(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
                //                 this->elgamal.SerializeCiphertexts(
                //                     this->path_evict_bucketCiphertexts_complete[(i * 2 + 1) * BucketConfig::BUCKET_SIZE + j], 
                //                     serializedData
                //                 );
                //                 ofs_evict.write(
                //                     serializedData.data(),
                //                     ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS
                //                 );
                //             }
                //         }));

                //         // Wait for both files to be written
                //         for (auto& future : inner_futures) {
                //             future.get();
                //         }
                //     }));
                // }

                // // Wait for all `i` iterations to complete
                // for (auto& future : futures) {
                //     future.get();
                // }
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
            } else if (this->command == ServerConfig::CMD_INIT_BINARY_TREE_SCHEME1_CLIENT_TO_SERVER) {
                std::cout << "Received command: CMD_INIT_BINARY_TREE_SCHEME1_CLIENT_TO_SERVER" << std::endl;
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                this->tree.GenTreeWithMDs();
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
            } else if (this->command == ServerConfig::CMD_READ_PATH_SCHEME1_CLIENT_TO_SERVER) {
                // this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                // std::cout << "Received command: CMD_READ_PATH_SCHEME1_CLIENT_TO_SERVER" << std::endl;
                this->communicator.receiveData(clientSockfd,
                                                reinterpret_cast<char*>(&this->pathID),
                                                sizeof(this->pathID));
                // this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                 #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_SERVER
                auto start = std::chrono::high_resolution_clock::now();
                #endif
                #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_SERVER
                this->logger.startTiming(this->LogReadPathGenBucketIDsScheme1);
                #endif
                this->path.ConvertPID2BIDs(this->pathID, TreeConfig::HEIGHT, this->bucketIDOffsets);
                #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_SERVER
                this->logger.stopTiming(this->LogReadPathGenBucketIDsScheme1);
                this->logger.writeToFile();
                #endif
                #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_SERVER
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<long long, std::nano> duration = end - start;
                std::cout << "[Server Computation] ConvertPID2BIDs: " << duration.count() << " ns" << std::endl;
                #endif
                #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_SERVER
                start = std::chrono::high_resolution_clock::now();
                #endif
                #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_SERVER
                this->logger.startTiming(this->LogReadPathLoadMDsScheme1);
                #endif
                // Load the MDs from the disk to the memory of this->pathBucketMDData1
                auto it = this->pathBucketMDData1.data();
                for (PathConfig::TYPE_PATH_SIZE i = 0; i < TreeConfig::HEIGHT; ++i) {
                    FileConfig::bucketFileLoad.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(this->bucketIDOffsets[i]), std::ios::binary);
                    FileConfig::bucketFileLoad.read(reinterpret_cast<char*>(&counter1), sizeof(counter1));
                    #if USE_COUT
                    // Check if the reading was successful
                    if (FileConfig::bucketFileLoad.gcount() == sizeof(size_t)) {
                        std::cout << "Read value: " << counter1 << std::endl;
                    } else {
                        std::cerr << "Failed to read the required number of bytes from the file." << std::endl;
                    }
                    #endif
                    FileConfig::bucketFileLoad.close();
                    FileConfig::bucketFileLoad.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(this->bucketIDOffsets[i]), std::ios::binary);
                    FileConfig::bucketFileLoad.read(it + i * BucketConfig::META_DATA_SIZE,
                                                    BucketConfig::META_DATA_SIZE);
                    FileConfig::bucketFileLoad.close();
                    // BucketConfig::META_DATA md;
                    // this->tree.DecryptMD(it + i * BucketConfig::META_DATA_SIZE, this->bucketIDOffsets[i]);
                    // md.Deserialize(it + i * BucketConfig::META_DATA_SIZE);
                    // md.print();
                }
                #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_SERVER
                this->logger.stopTiming(this->LogReadPathLoadMDsScheme1);
                this->logger.writeToFile();
                #endif
                #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_SERVER
                end = std::chrono::high_resolution_clock::now();
                duration = end - start;
                std::cout << "[Server IO] Load MDs from disk to memory: " << duration.count() << " ns" << std::endl;
                #endif
                // Send the MDs to the client
                #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_SERVER
                start = std::chrono::high_resolution_clock::now();
                #endif
                this->communicator.sendData(clientSockfd,
                                            this->pathBucketMDData1.data(),
                                            this->pathBucketMDData1.size());
                #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_SERVER
                end = std::chrono::high_resolution_clock::now();
                duration = end - start;
                std::cout << "Send MDs to the client: " << duration.count() << " ns" << std::endl;
                #endif
                #if USE_COUT
                // this->tree.DecryptMD(this->pathBucketMDData1.data(), this->bucketIDOffsets[0]);
                std::cout << "The first bucket ID: " << this->bucketIDOffsets[0] << std::endl;
                size_t blockSize;
                std::cout << "sizeof(size_t): " << sizeof(size_t) << std::endl;
                auto it_b = this->pathBucketMDData1.data() + sizeof(size_t) + BucketConfig::BUCKET_SIZE;
                std::memcpy(&blockSize, it_b, sizeof(size_t));
                std::cout << "blockSize: " << blockSize << std::endl;
                #endif

                // this->communicator.receiveCommand(clientSockfd, this->cmd1);
                // Receive the offset from the client
                this->communicator.receiveData(clientSockfd,
                                                this->pathCompleteOffsets.data(),
                                                this->pathCompleteOffsetsSize);
                // this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                #if USE_COUT
                std::cout << "The pathCompleteOffsets: ";
                for (auto& offset : this->pathCompleteOffsets) {
                    std::cout << offset << " ";
                }
                std::cout << std::endl;
                #endif
                // Receive the MDs from the client to the this->pathBucketMDData1
                this->communicator.receiveData(clientSockfd,
                                                this->pathBucketMDData1.data(),
                                                this->pathBucketMDDataSize1);
                // this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                #if USE_COUT
                std::cout << "The pathBucketMDData1: " << std::endl;
                it = this->pathBucketMDData1.data();
                for (PathConfig::TYPE_PATH_SIZE i = 0; i < TreeConfig::HEIGHT; ++i) {
                    this->tree.DecryptMD(it, this->bucketIDOffsets[i]);
                    BucketConfig::META_DATA md;
                    md.Deserialize(it);
                    md.print();
                    it += BucketConfig::META_DATA_SIZE;
                }
                #endif
                // Read the corresponding buckets from the disk to the this->pathBlockData1
                // And write the this->pathBlockData1 to the disk
                #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_SERVER
                start = std::chrono::high_resolution_clock::now();
                #endif
                #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_SERVER
                this->logger.startTiming(this->LogReadPathLoadBlocksScheme1);
                #endif
                auto it_md = this->pathBucketMDData1.data();
                auto it_block = this->pathBlocksData1.data();
                for (PathConfig::TYPE_PATH_SIZE i = 0; i < TreeConfig::HEIGHT; ++i) {
                    #if USE_COUT
                    std::cout << "Bucket ID: " << this->bucketIDOffsets[i] << std::endl;
                    std::cout << "Path Complete Offset: " << this->pathCompleteOffsets[i] << std::endl;
                    #endif
                    FileConfig::fileReadWriteScheme1.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(this->bucketIDOffsets[i]), std::ios::binary | std::ios::in | std::ios::out);
                    FileConfig::fileReadWriteScheme1.seekg(0, std::ios::beg);
                    FileConfig::fileReadWriteScheme1.write(it_md, BucketConfig::META_DATA_SIZE);
                    FileConfig::fileReadWriteScheme1.seekg(BucketConfig::META_DATA_SIZE + this->pathCompleteOffsets[i] * BlockConfig::BLOCK_SIZE, std::ios::beg);
                    FileConfig::fileReadWriteScheme1.read(it_block, BlockConfig::BLOCK_SIZE);
                                    
                    it_md += BucketConfig::META_DATA_SIZE;
                    it_block += BlockConfig::BLOCK_SIZE;
                    FileConfig::fileReadWriteScheme1.close();
                }
                #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_SERVER
                this->logger.stopTiming(this->LogReadPathLoadBlocksScheme1);
                this->logger.writeToFile();
                #endif
                #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_SERVER
                end = std::chrono::high_resolution_clock::now();
                duration = end - start;
                std::cout << "[Server IO] Load blocks from disk to memory: " << duration.count() << " ns" << std::endl;
                #endif
                // Apply the XOR operation to the blocks and store the result in this->targetBlockData1
                #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_SERVER
                start = std::chrono::high_resolution_clock::now();
                #endif
                #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_SERVER
                this->logger.startTiming(this->LogReadPathXORBlocksScheme1);
                #endif
                // Reset the targetBlockCiphertexts to 0
                std::memset(this->targetBlockCipherData1.data(), 0, BlockConfig::BLOCK_SIZE);
                uint64_t* targetData = reinterpret_cast<uint64_t*>(this->targetBlockCipherData1.data());
                uint64_t* pathData = reinterpret_cast<uint64_t*>(this->pathBlocksData1.data());
                // size_t numBlocks = BlockConfig::BLOCK_SIZE / sizeof(uint64_t);
                for (PathConfig::TYPE_PATH_SIZE i = 0; i < TreeConfig::HEIGHT; ++i) {
                    for (size_t j = 0; j < AESConfig::CHUNK_SIZE_BLOCK_XOR; ++j) {
                        targetData[j] ^= pathData[i * AESConfig::CHUNK_SIZE_BLOCK_XOR + j];
                    }
                }
                #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_SERVER
                this->logger.stopTiming(this->LogReadPathXORBlocksScheme1);
                this->logger.writeToFile();
                #endif
                #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_SERVER
                end = std::chrono::high_resolution_clock::now();
                duration = end - start;
                std::cout << "[Server Computation] XOR operation: " << duration.count() << " ns" << std::endl;
                #endif
                // Send the targetBlockData1 to the client
                this->communicator.sendData(clientSockfd,
                                            this->targetBlockCipherData1.data(),
                                            BlockConfig::BLOCK_SIZE);
                // this->communicator.receiveCommand(clientSockfd, this->cmd1);

                
            } else if (this->command == ServerConfig::CMD_EARLY_RESHUFFLE_SCHEME1_CLIENT_TO_SERVER) {
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                #if USE_COUT
                std::cout << "Received command: CMD_EARLY_RESHUFFLE_SCHEME1_CLIENT_TO_SERVER" << std::endl;
                #endif
                this->communicator.receiveData(clientSockfd,
                                            &this->bucketIDEarlyReshuffle1,
                                            sizeof(this->bucketIDEarlyReshuffle1));
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                #if USE_COUT
                std::cout << "The bucket ID for early reshuffle: " << this->bucketIDEarlyReshuffle1 << std::endl;
                #endif
                this->EarlyReshuffleScheme1(this->bucketIDEarlyReshuffle1);
            } else if (this->command == ServerConfig::CMD_EVICT_SCHEME1_CLIENT_TO_SERVER) {
                #if USE_COUT
                std::cout << "Received command: CMD_EVICT_SCHEME1_CLIENT_TO_SERVER" << std::endl;
                #endif
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                this->communicator.receiveData(clientSockfd,
                                                reinterpret_cast<char*>(&this->pathIDEviction1),
                                                sizeof(this->pathIDEviction1));
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                #if USE_COUT
                std::cout << "The path ID for eviction: " << this->pathIDEviction1 << std::endl;
                #endif
                this->EvictScheme1(this->pathIDEviction1);
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
            } else if (this->command == ServerConfig::CMD_INIT_PATH_SCHEME1_CLIENT_TO_SERVER) {
                #if USE_COUT
                std::cout << "Received command: CMD_INIT_PATH_SCHEME1_CLIENT_TO_SERVER" << std::endl;
                #endif
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                this->communicator.receiveData(clientSockfd,
                                                reinterpret_cast<char*>(&this->pathID),
                                                sizeof(this->pathID));
                this->tree.GenPathwithMDs(this->pathID);
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                #if USE_COUT
                // Test the correctness of the path buckets 
                std::vector<char> bucketData(this->bucketSizeEviction1);
                FileConfig::fileReadScheme1.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(7), std::ios::binary);
                FileConfig::fileReadScheme1.read(bucketData.data(), this->bucketSizeEviction1);
                FileConfig::fileReadScheme1.close();
                this->TestBucketContent(bucketData, 7);
                #endif
            } else if (this->command == ServerConfig::CMD_INIT_SINGLE_BUCKET_SCHEME1_CLIENT_TO_SERVER) {
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                #if USE_COUT
                std::cout << "Received command: CMD_INIT_SINGLE_BUCKET_SCHEME1_CLIENT_TO_SERVER" << std::endl;
                #endif
                BucketConfig::TYPE_BUCKET_ID bucketID;
                this->communicator.receiveData(clientSockfd,
                                                reinterpret_cast<char*>(&bucketID),
                                                sizeof(bucketID));
                this->tree.GenSingleBucketWithMD(bucketID);
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                #if USE_COUT
                std::cout << "The bucket ID: " << bucketID << std::endl;
                #endif
            } else if (this->command == ServerConfig::CMD_INIT_EVICT_PATH_SCHEME1_CLIENT_TO_SERVER) {
                std::cout << "Received command: CMD_INIT_EVICT_PATH_SCHEME1_CLIENT_TO_SERVER" << std::endl;
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                this->communicator.receiveData(clientSockfd,
                                                reinterpret_cast<char*>(&this->pathIDEviction1),
                                                sizeof(this->pathIDEviction1));
                this->tree.GenEvictPathWithMDs(this->pathIDEviction1);
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                #if USE_COUT
                std::cout << "Path ID for eviction: " << this->pathIDEviction1 << std::endl;
                #endif
            } else if (this->command == ServerConfig::CMD_TEST_CLIENT_TO_SERVER) {
                this->communicator.sendCommand(clientSockfd, ServerConfig::CMD_SUCCESS);
                std::cout << "Received command: CMD_TEST_CLIENT_TO_SERVER" << std::endl;
                AES_CTR aes_ctr;
                unsigned char iv[AES_BLOCK_SIZE];
                std::memset(iv, 0, AES_BLOCK_SIZE);
                std::vector<char> data(256, 0);
                std::vector<char> encryptedData(256, 1);
                std::vector<char> decryptedData(256, 2);
                aes_ctr.encrypt(reinterpret_cast<unsigned char*>(data.data()), 256,
                                reinterpret_cast<unsigned char*>(encryptedData.data()), iv);
                BlockConfig::TYPE_BLOCK_ID blockID;
                std::memcpy(&blockID, data.data(), sizeof(BlockConfig::TYPE_BLOCK_ID));
                std::cout << "Block ID: " << blockID << std::endl;
                std::memcpy(&blockID, encryptedData.data(), sizeof(BlockConfig::TYPE_BLOCK_ID));
                std::cout << "Block ID: " << blockID << std::endl;
                aes_ctr.decrypt(reinterpret_cast<unsigned char*>(encryptedData.data()), 256,
                                reinterpret_cast<unsigned char*>(decryptedData.data()), iv);
                std::memcpy(&blockID, decryptedData.data(), sizeof(BlockConfig::TYPE_BLOCK_ID));
                std::cout << "Block ID: " << blockID << std::endl;
                // Send the encrypted data to the client
                this->communicator.sendData(clientSockfd,
                                            encryptedData.data(),
                                            encryptedData.size());
                this->communicator.receiveCommand(clientSockfd, this->cmd1);

            }
        } 
    }
    close(clientSockfd);
}

inline void Server::PrintBlockIDsInBucket(const std::vector<std::vector<std::pair<ZZ_p, ZZ_p>>>& bucketCiphertexts) {
    for (auto& blockCiphertexts : bucketCiphertexts) {
        std::vector<char> blockData;
        this->elgamal.ParallelDecrypt(blockCiphertexts, blockData);
        BlockConfig::TYPE_BLOCK_ID blockID;
        std::memcpy(&blockID, blockData.data(), sizeof(BlockConfig::TYPE_BLOCK_ID));
        std::cout << "Block ID: " << blockID << std::endl;
    }
}

void Server::PrintBlockIDsInDiskBucket(const BucketConfig::TYPE_BUCKET_ID bucketID) {
    this->bucket.LoadDataFromDisk(BucketConfig::DATADIR,
                                BucketConfig::BUCKETPREFIX + std::to_string(bucketID),
                                this->bucketCiphertexts);
    this->PrintBlockIDsInBucket(this->bucketCiphertexts);
}

void Server::InitConnectThirdParty() {
    this->communicator2ThirdParty.connectToServer(ThirdPartyConfig::HOST, ThirdPartyConfig::PORT_THIRD_PARTY);
}

void Server::EnsureConnectThirdParty() {
    if (!this->communicator2ThirdParty.isConnected()) {
        std::cout << "Reconnecting to the third party" << std::endl;
        this->InitConnectThirdParty();
    }
}

void Server::EarlyReshuffleScheme1(BucketConfig::TYPE_BUCKET_ID bucketID) {
    EnsureConnectThirdParty();
    this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(),
                                            ServerConfig::CMD_EARLY_RESHUFFLE_SCHEME1_SERVER_TO_THIRD_PARTY);
    this->communicator2ThirdParty.receiveCommand(this->communicator2ThirdParty.getSockfd(), this->cmd1);
    #if PRINT_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    auto start = std::chrono::high_resolution_clock::now();
    #endif
    #if LOG_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    this->logger.startTiming(this->LogEarlyReshuffleLoadBucketMDScheme1);
    #endif
    FileConfig::fileReadScheme1.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(bucketID), std::ios::binary);
    FileConfig::fileReadScheme1.read(this->bucketMDataEarlyReshuffle1.data(), BucketConfig::META_DATA_SIZE);
    FileConfig::fileReadScheme1.close();
    #if LOG_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    this->logger.stopTiming(this->LogEarlyReshuffleLoadBucketMDScheme1);
    this->logger.writeToFile();
    #endif
    #if PRINT_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<long long, std::nano> duration = end - start;
    std::cout << "Read MDs from disk to memory: " << duration.count() << " ns" << std::endl;
    #endif
    #if PRINT_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    start = std::chrono::high_resolution_clock::now();
    #endif
    this->communicator2ThirdParty.sendData(this->communicator2ThirdParty.getSockfd(),
                                        this->bucketMDataEarlyReshuffle1.data(),
                                        this->bucketMDataEarlyReshuffle1.size());
    this->communicator2ThirdParty.receiveCommand(this->communicator2ThirdParty.getSockfd(), this->cmd1);
    #if PRINT_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "Send MDs to the third party: " << duration.count() << " ns" << std::endl;
    #endif
    #if PRINT_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    start = std::chrono::high_resolution_clock::now();
    #endif
    this->communicator2ThirdParty.sendData(this->communicator2ThirdParty.getSockfd(),
                                        reinterpret_cast<char*>(&bucketID),
                                        sizeof(bucketID));
    this->communicator2ThirdParty.receiveCommand(this->communicator2ThirdParty.getSockfd(), this->cmd1);
    #if PRINT_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "Send the bucket ID to the third party: " << duration.count() << " ns" << std::endl;
    #endif
    this->communicator2ThirdParty.receiveData(this->communicator2ThirdParty.getSockfd(),
                                            this->realBlocksOffsetEarlyReshuffle1.data(),
                                            this->realBlocksOffsetEarlyReshuffle1Size);
    this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(), ServerConfig::CMD_SUCCESS);
    #if USE_COUT
    std::cout << "realBlocksOffsetEarlyReshuffle1: " << std::endl;
    std::cout << this->realBlocksOffsetEarlyReshuffle1.size() << std::endl;
    for (auto& offset : this->realBlocksOffsetEarlyReshuffle1) {
        std::cout << offset << " ";
    }
    std::cout << std::endl;
    #endif
    #if PRINT_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    start = std::chrono::high_resolution_clock::now();
    #endif
    #if LOG_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    this->logger.startTiming(this->LogEarlyReshuffleLoadReadBlocksScheme1);
    #endif
    FileConfig::fileReadScheme1.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(bucketID), std::ios::binary);
    auto it = this->realBlocksDataEarlyReshuffle1.data();
    for (BucketConfig::TYPE_SLOT_ID i = 0; i < BucketConfig::BUCKET_REAL_BLOCK_CAPACITY; ++i) {
        FileConfig::fileReadScheme1.seekg(BucketConfig::META_DATA_SIZE + this->realBlocksOffsetEarlyReshuffle1[i] * BlockConfig::BLOCK_SIZE, std::ios::beg);
        FileConfig::fileReadScheme1.read(it, BlockConfig::BLOCK_SIZE);
        #if USE_COUT
        // Test the correctness of the realBlocksDataEarlyReshuffle1
        // Dcrypt the block data
        unsigned char iv_dec[AES_BLOCK_SIZE];
        std::memset(iv_dec, bucketID + this->realBlocksOffsetEarlyReshuffle1[i], AES_BLOCK_SIZE);
        AES_CTR aes_ctr;
        std::vector<char> blockData(BlockConfig::BLOCK_SIZE);
        aes_ctr.decrypt(reinterpret_cast<const unsigned char*>(it), BlockConfig::BLOCK_SIZE,
                        reinterpret_cast<unsigned char*>(blockData.data()), 
                        iv_dec);
        BlockConfig::TYPE_BLOCK_ID blockID;
        std::memcpy(&blockID, blockData.data(), sizeof(BlockConfig::TYPE_BLOCK_ID));
        std::cout << "Block ID: " << blockID << std::endl;
        #endif
        it += BlockConfig::BLOCK_SIZE;
    }
    FileConfig::fileReadScheme1.close();
    #if LOG_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    this->logger.stopTiming(this->LogEarlyReshuffleLoadReadBlocksScheme1);
    this->logger.writeToFile();
    #endif
    #if PRINT_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "Read real blocks from disk to memory: " << duration.count() << " ns" << std::endl;
    #endif
    #if PRINT_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    start = std::chrono::high_resolution_clock::now();
    #endif
    this->communicator2ThirdParty.sendData(this->communicator2ThirdParty.getSockfd(),
                                        this->realBlocksDataEarlyReshuffle1.data(),
                                        this->realBlocksDataEarlyReshuffle1.size());
    this->communicator2ThirdParty.receiveCommand(this->communicator2ThirdParty.getSockfd(), this->cmd1);
    #if PRINT_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "Send real blocks to the third party: " << duration.count() << " ns" << std::endl;
    #endif
    this->communicator2ThirdParty.receiveData(this->communicator2ThirdParty.getSockfd(),
                                            this->bucketCipherDataEarlyReshuffle1.data(),
                                            this->bucketCipherDataEarlyReshuffle1Size);
    #if USE_COUT
    // Test the correctness of the bucketCipherDataEarlyReshuffle1
    this->TestBucketContent(this->bucketCipherDataEarlyReshuffle1, bucketID);
    #endif
    this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(), ServerConfig::CMD_SUCCESS);

    // Write the bucketCipherDataEarlyReshuffle1 to the disk
    #if PRINT_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    start = std::chrono::high_resolution_clock::now();
    #endif
    #if LOG_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    this->logger.startTiming(this->LogEarlyReshuffleWriteBucketScheme1);
    #endif
    FileConfig::fileWriteScheme1.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(bucketID), std::ios::binary);
    FileConfig::fileWriteScheme1.write(this->bucketCipherDataEarlyReshuffle1.data(), this->bucketCipherDataEarlyReshuffle1Size);
    FileConfig::fileWriteScheme1.close();
    #if LOG_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    this->logger.stopTiming(this->LogEarlyReshuffleWriteBucketScheme1);
    this->logger.writeToFile();
    #endif
    #if PRINT_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "Write the bucketCipherDataEarlyReshuffle1 to the disk: " << duration.count() << " ns" << std::endl;
    #endif
}

void Server::TestBucketContent(std::vector<char>& bucketData, const BucketConfig::TYPE_BUCKET_ID bucketID) {
    BucketConfig::META_DATA md;
    this->tree.DecryptMD(bucketData.data(), bucketID);
    auto it = bucketData.data();
    md.Deserialize(it);
    std::cout << "The meta data of the bucket: " << std::endl;
    md.print();
    it += BucketConfig::META_DATA_SIZE;
    for (BucketConfig::TYPE_BUCKET_SIZE i = 0; i < BucketConfig::BUCKET_SIZE; ++i) {
        AES_CTR aes_ctr;
        unsigned char iv_dec[AES_BLOCK_SIZE];
        std::memset(iv_dec, bucketID + i, AES_BLOCK_SIZE);
        std::vector<char> blockData(BlockConfig::BLOCK_SIZE);
        aes_ctr.decrypt(reinterpret_cast<const unsigned char*>(it), BlockConfig::BLOCK_SIZE,
                        reinterpret_cast<unsigned char*>(blockData.data()),
                        iv_dec);
        BlockConfig::TYPE_BLOCK_ID blockID;
        std::memcpy(&blockID, blockData.data(), sizeof(BlockConfig::TYPE_BLOCK_ID));
        std::cout << "Block ID: " << blockID << std::endl;
        it += BlockConfig::BLOCK_SIZE;
    }
}

void Server::EvictScheme1(PathConfig::TYPE_PATH_ID path_id) {
    #if PRINT_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    auto start = std::chrono::high_resolution_clock::now();
    #endif
    #if LOG_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    this->logger.startTiming(this->LogEvictPathGenBucketIDsScheme1);
    #endif
    // Convert the path_id to the bunch of bucket IDs in the reverselexicographical order
    TreeConfig::GenPathBucketIDsInReverseOrder(path_id,
                                              TreeConfig::HEIGHT,
                                              this->evictPathBucketIDsComplete);
    #if LOG_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    this->logger.stopTiming(this->LogEvictPathGenBucketIDsScheme1);
    this->logger.writeToFile();
    #endif
    #if PRINT_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<long long, std::nano> duration = end - start;
    std::cout << "Convert the path_id to the bunch of bucket IDs in the reverselexicographical order: " << duration.count() << " ns" << std::endl;
    #endif
    #if PRINT_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    start = std::chrono::high_resolution_clock::now();
    #endif
    #if LOG_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    this->logger.startTiming(this->LogEvictPathReadPathBucketsScheme1);
    #endif
    auto it = this->pathBucketsDataEviction1.data();
    // Read the root bucket from the disk to the memory
    FileConfig::fileReadScheme1.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(0), std::ios::binary);
    FileConfig::fileReadScheme1.read(it, this->bucketSizeEviction1);
    FileConfig::fileReadScheme1.close();
    for (const auto& bucketID : this->evictPathBucketIDsComplete) {
        it += this->bucketSizeEviction1;
        FileConfig::fileReadScheme1.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(2 * bucketID + 1), std::ios::binary);
        FileConfig::fileReadScheme1.read(it, this->bucketSizeEviction1);
        FileConfig::fileReadScheme1.close();
        it += this->bucketSizeEviction1;
        FileConfig::fileReadScheme1.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(2 * bucketID + 2), std::ios::binary);
        FileConfig::fileReadScheme1.read(it, this->bucketSizeEviction1);
        FileConfig::fileReadScheme1.close();
    }
    #if LOG_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    this->logger.stopTiming(this->LogEvictPathReadPathBucketsScheme1);
    this->logger.writeToFile();
    #endif
    #if PRINT_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "Read the path buckets from the disk to the memory: " << duration.count() << " ns" << std::endl;
    #endif
    // Process the first triplet buckets
    // 1. Read the triplet buckets MD from the disk to the memory
    #if USE_COUT
    std::cout << "The size of the tripletBucketMDsDataEviction1: " << this->tripletBucketMDsDataEviction1.size() << std::endl;
    std::cout << "The size of the pathBucketsDataEviction1: " << this->pathBucketsDataEviction1.size() << std::endl;
    #endif
    #if PRINT_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    start = std::chrono::high_resolution_clock::now();
    #endif
    #if LOG_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    this->logger.startTiming(this->LogEvictPathLoadTripletBucketMDsScheme1);
    #endif
    it = this->pathBucketsDataEviction1.data();
    auto it_md = this->tripletBucketMDsDataEviction1.data();
    std::memcpy(it_md, this->evictPathBucketIDsComplete.data(), sizeof(BucketConfig::TYPE_BUCKET_ID));
    it_md += sizeof(BucketConfig::TYPE_BUCKET_ID);
    // std::memcpy(it_md, &path_id, sizeof(PathConfig::TYPE_PATH_ID));
    // it_md += sizeof(PathConfig::TYPE_PATH_ID);
    std::memcpy(it_md, &this->curLevel, sizeof(PathConfig::TYPE_PATH_SIZE));
    it_md += sizeof(PathConfig::TYPE_PATH_SIZE);
    for (BucketConfig::TYPE_SMALL_INDEX_U j = 0; j < 3; ++j) {
        if (j == 0) {
            std::memcpy(it_md, it, BucketConfig::META_DATA_SIZE);
        } else {
            it += this->bucketSizeEviction1;
            it_md += BucketConfig::META_DATA_SIZE;
            std::memcpy(it_md, it, BucketConfig::META_DATA_SIZE);
        }
    }
    #if LOG_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    this->logger.stopTiming(this->LogEvictPathLoadTripletBucketMDsScheme1);
    this->logger.writeToFile();
    #endif
    #if PRINT_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "Read the triplet buckets MD from the memory: " << duration.count() << " ns" << std::endl;
    #endif
    #if PRINT_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    start = std::chrono::high_resolution_clock::now();
    #endif
    this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(),
                                            ServerConfig::CMD_EVICT_SCHEME1_SERVER_TO_THIRD_PARTY);
    this->communicator2ThirdParty.receiveCommand(this->communicator2ThirdParty.getSockfd(), this->cmd1);
    // Send the triplet buckets MD to the third party
    this->communicator2ThirdParty.sendData(this->communicator2ThirdParty.getSockfd(),
                                        this->tripletBucketMDsDataEviction1.data(),
                                        this->tripletBucketMDsDataEviction1Size);
    this->communicator2ThirdParty.receiveCommand(this->communicator2ThirdParty.getSockfd(), this->cmd1);
    #if PRINT_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "Send the triplet buckets MD to the third party: " << duration.count() << " ns" << std::endl;
    #endif
    this->communicator2ThirdParty.receiveData(this->communicator2ThirdParty.getSockfd(),
                                this->tripletBucketRealBlocksOffsetEviction1.data(),
                                this->tripletBucketRealBlocksOffsetEviction1Size);
    this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(), ServerConfig::CMD_SUCCESS);
    // cout all the real blocks offset
    #if USE_COUT
    for (BucketConfig::TYPE_SMALL_INDEX_U i = 0; i < 3; ++i) {
        for (BucketConfig::TYPE_SLOT_ID j = 0; j < BucketConfig::BUCKET_REAL_BLOCK_CAPACITY; ++j) {
            std::cout << this->tripletBucketRealBlocksOffsetEviction1[i * BucketConfig::BUCKET_REAL_BLOCK_CAPACITY + j] << " ";
        }
        std::cout << std::endl;
    }
    #endif
    #if PRINT_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    start = std::chrono::high_resolution_clock::now();
    #endif
    #if LOG_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    this->logger.startTiming(this->LogEvictPathLoadRealBlocksScheme1);
    #endif
    it = this->pathBucketsDataEviction1.data();
    auto it_block = this->tripletBucketsRealBlocksDataEviction1.data();
    for (BucketConfig::TYPE_SMALL_INDEX_U i = 0; i < 3; ++i) {
        offset_base = i * BucketConfig::BUCKET_REAL_BLOCK_CAPACITY;
        for (BucketConfig::TYPE_SLOT_ID j = 0; j < BucketConfig::BUCKET_REAL_BLOCK_CAPACITY; ++j) {
            std::memcpy(it_block, it + BucketConfig::META_DATA_SIZE + this->tripletBucketRealBlocksOffsetEviction1[offset_base + j] * BlockConfig::BLOCK_SIZE, BlockConfig::BLOCK_SIZE);
            it_block += BlockConfig::BLOCK_SIZE;
        }
        it += this->bucketSizeEviction1;
    }
    #if LOG_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    this->logger.stopTiming(this->LogEvictPathLoadRealBlocksScheme1);
    this->logger.writeToFile();
    #endif
    #if PRINT_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "Read the real blocks from the memory: " << duration.count() << " ns" << std::endl;
    #endif
    // Send the real blocks to the third party
    #if PRINT_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    start = std::chrono::high_resolution_clock::now();
    #endif
    this->communicator2ThirdParty.sendData(this->communicator2ThirdParty.getSockfd(),
                                        this->tripletBucketsRealBlocksDataEviction1.data(),
                                        this->tripletBucketsRealBlocksDataEviction1Size);
    this->communicator2ThirdParty.receiveCommand(this->communicator2ThirdParty.getSockfd(), this->cmd1);
    #if PRINT_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "Send the triplet buckets real blocks to the third party: " << duration.count() << " ns" << std::endl;
    #endif
    // this->communicator2ThirdParty.receiveData(this->communicator2ThirdParty.getSockfd(),
    //                                         this->pathBucketsDataEviction1.data(),
    //                                         this->tripletBucketsDataEviction1Size);
    // this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(), ServerConfig::CMD_SUCCESS);
    for (BucketConfig::TYPE_SMALL_INDEX_U i = 0; i < 3; ++i) {
        it = this->pathBucketsDataEviction1.data() + i * this->bucketSizeEviction1;
        this->communicator2ThirdParty.receiveData(this->communicator2ThirdParty.getSockfd(),
                                                it,
                                                this->bucketSizeEviction1);
        this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(), ServerConfig::CMD_SUCCESS);
    }
    // this->communicator2ThirdParty.receiveData(this->communicator2ThirdParty.getSockfd(),
    //                                         this->tripletBucketsDataEviction1.data(),
    //                                         this->tripletBucketsDataEviction1Size);
    // this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(), ServerConfig::CMD_SUCCESS);
    // #if PRINT_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    // start = std::chrono::high_resolution_clock::now();
    // #endif
    // it = this->pathBucketsDataEviction1.data();
    // it_block = this->tripletBucketsDataEviction1.data();
    // std::memcpy(it, it_block, this->tripletBucketsDataEviction1Size);
    // #if PRINT_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    // end = std::chrono::high_resolution_clock::now();
    // duration = end - start;
    // std::cout << "Write the triplet buckets to the memory: " << duration.count() << " ns" << std::endl;
    // #endif
    #if USE_COUT
    // Test the correctness ofthe triplet buckets in this->pathBucketsDataEviction1
    for (BucketConfig::TYPE_SMALL_INDEX_U i = 0; i < 3; ++i) {
        std::vector<char> bucketCipherData(this->bucketSizeEviction1);
        std::memcpy(bucketCipherData.data(), this->pathBucketsDataEviction1.data() + i * this->bucketSizeEviction1, this->bucketSizeEviction1);
        this->TestBucketContent(bucketCipherData, i);

    }
    #endif

    for (BucketConfig::TYPE_SLOT_ID i = 1; i < this->evictPathBucketIDsComplete.size(); ++i) {
        #if USE_COUT
        std::cout << "Bucket ID: " << this->evictPathBucketIDsComplete[i] << std::endl;
        #endif
        this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(),
                                                ServerConfig::CMD_EVICT_SCHEME1_SERVER_TO_THIRD_PARTY);
        this->communicator2ThirdParty.receiveCommand(this->communicator2ThirdParty.getSockfd(), this->cmd1);
        #if LOG_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
        this->logger.startTiming(this->LogEvictPathLoadTripletBucketMDsScheme1);
        #endif
        it = this->pathBucketsDataEviction1.data() + (1 + i * 2) * this->bucketSizeEviction1;
        it_md = this->tripletBucketMDsDataEviction1.data();
        std::memcpy(it_md, this->evictPathBucketIDsComplete.data() + i, sizeof(BucketConfig::TYPE_BUCKET_ID));
        it_md += sizeof(BucketConfig::TYPE_BUCKET_ID);
        this->curLevel = i + 1;
        std::memcpy(it_md, &this->curLevel, sizeof(PathConfig::TYPE_PATH_SIZE));
        it_md += sizeof(PathConfig::TYPE_PATH_SIZE);
        for (BucketConfig::TYPE_SMALL_INDEX_U j = 0; j < 3; ++j) {
            if (j == 0) {
                if (this->evictPathBucketIDsComplete[i] % 2 == 0) {
                    std::memcpy(it_md, it - this->bucketSizeEviction1, BucketConfig::META_DATA_SIZE);
                } else {
                    std::memcpy(it_md, it - 2 * this->bucketSizeEviction1, BucketConfig::META_DATA_SIZE);
                }
            } else {
                std::memcpy(it_md, it + (j - 1) * this->bucketSizeEviction1, BucketConfig::META_DATA_SIZE);
            }
            it_md += BucketConfig::META_DATA_SIZE;
        }
        #if LOG_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
        this->logger.stopTiming(this->LogEvictPathLoadTripletBucketMDsScheme1);
        this->logger.writeToFile();
        #endif
        #if USE_COUT
        // Test the correctness of the tripletBucketMDsDataEviction1
        it_md = this->tripletBucketMDsDataEviction1.data() + sizeof(BucketConfig::TYPE_BUCKET_ID) + sizeof(PathConfig::TYPE_PATH_SIZE);
        for (BucketConfig::TYPE_SMALL_INDEX_U j = 0; j < 3; ++j) {
            BucketConfig::META_DATA md;
            BucketConfig::TYPE_BUCKET_ID bucketID;
            if (j == 0) {
                bucketID = this->evictPathBucketIDsComplete[i];
            } else {
                bucketID = this->evictPathBucketIDsComplete[i] * 2 + j;
            }
            std::cout << "Bucket ID: " << bucketID << std::endl;
            this->tree.DecryptMD(it_md, bucketID);
            md.Deserialize(it_md);
            std::cout << "The meta data of the bucket: " << std::endl;
            md.print();
            it_md += BucketConfig::META_DATA_SIZE;
        }
        #endif
        this->communicator2ThirdParty.sendData(this->communicator2ThirdParty.getSockfd(),
                                            this->tripletBucketMDsDataEviction1.data(),
                                            this->tripletBucketMDsDataEviction1Size);
        this->communicator2ThirdParty.receiveCommand(this->communicator2ThirdParty.getSockfd(), this->cmd1);
        this->communicator2ThirdParty.receiveData(this->communicator2ThirdParty.getSockfd(),
                                                this->tripletBucketRealBlocksOffsetEviction1.data(),
                                                this->tripletBucketRealBlocksOffsetEviction1Size);
        this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(), ServerConfig::CMD_SUCCESS);
        #if USE_COUT
        for (BucketConfig::TYPE_SMALL_INDEX_U i = 0; i < 3; ++i) {
            for (BucketConfig::TYPE_SLOT_ID j = 0; j < BucketConfig::BUCKET_REAL_BLOCK_CAPACITY; ++j) {
                std::cout << this->tripletBucketRealBlocksOffsetEviction1[i * BucketConfig::BUCKET_REAL_BLOCK_CAPACITY + j] << " ";
            }
            std::cout << std::endl;
        }
        #endif
        #if LOG_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
        this->logger.startTiming(this->LogEvictPathLoadRealBlocksScheme1);
        #endif
        it_block = this->tripletBucketsRealBlocksDataEviction1.data();
        for (BucketConfig::TYPE_SMALL_INDEX_U ii = 0; ii < 3; ++ii) {
            if (ii == 0) {
                if (this->evictPathBucketIDsComplete[i] % 2 == 0) {
                    it = this->pathBucketsDataEviction1.data() + i * 2 * this->bucketSizeEviction1;
                } else {
                    it = this->pathBucketsDataEviction1.data() + (i * 2 - 1) * this->bucketSizeEviction1;
                }
            } else {
                it = this->pathBucketsDataEviction1.data() + (i * 2 + ii) * this->bucketSizeEviction1;
            }
            for (BucketConfig::TYPE_SLOT_ID j = 0; j < BucketConfig::BUCKET_REAL_BLOCK_CAPACITY; ++j) {
                std::memcpy(it_block, it + BucketConfig::META_DATA_SIZE + this->tripletBucketRealBlocksOffsetEviction1[ii * BucketConfig::BUCKET_REAL_BLOCK_CAPACITY + j] * BlockConfig::BLOCK_SIZE, BlockConfig::BLOCK_SIZE);
                it_block += BlockConfig::BLOCK_SIZE;
            }
            #if USE_COUT
            // Test the correctness of the tripletBucketsRealBlocksDataEviction1
            unsigned char iv_dec[AES_BLOCK_SIZE];
            AES_CTR aes_ctr;
            std::vector<char> blockData(BlockConfig::BLOCK_SIZE);
            auto it_dec = this->tripletBucketsRealBlocksDataEviction1.data() + ii * BucketConfig::BUCKET_REAL_BLOCK_CAPACITY * BlockConfig::BLOCK_SIZE;
            for (BucketConfig::TYPE_SLOT_ID j = 0; j < BucketConfig::BUCKET_REAL_BLOCK_CAPACITY; ++j) {
                if (ii == 0) {
                std::memset(iv_dec, this->evictPathBucketIDsComplete[i] + this->tripletBucketRealBlocksOffsetEviction1[ii * BucketConfig::BUCKET_REAL_BLOCK_CAPACITY + j], AES_BLOCK_SIZE);
            } else {
                std::memset(iv_dec, this->evictPathBucketIDsComplete[i] * 2 + ii + this->tripletBucketRealBlocksOffsetEviction1[ii * BucketConfig::BUCKET_REAL_BLOCK_CAPACITY + j], AES_BLOCK_SIZE);
            }
                aes_ctr.decrypt(reinterpret_cast<const unsigned char*>(it_dec), BlockConfig::BLOCK_SIZE,
                                reinterpret_cast<unsigned char*>(blockData.data()),
                                iv_dec);
                BlockConfig::TYPE_BLOCK_ID blockID;
                std::memcpy(&blockID, blockData.data(), sizeof(BlockConfig::TYPE_BLOCK_ID));
                std::cout << "Block ID: " << blockID << std::endl;
                it_dec += BlockConfig::BLOCK_SIZE;
            }
            #endif
        }
        #if LOG_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
        this->logger.stopTiming(this->LogEvictPathLoadRealBlocksScheme1);
        this->logger.writeToFile();
        #endif
        this->communicator2ThirdParty.sendData(this->communicator2ThirdParty.getSockfd(),
                                            this->tripletBucketsRealBlocksDataEviction1.data(),
                                            this->tripletBucketsRealBlocksDataEviction1Size);
        this->communicator2ThirdParty.receiveCommand(this->communicator2ThirdParty.getSockfd(), this->cmd1);
        for (BucketConfig::TYPE_SLOT_ID ii = 0; ii < 3; ++ii) {
            if (ii == 0) {
                if (this->evictPathBucketIDsComplete[i] % 2 == 0) {
                    it = this->pathBucketsDataEviction1.data() + i * 2 * this->bucketSizeEviction1;
                } else {
                    it = this->pathBucketsDataEviction1.data() + (i * 2 - 1) * this->bucketSizeEviction1;
                }
            } else {
                it = this->pathBucketsDataEviction1.data() + (i * 2 + ii) * this->bucketSizeEviction1;
            }
            // std::vector<char> bucketCipherData(this->bucketSizeEviction1);
            this->communicator2ThirdParty.receiveData(this->communicator2ThirdParty.getSockfd(),
                                                    it,
                                                    this->bucketSizeEviction1);
            this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(), ServerConfig::CMD_SUCCESS);
        }
    }
    #if USE_COUT
    // Test the correctness of the pathBucketsDataEviction1
    it = this->pathBucketsDataEviction1.data();
    std::vector<char> bucketCipherDataTest(this->bucketSizeEviction1);
    std::memcpy(bucketCipherDataTest.data(), it, this->bucketSizeEviction1);
    this->TestBucketContent(bucketCipherDataTest, 0);
    for (const auto& bucketID : this->evictPathBucketIDsComplete) {
        std::cout << "Bucket ID: " << bucketID * 2 + 1 << std::endl;
        it += this->bucketSizeEviction1;
        std::memcpy(bucketCipherDataTest.data(), it, this->bucketSizeEviction1);
        this->TestBucketContent(bucketCipherDataTest, bucketID * 2 + 1);
        std::cout << "++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
        std::cout << "Bucket ID: " << bucketID * 2 + 2 << std::endl;
        it += this->bucketSizeEviction1;
        std::memcpy(bucketCipherDataTest.data(), it, this->bucketSizeEviction1);
        this->TestBucketContent(bucketCipherDataTest, bucketID * 2 + 2);
        std::cout << "++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
    }
    #endif
    // Write the pathBucketsDataEviction1 to the disk
    #if PRINT_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    start = std::chrono::high_resolution_clock::now();
    #endif
    #if LOG_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    this->logger.startTiming(this->LogEvictPathWritePathBucketsScheme1);
    #endif
    it = this->pathBucketsDataEviction1.data();
    FileConfig::fileWriteScheme1.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(0), std::ios::binary);
    FileConfig::fileWriteScheme1.write(it, this->bucketSizeEviction1);
    FileConfig::fileWriteScheme1.close();
    for (const auto& bucketID : this->evictPathBucketIDsComplete) {
        it += this->bucketSizeEviction1;
        FileConfig::fileWriteScheme1.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(2 * bucketID + 1), std::ios::binary);
        FileConfig::fileWriteScheme1.write(it, this->bucketSizeEviction1);
        FileConfig::fileWriteScheme1.close();
        it += this->bucketSizeEviction1;
        FileConfig::fileWriteScheme1.open(BucketConfig::DATADIR + BucketConfig::BUCKETPREFIX + std::to_string(2 * bucketID + 2), std::ios::binary);
        FileConfig::fileWriteScheme1.write(it, this->bucketSizeEviction1);
        FileConfig::fileWriteScheme1.close();
    }
    #if LOG_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    this->logger.stopTiming(this->LogEvictPathWritePathBucketsScheme1);
    this->logger.writeToFile();
    #endif
    #if PRINT_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "Write the path buckets to the disk: " << duration.count() << " ns" << std::endl;
    #endif
}