// helpers.h
#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <vector>
#include <cstdint> // For uint32_t, uint8_t
#include <ctime>   // For std::tm

// Forward declarations for OpenSSL types if needed (though usually not necessary in helper declarations)
// struct evp_md_ctx_st; // EVP_MD_CTX is usually a pointer to this struct
// typedef struct evp_md_ctx_st EVP_MD_CTX;

// Declarations of your helper functions
std::string remove_whitespace(const std::string& str);
std::string get_filename_base(const std::string& wordlist);
bool file_exists(const std::string& filename);
std::vector<std::string> load_wordlist(const std::string& filename);
std::vector<uint8_t> sha256(const std::vector<uint8_t>& data);
std::string generate_mnemonic_bip39(uint32_t seed_value, const std::vector<std::string>& wordlist);
void save_progress(const std::string& progress_file, uint32_t last_timestamp);
uint32_t load_progress(const std::string& progress_file);
std::tm parse_datetime(const std::string& datetime_str);
void parse_date_range(const std::string& range_str, std::tm& start_tm, std::tm& end_tm);
uint32_t get_unix_timestamp(int year, int month, int day, int hour, int minute, int second);

#endif // HELPERS_H
