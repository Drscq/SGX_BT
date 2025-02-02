#ifndef ELGAMAL_PARALLEL_NTL_H
#define ELGAMAL_PARALLEL_NTL_H

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <vector> 
#include <future>
#include "config.h"
// #include "ThreadPool.h"
#include "DurationLogger.h"
#include <pthread.h> // For setting CPU affinity

using namespace NTL;

class ElGamal_parallel_ntl {
public:
    DurationLogger logger;
    std::string LogSingleChunkDecrypt = "SingleChunkElGamalDecrypt";
    std::string LogConvertToVector = "ConvertToVectorDecrypt";
    std::string LogCopyBlock = "CopyBlockDecrypt";
    // ElGamal_parallel_ntl();
    ElGamal_parallel_ntl(size_t num_threads = 1, size_t data_size = 1024);
    ~ElGamal_parallel_ntl();
    // Function to generate ElGamal parameters
    void GenerateParams(long bitLength = 1024, long seed = 32);

    // Function to encrypt a single block
    std::pair<ZZ, ZZ> EncryptBlock(const ZZ &message);
    std::pair<ZZ_p, ZZ_p> EncryptBlock(const ZZ_p &message);
    // Function to decrypt a single block
    ZZ DecryptBlock(const std::pair<ZZ, ZZ> &ciphertext);
    ZZ_p DecryptBlock(const std::pair<ZZ_p, ZZ_p> &ciphertext);
    // Function to re-randomize a single block
    std::pair<ZZ, ZZ> ReRandomizeBlock(const std::pair<ZZ, ZZ> &ciphertext);
    // std::pair<ZZ_p, ZZ_p> ReRandomizeBlock(const std::pair<ZZ_p, ZZ_p> &ciphertext);
    void ReRandomizeBlock(std::pair<ZZ_p, ZZ_p>& ciphertext) noexcept;
    std::pair<ZZ_p, ZZ_p> ReRandomizeBlock(const std::pair<ZZ_p, ZZ_p> &ciphertext) noexcept;
    // Function to multiply two ciphertexts
    std::pair<ZZ, ZZ> MultiplyCiphertexts(const std::pair<ZZ, ZZ> &ciphertext1, const std::pair<ZZ, ZZ> &ciphertext2);
    std::pair<ZZ_p, ZZ_p> MultiplyCiphertexts(const std::pair<ZZ_p, ZZ_p> &ciphertext1, const std::pair<ZZ_p, ZZ_p> &ciphertext2);
    // Function to encrypt data in parallel
    void ParallelEncrypt(const std::vector<char>& data, std::vector<std::pair<ZZ, ZZ>>& ciphertexts);
    void ParallelEncrypt(const std::vector<char>& data, std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts);
    // Function to decrypt data in parallel
    void ParallelDecrypt(const std::vector<std::pair<ZZ, ZZ>>& ciphertexts, std::vector<char>& data);
    void ParallelDecrypt(const std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts, std::vector<char>& data);
    // Function to re-randomize data in parallel
    void ParallelRerandomize(std::vector<std::pair<ZZ, ZZ>>& ciphertexts);
    void ParallelRerandomize(std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts);
    // Function to multiply two sets of ciphertexts in parallel
    void ParallelMultiplyCiphertexts(const std::vector<std::pair<ZZ, ZZ>>& ciphertexts1, const std::vector<std::pair<ZZ, ZZ>>& ciphertexts2, std::vector<std::pair<ZZ, ZZ>>& result);
    void ParallelMultiplyCiphertexts(const std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts1, const std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts2, std::vector<std::pair<ZZ_p, ZZ_p>>& result);
    // Function to serialize ciphertexts to a vector of chars
    void SerializeCiphertexts(const std::vector<std::pair<ZZ, ZZ>>& ciphertexts, std::vector<char>& serializedData);
    void SerializeCiphertexts(const std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts, std::vector<char>& serializedData);
    // Function to deserialize ciphertexts from a vector of chars
    void DeserializeCiphertexts(const std::vector<char>& serializedData, std::vector<std::pair<ZZ, ZZ>>& ciphertexts);
    void DeserializeCiphertexts(const std::vector<char>& serializedData, std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts);
    void DeserializeCiphertexts(const unsigned char* data_ptr, std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts);
    void DeserializeCiphertexts(const unsigned char* data_ptr, std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts, ElGamalNTLConfig::TYPE_BATCH_SIZE num_pairs_to_deserialize);
    void DeserializeCiphertexts(const char* dataPtr, size_t dataSize, std::vector<std::pair<ZZ_p, ZZ_p>>& ciphertexts);
    // Function to convert a byte array to a ZZ
    ZZ ZZFromBytes(const unsigned char* data, size_t size);
    ZZ_p ZZ_pFromBytes(const unsigned char* data, size_t size);
    // Helper function to convert a vector of chars to a ZZ, ignoring leading zeros
    // std::pair<ZZ, size_t> vector_to_ZZ(const std::vector<char> &data);
    // std::pair<ZZ_p, size_t> vector_to_ZZ_p(const std::vector<char> &data);
    ZZ vector_to_ZZ(const std::vector<char> &data);
    ZZ_p vector_to_ZZ_p(const std::vector<char> &data);

    // Helper function to convert a ZZ to a vector of chars restoring leading zeros
    std::vector<char> ZZ_to_vector(const ZZ &zz_data, size_t original_size);
    std::vector<char> ZZ_p_to_vector(const ZZ_p &zz_data, size_t original_size);
    void ZZ_p_to_bytes(unsigned char* buffer, size_t buffer_size, const ZZ_p& value);
private:
    size_t num_threads;
    size_t num_threads_deseralize = 4;
    size_t chunk_size;
    size_t per_ciphertext_size;
    size_t data_size;
    ZZ p; // Prime modulus
    ZZ g; // Generator
    ZZ_p g_p; // Generator in ZZ_p
    ZZ x;  // Private key
    ZZ_p x_p; // Private key in ZZ_p
    ZZ h; // Public key (h = g^x mod p)
    ZZ_p h_p; // Public key in ZZ_p
    ZZ k;
    ZZ_p k_p;
    ZZ_p g_pow_k;
    ZZ_p g_pow_k_x_inv;
    ZZ_p h_pow_k;
    // Variables for the SerializeCiphertexts and DeserializeCiphertexts functions
    size_t first_size_serialized;
    size_t second_size_serialized;

    // Variables to the parallel operations for ElGamal encryption and decryption
    // std::vector<std::future<void>> futures_rerandomize;
    ElGamalNTLConfig::TYPE_BATCH_SIZE batch_size_rerandomize = 86;
    ElGamalNTLConfig::TYPE_BATCH_SIZE batch_size_decrypt = 86;
    ElGamalNTLConfig::TYPE_BATCH_SIZE num_batches;
    ElGamalNTLConfig::TYPE_BATCH_SIZE batch_size_encrypt = 86, batch_size_total_encrypt;
    ElGamalNTLConfig::TYPE_BATCH_SIZE total_chunks;
    // Variables for the ParallelDecrypt function
    size_t i_parallel_decrypt;
    size_t c_index;
    // Variables for the ParallelDecrypt function
    std::vector<char> decrypted_chunk;
    ZZ_p decrypted_message_p;
    size_t chunk_size_decrypt_p;
    // Declare variables outside the loop to avoid reallocation
    ZZ temp_zz_dser;
    ZZ_p first_part_dser;
    ZZ_p second_part_dser;
    const unsigned char* data_ptr;
    const unsigned char* data_end;
    size_t num_pairs;
    unsigned char *buffer;
    size_t offset_searialized;

    // Multi-threading
    // pthread_t thread_compute[this->num_threads];
    pthread_t* thread_compute;
    void set_thread_affinity(std::thread& thread, int cpu_id);
   struct ThreadArgs {
        std::vector<std::pair<ZZ_p, ZZ_p>>* ciphertexts;
        ElGamalNTLConfig::TYPE_BATCH_SIZE startIdx;
        ElGamalNTLConfig::TYPE_BATCH_SIZE endIdx;
        ZZ_p g_pow_k;
        ZZ_p h_pow_k;
        ElGamalNTLConfig::TYPE_BATCH_SIZE core_id;
    };
    static void ReRandomizeBlock(std::pair<ZZ_p, ZZ_p>& ciphertext, const ZZ_p& g_pow_k, const ZZ_p& h_pow_k) noexcept {
        ciphertext.first *= g_pow_k;   // Efficient in-place multiplication
        ciphertext.second *= h_pow_k;  // Efficient in-place multiplication
    }

    static void set_thread_affinity(pthread_t thread, int cpu_id) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(cpu_id, &cpuset);
        int rc = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
        if (rc != 0) {
            std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
        }
    }
    static void* thread_func(void* arg) {
        ThreadArgs* args = static_cast<ThreadArgs*>(arg);
        // Set the CPU affinity for the thread
        set_thread_affinity(pthread_self(), static_cast<int>(args->core_id));
        auto& ciphertexts = *(args->ciphertexts);
        auto startIdx = args->startIdx;
        auto endIdx = args->endIdx;
        auto& g_pow_k = args->g_pow_k;
        auto& h_pow_k = args->h_pow_k;

        ZZ_p::init(ElGamalNTLConfig::P); // Initialize ZZ_p

        for (ElGamalNTLConfig::TYPE_BATCH_SIZE j = startIdx; j < endIdx && j < ciphertexts.size(); j++) {
            ReRandomizeBlock(ciphertexts[j], g_pow_k, h_pow_k);
        }
        #if USE_COUT
        std::cout << "The core used for the thread is: " << sched_getcpu() << std::endl;
        #endif

        return nullptr;
    }
    std::vector<pthread_t> threads;
    std::vector<ThreadArgs> thread_args;
    ElGamalNTLConfig::TYPE_BATCH_SIZE startIdx, endIdx;

    struct ThreadArgsDeserialize {
        const unsigned char* data_ptr;
        std::vector<std::pair<ZZ_p, ZZ_p>>* ciphertexts;
        ElGamalNTLConfig::TYPE_BATCH_SIZE startIdx;
        ElGamalNTLConfig::TYPE_BATCH_SIZE endIdx;
        ElGamalNTLConfig::TYPE_BATCH_SIZE core_id;
    };
    static void* thread_func_deserialize(void* arg) {
        ThreadArgsDeserialize* args = static_cast<ThreadArgsDeserialize*>(arg);
        set_thread_affinity(pthread_self(), static_cast<int>(args->core_id));
        const unsigned char* data_ptr = args->data_ptr;
        auto& ciphertexts = *(args->ciphertexts);
        auto startIdx = args->startIdx;
        auto endIdx = args->endIdx;

        ZZ_p::init(ElGamalNTLConfig::P); // Initialize ZZ_p

        for (ElGamalNTLConfig::TYPE_BATCH_SIZE j = startIdx; j < endIdx && j < ciphertexts.size(); j++) {
            ZZ& first_rep = const_cast<ZZ&>(rep(ciphertexts[j].first));
            ZZ& second_rep = const_cast<ZZ&>(rep(ciphertexts[j].second));
            NTL::ZZFromBytes(first_rep, data_ptr + (j * 2 * ElGamalNTLConfig::PER_CIPHERTEXT_SIZE), ElGamalNTLConfig::PER_CIPHERTEXT_SIZE);
            NTL::ZZFromBytes(second_rep, data_ptr + ((j * 2 + 1) * ElGamalNTLConfig::PER_CIPHERTEXT_SIZE), ElGamalNTLConfig::PER_CIPHERTEXT_SIZE);
        }

        return nullptr;
    }

    std::vector<ThreadArgsDeserialize> thread_args_deserialize;


};

#endif // ELGAMAL_PARALLEL_NTL_H;
