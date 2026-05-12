#!/usr/bin/env python3
import argparse
import re
from collections import Counter


def extract_words(text: str) -> list[str]:
    return re.findall(r"[a-z]+", text.lower())


def main():
    parser = argparse.ArgumentParser(
        description="Display the 20 most common words from a text file."
    )
    parser.add_argument("file", help="Path to the input text file")
    parser.add_argument(
        "-n", "--top", type=int, default=20, help="Number of top words to show (default: 20)"
    )
    args = parser.parse_args()

    with open(args.file, encoding="utf-8") as f:
        words = extract_words(f.read())

    counter = Counter(words)
    most_common = counter.most_common(args.top)

    max_word = max(len(w) for w, _ in most_common) if most_common else 0
    max_count = max(len(str(c)) for _, c in most_common) if most_common else 0

    header = f"{'Rank':<5} {'Word':<{max_word}}  Count"
    sep = "-" * len(header)
    print(sep)
    print(header)
    print(sep)
    for rank, (word, count) in enumerate(most_common, start=1):
        print(f"{rank:<5} {word:<{max_word}}  {count:> {max_count}}")
    print(sep)


if __name__ == "__main__":
    main()
