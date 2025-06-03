#include "helpers.h"
#include "constants.h"
#include "WorkerThread.h"
#include <QFile>
#include <QTextStream>
#include <QDebug> // For qWarning, qInfo if you want to use them

// Include necessary headers for helper functions used in WorkerThread
#include <random>
#include <vector>
#include <string>
#include <iomanip>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h> // For unlink
#include <sstream>
#include <ctime>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <openssl/sha.h>
#include <cctype>
#include <openssl/evp.h> // Include for EVP functions

WorkerThread::WorkerThread(QObject *parent) : QThread(parent), m_stopRequested(false) {}

void WorkerThread::setup(GenerationMode mode, const std::vector<std::string>& wordlist,
                         const std::string& wordlist_base, uint32_t start_ts, uint32_t end_ts) {
    m_mode = mode;
    m_wordlist = wordlist;
    m_wordlist_base = wordlist_base;
    m_start_timestamp = start_ts;
    m_end_timestamp = end_ts;
    m_stopRequested = false;
}

void WorkerThread::stop() {
    QMutexLocker locker(&m_mutex);
    m_stopRequested = true;
}

void WorkerThread::run() {
    try {
        if (m_mode == SINGLE) {
            emit statusUpdated("Generating single mnemonic...");
            QString output_file = QString::fromStdString(OUTPUT_FILE_SINGLE_PREFIX + m_wordlist_base + ".txt");
            QFile file(output_file);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                emit errorOccurred("Failed to open output file " + output_file);
                return;
            }

            QTextStream out(&file);
            std::string mnemonic = generate_mnemonic_bip39(m_start_timestamp, m_wordlist);
            out << QString::fromStdString(mnemonic) << "\n";
            file.close();

            emit singleMnemonicGenerated(QString::fromStdString(mnemonic));
            emit generationComplete("Mnemonic saved to " + output_file);
        } else if (m_mode == RANGE) {
            emit statusUpdated("Generating mnemonics for date range...");
            QString output_file = QString::fromStdString(OUTPUT_FILE_RANGE_PREFIX + m_wordlist_base + ".txt");
            QFile file(output_file);
            if (!file.open(QIODevice::Append | QIODevice::Text)) {
                emit errorOccurred("Failed to open output file " + output_file);
                return;
            }

            QTextStream out(&file);
            uint32_t current_timestamp = load_progress(PROGRESS_FILE_RANGE);
            if (current_timestamp < m_start_timestamp) {
                current_timestamp = m_start_timestamp;
            }

            for (; current_timestamp <= m_end_timestamp; ++current_timestamp) {
                {
                    QMutexLocker locker(&m_mutex);
                    if (m_stopRequested) break;
                }

                out << QString::fromStdString(generate_mnemonic_bip39(current_timestamp, m_wordlist)) << "\n";

                if (current_timestamp % 100000 == 0 || current_timestamp == m_end_timestamp) {
                    save_progress(PROGRESS_FILE_RANGE, current_timestamp + 1);
                }
            }

            file.close();
            if (!m_stopRequested) {
                unlink(PROGRESS_FILE_RANGE.c_str());
                emit generationComplete("Generation complete. All mnemonics saved to " + output_file);
            } else {
                emit generationComplete("Generation stopped. Progress saved.");
            }
        } else if (m_mode == FULL) {
            emit statusUpdated("Generating full range mnemonics...");
            QString output_file = QString::fromStdString(OUTPUT_FILE_FULL_PREFIX + m_wordlist_base + ".txt");
            QFile file(output_file);
            if (!file.open(QIODevice::Append | QIODevice::Text)) {
                emit errorOccurred("Failed to open output file " + output_file);
                return;
            }

            QTextStream out(&file);
            uint32_t start_timestamp = load_progress(PROGRESS_FILE_FULL);
            uint32_t end_timestamp = std::numeric_limits<uint32_t>::max();

            for (uint32_t current_timestamp_loop = start_timestamp; current_timestamp_loop <= end_timestamp; ++current_timestamp_loop) {
                {
                    QMutexLocker locker(&m_mutex);
                    if (m_stopRequested) break;
                }

                out << QString::fromStdString(generate_mnemonic_bip39(current_timestamp_loop, m_wordlist)) << "\n";

                if (current_timestamp_loop % 100000 == 0 || current_timestamp_loop == end_timestamp) {
                    save_progress(PROGRESS_FILE_FULL, current_timestamp_loop + 1);
                }
            }

            file.close();
            if (!m_stopRequested) { // If it completed without being stopped
                unlink(PROGRESS_FILE_FULL.c_str());
                emit generationComplete("Generation complete. All mnemonics saved to " + output_file);
            } else {
                emit generationComplete("Generation stopped. Progress saved.");
            }
        }
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Error: ") + e.what());
    }
}
