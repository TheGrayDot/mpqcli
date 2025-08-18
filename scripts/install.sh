#!/usr/bin/env bash

set -euo pipefail

REPO="thegraydot/mpqcli"
BASE_URL="https://github.com/${REPO}/releases"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1"
    exit 1
}

# Detect OS and architecture
detect_platform() {
    local os arch lib

    # Detect OS
    case "$(uname -s)" in
        Linux*)     os="linux";;
        *)          error "Unsupported operating system: $(uname -s)";;
    esac

    # Detect architecture
    case "$(uname -m)" in
        x86_64|amd64)   arch="amd64";;
        aarch64|arm64)  arch="arm64";;
        *)              error "Unsupported architecture: $(uname -m)";;
    esac

    # Determine if we should use glibc or musl
    # Priority: Try glibc first (if available), fallback to musl

    # Check if we have ldd command
    if command -v ldd >/dev/null 2>&1; then
        # Check for a musl-based system (like Alpine)
        if ldd --version 2>&1 | grep -q musl; then
            lib="musl"
        # Check for a glibc-based system (like Ubuntu, Debian etc.)
        elif ldd --version 2>&1 | grep -q "GNU\|GLIBC"; then
            lib="glibc"
        fi
    # If ldd is not available, check for common glibc files
    elif [[ -f /lib/x86_64-linux-gnu/libc.so.6 ]] || \
       [[ -f /lib64/libc.so.6 ]] || \
       [[ -f /usr/lib/libc.so.6 ]]; then
        lib="glibc"
    else
        # If ldd is not available or cannot find glibc
        # default to musl for compatibility
        lib="musl"
    fi

    echo "${os}-${arch}-${lib}"
}

# Download file
download_file() {
    local version="$1"
    local platform="$2"
    local filename="mpqcli-${platform}"
    local download_url="${BASE_URL}/download/v${version}/${filename}"
    
    # Try to download the preferred binary
    info "Downloading from: $download_url"

    if command -v curl >/dev/null 2>&1; then
        if ! curl -fsSL "$download_url" -o "mpqcli"; then
            return 1
        fi
    elif command -v wget >/dev/null 2>&1; then
        if ! wget -q "$download_url" -O "mpqcli"; then
            return 1
        fi
    else
        error "Neither curl nor wget found. Please install one of them."
    fi
    
    error "Failed to download binary for platform: $platform"
}

# Get latest release version
get_latest_version() {
    local version
    version=$(curl -fsSL "https://api.github.com/repos/${REPO}/releases/latest" \
        | grep '"tag_name":' \
        | sed -E 's/.*"v([^"]+)".*/\1/')

    if [[ -z "$version" ]]; then
        error "Failed to get latest version"
    fi

    echo "$version"
}

# Main installation function
install() {
    local version="${1:-}"
    local install_dir="${INSTALL_DIR:-/usr/local/bin}"

    # Get version if not specified
    if [[ -z "$version" ]]; then
        info "Fetching latest version..."
        version=$(get_latest_version)
    fi

    info "Installing mpqcli ${version}"

    # Detect platform
    local platform
    platform=$(detect_platform)
    
    # Show appropriate message based on detected platform
    if [[ "$platform" == *"-glibc" ]]; then
        info "System appears compatible with GLIBC binaries"
    else
        info "Using MUSL binary for better compatibility"
    fi
    
    info "Detected platform: ${platform}"

    # Download binar
    download_file "$version" "$platform"

    # Ensure binary is executable
    if [[ -f "mpqcli" ]]; then
        info "Making binary executable..."
        chmod +x "mpqcli"
    else
        error "Binary mpqcli not found after download"
    fi

    # Install binary
    info "Installing to $install_dir"
    warn "You may need to enter your password..."
    sudo mv "mpqcli" "$install_dir/"

    # Verify installation
    if "${install_dir}/mpqcli" version >/dev/null 2>&1; then
        info "Installation successful!"
        info "Binary installed to: ${install_dir}/mpqcli"
        
        # Check if install_dir is in PATH
        if ! echo "$PATH" | grep -q "$install_dir"; then
            warn "${install_dir} is not in your PATH"
            warn "Add it to your PATH by adding this to your shell configuration:"
            warn "  export PATH=\"${install_dir}:\$PATH\""
        fi
    else
        error "Installation verification failed"
    fi
}

# Show help
show_help() {
    cat << EOF
Install script for mpqcli

USAGE:
    $0 [OPTIONS] [VERSION]

OPTIONS:
    -h, --help          Show this help message
    -d, --dir DIR       Install directory (default: \$HOME/.local/bin)
    -t, --tag TAG       Install specific release tag

ENVIRONMENT VARIABLES:
    INSTALL_DIR         Override default install directory

EXAMPLES:
    # Install latest version
    $0

    # Install specific version
    $0 0.8.0

    # Install to custom directory
    INSTALL_DIR=$HOME/.local/bin $0

    # Install specific tag
    $0 --tag v0.8.0
EOF
}

# Parse command line arguments
main() {
    local version=""
    local tag=""

    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -d|--dir)
                INSTALL_DIR="$2"
                shift 2
                ;;
            -t|--tag)
                tag="$2"
                shift 2
                ;;
            -*)
                error "Unknown option: $1"
                ;;
            *)
                version="$1"
                shift
                ;;
        esac
    done

    # Extract version from tag if provided
    if [[ -n "$tag" ]]; then
        version="${tag#v}"
    fi

    install "$version"
}

# Run main function
main "$@"
