#!/bin/bash

# Script to run pytest tests under Valgrind for comprehensive memory leak detection
# This wraps the mpqcli binary with Valgrind during test execution

set -e

SCRIPT_DIR="$(dirname "$(readlink -fm "$0")")"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default values
USE_DOCKER=false
BUILD_DIR="build"
VALGRIND_LOG_DIR="$PROJECT_DIR/valgrind_logs"

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--docker)
            USE_DOCKER=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Run pytest tests with Valgrind memory leak detection"
            echo ""
            echo "Options:"
            echo "  -d, --docker    Run in Docker container"
            echo "  -h, --help      Show this help message"
            echo ""
            echo "This script creates a wrapper that runs mpqcli under Valgrind"
            echo "during pytest execution, providing comprehensive memory leak detection."
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# Create Valgrind log directory
mkdir -p "$VALGRIND_LOG_DIR"

if [ "$USE_DOCKER" = true ]; then
    echo -e "${GREEN}Running tests with Valgrind in Docker...${NC}"

    # Build Docker image with current user's UID/GID
    echo -e "${YELLOW}Building Docker image...${NC}"
    docker build \
        --build-arg USER_ID="$(id -u)"\
        --build-arg GROUP_ID="$(id -g)" \
        -t mpqcli-valgrind \
        -f "$PROJECT_DIR/Dockerfile.valgrind" \
        "$PROJECT_DIR"

    # Run tests in Docker with Valgrind wrapper
    echo -e "${YELLOW}Running pytest with Valgrind...${NC}"
    docker run --rm -v "$PROJECT_DIR":/mpqcli mpqcli-valgrind bash -c "
        # Create Valgrind wrapper script
        cat > /tmp/mpqcli_wrapper.sh << 'EOF'
#!/bin/bash
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --log-file=/mpqcli/valgrind_logs/valgrind_%p.log \
         /mpqcli/build/bin/mpqcli \"\$@\"
EOF
        chmod +x /tmp/mpqcli_wrapper.sh

        # Run tests with wrapper
        cd /mpqcli
        . test/venv/bin/activate
        MPQCLI_BIN=/tmp/mpqcli_wrapper.sh python3 -m pytest test -s
    "
else
    echo -e "${GREEN}Running tests with Valgrind locally...${NC}"

    # Check if Valgrind is installed
    if ! command -v valgrind &> /dev/null; then
        echo -e "${RED}Error: Valgrind is not installed${NC}"
        echo "Install it with: sudo apt-get install valgrind"
        exit 1
    fi

    # Check if binary exists
    if [ ! -f "$PROJECT_DIR/$BUILD_DIR/bin/mpqcli" ]; then
        echo -e "${YELLOW}Binary not found. Building with debug symbols...${NC}"
        cd "$PROJECT_DIR"
        cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Debug -DBUILD_MPQCLI=ON
        cmake --build "$BUILD_DIR"
    fi

    # Check if Python venv exists
    if [ ! -d "$PROJECT_DIR/test/venv" ]; then
        echo -e "${YELLOW}Setting up Python test environment...${NC}"
        python3 -m venv "$PROJECT_DIR/test/venv"
        . "$PROJECT_DIR/test/venv/bin/activate"
        pip3 install -r "$PROJECT_DIR/test/requirements.txt"
    else
        . "$PROJECT_DIR/test/venv/bin/activate"
    fi

    # Create Valgrind wrapper script
    WRAPPER_SCRIPT="/tmp/mpqcli_valgrind_wrapper_$$.sh"
    cat > "$WRAPPER_SCRIPT" << EOF
#!/bin/bash
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --log-file=$VALGRIND_LOG_DIR/valgrind_\$\$.log \
         "$PROJECT_DIR/$BUILD_DIR/bin/mpqcli" "\$@"
EOF
    chmod +x "$WRAPPER_SCRIPT"

    # Run tests with wrapper
    echo -e "${YELLOW}Running pytest with Valgrind wrapper...${NC}"
    cd "$PROJECT_DIR"
    MPQCLI_BIN="$WRAPPER_SCRIPT" python3 -m pytest test -s

    # Cleanup wrapper
    rm -f "$WRAPPER_SCRIPT"
fi

echo -e "${GREEN}Tests complete!${NC}"
echo -e "${YELLOW}Valgrind logs saved to: $VALGRIND_LOG_DIR${NC}"
echo ""
echo -e "${YELLOW}Analyzing results...${NC}"

# Check for memory leaks in logs
if grep -q "definitely lost" "$VALGRIND_LOG_DIR"/*.log 2>/dev/null; then
    echo -e "${RED}Memory leaks detected! Check logs in $VALGRIND_LOG_DIR${NC}"
    grep -H "definitely lost" "$VALGRIND_LOG_DIR"/*.log | head -20
    exit 1
else
    echo -e "${GREEN}No definite memory leaks detected!${NC}"
fi
