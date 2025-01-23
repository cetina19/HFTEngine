set -e

mkdir -p build

BUILD_DIR="build"

EXECUTABLE="HFTEngine"

function echo_info() {
    echo -e "\e[32m[INFO]\e[0m $1"
}

function echo_error() {
    echo -e "\e[31m[ERROR]\e[0m $1" >&2
}

if [ ! -d "$BUILD_DIR" ]; then
    echo_info "Build directory '$BUILD_DIR' does not exist. Creating it..."
    mkdir "$BUILD_DIR"
    
    echo_info "Configuring the project with CMake..."
    cmake -S . -B "$BUILD_DIR"
else
    echo_info "Navigating to the build directory..."
fi

cd "$BUILD_DIR"

if [ ! -f "Makefile" ]; then
    echo_info "Makefile not found. Configuring the project with CMake..."
    cmake ..
fi

echo_info "Compiling the project..."
make

if [ ! -f "$EXECUTABLE" ]; then
    echo_error "Executable '$EXECUTABLE' not found in '$BUILD_DIR'. Compilation might have failed."
    exit 1
fi

echo_info "Running the executable '$EXECUTABLE'..."
./"$EXECUTABLE"

cd ..

echo_info "Script execution completed successfully."
