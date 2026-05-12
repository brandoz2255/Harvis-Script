#!/usr/bin/env python3
"""Deep merge two JSON files and output the result to stdout."""

import argparse
import json
import sys


def deep_merge(base, override):
    """Recursively merge override into base.

    - Dicts are merged recursively.
    - Lists are concatenated (base + override).
    - All other types are replaced by the override value.
    """
    if not isinstance(base, dict) or not isinstance(override, dict):
        return override

    merged = base.copy()
    for key, value in override.items():
        if key in merged and isinstance(merged[key], dict) and isinstance(value, dict):
            merged[key] = deep_merge(merged[key], value)
        elif key in merged and isinstance(merged[key], list) and isinstance(value, list):
            merged[key] = merged[key] + value
        else:
            merged[key] = value
    return merged


def load_json(path):
    """Load and parse a JSON file."""
    try:
        with open(path, "r", encoding="utf-8") as f:
            return json.load(f)
    except FileNotFoundError:
        print(f"Error: File not found: {path}", file=sys.stderr)
        sys.exit(1)
    except json.JSONDecodeError as e:
        print(f"Error: Invalid JSON in {path}: {e}", file=sys.stderr)
        sys.exit(1)
    except OSError as e:
        print(f"Error: Could not read {path}: {e}", file=sys.stderr)
        sys.exit(1)


def main():
    parser = argparse.ArgumentParser(
        description="Deep merge two JSON files and output the result to stdout."
    )
    parser.add_argument("first", help="Base JSON file")
    parser.add_argument("second", help="Override JSON file (merged into first)")
    args = parser.parse_args()

    base = load_json(args.first)
    override = load_json(args.second)
    result = deep_merge(base, override)

    json.dump(result, sys.stdout, indent=2, ensure_ascii=False)
    print()


if __name__ == "__main__":
    main()
