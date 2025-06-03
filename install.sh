#!/bin/bash

# --- Configuration ---
# Define the final destination for executables and data
# This script will place everything in the current directory (project root)
DEST_DIR="." # Current directory (project root)

# --- Dependency Check and Install Function ---
check_dependency() {
    local cmd="$1"
    local package="$2"
    local apt_update_output_file=$(mktemp) # Create a temporary file for apt update output
    local apt_update_failed=0 # Flag to track if apt update had a non-zero exit

    if ! command -v "$cmd" &> /dev/null; then
        echo "Dependency '$cmd' not found."
        
        # Attempt to install if apt is available
        if command -v apt &> /dev/null; then
            echo "Attempting to install '$package' using apt..."
            
            # --- Perform apt update, capturing output and handling specific errors ---
            echo "Running: sudo apt update (ignoring non-critical repository errors for now)..."
            # We use 'tee' to show output to user AND capture to file
            if sudo apt update 2>&1 | tee "$apt_update_output_file"; then
                echo "apt update completed without critical errors."
            else
                apt_update_failed=1
                # Check for the specific "Release file is not valid yet" message
                if grep -q "Release file for .* is not valid yet" "$apt_update_output_file"; then
                    echo "WARNING: An 'apt update' error occurred:"
                    grep "Release file for .* is not valid yet" "$apt_update_output_file"
                    echo "  This usually means a timestamp issue for a repository (e.g., your system clock vs. server)."
                    echo "  We will *attempt* to install '$package' anyway, but it might not be the latest version."
                    echo "  You may need to fix your repository configuration or system clock later for full updates."
                else
                    echo "CRITICAL ERROR: 'sudo apt update' failed for an unknown reason (not a timestamp issue)."
                    echo "  Please resolve the apt update issues manually, then try again."
                    rm "$apt_update_output_file"
                    exit 1 # Exit for critical apt update failure
                fi
            fi
            
            # --- Now attempt to install the package ---
            echo "Running: sudo apt install -y $package"
            if sudo apt install -y "$package"; then
                echo "Successfully installed '$package'."
            else
                echo "Error: Failed to install '$package' automatically."
                echo "  This could be due to the previous 'apt update' issue or other reasons."
                echo "  Please install it manually: sudo apt update && sudo apt install $package"
                rm "$apt_update_output_file"
                exit 1 # Exit if apt install fails
            fi
        else
            echo "Error: '$cmd' not found and 'apt' package manager not detected."
            echo "Please install '$package' manually using your distribution's package manager."
            exit 1 # Exit if apt is not available
        fi
    fi
    rm "$apt_update_output_file" # Clean up temporary file
}

# --- Main Installation Process ---

echo "--- Milk Sad Generator Installation Script ---"
echo "Starting installation process. This may take a few moments..."

# --- 1. Check for Required Dependencies and Install if Missing ---
echo "Checking for essential build tools and libraries..."
check_dependency "g++" "build-essential"
check_dependency "make" "build-essential"
check_dependency "qmake" "qt5-qmake" # For Qt projects
check_dependency "libssl-dev" "libssl-dev" # For OpenSSL crypto operations
check_dependency "libqt5widgets5-dev" "qtbase5-dev" # For Qt GUI development (includes core, gui, widgets)

echo "All required dependencies are present or have been installed. Proceeding with compilation."
echo ""

# --- 2. Compile CLI Application ---
echo "--- Compiling CLI application (milk_sad_generator) ---"
CLI_DIR="src/cli"

# Clean previous CLI build artifacts
echo "Cleaning previous CLI build artifacts in $CLI_DIR..."
rm -f "$CLI_DIR/milk_sad_generator" "$CLI_DIR/milk_sad_generator.o"

# Navigate to CLI directory and compile
cd "$CLI_DIR" || { echo "Error: Could not enter $CLI_DIR. Aborting."; exit 1; }

# Compile CLI using g++ directly, as it doesn't have a .pro file
g++ -std=gnu++11 -Wall -Wextra -O2 milk_sad_generator.cpp -o milk_sad_generator -lcrypto -lssl || { echo "CLI compilation failed. Aborting."; exit 1; }
echo "CLI compilation successful."

# Return to project root
cd - > /dev/null # 'cd -' returns to the previous directory, > /dev/null suppresses output
echo ""

# --- 3. Compile GUI Application ---
echo "--- Compiling GUI application (milk_sad_generator_gui) ---"
GUI_DIR="src/gui"
GUI_PROJ="mnemonic_generator.pro"

# Clean previous GUI build artifacts
echo "Cleaning previous GUI build artifacts in $GUI_DIR..."
# This will remove all build files specified in your .gitignore for src/gui
cd "$GUI_DIR" || { echo "Error: Could not enter $GUI_DIR. Aborting."; exit 1; }
make clean > /dev/null # Clean with make, suppress output
cd - > /dev/null

# Navigate to GUI directory, run qmake, then make
cd "$GUI_DIR" || { echo "Error: Could not enter $GUI_DIR. Aborting."; exit 1; }
echo "Running qmake in $GUI_DIR..."
qmake "$GUI_PROJ" || { echo "qmake for GUI failed. Aborting."; exit 1; }

echo "Running make in $GUI_DIR..."
make || { echo "GUI compilation failed. Aborting."; exit 1; }
echo "GUI compilation successful."

# Return to project root
cd - > /dev/null
echo ""

# --- 4. Move Executables to Root ---
echo "--- Moving executables to project root ($DEST_DIR) ---"
CLI_EXECUTABLE="src/cli/milk_sad_generator"
GUI_EXECUTABLE="src/gui/milk_sad_generator_gui"

if [ -f "$CLI_EXECUTABLE" ]; then
    mv "$CLI_EXECUTABLE" "$DEST_DIR/" || { echo "Failed to move CLI executable. Aborting."; exit 1; }
    echo "Moved '$CLI_EXECUTABLE' to '$DEST_DIR/'"
else
    echo "Warning: CLI executable '$CLI_EXECUTABLE' not found. It might not have compiled."
fi

if [ -f "$GUI_EXECUTABLE" ]; then
    mv "$GUI_EXECUTABLE" "$DEST_DIR/" || { echo "Failed to move GUI executable. Aborting."; exit 1; }
    echo "Moved '$GUI_EXECUTABLE' to '$DEST_DIR/'"
else
    echo "Warning: GUI executable '$GUI_EXECUTABLE' not found. It might not have compiled."
fi
echo ""

# --- 5. Ensure Data/Asset Folders are Correctly Placed for Executables ---
# If GUI executable is moved to root, it will now look for './Pic/'
# so Pic needs to be moved to root as well.
echo "Handling Pic/ directory for GUI executable..."
if [ -d "src/gui/Pic" ]; then
    if [ ! -d "Pic" ]; then
        echo "  Moving src/gui/Pic/ to project root (./Pic/)..."
        mv src/gui/Pic . || { echo "Error: Failed to move Pic/ to root. Aborting."; exit 1; }
        echo "  Pic/ successfully moved to project root."
    else
        echo "  Pic/ directory already exists at project root."
        # Verify content if user has run this before and Pic/ might be empty
        if [ ! -f "Pic/1_0LOPQwRdahE_ABkF8idXgg.png" ]; then
            echo "  WARNING: Pic/ at root does NOT contain '1_0LOPQwRdahE_ABkF8idXgg.png'."
            echo "  Copying missing image from src/gui/Pic/ to existing Pic/ at root."
            cp -r src/gui/Pic/* Pic/ || { echo "Error: Failed to copy missing Pic/ content. Aborting."; exit 1; }
        fi
    fi
else
    echo "  WARNING: src/gui/Pic/ not found. Ensure Pic/ is correctly placed for GUI."
fi

echo "Handling Wordlist/ directory..."
if [ ! -d "Wordlist" ]; then
    echo "  WARNING: Wordlist/ directory not found at root. Applications may not function correctly."
else
    echo "  Wordlist/ directory already exists at root."
fi
echo ""

# --- 6. Final Cleanup (Optional) ---
echo "--- Performing final cleanup of build directories ---"
# Clean up remaining build artifacts in source directories
cd src/cli && rm -f *.o && cd - > /dev/null
cd src/gui && make clean > /dev/null && cd - > /dev/null

echo "Installation complete!"
echo "You can now run your applications from the project root:"
echo "  ./milk_sad_generator"
echo "  ./milk_sad_generator_gui"
echo ""