#include "../Path.h"
#include "../config.h"

void InitializeElGamalParams() {
    std::cout << "Initializing ElGamal Parameters..." << std::endl;
    
    // Set seed for random number generation
    SetSeed(ElGamalNTLConfig::SEED);
    
    // Generate prime P
    GenPrime(ElGamalNTLConfig::P, ElGamalNTLConfig::KEY_SIZE);
    ZZ_p::init(ElGamalNTLConfig::P);
    
    // Convert G to G_p
    ElGamalNTLConfig::G_p = conv<ZZ_p>(ElGamalNTLConfig::G);

    // Generate private key X and convert to X_p
    ElGamalNTLConfig::X = RandomLen_ZZ(ElGamalNTLConfig::RANDOM_SIZE);
    ElGamalNTLConfig::X_p = conv<ZZ_p>(ElGamalNTLConfig::X);

    
    // Calculate public key Y and convert to Y_p
    ElGamalNTLConfig::Y = PowerMod(ElGamalNTLConfig::G, ElGamalNTLConfig::X, ElGamalNTLConfig::P);
    ElGamalNTLConfig::Y_p = conv<ZZ_p>(ElGamalNTLConfig::Y);
    
    // Generate random K and convert to K_p
    ElGamalNTLConfig::K = RandomLen_ZZ(ElGamalNTLConfig::RANDOM_SIZE);
    ElGamalNTLConfig::K_p = conv<ZZ_p>(ElGamalNTLConfig::K);
     ElGamalNTLConfig::GPowK = power(ElGamalNTLConfig::G_p, ElGamalNTLConfig::K);
     ElGamalNTLConfig::YPowK = power(ElGamalNTLConfig::Y_p, ElGamalNTLConfig::K);
}

int main() {
    // Initialize ElGamal parameters
    InitializeElGamalParams();
    Block block;
    std::vector<char> block_data;
    //  void GenData(const BlockConfig::TYPE_BLOCK_SIZE data_size, const bool is_real, const BlockConfig::TYPE_BLOCK_ID id, std::vector<char>& data);
    block.GenData(BlockConfig::BLOCK_SIZE, false, -1, block_data);
    int i = 0;
    BIGNUM* chunk_data_bn = BN_new();
    for (i = 0; i + ElGamalNTLConfig::CHUNK_SIZE <= block_data.size(); i += ElGamalNTLConfig::CHUNK_SIZE) {
        BN_bin2bn(reinterpret_cast<const unsigned char*>(block_data.data() + i), ElGamalNTLConfig::CHUNK_SIZE, chunk_data_bn);
        std::cout << "chunk_data_bn[" << i / ElGamalNTLConfig::CHUNK_SIZE << "]: " << BN_bn2dec(chunk_data_bn) << std::endl;
    }

    BN_free(chunk_data_bn);
    Path path(0, PathConfig::HEIGHT);
    path.GenEvictPath(0, PathConfig::HEIGHT);
    auto data_dir = BucketConfig::DATADIR;
    std::string fileName = BucketConfig::BUCKETPREFIX + std::to_string(2);
    std::string bucket_0_path = data_dir + "/" + fileName;
    std::vector<std::vector<BIGNUM*>> ciphertexts(2);
    ciphertexts[0].resize(ElGamalNTLConfig::BUCKET_CHUNK_SIZE);
    ciphertexts[1].resize(ElGamalNTLConfig::BUCKET_CHUNK_SIZE);
    std::vector<BIGNUM*> data_bn(ElGamalNTLConfig::BUCKET_CHUNK_SIZE);
    for (size_t i = 0; i < ElGamalNTLConfig::BUCKET_CHUNK_SIZE; i++) {
        data_bn[i] = BN_new();
        ciphertexts[0][i] = BN_new();
        ciphertexts[1][i] = BN_new();
    }
    std::ifstream bucketFile(bucket_0_path, std::ios::binary);
    std::vector<char> ciphertextsData(ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
    bucketFile.read(ciphertextsData.data(), ElGamalNTLConfig::BUCKET_CIPHERTEXT_NUM_CHARS);
    bucketFile.close();
    ElGamal_parallel_ntl elgamal(2, BlockConfig::BLOCK_SIZE);
    elgamal.ConvertVecCharCipher2VecBN(ciphertextsData, ciphertexts);
    auto start = std::chrono::high_resolution_clock::now();
    elgamal.ParallelRerandomize(ciphertexts[0], ciphertexts[1]);
    auto end = std::chrono::high_resolution_clock::now();
    auto dur_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Time taken for ParallelRerandomize: " << dur_ns.count() << " ns" << std::endl;
    std::vector<BucketConfig::TYPE_SLOT_ID> permutation(BucketConfig::BUCKET_SIZE);
    for (size_t i = 0; i < BucketConfig::BUCKET_SIZE; i++) {
        permutation[i] = i;
    }
    std::random_shuffle(permutation.begin(), permutation.end());
    // print out the permutation
    std::cout << "Permutation: ";
    for (size_t i = 0; i < permutation.size(); i++) {
        std::cout << permutation[i] << " ";
    }
    std::cout << std::endl;
    start = std::chrono::high_resolution_clock::now();
    BNConfig::ApplyPermutation(ciphertexts, ElGamalNTLConfig::BUCKET_CHUNK_SIZE, BucketConfig::BUCKET_SIZE, permutation);
    end = std::chrono::high_resolution_clock::now();
    dur_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Time taken for ApplyPermutation: " << dur_ns.count() << " ns" << std::endl;
    // decrypt the ciphertexts
    elgamal.ParallelDecrypt(ciphertexts[0], ciphertexts[1], data_bn);
    BIGNUM* bn_one = BN_new();
    BN_one(bn_one);
    for (size_t i = 0; i < ElGamalNTLConfig::BUCKET_CHUNK_SIZE; i++) {
        assert(BN_cmp(data_bn[i], bn_one) == 0 && "[ElGamal]Error: data_bn is not equal to 1");
    }
    std::cout << "Finsihed decrypting the ciphertexts" << std::endl;
}