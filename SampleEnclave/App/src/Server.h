#ifndef SERVER_H
#define SERVER_H
#include "Path.h"
#include "config.h"
#include "SocketCommunicator.h"
#include "Path.h"
#include "Tree.h"
#include <fstream> 
#include "DurationLogger.h"
#include "sgx_urts.h"

class Server {
public:
    Server(ServerConfig::TYPE_PORT_NUM port);
    ~Server();
    void Start();
    void handleClient(int clientSockfd);
    void InitConnectThirdParty();
    void EnsureConnectThirdParty();

    inline void PrintBlockIDsInBucket(const std::vector<std::vector<std::pair<ZZ_p, ZZ_p>>>& bucketCiphertexts);
    void PrintBlockIDsInDiskBucket(const BucketConfig::TYPE_BUCKET_ID bucketID);
    void TestBucketContent(std::vector<char>& bucketData, const BucketConfig::TYPE_BUCKET_ID bucketID);

    ServerConfig::TYPE_PORT_NUM port;
    SocketCommunicator communicator;
    SocketCommunicator communicator2ThirdParty;
    ServerConfig::TYPE_CMD command;

    // Variables for the CMD_CREATE_DB command
    PathConfig::TYPE_PATH_ID pathID;
    std::vector<char> pathIDChars;
    BlockConfig::TYPE_BLOCK_ID blockID;
    std::vector<char> blockIDChars;
    std::vector<BucketConfig::TYPE_SLOT_ID> offsets;
    std::vector<char> offsetsChars; 
    ClientConfig::TYPE_CHAR_SIZE offsetsCharsSize;
    Bucket bucket;
    Path path;
    ElGamal_parallel_ntl elgamal;
    // std::vector<std::pair<ZZ, ZZ>> targetBlockCiphertexts;
    std::vector<std::pair<ZZ_p, ZZ_p>> targetBlockCiphertexts;
    // std::vector<std::pair<ZZ, ZZ>> originalTargetBlockCiphertexts;
    std::vector<std::pair<ZZ_p, ZZ_p>> originalTargetBlockCiphertexts;
    std::vector<char> targetBlockCiphertextsSerializedData;
    // std::vector<std::pair<ZZ, ZZ>> resultCiphertexts;
    std::vector<std::pair<ZZ_p, ZZ_p>> resultCiphertexts;


    // Variable for the early reshuffle
    BucketConfig::META_DATA rootBucketMD;
    ClientConfig::TYPE_CHAR_SIZE permDataSize;
    std::vector<char> permData2;
    std::vector<BucketConfig::TYPE_SLOT_ID> perm2;
    // std::vector<std::vector<std::pair<ZZ, ZZ>>> bucketCiphertexts;
    std::vector<std::vector<std::pair<ZZ_p, ZZ_p>>> bucketCiphertexts;
    std::vector<std::pair<ZZ_p, ZZ_p>> bucketCiphertexts_flat;
    std::vector<std::vector<std::pair<ZZ_p, ZZ_p>>> bucketCiphertextsPermuted;
    std::vector<char> bucketCiphertextsSerializedData;
    std::vector<char> blockCiphertextsSerializedData;
    ClientConfig::TYPE_CHAR_SIZE size;

    // Variables for the eviction
    std::vector<BucketConfig::TYPE_BUCKET_ID> tripletBucketIDs;
    std::vector<BucketConfig::META_DATA> tripletBucketMDs;
    BucketConfig::TYPE_BUCKET_SIZE realBlockNumForEviction;
    std::vector<char> rootBucketData;
    std::vector<BucketConfig::TYPE_SLOT_ID> triplet_evict_perm2;
    // BucketConfig::TYPE_SLOT_ID triplet_evict_perm_size;
    std::vector<std::vector<std::pair<ZZ_p, ZZ_p>>> path_evict_bucketCiphertexts_complete;
    std::vector<std::vector<std::pair<ZZ_p, ZZ_p>>> triplet_evict_bucketCiphertexts;
    std::vector<std::pair<ZZ_p, ZZ_p>> triplet_evict_bucketCiphertexts_flat;
    std::vector<std::pair<ZZ_p, ZZ_p>> single_bucketCiphertexts_flat;
    std::vector<char> triplet_evict_bucketCiphertextsSerializedData;
    ClientConfig::TYPE_CHAR_SIZE triplet_evict_bucketCiphertextsSerializedDataSize;
    std::vector<char> triplet_evict_permData2;
    ClientConfig::TYPE_CHAR_SIZE triplet_evict_perm_size;

    // Variables for the complete operations
    std::vector<std::pair<ZZ_p, ZZ_p>> blockCiphertextsScheme2;
    std::vector<BucketConfig::META_DATA> treeMetaDatas;
    Tree tree;
    PathConfig::TYPE_PATH_ID pathIDComplete;
    std::vector<BucketConfig::TYPE_SLOT_ID> pathCompleteOffsets;
    std::vector<BucketConfig::TYPE_BUCKET_ID> bucketIDOffsets;
    ClientConfig::TYPE_CHAR_SIZE pathCompleteOffsetsSize;
    // std::vector<BucketConfig::TYPE_SLOT_ID> perm1EarlyReshuffleComplete;
    std::vector<BucketConfig::TYPE_SLOT_ID> perm2EarlyReshuffleComplete;
    BucketConfig::TYPE_BUCKET_ID bucketIDEarlyReshuffleComplete;
    ClientConfig::TYPE_CHAR_SIZE offsetEarlyReshuffleComplete;
        // Variables for the complete eviction
    std::vector<char> rootBucketDataEvictComplete;
    std::vector<std::vector<std::pair<ZZ_p, ZZ_p>>> bucketCiphertextsEvictComplete;
    std::vector<BucketConfig::TYPE_SLOT_ID> tripletEvictPermComplete;
    std::vector<BucketConfig::TYPE_BUCKET_ID> tripletBucketIDsComplete;
    ClientConfig::TYPE_CHAR_SIZE tripletBucketIDsCompleteCharNum;
    std::vector<BucketConfig::TYPE_BUCKET_ID> evictPathBucketIDsComplete;
    // file class
    std::ofstream ofs_evict;
    std::ofstream ofs_evict_root;
    std::ifstream ifs_evict;

    std::ofstream ofs_early_reshuffle;

    // Logger related
    DurationLogger logger;
    std::string LogEvictPreparePathBuckets = "EvictPreparePathBuckets";
    std::string LogEvictUpdateRootBucket = "EvictUpdateRootBucket";
    std::string LogEvictPrepareFirstTripletBuckets = "EvictPrepareFirstTripletBuckets";
        std::string LogEvictCopyCiphertextsFromPath2Triplet = "ServerComputationEvictCopyCiphertextsFromPath2Triplet";
        std::string LogEvictRerandomizeFirstTripletBuckets = "EvictRerandomizeFirstTripletBuckets";
        std::string LogEvictPermutateFirstTripletBuckets = "ServerComputationEvictPermutateFirstTripletBuckets";
        std::string LogEvictSerializeCiphertextsFirstTripletBuckets = "ServerComputationEvictSerializeCiphertextsFirstTripletBuckets";
    std::string LogEvictDesearilizeData2PathBuckets = "EvictDesearilizeData2PathBuckets";
    std::string LogEvictProcessNonRootTripletBuckets = "EvictProcessNonRootTripletBuckets";


    // Read Path Scheme 1
    std::string LogReadPathGenBucketIDsScheme1 = "ServerComputationReadPathGenBucketIDsScheme1";
    std::string LogReadPathLoadMDsScheme1 = "ServerIOReadPathLoadMDsScheme1";
    std::string LogReadPathLoadBlocksScheme1 = "ServerIOReadPathLoadBlocksScheme1";
    std::string LogReadPathXORBlocksScheme1 = "ServerComputationReadPathXORBlocksScheme1";
    // Early Reshuffle Scheme 1
    std::string LogEarlyReshuffleLoadBucketMDScheme1 = "ServerIOEarlyReshuffleLoadBucketMDScheme1";
    std::string LogEarlyReshuffleLoadReadBlocksScheme1 = "ServerIOEarlyReshuffleLoadReadBlocksScheme1";
    std::string LogEarlyReshuffleWriteBucketScheme1 = "ServerIOEarlyReshuffleWriteBucketScheme1";
    // Eviction Scheme 1
    std::string LogEvictPathGenBucketIDsScheme1 = "ServerComputationEvictPathGenBucketIDsScheme1";
    std::string LogEvictPathReadPathBucketsScheme1 = "ServerIOEvictPathReadPathBucketsScheme1";
    std::string LogEvictPathLoadTripletBucketMDsScheme1 = "ServerComputationEvictPathLoadTripletBucketMDsScheme1";
    std::string LogEvictPathLoadRealBlocksScheme1 = "ServerComputationEvictPathLoadRealBlocksScheme1";
    std::string LogEvictPathWritePathBucketsScheme1 = "ServerIOEvictPathWritePathBucketsScheme1";

    // Read Path Scheme 2
    std::string LogReadPathGenBucketIDsScheme2 = "ServerComputationReadPathGenBucketIDsScheme2";
    std::string LogReadpathLoadSingleBlockScheme2 = "ServerIOReadpathLoadSingleBlockScheme2";
    std::string LogReadPathMultiplyCiphertextsScheme2 = "ServerComputationReadPathMultiplyCiphertextsScheme2";
    std::string LogReadPathSerializeCiphertextScheme2 = "ServerIOReadPathSerializeCiphertextScheme2";
    std::string LogReadPathSendCiphertextScheme2 = "ClientServerBandwidthReadPathSendCiphertextScheme2";
    // Early Reshuffle Scheme 2
    
    std::string LogEarlyReshuffleRerandomizeandSerializeScheme2 = "ServerComputationEarlyReshuffleRerandomizeandSerializeScheme2";
    std::string LogEarlyReshuffleSendBucketToThirdPartyScheme2 = "ServerThirdPartyBandwidthEarlyReshuffleSendBucketToThirdPartyScheme2";
    std::string LogEarlyReshuffleWriteBucketBackToDiskScheme2 = "ServerIOEarlyReshuffleWriteBucketBackToDiskScheme2";
    // Eviction Scheme 2
    std::string LogEvictionGenPathBucketIDsScheme2 = "ServerComputationEvictionGenPathBucketIDsScheme2";
    std::string LogEvictionLoadPathBucketsFromDiskScheme2 = "ServerIOEvictionLoadPathBucketsFromDiskScheme2";
    std::string LogEvictionLoadRootBucketFromDiskScheme2 = "ServerIOEvictionLoadRootBucketFromDiskScheme2";
    std::string LogEvictionSendRootBucketToClientScheme2 = "ClientServerBandwidthEvictionSendRootBucketToClientScheme2";
    std::string LogEvictionProcessTripletBucketsScheme2 = "ServerComputationEvictionProcessTripletBucketsScheme2";
    std::string LogEvictionSendTripletBucketsToThirdPartyScheme2 = "ServerThirdPartyBandwidthEvictionSendTripletBucketsToThirdPartyScheme2";
    std::string LogEvictionDeserializeTripletBucketsScheme2 = "ServerComputationEvictionDeserializeTripletBucketsScheme2";
    std::string LogEvictionDeserializeBucketScheme2 = "ServerComputationEvictionDeserializeBucketScheme2";
    std::string LogEvictionRerandomizationTripletBucketsScheme2 = "ServerComputationEvictionRerandomizationTripletBucketsScheme2";
    std::string LogEvictionPermutateAndSerializeTripletBucketsScheme2 = "ServerComputationEvictionPermutateAndSerializeTripletBucketsScheme2";
    std::string LogEvictionWritePathBucketsBackToDiskScheme2 = "ServerIOEvictionWritePathBucketsBackToDiskScheme2";
    std::string LogEvictionWriteRootBucketBackToDiskScheme2 = "ServerIOEvictinoWriteRootBucketBackToDiskScheme2";

    /// Firt scheme variables
    std::vector<char> pathBucketMDData1;
    ClientConfig::TYPE_CHAR_SIZE pathBucketMDDataSize1;
    ServerConfig::TYPE_CMD cmd1;
    std::vector<char> pathBlocksData1;
    ClientConfig::TYPE_CHAR_SIZE pathBlocksDataSize1;
    std::vector<char> targetBlockCipherData1;
    size_t counter1;

    // Functions for the scheme 1
    void EarlyReshuffleScheme1(BucketConfig::TYPE_BUCKET_ID bucketID = 0);
        //SGX related
    void SgxEarlyReshuffleScheme1(sgx_enclave_id_t eid, BucketConfig::TYPE_BUCKET_ID bucketID = 0);
    void SgxEnclaveThreadFunc(void* arg);
    pthread_t enclaveThread;

    // EarlyReshuffleScheme1 variables
    BucketConfig::TYPE_BUCKET_ID bucketIDEarlyReshuffle1;
    std::vector<char> bucketMDataEarlyReshuffle1;
    std::vector<char> bucketRealDataEarlyReshuffle1;
    std::vector<BucketConfig::TYPE_SLOT_ID> realBlocksOffsetEarlyReshuffle1;
    ClientConfig::TYPE_CHAR_SIZE realBlocksOffsetEarlyReshuffle1Size;
    std::vector<char> realBlocksDataEarlyReshuffle1;
    std::vector<char> bucketCipherDataEarlyReshuffle1;
    ClientConfig::TYPE_CHAR_SIZE bucketCipherDataEarlyReshuffle1Size;
    // Eviction variables
    PathConfig::TYPE_PATH_ID pathIDEviction1;
    void EvictScheme1(PathConfig::TYPE_PATH_ID);
    std::vector<char> pathBucketsDataEviction1;
    ClientConfig::TYPE_CHAR_SIZE pathBucketsDataEviction1Size;
    ClientConfig::TYPE_CHAR_SIZE bucketSizeEviction1;
    std::vector<char> tripletBucketMDsDataEviction1;
    ClientConfig::TYPE_CHAR_SIZE tripletBucketMDsDataEviction1Size;
    std::vector<BucketConfig::TYPE_SLOT_ID> tripletBucketRealBlocksOffsetEviction1;
    ClientConfig::TYPE_CHAR_SIZE tripletBucketRealBlocksOffsetEviction1Size;
    std::vector<char> tripletBucketsRealBlocksDataEviction1;
    ClientConfig::TYPE_CHAR_SIZE tripletBucketsRealBlocksDataEviction1Size;
    ClientConfig::TYPE_CHAR_SIZE offset_base;
    PathConfig::TYPE_PATH_SIZE curLevel = 1;
    std::vector<char> tripletBucketsDataEviction1;
    ClientConfig::TYPE_CHAR_SIZE tripletBucketsDataEviction1Size;
    // Variables for the multi-thrading of the second scheme: eviction operation and the triplet buckets
    // Multi-threading variables
    ElGamalNTLConfig::TYPE_BATCH_SIZE bucketCiphertextsNumPairs;
    ElGamalNTLConfig::TYPE_BATCH_SIZE blockCiphertextsNumPairs;
    // SGX variables
        // Early Reshuffle Scheme 1
        std::vector<char> sharedBucketBuffer;
};


#endif // SERVER_H