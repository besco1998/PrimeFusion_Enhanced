#!/bin/bash
# CHECKSUM: da39a3ee5e6b4b0d3255bfef95601890afd80709 (Update this when modifying)

# ==============================================================================
# PrimeFusion Enhanced - Universal Dependency Installer
# ==============================================================================
# Role: Sets up the build environment on Debian-based systems (Ubuntu/RPi OS).
# Usage: ./install_dependencies.sh
# ==============================================================================

set -e  # Exit immediately if a command exits with a non-zero status.

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

log() {
    echo -e "${GREEN}[SETUP]${NC} $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 1. System Check
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$NAME
    VER=$VERSION_ID
    log "Detected OS: $OS $VER"
else
    error "Cannot detect OS. This script supports Debian/Ubuntu/Raspbian."
    exit 1
fi

# 2. Package Manager Audit
REQUIRED_PACKAGES=(
    "build-essential"
    "cmake"
    "git"
    "python3"
    "python3-pip"
    "libprotobuf-dev"
    "protobuf-compiler"
    "libcbor-dev"
    "libmsgpack-dev"
    "librapidjson-dev"
)

MISSING_PACKAGES=()

log "Auditing installed packages..."
for pkg in "${REQUIRED_PACKAGES[@]}"; do
    if ! dpkg -s "$pkg" >/dev/null 2>&1; then
        MISSING_PACKAGES+=("$pkg")
    fi
done

if [ ${#MISSING_PACKAGES[@]} -eq 0 ]; then
    log "All system packages are already installed."
else
    log "Missing packages: ${MISSING_PACKAGES[*]}"
    echo "To install, run:"
    echo "sudo apt-get update && sudo apt-get install -y ${MISSING_PACKAGES[*]}"
    # We DO NOT auto-install with sudo to respect user privilege boundaries
    error "Please install missing packages manually and re-run this script."
    exit 1
fi

# 3. Python Environment
log "Checking Python dependencies..."
if [ -f "requirements.txt" ]; then
    pip3 install -r requirements.txt --user
else
    # Create default requirements if missing
    cat <<EOF > requirements.txt
numpy>=1.20
pandas>=1.3
matplotlib>=3.4
scipy>=1.7
EOF
    log "Created requirements.txt"
    pip3 install -r requirements.txt --user
fi

# 4. Directory Structure Check
DIRS=("benchmarks" "results/raw" "results/processed" "tools" "cmake")
for dir in "${DIRS[@]}"; do
    if [ ! -d "../$dir" ]; then
        log "Creating directory: $dir"
        mkdir -p "../$dir"
    fi
done

log "Environment setup complete. You are ready to build."
log "Next step: cd .. && cmake -B build -S ."
