#include <iostream>
#include <vector>
#include <random>
// #include "ElGamal.h"  // Include the header file for the ElGamal class
// #include "ECElGamal_parallel.h"
#include "DurationLogger.h"
#include <fstream>
#include <sys/stat.h>
#include <chrono>
#include <time.h>
#include <cassert>
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/ec.h>
// #include "ElGamal_standard.h"
// #include "ElGamal_parallel.h"
// #include "ElGamal_standard_ntl.h"
#include "ElGamal_parallel_ntl.h"
#include "config.h"
#include "Bucket.h"
#include "Block.h"
#include "Path.h"
#include "Client.h"
#include "Tree.h"
#include "AES_CTR.h"    
using namespace std::chrono;

void InitializeElGamalParams() {
    std::cout << "Initializing ElGamal Parameters..." << std::endl;
    
    // Set seed for random number generation
    SetSeed(ElGamalNTLConfig::SEED);
    
    // Generate prime P
    GenPrime(ElGamalNTLConfig::P, ElGamalNTLConfig::KEY_SIZE);
    ZZ_p::init(ElGamalNTLConfig::P);
//     std::cout << "P: " << ElGamalNTLConfig::P << std::endl;
    
    // Convert G to G_p
    ElGamalNTLConfig::G_p = conv<ZZ_p>(ElGamalNTLConfig::G);
//     std::cout << "G_p: " << ElGamalNTLConfig::G_p << std::endl;
    
    // Generate private key X and convert to X_p
    ElGamalNTLConfig::X = RandomLen_ZZ(ElGamalNTLConfig::RANDOM_SIZE);
    ElGamalNTLConfig::X_p = conv<ZZ_p>(ElGamalNTLConfig::X);
//     std::cout << "X_p: " << ElGamalNTLConfig::X_p << std::endl;
    
    // Calculate public key Y and convert to Y_p
    ElGamalNTLConfig::Y = PowerMod(ElGamalNTLConfig::G, ElGamalNTLConfig::X, ElGamalNTLConfig::P);
    ElGamalNTLConfig::Y_p = conv<ZZ_p>(ElGamalNTLConfig::Y);
//     std::cout << "Y_p: " << ElGamalNTLConfig::Y_p << std::endl;
    
    // Generate random K and convert to K_p
    ElGamalNTLConfig::K = RandomLen_ZZ(ElGamalNTLConfig::RANDOM_SIZE);
    ElGamalNTLConfig::K_p = conv<ZZ_p>(ElGamalNTLConfig::K);
//     std::cout << "K_p: " << ElGamalNTLConfig::K_p << std::endl;
     ElGamalNTLConfig::GPowK = power(ElGamalNTLConfig::G_p, ElGamalNTLConfig::K);
     ElGamalNTLConfig::YPowK = power(ElGamalNTLConfig::Y_p, ElGamalNTLConfig::K);
}


int main(int argc, char* argv[]) {
    LogConfig::CheckLogDir();
    InitializeElGamalParams();
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
   } else if (argv[1] == std::string("client_read_path")) {
          #if LOG_READ_PATH_TOTAL_DELAY
            LogConfig::CheckLogDir();
            DurationLogger logger(LogConfig::LOG_DIR + LogConfig::LOG_FILE);
          #endif 
          // warm up
          for (int ii = 0; ii < 1; ii++) {
                std::cout << "Iteration " << ii << "for warm up" << std::endl;
                Client client(ClientConfig::HOST, ClientConfig::PORT);
                client.InitReadPath(ClientConfig::PATH_ID);
                client.ReadPath(ClientConfig::TARGET_BLOCK_ID, ClientConfig::PATH_ID);
                client.blockDataStash.erase(0);
          }
          for (int i = 0; i < 0; i++) {
                #if LOG_READ_PATH_TOTAL_DELAY
                    std::cout << "Iteration " << i << std::endl;
                #endif 
                Client client(ClientConfig::HOST, ClientConfig::PORT);
               client.InitReadPath(ClientConfig::PATH_ID);
               #if LOG_READ_PATH_TOTAL_DELAY
                    logger.startTiming("ReadPathTotalDelay");
               #endif
               client.ReadPath(ClientConfig::TARGET_BLOCK_ID, ClientConfig::PATH_ID);
               #if LOG_READ_PATH_TOTAL_DELAY
                    logger.stopTiming("ReadPathTotalDelay");
                    logger.writeToFile();
               #endif
               client.blockDataStash.erase(0);
          }
    } else if (argv[1] == std::string("client_early_reshuffle")) {
         #if LOG_EARLY_RESHUFFLE_TOTAL_DELAY
        //  LogConfig::CheckLogDir();
         DurationLogger logger(LogConfig::LOG_DIR + LogConfig::LOG_FILE);
         #endif
         Client client(ClientConfig::HOST, ClientConfig::PORT);
         client.InitEarlyReshuffle();
          for (int i = 0; i < 5; i++) {
                std::cout << "Iteration " << i << std::endl;
                client.EarlyReshuffle();
          }
         for (int i = 0; i < 0; i++) {
               #if LOG_EARLY_RESHUFFLE_TOTAL_DELAY
               std::cout << "Iteration " << i << std::endl;
               logger.startTiming("InitEarlyReshuffleTotalDelay");
               #endif
               client.EarlyReshuffle();
                #if LOG_EARLY_RESHUFFLE_TOTAL_DELAY
               logger.stopTiming("InitEarlyReshuffleTotalDelay");
               logger.writeToFile();
                #endif
         }
    } else if (argv[1] == std::string("client_eviction")) {
            #if LOG_EVICT_TOTAL_DELAY
            // LogConfig::CheckLogDir();
            DurationLogger logger(LogConfig::LOG_DIR + LogConfig::LOG_FILE);
            #endif 
            // warm up
            Client client(ClientConfig::HOST, ClientConfig::PORT);
            for (int i = 0; i < 2; i++) {
                std::cout << "Iteration " << i << "for warm up" << std::endl;
                client.InitEviction();
                client.EvictComplete(ClientConfig::PATH_ID);
            }
            for (int i = 0; i < 0; ++i) {
                Client client(ClientConfig::HOST, ClientConfig::PORT);
                client.InitEviction();
                #if LOG_EVICT_TOTAL_DELAY
                std::cout << "Iteration " << i << std::endl;
                logger.startTiming("EvictionTotalDelay");
                #endif
                client.EvictComplete(ClientConfig::PATH_ID);
                #if LOG_EVICT_TOTAL_DELAY
                logger.stopTiming("EvictionTotalDelay");
                logger.writeToFile();
                #endif
            }
    } else if (argv[1] == std::string("client_complete")) {
        Client client(ClientConfig::HOST, ClientConfig::PORT);
        client.InitCreateBinaryTree();
        
        // Randomly access the blocks
        size_t accessCount = 30;
        for (size_t i = 0; i < accessCount; ++i) {
            std::cout << "Iteration " << i << std::endl;
            // DEBUG_PRINT("Iteration " << i);
            std::random_device rd;
            std::mt19937 g(rd());
            std::uniform_int_distribution<BlockConfig::TYPE_BLOCK_ID> distribution(0, TreeConfig::REAL_BLOCK_NUM - 1);
            BlockConfig::TYPE_BLOCK_ID blockID = distribution(g);
            
            // The EarlyReshuffle Operation
            PathConfig::TYPE_PATH_ID pathID = client.PositionMap[blockID];
            client.pathComplete.ConvertPID2BIDs(
                pathID, TreeConfig::HEIGHT, client.bucketIDOffsets);
            for (const auto& bucketID : client.bucketIDOffsets) {
                if (client.treeMetaDatas[bucketID].count == BucketConfig::BUCKET_DUMMY_BLOCK_CAPACITY) {
                    // DEBUG_PRINT("Early Reshuffle for bucketID: " << bucketID);
                    client.EarlyReshuffleComplete(bucketID);
                }
            }
            // Read Path
            client.ReadPathComplete(blockID);
            // Eviction Operation
            ++TreeConfig::ACCESS_COUNT_EVICTION_COMPLETE;
            TreeConfig::ACCESS_COUNT_EVICTION_COMPLETE %= TreeConfig::EVICTION_FREQUENCY;
            if (TreeConfig::ACCESS_COUNT_EVICTION_COMPLETE == 0) {
                client.EvictComplete(TreeConfig::EVICTION_PATH_ID);
                TreeConfig::EVICTION_PATH_ID = (TreeConfig::EVICTION_PATH_ID + 1) % TreeConfig::TOTAL_NUM_LEAF_BUCKETS;
            }
            
        } 
    } else if (argv[1] == std::string("client_access_scheme_1")) {
       Client client(ClientConfig::HOST, ClientConfig::PORT);
       client.InitBinaryTreeScheme1();
       BlockConfig::TYPE_BLOCK_ID blockID = 0;
    } else if (argv[1] == std::string("client_read_path_scheme1")) {
        Client client(ClientConfig::HOST, ClientConfig::PORT);
        #if LOG_READ_PATH_TOTAL_DELAY_SCHEME1
        DurationLogger logger(LogConfig::LOG_DIR + LogConfig::LOG_FILE);
        std::string logDescription = "ReadPathTotalDelayScheme1";
        #endif
        BlockConfig::TYPE_BLOCK_ID blockID = 0;
        // Warmup phase
        for (int i = 0; i < 1; i++) {
            std::cout << "Warmup iteration " << i << std::endl;
            client.InitPathScheme1(0);
            client.ReadPathScheme1(blockID);
            client.PositionMap[blockID] = 0;
        }
        size_t accessCount = 10;
        for (size_t i = 0; i < accessCount; ++i) {
            client.InitPathScheme1(0);
            #if LOG_READ_PATH_TOTAL_DELAY_SCHEME1
            logger.startTiming(logDescription);
            #endif
            client.ReadPathScheme1(blockID);
            #if LOG_READ_PATH_TOTAL_DELAY_SCHEME1
            logger.stopTiming(logDescription);
            logger.writeToFile();
            #endif
            client.PositionMap[blockID] = 0;
        }
    } else if (argv[1] == std::string("client_early_reshuffle_scheme1")) {
        Client client(ClientConfig::HOST, ClientConfig::PORT);
        client.InitSingleBucketScheme1(0);
        BucketConfig::TYPE_BUCKET_ID bucketID = 0;
        #if LOG_EARLY_RESHUFFLE_TOTAL_DELAY_SCHEME1
        DurationLogger logger(LogConfig::LOG_DIR + LogConfig::LOG_FILE);
        std::string logDescription = "EarlyReshuffleTotalDelayScheme1";
        #endif
        size_t accessCount = 10;
        for (size_t i = 0; i < accessCount; ++i) {
            #if LOG_EARLY_RESHUFFLE_TOTAL_DELAY_SCHEME1
            logger.startTiming(logDescription);
            #endif
            client.EarlyReshuffleScheme1(bucketID);
            #if LOG_EARLY_RESHUFFLE_TOTAL_DELAY_SCHEME1
            logger.stopTiming(logDescription);
            logger.writeToFile();
            #endif
        }
     } else if (argv[1] == std::string("client_evict_scheme1")) {
        Client client(ClientConfig::HOST, ClientConfig::PORT);
        client.InitEvictPathScheme1(0);
        PathConfig::TYPE_PATH_ID pathID = 0;
        #if LOG_EVICT_TOTAL_DELAY_SCHEME1
        DurationLogger logger(LogConfig::LOG_DIR + LogConfig::LOG_FILE);
        std::string logDescription = "EvictTotalDelayScheme1";
        #endif
        size_t accessCount = 10;
        for (size_t i = 0; i < accessCount; ++i) {
            #if LOG_EVICT_TOTAL_DELAY_SCHEME1
            logger.startTiming(logDescription);
            #endif
            client.EvictScheme1(pathID);
            #if LOG_EVICT_TOTAL_DELAY_SCHEME1
            logger.stopTiming(logDescription);
            logger.writeToFile();
            #endif
        }
     } else if (argv[1] == std::string("test")) {
        // Client client(ClientConfig::HOST, ClientConfig::PORT);
        // client.Test();
        int num_threads = 20;
        int num_items = 64 * 1024;
        for (int i = 1; i <= num_threads; ++i) {
            std::vector<std::pair<ZZ_p, ZZ_p>> ciphertexts(num_items);
            for (int i = 0; i < num_items; ++i) {
                ciphertexts[i].first = conv<ZZ_p>(100);
                ciphertexts[i].second = conv<ZZ_p>(200);
            }
            ElGamal_parallel_ntl elgamal(i, BlockConfig::BLOCK_SIZE);
            auto start = std::chrono::high_resolution_clock::now();
            elgamal.ParallelRerandomize(ciphertexts);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            std::cout << "ParallelRerandomize took: " << duration << " microseconds with " << i << " threads." << std::endl;
            // wait one second
            // std::this_thread::sleep_for(std::chrono::seconds(1));

        }
     }
    return 0;
}