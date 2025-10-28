#!/bin/bash
#
# Script to check or format C/C++ source files with clang-format
#
# Usage:
#   ./format-code.sh              - Format all C/C++ files in src/ and tests/
#   ./format-code.sh check        - Check formatting without modifying files
#   ./format-code.sh <file>       - Format a specific file
#   ./format-code.sh check <file> - Check a specific file
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if clang-format is installed
if ! command -v clang-format &> /dev/null; then
    echo -e "${RED}Error: clang-format is not installed${NC}"
    echo "Please install it with: sudo apt-get install clang-format"
    exit 1
fi

MODE="format"
TARGET=""

# Show help
show_help() {
    echo "Usage: $0 [check] [file]"
    echo ""
    echo "Format or check C/C++ source files with clang-format"
    echo ""
    echo "Examples:"
    echo "  $0              - Format all C/C++ files in src/ and tests/"
    echo "  $0 check        - Check formatting without modifying files"
    echo "  $0 <file>       - Format a specific file"
    echo "  $0 check <file> - Check a specific file"
    exit 0
}

# Parse arguments
if [ $# -eq 0 ]; then
    MODE="format"
elif [ "$1" = "--help" ] || [ "$1" = "-h" ] || [ "$1" == "help" ]; then
    show_help
elif [ "$1" = "check" ]; then
    MODE="check"
    if [ $# -eq 2 ]; then
        TARGET="$2"
    fi
elif [ $# -eq 1 ]; then
    TARGET="$1"
else
    show_help
fi

# Get list of files to process
if [ -n "$TARGET" ]; then
    if [ ! -f "$TARGET" ]; then
        echo -e "${RED}Error: File '$TARGET' not found${NC}"
        exit 1
    fi
    FILES="$TARGET"
else
    # Find all C/C++ files in src/ and tests/
    FILES=$(find src tests -type f \( -name "*.c" -o -name "*.h" -o -name "*.cpp" -o -name "*.hpp" \) 2>/dev/null | sort)
    
    if [ -z "$FILES" ]; then
        echo -e "${YELLOW}No C/C++ files found to format${NC}"
        exit 0
    fi
fi

FAILED=0
TOTAL=0
FORMATTED=0

if [ "$MODE" = "check" ]; then
    echo -e "${YELLOW}Checking code formatting...${NC}"
    echo ""
    
    for file in $FILES; do
        TOTAL=$((TOTAL + 1))
        if ! clang-format --dry-run --Werror "$file" 2>&1 > /dev/null; then
            echo -e "${RED}✗${NC} $file needs formatting"
            FAILED=$((FAILED + 1))
        else
            echo -e "${GREEN}✓${NC} $file"
        fi
    done
    
    echo ""
    echo "================================"
    echo "Checked $TOTAL files"
    
    if [ $FAILED -eq 0 ]; then
        echo -e "${GREEN}All files are properly formatted!${NC}"
        exit 0
    else
        echo -e "${RED}$FAILED files need formatting${NC}"
        echo ""
        echo "Run './format-code.sh' to format all files"
        echo "Or run './format-code.sh <file>' to format specific files"
        exit 1
    fi
else
    echo -e "${YELLOW}Formatting code...${NC}"
    echo ""
    
    for file in $FILES; do
        TOTAL=$((TOTAL + 1))
        
        # Check if file needs formatting
        if ! clang-format --dry-run --Werror "$file" 2>&1 > /dev/null; then
            echo -e "${YELLOW}→${NC} Formatting $file"
            clang-format -i "$file"
            FORMATTED=$((FORMATTED + 1))
        else
            echo -e "${GREEN}✓${NC} $file (already formatted)"
        fi
    done
    
    echo ""
    echo "================================"
    echo "Processed $TOTAL files"
    
    if [ $FORMATTED -eq 0 ]; then
        echo -e "${GREEN}All files were already properly formatted!${NC}"
    else
        echo -e "${GREEN}Formatted $FORMATTED files${NC}"
    fi
fi
