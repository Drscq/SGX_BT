#include <iostream>
#include "../ElGamal_parallel_ntl.h"

void InitializeElGamalParams() {
    std::cout << "Initializing ElGamal Parameters..." << std::endl;
    
    // Set seed for random number generation
    SetSeed(ElGamalNTLConfig::SEED);
    
    // Generate prime P
    GenPrime(ElGamalNTLConfig::P, ElGamalNTLConfig::KEY_SIZE);
    ZZ_p::init(ElGamalNTLConfig::P);
    // std::cout << "P: " << ElGamalNTLConfig::P << std::endl;  
    
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
int main() {
    // Initialize ElGamal parameters
    InitializeElGamalParams();
    size_t num_threads = 5;
    size_t data_size = 64 * 1024; // 64 KB
    ElGamal_parallel_ntl elgamal(num_threads, data_size);
    // check the parameters
    assert(elgamal.p == ElGamalNTLConfig::P && "p is not equal to ElGamalNTLConfig::P");
    assert(elgamal.g == ElGamalNTLConfig::G && "g is not equal to ElGamalNTLConfig::G");
    assert(elgamal.x == ElGamalNTLConfig::X && "x is not equal to ElGamalNTLConfig::X");
    assert(elgamal.h == ElGamalNTLConfig::Y && "h is not equal to ElGamalNTLConfig::Y");
    assert(elgamal.k == ElGamalNTLConfig::K && "k is not equal to ElGamalNTLConfig::K");
    assert(elgamal.g_pow_k == ElGamalNTLConfig::GPowK && "g_pow_k is not equal to ElGamalNTLConfig::GPowK");
    assert(elgamal.h_pow_k == ElGamalNTLConfig::YPowK && "h_pow_k is not equal to ElGamalNTLConfig::YPowK");
    // check the correctness of the operations: ParallelEncrypt, ParallelDecrypt, ParallelMultiplyCiphertexts, ParallelRerandomize
    size_t chunk_num_triplet_buckets = 3 * ElGamalNTLConfig::BUCKET_CHUNK_SIZE;
    std::vector<BIGNUM*> data(chunk_num_triplet_buckets);
    std::vector<BIGNUM*> data_decrypted(chunk_num_triplet_buckets);
    std::vector<ZZ_p> data_p(chunk_num_triplet_buckets);
    std::vector<std::vector<ZZ_p>> ciphertexts_p(2);
    ciphertexts_p[0].resize(chunk_num_triplet_buckets);
    ciphertexts_p[1].resize(chunk_num_triplet_buckets);
    std::vector<ZZ_p> data_p_decrypted(chunk_num_triplet_buckets);
    std::vector<std::vector<BIGNUM*>> ciphertexts;
    ciphertexts.resize(2);
    ciphertexts[0].resize(chunk_num_triplet_buckets);
    ciphertexts[1].resize(chunk_num_triplet_buckets);
    // generate random data with ElGamalNTLConfig::CHUNK_SIZE
    for (size_t i = 0; i < chunk_num_triplet_buckets; ++i) {
        data[i] = BN_new();
        ciphertexts[0][i] = BN_new();
        ciphertexts[1][i] = BN_new();
        data_decrypted[i] = BN_new();
        BN_rand(data[i], ElGamalNTLConfig::CHUNK_SIZE, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY);
        // convert the data to ZZ_p
        NTL::conv(data_p[i], BN_bn2dec(data[i]));
    }
    // Encrypt the data
    elgamal.ParallelEncrypt(data, ciphertexts[0], ciphertexts[1]);
    // Decrypt the data
    elgamal.ParallelDecrypt(ciphertexts[0], ciphertexts[1], data_decrypted);
    // check the correctness of the decryption
    for (size_t i = 0; i < chunk_num_triplet_buckets; ++i) {
        if (BN_cmp(data[i], data_decrypted[i]) != 0) {
            std::cout << "[Openssl]Decryption failed at index " << i << std::endl;
            break;
        }
    }
    elgamal.ParallelRerandomize(ciphertexts[0], ciphertexts[1]);
    // check the correctness of the rerandomization
    elgamal.ParallelDecrypt(ciphertexts[0], ciphertexts[1], data_decrypted);
    for (size_t i = 0; i < chunk_num_triplet_buckets; ++i) {
        if (BN_cmp(data[i], data_decrypted[i]) != 0) {
            std::cout << "[Openssl]Rerandomization failed at index " << i << std::endl;
            break;
        }
    }
    // NTL version check
    elgamal.ParallelEncrypt(data_p, ciphertexts_p[0], ciphertexts_p[1]);
    // Decrypt the data
    elgamal.ParallelDecrypt(ciphertexts_p[0], ciphertexts_p[1], data_p_decrypted);
    // check the correctness of the decryption
    for (size_t i = 0; i < chunk_num_triplet_buckets; ++i) {
        if (data_p[i] != data_p_decrypted[i]) {
            std::cout << "[NTL]Decryption failed at index " << i << std::endl;
            break;
        }
    }
    // Rerandomize the ciphertexts
    elgamal.ParallelRerandomize(ciphertexts_p[0], ciphertexts_p[1]);
    // check the correctness of the rerandomization
    elgamal.ParallelDecrypt(ciphertexts_p[0], ciphertexts_p[1], data_p_decrypted);
    for (size_t i = 0; i < chunk_num_triplet_buckets; ++i) {
        if (data_p[i] != data_p_decrypted[i]) {
            std::cout << "[NTL]Rerandomization failed at index " << i << std::endl;
            break;
        }
    }
    // int run_times = 1000;
    int threads_num = 42;
    std::vector<long long> elapsed_durations_openssl(threads_num);
    std::vector<long long> elapsed_durations_ntl(threads_num);
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto eplapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    for (int i = 1; i <= threads_num; ++i) {
        ElGamal_parallel_ntl elgamal_inside(i, BlockConfig::BLOCK_SIZE);
        for (size_t k = 0; k < chunk_num_triplet_buckets; ++k) {
            BN_rand(data[k], ElGamalNTLConfig::CHUNK_SIZE, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY);
            // convert the data to ZZ_p
            NTL::conv(data_p[k], BN_bn2dec(data[k]));
        }
        elgamal_inside.ParallelEncrypt(data, ciphertexts[0], ciphertexts[1]);
        start = std::chrono::high_resolution_clock::now();
        elgamal_inside.ParallelRerandomize(ciphertexts[0], ciphertexts[1]);
        end = std::chrono::high_resolution_clock::now();
        eplapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        elapsed_durations_openssl[i - 1] = eplapsed_ns.count();

        // NTL version check
        elgamal_inside.ParallelEncrypt(data_p, ciphertexts_p[0], ciphertexts_p[1]);
        start = std::chrono::high_resolution_clock::now();
        elgamal_inside.ParallelRerandomize(ciphertexts_p[0], ciphertexts_p[1]);
        end = std::chrono::high_resolution_clock::now();
        eplapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        elapsed_durations_ntl[i - 1] = eplapsed_ns.count();
    }
        

    // free the data
    for (size_t i = 0; i < chunk_num_triplet_buckets; ++i) {
        BN_free(data[i]);
        BN_free(ciphertexts[0][i]);
        BN_free(ciphertexts[1][i]);
        BN_free(data_decrypted[i]);
    }

    {
        std::ofstream ofs("elapsed_durations_parallel.csv");
        ofs << "OpenSSL,NTL\n";
        for (int i = 0; i < threads_num; i++) {
            ofs << elapsed_durations_openssl[i] << "," << elapsed_durations_ntl[i] << "\n";
        }
        ofs.flush();
        ofs.close();
        std::cout << "elapsed_durations.csv has been generated." << std::endl;
    }


    std::cout << "ElGamal parameters generated successfully." << std::endl;
    
}