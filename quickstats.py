#!/usr/bin/env python3
"""Quick CSV stats utility — prints mean, median, min, max, count per numeric column."""

import argparse
import csv
import statistics
import sys
from pathlib import Path


def parse_args():
    parser = argparse.ArgumentParser(
        description="Print basic statistics for each numeric column in a CSV file."
    )
    parser.add_argument("file", help="Path to the CSV file")
    return parser.parse_args()


def stats(values):
    """Return a summary dict for a list of numbers."""
    return {
        "count": len(values),
        "mean": statistics.mean(values),
        "median": statistics.median(values),
        "min": min(values),
        "max": max(values),
    }


def main():
    args = parse_args()
    path = Path(args.file)

    if not path.is_file():
        print(f"Error: file not found: {path}", file=sys.stderr)
        sys.exit(1)

    # First pass: read all rows
    try:
        with path.open(newline="") as fh:
            reader = csv.DictReader(fh)
            rows = list(reader)
    except Exception as exc:
        print(f"Error reading CSV: {exc}", file=sys.stderr)
        sys.exit(1)

    if not rows:
        print("No data rows in CSV.", file=sys.stderr)
        sys.exit(0)

    # Detect numeric columns
    numeric_cols = {}
    for col in rows[0].keys():
        values = []
        for row in rows:
            raw = row.get(col, "").strip()
            if raw == "":
                continue
            try:
                values.append(float(raw))
            except ValueError:
                break
        else:
            if values:
                numeric_cols[col] = values

    if not numeric_cols:
        print("No numeric columns found.", file=sys.stderr)
        sys.exit(0)

    # Print results
    header = f"{'Column':<20} {'Count':>7} {'Mean':>12} {'Median':>12} {'Min':>12} {'Max':>12}"
    print(header)
    print("-" * len(header))
    for col, values in numeric_cols.items():
        s = stats(values)
        print(
            f"{col:<20} {s['count']:>7} {s['mean']:>12.4f} {s['median']:>12.4f} "
            f"{s['min']:>12.4f} {s['max']:>12.4f}"
        )


if __name__ == "__main__":
    main()
