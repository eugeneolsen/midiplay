#!/usr/bin/env python3
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

EXTS = ('.cpp', '.hpp')
excluded_dir = 'test'

counts = []
total = 0

for dirpath, dirnames, filenames in os.walk(ROOT):
    # skip test directory and any subdirectories under test
    if excluded_dir in Path(dirpath).parts:
        continue
    for fname in filenames:
        if fname.endswith(EXTS):
            full = Path(dirpath) / fname
            # Read file and count lines, ignoring encoding issues
            try:
                with open(full, 'r', encoding='utf-8', errors='ignore') as f:
                    lines = sum(1 for _ in f)
            except Exception:
                lines = 0
            rel = full.relative_to(ROOT)
            counts.append((str(rel), lines))
            total += lines

counts.sort()

for path_, n in counts:
    print(f"{path_} — {n} lines")

print("Total lines (excluding test/):", total)
