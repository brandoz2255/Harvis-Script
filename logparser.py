#!/usr/bin/env python3
"""Apache/Nginx log parser with summary statistics."""

import argparse
import collections
import json
import re
import sys
from datetime import datetime

LOG_PATTERN = re.compile(
    r'(?P<ip>\d{1,3}(?:\.\d{1,3}){3})\s+'
    r'(?P<ident>\S+)\s+'
    r'(?P<user>\S+)\s+'
    r'\[(?P<timestamp>[^\]]+)\]\s+'
    r'"(?P<method>[A-Z]+)\s+'
    r'(?P<path>\S+)\s+'
    r'(?P<protocol>[^"]+)"\s+'
    r'(?P<status>\d{3})\s+'
    r'(?P<size>\S+)'
)

TIMESTAMP_FORMAT = "%d/%b/%Y:%H:%M:%S %z"


def parse_line(line):
    """Parse a single log line. Returns a dict or None on failure."""
    line = line.strip()
    if not line:
        return None

    match = LOG_PATTERN.match(line)
    if not match:
        return None

    data = match.groupdict()

    try:
        data["timestamp_dt"] = datetime.strptime(data["timestamp"], TIMESTAMP_FORMAT)
    except ValueError:
        data["timestamp_dt"] = None

    size = data["size"]
    data["size"] = 0 if size == "-" else int(size)
    data["status"] = int(data["status"])

    return data


def parse_file(filepath):
    """Parse an entire log file. Returns (entries, malformed_count)."""
    entries = []
    malformed = 0

    try:
        with open(filepath, "r", encoding="utf-8", errors="replace") as f:
            for lineno, line in enumerate(f, 1):
                entry = parse_line(line)
                if entry is None:
                    if line.strip():
                        malformed += 1
                else:
                    entries.append(entry)
    except FileNotFoundError:
        print(f"Error: file not found: {filepath}", file=sys.stderr)
        sys.exit(1)
    except PermissionError:
        print(f"Error: permission denied: {filepath}", file=sys.stderr)
        sys.exit(1)

    return entries, malformed


def compute_stats(entries):
    """Compute summary statistics from parsed entries."""
    ip_counter = collections.Counter()
    path_counter = collections.Counter()
    status_counter = collections.Counter()
    total_size = 0

    for entry in entries:
        ip_counter[entry["ip"]] += 1
        path_counter[entry["path"]] += 1
        status_counter[entry["status"]] += 1
        total_size += entry["size"]

    return {
        "total_requests": len(entries),
        "total_bytes": total_size,
        "top_10_ips": ip_counter.most_common(10),
        "top_10_paths": path_counter.most_common(10),
        "status_codes": dict(sorted(status_counter.items())),
    }


def format_text(stats):
    """Format stats as human-readable text."""
    lines = [
        f"Total requests: {stats['total_requests']}",
        f"Total bytes: {stats['total_bytes']}",
        "",
        "Top 10 IPs:",
    ]

    for ip, count in stats["top_10_ips"]:
        lines.append(f"  {ip:>15s}  {count}")

    lines.append("")
    lines.append("Top 10 Paths:")

    for path, count in stats["top_10_paths"]:
        lines.append(f"  {path:<50s}  {count}")

    lines.append("")
    lines.append("Status Code Distribution:")

    for code, count in stats["status_codes"].items():
        lines.append(f"  {code}  {count}")

    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description="Parse Apache/Nginx log files.")
    parser.add_argument("logfile", help="Path to the log file")
    parser.add_argument("--json", action="store_true", dest="as_json", help="Output as JSON")
    args = parser.parse_args()

    entries, malformed = parse_file(args.logfile)

    if not entries and not malformed:
        print("No entries found in log file.")
        return

    stats = compute_stats(entries)

    if args.as_json:
        output = {
            "summary": {
                "total_requests": stats["total_requests"],
                "total_bytes": stats["total_bytes"],
                "top_10_ips": [{"ip": ip, "count": count} for ip, count in stats["top_10_ips"]],
                "top_10_paths": [{"path": path, "count": count} for path, count in stats["top_10_paths"]],
                "status_codes": {str(k): v for k, v in stats["status_codes"].items()},
            },
            "malformed_lines": malformed,
        }
        print(json.dumps(output, indent=2))
    else:
        print(format_text(stats))
        if malformed:
            print(f"\nMalformed lines skipped: {malformed}")


if __name__ == "__main__":
    main()
