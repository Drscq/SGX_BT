#ifndef CONFIG_H
#define CONFIG_H
// LOG headers
#include <filesystem>
#include <fstream>
#include <thread>
// NTL headers
#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <vector>
#include <random>
using namespace NTL;

// OpenSSL headers
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <memory>

#include <algorithm>
#include <cassert>  
#include <bitset>   
#include <cstdint>
#include <unordered_map>  
#include <pthread.h> // For setting CPU affinity 

// Macros
#define USE_PERM_TO_GEN_INTERMEDIATE_PERM 0
#define USE_ASSERT 0
#define USE_COUT 0
#define USE_PRINT_TARGET_BLOCK 0
#define READ_PATH_SIMULATION 0
#define LOG_READ_PATH_TOTAL_DELAY 0
#define LOG_BREAKDOWN_COST 1
#define LOG_BREAKDOWN_COST_READ_PATH_FURTHER 0
#define EARLY_RESHUFFLE_SIMULATION 0
#define LOG_EARLY_RESHUFFLE_TOTAL_DELAY 0
#define EVICT_SIMULATION 0
#define LOG_EVICT_TOTAL_DELAY 0
#define LOG_EVICT_BREAKDOWN_COST_CLIENT 0
#define LOG_EVICT_BREAKDOWN_COST_SERVER 0
#define LOG_EVICT_BREAKDOWN_COST_THIRD_PARTY 0
#define LOG_EVICT_BREAKDOWN_COST_SERVER_FURTHER_1 0

#define PRINT_READ_PATH_DELAY_FOR_SCHEME1 0
#define ASSERT_CORRECTNESS_TARGET_BLOCK_SCHEME1 0
#define PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1 0
#define LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT 0
#define  LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_CLIENT_BLOCKS_STASH 0
#define PRINT_READ_PATH_ASSERT_SCHEME1 0
#define PRINT_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_SERVER 0
#define LOG_READ_PATH_BREAKDOWN_COST_FOR_SCHEME1_SERVER 0

#define PRINT_EARLY_RESHUFFLE_DELAY_FOR_SCHEME1 0
#define PRINT_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_SERVER 0
#define LOG_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_SERVER 0
#define PRINT_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_THIRD_PARTY 0
#define LOG_EARLY_RESHUFFLE_BREAKDOWN_COST_FOR_SCHEME1_THIRD_PARTY 0

#define PRINT_EVICT_DELAY_FOR_SCHEME1 0
#define PRINT_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER 0
#define LOG_EVICT_BREAKDOWN_COST_FOR_SCHEME1_SERVER 0
#define PRINT_EVICT_BREAKDOWN_COST_FOR_SCHEME1_THIRD_PARTY 0
#define LOG_EVICT_BREAKDOWN_COST_FOR_SCHEME1_THIRD_PARTY 0


#define LOG_READ_PATH_TOTAL_DELAY_SCHEME1 0
#define LOG_EARLY_RESHUFFLE_TOTAL_DELAY_SCHEME1 0
#define LOG_EVICT_TOTAL_DELAY_SCHEME1 0


#include <openssl/aes.h>
// First scheme MACROS
#define LOG_AES_CTR_ENCRYPTION_DECRIPTION 0
namespace SizeConfig {
    typedef uint64_t TYPE_UNSIGNED_SIZE;
}

// Macros for the ElGamal_parallel_ntl
#define MULTI_THREAD_SWITCH 0
#define MULTI_THREAD_RERANDOMIZE_SWITCH 0
#define MULTI_THREAD_DESERIALIZE_SWITCH 0
#define MULTI_THREAD_BASIC_THREAD_POOL 0
#define MULTI_THREAD_FUTURE_VERSION 0
#define MULTI_THREAD_THREAD_VERSION 0
#define PRINT_BREAKDOWN_COST_RERANDOMIZE 0

#define USE_INVERSE_PERM_TO_GEN_INTERMEDIATE_PERM 0

#define BLOCK_SIZE_IS_64KB 1
namespace ThreadConfig {
    typedef unsigned int TYPE_THREAD_NUM;
    inline const TYPE_THREAD_NUM MAX_NUM_THREADS = std::thread::hardware_concurrency();
    inline std::vector<pthread_t> THREADS(MAX_NUM_THREADS);
}

namespace FileConfig
{
    inline std::ifstream bucketFileLoad;

    inline std::ifstream fileReadScheme1;

    inline std::fstream fileReadWriteScheme1;

    inline std::ofstream fileWriteScheme1;
} // namespace FileConfig


namespace ServerConfig {
    #if  MULTI_THREAD_RERANDOMIZE_SWITCH
        #if BLOCK_SIZE_IS_64KB
        inline size_t num_threads = 32;
        #endif
    #else
        inline size_t num_threads = 1;
    #endif
    // communication related configurations
    typedef uint16_t TYPE_PORT_NUM;
    typedef int TYPE_CMD;
    inline int PORT = 2000;
    // commannds for socket communication
    constexpr TYPE_CMD CMD_CREATE_DB = 12;
    constexpr TYPE_CMD CMD_READ_PATH = 13;
    constexpr TYPE_CMD CMD_SUCCESS = 14;
    constexpr TYPE_CMD CMD_EARLY_RESHUFFLE_INIT = 15;
    constexpr TYPE_CMD CMD_EARLY_RESHUFFLE = 16;
    constexpr TYPE_CMD CMD_EARLY_RESHUFFLE_CLIENT_THRID_PARTY = 17;
    constexpr TYPE_CMD CMD_EARLY_RESHUFFLE_SERVER_THRID_PARTY = 18;
    constexpr TYPE_CMD CMD_EVICT_SERVER_INIT = 19;
    constexpr TYPE_CMD CMD_EVICT_THIRD_PARTY = 20;
    constexpr TYPE_CMD CMD_EVICT_SERVER_OPERATION = 21;
    constexpr TYPE_CMD CMD_EVICT_COMMUNICATION_ROOT_BUCKET = 22;
    constexpr TYPE_CMD CMD_EVICT_FROM_SERVER_TO_THIRD_PARTY = 23;
    constexpr TYPE_CMD CMD_COMPLETE_CREATE_BINARY_TREE = 24;
    constexpr TYPE_CMD CMD_COMPLETE_READ_PATH = 25;
    constexpr TYPE_CMD CMD_TEST = 26;
    constexpr TYPE_CMD CMD_COMPLETE_EARLY_RESHUFFLE = 27;
    constexpr TYPE_CMD CMD_COMPLETE_EARLY_RESHUFFLE_CLIENT_THIRD_PARTY = 28;
    constexpr TYPE_CMD CMD_COMPLETE_EARLY_RESHUFFLE_SERVER_THIRD_PARTY = 29;
    constexpr TYPE_CMD CMD_COMPLETE_EVICT_CLIENT_TO_SERVER = 30;
    constexpr TYPE_CMD CMD_COMPLETE_EVICT_CLIENT_TO_THIRD_PARTY = 31;
    constexpr TYPE_CMD CMD_COMPLETE_EVICT_SERVER_TO_THIRD_PARTY = 32;
    // for the first scheme
    constexpr TYPE_CMD CMD_INIT_BINARY_TREE_SCHEME1_CLIENT_TO_SERVER = 33;
    constexpr TYPE_CMD CMD_READ_PATH_SCHEME1_CLIENT_TO_SERVER = 34;
    constexpr TYPE_CMD CMD_EARLY_RESHUFFLE_SCHEME1_CLIENT_TO_SERVER = 35;
    constexpr TYPE_CMD CMD_EARLY_RESHUFFLE_SCHEME1_SERVER_TO_THIRD_PARTY = 36;
    // for the eviction
    constexpr TYPE_CMD CMD_EVICT_SCHEME1_CLIENT_TO_SERVER = 37;
    constexpr TYPE_CMD CMD_EVICT_SCHEME1_SERVER_TO_THIRD_PARTY = 38;
    constexpr TYPE_CMD CMD_STASH_SCHEME1_CLIENT_TO_THIRD_PARTY = 39;
    // for the simulation
    constexpr TYPE_CMD CMD_INIT_PATH_SCHEME1_CLIENT_TO_SERVER = 40;
    constexpr TYPE_CMD CMD_INIT_SINGLE_BUCKET_SCHEME1_CLIENT_TO_SERVER = 41;
    constexpr TYPE_CMD CMD_INIT_EVICT_PATH_SCHEME1_CLIENT_TO_SERVER = 42;
    // For the test
    constexpr TYPE_CMD CMD_TEST_CLIENT_TO_SERVER = 43;
}

// namespace ThirdPartyConfig {
    
// }

namespace PathConfig {
    typedef long long TYPE_PATH_ID;
    typedef size_t TYPE_PATH_SIZE;
    inline constexpr TYPE_PATH_SIZE HEIGHT = 4;
    inline TYPE_PATH_SIZE REAL_BLOCK_NUM = 2 * HEIGHT;
    inline TYPE_PATH_ID GenPathID() {
        std::random_device rd;
        std::mt19937 g(rd());
        std::uniform_int_distribution<TYPE_PATH_ID> distribution(0, std::pow(2, HEIGHT - 1) - 1);
        return distribution(g);
    }
}


// Block headers
namespace BlockConfig {
    typedef long long TYPE_BLOCK_ID;
    typedef unsigned long long TYPE_BLOCK_SIZE;
    inline TYPE_BLOCK_SIZE BLOCK_SIZE = 1024;
}

namespace AESConfig {
    inline unsigned char key[AES_BLOCK_SIZE] = {
        0x2b, 0x7e, 0x15, 0x16,
        0x28, 0xae, 0xd2, 0xa6,
        0xab, 0xf7, 0x15, 0x88,
        0x09, 0xcf, 0x4f, 0x3c
    };
    inline size_t CHUNK_SIZE_BLOCK_XOR = BlockConfig::BLOCK_SIZE / sizeof(uint64_t);
}

namespace ClientConfig {
    #if MULTI_THREAD_SWITCH
        inline size_t num_threads = 8;
    #else
        inline size_t num_threads = 1;
    #endif
    typedef std::string TYPE_HOST;
    typedef int TYPE_PORT;
    // typedef unsigned long long TYPE_CHAR_SIZE;
    typedef size_t TYPE_CHAR_SIZE;
    inline TYPE_HOST HOST = "198.82.162.130";
    inline TYPE_PORT PORT = 2000;
    inline PathConfig::TYPE_PATH_ID PATH_ID = 0;
    inline BlockConfig::TYPE_BLOCK_ID TARGET_BLOCK_ID = 0;
    inline void SerializeBlockStash(const BlockConfig::TYPE_BLOCK_ID& block_id, 
                            const PathConfig::TYPE_PATH_ID& path_id,
                            const std::vector<char>& block_data,
                            std::vector<char>& buffer) {
        auto it = buffer.data();
        std::memcpy(it, &block_id, sizeof(BlockConfig::TYPE_BLOCK_ID));
        it += sizeof(BlockConfig::TYPE_BLOCK_ID);
        std::memcpy(it, &path_id, sizeof(PathConfig::TYPE_PATH_ID));
        it += sizeof(PathConfig::TYPE_PATH_ID);
        std::memcpy(it, block_data.data(), block_data.size());    
    }
    inline void SerializeBlockStash(const BlockConfig::TYPE_BLOCK_ID& block_id,
                                    const PathConfig::TYPE_PATH_ID& path_id,
                                    void* block_data,
                                    const SizeConfig::TYPE_UNSIGNED_SIZE& block_size,
                                    void* buffer) {
        auto it = reinterpret_cast<char*>(buffer);
        std::memcpy(it, &block_id, sizeof(BlockConfig::TYPE_BLOCK_ID)); 
        it += sizeof(BlockConfig::TYPE_BLOCK_ID);
        std::memcpy(it, &path_id, sizeof(PathConfig::TYPE_PATH_ID));
        it += sizeof(PathConfig::TYPE_PATH_ID);
        std::memcpy(it, reinterpret_cast<char*>(block_data), block_size);
    }
}
namespace ThirdPartyConfig {
    #if MULTI_THREAD_RERANDOMIZE_SWITCH
        inline size_t num_threads = 4;
    #else
        inline size_t num_threads = 1;
    #endif
    typedef int TYPE_PORT;
    inline TYPE_PORT PORT_THIRD_PARTY = 5000;
    inline ClientConfig::TYPE_HOST HOST = "10.0.0.2";
}
// Bucket headers
#include <unordered_set>
#include <cstddef>
#include <array>
#include <algorithm>    
namespace BucketConfig {
    typedef uint_fast16_t TYPE_SMALL_INDEX_U;
    typedef int_fast16_t TYPE__SMALL_INDEX;
    /// directory to store the bucket data
    inline std::string DATADIR = "data/";
    inline std::string BUCKETPREFIX = "bucket_";
    typedef long unsigned int TYPE_BUCKET_SIZE;
    // typedef unsigned int TYPE_SLOT_ID;
    typedef unsigned short TYPE_SLOT_ID;
    typedef short TYPE_SLOT_ID_S;
    typedef size_t TYPE_BUCKET_ID;
    typedef size_t TYPE_THREAD_NUM;
    const TYPE_BUCKET_SIZE BUCKET_REAL_BLOCK_CAPACITY = 30;
    const TYPE_BUCKET_SIZE BUCKET_DUMMY_BLOCK_CAPACITY = 43;
    const TYPE_BUCKET_SIZE BUCKET_SIZE = BUCKET_REAL_BLOCK_CAPACITY + BUCKET_DUMMY_BLOCK_CAPACITY;
    inline void ApplyPerm(std::vector<std::vector<std::pair<ZZ, ZZ>>>& bucketCiphertexts, const std::vector<TYPE_SLOT_ID>& perm) {
            TYPE_BUCKET_SIZE n = bucketCiphertexts.size();
            std::vector<bool> visited(n, false);
            for (TYPE_BUCKET_SIZE i = 0; i < n; i++) {
                if (visited[i]) continue;
                TYPE_BUCKET_SIZE current = i;
                std::vector<std::pair<ZZ, ZZ>> currentCiphertexts = std::move(bucketCiphertexts[current]);
                // Follow the cycle of the permutation
                while (!visited[current]) {
                    visited[current] = true;
                    TYPE_BUCKET_SIZE next = perm[current];
                    if (visited[next]) {
                        // Place the originally held element in its final position
                        bucketCiphertexts[current] = std::move(currentCiphertexts);
                        break;
                    }
                    // Move the element at 'next' to 'current'
                    bucketCiphertexts[current] = std::move(bucketCiphertexts[next]);
                    current = next;
                }
                
            }
        }
    // inline void ApplyPerm(std::vector<std::vector<std::pair<ZZ_p, ZZ_p>>>& bucketCipherTexts, const std::vector<TYPE_SLOT_ID>& perm) {
    //     TYPE_BUCKET_SIZE n = bucketCipherTexts.size();
    //     std::vector<bool> visited(n, false);
    //     for (TYPE_BUCKET_SIZE i = 0; i < n; i++) {
    //         if (visited[i]) continue;
    //         TYPE_BUCKET_SIZE current = i;
    //         std::vector<std::pair<ZZ_p, ZZ_p>> currentCiphertexts = std::move(bucketCipherTexts[current]);
    //         // Follow the cycle of the permutation
    //         while (!visited[current]) {
    //             visited[current] = true;
    //             TYPE_BUCKET_SIZE next = perm[current];
    //             if (visited[next]) {
    //                 // Place the originally held element in its final position
    //                 bucketCipherTexts[current] = std::move(currentCiphertexts);
    //                 break;
    //             }
    //             // Move the element at 'next' to 'current'
    //             bucketCipherTexts[current] = std::move(bucketCipherTexts[next]);
    //             current = next;
    //         }
    //     }
    // }
    inline void ApplyPermInPlace(std::vector<std::vector<std::pair<ZZ_p, ZZ_p>>>& bucketCipherTexts, std::vector<TYPE_SLOT_ID>& perm) {
    // TYPE_BUCKET_SIZE n = bucketCipherTexts.size();
    #if USE_ASSERT
        assert(bucketCipherTexts.size() == perm.size() && "The size of the bucketCipherTexts and perm should be the same");
    #endif 
    
        for (TYPE_BUCKET_SIZE i = 0; i < bucketCipherTexts.size(); ++i) {
            while (perm[i] != i) {
                TYPE_SLOT_ID swap_idx = perm[i];
                
                // Swap the elements in bucketCipherTexts
                std::swap(bucketCipherTexts[i], bucketCipherTexts[swap_idx]);
                
                // Swap the indices in the perm array
                std::swap(perm[i], perm[swap_idx]);
            }
        }
    }


    inline void ApplyPerm(std::vector<std::vector<std::pair<ZZ_p, ZZ_p>>>& bucketCipherTexts, const std::vector<TYPE_SLOT_ID>& perm, std::vector<std::vector<std::pair<ZZ_p, ZZ_p>>>& bucketCipherTextsPermuted) {
        #if USE_ASSERT
        assert(bucketCipherTexts.size() == bucketCipherTextsPermuted.size());
        assert(perm.size() == bucketCipherTexts.size());
        #endif
        for (TYPE_BUCKET_SIZE i = 0; i < bucketCipherTexts.size(); i++) {
            bucketCipherTextsPermuted[i] = std::move(bucketCipherTexts[perm[i]]);
        }
    }
    inline ClientConfig::TYPE_CHAR_SIZE META_DATA_SIZE = sizeof(size_t) + BUCKET_SIZE * (sizeof(bool))
                                                        + sizeof(size_t) + BUCKET_REAL_BLOCK_CAPACITY * sizeof(BlockConfig::TYPE_BLOCK_ID) + 
                                                        BUCKET_REAL_BLOCK_CAPACITY * sizeof(BlockConfig::TYPE_BLOCK_ID) 
                                                        + BUCKET_SIZE * sizeof(PathConfig::TYPE_PATH_ID) 
                                                        + BUCKET_SIZE * sizeof(TYPE_SLOT_ID)
                                                        + sizeof(TYPE_SLOT_ID) * 2; 

                                                    
                                                         
                                                            
    struct META_DATA {
        size_t count; // Used to log the access time for the bucket
        bool valids[BUCKET_SIZE]; // Indicates whether each of the slots in the bucket is valid or not
        std::unordered_set<BlockConfig::TYPE_BLOCK_ID> block_ids; // Used to store the block ids in the bucket
        BlockConfig::TYPE_BLOCK_ID addrs[BUCKET_REAL_BLOCK_CAPACITY]; // Contains addresses or identifiers of real blocks
        PathConfig::TYPE_PATH_ID leaves[BUCKET_REAL_BLOCK_CAPACITY]; // Contains the leaf ids of the blocks
        TYPE_SLOT_ID offsets[BUCKET_SIZE]; // Contains the offsets of the blocks
        TYPE_SLOT_ID offsetsGuidence[BUCKET_SIZE]; // Contains the offsets of the blocks
        TYPE_SLOT_ID offsetsInv[BUCKET_SIZE]; // Contains the offsets of the blocks
        TYPE_SLOT_ID nextDummyIndex; // Used to keep track of the next available dummy block index
        TYPE_SLOT_ID nextRealIndex; // Used to keep track of the next available real block index
        size_t block_ids_size;
        BlockConfig::TYPE_BLOCK_ID id_serialized;
        // Constructor to initialize the data members of the META_DATA struct
        META_DATA() : count(0), nextDummyIndex(0), nextRealIndex(0) {
            for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_SIZE; i++) {
                valids[i] = true;
            }
            for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_REAL_BLOCK_CAPACITY; i++) {
                addrs[i] = -1;
                leaves[i] = -1;
            }
            GenPRP();
        }
        void Initialize() {
            count = 0;
            nextDummyIndex = 0;
            nextRealIndex = 0;
            for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_SIZE; i++) {
                valids[i] = true;
            }
            for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_REAL_BLOCK_CAPACITY; i++) {
                addrs[i] = -1;
                leaves[i] = -1;
            }
            GenPRP();
            // erase the block_ids
            block_ids.clear();
        }
        void SimpleReset() {
            count = 0;
            for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_SIZE; i++) {
                if (!valids[i]) {
                    valids[i] = true;
                }
            }
            shiftNextDummyIndex2End();
        }
        void shiftNextDummyIndex2End() {
            nextDummyIndex = BUCKET_SIZE;
        }
        // Function to convert the C-style array 'offsets' to a vector
        std::vector<TYPE_SLOT_ID> Offsets2Vector() {
            std::vector<TYPE_SLOT_ID> vec(offsets, offsets + BUCKET_SIZE);
            return vec;
        }
        static std::vector<TYPE_SLOT_ID> GenRandomPerm(TYPE_BUCKET_SIZE n) {
            std::vector<TYPE_SLOT_ID> perm(n);
            std::iota(perm.begin(), perm.end(), 0); // Fill the perm vector with 0, 1, 2, ..., n-1
            std::mt19937 g(10);
            std::shuffle(perm.begin(), perm.end(), g);
            return perm;
        }
        static std::vector<TYPE_SLOT_ID> InvertPerm(std::vector<TYPE_SLOT_ID>& perm) {
            std::vector<TYPE_SLOT_ID> inv(perm.size());
            for (TYPE_SLOT_ID i = 0; i < perm.size(); i++) {
                inv[perm[i]] = i;  // Correctly map the index
            }
            return inv;
        }
        void InvertPerm(TYPE_SLOT_ID perm[], TYPE_SLOT_ID inv[], TYPE_BUCKET_ID size) {
            for (TYPE_SLOT_ID i = 0; i < size; ++i) {
                inv[perm[i]] = i;  // Correctly map the index
            }
        }
        static void DividePerm(const std::vector<TYPE_SLOT_ID>& perm, std::vector<TYPE_SLOT_ID>& perm1, std::vector<TYPE_SLOT_ID>& perm2) {
            // Step 1: Generate the first random permutation (perm1)
            perm1 = GenRandomPerm(perm.size());
            // Step 2: Invert the first random permutation
            std::vector<TYPE_SLOT_ID> inv1 = InvertPerm(perm1);
            // Step 3: Generate perm2 as the inverse of the perm1 applied to the perm
            perm2.resize(perm.size());
            std::transform(perm.begin(), perm.end(), perm2.begin(), [&](TYPE_SLOT_ID val) {
                return inv1[val];
            });
        }
        std::vector<TYPE_SLOT_ID> combine_permutations(const std::vector<TYPE_SLOT_ID>& perm1, const std::vector<TYPE_SLOT_ID>& perm2) {
            std::vector<TYPE_SLOT_ID> combined(perm1.size());
            for (TYPE_SLOT_ID i = 0; i < perm1.size(); ++i) {
                    combined[i] = perm1[perm2[i]];
                }
                return combined;
        }
        void ReShuffleOffsets() {
            TYPE_SLOT_ID temp[BUCKET_SIZE];
            TYPE_SLOT_ID updated[BUCKET_SIZE];
            for (TYPE_SLOT_ID i = 0; i < BUCKET_SIZE; i++) {
                temp[i] = offsets[i];
                updated[i] = i;
            }
            std::mt19937 g(20);
            std::shuffle(updated, updated + BUCKET_SIZE, g);
            for (TYPE_SLOT_ID i = 0; i < BUCKET_SIZE; i++) {
                offsets[temp[i]] = updated[i];
            }
        }
        void GenPRP(int seed = 32) {
            for (TYPE_SLOT_ID i = 0; i < BUCKET_SIZE; i++) {
                offsets[i] = i;
            }
            // Seed with a real random value, if available
            std::mt19937 g(seed);
            std::shuffle(offsets, offsets + BUCKET_SIZE, g);
        }
        void LogAccess() {
            ++count;
        }
        void AddRealBlock(BlockConfig::TYPE_BLOCK_ID block_id, PathConfig::TYPE_PATH_ID leaf_id) {
            if (nextRealIndex >= BUCKET_REAL_BLOCK_CAPACITY) {
                std::cerr << "Error: Real block capacity exceeded" << std::endl;
                return;
            }
            addrs[nextRealIndex] = block_id;
            leaves[nextRealIndex] = leaf_id;
            block_ids.insert(block_id);
            nextRealIndex++;
            nextDummyIndex = nextRealIndex;
        }
        void AddDummyBlock() {
            ++nextDummyIndex;
        }
        void AddRealBlockPure(BlockConfig::TYPE_BLOCK_ID block_id, PathConfig::TYPE_PATH_ID leaf_id) {
            if (nextRealIndex >= BUCKET_REAL_BLOCK_CAPACITY) {
                std::cerr << "Error: Real block capacity exceeded" << std::endl;
                return;
            }
            addrs[nextRealIndex] = block_id;
            leaves[nextRealIndex] = leaf_id;
            block_ids.insert(block_id);
            nextRealIndex++;
        }
        void DenoteDeletedRealBlock(TYPE_SLOT_ID idx) {
            block_ids.erase(addrs[idx]);
            addrs[idx] = -1;
            leaves[idx] = -1;
            // nextRealIndex--;
        }
        void OrganizeRealBlocksIDs() {
            TYPE_SLOT_ID_S start = 0;
            // TYPE_SLOT_ID end = nextRealIndex - 1 < 0 ? 0 : nextRealIndex - 1;
            TYPE_SLOT_ID_S end = BucketConfig::BUCKET_REAL_BLOCK_CAPACITY - 1;
            while (start <= end) {
                while (start <= end && addrs[start] != -1) {
                    start++;
                }
                while (start <= end && addrs[end] == -1) {
                    end--;
                }
                if (start < end) {
                    std::swap(addrs[start], addrs[end]); 
                    std::swap(leaves[start], leaves[end]);
                    std::swap(offsets[start], offsets[end]);
                }
            }
            // std::cout << "The nextRealIndex: " << nextRealIndex << std::endl;
            nextRealIndex = start;
        }
        bool ContainsBlockID(BlockConfig::TYPE_BLOCK_ID block_id) {
            return block_ids.find(block_id) != block_ids.end();
        }
        void DeleteBlockID(BlockConfig::TYPE_BLOCK_ID block_id) {
            block_ids.erase(block_id);
        }
        // Serialize the META_DATA structure into a vector<char>
        void Serialize(std::vector<char>& data) {
            assert(data.size() == META_DATA_SIZE);
            auto it = data.data();
            std::memcpy(data.data(), &count, sizeof(count));
            it += sizeof(count);
            // Serialize the valids array
            for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_SIZE; i++) {
                std::memcpy(it, &valids[i], sizeof(valids[i]));
                it += sizeof(valids[i]);
            }
            // Serialize the block_ids
            block_ids_size = block_ids.size();
            std::memcpy(it, &block_ids_size, sizeof(block_ids_size));
            it += sizeof(block_ids_size);
            for (auto id : block_ids) {
                std::memcpy(it, &id, sizeof(id));
                it += sizeof(id);
            }
            // Serialize the addrs array
            for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_REAL_BLOCK_CAPACITY; i++) {
                std::memcpy(it, &addrs[i], sizeof(addrs[i]));
                it += sizeof(addrs[i]);
            }
            // Serialize the leaves array
            for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_REAL_BLOCK_CAPACITY; i++) {
                std::memcpy(it, &leaves[i], sizeof(leaves[i]));
                it += sizeof(leaves[i]);
            }
            // Serialize the offsets array
            for (TYPE_SLOT_ID i = 0; i < BUCKET_SIZE; i++) {
                std::memcpy(it, &offsets[i], sizeof(offsets[i]));
                it += sizeof(offsets[i]);
            }
            // Serialize the nextDummyIndex
            std::memcpy(it, &nextDummyIndex, sizeof(nextDummyIndex));
            it += sizeof(nextDummyIndex);
            // Serialize the nextRealIndex
            std::memcpy(it, &nextRealIndex, sizeof(nextRealIndex));
        }
        void Serialize(void* raw_data) {
        assert(raw_data != nullptr);
        auto it = static_cast<char*>(raw_data);
        
        // Serialize count
        std::memcpy(it, &count, sizeof(count));
        it += sizeof(count);
        
        // Serialize the valids array
        for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_SIZE; i++) {
            std::memcpy(it, &valids[i], sizeof(valids[i]));
            it += sizeof(valids[i]);
        }
        
        // Serialize the block_ids
        block_ids_size = block_ids.size();
        std::memcpy(it, &block_ids_size, sizeof(block_ids_size));
        it += sizeof(block_ids_size);
        for (auto id : block_ids) {
            std::memcpy(it, &id, sizeof(id));
            it += sizeof(id);
        }
        
        // Serialize the addrs array
        for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_REAL_BLOCK_CAPACITY; i++) {
            std::memcpy(it, &addrs[i], sizeof(addrs[i]));
            it += sizeof(addrs[i]);
        }
        
        // Serialize the leaves array
        for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_REAL_BLOCK_CAPACITY; i++) {
            std::memcpy(it, &leaves[i], sizeof(leaves[i]));
            it += sizeof(leaves[i]);
        }
        
        // Serialize the offsets array
        for (TYPE_SLOT_ID i = 0; i < BUCKET_SIZE; i++) {
            std::memcpy(it, &offsets[i], sizeof(offsets[i]));
            it += sizeof(offsets[i]);
        }
        
        // Serialize the nextDummyIndex
        std::memcpy(it, &nextDummyIndex, sizeof(nextDummyIndex));
        it += sizeof(nextDummyIndex);
        
        // Serialize the nextRealIndex
        std::memcpy(it, &nextRealIndex, sizeof(nextRealIndex));
    }

        // Deserialize the vector<char> into the META_DATA structure
        void Deserialize(const std::vector<char>& data) {
            assert(data.size() == META_DATA_SIZE);
            auto it = data.data();
            // Deserialize the count
            std::memcpy(&count, it, sizeof(count));
            it += sizeof(count);
            // Deserialize the valids array
            for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_SIZE; i++) {
                std::memcpy(&valids[i], it, sizeof(valids[i]));
                it += sizeof(valids[i]);
            }
            // Deserialize the block_ids
            std::memcpy(&block_ids_size, it, sizeof(block_ids_size));
            it += sizeof(block_ids_size);
            for (size_t i = 0; i < block_ids_size; i++) {
                std::memcpy(&id_serialized, it, sizeof(id_serialized));
                block_ids.insert(id_serialized);
                it += sizeof(id_serialized);
            }
            // Deserialize the addrs array
            for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_REAL_BLOCK_CAPACITY; i++) {
                std::memcpy(&addrs[i], it, sizeof(addrs[i]));
                it += sizeof(addrs[i]);
            }
            // Deserialize the leaves array
            for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_REAL_BLOCK_CAPACITY; i++) {
                std::memcpy(&leaves[i], it, sizeof(leaves[i]));
                it += sizeof(leaves[i]);
            }
            // Deserialize the offsets array
            for (TYPE_SLOT_ID i = 0; i < BUCKET_SIZE; i++) {
                std::memcpy(&offsets[i], it, sizeof(offsets[i]));
                it += sizeof(offsets[i]);
            }
            // Deserialize the nextDummyIndex
            std::memcpy(&nextDummyIndex, it, sizeof(nextDummyIndex));
            it += sizeof(nextDummyIndex);
            // Deserialize the nextRealIndex
            std::memcpy(&nextRealIndex, it, sizeof(nextRealIndex));
        }
        
        void Deserialize(const void* data) {
        assert(data != nullptr);
        auto it = static_cast<const char*>(data);
        // Deserialize the count
        std::memcpy(&count, it, sizeof(count));
        it += sizeof(count);
        // Deserialize the valids array
        for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_SIZE; i++) {
            std::memcpy(&valids[i], it, sizeof(valids[i]));
            it += sizeof(valids[i]);
        }
        // Deserialize the block_ids
        std::memcpy(&block_ids_size, it, sizeof(block_ids_size));
        it += sizeof(block_ids_size);
        for (size_t i = 0; i < block_ids_size; i++) {
            std::memcpy(&id_serialized, it, sizeof(id_serialized));
            block_ids.insert(id_serialized);
            it += sizeof(id_serialized);
        }
        // Deserialize the addrs array
        for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_REAL_BLOCK_CAPACITY; i++) {
            std::memcpy(&addrs[i], it, sizeof(addrs[i]));
            it += sizeof(addrs[i]);
        }
        // Deserialize the leaves array
        for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_REAL_BLOCK_CAPACITY; i++) {
            std::memcpy(&leaves[i], it, sizeof(leaves[i]));
            it += sizeof(leaves[i]);
        }
        // Deserialize the offsets array
        for (TYPE_SLOT_ID i = 0; i < BUCKET_SIZE; i++) {
            std::memcpy(&offsets[i], it, sizeof(offsets[i]));
            it += sizeof(offsets[i]);
        }
        // Deserialize the nextDummyIndex
        std::memcpy(&nextDummyIndex, it, sizeof(nextDummyIndex));
        it += sizeof(nextDummyIndex);
        // Deserialize the nextRealIndex
        std::memcpy(&nextRealIndex, it, sizeof(nextRealIndex));
    }

        void print() {
            std::cout << "count: " << count << std::endl;
            std::cout << "valids: ";
            for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_SIZE; i++) {
                std::cout << valids[i] << " ";
            }
            std::cout << std::endl;
            std::cout << "block_ids: ";
            for (auto it = block_ids.begin(); it != block_ids.end(); it++) {
                std::cout << *it << " ";
            }
            std::cout << std::endl;
            std::cout << "addrs: ";
            for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_REAL_BLOCK_CAPACITY; i++) {
                std::cout << addrs[i] << " ";
            }
            std::cout << std::endl;
            std::cout << "leaves: ";
            for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_REAL_BLOCK_CAPACITY; i++) {
                std::cout << leaves[i] << " ";
            }
            std::cout << std::endl;
            std::cout << "offsets: ";
            for (TYPE_SLOT_ID i = 0; i < BUCKET_SIZE; i++) {
                std::cout << offsets[i] << " ";
            }
            std::cout << std::endl;
            std::cout << "nextDummyIndex: " << nextDummyIndex << std::endl;
            std::cout << "nextRealIndex: " << nextRealIndex << std::endl;
        }
        void reset(std::vector<TYPE_SLOT_ID>& originalPerms) {
            #if USE_INVERSE_PERM_TO_GEN_INTERMEDIATE_PERM
                InvertPerm(offsets, offsetsInv, BUCKET_SIZE);
                GenPRP(33);
                // generate offsetsGuidence as the inverse of offsets applied to current offsets
                
                std::transform(offsetsInv, offsetsInv + BUCKET_SIZE, offsetsGuidence, [&](TYPE_SLOT_ID val) {
                    return offsets[val];
                });
                originalPerms.clear();
                originalPerms.assign(offsetsGuidence, offsetsGuidence + BUCKET_SIZE);
            #else
                #if USE_ASSERT
                assert(originalPerms.size() == BUCKET_SIZE);
                #endif 
                // copy the content of offsets to offsetsGuidence
                std::copy(offsets, offsets + BUCKET_SIZE, offsetsGuidence);
                GenPRP(33);
                for (TYPE_SLOT_ID i = 0; i < BUCKET_SIZE; i++) {
                    originalPerms[offsetsGuidence[i]] = offsets[i];
                }
            #endif
            #if USE_ASSERT
            assert(nextRealIndex >= 0);
            #endif 
            shiftNextDummyIndex2End();
            count = 0;
            for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_SIZE; i++) {
                if (valids[i] == false) {
                    valids[i] = true;
                }
            }
        }
        void ResetTriplets() {
            shiftNextDummyIndex2End();
            count = 0;
            for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_SIZE; i++) {
                if (valids[i] == false) {
                    valids[i] = true;
                }
            }
        }
        void ResetEarlyReshuffle1() {
            count = 0;
            for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_SIZE; i++) {
                if (valids[i] == false) {
                    valids[i] = true;
                }
            }
            ReShuffleOffsets();
            shiftNextDummyIndex2End();
            
        }
        void ResetEviction1() {
            count = 0;
            for (TYPE_BUCKET_SIZE i = 0; i < BUCKET_SIZE; i++) {
                if (valids[i] == false) {
                    valids[i] = true;
                }
            }
            ReShuffleOffsets();
            for(TYPE_BUCKET_SIZE i = 0; i < nextRealIndex; ++i) {
                addrs[i] = -1;
                leaves[i] = -1;
            }
            nextRealIndex = 0;
            nextDummyIndex = 0;
            block_ids.clear();
        }
    };
    inline void GenIntermediatePerm(std::vector<TYPE_SLOT_ID>& permCur,
                                    std::vector<TYPE_SLOT_ID>& permTarget,
                                    std::vector<TYPE_SLOT_ID>& intermediatePerm) {
        #if USE_ASSERT
        assert(permCur.size() == permTarget.size() && permCur.size() == intermediatePerm.size());
        #endif 
        #if USE_INVERSE_PERM_TO_GEN_INTERMEDIATE_PERM
            std::vector<TYPE_SLOT_ID> permCurInv = META_DATA::InvertPerm(permCur);
            std::transform(permCurInv.begin(), permCurInv.end(), intermediatePerm.begin(), [&](TYPE_SLOT_ID val) {
                return permTarget[val];
            });
        #else 
            for (TYPE_SLOT_ID i = 0; i < permCur.size(); i++) {
                intermediatePerm[permCur[i]] = permTarget[i];
            }
        #endif
    }
}

namespace LogConfig {
    // inline std::string LOG_DIR = "../logs/";
    inline std::string LOG_DIR = "logs/";
    // check if the directory exists otherwise create it
    inline void CheckLogDir() {
        if (!std::filesystem::exists(LogConfig::LOG_DIR)) {
            std::filesystem::create_directory(LogConfig::LOG_DIR);
        }
    }
    inline std::string LOG_FILE = "Height_" + std::to_string(PathConfig::HEIGHT) 
                                  + "_BlockSize_" + std::to_string(BlockConfig::BLOCK_SIZE) + ".txt";
}
namespace TreeConfig {
    inline constexpr PathConfig::TYPE_PATH_SIZE HEIGHT = PathConfig::HEIGHT;
    typedef unsigned long long TYPE_BUCKET_NUM;
    inline TYPE_BUCKET_NUM TOTAL_NUM_BUCKETS = (1 << HEIGHT) - 1;
    inline TYPE_BUCKET_NUM TOTAL_NUM_NON_LEAF_BUCKETS = (1 << (HEIGHT - 1)) - 1;
    inline TYPE_BUCKET_NUM TOTAL_NUM_LEAF_BUCKETS = (1 << (HEIGHT - 1));
    inline TYPE_BUCKET_NUM REAL_BLOCK_NUM = 2 * TOTAL_NUM_LEAF_BUCKETS;
    inline BucketConfig::TYPE_BUCKET_SIZE EVICTION_FREQUENCY = 43;
    typedef BucketConfig::TYPE_BUCKET_SIZE TYPE_ACCESS_COUNT;
    inline TYPE_ACCESS_COUNT ACCESS_COUNT_EVICTION_COMPLETE = 0;
    inline PathConfig::TYPE_PATH_ID EVICTION_PATH_ID = 0;
    inline bool CanBlockGoThroughBucket(PathConfig::TYPE_PATH_ID pathID,
                                        PathConfig::TYPE_PATH_SIZE height,
                                        BucketConfig::TYPE_BUCKET_ID bucketID,
                                        PathConfig::TYPE_PATH_SIZE curLevel) {
        #if USE_ASSERT
        assert(height - 1 < 64 && "Height should be less than 64");
        #endif
        std::string pathBinary = std::bitset<64>(pathID).to_string().substr(64 - (height - 1), height - 1);
        std::string currentLevelBits = pathBinary.substr(0, curLevel);
        BucketConfig::TYPE_BUCKET_ID newBucketID = 0;
        for (auto bit : currentLevelBits) {
            newBucketID = newBucketID * 2 + (bit - '0') + 1;
        }
        return newBucketID == bucketID;                                  
    }
    inline void GenPathBucketIDsInReverseOrder(PathConfig::TYPE_PATH_ID pathID,
                                               const PathConfig::TYPE_PATH_SIZE height,
                                               std::vector<BucketConfig::TYPE_BUCKET_ID>& bucketIDs) {
        #if USE_ASSERT
        assert(bucketIDs.size() == height - 1 && "The size of the bucketIDs should be equal to height - 1");
        #endif
        std::bitset<HEIGHT - 1> pathBinary(pathID);
        for (PathConfig::TYPE_PATH_SIZE i = 0; i < height - 1; i++) {
            if (i == 0) {
                bucketIDs[i] = 0;
            } else {
                bucketIDs[i] = bucketIDs[i - 1] * 2 + (pathBinary[i - 1] ? 2 : 1);
            }
        }

    }
}
namespace ElGamalNTLConfig {
    inline long KEY_SIZE = 1024;
    inline long RANDOM_SIZE = KEY_SIZE - 8;
    inline size_t CHUNK_SIZE = 127;
    inline size_t PER_CIPHERTEXT_SIZE = CHUNK_SIZE + 1;
    inline unsigned long long BLOCK_CHUNK_SIZE = (BlockConfig::BLOCK_SIZE + CHUNK_SIZE - 1) / CHUNK_SIZE;
    inline unsigned long long BLOCK_CIPHERTEXT_NUM_CHARS = (BlockConfig::BLOCK_SIZE + CHUNK_SIZE  - 1) / CHUNK_SIZE * PER_CIPHERTEXT_SIZE * 2;
    inline unsigned long long BUCKET_CIPHERTEXT_NUM_CHARS = BucketConfig::BUCKET_SIZE * BLOCK_CIPHERTEXT_NUM_CHARS;
    typedef unsigned long long TYPE_BATCH_SIZE;
    // inline ZZ P = conv<ZZ>("137023064178165891084793017251156271253821108640440598403496780227279482618583204037416533837112178853273808032655507926785217847581370258269600498749583511409070428574934366756781235386968103578609364846782118404312687419732948149808671672944414544420165757407493410634344729271160690663650489920898889040109");
    inline ZZ P;
    inline ZZ G = conv<ZZ>("2");
    inline ZZ_p G_p;
    inline ZZ SEED = conv<ZZ>("32");
    // // set the seed for the random number generator in NTL
    // // inline ZZ X = RandomLen_ZZ(RANDOM_SIZE);
    inline ZZ X;
    inline ZZ_p X_p;
    // // inline ZZ Y = PowerMod(G, X, P); // Calculate public key y = g^x mod p
    inline ZZ Y;
    inline ZZ_p Y_p;
    // // inline ZZ K = RandomLen_ZZ(RANDOM_SIZE);
    inline ZZ K;
    inline ZZ_p K_p; 

    inline ZZ_p GPowK;
    inline ZZ_p YPowK;
    

}

// Directly define and initialize the shared pointers for BIGNUMs
namespace ElGamalConfig {
    // Initialize BN_CTX once for all operations
    inline BN_CTX* global_ctx = BN_CTX_new();
    // // inline std::shared_ptr<BIGNUM> p = std::shared_ptr<BIGNUM>(BN_new(), BN_free);
    // // inline std::shared_ptr<BIGNUM> g = std::shared_ptr<BIGNUM>(BN_new(), BN_free);
    // // inline std::shared_ptr<BIGNUM> x = std::shared_ptr<BIGNUM>(BN_new(), BN_free);
    // // inline std::shared_ptr<BIGNUM> y = std::shared_ptr<BIGNUM>(BN_new(), BN_free);
    // inline std::unique_ptr<BIGNUM, decltype(bn_deleter)> p(BN_new(), bn_deleter);
    // inline std::unique_ptr<BIGNUM, decltype(bn_deleter)> g(BN_new(), bn_deleter);
    // inline std::unique_ptr<BIGNUM, decltype(bn_deleter)> x(BN_new(), bn_deleter);
    inline BIGNUM* p = BN_new();
    inline BIGNUM* g = BN_new();
    inline BIGNUM* x = BN_new();
    inline BIGNUM* y = BN_new();
    inline size_t key_size = 1024;
    inline size_t num_threads_ed_server = 32; // Number of threads for the server's parallel operations for ElGamal encryption and decryption
    inline size_t num_threads_ed_client = 12; // Number of threads for the client's parallel operations for ElGamal encryption and decryption
    // Static block to initialize p, g, x, and y
    struct Initialize {
        Initialize() {
            // // Initialize p with a predefined prime value
            // Set the prime p
            BN_dec2bn(&p, "137023064178165891084793017251156271253821108640440598403496780227279482618583204037416533837112178853273808032655507926785217847581370258269600498749583511409070428574934366756781235386968103578609364846782118404312687419732948149808671672944414544420165757407493410634344729271160690663650489920898889040109");
            // Set the generator g
            BN_dec2bn(&g, "2");

            // // Generate the private key x deterministically within the range [1, p-1]
            // BN_pseudo_rand(x, key_size - 8, 1, 0);

            // assert(BN_cmp(x, p) < 0);  // Ensure x < p
            BN_dec2bn(&x, "688880906138027505018749713494996639691597296480853368000695721205090299584875266344628218372660633766634197806210860664264183178672964084752041142612214173899006181263338824498598114693628967919869074552155803919983954513698522164404801832316334912523803436641646994022131440646533427136560612519579437844");
    
            // Calculate public key y = g^x mod p
            BN_mod_exp(y, g, x, p, global_ctx);
        }
    };

    // This static instance will ensure the values are initialized only once before main
    inline Initialize initializer;
    // Function to generate binary data of a given size
    inline std::vector<char> generate_binary_data(size_t size, unsigned int seed = 32) {  // Optional seed parameter
        std::vector<char> data(size);  // Initialize the vector with the specified size
        // use random_device to generate a seed for the random number engine
        std::random_device rd;
        // std::mt19937 generator(rd());  // Use a random seed
        std::mt19937 generator(seed);  // Use a fixed seed for reproducibility
        std::uniform_int_distribution<int> distribution(0, 1);  // Generate random numbers between 0 and 1

        for (size_t i = 0; i < size; i++) {
            data[i] = static_cast<char>(distribution(generator));
        }
        return data;
    }
    inline std::vector<char> generate_zero_data(size_t size) {
        // fill the vector with zeros
        std::vector<char> data(size, 0);
        return data;
    }
    inline std::vector<char> generate_one_data(size_t size) {
        ZZ_p::init(ElGamalNTLConfig::P);
        // fill the vector with ones
        std::vector<char> data(size, 0);
        // Create ZZ_p representing '1'
        ZZ one = conv<ZZ>("1");
        size_t i = 0;
        size_t chunk_size = ElGamalNTLConfig::CHUNK_SIZE;
        while (i + chunk_size <= size) {
            BytesFromZZ(reinterpret_cast<unsigned char*>(data.data()) + i, one, chunk_size);
            i += chunk_size;
        }
        if (i < size) {
            BytesFromZZ(reinterpret_cast<unsigned char*>(data.data()) + i, one, size - i);
        }
        return data;
    }
}


// extern "C" {
//     #include "crtecelgamal.h"
// }

// namespace ECElGamalConfig {

// }

#endif // CONFIG_H
