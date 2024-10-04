#include <iostream>

#include "include/pthash.hpp"
#include "src/util.hpp"  // for functions distinct_keys and check

int main() {
    using namespace pthash;

    /* Generate 10M random 64-bit keys as input data. */
    static const uint64_t num_keys = 70000;
    static const uint64_t seed = 9;
    std::cout << "generating input data..." << std::endl;
    std::vector<uint64_t> keys = distinct_keys<uint64_t>(num_keys, seed);
    assert(keys.size() == num_keys);

    /* Set up a build configuration. */
    build_configuration config;
    config.seed = seed;
    config.lambda = 4;
    config.alpha = 1;
    config.search = pthash_search_type::add_displacement;
    config.minimal_output = true;  // mphf
    config.verbose_output = true;
    config.secondary_sort = true;
    config.avg_partition_size=2000;

    /* Declare the PTHash function. */

    /*
        Caveat:
        when using single_phf, config.dense_partitioning must be set to false;
        when using dense_partitioned_phf, config.dense_partitioning must be set to true.
    */

   typedef single_phf<xxhash128,                       // base hasher
                       skew_bucketer,                        // bucketer type
                       dictionary_dictionary ,                // encoder type
                       true,                                 // minimal
                       pthash_search_type::add_displacement  // additive displacement
                       >
        pthash_type;
    config.dense_partitioning = false;

    /*typedef dense_partitioned_phf<xxhash128,                       // base hasher
                                   table_bucketer<opt_bucketer>,                         // bucketer type
                                   inter_R ,                              // encoder type
                                   false,                                 // minimal
                                   pthash_search_type::add_displacement  // additive displacement
                                   >
         pthash_type;*/
     config.dense_partitioning = true;

    pthash_type f;

    /* Build the function in internal memory. */
    std::cout << "building the function..." << std::endl;
    auto start = clock_type::now();
    auto timings = f.build_in_internal_memory(keys.begin(), keys.size(), config);
    /* Compute and print the number of bits spent per key. */
    double bits_per_key = static_cast<double>(f.num_bits()) / f.num_keys();
    std::cout << "function uses " << bits_per_key << " [bits/key]" << std::endl;

    /* Sanity check! */
    if (check(keys.begin(), f)) std::cout << "EVERYTHING OK!" << std::endl;

    /* Now evaluate f on some keys. */

    /* Serialize the data structure to a file. */
    std::cout << "serializing the function to disk..." << std::endl;
    std::string output_filename("pthash.bin");
    essentials::save(f, output_filename.c_str());

    std::remove(output_filename.c_str());
    return 0;
}