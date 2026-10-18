#!/usr/bin/env python3
"""fuzz.py: makes random scenarios, plays each one with the code of Balatro
(tools/oracle.sh score) and with Balatro4S (build/fuzz/scenario), and
prints each scenario where the results are not the same.

Usage: tools/fuzz.py [COUNT] [SEED] [--jokers j_a,j_b] [--keep]
"""

import os
import random
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "build", "fuzz")

SUITS = "SHCD"
RANKS = "23456789TJQKA"
ENHANCEMENTS = ["m_bonus", "m_mult", "m_wild", "m_glass", "m_steel", "m_stone", "m_gold", "m_lucky"]
EDITIONS = ["foil", "holo", "polychrome"]
SEALS = ["Gold", "Red", "Blue", "Purple"]
HANDS = ["Flush_Five", "Flush_House", "Five_of_a_Kind", "Straight_Flush", "Four_of_a_Kind", "Full_House",
         "Flush", "Straight", "Three_of_a_Kind", "Two_Pair", "Pair", "High_Card"]
SUIT_NAMES = ["Spades", "Hearts", "Clubs", "Diamonds"]


# To Do List picks its hand from pairs(G.GAME.hands). The LuaJIT of
# Balatro hashes strings with a new random seed in each process, so the
# original itself gives another hand in each session. It cannot be compared.
SKIP = {"j_todo_list"}


def all_jokers():
    keys = []
    with open(os.path.join(ROOT, "src", "core", "bo_data.c")) as f:
        for line in f:
            m = re.match(r'\s*\{"(j_[a-z0-9_]+)"', line)
            if m and m.group(1) not in SKIP:
                keys.append(m.group(1))
    return keys


def card(rng, plain=False):
    token = rng.choice(SUITS) + "_" + rng.choice(RANKS)
    if plain:
        return token
    if rng.random() < 0.3:
        token += ":" + rng.choice(ENHANCEMENTS)
    if rng.random() < 0.15:
        token += ":" + rng.choice(EDITIONS)
    if rng.random() < 0.15:
        token += ":" + rng.choice(SEALS)
    return token


def hand_cards(rng, count):
    # Many hands are pairs, flushes, or straights, so that the jokers of the
    # hand types show.
    kind = rng.random()
    cards = [card(rng) for _ in range(count)]
    if kind < 0.3 and count >= 2:
        rank = rng.choice(RANKS)
        for i in range(rng.randint(2, count)):
            cards[i] = rng.choice(SUITS) + "_" + rank + cards[i][3:]
    elif kind < 0.45 and count >= 4:
        suit = rng.choice(SUITS)
        for i in range(count):
            cards[i] = suit + cards[i][1:]
    elif kind < 0.6 and count >= 4:
        start = rng.randint(0, len(RANKS) - count)
        for i in range(count):
            cards[i] = cards[i][0:2] + RANKS[start + i] + cards[i][3:]
    return cards


def scenario(rng, jokers):
    lines = ["seed " + "".join(rng.choice("ABCDEFGHJKLMNPQRSTUVWXYZ123456789") for _ in range(8))]
    lines.append("ante %d" % rng.randint(1, 8))
    lines.append("dollars %d" % rng.randint(-5, 60))
    lines.append("hands_left %d" % rng.randint(1, 4))
    lines.append("discards_left %d" % rng.randint(0, 4))
    lines.append("hands_played %d" % rng.randint(0, 3))
    lines.append("discards_used %d" % rng.randint(0, 3))
    for hand in rng.sample(HANDS, rng.randint(0, 3)):
        lines.append("level %s %d" % (hand, rng.randint(1, 6)))
    for hand in rng.sample(HANDS, rng.randint(0, 4)):
        lines.append("played %s %d" % (hand, rng.randint(0, 9)))
    lines.append("idol %d %s" % (rng.randint(2, 14), rng.choice(SUIT_NAMES)))
    lines.append("mail %d" % rng.randint(2, 14))
    lines.append("ancient " + rng.choice(SUIT_NAMES))
    lines.append("castle " + rng.choice(SUIT_NAMES))
    for key in rng.sample(jokers, min(len(jokers), rng.randint(1, 5))):
        words = ["joker", key]
        if rng.random() < 0.2:
            words.append("edition=" + rng.choice(EDITIONS + ["negative"]))
        lines.append(" ".join(words))
    lines.append("deck %d" % rng.randint(0, 40))
    lines.append("play " + " ".join(hand_cards(rng, rng.randint(1, 5))))
    held = [card(rng) for _ in range(rng.randint(0, 7))]
    if held:
        lines.append("hand " + " ".join(held))
    return "\n".join(lines) + "\n"


def run(command):
    return subprocess.run(command, capture_output=True, text=True).stdout


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    count = int(args[0]) if args else 50
    rng = random.Random(int(args[1]) if len(args) > 1 else 1)
    jokers = all_jokers()
    if "--jokers" in sys.argv:
        jokers = sys.argv[sys.argv.index("--jokers") + 1].split(",")
    os.makedirs(OUT, exist_ok=True)
    subprocess.run(["clang", "-ffp-contract=off", "-O1", "-Wall", "-I" + os.path.join(ROOT, "src", "core"),
                    os.path.join(ROOT, "tools", "scenario.c")] +
                   [os.path.join(ROOT, "src", "core", f) for f in os.listdir(os.path.join(ROOT, "src", "core"))
                    if f.endswith(".c")] + ["-lz", "-o", os.path.join(OUT, "scenario")], check=True)
    failures = 0
    for i in range(count):
        text = scenario(rng, jokers)
        path = os.path.join(OUT, "case%d.txt" % i)
        with open(path, "w") as f:
            f.write(text)
        lua = run([os.path.join(ROOT, "tools", "oracle.sh"), "score", path])
        c = run([os.path.join(OUT, "scenario"), path])
        if lua != c or not lua:
            failures += 1
            print("=== case %d: %s" % (i, path))
            print(text)
            lua_lines, c_lines = lua.splitlines(), c.splitlines()
            for n in range(max(len(lua_lines), len(c_lines))):
                a = lua_lines[n] if n < len(lua_lines) else ""
                b = c_lines[n] if n < len(c_lines) else ""
                if a != b:
                    print("  balatro:   " + a)
                    print("  balatro4s: " + b)
        elif "--keep" not in sys.argv:
            os.remove(path)
    print("%d cases, %d different" % (count, failures))
    sys.exit(1 if failures else 0)


if __name__ == "__main__":
    main()
