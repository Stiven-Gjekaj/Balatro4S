#!/usr/bin/env python3
"""fuzz.py: makes random scenarios, plays each one with the code of Balatro
(tools/oracle.sh score) and with Balatro4S (build/fuzz/scenario), and
prints each scenario where the results are not the same.

Usage: tools/fuzz.py [COUNT] [SEED] [--jokers j_a,j_b] [--use] [--pack] [--voucher] [--blind] [--tag] [--start] [--round] [--each N] [--keep]
With --use, each scenario uses a Tarot, Planet, or Spectral card on the hand
in place of a play. With --pack, each scenario opens a booster pack in the
shop and takes some of its cards. With --voucher, each scenario buys a
voucher in the shop. With --blind, each play has a random blind. With --tag,
each scenario skips blinds for random tags, then opens a shop. With --start,
each scenario starts a run with a random deck and compares the start. With --round,
each play comes after a discard and ends the round, and the money of
evaluate_round is compared. About a third of the plays have a random deck. With --each N, each
joker comes in N scenarios.
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
BLINDS = ["bl_arm", "bl_club", "bl_eye", "bl_final_acorn", "bl_final_bell", "bl_final_heart", "bl_final_leaf",
          "bl_final_vessel", "bl_fish", "bl_flint", "bl_goad", "bl_head", "bl_hook", "bl_house", "bl_manacle",
          "bl_mark", "bl_mouth", "bl_needle", "bl_ox", "bl_pillar", "bl_plant", "bl_psychic", "bl_serpent",
          "bl_small", "bl_big", "bl_tooth", "bl_wall", "bl_water", "bl_wheel", "bl_window"]


# To Do List picks its hand from pairs(G.GAME.hands). The LuaJIT of
# Balatro hashes strings with a new random seed in each process, so the
# original itself gives another hand in each session. It cannot be compared.
SKIP = {"j_todo_list"}

# --each: the joker that each scenario must have.
FORCED = []


def all_jokers():
    keys = []
    with open(os.path.join(ROOT, "src", "core", "bo_data.c")) as f:
        for line in f:
            m = re.match(r'\s*\{"(j_[a-z0-9_]+)"', line)
            if m and m.group(1) not in SKIP:
                keys.append(m.group(1))
    return keys


def all_consumables():
    keys = []
    with open(os.path.join(ROOT, "src", "core", "bo_data.c")) as f:
        for line in f:
            m = re.match(r'\s*\{"(c_[a-z0-9_]+)", "[^"]*", BO_SET_(TAROT|PLANET|SPECTRAL)', line)
            if m:
                keys.append(m.group(1))
    return keys


# The jokers that see the use of a card or the cards that it adds or removes.
USE_JOKERS = ["j_constellation", "j_glass", "j_hologram", "j_caino", "j_fortune_teller", "j_juggler",
              "j_merry_andy", "j_troubadour", "j_credit_card", "j_turtle_bean"]


def use_scenario(rng, jokers, consumables):
    lines = ["seed " + "".join(rng.choice("ABCDEFGHJKLMNPQRSTUVWXYZ123456789") for _ in range(8))]
    lines.append("ante %d" % rng.randint(1, 8))
    lines.append("dollars %d" % rng.randint(-5, 60))
    for hand in rng.sample(HANDS, rng.randint(0, 3)):
        lines.append("level %s %d" % (hand, rng.randint(1, 6)))
    pool = USE_JOKERS + rng.sample(jokers, 10)
    chosen = rng.sample(pool, rng.randint(0, 5))
    for key in chosen:
        words = ["joker", key]
        if rng.random() < 0.25:
            words.append("edition=" + rng.choice(EDITIONS + ["negative"]))
        lines.append(" ".join(words))
    for i in range(len(chosen)):
        if rng.random() < 0.15:
            lines.append("eternal %d" % (i + 1))
    for _ in range(rng.randint(0, 1)):
        words = ["consumable", rng.choice(consumables)]
        if rng.random() < 0.2:
            words.append("edition=negative")
        lines.append(" ".join(words))
    if rng.random() < 0.7:
        lines.append("last " + rng.choice(consumables))
    lines.append("deck %d" % rng.randint(0, 20))
    count = rng.randint(0, 8)
    held = [card(rng) for _ in range(count)]
    if held:
        lines.append("hand " + " ".join(held))
    picks = sorted(rng.sample(range(1, count + 1), min(count, rng.choice([0, 1, 1, 2, 2, 3, 5]))))
    lines.append("use " + rng.choice(consumables) + "".join(" %d" % p for p in picks))
    return "\n".join(lines) + "\n"


def all_packs():
    keys = []
    with open(os.path.join(ROOT, "src", "core", "bo_data.c")) as f:
        for line in f:
            m = re.match(r'\s*\{"(p_[a-z0-9_]+)"', line)
            if m:
                keys.append(m.group(1))
    return keys


PACK_JOKERS = ["j_hallucination", "j_red_card", "j_hologram", "j_constellation", "j_glass", "j_caino",
               "j_merry_andy", "j_juggler"]


def pack_scenario(rng, jokers, consumables, packs):
    lines = ["seed " + "".join(rng.choice("ABCDEFGHJKLMNPQRSTUVWXYZ123456789") for _ in range(8))]
    lines.append("ante %d" % rng.randint(1, 8))
    lines.append("dollars %d" % rng.randint(0, 40))
    pool = PACK_JOKERS + rng.sample(jokers, 6)
    chosen = rng.sample(pool, rng.randint(0, 5))
    for key in chosen:
        words = ["joker", key]
        if rng.random() < 0.2:
            words.append("edition=" + rng.choice(EDITIONS + ["negative"]))
        lines.append(" ".join(words))
    for _ in range(rng.randint(0, 2)):
        lines.append("consumable " + rng.choice(consumables))
    if rng.random() < 0.5:
        lines.append("last " + rng.choice(consumables))
    deck = [card(rng) for _ in range(rng.randint(0, 20))]
    if deck:
        lines.append("deck_cards " + " ".join(deck))
    lines.append("open " + rng.choice(packs))
    for _ in range(rng.randint(1, 3)):
        if rng.random() < 0.15:
            lines.append("skip")
            break
        picks = sorted(rng.sample(range(1, 9), rng.choice([0, 1, 1, 2, 3])))
        lines.append("choose %d" % rng.randint(1, 5) + "".join(" %d" % p for p in picks))
    return "\n".join(lines) + "\n"


def all_vouchers():
    keys = []
    with open(os.path.join(ROOT, "src", "core", "bo_data.c")) as f:
        for line in f:
            m = re.match(r'\s*\{"(v_[a-z0-9_]+)"', line)
            if m:
                keys.append(m.group(1))
    return keys


def voucher_scenario(rng, jokers, vouchers):
    lines = ["seed " + "".join(rng.choice("ABCDEFGHJKLMNPQRSTUVWXYZ123456789") for _ in range(8))]
    lines.append("ante %d" % rng.randint(1, 8))
    # Enough money for the voucher: the test is the effect, not the price.
    lines.append("dollars %d" % rng.randint(20, 60))
    lines.append("hands_left %d" % rng.randint(1, 4))
    lines.append("discards_left %d" % rng.randint(0, 4))
    for key in rng.sample(jokers, rng.randint(0, 4)):
        words = ["joker", key]
        if rng.random() < 0.2:
            words.append("edition=" + rng.choice(EDITIONS + ["negative"]))
        lines.append(" ".join(words))
    lines.append("redeem " + rng.choice(vouchers))
    return "\n".join(lines) + "\n"


# The tags that open a pack or reroll the boss use the screen of Balatro.
# The pack tests cover their packs. The hand of the Orbital Tag comes from
# pairs() over strings, which LuaJIT orders in another way in each process.
TAG_SKIP = {"tag_charm", "tag_meteor", "tag_ethereal", "tag_standard", "tag_buffoon", "tag_boss", "tag_orbital"}


def all_tags():
    keys = []
    with open(os.path.join(ROOT, "src", "core", "bo_data.c")) as f:
        for line in f:
            m = re.match(r'\s*\{"(tag_[a-z0-9_]+)"', line)
            if m and m.group(1) not in TAG_SKIP:
                keys.append(m.group(1))
    return keys


def tag_scenario(rng, jokers, tags):
    lines = ["seed " + "".join(rng.choice("ABCDEFGHJKLMNPQRSTUVWXYZ123456789") for _ in range(8))]
    lines.append("ante %d" % rng.randint(1, 8))
    lines.append("dollars %d" % rng.randint(-5, 60))
    for key in rng.sample(jokers, rng.randint(0, 4)):
        lines.append("joker " + key)
    for _ in range(rng.randint(1, 4)):
        lines.append("tag " + rng.choice(tags))
    return "\n".join(lines) + "\n"


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
    lines.append("discards_left %d" % rng.randint(1 if "--round" in sys.argv else 0, 4))
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
    chosen = rng.sample(jokers, min(len(jokers), rng.randint(1, 5)))
    if FORCED:
        chosen = [FORCED[0]] + [k for k in chosen if k != FORCED[0]][:rng.randint(0, 4)]
        rng.shuffle(chosen)
    for key in chosen:
        words = ["joker", key]
        if rng.random() < 0.2:
            words.append("edition=" + rng.choice(EDITIONS + ["negative"]))
        lines.append(" ".join(words))
    lines.append("deck %d" % rng.randint(0, 40))
    lines.append("play " + " ".join(hand_cards(rng, rng.randint(1, 5))))
    held = [card(rng) for _ in range(rng.randint(0, 7))]
    if held:
        lines.append("hand " + " ".join(held))
    if rng.random() < 0.3:
        lines.append("back %s" % rng.choice(BACKS))
    if rng.random() < 0.3:
        lines.append("rules %d" % rng.choice(RULE_CHALLENGES))
    if "--blind" in sys.argv:
        lines.append("blind " + rng.choice(BLINDS))
        lines.append("most_played " + rng.choice(HANDS))
    if "--round" in sys.argv:
        deck = [card(rng) for _ in range(rng.randint(0, 12))]
        if deck:
            lines.append("deck_cards " + " ".join(deck))
        if held and rng.random() < 0.8:
            picks = sorted(rng.sample(range(1, len(held) + 1), rng.randint(1, min(5, len(held)))))
            lines.append("discard " + " ".join(str(p) for p in picks))
        if rng.random() < 0.6:
            lines.append("target %d" % rng.randint(1, 50))
        lines.append("end_round")
    return "\n".join(lines) + "\n"


BACKS = ["b_red", "b_blue", "b_yellow", "b_green", "b_black", "b_magic", "b_nebula", "b_ghost", "b_abandoned",
         "b_checkered", "b_zodiac", "b_painted", "b_anaglyph", "b_plasma", "b_erratic"]


# The challenges with custom rules for a play. Luxury Tax (6) changes the
# hand size in CardArea:update, which the oracle does not run.
RULE_CHALLENGES = [0, 2, 4, 5, 7, 9, 10, 11, 12, 17, 18, 19]


def start_scenario(rng):
    # A new run with a random deck and seed: tools/start.lua runs start_run.
    seed = "".join(rng.choice("123456789ABCDEFGHIJKLMNPQRSTUVWXYZ") for _ in range(8))
    if rng.random() < 0.3:
        return "seed %s\nchallenge %d\nstart\n" % (seed, rng.randint(0, 19))
    return "seed %s\nback %s\nstake %d\nstart\n" % (seed, rng.choice(BACKS), rng.randint(1, 8))


def run(command):
    return subprocess.run(command, capture_output=True, text=True, errors="replace").stdout


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    count = int(args[0]) if args else 50
    rng = random.Random(int(args[1]) if len(args) > 1 else 1)
    jokers = all_jokers()
    consumables = all_consumables()
    packs = all_packs()
    if "--jokers" in sys.argv:
        jokers = sys.argv[sys.argv.index("--jokers") + 1].split(",")
    os.makedirs(OUT, exist_ok=True)
    subprocess.run(["clang", "-ffp-contract=off", "-O1", "-Wall", "-I" + os.path.join(ROOT, "src", "core"),
                    os.path.join(ROOT, "tools", "scenario.c")] +
                   [os.path.join(ROOT, "src", "core", f) for f in os.listdir(os.path.join(ROOT, "src", "core"))
                    if f.endswith(".c")] + ["-lz", "-o", os.path.join(OUT, "scenario")], check=True)
    failures = 0
    if "--each" in sys.argv:
        per = int(sys.argv[sys.argv.index("--each") + 1])
        order = [k for k in jokers for _ in range(per)]
        count = len(order)
    for i in range(count):
        if "--each" in sys.argv:
            FORCED[:] = [order[i]]
        if "--start" in sys.argv:
            text = start_scenario(rng)
        elif "--tag" in sys.argv:
            text = tag_scenario(rng, jokers, all_tags())
        elif "--voucher" in sys.argv:
            text = voucher_scenario(rng, jokers, all_vouchers())
        elif "--pack" in sys.argv:
            text = pack_scenario(rng, jokers, consumables, packs)
        elif "--use" in sys.argv:
            text = use_scenario(rng, jokers, consumables)
        else:
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
