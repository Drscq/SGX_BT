#ifndef CONFIGSGX_H
#define CONFIGSGX_H
// #define UNIT_TEST_SGX
#define UNIT_TEST_OPENSSL 0
#define USE_OPENSSL 1
#define USE_NTL 0
#define FLAG_ENCRYPT_BLOCK_SGX 0
#define FLAG_PARALLEL_ENCRYPT_SGX 0
#define MODULUS_SGX_STR "127584585272019464248550689001494335089005706365070593228774866021859296684878671063534087601345701705074143342155398610282864933991154081845709883272032834902599526248307926892808776816768645411849000298403789217318224840138553553706229104932113309533452119580791708821809060741851733422802747352257459692961"
#define G_POW_K_SGX_STR "40083997855862118923937149344645589792674528038057978735481375139222151232361106251704467686182942665522560614084897132392533394398852097840465776782001027075746728893709508786343499198273674380907212414800497498145397548260423112481243950738720675694427946192983487310949830082117540629404300848853885591847"
#define H_POW_K_SGX_STR "29618508524561166517497136886316688555076983847870737959949097739501240636104977753252594986224341925077749351500447866214294446970585478997054852466103612436615858995071186029084625705788845743453097498903225997726460262018803024684635745186667474696683522206632417563612057972685348857762605895901004828228"
#define X_STR "482799682945489710324915308594656721837549104706481546815960313315968546587473138105505672417597287418151889032693215803732661915256252364292438852312505745941248448648212718844897260941397054789545253695961462139914464984331999185806358791857634150244873041344597091874398496549135753301137525998831856724"
#include <openssl/bn.h>
#include <pthread.h>  // or <pthread.h> if aliased by the SGX environment
typedef long long TYPE_BLOCK_ID_SGX;
typedef unsigned long TYPE_UNSIGNED_SIZE_SGX;
typedef long long TYPE_PATH_ID_SGX;
typedef unsigned short TYPE_SLOT_ID_SGX;
typedef short TYPE_SLOT_ID_S_SGX;
const TYPE_UNSIGNED_SIZE_SGX BUCKET_REAL_BLOCK_CAPACITY_SGX = 3;
const TYPE_UNSIGNED_SIZE_SGX BUCKET_SIZE_SGX = 4;
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
inline TYPE_BLOCK_SIZE_SGX BLOCK_SIZE_SGX = 256;
typedef size_t TYPE_PATH_SIZE_SGX;
namespace BNConfig {
    inline int CHUNK_SIZE_SGX = 127;
    inline int PER_CIPHERTEXT_SIZE_SGX = CHUNK_SIZE_SGX + 1;
    inline int BLOCK_CHUNK_SIZE_SGX = (BLOCK_SIZE_SGX + CHUNK_SIZE_SGX - 1) / CHUNK_SIZE_SGX;
    inline int BUCKET_CHUNK_SIZE_SGX = BUCKET_SIZE_SGX * BLOCK_CHUNK_SIZE_SGX;
    inline int BLOCK_CIPHERTEXT_NUM_CHARS_SGX = BLOCK_CHUNK_SIZE_SGX * PER_CIPHERTEXT_SIZE_SGX * 2;
    inline int BUCKET_CIPHERTEXT_NUM_CHARS_SGX = BUCKET_SIZE_SGX * BLOCK_CIPHERTEXT_NUM_CHARS_SGX;
    inline void ApplyPermutation(std::vector<std::vector<BIGNUM*>>& bucketCiphertextsBN, int numChunks, int numBlocks, std::vector<TYPE_SLOT_ID_SGX>& perm) {
        std::vector<BIGNUM*> newCiphertexts0(numChunks);
        std::vector<BIGNUM*> newCiphertexts1(numChunks);
        int oldStart, newStart;
        for (int i = 0; i < numBlocks; ++i) {
            oldStart = i * BLOCK_CHUNK_SIZE_SGX;
            newStart = perm[i] * BLOCK_CHUNK_SIZE_SGX;
            for (int j = 0; j < BLOCK_CHUNK_SIZE_SGX; ++j) {
                newCiphertexts0[newStart + j] = bucketCiphertextsBN[0][oldStart + j];
                newCiphertexts1[newStart + j] = bucketCiphertextsBN[1][oldStart + j];
            }
        }

        bucketCiphertextsBN[0] = std::move(newCiphertexts0);
        bucketCiphertextsBN[1] = std::move(newCiphertexts1);
    }
    struct ThreadBNData {
        std::vector<BIGNUM*>* c1; // pointer to c1 vector
        std::vector<BIGNUM*>* c2; // pointer to c2 vector
        int startIdx; // start index
        int endIdx; // end index
        BIGNUM* g_pow_k_sgx; // pointer to g_pow_k_sgx
        BIGNUM* h_pow_k_sgx; // pointer to h_pow_k_sgx
        BIGNUM* modulus_sgx; // pointer to modulus_sgx
        BN_CTX* ctx_sgx; // pointer to ctx_sgx
    };
    static void* WorkerFunction(void* arg) {
        BNConfig::ThreadBNData* thread_data = reinterpret_cast<BNConfig::ThreadBNData*>(arg);
        for (int i = thread_data->startIdx; i < thread_data->endIdx; ++i) {
            BN_mod_mul((*thread_data->c1)[i], (*thread_data->c1)[i], thread_data->g_pow_k_sgx, thread_data->modulus_sgx, thread_data->ctx_sgx);
            BN_mod_mul((*thread_data->c2)[i], (*thread_data->c2)[i], thread_data->h_pow_k_sgx, thread_data->modulus_sgx, thread_data->ctx_sgx);
        }
        return nullptr;
    }
    inline void ConvertVecCharCipher2VecBN(const char* data, std::vector<std::vector<BIGNUM*>>& ciphertexts) {
        auto it = data;
        for (size_t ii = 0, size = ciphertexts[0].size(); ii < size; ++ii) {
            BN_bin2bn(reinterpret_cast<const unsigned char*>(it), PER_CIPHERTEXT_SIZE_SGX, ciphertexts[0][ii]);
            it += PER_CIPHERTEXT_SIZE_SGX;
            BN_bin2bn(reinterpret_cast<const unsigned char*>(it), PER_CIPHERTEXT_SIZE_SGX, ciphertexts[1][ii]);
            if (ii != size - 1) {
                it += PER_CIPHERTEXT_SIZE_SGX;
            }
        }
    }
    inline void ConvertVecBNCipher2VecChar(const std::vector<BIGNUM*>& c1, const std::vector<BIGNUM*>& c2, char* data) {
        auto it = reinterpret_cast<unsigned char*>(data);
        for (size_t i = 0, size = c1.size(); i < size; ++i) {
            BN_bn2binpad(c1[i], it, PER_CIPHERTEXT_SIZE_SGX);
            it += PER_CIPHERTEXT_SIZE_SGX;
            BN_bn2binpad(c2[i], it, PER_CIPHERTEXT_SIZE_SGX);
            if (i != size - 1) {
                it += PER_CIPHERTEXT_SIZE_SGX;
            }
        }
    }
    
    inline const int num_threads = 4;
    inline BIGNUM* MODULUS_SGX_BN = nullptr;
    inline BIGNUM* G_POW_K_SGX_BN = nullptr;
    inline BIGNUM* H_POW_K_SGX_BN = nullptr;
    inline BIGNUM* X_SGX_BN = nullptr;
    inline BN_CTX* CTX_SGX = nullptr;
    inline std::vector<BN_CTX*> m_ctx_vec_sgx(4);
    inline pthread_t threads[num_threads];
    inline void InitMGHC() {
        MODULUS_SGX_BN = BN_new();
        BN_dec2bn(&MODULUS_SGX_BN, MODULUS_SGX_STR);
        G_POW_K_SGX_BN = BN_new();
        BN_dec2bn(&G_POW_K_SGX_BN, G_POW_K_SGX_STR);
        H_POW_K_SGX_BN = BN_new();
        BN_dec2bn(&H_POW_K_SGX_BN, H_POW_K_SGX_STR);
        X_SGX_BN = BN_new();
        BN_dec2bn(&X_SGX_BN, X_STR);
        CTX_SGX = BN_CTX_new();
        for (int i = 0; i < 4; ++i) {
            m_ctx_vec_sgx[i] = BN_CTX_new();
        }
    }
    inline void FreeMGHC() {
        BN_free(MODULUS_SGX_BN);
        BN_free(G_POW_K_SGX_BN);
        BN_free(H_POW_K_SGX_BN);
        BN_free(X_SGX_BN);
        BN_CTX_free(CTX_SGX);
        for (int i = 0; i < 4; ++i) {
            BN_CTX_free(m_ctx_vec_sgx[i]);
        }
    }
    inline void ReRandomizeChunk(BIGNUM* c1, BIGNUM* c2) {
        BN_mod_mul(c1, c1, G_POW_K_SGX_BN, MODULUS_SGX_BN, CTX_SGX);
        BN_mod_mul(c2, c2, H_POW_K_SGX_BN, MODULUS_SGX_BN, CTX_SGX);
    }
    inline void ParallelDecrypt(const std::vector<BIGNUM*>& c1, const std::vector<BIGNUM*>& c2, std::vector<BIGNUM*>& data) {
        assert(c1.size() == c2.size() && c1.size() == data.size());
        BIGNUM* m_g_pow_k_x_inv_bn_sgx = BN_new();
        for (int i = 0; i < c1.size(); ++i) {
            BN_mod_exp(m_g_pow_k_x_inv_bn_sgx, c1[i], X_SGX_BN, MODULUS_SGX_BN, CTX_SGX);
            BN_mod_inverse(m_g_pow_k_x_inv_bn_sgx, m_g_pow_k_x_inv_bn_sgx, MODULUS_SGX_BN, CTX_SGX);
            BN_mod_mul(data[i], c2[i], m_g_pow_k_x_inv_bn_sgx, MODULUS_SGX_BN, CTX_SGX);
        }
        BN_free(m_g_pow_k_x_inv_bn_sgx);
    }
    inline int threadChunkSizeBN,currentIdxBN, actualThreads, startIdxBN, endIdxBN;
    inline std::vector<ThreadBNData> thread_bn_data(num_threads);
    inline pthread_t threadsBN[num_threads];
    inline void ParallelReRandomize(std::vector<BIGNUM*>& c1, std::vector<BIGNUM*>& c2) {
        threadChunkSizeBN = (c1.size() + num_threads - 1) / num_threads;
        currentIdxBN = 0;
        actualThreads = 0; // track how many threads we actually start
        for (int t = 0; t < num_threads; ++t) {
            // std::cout << "Starting thread " << t << std::endl;
            startIdxBN = currentIdxBN;
            endIdxBN = std::min(startIdxBN + threadChunkSizeBN, static_cast<int>(c1.size()));
            thread_bn_data[t].c1 = &c1;
            thread_bn_data[t].c2 = &c2;
            thread_bn_data[t].g_pow_k_sgx = G_POW_K_SGX_BN;
            thread_bn_data[t].h_pow_k_sgx = H_POW_K_SGX_BN;
            thread_bn_data[t].modulus_sgx = MODULUS_SGX_BN;
            thread_bn_data[t].ctx_sgx = m_ctx_vec_sgx[t];
            thread_bn_data[t].startIdx = startIdxBN;
            thread_bn_data[t].endIdx = endIdxBN;

            pthread_create(&threadsBN[t], nullptr, WorkerFunction, &thread_bn_data[t]);
            // // Then set CPU affinity:
            // cpu_set_t cpuset;
            // CPU_ZERO(&cpuset);
            // // Suppose you pin to core t if it exists, or you pick some mapping
            // CPU_SET(t + 1, &cpuset);
            // pthread_setaffinity_np(this->threads[t], sizeof(cpu_set_t), &cpuset);
            currentIdxBN = endIdxBN;
            actualThreads++;
            if (currentIdxBN >= (int)c1.size()) {
                // No more data to process
                break;
            }
        }

        for (int t = 0; t < actualThreads; ++t) {
            pthread_join(threadsBN[t], nullptr);
        }
    }
}
// const uint8_t iv[AES_BLOCK_SIZE] = {0};
#include <unordered_set>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <numeric>
#include <random>
#include <bitset>
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
    inline constexpr TYPE_PATH_SIZE_SGX HEIGHT = 8;
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