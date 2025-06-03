#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <string>
#include <iomanip>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>
#include <ctime>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <openssl/sha.h>
#include <cctype>

// Configuration
const std::string WORDLIST_DIR = "./Wordlist/";
const std::string OUTPUT_FILE_SINGLE_PREFIX = "mnemonic_";
const std::string OUTPUT_FILE_RANGE_PREFIX = "mnemonics_range_";
const std::string OUTPUT_FILE_FULL_PREFIX = "all_mnemonics_";
const std::string PROGRESS_FILE_RANGE = "generation_progress_range.bin";
const std::string PROGRESS_FILE_FULL = "generation_progress_full.bin";
const size_t MNEMONIC_WORD_COUNT = 24;
const uint64_t REPORT_INTERVAL = 10000000ULL;

// Available wordlists
const std::vector<std::string> WORDLIST_FILES = {
    "english.txt",
    "spanish.txt",
    "french.txt",
    "italian.txt",
    "portuguese.txt",
    "japanese.txt",
    "korean.txt",
    "chinese_simplified.txt",
    "chinese_traditional.txt",
    "russian.txt",
    "ukrainian.txt",
    "czech.txt"
};

// Helper functions
std::string remove_whitespace(const std::string& str) {
    std::string result = str;
    result.erase(std::remove_if(result.begin(), result.end(), ::isspace), result.end());
    return result;
}

std::string get_filename_base(const std::string& wordlist) {
    size_t dot_pos = wordlist.find('.');
    return (dot_pos != std::string::npos) ? wordlist.substr(0, dot_pos) : wordlist;
}

bool file_exists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

std::vector<std::string> load_wordlist(const std::string& filename) {
    std::vector<std::string> wordlist;
    std::ifstream file(WORDLIST_DIR + filename);

    if (!file) {
        std::cerr << "Error: Could not open wordlist file at " << WORDLIST_DIR + filename << std::endl;
        exit(1);
    }

    std::string word;
    while (std::getline(file, word)) {
        word.erase(word.find_last_not_of(" \t\r\n") + 1);
        if (!word.empty()) {
            wordlist.push_back(word);
        }
    }

    if (wordlist.size() != 2048) {
        std::cerr << "Error: Wordlist must contain exactly 2048 words (found " << wordlist.size() << ")" << std::endl;
        exit(1);
    }

    return wordlist;
}

std::vector<uint8_t> sha256(const std::vector<uint8_t>& data) {
    SHA256_CTX sha256_ctx;
    SHA256_Init(&sha256_ctx);
    SHA256_Update(&sha256_ctx, data.data(), data.size());
    std::vector<uint8_t> hash(SHA256_DIGEST_LENGTH);
    SHA256_Final(hash.data(), &sha256_ctx);
    return hash;
}

std::string generate_mnemonic_bip39(uint32_t seed_value, const std::vector<std::string>& wordlist) {
    std::mt19937 engine(seed_value);
    std::vector<uint8_t> entropy(32);
    std::uniform_int_distribution<uint16_t> distribution(0, std::numeric_limits<uint8_t>::max());

    for (size_t i = 0; i < 32; ++i) {
        entropy[i] = static_cast<uint8_t>(distribution(engine));
    }

    std::vector<uint8_t> hash = sha256(entropy);
    const size_t CHECKSUM_LENGTH_BITS = entropy.size() * 8 / 32;
    std::vector<uint8_t> combined_data = entropy;
    combined_data.insert(combined_data.end(), hash.begin(), hash.begin() + (CHECKSUM_LENGTH_BITS + 7) / 8);

    std::string mnemonic;
    size_t combined_data_bit_index = 0;
    for (size_t i = 0; i < MNEMONIC_WORD_COUNT; ++i) {
        uint16_t word_index = 0;
        for (int bit = 10; bit >= 0; --bit) {
            size_t byte_index = combined_data_bit_index / 8;
            int bit_in_byte = 7 - (combined_data_bit_index % 8);

            if (byte_index < combined_data.size()) {
                if ((combined_data[byte_index] >> bit_in_byte) & 1) {
                    word_index |= (1 << bit);
                }
                combined_data_bit_index++;
            }
        }
        if (!mnemonic.empty()) mnemonic += " ";
        mnemonic += wordlist[word_index];
    }
    return mnemonic;
}

void save_progress(const std::string& progress_file, uint32_t last_timestamp) {
    std::ofstream out(progress_file, std::ios::binary);
    if (out.is_open()) {
        out.write(reinterpret_cast<const char*>(&last_timestamp), sizeof(last_timestamp));
    }
    else {
        std::cerr << "Error: Could not write to progress file " << progress_file << std::endl;
    }
}

uint32_t load_progress(const std::string& progress_file) {
    if (!file_exists(progress_file)) return 0;
    std::ifstream in(progress_file, std::ios::binary);
    uint32_t last_timestamp;
    if (in.is_open()) {
        in.read(reinterpret_cast<char*>(&last_timestamp), sizeof(last_timestamp));
        if (!in.good()) {
            return 0;
        }
    }
    else {
        std::cerr << "Error: Could not read from progress file " << progress_file << std::endl;
        return 0;
    }
    return last_timestamp;
}

void signal_handler(int signum) {
    std::cout << "\nInterrupt received. Saving progress..." << std::endl;
    exit(signum);
}

std::tm parse_datetime(const std::string& datetime_str) {
    std::tm tm = {};
    std::stringstream ss(datetime_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (ss.fail()) {
        throw std::runtime_error("Invalid date/time format. Use YYYY-MM-DD HH:MM:SS");
    }
    return tm;
}

void parse_date_range(const std::string& range_str, std::tm& start_tm, std::tm& end_tm) {
    std::string clean_str = remove_whitespace(range_str);
    size_t colon_pos = clean_str.find(':');
    if (colon_pos == std::string::npos) {
        throw std::runtime_error("Invalid format. Use YYYY-MM-DD:YYYY-MM-DD");
    }

    std::string start_str = clean_str.substr(0, colon_pos);
    std::string end_str = clean_str.substr(colon_pos + 1);

    std::stringstream ss_start(start_str);
    ss_start >> std::get_time(&start_tm, "%Y-%m-%d");
    if (ss_start.fail()) {
        throw std::runtime_error("Invalid start date format.");
    }

    std::stringstream ss_end(end_str);
    ss_end >> std::get_time(&end_tm, "%Y-%m-%d");
    if (ss_end.fail()) {
        throw std::runtime_error("Invalid end date format.");
    }
}

uint32_t get_unix_timestamp(int year, int month, int day, int hour, int minute, int second) {
    std::tm t{};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = second;
    t.tm_isdst = 0;

    #ifdef _WIN32
        _putenv("TZ=UTC");
        _tzset();
    #else
        setenv("TZ", "UTC", 1);
        tzset();
    #endif

    std::time_t timestamp = std::mktime(&t);
    if (timestamp == -1) {
        throw std::runtime_error("Invalid date/time for timestamp conversion");
    }
    return static_cast<uint32_t>(timestamp);
}

int select_wordlist() {
    std::cout << "Available wordlists:\n";
    for (size_t i = 0; i < WORDLIST_FILES.size(); ++i) {
        std::cout << i + 1 << ". " << WORDLIST_FILES[i] << "\n";
    }
    std::cout << "Select wordlist (1-" << WORDLIST_FILES.size() << "): ";
    
    int choice;
    while (true) {
        std::cin >> choice;
        if (std::cin.fail() || choice < 1 || choice > static_cast<int>(WORDLIST_FILES.size())) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid choice. Please enter a number between 1 and " << WORDLIST_FILES.size() << ": ";
        } else {
            break;
        }
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return choice - 1;
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    std::cout << "Milk Sad Mnemonic Generator" << std::endl;
    std::cout << "------------------------" << std::endl;
    std::cout << "Developed by z1ph1us" << std::endl;
    std::cout << "------------------------" << std::endl;
    
    int wordlist_choice = select_wordlist();
    std::string wordlist_name = WORDLIST_FILES[wordlist_choice];
    std::string wordlist_base = get_filename_base(wordlist_name);
    std::cout << "Loading " << wordlist_name << " wordlist..." << std::endl;
    auto wordlist = load_wordlist(wordlist_name);

    std::cout << "Options:\n";
    std::cout << "1. Generate mnemonic for a specific date/time. (Mostly for testing purposes)\n";
    std::cout << "2. Generate mnemonics for a date range.\n";
    std::cout << "3. Generate mnemonics for the full Unix timestamp range (IMPORTANT: If you choose this - the output file can reach 300-600GB, make sure that you have enough storage or use option 2).\n";
    std::cout << "Type your choice (1, 2, or 3) and press Enter: ";

    int choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (choice < 1 || choice > 3) {
        std::cerr << "Error: Invalid choice." << std::endl;
        return 1;
    }

    if (choice == 1) {
        std::string datetime_str;
        std::cout << "Enter the date/time (YYYY-MM-DD HH:MM:SS)(Use 1970-01-01 00:00:00 for 0 value): ";
        std::getline(std::cin, datetime_str);

        std::tm tm;
        try {
            tm = parse_datetime(datetime_str);
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
        
        uint32_t timestamp = get_unix_timestamp(
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec
        );
        
        std::string output_file = OUTPUT_FILE_SINGLE_PREFIX + wordlist_base + ".txt";
        std::ofstream outfile(output_file);
        if (outfile.is_open()) {
            std::string mnemonic = generate_mnemonic_bip39(timestamp, wordlist);
            outfile << mnemonic << std::endl;
            outfile.close();
            std::cout << "Mnemonic for " << datetime_str << " (timestamp " << timestamp << "): " << mnemonic << std::endl;
            std::cout << "Saved to " << output_file << std::endl;
        }
        else {
            std::cerr << "Error opening output file " << output_file << std::endl;
            return 1;
        }
    }
    else if (choice == 2) {
        std::string date_range_str;
        std::cout << "Enter the date range in format YYYY-MM-DD:YYYY-MM-DD (Start:End): ";
        std::getline(std::cin, date_range_str);

        std::tm start_tm = {}, end_tm = {};
        try {
            parse_date_range(date_range_str, start_tm, end_tm);
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }

        uint32_t start_timestamp = get_unix_timestamp(
            start_tm.tm_year + 1900, start_tm.tm_mon + 1, start_tm.tm_mday, 0, 0, 0);
        uint32_t end_timestamp = get_unix_timestamp(
            end_tm.tm_year + 1900, end_tm.tm_mon + 1, end_tm.tm_mday, 0, 0, 0);

        if (start_timestamp > end_timestamp) {
            std::cerr << "Error: Start date must be before or equal to end date." << std::endl;
            return 1;
        }

        std::string output_file = OUTPUT_FILE_RANGE_PREFIX + wordlist_base + ".txt";
        std::cout << "Generating mnemonics for timestamps between "
                  << start_timestamp << " and " << end_timestamp << std::endl;
        std::cout << "Output file: " << output_file << std::endl;

        std::ofstream outfile(output_file, std::ios::app);
        if (!outfile) {
            std::cerr << "Failed to open output file " << output_file << "!" << std::endl;
            return 1;
        }

        uint32_t current_timestamp = load_progress(PROGRESS_FILE_RANGE);
        if (current_timestamp < start_timestamp) {
            current_timestamp = start_timestamp;
        }

        for (; current_timestamp <= end_timestamp; ++current_timestamp) {
            outfile << generate_mnemonic_bip39(current_timestamp, wordlist) << "\n";

            if (current_timestamp % 100000 == 0 || current_timestamp == end_timestamp) {
                save_progress(PROGRESS_FILE_RANGE, current_timestamp + 1);
            }

            if (current_timestamp % REPORT_INTERVAL == 0) {
                double progress = static_cast<double>(current_timestamp - start_timestamp) / (end_timestamp - start_timestamp);
                std::cout << "Progress: " << std::fixed << std::setprecision(2) << (progress * 100.0) << "% complete\n";
            }
        }

        unlink(PROGRESS_FILE_RANGE.c_str());
        std::cout << "Generation complete. All mnemonics saved to " << output_file << "\n";
    }
    else if (choice == 3) {
        std::string output_file = OUTPUT_FILE_FULL_PREFIX + wordlist_base + ".txt";
        std::cout << "Generating mnemonics for the full 32-bit Unix timestamp range." << std::endl;
        std::cout << "Output file: " << output_file << std::endl;

        std::ofstream outfile(output_file, std::ios::app);
        if (!outfile) {
            std::cerr << "Failed to open output file " << output_file << "!" << std::endl;
            return 1;
        }

        uint32_t start_timestamp = load_progress(PROGRESS_FILE_FULL);
        uint32_t end_timestamp = std::numeric_limits<uint32_t>::max();

        for (uint32_t current_timestamp = start_timestamp; current_timestamp <= end_timestamp; ++current_timestamp) {
            outfile << generate_mnemonic_bip39(current_timestamp, wordlist) << "\n";

            if (current_timestamp % 100000 == 0 || current_timestamp == end_timestamp) {
                save_progress(PROGRESS_FILE_FULL, current_timestamp + 1);
            }

            if (current_timestamp % REPORT_INTERVAL == 0) {
                double progress = static_cast<double>(current_timestamp - start_timestamp) / (static_cast<double>(end_timestamp - start_timestamp));
                std::cout << "Progress: " << std::fixed << std::setprecision(2) << (progress * 100.0) << "% complete\n";
            }
        }

        unlink(PROGRESS_FILE_FULL.c_str());
        std::cout << "Generation complete. All mnemonics saved to " << output_file << "\n";
    }

    return 0;
}