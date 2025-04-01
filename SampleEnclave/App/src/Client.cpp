#include "Client.h"
#include "DurationLogger.h"
#include <numeric>


Client::Client(ClientConfig::TYPE_HOST host, 
               ClientConfig::TYPE_PORT port) : host(host), port(port),
               tripletBucketIDs{0, 1, 2},
               realBlockNumForEviction(BucketConfig::BUCKET_REAL_BLOCK_CAPACITY),
               tree(TreeConfig::HEIGHT, TreeConfig::TOTAL_NUM_BUCKETS, 
                    TreeConfig::TOTAL_NUM_NON_LEAF_BUCKETS, TreeConfig::TOTAL_NUM_LEAF_BUCKETS, 
                    TreeConfig::REAL_BLOCK_NUM),
                pathComplete(0, TreeConfig::HEIGHT),
                elgamal(ClientConfig::num_threads, BlockConfig::BLOCK_SIZE)
                {
        this->InitConnection();
        this->pathIDChars.resize(sizeof(PathConfig::TYPE_PATH_ID));
        this->pathIDChars.reserve(sizeof(PathConfig::TYPE_PATH_ID));
        this->blockIDChars.resize(sizeof(BlockConfig::TYPE_BLOCK_ID));
        this->blockIDChars.reserve(sizeof(BlockConfig::TYPE_BLOCK_ID));
        offsets.resize(PathConfig::HEIGHT);
        offsetsCharsSize = PathConfig::HEIGHT * sizeof(BucketConfig::TYPE_SLOT_ID);
        offsetsChars.resize(offsetsCharsSize);
        // this->elgamal = ElGamal_parallel_ntl(ClientConfig::num_threads, BlockConfig::BLOCK_SIZE);
        this->targetBlockData.resize(BlockConfig::BLOCK_SIZE);
        this->targetBlockData.reserve(BlockConfig::BLOCK_SIZE);
        this->permDataSize = BucketConfig::BUCKET_SIZE * sizeof(BucketConfig::TYPE_SLOT_ID);
        this->permData1.resize(this->permDataSize);
        this->permData2.resize(this->permDataSize);
        size_t md_size = sizeof(BucketConfig::META_DATA);
        this->rootBucketData.resize(ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
        this->rootBucketData.reserve(ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
        this->evictPermSize = 3 * BucketConfig::BUCKET_SIZE;
        this->original_perms.resize(this->evictPermSize);
        std::iota(this->original_perms.begin(), this->original_perms.end(), 0);
        this->triplet_evict_perm_size = this->evictPermSize * sizeof(BucketConfig::TYPE_SLOT_ID);
        this->triplet_evict_permData1.resize(this->triplet_evict_perm_size);
        this->triplet_evict_permData2.resize(this->triplet_evict_perm_size);
        this->targetBlockCiphertextsSerializedData.reserve(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
        this->targetBlockCiphertextsSerializedData.resize(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);

        // Variables for the complete operations
        this->treeMetaDatas.reserve(TreeConfig::TOTAL_NUM_BUCKETS);
        this->PositionMap.reserve(TreeConfig::REAL_BLOCK_NUM);
        this->PositionMap.resize(TreeConfig::REAL_BLOCK_NUM);
        this->bucketIDOffsets.reserve(TreeConfig::HEIGHT);
        // Initialize the this->isInStash with the size of BucketConfig::REAL_BLOCK_NUM and false
        // this->isInStash.resize(TreeConfig::REAL_BLOCK_NUM, false);
        this->rootBucketDataEvictComplete.reserve(ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
        this->rootBucketDataEvictComplete.resize(ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
        this->rootBlockCiphertextsSerializedEvictComplete.reserve(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
        this->rootBlockCiphertextsSerializedEvictComplete.resize(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
        this->rootBucketCiphertextsEvictComplete.reserve(BucketConfig::BUCKET_SIZE);
        this->rootBucketCiphertextsEvictComplete.resize(BucketConfig::BUCKET_SIZE);
        for (size_t i = 0; i < BucketConfig::BUCKET_SIZE; i++) {
            this->rootBucketCiphertextsEvictComplete[i].reserve(ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
        }
        this->blockIDsDeletedFromStash.reserve(TreeConfig::EVICTION_FREQUENCY);
        this->tripletPermCurComplete.reserve(3 * BucketConfig::BUCKET_SIZE);
        this->tripletPermCurComplete.resize(3 * BucketConfig::BUCKET_SIZE);
        this->tripletPermIntermediateComplete.reserve(3 * BucketConfig::BUCKET_SIZE);
        this->tripletPermIntermediateComplete.resize(3 * BucketConfig::BUCKET_SIZE);
        this->tripletPermTargetComplete.reserve(3 * BucketConfig::BUCKET_SIZE);
        this->tripletPermTargetComplete.resize(3 * BucketConfig::BUCKET_SIZE);
        this->tripletBucketIDsComplete.reserve(3);
        this->tripletBucketIDsComplete.resize(3);
        this->tripletBucketIDsCompleteCharNum = 3 * sizeof(BucketConfig::TYPE_BUCKET_ID);
        this->tripletBucketMDsComplete.reserve(3);
        this->tripletBucketMDsComplete.resize(3);
        this->evictPathBucketIDsComplete.reserve(TreeConfig::HEIGHT - 1);
        this->evictPathBucketIDsComplete.resize(TreeConfig::HEIGHT - 1);
        this->originalPermComplete.reserve(BucketConfig::BUCKET_SIZE);
        this->originalPermComplete.resize(BucketConfig::BUCKET_SIZE);
        LogConfig::CheckLogDir();
        this->logger = DurationLogger(LogConfig::LOG_DIR + LogConfig::LOG_FILE);

        this->pathBucketMDsSerializedDataSize1 = TreeConfig::HEIGHT * BucketConfig::META_DATA_SIZE;
        this->pathBucketMDsSerializedData1.resize(this->pathBucketMDsSerializedDataSize1);
        this->targetBlockCipherData1.resize(BlockConfig::BLOCK_SIZE);
        this->targetBlockData1.resize(BlockConfig::BLOCK_SIZE);
        this->targetBlockData1.reserve(BlockConfig::BLOCK_SIZE);
        this->recoverDummyBlockData1.reserve(BlockConfig::BLOCK_SIZE);
        this->block1.GenData(BlockConfig::BLOCK_SIZE, false, -1, this->recoverDummyBlockData1);
        this->recoverDummyBlockCipherData1.reserve(BlockConfig::BLOCK_SIZE);
        this->recoverDummyBlockCipherData1.resize(BlockConfig::BLOCK_SIZE);
        this->ivIndex1.resize(TreeConfig::HEIGHT - 1);
        this->blockInStashDataSize1 = sizeof(BlockConfig::TYPE_BLOCK_ID) +
                                      sizeof(PathConfig::TYPE_PATH_ID) +
                                      BlockConfig::BLOCK_SIZE;
        this->blocksInStashDataSize1 = this->blockInStashDataSize1 * TreeConfig::EVICTION_FREQUENCY + sizeof(SizeConfig::TYPE_UNSIGNED_SIZE);
        this->blocksInStashData1.resize(this->blocksInStashDataSize1);
        this->bucketCiphertextsBNSgxSize = BucketConfig::BUCKET_SIZE * ElGamalNTLConfig::BLOCK_CHUNK_SIZE;
        this->bucketCiphertextsBNSgx.resize(2);
        this->bucketCiphertextsBNSgx[0].resize(this->bucketCiphertextsBNSgxSize);
        this->bucketCiphertextsBNSgx[1].resize(this->bucketCiphertextsBNSgxSize);
        for (int i = 0; i < this->bucketCiphertextsBNSgxSize; i++) {
            this->bucketCiphertextsBNSgx[0][i] = BN_new();
            this->bucketCiphertextsBNSgx[1][i] = BN_new();
        }
        // triplet_evict_perm_size
        this->tripletPermIntermediateCompleteBothSize = 2 * this->evictPermSize;
        this->tripletPermIntermediateCompleteBoth.resize(this->tripletPermIntermediateCompleteBothSize);

        this->opensslTargetBlockCiphertextsBN.resize(2);
        this->opensslTargetBlockCiphertextsBN[0].resize(ElGamalNTLConfig::BLOCK_CHUNK_SIZE);
        this->opensslTargetBlockCiphertextsBN[1].resize(ElGamalNTLConfig::BLOCK_CHUNK_SIZE);
        this->opensslTargetBlockDataBN.resize(ElGamalNTLConfig::BLOCK_CHUNK_SIZE);
        for (size_t i = 0; i < ElGamalNTLConfig::BLOCK_CHUNK_SIZE; i++) {
            this->opensslTargetBlockCiphertextsBN[0][i] = BN_new();
            this->opensslTargetBlockCiphertextsBN[1][i] = BN_new();
            this->opensslTargetBlockDataBN[i] = BN_new();
        }
        this->opensslRootBucketCiphertextsBN.resize(2);
        this->opensslRootBucketCiphertextsBN[0].resize(ElGamalNTLConfig::BUCKET_CHUNK_SIZE);
        this->opensslRootBucketCiphertextsBN[1].resize(ElGamalNTLConfig::BUCKET_CHUNK_SIZE);
        for (size_t i = 0; i < ElGamalNTLConfig::BUCKET_CHUNK_SIZE; i++) {
            this->opensslRootBucketCiphertextsBN[0][i] = BN_new();
            this->opensslRootBucketCiphertextsBN[1][i] = BN_new();
        }
        
}
Client::~Client() {
    for (int i = 0; i < this->bucketCiphertextsBNSgxSize; i++) {
        if (this->bucketCiphertextsBNSgx[0][i] != nullptr) {
            BN_free(this->bucketCiphertextsBNSgx[0][i]);
            this->bucketCiphertextsBNSgx[0][i] = nullptr;
        }
        if (this->bucketCiphertextsBNSgx[1][i] != nullptr) {
            BN_free(this->bucketCiphertextsBNSgx[1][i]);
            this->bucketCiphertextsBNSgx[1][i] = nullptr;
        }
    }
    for (size_t i = 0; i < ElGamalNTLConfig::BLOCK_CHUNK_SIZE; i++) {
        if (this->opensslTargetBlockCiphertextsBN[0][i] != nullptr) {
            BN_free(this->opensslTargetBlockCiphertextsBN[0][i]);
            this->opensslTargetBlockCiphertextsBN[0][i] = nullptr;
        }
        if (this->opensslTargetBlockCiphertextsBN[1][i] != nullptr) {
            BN_free(this->opensslTargetBlockCiphertextsBN[1][i]);
            this->opensslTargetBlockCiphertextsBN[1][i] = nullptr;
        }
        if (this->opensslTargetBlockDataBN[i] != nullptr) {
            BN_free(this->opensslTargetBlockDataBN[i]);
            this->opensslTargetBlockDataBN[i] = nullptr;
        }
    }
    for (size_t i = 0; i < ElGamalNTLConfig::BUCKET_CHUNK_SIZE; i++) {
        if (this->opensslRootBucketCiphertextsBN[0][i] != nullptr) {
            BN_free(this->opensslRootBucketCiphertextsBN[0][i]);
            this->opensslRootBucketCiphertextsBN[0][i] = nullptr;
        }
        if (this->opensslRootBucketCiphertextsBN[1][i] != nullptr) {
            BN_free(this->opensslRootBucketCiphertextsBN[1][i]);
            this->opensslRootBucketCiphertextsBN[1][i] = nullptr;
        }
    }
    for (auto& block : this->blockDataStash) {
        for (auto& ciphertext : block.second.first[0]) {
            BN_free(ciphertext);
        }
        for (auto& ciphertext : block.second.first[1]) {
            BN_free(ciphertext);
        }
    }
}
void Client::InitConnection() {
    this->communicator.connectToServer(this->host, this->port);
    std::cout << "Connected to the server at " << this->host << ":" << this->port << std::endl;
}

void Client::EnsureConnection() {
    if (!this->communicator.isConnected()) {
        this->InitConnection();
    }
}

void Client::InitReadPath(PathConfig::TYPE_PATH_ID path_id) {
    EnsureConnection();
    this->communicator.sendCommand(this->communicator.getSockfd(),
                                      ServerConfig::CMD_CREATE_DB);
    std::memcpy(this->pathIDChars.data(), &path_id, sizeof(PathConfig::TYPE_PATH_ID));
    this->communicator.sendData(this->communicator.getSockfd(), this->pathIDChars);
    #if USE_ASSERT
    Path path(path_id, PathConfig::HEIGHT);
    path.GenPath(PathConfig::REAL_BLOCK_NUM, this->mds, true);
    #endif
    // this->tree.GenTree(this->treeMetaDatas, true);
    // this->tree.GenPathMDs(this->treeMetaDatas, path_id);
    Path path(path_id, PathConfig::HEIGHT);
    path.GenPathMetaDatas(this->treeMetaDatas);
    for (size_t i = 0; i < TreeConfig::REAL_BLOCK_NUM; i++) {
        this->PositionMap[i] = i % TreeConfig::TOTAL_NUM_LEAF_BUCKETS;
    }
    this->PositionMap[ClientConfig::TARGET_BLOCK_ID] = path_id;
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
}

void Client::ReadPath(BlockConfig::TYPE_BLOCK_ID block_id, PathConfig::TYPE_PATH_ID path_id) {
    #if READ_PATH_SIMULATION
    // EnsureConnection();
    this->communicator.sendCommand(this->communicator.getSockfd(),
                                      ServerConfig::CMD_READ_PATH);

    // send the path ID to the server
    this->communicator.sendData(this->communicator.getSockfd(),
                                reinterpret_cast<char*>(&path_id),
                                sizeof(path_id));
    // Get the offsets from the this->mds
    for (PathConfig::TYPE_PATH_SIZE i = 0; i < PathConfig::HEIGHT; i++) {
        mds[i].LogAccess();
        if (this->mds[i].ContainsBlockID(block_id)) {
            for (BucketConfig::TYPE_SLOT_ID j = 0; j < mds[i].nextRealIndex; j++) {
                if (mds[i].addrs[j] == block_id && mds[i].valids[j]) {
                    offsets[i] = mds[i].offsets[j];
                    mds[i].valids[mds[i].offsets[j]] = false;
                    mds[i].leaves[j] = PathConfig::GenPathID();
                }
            }
        } else {
            if (this->mds[i].valids[mds[i].offsets[mds[i].nextDummyIndex - 1]]) {
                offsets[i] = mds[i].offsets[mds[i].nextDummyIndex - 1];
                mds[i].valids[mds[i].offsets[mds[i].nextDummyIndex - 1]] = false;
            }
            mds[i].nextDummyIndex--;
        }
    }
    // std::cout << "Offsets: ";
    // for (size_t i = 0; i < offsets.size(); i++) {
    //     std::cout << offsets[i] << " ";
    // }
    // std::cout << std::endl;
    this->communicator.sendData(this->communicator.getSockfd(), 
                                this->offsets.data(),
                                this->offsetsCharsSize);
    // this->communicator.receiveDataWithoutKnownSize(this->communicator.getSockfd(), this->targetBlockCiphertextsSerializedData);
    this->communicator.receiveData(this->communicator.getSockfd(), 
                                   this->targetBlockCiphertextsSerializedData.data(),
                                   ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
     this->elgamal.DeserializeCiphertexts(this->targetBlockCiphertextsSerializedData, this->targetBlockCiphertexts);
    this->elgamal.ParallelDecrypt(this->targetBlockCiphertexts, this->targetBlockData);
    // BlockConfig::TYPE_BLOCK_ID blockID;
    // std::memcpy(&blockID, this->targetBlockData.data(), sizeof(BlockConfig::TYPE_BLOCK_ID));
    // std::cout << "Block ID: " << blockID << std::endl;
    #else
        this->ReadPathComplete(block_id);
        // erase all the data in the stash
        // this->blockDataStash.erase(0);
    #endif 
}

void Client::InitEarlyReshuffle() {
    this->EnsureConnection();
    this->communicator.sendCommand(this->communicator.getSockfd(),
                                   ServerConfig::CMD_EARLY_RESHUFFLE_INIT);
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    #if EARLY_RESHUFFLE_SIMULATION
    Path path(0, PathConfig::HEIGHT);
    path.GenRootBucket(0, this->rootBucketMD, BucketConfig::BUCKET_REAL_BLOCK_CAPACITY - 1, true);
    #else
    // this->tree.GenTree(this->treeMetaDatas, true);
    this->tree.GenPathMDs(this->treeMetaDatas, 0);
    for (size_t i = 0; i < TreeConfig::REAL_BLOCK_NUM; i++) {
        this->PositionMap[i] = i % TreeConfig::TOTAL_NUM_LEAF_BUCKETS;
    }
    #endif
}


void Client::EarlyReshuffle(BucketConfig::TYPE_BUCKET_ID bucket_id) {
    #if EARLY_RESHUFFLE_SIMULATION
    this->communicator.sendCommand(this->communicator.getSockfd(),
                                   ServerConfig::CMD_EARLY_RESHUFFLE);
    this->rootBucketMD.ReShuffleOffsets();
    std::vector<BucketConfig::TYPE_SLOT_ID> original_offsets = std::move(this->rootBucketMD.Offsets2Vector());
    this->rootBucketMD.DividePerm(original_offsets, perm1, perm2);
    this->communicator.sendData(this->communicator.getSockfd(), this->perm2.data(), this->permDataSize);  
    this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(),
                                             ServerConfig::CMD_EARLY_RESHUFFLE_CLIENT_THRID_PARTY);
    // this->communicator2ThirdParty.sendData(this->communicator2ThirdParty.getSockfd(),
    //                                        this->permData1);
    this->communicator2ThirdParty.sendData(this->communicator2ThirdParty.getSockfd(),
                                          this->perm1.data(), this->permDataSize);
                                       
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    #else
    this->EarlyReshuffleComplete(bucket_id);
    #endif
}
/*
* Create a triplet of buckets including bucket 0, bucket 1, and bucket 2
*/
void Client::InitEviction(PathConfig::TYPE_PATH_ID path_id) {
    this->communicator.sendCommand(this->communicator.getSockfd(),
                                   ServerConfig::CMD_EVICT_SERVER_INIT);
    #if EVICT_SIMULATION
    Path path(0, PathConfig::HEIGHT);
    this->tripletBucketMDs.clear();
    path.GenTripletBuckets(this->tripletBucketIDs, this->tripletBucketMDs, this->realBlockNumForEviction, true);
    #else
    // this->tree.GenTree(this->treeMetaDatas, true);
    // this->tree.GenTree(this->treeMetaDatas, path_id);
    this->tree.GenEvictPathMDs(this->treeMetaDatas, path_id);
    for (size_t i = 0; i < TreeConfig::REAL_BLOCK_NUM; i++) {
        this->PositionMap[i] = i % TreeConfig::TOTAL_NUM_LEAF_BUCKETS;
    }
    #endif
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
}


void Client::CommunicateRootBucket() {
    this->communicator.sendCommand(this->communicator.getSockfd(),
                                   ServerConfig::CMD_EVICT_COMMUNICATION_ROOT_BUCKET);
    //Step1: Retrieve the root bucket from the server and send it back
    this->communicator.receiveData(this->communicator.getSockfd(), this->rootBucketData.data(), ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
    this->communicator.sendData(this->communicator.getSockfd(), this->rootBucketData.data(), ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
    // this->communicator.receiveData(this->communicator.getSockfd(), this->rootBucketData, ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
    // this->communicator.sendData(this->communicator.getSockfd(), this->rootBucketData);
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
}


void Client::Evict() {
    // Generate the re-order permutation index from 0 to 3 * BUCKET_SIZE - 1
    this->rootBucketMD.DividePerm(this->original_perms,
                                  this->triplet_evict_perm1,
                                  this->triplet_evict_perm2);
    this->communicator.sendCommand(this->communicator.getSockfd(),
                                   ServerConfig::CMD_EVICT_SERVER_OPERATION);
    this->communicator.sendData(this->communicator.getSockfd(),
                                this->triplet_evict_perm2.data(),
                                this->triplet_evict_perm_size);
    this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(),
                                            ServerConfig::CMD_EVICT_THIRD_PARTY);
    this->communicator2ThirdParty.sendData(this->communicator2ThirdParty.getSockfd(),
                                          this->triplet_evict_perm1.data(),
                                          this->triplet_evict_perm_size);
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    // std::cout << "Eviction is done!" << std::endl; 
}

void Client::InitCreateBinaryTree() {
    this->EnsureConnection();
    this->communicator.sendCommand(this->communicator.getSockfd(),
                                   ServerConfig::CMD_COMPLETE_CREATE_BINARY_TREE);
    this->tree.GenTree(this->treeMetaDatas, true);
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    for (BlockConfig::TYPE_BLOCK_SIZE i = 0; i < TreeConfig::REAL_BLOCK_NUM; ++i) {
        this->PositionMap[i] = i % TreeConfig::TOTAL_NUM_LEAF_BUCKETS;
    }
    // // Print the PositionMap
    // std::cout << "PositionMap: ";
    // for (auto& p : this->PositionMap) {
    //     std::cout << p << " ";
    // }
    // std::cout << std::endl;

}

void Client::ReadPathComplete(BlockConfig::TYPE_BLOCK_ID block_id) {
    this->communicator.sendCommand(this->communicator.getSockfd(),
                                   ServerConfig::CMD_COMPLETE_READ_PATH);
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    // Get the path ID of the block with block_id
    this->pathIDComplete = this->PositionMap[block_id];
    // std::cout << "The current path ID of the block " << block_id << " is: " << this->pathIDComplete << std::endl;
    #if LOG_BREAKDOWN_COST
    this->logger.startTiming(this->LogReadPathSendPathIDScheme2);
    #endif
    this->communicator.sendData(this->communicator.getSockfd(),
                                reinterpret_cast<char*>(&this->pathIDComplete),
                                sizeof(this->pathIDComplete));
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    #if LOG_BREAKDOWN_COST
    this->logger.stopTiming(this->LogReadPathSendPathIDScheme2);
    this->logger.writeToFile();
    #endif
    // Update the the this->PositionMap[block_id] to the new path ID
    #if LOG_BREAKDOWN_COST
    this->logger.startTiming(this->LogReadPathUpdatePathIDScheme2);
    #endif
    this->PositionMap[block_id] = PathConfig::GenPathID();
    #if LOG_BREAKDOWN_COST
    this->logger.stopTiming(this->LogReadPathUpdatePathIDScheme2);
    this->logger.writeToFile();
    #endif
    #if USE_ASSERT
        assert(this->PositionMap[block_id] >= 0 && this->PositionMap[block_id] < TreeConfig::TOTAL_NUM_LEAF_BUCKETS);
    #endif
    // Collect the corresponding offsets
    #if LOG_BREAKDOWN_COST
    this->logger.startTiming(this->LogReadPathGenOffsetsScheme2);
    #endif
    this->pathComplete.ConvertPID2BIDs(this->pathIDComplete, TreeConfig::HEIGHT, this->bucketIDOffsets);
    this->counter = 0;
    for (const auto& bucketID : this->bucketIDOffsets) {
        this->treeMetaDatas[bucketID].LogAccess();
        if (this->treeMetaDatas[bucketID].ContainsBlockID(block_id)) {
            #if USE_ASSERT
            bool flag = false;
            #endif
            for (BucketConfig::TYPE_SLOT_ID j = 0; j < this->treeMetaDatas[bucketID].nextRealIndex; ++j) {
                if (this->treeMetaDatas[bucketID].addrs[j] == block_id && this->treeMetaDatas[bucketID].valids[this->treeMetaDatas[bucketID].offsets[j]]) {
                    #if USE_COUT
                    std::cout << "Block ID: " << block_id << " is found in the bucket " << bucketID << std::endl;
                    #endif
                    this->offsets[this->counter] = this->treeMetaDatas[bucketID].offsets[j];
                    this->treeMetaDatas[bucketID].valids[this->treeMetaDatas[bucketID].offsets[j]] = false;
                    // this->treeMetaDatas[bucketID].leaves[j] = this->PositionMap[block_id];
                    this->treeMetaDatas[bucketID].addrs[j] = -1;
                    this->treeMetaDatas[bucketID].leaves[j] = -1;
                    #if USE_ASSERT
                    flag = true;
                    #endif
                    break;
                } 
            }
            #if USE_ASSERT
            if (!flag) {
                std::cout << "The block ID: " << block_id << " is not found in the bucket " << bucketID << std::endl;
            }
            #endif
            this->treeMetaDatas[bucketID].DeleteBlockID(block_id);
            this->treeMetaDatas[bucketID].OrganizeRealBlocksIDs(); 
        } else {
            if (this->treeMetaDatas[bucketID].valids[this->treeMetaDatas[bucketID].offsets[this->treeMetaDatas[bucketID].nextDummyIndex - 1]]) {
                this->offsets[this->counter] = this->treeMetaDatas[bucketID].offsets[this->treeMetaDatas[bucketID].nextDummyIndex - 1];
                this->treeMetaDatas[bucketID].valids[this->treeMetaDatas[bucketID].offsets[this->treeMetaDatas[bucketID].nextDummyIndex - 1]] = false;
            }
            this->treeMetaDatas[bucketID].nextDummyIndex--;
            #if USE_ASSERT
                assert(this->treeMetaDatas[bucketID].nextDummyIndex != BucketConfig::BUCKET_REAL_BLOCK_CAPACITY - 1 && "[ReadPath]The nextDummyIndex is not correct!");
            #endif
        }
        this->counter++;
    }
    #if LOG_BREAKDOWN_COST
    this->logger.stopTiming(this->LogReadPathGenOffsetsScheme2);
    this->logger.writeToFile();
    #endif
    #if LOG_BREAKDOWN_COST
    this->logger.startTiming(this->LogReadPathSendOffsetsScheme2);
    #endif
    this->communicator.sendData(this->communicator.getSockfd(), 
                                this->offsets.data(),
                                this->offsetsCharsSize);
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    #if LOG_BREAKDOWN_COST
    this->logger.stopTiming(this->LogReadPathSendOffsetsScheme2);
    this->logger.writeToFile();
    #endif
    // std::cout << "The size of the offsets: " << this->offsetsCharsSize << std::endl;
    this->communicator.receiveData(this->communicator.getSockfd(),
                                  this->targetBlockCiphertextsSerializedData,
                                  ElGamalNTLConfig::BLOCK_CIPHERTEXT_NUM_CHARS);
    this->communicator.sendCommand(this->communicator.getSockfd(), ServerConfig::CMD_SUCCESS);
    // this->elgamal.DeserializeCiphertexts(this->targetBlockCiphertextsSerializedData, this->targetBlockCiphertexts);
    // this->elgamal.ParallelDecrypt(this->targetBlockCiphertexts, this->targetBlockData);
    if (this->blockDataStash.find(block_id) == this->blockDataStash.end()) {
        this->elgamal.ConvertVecCharCipher2VecBN(
            this->targetBlockCiphertextsSerializedData,
            this->opensslTargetBlockCiphertextsBN
        );
        #if LOG_BREAKDOWN_COST
        this->logger.startTiming(this->LogReadPathDecryptTargetBlockScheme2);
        #endif
        this->elgamal.ParallelDecrypt(this->opensslTargetBlockCiphertextsBN[0], 
            this->opensslTargetBlockCiphertextsBN[1],
            this->opensslTargetBlockDataBN
        );
        #if LOG_BREAKDOWN_COST
        this->logger.stopTiming(this->LogReadPathDecryptTargetBlockScheme2);
        this->logger.writeToFile();
        #endif 
        #if UNIT_TEST_OPENSSL_FINAL_CHECK
            BIGNUM* targetBlockIDBN = BN_new();
            BN_set_word(targetBlockIDBN, block_id);
            for (size_t i = 0; i < ElGamalNTLConfig::BLOCK_CHUNK_SIZE; i++) {
                assert(
                    BN_cmp(this->opensslTargetBlockDataBN[i], targetBlockIDBN) == 0 && 
                    "The block ID is not correct!"
                );
                std::cout << "The block ID: " << BN_get_word(this->opensslTargetBlockDataBN[i]) << std::endl;
            }
            BN_free(targetBlockIDBN);
        #endif
        
        
        this->elgamal.ConvertVecBN2VecChar(
            this->opensslTargetBlockDataBN,
            this->targetBlockData,
            ElGamalNTLConfig::CHUNK_SIZE
        );

        this->blockDataStash[block_id].second = this->targetBlockData;
        this->blockDataStash[block_id].first.resize(2);
        this->blockDataStash[block_id].first[0].resize(ElGamalNTLConfig::BLOCK_CHUNK_SIZE);
        this->blockDataStash[block_id].first[1].resize(ElGamalNTLConfig::BLOCK_CHUNK_SIZE);
        for (size_t i = 0; i < ElGamalNTLConfig::BLOCK_CHUNK_SIZE; i++) {
            this->blockDataStash[block_id].first[0][i] = BN_new();
            this->blockDataStash[block_id].first[1][i] = BN_new();
        }
        // this->blockDataStash[block_id].first = std::move(this->opensslTargetBlockCiphertextsBN);
        for (size_t i = 0; i < ElGamalNTLConfig::BLOCK_CHUNK_SIZE; i++) {
            BN_copy(this->blockDataStash[block_id].first[0][i], this->opensslTargetBlockCiphertextsBN[0][i]);
            BN_copy(this->blockDataStash[block_id].first[1][i], this->opensslTargetBlockCiphertextsBN[1][i]);
        }
        
    } else {
        #if USE_PRINT_TARGET_BLOCK
            std::cout << "The block ID: " << block_id << " is already in the stash!" << std::endl;
        #endif
        this->targetBlockData = this->blockDataStash[block_id].second;
    }
    
    
}
void Client::EarlyReshuffleComplete(BucketConfig::TYPE_BUCKET_ID bucket_id) {
    std::vector<char> buffer_sgx;
    buffer_sgx.resize(this->permDataSize * 2 + sizeof(bucket_id));
    this->communicator.sendCommand(this->communicator.getSockfd(),
                                   ServerConfig::CMD_COMPLETE_EARLY_RESHUFFLE);
    #if LOG_BREAKDOWN_COST
    this->logger.startTiming(this->LogEarlyReshuffleGenPermsScheme2);
    #endif
    this->treeMetaDatas[bucket_id].reset(this->originalPermComplete);
    this->treeMetaDatas[bucket_id].DividePerm(this->originalPermComplete, 
                                                this->perm1Complete,
                                                this->perm2Complete);
    #if LOG_BREAKDOWN_COST
    this->logger.stopTiming(this->LogEarlyReshuffleGenPermsScheme2);
    this->logger.writeToFile();
    #endif
    auto it = buffer_sgx.begin();
    std::memcpy(&(*it), this->perm1Complete.data(), this->permDataSize);
    it += this->permDataSize;
    std::memcpy(&(*it), this->perm2Complete.data(), this->permDataSize);
    it += this->permDataSize;
    std::memcpy(&(*it), &bucket_id, sizeof(bucket_id));
    #if LOG_BREAKDOWN_COST
    this->logger.startTiming(this->LogEarlyReshuffleSendPermToServerScheme2);
    #endif
    this->communicator.sendData(this->communicator.getSockfd(), buffer_sgx.data(), buffer_sgx.size());
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    #if LOG_BREAKDOWN_COST
    this->logger.stopTiming(this->LogEarlyReshuffleSendPermToServerScheme2);
    this->logger.writeToFile();
    #endif
}

void Client::EvictComplete(PathConfig::TYPE_PATH_ID path_id) {
    // EnsureConnection();
    this->communicator.sendCommand(this->communicator.getSockfd(),
                                   ServerConfig::CMD_COMPLETE_EVICT_CLIENT_TO_SERVER);
    this->communicator.receiveData(this->communicator.getSockfd(),
                                  this->rootBucketDataEvictComplete.data(),
                                  ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
    this->communicator.sendCommand(this->communicator.getSockfd(), ServerConfig::CMD_SUCCESS);
    this->elgamal.ConvertVecCharCipher2VecBN(this->rootBucketDataEvictComplete, this->bucketCiphertextsBNSgx);
    auto start = std::chrono::high_resolution_clock::now();
    this->blockIDsDeletedFromStash.clear();
    for (auto& block : this->blockDataStash) {
        if (this->treeMetaDatas[rootBucketIDComplete].nextRealIndex < BucketConfig::BUCKET_REAL_BLOCK_CAPACITY) {
            this->treeMetaDatas[rootBucketIDComplete].AddRealBlock(block.first, this->PositionMap[block.first]);
            this->blockIDsDeletedFromStash.emplace_back(block.first);
            for (int i = 0; i < ElGamalNTLConfig::BLOCK_CHUNK_SIZE; ++i) {
                int base_index = this->treeMetaDatas[rootBucketIDComplete].offsets[
                    this->treeMetaDatas[rootBucketIDComplete].nextRealIndex - 1] * ElGamalNTLConfig::BLOCK_CHUNK_SIZE;
                BN_copy(this->opensslRootBucketCiphertextsBN[0][base_index + i], block.second.first[0][i]);
                BN_copy(this->opensslRootBucketCiphertextsBN[1][base_index + i], block.second.first[1][i]);
            }
        }
    }
    this->treeMetaDatas[rootBucketIDComplete].SimpleReset();
    for (const auto& blockID : this->blockIDsDeletedFromStash) {
        auto it = this->blockDataStash.find(blockID);
        if (it != this->blockDataStash.end()) {
            for (int i = 0; i < ElGamalNTLConfig::BLOCK_CHUNK_SIZE; ++i) {
                BN_free(it->second.first[0][i]);
                BN_free(it->second.first[1][i]);
            }
            this->blockDataStash.erase(it);
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "ClientComputationUpdateMetaData:" << elapsed_ns.count() << " ns" << std::endl;
    start = std::chrono::high_resolution_clock::now();
    this->elgamal.ParallelRerandomize(this->bucketCiphertextsBNSgx[0], this->bucketCiphertextsBNSgx[1]);
    end = std::chrono::high_resolution_clock::now();
    elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "ClientComputationRerandomization: " << elapsed_ns.count() << " ns" << std::endl;
    // Serialize the blockCiphertexts in the rootBucketCiphertextsEvictComplete into the rootBucketDataEvictComplete
    this->elgamal.ConvertVecBNCipher2VecChar(this->bucketCiphertextsBNSgx[0], this->bucketCiphertextsBNSgx[1], this->rootBucketDataEvictComplete);
    start = std::chrono::high_resolution_clock::now();
    this->communicator.sendData(this->communicator.getSockfd(),
                                this->rootBucketDataEvictComplete.data(),
                                ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    end = std::chrono::high_resolution_clock::now();
    elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "ClientCommunicationSendRootBucket: " << elapsed_ns.count() << " ns" << std::endl;
    // Prepare the data structure for the GenTripletEvictPerms
    for (BucketConfig::TYPE_SMALL_INDEX_U i = 0; i < 3; ++i) {
        this->tripletBucketIDsComplete[i] = i;
        this->tripletBucketMDsComplete[i] = std::move(this->treeMetaDatas[
                                        this->tripletBucketIDsComplete[i]]);
    }
    // std::cout << "The tripletBucketMDsComplete: " << std::endl;
    start = std::chrono::high_resolution_clock::now();
    this->GenTripletEvictPerms(this->tripletBucketMDsComplete,
                                this->tripletBucketIDsComplete,
                                TreeConfig::HEIGHT,
                                1,
                                this->tripletPermCurComplete,
                                this->tripletPermTargetComplete);
    for (const auto& bucketID : this->tripletBucketIDsComplete) {
        this->treeMetaDatas[bucketID] = std::move(this->tripletBucketMDsComplete[bucketID]);
    }
    // Call the function GenIntermediatePerm
    BucketConfig::GenIntermediatePerm(this->tripletPermCurComplete,
                                      this->tripletPermTargetComplete,
                                      this->tripletPermIntermediateComplete);
    // std::cout << "The tripletPermIntermediateComplete: ";
    // for (const auto& p : this->tripletPermIntermediateComplete) {
    //     std::cout << p << " ";
    // }
    // std::cout << std::endl;
    BucketConfig::META_DATA::DividePerm(this->tripletPermIntermediateComplete,
                                        this->tripletPermIntermediateCompleteThirdParty,
                                        this->tripletPermIntermediateCompleteServer);
    end = std::chrono::high_resolution_clock::now();
    elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "ClientComputationGenTripletEvictPerms: " << elapsed_ns.count() << " ns" << std::endl;
    // std::cout << "The tripletPermIntermediateCompleteThirdParty: ";
    // for (const auto& p : this->tripletPermIntermediateCompleteThirdParty) {
    //     std::cout << p << " ";
    // }
    // std::cout << std::endl;
    // std::cout << "The tripletPermIntermediateCompleteServer: ";
    // for (const auto& p : this->tripletPermIntermediateCompleteServer) {
    //     std::cout << p << " ";
    // }
    // std::cout << std::endl;
    // #if LOG_EVICT_BREAKDOWN_COST_CLIENT
    //     this->logger.stopTiming(this->LogEvictGenTripletEvictPermsScheme2);
    //     this->logger.writeToFile();
    // #endif
    std::memcpy(this->tripletPermIntermediateCompleteBoth.data(), this->tripletPermIntermediateCompleteThirdParty.data(), this->triplet_evict_perm_size);
    std::memcpy(this->tripletPermIntermediateCompleteBoth.data() + this->evictPermSize, this->tripletPermIntermediateCompleteServer.data(), this->triplet_evict_perm_size);
    // std::cout << "The tripletPermIntermediateCompleteBoth: ";
    // for (const auto& p : this->tripletPermIntermediateCompleteBoth) {
    //     std::cout << p << " ";
    // }
    // std::cout << std::endl;
    start = std::chrono::high_resolution_clock::now();
    this->communicator.sendData(this->communicator.getSockfd(),
                                this->tripletPermIntermediateCompleteBoth.data(),
                                this->tripletPermIntermediateCompleteBothSize * sizeof(BucketConfig::TYPE_SLOT_ID));
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    end = std::chrono::high_resolution_clock::now();
    elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "ClientCommunicationSendTripletEvictPerms: " << elapsed_ns.count() << " ns" << std::endl;
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    TreeConfig::GenPathBucketIDsInReverseOrder(path_id,
                                              TreeConfig::HEIGHT,
                                              this->evictPathBucketIDsComplete);
    for (PathConfig::TYPE_PATH_SIZE i = 1; i < this->evictPathBucketIDsComplete.size(); ++i) {
        // std::cout << "The Bucket ID: " << this->evictPathBucketIDsComplete[i] << std::endl;
        start = std::chrono::high_resolution_clock::now();
        for (BucketConfig::TYPE_SMALL_INDEX_U ii = 0; ii < 3; ++ii) {
            if (ii == 0) {
                this->tripletBucketIDsComplete[ii] = this->evictPathBucketIDsComplete[i];
            } else {
                this->tripletBucketIDsComplete[ii] = this->tripletBucketIDsComplete[0] * 2 + ii;
            }
            this->tripletBucketMDsComplete[ii] = std::move(this->treeMetaDatas[this->tripletBucketIDsComplete[ii]]);
        }
        this->GenTripletEvictPerms(this->tripletBucketMDsComplete,
                                    this->tripletBucketIDsComplete,
                                    TreeConfig::HEIGHT,
                                    i + 1,
                                    this->tripletPermCurComplete,
                                    this->tripletPermTargetComplete);
        for (BucketConfig::TYPE_SMALL_INDEX_U ii = 0; ii < 3; ++ii) {
            this->treeMetaDatas[this->tripletBucketIDsComplete[ii]] = std::move(this->tripletBucketMDsComplete[ii]);
        }
        BucketConfig::GenIntermediatePerm(this->tripletPermCurComplete,
                                        this->tripletPermTargetComplete,
                                        this->tripletPermIntermediateComplete);
        BucketConfig::META_DATA::DividePerm(this->tripletPermIntermediateComplete,
                                            this->tripletPermIntermediateCompleteThirdParty,
                                            this->tripletPermIntermediateCompleteServer);
        end = std::chrono::high_resolution_clock::now();
        elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        std::cout << "ClientComputationGenTripletEvictPerms: " << elapsed_ns.count() << " ns" << std::endl;
        // std::cout << "The tripletPermIntermediateCompleteThirdParty: ";
        // for (const auto& p : this->tripletPermIntermediateCompleteThirdParty) {
        //     std::cout << p << " ";
        // }
        // std::cout << std::endl;
        // std::cout << "The tripletPermIntermediateCompleteServer: ";
        // for (const auto& p : this->tripletPermIntermediateCompleteServer) {
        //     std::cout << p << " ";
        // }
        // std::cout << std::endl;
        // this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(),
        //                                          ServerConfig::CMD_COMPLETE_EVICT_CLIENT_TO_THIRD_PARTY);
      
        // this->communicator2ThirdParty.sendData(this->communicator2ThirdParty.getSockfd(),
        //                                         this->tripletPermIntermediateCompleteThirdParty.data(),
        //                                         this->triplet_evict_perm_size);
        // this->communicator2ThirdParty.receiveCommand(this->communicator2ThirdParty.getSockfd(), this->cmd);
       
        std::memcpy(this->tripletPermIntermediateCompleteBoth.data(), this->tripletPermIntermediateCompleteThirdParty.data(), this->triplet_evict_perm_size);
        std::memcpy(this->tripletPermIntermediateCompleteBoth.data() + this->evictPermSize, this->tripletPermIntermediateCompleteServer.data(), this->triplet_evict_perm_size);
        start = std::chrono::high_resolution_clock::now();
        this->communicator.sendData(this->communicator.getSockfd(),
                                    this->tripletPermIntermediateCompleteBoth.data(),
                                    this->tripletPermIntermediateCompleteBothSize * sizeof(BucketConfig::TYPE_SLOT_ID));
        this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
        end = std::chrono::high_resolution_clock::now();
        elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        std::cout << "ClientCommunicationSendTripletEvictPerms: " << elapsed_ns.count() << " ns" << std::endl;

        this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    }
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
}

void Client::GenTripletEvictPerms(std::vector<BucketConfig::META_DATA>& tripletBucketMDs,
                                 std::vector<BucketConfig::TYPE_BUCKET_ID>& tripletBucketIDs,
                                 PathConfig::TYPE_PATH_SIZE height,
                                 PathConfig::TYPE_PATH_SIZE curLevel,
                                 std::vector<BucketConfig::TYPE_SLOT_ID>& tripletPermCur,
                                 std::vector<BucketConfig::TYPE_SLOT_ID>& tripletPermTarget) {
    #if USE_ASSERT
    assert(tripletBucketIDs.size() == 3 && tripletBucketMDs.size() == 3);
    assert(tripletPermCur.size() == 3 * BucketConfig::BUCKET_SIZE && tripletPermTarget.size() == 3 * BucketConfig::BUCKET_SIZE);
    #endif
    this->swapIndexes.clear();
    for (BucketConfig::TYPE__SMALL_INDEX j = tripletBucketMDs[0].nextRealIndex -1; j >= 0; --j) {
        // BlockConfig::TYPE_BLOCK_ID blockID = tripletBucketMDs[0].addrs[j];
        // assert(blockID != -1 && "The block ID is not correct in GenTripletEvictPerms!");
        // PathConfig::TYPE_PATH_ID pathID = tripletBucketMDs[0].leaves[j];
        // if (TreeConfig::CanBlockGoThroughBucket(pathID, TreeConfig::HEIGHT, tripletBucketIDs[1], curLevel)
        if (TreeConfig::CanBlockGoThroughBucket(tripletBucketMDs[0].leaves[j], TreeConfig::HEIGHT, tripletBucketIDs[1], curLevel)
        && tripletBucketMDs[1].nextRealIndex < BucketConfig::BUCKET_REAL_BLOCK_CAPACITY)  {
            // Add the blockID to the bucket 1
            // tripletBucketMDs[1].AddRealBlockPure(blockID, pathID);
            tripletBucketMDs[1].AddRealBlock(tripletBucketMDs[0].addrs[j], tripletBucketMDs[0].leaves[j]);
            tripletBucketMDs[0].DenoteDeletedRealBlock(j);
            // std::swap(tripletPermTarget[j], tripletPermTarget[tripletBucketMDs[1].nextRealIndex -1 + BucketConfig::BUCKET_SIZE]);
            #if USE_ASSERT
                assert(tripletBucketMDs[1].nextRealIndex != 0 && "The nextRealIndex in the bucket 1 is not correct!");
            #endif
            swapIndexes.emplace_back(j, tripletBucketMDs[1].nextRealIndex - 1 + BucketConfig::BUCKET_SIZE);
        // } else if (TreeConfig::CanBlockGoThroughBucket(pathID, TreeConfig::HEIGHT, tripletBucketIDs[2], curLevel)
        } else if (TreeConfig::CanBlockGoThroughBucket(tripletBucketMDs[0].leaves[j], TreeConfig::HEIGHT, tripletBucketIDs[2], curLevel)
                    && tripletBucketMDs[2].nextRealIndex < BucketConfig::BUCKET_REAL_BLOCK_CAPACITY) {
            // Add the blockID to the bucket 2
            // tripletBucketMDs[2].AddRealBlockPure(blockID, pathID);
            tripletBucketMDs[2].AddRealBlock(tripletBucketMDs[0].addrs[j], tripletBucketMDs[0].leaves[j]);
            tripletBucketMDs[0].DenoteDeletedRealBlock(j);
            // std::swap(tripletPermTarget[j], tripletPermTarget[tripletBucketMDs[2].nextRealIndex - 1 + 2 * BucketConfig::BUCKET_SIZE]);
            #if USE_ASSERT
                assert(tripletBucketMDs[2].nextRealIndex != 0 && "The nextRealIndex in the bucket 2 is not correct!");
            #endif
            swapIndexes.emplace_back(j, tripletBucketMDs[2].nextRealIndex - 1 + 2 * BucketConfig::BUCKET_SIZE);
        }
    }
    #if USE_ASSERT
        assert(tripletBucketIDs.size() == 3 && tripletBucketMDs.size() == 3);
    #endif
    for (BucketConfig::TYPE_SMALL_INDEX_U i = 0; i < tripletBucketIDs.size(); ++i) {
        tripletBucketMDs[i].SimpleReset();
        for (BucketConfig::TYPE_BUCKET_SIZE j = 0; j < BucketConfig::BUCKET_SIZE; ++j) {
            tripletPermCur[i * BucketConfig::BUCKET_SIZE + j] = tripletBucketMDs[i].offsets[j] + i * BucketConfig::BUCKET_SIZE;
            tripletPermTarget[i * BucketConfig::BUCKET_SIZE + j] = tripletPermCur[i * BucketConfig::BUCKET_SIZE + j];
        }
        // Apply the swap operation on tripletPermTarget according to the swapIndexes
    }
    for (const auto& swapIndex : swapIndexes) {
        std::swap(tripletPermTarget[swapIndex.first], tripletPermTarget[swapIndex.second]);
    }
    tripletBucketMDs[0].OrganizeRealBlocksIDs();

}


void Client::InitBinaryTreeScheme1() {
    this->communicator.sendCommand(this->communicator.getSockfd(),
                                   ServerConfig::CMD_INIT_BINARY_TREE_SCHEME1_CLIENT_TO_SERVER);
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    // Prepare the position map
    for (size_t i = 0; i < TreeConfig::REAL_BLOCK_NUM; i++) {
        this->PositionMap[i] = i % TreeConfig::TOTAL_NUM_LEAF_BUCKETS;
    }
}


void Client::ReadPathScheme1(BlockConfig::TYPE_BLOCK_ID block_id) {
    #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1
    auto start = std::chrono::high_resolution_clock::now();
    #endif
    this->communicator.sendCommand(this->communicator.getSockfd(),
                                   ServerConfig::CMD_READ_PATH_SCHEME1_CLIENT_TO_SERVER);
    // this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    this->communicator.sendData(this->communicator.getSockfd(),
                                &this->PositionMap[block_id],
                                sizeof(this->pathIDComplete));
    // this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<long long, std::nano> duration = end - start;
    std::cout << "The time for sending the path ID: " << duration.count() << " ns" << std::endl;
    #endif
    #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1
    start = std::chrono::high_resolution_clock::now();
    #endif
    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT
    this->logger.startTiming(this->LogReadPathGenBIDsScheme1);
    #endif
    ++TreeConfig::ACCESS_COUNT_EVICTION_COMPLETE;
    TreeConfig::ACCESS_COUNT_EVICTION_COMPLETE %= TreeConfig::EVICTION_FREQUENCY;
    this->pathComplete.ConvertPID2BIDs(this->PositionMap[block_id], TreeConfig::HEIGHT, this->bucketIDOffsets);
    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT
    this->logger.stopTiming(this->LogReadPathGenBIDsScheme1);
    this->logger.writeToFile();
    #endif
    #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "[Client Computation] The time for converting pID to bID: " << duration.count() << " ns" << std::endl;
    #endif
    // Update the path ID of the block with block_id to a new path ID
     #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1
    start = std::chrono::high_resolution_clock::now();
    #endif
    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT
    this->logger.startTiming(this->LogReadPathUpdatePathIDScheme1);
    #endif
    this->PositionMap[block_id] = PathConfig::GenPathID();
    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT
    this->logger.stopTiming(this->LogReadPathUpdatePathIDScheme1);
    this->logger.writeToFile();
    #endif
     #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "[Client Computation] The time for update pID: " << duration.count() << " ns" << std::endl;
    #endif
    this->communicator.receiveData(this->communicator.getSockfd(),
                                  this->pathBucketMDsSerializedData1.data(),
                                    this->pathBucketMDsSerializedDataSize1);
    this->communicator.sendCommand(this->communicator.getSockfd(), ServerConfig::CMD_SUCCESS);
    #if USE_COUT
    std::cout << "The size of the pathBucketMDsSerializedData1: " << this->pathBucketMDsSerializedDataSize1 << std::endl;
    auto it = this->pathBucketMDsSerializedData1.data();
    // this->tree.DecryptMD(it, this->bucketIDOffsets[0]);
    std::cout << "The bucket ID: " << this->bucketIDOffsets[0] << std::endl;
    size_t blockSize = 1;
    std::cout << "Size of size_t: " << sizeof(size_t) << std::endl;
    std::memcpy(&blockSize, it + sizeof(size_t) + BucketConfig::BUCKET_SIZE, sizeof(size_t));
    std::cout << "The block size: " << blockSize << std::endl;
    // for (PathConfig::TYPE_PATH_SIZE i = 0; i < TreeConfig::HEIGHT; ++i) {
    //     this->tree.DecryptMD(it, this->bucketIDOffsets[i]);
    //     this->bucketMD1.Deserialize(it);
    //     std::cout << "Before the read operation: " << std::endl;
    //     this->bucketMD1.print();
    //     it += BucketConfig::META_DATA_SIZE;
    // }
    #endif
    #if USE_COUT
    std::cout << "The size of the pathBucketMDsSerializedData1: " << this->pathBucketMDsSerializedDataSize1 << std::endl;
    // Decrypt the meta data
    std::cout << "The meta data of the buckets: " << std::endl;
    #endif
    #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1
    start = std::chrono::high_resolution_clock::now();
    #endif
    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT
    this->logger.startTiming(this->LogReadPathGenOffsetsScheme1);
    #endif
    auto it = this->pathBucketMDsSerializedData1.data();
    this->counter = 0;
    this->findTargetBlock1 = false;
    for (PathConfig::TYPE_PATH_SIZE i = 0; i < TreeConfig::HEIGHT; ++i) {
        this->tree.DecryptMD(it, this->bucketIDOffsets[i]);
        this->bucketMD1.Deserialize(it);
        // std::cout << "Before the read operation: " << std::endl;
        // this->bucketMD1.print();

        // Check the block ID in the bucket or not and update the meta data
        this->bucketMD1.LogAccess(); 
        if (this->bucketMD1.ContainsBlockID(block_id)) {
            #if PRINT_READ_PATH_ASSERT_SCHEME1
            assert(this->bucketMD1.nextRealIndex != 0 && "[Read Path]The nextRealIndex is not correct!");
            #endif
            for (BucketConfig::TYPE_SLOT_ID j = 0; j < this->bucketMD1.nextRealIndex; ++j) {
                if (this->bucketMD1.addrs[j] == block_id && this->bucketMD1.valids[this->bucketMD1.offsets[j]]) {
                    // this->targetBucketSlotID1 = i;
                    // this->targetBucketID1 = i;
                    this->targetBlockIndex1 = i;
                    this->offsets[i] = this->bucketMD1.offsets[j];
                    this->bucketMD1.valids[this->bucketMD1.offsets[j]] = false;
                    this->bucketMD1.leaves[j] = -1;
                    this->bucketMD1.addrs[j] = -1;
                    this->bucketMD1.DeleteBlockID(block_id);
                    this->bucketMD1.OrganizeRealBlocksIDs();
                    this->findTargetBlock1 = true;
                    break;
                }
            }
        } else {
            if (this->bucketMD1.valids[this->bucketMD1.offsets[this->bucketMD1.nextDummyIndex - 1]]) {
                this->offsets[i] = this->bucketMD1.offsets[this->bucketMD1.nextDummyIndex - 1];
                this->bucketMD1.valids[this->bucketMD1.offsets[this->bucketMD1.nextDummyIndex - 1]] = false;
                this->bucketMD1.nextDummyIndex--;
                this->ivIndex1[this->counter++] = this->bucketIDOffsets[i] + this->offsets[i];
                #if PRINT_READ_PATH_ASSERT_SCHEME1
                assert(this->bucketMD1.nextDummyIndex != BucketConfig::BUCKET_REAL_BLOCK_CAPACITY && "[ReadPath]The nextDummyIndex is not correct!");
                #endif
            }

        }
        // write the meta data back to the server
        this->bucketMD1.Serialize(it);
        this->tree.EncryptMD(it, this->bucketIDOffsets[i]);

        #if USE_COUT
        // Test the meta data after the read operation
        std::cout << "After the read operation: " << std::endl;
        this->tree.DecryptMD(it, this->bucketIDOffsets[i]);
        this->bucketMD1.Deserialize(it);
        this->bucketMD1.print();
        #endif

        it += BucketConfig::META_DATA_SIZE;
        // this->counter++;
    }
    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT
    this->logger.stopTiming(this->LogReadPathGenOffsetsScheme1);
    this->logger.writeToFile();
    #endif
    #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "[Client Computation] The time for generating the offsets: " << duration.count() << " ns" << std::endl;
    #endif
    #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1
    start = std::chrono::high_resolution_clock::now();
    #endif
    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT
    this->logger.startTiming(this->LogReadPathSendOffsetsSchme1);
    #endif
    this->communicator.sendData(this->communicator.getSockfd(),
                                this->offsets.data(),
                                this->offsetsCharsSize);
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT
    this->logger.stopTiming(this->LogReadPathSendOffsetsSchme1);
    this->logger.writeToFile();
    #endif
    #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "The time for sending the offsets: " << duration.count() << " ns" << std::endl;
    #endif
    #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1
    start = std::chrono::high_resolution_clock::now();
    #endif
    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT
    this->logger.startTiming(this->LogReadPathSendPathMDsScheme1);
    #endif
    this->communicator.sendData(this->communicator.getSockfd(),
                                this->pathBucketMDsSerializedData1.data(),
                                this->pathBucketMDsSerializedDataSize1);
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT
    this->logger.stopTiming(this->LogReadPathSendPathMDsScheme1);
    this->logger.writeToFile();
    #endif
    #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "The time for sending the MDs back to Server: " << duration.count() << " ns" << std::endl;
    #endif
    #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1
    start = std::chrono::high_resolution_clock::now();
    #endif
    this->communicator.receiveData(this->communicator.getSockfd(),
                                    this->targetBlockCipherData1.data(),
                                    BlockConfig::BLOCK_SIZE);
    this->communicator.sendCommand(this->communicator.getSockfd(), ServerConfig::CMD_SUCCESS);
    #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "The time for receiving the target block: " << duration.count() << " ns" << std::endl;
    #endif
    #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1
    start = std::chrono::high_resolution_clock::now();
    #endif
    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT
    this->logger.startTiming(this->LogReadPathGenDummyXORScheme1);
    #endif
    // Generate the corresponding ciphertext for the dummy blocks
    for (const auto& index : this->ivIndex1) {
        #if USE_COUT
        std::cout << "The index: " << index << std::endl;
        #endif
        std::memset(iv_client, index, AES_BLOCK_SIZE);
        this->aes_client.encrypt(reinterpret_cast<const unsigned char*>(this->recoverDummyBlockData1.data()),
                                BlockConfig::BLOCK_SIZE,
                                reinterpret_cast<unsigned char*>(this->recoverDummyBlockCipherData1.data()),
                                iv_client);
        uint64_t* targetData = reinterpret_cast<uint64_t*>(this->targetBlockCipherData1.data());
        uint64_t* recoverData = reinterpret_cast<uint64_t*>(this->recoverDummyBlockCipherData1.data());
        // size_t numBlocks = BlockConfig::BLOCK_SIZE / sizeof(uint64_t);
        for (size_t i = 0; i < AESConfig::CHUNK_SIZE_BLOCK_XOR; i++) {
            targetData[i] ^= recoverData[i];
        }

    }
    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT
    this->logger.stopTiming(this->LogReadPathGenDummyXORScheme1);
    this->logger.writeToFile();
    #endif
    #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "[Client Computation] The time for generating the dummy blocks and xor: " << duration.count() << " ns" << std::endl;
    #endif
    // Decrypt the target block
    #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1
    start = std::chrono::high_resolution_clock::now();
    #endif
    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT
    this->logger.startTiming(this->LogReadPathDecryptTargetBlockScheme1);
    #endif
    this->targetBlockData1.resize(BlockConfig::BLOCK_SIZE);
    std::memset(iv_client, this->offsets[this->targetBlockIndex1] + this->bucketIDOffsets[this->targetBlockIndex1], AES_BLOCK_SIZE);
    this->aes_client.decrypt(reinterpret_cast<const unsigned char*>(this->targetBlockCipherData1.data()),
                            BlockConfig::BLOCK_SIZE,
                            reinterpret_cast<unsigned char*>(this->targetBlockData1.data()),
                            iv_client);
    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT
    this->logger.stopTiming(this->LogReadPathDecryptTargetBlockScheme1);
    this->logger.writeToFile();
    #endif
    #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "[Client Computation] The time for decrypting the target block: " << duration.count() << " ns" << std::endl;
    #endif
    #if ASSERT_CORRECTNESS_TARGET_BLOCK_SCHEME1
    BlockConfig::TYPE_BLOCK_ID blockID;
    std::memcpy(&blockID, this->targetBlockData1.data(), sizeof(BlockConfig::TYPE_BLOCK_ID));
    // assert(blockID == block_id && "The block ID is not correct!");
    std::cout << "The block ID: " << blockID << std::endl;
    #endif
    #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1
    start = std::chrono::high_resolution_clock::now();
    #endif
    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT
    this->logger.startTiming(this->LogReadPathStoreAccessedBlock2StashScheme1);
    #endif
    // Store the target block into the blockDataStash
    if (findTargetBlock1) {
        this->blockDataStash1[block_id].first = this->PositionMap[block_id];
        this->blockDataStash1[block_id].second = std::move(this->targetBlockData1);
    } 
    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT_BLOCKS_STASH
    this->logger.startTiming(this->LogReadPathStoreAccessedBlock2StashScheme1);
    #endif
    if (TreeConfig::ACCESS_COUNT_EVICTION_COMPLETE == 0) {
            // Write the size of the blockDataStash1 to the this->blockInStashDataSize1
            it = this->blocksInStashData1.data();
            this->blockInStashDataSize1 = this->blockDataStash1.size();
            std::memcpy(it, &this->blockInStashDataSize1, sizeof(SizeConfig::TYPE_UNSIGNED_SIZE));
            it += sizeof(SizeConfig::TYPE_UNSIGNED_SIZE); 
            for (auto& block : this->blockDataStash1) {
                ClientConfig::SerializeBlockStash(block.first,
                                                block.second.first,
                                                block.second.second.data(),
                                                BlockConfig::BLOCK_SIZE,
                                                it);
                it += this->blockInStashDataSize1;
            }
            // clear the blockDataStash1
            this->blockDataStash1.clear();
            this->communicator2ThirdParty.sendCommand(this->communicator2ThirdParty.getSockfd(),
                                                    ServerConfig::CMD_STASH_SCHEME1_CLIENT_TO_THIRD_PARTY);
            this->communicator2ThirdParty.receiveCommand(this->communicator2ThirdParty.getSockfd(),
                                                        this->cmd);
            this->communicator2ThirdParty.sendData(this->communicator2ThirdParty.getSockfd(),
                                                this->blocksInStashData1.data(),
                                                this->blocksInStashDataSize1);
            this->communicator2ThirdParty.receiveCommand(this->communicator2ThirdParty.getSockfd(),
                                                        this->cmd);
    }
    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT_BLOCKS_STASH
    this->logger.stopTiming(this->LogReadPathStoreAccessedBlock2StashScheme1);
    this->logger.writeToFile();
    #endif
    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT
    this->logger.stopTiming(this->LogReadPathStoreAccessedBlock2StashScheme1);
    this->logger.writeToFile();
    #endif
    #if PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1
    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "[Client Computation] The time for storing the target block to stash: " << duration.count() << " ns" << std::endl;
    #endif
    #if USE_COUT
    std::cout << "The size of the blockDataStash1: " << this->blockDataStash1.size() << std::endl;
    std::cout << "Element in the blockDataStash1: " << std::endl;
    for (const auto& block : this->blockDataStash1) {
        std::cout << block.first << " ";
        BlockConfig::TYPE_BLOCK_ID blockID;
        std::memcpy(&blockID, block.second.second.data(), sizeof(BlockConfig::TYPE_BLOCK_ID));
        std::cout << blockID << std::endl;
    }
    std::cout << std::endl;
    #endif
    #if USE_COUT  
    // Check out the data in the blockDataStash1
    for (const auto& block : this->blockDataStash1) {
        BlockConfig::TYPE_BLOCK_ID blockID;
        std::memcpy(&blockID, block.second.second.data(), sizeof(BlockConfig::TYPE_BLOCK_ID));
        std::cout << "The block ID: " << blockID << std::endl;
    }
    #endif
    // Create a buffer to store the block data and its path ID
    std::vector<char> buffer(BlockConfig::BLOCK_SIZE + sizeof(PathConfig::TYPE_PATH_ID));
    std::memcpy(buffer.data(), &this->blockDataStash1[block_id].first, sizeof(PathConfig::TYPE_PATH_ID));
    std::memcpy(buffer.data() + sizeof(PathConfig::TYPE_PATH_ID), this->blockDataStash1[block_id].second.data(), BlockConfig::BLOCK_SIZE);
    // Encrypt above buffer via this->aes_client
    std::memset(iv_client, block_id, AES_BLOCK_SIZE);
    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT
    this->logger.startTiming(this->LogReadPathEncryptBlockWithPathIdScheme1);
    #endif
    this->aes_client.encrypt(reinterpret_cast<const unsigned char*>(buffer.data()),
                            buffer.size(),
                            reinterpret_cast<unsigned char*>(buffer.data()),
                            iv_client);
    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT
    this->logger.stopTiming(this->LogReadPathEncryptBlockWithPathIdScheme1);
    this->logger.writeToFile();
    #endif

    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT
    this->logger.startTiming(this->LogReadPathSendAccessedBlocks2ThirdPartyScheme1);
    #endif
    // Send the encrypted buffer to the server
    this->communicator.sendData(this->communicator.getSockfd(),
                                buffer.data(),
                                buffer.size());
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    #if LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT
    this->logger.stopTiming(this->LogReadPathSendAccessedBlocks2ThirdPartyScheme1);
    this->logger.writeToFile();
    #endif
}

void Client::EarlyReshuffleScheme1(BucketConfig::TYPE_BUCKET_ID bucket_id) {
    this->communicator.sendCommand(this->communicator.getSockfd(),
                                   ServerConfig::CMD_EARLY_RESHUFFLE_SCHEME1_CLIENT_TO_SERVER);
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    this->communicator.sendData(this->communicator.getSockfd(),
                                reinterpret_cast<char*>(&bucket_id),
                                sizeof(bucket_id));
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
}

void Client::EvictScheme1(PathConfig::TYPE_PATH_ID path_id) {
    #if PRINT_EVICT_DELAY_FOR_SCHEME1
    auto start = std::chrono::high_resolution_clock::now();
    #endif
    this->communicator.sendCommand(this->communicator.getSockfd(),
                                      ServerConfig::CMD_EVICT_SCHEME1_CLIENT_TO_SERVER);
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    this->communicator.sendData(this->communicator.getSockfd(),
                                reinterpret_cast<char*>(&path_id),
                                sizeof(path_id));
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    #if PRINT_EVICT_DELAY_FOR_SCHEME1
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<long long, std::nano> duration = end - start;
    std::cout << "The time for eviction: " << duration.count() << " ns" << std::endl;
    #endif
}

void Client::TestBucket(std::string& pathDir, BucketConfig::TYPE_BUCKET_ID bucketID) {
    FileConfig::fileReadScheme1.open(pathDir + std::to_string(bucketID), std::ios::binary);
    std::vector<char> buffer;
    buffer.resize(BucketConfig::META_DATA_SIZE);
    BucketConfig::META_DATA metaData;
    std::cout << "Before loading the bucket: " << std::endl;
    metaData.print();
    std::cout << std::endl;
    FileConfig::fileReadScheme1.read(buffer.data(), BucketConfig::META_DATA_SIZE);
    this->tree.DecryptMD(buffer, bucketID);
    metaData.Deserialize(buffer);
    std::cout << "After loading the bucket: " << std::endl;
    metaData.print();
    std::cout << std::endl;
    std::vector<char> blockBuffer(BlockConfig::BLOCK_SIZE);
    for (BucketConfig::TYPE_SLOT_ID i = 0; i < BucketConfig::BUCKET_SIZE; ++i) {
        buffer.resize(BlockConfig::BLOCK_SIZE);
        FileConfig::fileReadScheme1.read(buffer.data(), BlockConfig::BLOCK_SIZE);
        std::memset(iv_client, bucketID + i, AES_BLOCK_SIZE);
        this->aes_client.decrypt(reinterpret_cast<const unsigned char*>(buffer.data()),
                                BlockConfig::BLOCK_SIZE,
                                reinterpret_cast<unsigned char*>(blockBuffer.data()),
                                iv_client);
        BlockConfig::TYPE_BLOCK_ID blockID;
        std::memcpy(&blockID, blockBuffer.data(), sizeof(BlockConfig::TYPE_BLOCK_ID));
        std::cout << "Block ID: " << blockID << std::endl;
    }
}

void Client::InitPathScheme1(PathConfig::TYPE_PATH_ID path_id) {
    this->communicator.sendCommand(this->communicator.getSockfd(),
                                   ServerConfig::CMD_INIT_PATH_SCHEME1_CLIENT_TO_SERVER);
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    this->communicator.sendData(this->communicator.getSockfd(),
                                reinterpret_cast<char*>(&path_id),
                                sizeof(path_id));
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
}

void Client::InitSingleBucketScheme1(BucketConfig::TYPE_BUCKET_ID bucket_id) {
    this->communicator.sendCommand(this->communicator.getSockfd(),
                                   ServerConfig::CMD_INIT_SINGLE_BUCKET_SCHEME1_CLIENT_TO_SERVER);
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    this->communicator.sendData(this->communicator.getSockfd(),
                                reinterpret_cast<char*>(&bucket_id),
                                sizeof(bucket_id));
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
}

void Client::InitEvictPathScheme1(PathConfig::TYPE_PATH_ID path_id) {
    this->communicator.sendCommand(this->communicator.getSockfd(),
                                   ServerConfig::CMD_INIT_EVICT_PATH_SCHEME1_CLIENT_TO_SERVER);
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    this->communicator.sendData(this->communicator.getSockfd(),
                                reinterpret_cast<char*>(&path_id),
                                sizeof(path_id));
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
}

void Client::Test() {
    this->communicator.sendCommand(this->communicator.getSockfd(),
                                   ServerConfig::CMD_TEST_CLIENT_TO_SERVER);
    this->communicator.receiveCommand(this->communicator.getSockfd(), this->cmd);
    std::vector<char> encryptedData(256, 0);
    std::vector<char> decryptedData(256, 3);
    this->communicator.receiveData(this->communicator.getSockfd(),
                                  encryptedData.data(),
                                  256);
    this->communicator.sendCommand(this->communicator.getSockfd(), ServerConfig::CMD_SUCCESS);
    BlockConfig::TYPE_BLOCK_ID blockID;
    std::memset(iv_client, 0, AES_BLOCK_SIZE);
    std::memcpy(&blockID, encryptedData.data(), sizeof(BlockConfig::TYPE_BLOCK_ID));
    this->aes_client.decrypt(reinterpret_cast<const unsigned char*>(encryptedData.data()),
                            256,
                            reinterpret_cast<unsigned char*>(decryptedData.data()),
                            iv_client);
    std::memcpy(&blockID, decryptedData.data(), sizeof(BlockConfig::TYPE_BLOCK_ID));
    std::cout << "The block ID: " << blockID << std::endl;
}

