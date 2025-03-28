#include <iostream>
#include <openssl/bn.h>
#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <chrono>
#include <vector>
#include <fstream>

int main() {
    // generate a random number with 1024 bits
    std::string a_str = "157042188556256613148171243922956907873058792839076206297346893420311906649012452261158968647447480792300869240773758310977973942196994142272613197057484236877874466165991812973520466388340672878930217411610991061373581739839224732947489952027141880699905712603179297245156915799845229861357681888887627070391";
    std::string b_str = "157042188556256613148171243922956907873058792839076206297346893420311906649012452261158968647447480792300869240773758310977973942196994142272613197057484236877874466165991812973520466388340672878930217411610991061373581739839224732947489952027141880699905712603179297245156915799845229861357681888887627070391";
    std::string mod_str = "127584585272019464248550689001494335089005706365070593228774866021859296684878671063534087601345701705074143342155398610282864933991154081845709883272032834902599526248307926892808776816768645411849000298403789217318224840138553553706229104932113309533452119580791708821809060741851733422802747352257459692961";
    BIGNUM *a_bn = BN_new();
    BIGNUM *b_bn = BN_new();
    BIGNUM *result = BN_new();
    BIGNUM *module = BN_new();
    BN_CTX *ctx = BN_CTX_new();

    BN_dec2bn(&a_bn, a_str.c_str());
    BN_dec2bn(&b_bn, b_str.c_str());
    BN_dec2bn(&module, mod_str.c_str());
    const int run_times = 1000;
    std::vector<long long> elapsed_durations_openssl(run_times);
    std::vector<long long> elapsed_durations_ntl(run_times);
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto eplapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
     // use mod_str init the zz_p
     NTL::ZZ mod;
     NTL::conv(mod, mod_str.c_str());
     NTL::ZZ_p::init(mod);
     NTL::ZZ_p a_p, b_p, res_p;
     NTL::ZZ a_zz, b_zz;
     NTL::conv(a_zz, a_str.c_str());
     NTL::conv(b_zz, b_str.c_str());
     a_p = NTL::conv<NTL::ZZ_p>(a_zz);
     b_p = NTL::conv<NTL::ZZ_p>(b_zz);
    for (int j = 1; j <= run_times; j++) {
        // generate a 1024 bits random value to the a_bn
        BN_rand(a_bn, 1024, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY);
        BN_rand(b_bn, 1024, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY);
        // use the a_bn and b_bn to init the a_p and b_p
        NTL::conv(a_zz, BN_bn2dec(a_bn));
        NTL::conv(b_zz, BN_bn2dec(b_bn));
        a_p = NTL::conv<NTL::ZZ_p>(a_zz);
        b_p = NTL::conv<NTL::ZZ_p>(b_zz);

        start = std::chrono::high_resolution_clock::now();
            // for (int i = 0; i < j; i++) {
                BN_mod_mul(result, a_bn, b_bn, module, ctx);
            // }
        end = std::chrono::high_resolution_clock::now();
        eplapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        elapsed_durations_openssl[j - 1] = eplapsed_ns.count();

        start = std::chrono::high_resolution_clock::now();
            // for (int i = 0; i < j; i++) {
                res_p = a_p * b_p;
            // }
        end = std::chrono::high_resolution_clock::now();
        eplapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        elapsed_durations_ntl[j - 1] = eplapsed_ns.count();
    }
    {
        std::ofstream ofs("elapsed_durations.csv");
        ofs << "OpenSSL,NTL\n";
        for (int i = 0; i < run_times; i++) {
            ofs << elapsed_durations_openssl[i] << "," << elapsed_durations_ntl[i] << "\n";
        }
        ofs.flush();
        ofs.close();
        std::cout << "elapsed_durations.csv has been generated." << std::endl;
    }
    // std::cout << "result: " << BN_bn2dec(result) << std::endl;
    BN_free(a_bn);
    BN_free(b_bn);
    BN_free(result);
    BN_free(module);
    BN_CTX_free(ctx);
    return 0;
}