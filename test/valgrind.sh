#!/bin/bash

# Valgrind helper script for midiplay
# Runs valgrind with appropriate options to focus on application code leaks

# Default values
OUTPUT_FILE="valgrind_report.txt"
# Use XDG config directory for global suppression file, fallback to local
if [[ -f "${HOME}/.config/valgrind/default.supp" ]]; then
    SUPPRESSION_FILE="${HOME}/.config/valgrind/default.supp"
else
    SUPPRESSION_FILE="$(dirname "$0")/valgrind.supp"
fi
EXECUTABLE="../play"
SHOW_REACHABLE=false

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Usage function
usage() {
    cat << EOF
Usage: $0 [OPTIONS] [-- PROGRAM_ARGS]

Run valgrind on midiplay with filtering for third-party library leaks.

OPTIONS:
    -o, --output FILE       Output file (default: valgrind_report.txt)
    -s, --suppressions FILE Suppression file (default: ./valgrind.supp)
    -e, --executable PATH   Executable to test (default: ../play)
    -r, --show-reachable    Include reachable memory in report
    -g, --gen-suppressions  Generate suppressions for new issues
    -h, --help              Show this help message

PROGRAM_ARGS:
    Arguments to pass to the program (e.g., -s with_intro)

EXAMPLES:
    $0 -- -s with_intro
    $0 -o memory_leak.txt -- -s simple
    $0 -g -- -s ritardando
    $0 -r -o full_report.txt -- -s dc_al_fine -n2

EOF
    exit 1
}

# Parse command line arguments
GEN_SUPPRESSIONS=false
PROGRAM_ARGS=()

while [[ $# -gt 0 ]]; do
    case $1 in
        -o|--output)
            OUTPUT_FILE="$2"
            shift 2
            ;;
        -s|--suppressions)
            SUPPRESSION_FILE="$2"
            shift 2
            ;;
        -e|--executable)
            EXECUTABLE="$2"
            shift 2
            ;;
        -r|--show-reachable)
            SHOW_REACHABLE=true
            shift
            ;;
        -g|--gen-suppressions)
            GEN_SUPPRESSIONS=true
            shift
            ;;
        -h|--help)
            usage
            ;;
        --)
            shift
            PROGRAM_ARGS=("$@")
            break
            ;;
        *)
            echo -e "${RED}Error: Unknown option: $1${NC}"
            usage
            ;;
    esac
done

# Check if executable exists
if [[ ! -f "$EXECUTABLE" ]]; then
    echo -e "${RED}Error: Executable not found: $EXECUTABLE${NC}"
    exit 1
fi

# Check if suppression file exists
if [[ ! -f "$SUPPRESSION_FILE" ]]; then
    echo -e "${YELLOW}Warning: Suppression file not found: $SUPPRESSION_FILE${NC}"
    echo -e "${YELLOW}Continuing without suppressions...${NC}"
    SUPPRESSION_FILE=""
fi

# Build valgrind command
VALGRIND_CMD=(
    valgrind
    --leak-check=full
    --track-origins=yes
    --verbose
    --log-file="$OUTPUT_FILE"
)

# Add leak kinds based on show-reachable flag
if [[ "$SHOW_REACHABLE" == true ]]; then
    VALGRIND_CMD+=(--show-leak-kinds=all)
else
    VALGRIND_CMD+=(--show-leak-kinds=definite,possible)
fi

# Add suppression file if it exists
if [[ -n "$SUPPRESSION_FILE" ]]; then
    VALGRIND_CMD+=(--suppressions="$SUPPRESSION_FILE")
fi

# Add gen-suppressions if requested
if [[ "$GEN_SUPPRESSIONS" == true ]]; then
    VALGRIND_CMD+=(--gen-suppressions=all)
fi

# Add executable and its arguments
VALGRIND_CMD+=("$EXECUTABLE" "${PROGRAM_ARGS[@]}")

# Display command being run
echo -e "${GREEN}Running valgrind...${NC}"
echo -e "${YELLOW}Command: ${VALGRIND_CMD[*]}${NC}"
echo -e "${YELLOW}Output will be written to: $OUTPUT_FILE${NC}"
echo ""

# Run valgrind
"${VALGRIND_CMD[@]}"

# Check exit status
EXIT_CODE=$?

echo ""
echo -e "${GREEN}Valgrind analysis complete!${NC}"
echo -e "${GREEN}Report saved to: $OUTPUT_FILE${NC}"

# Display summary if file exists
if [[ -f "$OUTPUT_FILE" ]]; then
    echo ""
    echo -e "${YELLOW}=== Memory Leak Summary ===${NC}"
    grep -A 5 "LEAK SUMMARY" "$OUTPUT_FILE" || echo "No leak summary found"
    echo ""
    echo -e "${YELLOW}=== Error Summary ===${NC}"
    grep "ERROR SUMMARY" "$OUTPUT_FILE" || echo "No error summary found"
fi

exit $EXIT_CODE