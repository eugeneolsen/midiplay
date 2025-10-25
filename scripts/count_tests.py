#!/usr/bin/env python3
"""
scripts/count_tests.py

Recursively scan the test/ directory for .cpp files and, for each file, compute and print:
 - number of test cases (common TEST macros)
 - number of assertions (common ASSERT/EXPECT/etc macros)
 - number of lines of code (LOC) = non-blank, non-comment source lines

The script:
 - Ignores matches inside C/C++ comments (// and /* */) and inside string or char literals.
 - Handles UTF-8 files and CRLF line endings.
 - Preserves newlines while stripping comments/strings so LOC counting remains accurate.
 - Is dependency-free (standard library only).
 - Offers a --json flag to emit machine-readable JSON output.
 - Exits with non-zero status on I/O errors or if test/ does not exist.
"""
from __future__ import annotations

import argparse
import io
import json
import os
import re
import sys
from typing import Dict, List, Tuple

# Regexes used after comments/strings are removed
TEST_MACROS_RE = re.compile(
    r'\b(?:TEST|TEST_F|TEST_P|TYPED_TEST|TYPED_TEST_P|TEST_CASE|SCENARIO|BOOST_AUTO_TEST_CASE)\s*\('
)
ASSERTION_MACROS_RE = re.compile(
    r'\b(?:ASSERT_[A-Z0-9_]+|EXPECT_[A-Z0-9_]+|CHECK_[A-Z0-9_]+|REQUIRE(?:_[A-Z0-9_]+)?|'
    r'BOOST_CHECK_[A-Z0-9_]+|BOOST_REQUIRE_[A-Z0-9_]+|DOCTEST_CHECK[A-Z0-9_]*|CHECK_THROWS(?:_A|_AS|_.*)?'
    r')\s*\('
)


def read_file_text(path: str) -> str:
    # Read using binary and decode to preserve robustness for different line endings/encodings.
    with open(path, 'rb') as f:
        raw = f.read()
    # Decode as UTF-8 with replacement for any invalid bytes
    text = raw.decode('utf-8', errors='replace')
    # Normalize CRLF to LF so we have consistent newlines
    text = text.replace('\r\n', '\n')
    return text


def remove_comments_and_strings(src: str) -> str:
    """
    Remove C/C++ comments and string/char literals from src.
    Returns a string of the same length where characters that were inside
    comments or strings are replaced with spaces, but newline characters are preserved.
    This allows line-oriented metrics (LOC) to remain accurate.
    """
    out_chars: List[str] = []
    i = 0
    n = len(src)

    def emit_space_for_range(start: int, end: int):
        # replace chars in [start, end) with spaces except newlines
        for j in range(start, end):
            out_chars.append(src[j] if src[j] == '\n' else ' ')

    while i < n:
        ch = src[i]

        # Handle single-line comment //
        if ch == '/' and i + 1 < n and src[i + 1] == '/':
            # consume until newline or EOF
            start = i
            i += 2
            while i < n and src[i] != '\n':
                i += 1
            emit_space_for_range(start, i)
            continue

        # Handle multi-line comment /* ... */
        if ch == '/' and i + 1 < n and src[i + 1] == '*':
            start = i
            i += 2
            while i + 1 < n and not (src[i] == '*' and src[i + 1] == '/'):
                i += 1
            # consume closing */
            if i + 1 < n:
                i += 2
            emit_space_for_range(start, i)
            continue

        # Handle raw string literals R"delim( ... )delim"
        # Also try to catch prefixes like u8R" or LR"
        if ch == 'R' or (ch in 'uUL' and i + 1 < n and src[i + 1] == 'R'):
            # attempt to detect R"delim(
            # Start at possible quote character
            rstart = i
            # advance to the quote if prefixed
            k = i
            if src[k] in 'uUL' and k + 1 < n and src[k + 1] == 'R':
                k += 1
            if k + 1 < n and src[k + 1] == '"':
                # We expect sequence R"delim(
                k += 2  # position after R"
                # find the '(' which terminates the delimiter
                delim_start = k
                while k < n and src[k] != '(' and src[k] != '\n':
                    k += 1
                if k < n and src[k] == '(':
                    delim = src[delim_start:k]
                    # end token is ')' + delim + '"'
                    end_token = ')' + delim + '"'
                    i = k + 1  # position after the '(' starting the raw contents
                    # Walk until we find end_token
                    found = False
                    while i < n:
                        if src.startswith(end_token, i):
                            i += len(end_token)
                            found = True
                            break
                        # preserve newlines if present by copying them, otherwise skip
                        if src[i] == '\n':
                            out_chars.append('\n')
                        else:
                            out_chars.append(' ')
                        i += 1
                    # If not found, we've reached EOF; replace the whole region with spaces/newlines preserved
                    # We already appended the raw string body; we must ensure the area before raw body (from rstart) has been cleared
                    # To simplify, we will retroactively replace the region from rstart to current out length by spaces/newlines
                    # Build replacement: we have currently appended some characters corresponding to the raw body.
                    # Instead of performing a retroactive fix, simpler approach: roll back out_chars to length corresponding to characters before rstart
                    # Find how many output chars we already appended for content before rstart:
                    # To avoid complexity, we will restart building out_chars less efficient but correct:
                    pass  # fallthrough to general string handling below

        # Handle normal string literals "..." and char literals '...'
        if ch == '"' or ch == "'":
            quote = ch
            start = i
            i += 1
            escaped = False
            while i < n:
                c = src[i]
                if c == '\n':
                    # string literal newline (unterminated on this line) - keep newline and continue
                    i += 1
                    continue
                if escaped:
                    escaped = False
                    i += 1
                    continue
                if c == '\\':
                    escaped = True
                    i += 1
                    continue
                if c == quote:
                    i += 1
                    break
                i += 1
            emit_space_for_range(start, i)
            continue

        # Default: copy the character as-is
        out_chars.append(ch)
        i += 1

    # If we hit raw string handling 'pass' above, the naive branch won't have treated raw strings.
    # Instead of complex retroactive handling, implement a separate, simpler pass that strips raw strings using regex.
    interim = ''.join(out_chars)

    # Remove raw strings R"delim(... )delim" using regex while preserving newlines
    # Pattern: (prefix)?R"delim\(.*?\)delim"
    # We will find occurrences and replace with spaces except newlines.
    raw_string_pattern = re.compile(r'(?:[uUL]?R)"([^\n()]*)\((.*?)\)\1"', re.DOTALL)
    def raw_replacer(m: re.Match) -> str:
        full = m.group(0)
        return ''.join(c if c == '\n' else ' ' for c in full)
    cleaned = raw_string_pattern.sub(raw_replacer, interim)

    return cleaned


def count_in_file(path: str) -> Tuple[int, int, int]:
    """
    Returns (test_cases, assertions, loc) for the given file.
    """
    try:
        src = read_file_text(path)
    except Exception:
        raise
    cleaned = remove_comments_and_strings(src)

    # Count LOC: non-blank lines after stripping comments/strings
    loc = 0
    for line in cleaned.splitlines():
        if line.strip():
            loc += 1

    # Count test cases and assertions using regex on cleaned text
    test_cases = len(TEST_MACROS_RE.findall(cleaned))
    assertions = len(ASSERTION_MACROS_RE.findall(cleaned))

    return test_cases, assertions, loc


def find_cpp_files(root: str) -> List[str]:
    files: List[str] = []
    for dirpath, dirnames, filenames in os.walk(root):
        # Do not descend into vendor/external directories that should be ignored.
        # Skip directories named 'external' or 'externa' so files inside them are not counted.
        dirnames[:] = [d for d in dirnames if d not in ('external', 'externa')]
        for fn in filenames:
            if fn.lower().endswith('.cpp'):
                files.append(os.path.join(dirpath, fn))
    return sorted(files)


def human_readable_report(results: Dict[str, Tuple[int, int, int]]) -> str:
    lines: List[str] = []
    header = f"{'File':60}  {'Tests':>6}  {'Asserts':>8}  {'LOC':>6}"
    sep = "-" * len(header)
    lines.append(header)
    lines.append(sep)
    total_tests = total_asserts = total_loc = 0
    for fname, (t, a, l) in sorted(results.items()):
        short = os.path.relpath(fname)
        lines.append(f"{short:60}  {t:6d}  {a:8d}  {l:6d}")
        total_tests += t
        total_asserts += a
        total_loc += l
    lines.append(sep)
    lines.append(f"{'TOTAL':60}  {total_tests:6d}  {total_asserts:8d}  {total_loc:6d}")
    return "\n".join(lines)


def main(argv: List[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Count test cases, assertions, and LOC in .cpp files under test/"
    )
    parser.add_argument(
        "--json",
        action="store_true",
        help="Emit results as JSON (filename -> {tests, assertions, loc})"
    )
    parser.add_argument(
        "--root",
        default="test",
        help="Root directory to search (default: test)"
    )
    args = parser.parse_args(argv)

    root = args.root
    if not os.path.exists(root) or not os.path.isdir(root):
        print(f"Error: directory '{root}' does not exist or is not a directory.", file=sys.stderr)
        return 2

    try:
        files = find_cpp_files(root)
    except Exception as e:
        print(f"Error while scanning '{root}': {e}", file=sys.stderr)
        return 3

    if not files:
        print(f"No .cpp files found under '{root}'.", file=sys.stderr)
        return 0

    results: Dict[str, Tuple[int, int, int]] = {}
    grand_tests = grand_asserts = grand_loc = 0
    for f in files:
        try:
            t, a, l = count_in_file(f)
        except Exception as e:
            print(f"Error processing '{f}': {e}", file=sys.stderr)
            return 4
        results[f] = (t, a, l)
        grand_tests += t
        grand_asserts += a
        grand_loc += l

    if args.json:
        out = {
            "files": {
                os.path.relpath(fname): {"tests": v[0], "assertions": v[1], "loc": v[2]}
                for fname, v in results.items()
            },
            "total": {"tests": grand_tests, "assertions": grand_asserts, "loc": grand_loc},
        }
        print(json.dumps(out, ensure_ascii=False, indent=2))
    else:
        print(human_readable_report(results))

    return 0


if __name__ == "__main__":
    sys.exit(main())