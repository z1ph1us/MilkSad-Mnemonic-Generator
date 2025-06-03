// constants.h
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>
#include <vector>
#include <cstdint> // For uint64_t and size_t (which is often related to std::size_t)

// Configuration - GLOBAL CONSTANTS
// These are now just 'const', not 'inline const', as they are in a header.
// By being 'const', they have internal linkage *within each compilation unit*,
// meaning each .cpp file gets its own copy. This prevents linker errors.
const std::string WORDLIST_DIR = "./Wordlist/";
const std::string OUTPUT_FILE_SINGLE_PREFIX = "mnemonic_";
const std::string OUTPUT_FILE_RANGE_PREFIX = "mnemonics_range_";
const std::string OUTPUT_FILE_FULL_PREFIX = "all_mnemonics_";
const std::string PROGRESS_FILE_RANGE = "generation_progress_range.bin";
const std::string PROGRESS_FILE_FULL = "generation_progress_full.bin";
const size_t MNEMONIC_WORD_COUNT = 24;
const uint64_t REPORT_INTERVAL = 10000000ULL;

// Available wordlists - GLOBAL CONSTANT
const std::vector<std::string> WORDLIST_FILES = {
    "english.txt", "spanish.txt", "french.txt", "italian.txt",
    "portuguese.txt", "japanese.txt", "korean.txt", "chinese_simplified.txt",
    "chinese_traditional.txt", "russian.txt", "ukrainian.txt", "czech.txt"
};

#endif // CONSTANTS_H
