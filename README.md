---

# Milk Sad Mnemonic Generator

This tool is designed to generate BIP-39 mnemonic phrases (24-word seed phrases) based on Unix timestamps, specifically exploring phrases that could be generated through the "Milk Sad" vulnerability (CVE_2023-39910).

---

## What This Tool Does

The Milk Sad Mnemonic Generator creates 24-word BIP-39 mnemonic phrases by leveraging Unix timestamps as an entropy source. It aims to generate all possible seed phrases that might have been produced due to the specific timestamp-based entropy collection flaw associated with the "Milk Sad" vulnerability.

### Key Features:
* Generates **24-word BIP-39 mnemonics**.
* Supports multiple **language wordlists** (English, Spanish, Japanese, French, Italian, Portuguese, Korean, Russian, Ukrainian, Czech, Chinese Simplified, Chinese Traditional).
* Can generate for **specific dates**, **date ranges**, or the **entire 32-bit timestamp range**.
* Automatically **saves progress** and can **resume interrupted generations**.

---

## Installation

You have two options how to install and run on your system:

### Option 1: Use the `install.sh` Script

This script automates the compilation process and handles common dependencies for Debian/Ubuntu-based Linux systems.

1.  **Open your terminal** and navigate to the project's root directory:
    ```bash
    cd MilkSad-Mnemonic-Generator
    ```
2.  **Make the script executable:**
    ```bash
    chmod +x install.sh
    ```
3.  **Run the installation script:**
    ```bash
    ./install.sh
    ```
    The script will check for necessary build tools (like `g++`, `make`, `qmake`, `libssl-dev`, `libqt5widgets5-dev`). If any are missing, it will attempt to install them (prompting for your `sudo` password if needed). After successfully setting up dependencies, it will compile both the CLI and GUI applications and place them directly in the project's root directory.

### Option 2: Manual Compilation

If you prefer to compile the applications yourself, follow these manual steps:

#### **Prerequisites:**
Ensure you have the following development packages installed on your Debian/Ubuntu system:
* `g++` (typically part of the `build-essential` package)
* `make` (also part of `build-essential`)
* `qmake` (provided by `qt5-qmake` or `qtbase5-dev`)
* `libssl-dev` (essential for cryptographic functions used by the tool)
* `libqt5widgets5-dev` (required for Qt GUI development; this package often includes `qtbase5-dev`)

You can install these prerequisites using:
```bash
sudo apt update
sudo apt install build-essential qt5-qmake libssl-dev qtbase5-dev
```

#### **Compile CLI Application:**
1.  Navigate to the CLI source directory:
    ```bash
    cd src/cli
    ```
2.  Compile the application using `g++`:
    ```bash
    g++ -std=gnu++11 -Wall -Wextra -O2 milk_sad_generator.cpp -o milk_sad_generator -lcrypto -lssl
    ```
3.  Move the compiled CLI executable to the project root:
    ```bash
    mv milk_sad_generator ../../
    ```
4.  Return to the project root:
    ```bash
    cd ../..
    ```

#### **Compile GUI Application:**
1.  Navigate to the GUI source directory:
    ```bash
    cd src/gui
    ```
2.  Run `qmake` to generate the Makefile for the Qt project:
    ```bash
    qmake mnemonic_generator.pro
    ```
3.  Compile the GUI application using `make`:
    ```bash
    make
    ```
4.  Move the compiled GUI executable to the project root:
    ```bash
    mv milk_sad_generator_gui ../../
    ```
5.  Return to the project root:
    ```bash
    cd ../..
    ```
6.  **Move the `Pic/` folder**: For the GUI to correctly find its background image you must move the `Pic/` folder from `src/gui/` to the project root:
    ```bash
    mv src/gui/Pic .
    ```

---

## How To Use The Tool

Once installed, you'll find two executables directly in your project's root directory: `./milk_sad_generator` (the Command Line Interface version) and `./milk_sad_generator_gui` (the Graphical User Interface version).

1.  **Start the program:**
    ```bash
    ./milk_sad_generator_gui  # To launch the GUI application
    # OR
    ./milk_sad_generator      # To launch the CLI application
    ```
2.  **Select the mnemonic phrase language** from the options presented (e.g., English, Spanish, Japanese).
3.  **Choose one of the three available generation modes:**

### **Option 1: Single Timestamp (For Testing)**
This mode generates a single 24-word mnemonic phrase for a precise date and time you specify.
* **Example Usage:**
    * Enter date/time: `1970-01-01 00:00:00` for first timestamp. If chosen english it will generate "milk sad ..." mnemonic.
    * **Output:** A single mnemonic phrase will be generated and saved to `mnemonic_[language].txt`.

### **Option 2: Date Range**
This mode generates mnemonics for every timestamp within a specified start and end date range.
* **Example Usage:**
    * Enter date range: `2017-01-01:2018-12-31` (This will generate mnemonics for all timestamps between January 1, 2017, and December 31, 2018, inclusive.)
    * **Output:** All generated mnemonics for the specified range will be saved to `mnemonics_range_[language].txt`.
    * **Progress** is automatically saved every 100,000 timestamps. This feature allows you to safely stop the generation (e.g., by pressing `Ctrl+C`) and resume later without losing progress.

### **Option 3: Full Range**
This mode generates all possible mnemonics across the entire 32-bit Unix timestamp range (from January 1, 1970, to January 19, 2038).
* **WARNING:** The output file generated by this option can be **extremely large (potentially 300-600GB)**. Ensure you have a significant amount of free disk space before starting this mode and test with smaller ranges first.
* **Output:** All mnemonics will be saved to `all_mnemonics_[language].txt`.
* **Progress** is saved periodically. Be aware that this generation process can take days complete, depending on your system's performance.

---

## Output Files

* **Single mnemonic generation:** `mnemonic_[language].txt`
* **Date range generation:** `mnemonics_range_[language].txt`
* **Full range generation:** `all_mnemonics_[language].txt`
* **Progress files (for resuming):** `generation_progress_range.bin` (for Option 2) / `generation_progress_full.bin` (for Option 3)

---

## Important Notes

1.  The tool performs all timestamp calculations using **UTC (Coordinated Universal Time)**.
2.  For large-scale generations, particularly the "Full Range" option, it is crucial to **ensure you have sufficient disk space** available.
3.  You can safely **stop a generation at any time by pressing `Ctrl+C`**. The tool will automatically save its current progress.
4.  Upon successful completion of a generation mode, the corresponding `.bin` progress file will be **automatically deleted**.

---

## Resuming Generation After Pausing

If you had to stop a generation process (e.g., using `Ctrl+C`), you can seamlessly resume from where you left off:

* **For Date Range generation (Option 2):**
    * Simply run the program again and select **Option 2**.
    * Enter the **exact same date range** you used previously.
    * The tool will automatically detect the `generation_progress_range.bin` file and continue generating from the last saved timestamp.

* **For Full Range generation (Option 3):**
    * Run the program again and select **Option 3**.
    * The tool will automatically locate the `generation_progress_full.bin` file and resume the generation from the point it was interrupted.

### **Important Resumption Considerations:**

* **Do NOT delete the `.bin` progress files** (`generation_progress_range.bin` or `generation_progress_full.bin`) if you intend to resume a generation.
* The progress files are **automatically deleted** only when the generation process completes successfully.
* If you wish to **start a generation over from the very beginning** (disregarding previous progress), you must manually delete the corresponding `.bin` progress file *and* rename/delete/move any existing output file (`mnemonics_range_[language].txt` or `all_mnemonics_[language].txt`) from previous runs before starting the program again.
* Be aware that if you start a "Date Range" generation (Option 2) with a different date range, or a "Full Range" generation (Option 3) with the same language where an `all_mnemonics_[language].txt` already exists, the `.bin` progress file will be overwritten, and new mnemonics will be appended to the existing output file.

---

## Example Output

Here's an example of a 24-word BIP-39 mnemonic phrase generated by this tool:

```
vault alarm sad mass witness property virus style good flower rice alpha viable evidence run glare pretty scout evil judge offer refuse another breeze
```
For verification purposes or quick reference, the `tests/` directory in this repository contains examples of generated mnemonic phrases for various languages for **1970-01-01 00:00:00 UTC** timestamp.

---

## Generate Bitcoin Addresses from Mnemonics

To convert the generated mnemonic phrases into Bitcoin addresses (supporting P2PKH, P2SH-P2WPKH, and P2WPKH formats), you can use this tool:

* **z1ph1us/btc-address-generator:** [https://github.com/z1ph1us/btc-address-generator](https://github.com/z1ph1us/btc-address-generator)

## Support
If you found this project useful and would like to encourage me to create more tools like it, consider donating:

- **Bitcoin (BTC):** `bc1qg7xmlsdxfgu3muxherh4eee2ywj3gz8qfgct3s`  
- **Ethereum (ETH):** `0x1B449E1D545bD0dc50f361d96706F6C904553929`  
- **Solana (SOL):** `F776tt1it7vifCzD9icrsby3ujkZdJ8EY9917GUM3skr`  
- **Tron (TRX):** `TJj96B4SukPJSC4M2FyssoxduVyviv9aGr`  
- **Polygon (POL):** `0x1B449E1D545bD0dc50f361d96706F6C904553929`  
- **Monero (XMR):** `48aaDb1g4Ms7PB3WMj6ttbMWuEwe171d6Yjao59uFJR38tHa75nKwPqYoPAYmWZPUhXmDbDvqtE6d2FX3YaF1dVE7zhD9Dt`  
