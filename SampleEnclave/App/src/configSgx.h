#ifndef CONFIGSGX_H
#define CONFIGSGX_H
// #define UNIT_TEST_SGX
#define FLAG_ENCRYPT_BLOCK_SGX 0
#define FLAG_PARALLEL_ENCRYPT_SGX 1
#define MODULUS_SGX_STR "127584585272019464248550689001494335089005706365070593228774866021859296684878671063534087601345701705074143342155398610282864933991154081845709883272032834902599526248307926892808776816768645411849000298403789217318224840138553553706229104932113309533452119580791708821809060741851733422802747352257459692961"
typedef long long TYPE_BLOCK_ID_SGX;
typedef unsigned long TYPE_UNSIGNED_SIZE_SGX;
typedef long long TYPE_PATH_ID_SGX;
typedef unsigned short TYPE_SLOT_ID_SGX;
typedef short TYPE_SLOT_ID_S_SGX;
const TYPE_UNSIGNED_SIZE_SGX BUCKET_REAL_BLOCK_CAPACITY_SGX = 30;
const TYPE_UNSIGNED_SIZE_SGX BUCKET_SIZE_SGX = 73;
inline TYPE_UNSIGNED_SIZE_SGX META_DATA_SIZE_SGX = sizeof(size_t) + BUCKET_SIZE_SGX * (sizeof(bool)) +
                                            sizeof(size_t) + BUCKET_REAL_BLOCK_CAPACITY_SGX * sizeof(TYPE_BLOCK_ID_SGX) +
                                            BUCKET_REAL_BLOCK_CAPACITY_SGX * sizeof(TYPE_BLOCK_ID_SGX) +
                                            BUCKET_SIZE_SGX * sizeof(TYPE_PATH_ID_SGX) +
                                            BUCKET_SIZE_SGX * sizeof(TYPE_SLOT_ID_SGX) +
                                            sizeof(TYPE_SLOT_ID_SGX) * 2;
inline TYPE_UNSIGNED_SIZE_SGX PLAINMDSIZE_SGX = sizeof(size_t) + BUCKET_SIZE_SGX;
typedef size_t TYPE_BUCKET_ID_SGX;
typedef long unsigned int TYPE_BUCKET_SIZE_SGX;
typedef unsigned long long TYPE_BLOCK_SIZE_SGX;
typedef uint_fast16_t TYPE_SMALL_INDEX_U_SGX;
typedef size_t TYPE_PATH_SIZE_SGX;
inline TYPE_BLOCK_SIZE_SGX BLOCK_SIZE_SGX = 1024;
typedef size_t TYPE_PATH_SIZE_SGX;
// const uint8_t iv[AES_BLOCK_SIZE] = {0};
#include <unordered_set>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <numeric>
#include <random>
struct META_DATA_SGX {
    size_t count; // Used to log the access time for the bucket
    bool valids[BUCKET_SIZE_SGX]; // Indicates whether each of the slots in the bucket is valid or not
    std::unordered_set<TYPE_BLOCK_ID_SGX> block_ids; // Used to store the block ids in the bucket
    TYPE_BLOCK_ID_SGX addrs[BUCKET_REAL_BLOCK_CAPACITY_SGX]; // Contains addresses or identifiers of real blocks
    TYPE_PATH_ID_SGX leaves[BUCKET_REAL_BLOCK_CAPACITY_SGX]; // Contains the leaf ids of the blocks
    TYPE_SLOT_ID_SGX offsets[BUCKET_SIZE_SGX]; // Contains the offsets of the blocks
    TYPE_SLOT_ID_SGX offsetsGuidence[BUCKET_SIZE_SGX]; // Contains the offsets of the blocks
    TYPE_SLOT_ID_SGX offsetsInv[BUCKET_SIZE_SGX]; // Contains the offsets of the blocks
    TYPE_SLOT_ID_SGX nextDummyIndex; // Used to keep track of the next available dummy block index
    TYPE_SLOT_ID_SGX nextRealIndex; // Used to keep track of the next available real block index
    size_t block_ids_size;
    TYPE_BLOCK_ID_SGX id_serialized;
    // Constructor to initialize the data members of the META_DATA struct
    META_DATA_SGX() : count(0), nextDummyIndex(0), nextRealIndex(0) {
        for (TYPE_BUCKET_SIZE_SGX i = 0; i < BUCKET_SIZE_SGX; i++) {
            valids[i] = true;
        }
        for (TYPE_BUCKET_SIZE_SGX i = 0; i < BUCKET_REAL_BLOCK_CAPACITY_SGX; i++) {
            addrs[i] = -1;
            leaves[i] = -1;
        }
        GenPRP();
    }
    void Initialize() {
        count = 0;
        nextDummyIndex = 0;
        nextRealIndex = 0;
        for (TYPE_BUCKET_SIZE_SGX i = 0; i < BUCKET_SIZE_SGX; i++) {
            valids[i] = true;
        }
        for (TYPE_BUCKET_SIZE_SGX i = 0; i < BUCKET_REAL_BLOCK_CAPACITY_SGX; i++) {
            addrs[i] = -1;
            leaves[i] = -1;
        }
        GenPRP();
        // erase the block_ids
        block_ids.clear();
    }
    void SimpleReset() {
        count = 0;
        for (TYPE_BUCKET_SIZE_SGX i = 0; i < BUCKET_SIZE_SGX; i++) {
            if (!valids[i]) {
                valids[i] = true;
            }
        }
        shiftNextDummyIndex2End();
    }
    void shiftNextDummyIndex2End() {
        nextDummyIndex = BUCKET_SIZE_SGX;
    }
    // Function to convert the C-style array 'offsets' to a vector
    std::vector<TYPE_SLOT_ID_SGX> Offsets2Vector() {
        std::vector<TYPE_SLOT_ID_SGX> vec(offsets, offsets + BUCKET_SIZE_SGX);
        return vec;
    }
    static std::vector<TYPE_SLOT_ID_SGX> GenRandomPerm(TYPE_BUCKET_SIZE_SGX n) {
        std::vector<TYPE_SLOT_ID_SGX> perm(n);
        std::iota(perm.begin(), perm.end(), 0); // Fill the perm vector with 0, 1, 2, ..., n-1
        std::mt19937 g(10);
        std::shuffle(perm.begin(), perm.end(), g);
        return perm;
    }
    static std::vector<TYPE_SLOT_ID_SGX> InvertPerm(std::vector<TYPE_SLOT_ID_SGX>& perm) {
        std::vector<TYPE_SLOT_ID_SGX> inv(perm.size());
        for (TYPE_SLOT_ID_SGX i = 0; i < perm.size(); i++) {
            inv[perm[i]] = i;  // Correctly map the index
        }
        return inv;
    }
    void InvertPerm(TYPE_SLOT_ID_SGX perm[], TYPE_SLOT_ID_SGX inv[], TYPE_BUCKET_ID_SGX size) {
        for (TYPE_SLOT_ID_SGX i = 0; i < size; ++i) {
            inv[perm[i]] = i;  // Correctly map the index
        }
    }
    static void DividePerm(const std::vector<TYPE_SLOT_ID_SGX>& perm, std::vector<TYPE_SLOT_ID_SGX>& perm1, std::vector<TYPE_SLOT_ID_SGX>& perm2) {
        // Step 1: Generate the first random permutation (perm1)
        perm1 = GenRandomPerm(perm.size());
        // Step 2: Invert the first random permutation
        std::vector<TYPE_SLOT_ID_SGX> inv1 = InvertPerm(perm1);
        // Step 3: Generate perm2 as the inverse of the perm1 applied to the perm
        perm2.resize(perm.size());
        std::transform(perm.begin(), perm.end(), perm2.begin(), [&](TYPE_SLOT_ID_SGX val) {
            return inv1[val];
        });
    }
    std::vector<TYPE_SLOT_ID_SGX> combine_permutations(const std::vector<TYPE_SLOT_ID_SGX>& perm1, const std::vector<TYPE_SLOT_ID_SGX>& perm2) {
        std::vector<TYPE_SLOT_ID_SGX> combined(perm1.size());
        for (TYPE_SLOT_ID_SGX i = 0; i < perm1.size(); ++i) {
                combined[i] = perm1[perm2[i]];
            }
            return combined;
    }
    void ReShuffleOffsets() {
        TYPE_SLOT_ID_SGX temp[BUCKET_SIZE_SGX];
        TYPE_SLOT_ID_SGX updated[BUCKET_SIZE_SGX];
        for (TYPE_SLOT_ID_SGX i = 0; i < BUCKET_SIZE_SGX; i++) {
            temp[i] = offsets[i];
            updated[i] = i;
        }
        std::mt19937 g(20);
        std::shuffle(updated, updated + BUCKET_SIZE_SGX, g);
        for (TYPE_SLOT_ID_SGX i = 0; i < BUCKET_SIZE_SGX; i++) {
            offsets[temp[i]] = updated[i];
        }
    }
    void GenPRP(int seed = 32) {
        for (TYPE_SLOT_ID_SGX i = 0; i < BUCKET_SIZE_SGX; i++) {
            offsets[i] = i;
        }
        // Seed with a real random value, if available
        std::mt19937 g(seed);
        std::shuffle(offsets, offsets + BUCKET_SIZE_SGX, g);
    }
    void LogAccess() {
        ++count;
    }
    void AddRealBlock(TYPE_BLOCK_ID_SGX block_id, TYPE_PATH_ID_SGX leaf_id) {
        if (nextRealIndex >= BUCKET_REAL_BLOCK_CAPACITY_SGX) {
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
    void AddRealBlockPure(TYPE_BLOCK_ID_SGX block_id, TYPE_PATH_ID_SGX leaf_id) {
        if (nextRealIndex >= BUCKET_REAL_BLOCK_CAPACITY_SGX) {
            return;
        }
        addrs[nextRealIndex] = block_id;
        leaves[nextRealIndex] = leaf_id;
        block_ids.insert(block_id);
        nextRealIndex++;
    }
    void DenoteDeletedRealBlock(TYPE_SLOT_ID_SGX idx) {
        block_ids.erase(addrs[idx]);
        addrs[idx] = -1;
        leaves[idx] = -1;
        // nextRealIndex--;
    }
    void OrganizeRealBlocksIDs() {
        TYPE_SLOT_ID_S_SGX start = 0;
        // TYPE_SLOT_ID end = nextRealIndex - 1 < 0 ? 0 : nextRealIndex - 1;
        TYPE_SLOT_ID_S_SGX end = BUCKET_REAL_BLOCK_CAPACITY_SGX - 1;
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
    bool ContainsBlockID(TYPE_BLOCK_ID_SGX block_id) {
        return block_ids.find(block_id) != block_ids.end();
    }
    void DeleteBlockID(TYPE_BLOCK_ID_SGX block_id) {
        block_ids.erase(block_id);
    }
    // Serialize the META_DATA structure into a vector<char>
    void Serialize(std::vector<char>& data) {
        assert(data.size() == META_DATA_SIZE_SGX);
        auto it = data.data();
        std::memcpy(data.data(), &count, sizeof(count));
        it += sizeof(count);
        // Serialize the valids array
        for (TYPE_BUCKET_SIZE_SGX i = 0; i < BUCKET_SIZE_SGX; i++) {
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
        for (TYPE_BUCKET_SIZE_SGX i = 0; i < BUCKET_REAL_BLOCK_CAPACITY_SGX; i++) {
            std::memcpy(it, &addrs[i], sizeof(addrs[i]));
            it += sizeof(addrs[i]);
        }
        // Serialize the leaves array
        for (TYPE_BUCKET_SIZE_SGX i = 0; i < BUCKET_REAL_BLOCK_CAPACITY_SGX; i++) {
            std::memcpy(it, &leaves[i], sizeof(leaves[i]));
            it += sizeof(leaves[i]);
        }
        // Serialize the offsets array
        for (TYPE_SLOT_ID_SGX i = 0; i < BUCKET_SIZE_SGX; i++) {
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
        for (TYPE_BUCKET_SIZE_SGX i = 0; i < BUCKET_SIZE_SGX; i++) {
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
        for (TYPE_BUCKET_SIZE_SGX i = 0; i < BUCKET_REAL_BLOCK_CAPACITY_SGX; i++) {
            std::memcpy(it, &addrs[i], sizeof(addrs[i]));
            it += sizeof(addrs[i]);
        }
        
        // Serialize the leaves array
        for (TYPE_BUCKET_SIZE_SGX i = 0; i < BUCKET_REAL_BLOCK_CAPACITY_SGX; i++) {
            std::memcpy(it, &leaves[i], sizeof(leaves[i]));
            it += sizeof(leaves[i]);
        }
        
        // Serialize the offsets array
        for (TYPE_SLOT_ID_SGX i = 0; i < BUCKET_SIZE_SGX; i++) {
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
        assert(data.size() == META_DATA_SIZE_SGX);
        auto it = data.data();
        // Deserialize the count
        std::memcpy(&count, it, sizeof(count));
        it += sizeof(count);
        // Deserialize the valids array
        for (TYPE_BUCKET_SIZE_SGX i = 0; i < BUCKET_SIZE_SGX; i++) {
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
        for (TYPE_BUCKET_SIZE_SGX i = 0; i < BUCKET_REAL_BLOCK_CAPACITY_SGX; i++) {
            std::memcpy(&addrs[i], it, sizeof(addrs[i]));
            it += sizeof(addrs[i]);
        }
        // Deserialize the leaves array
        for (TYPE_BUCKET_SIZE_SGX i = 0; i < BUCKET_REAL_BLOCK_CAPACITY_SGX; i++) {
            std::memcpy(&leaves[i], it, sizeof(leaves[i]));
            it += sizeof(leaves[i]);
        }
        // Deserialize the offsets array
        for (TYPE_SLOT_ID_SGX i = 0; i < BUCKET_SIZE_SGX; i++) {
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
        for (TYPE_BUCKET_SIZE_SGX i = 0; i < BUCKET_SIZE_SGX; i++) {
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
        for (TYPE_BUCKET_SIZE_SGX i = 0; i < BUCKET_REAL_BLOCK_CAPACITY_SGX; i++) {
            std::memcpy(&addrs[i], it, sizeof(addrs[i]));
            it += sizeof(addrs[i]);
        }
        // Deserialize the leaves array
        for (TYPE_BUCKET_SIZE_SGX i = 0; i < BUCKET_REAL_BLOCK_CAPACITY_SGX; i++) {
            std::memcpy(&leaves[i], it, sizeof(leaves[i]));
            it += sizeof(leaves[i]);
        }
        // Deserialize the offsets array
        for (TYPE_SLOT_ID_SGX i = 0; i < BUCKET_SIZE_SGX; i++) {
            std::memcpy(&offsets[i], it, sizeof(offsets[i]));
            it += sizeof(offsets[i]);
        }
        // Deserialize the nextDummyIndex
        std::memcpy(&nextDummyIndex, it, sizeof(nextDummyIndex));
        it += sizeof(nextDummyIndex);
        // Deserialize the nextRealIndex
        std::memcpy(&nextRealIndex, it, sizeof(nextRealIndex));
    }
    void reset(std::vector<TYPE_SLOT_ID_S_SGX>& originalPerms) {
        // copy the content of offsets to offsetsGuidence
        std::copy(offsets, offsets + BUCKET_SIZE_SGX, offsetsGuidence);
        GenPRP(33);
        for (TYPE_SLOT_ID_SGX i = 0; i < BUCKET_SIZE_SGX; i++) {
            originalPerms[offsetsGuidence[i]] = offsets[i];
        }
        shiftNextDummyIndex2End();
        count = 0;
        for (TYPE_BUCKET_SIZE_SGX i = 0; i < BUCKET_SIZE_SGX; i++) {
            if (valids[i] == false) {
                valids[i] = true;
            }
        }
    }
    void ResetTriplets() {
        shiftNextDummyIndex2End();
        count = 0;
        for (TYPE_BUCKET_SIZE_SGX i = 0; i < BUCKET_SIZE_SGX; i++) {
            if (valids[i] == false) {
                valids[i] = true;
            }
        }
    }
    void ResetEarlyReshuffle1() {
        count = 0;
        for (TYPE_BUCKET_SIZE_SGX i = 0; i < BUCKET_SIZE_SGX; i++) {
            if (valids[i] == false) {
                valids[i] = true;
            }
        }
        ReShuffleOffsets();
        shiftNextDummyIndex2End();
        
    }
    void ResetEviction1() {
        count = 0;
        for (TYPE_BUCKET_SIZE_SGX i = 0; i < BUCKET_SIZE_SGX; i++) {
            if (valids[i] == false) {
                valids[i] = true;
            }
        }
        ReShuffleOffsets();
        for(TYPE_BUCKET_SIZE_SGX i = 0; i < nextRealIndex; ++i) {
            addrs[i] = -1;
            leaves[i] = -1;
        }
        nextRealIndex = 0;
        nextDummyIndex = 0;
        block_ids.clear();
    }
};

namespace TreeConfigSgx {
    inline constexpr TYPE_PATH_SIZE_SGX HEIGHT = 4;
    inline bool CanBlockGoThroughBucket(TYPE_PATH_ID_SGX pathID,
                                        TYPE_PATH_SIZE_SGX height,
                                        TYPE_BUCKET_ID_SGX bucketID,
                                        TYPE_PATH_SIZE_SGX curLevel) {
        std::string pathBinary = std::bitset<64>(pathID).to_string().substr(64 - (height - 1), height - 1);
        std::string currentLevelBits = pathBinary.substr(0, curLevel);
        TYPE_BUCKET_ID_SGX newBucketID = 0;
        for (auto bit : currentLevelBits) {
            newBucketID = newBucketID * 2 + (bit - '0') + 1;
        }
        return newBucketID == bucketID;                                  
    }
    inline void GenPathBucketIDsInReverseOrder(TYPE_PATH_ID_SGX pathID,
                                               const TYPE_PATH_SIZE_SGX height,
                                               std::vector<TYPE_BUCKET_ID_SGX>& bucketIDs) {
        #if USE_ASSERT
        assert(bucketIDs.size() == height - 1 && "The size of the bucketIDs should be equal to height - 1");
        #endif
        std::bitset<HEIGHT - 1> pathBinary(pathID);
        for (TYPE_PATH_SIZE_SGX i = 0; i < height - 1; i++) {
            if (i == 0) {
                bucketIDs[i] = 0;
            } else {
                bucketIDs[i] = bucketIDs[i - 1] * 2 + (pathBinary[i - 1] ? 2 : 1);
            }
        }

    }
}

#endif // CONFIGSGX_H