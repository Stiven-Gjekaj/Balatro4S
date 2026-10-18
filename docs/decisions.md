# Decisions

This file records each choice of the owner and each technical decision.
Each entry gives the reason. The owner made these choices on 2026-10-18.

## What the app is

- **Balatro4S is a demake of Balatro for the iPhone 4S on iOS 6.1.3**
  (jailbroken). It is a native app, installed as a .deb package.
- **The rules are a native rewrite.** The game logic is new C code. The
  renderer is OpenGL ES 2.0. The app does not run the Lua code of the
  original. Reason: the A5 has two cores at 800 MHz and 512 MB of memory.
  A native rewrite is fast, and it does not need a Lua runtime.
- **The original Lua code is the reference for the rules.** Each rule,
  number, and text comes from Balatro 1.0.1o. A port of a joker, a blind,
  or a voucher must give the same result as the original.
- **1.0.0 has all the content of Balatro 1.0.1o.** This is all 150 jokers,
  all decks, stakes, vouchers, tags, blinds, consumables, challenges, and
  the collection.

## The assets of Balatro

- **The repository is public and holds no file of Balatro.** The licence of
  the new code is MIT. The art, the sounds, the font, and the text of
  Balatro belong to their owners.
- **The app reads the assets from the Balatro.love of the owner of the
  phone.** The owner copies `Balatro.love` from a bought copy of Balatro to
  `/var/mobile/Media/Balatro4S/Balatro.love`. AFC tools and `scp` reach
  this directory. At the first start, the app unpacks the file into its
  cache. Reason: the .deb then holds no file of Balatro, so it can go to
  GitHub and to the Cydia repository like the other 4S apps. This is a
  change to the first plan, where the .deb held the assets and stayed
  private.
- **`scripts/install.sh` copies `Balatro.love` to the phone** when the
  phone does not have it. It reads the file from
  `/Applications/Balatro.app`, or from the path in `BALATRO_LOVE`.
- **The tests do not read `Balatro.love`.** A test builds the state that it
  needs.
- **The reference version is Balatro 1.0.1o** (`version.jkr` says
  `1.0.1o-FULL`). The app checks the version in the file and shows a
  message for another version.

## Identity

- Bundle ID `io.github.stivengjekaj.balatro4s`, display name `Balatro4S`.
- iPhone only. iOS 6.0 or later. Landscape only. Reason: Balatro is a
  landscape game. The screen of the 4S is 480 x 320 points (960 x 640
  pixels).
- The class prefix is `BO`.

## Toolchain

- The same toolchain as Groovelet, RetroTube, and ZettaLite: `build.sh`,
  the iPhoneOS 9.3 SDK with `fix-sdk.sh`, clang for `armv7-apple-ios6.0`,
  ARM mode only, ARC for Objective-C.
- **The rules core is plain C99**, in `src/core`. It has no UIKit and no
  OpenGL. The tests of the core operate on the Mac and on the phone.

## Releases

- Each big part of the roadmap is one minor release. A release gets a git
  tag `vX.Y.Z`, a changelog entry, a GitHub release with the .deb, and an
  entry in the Cydia repository.
- **An agent may make a full release with no review** (2026-10-18). When
  a milestone is complete and its tests pass on the phone, the agent
  pushes the commits, adds the tag, publishes the GitHub release with the
  .deb, adds the .deb to the Cydia repository, and closes the issues.

## The random numbers

- **A seed gives the same run as in Balatro** (0.2, 2026-10-18). Balatro
  seeds the `math.random` of LuaJIT with `pseudoseed`, a hash of a key such
  as `nr1` and the seed of the run. `bo_random` ports `pseudohash`,
  `pseudoseed`, `pseudoshuffle`, `random_string`, and the TW223 generator
  of LuaJIT.
- **The game math turns off the fused multiply-add.** Clang joins `a * b +
  c` into one instruction on arm64, which rounds one time less. LuaJIT does
  each step on its own, so `build.sh` and `scripts/test.sh` give
  `-ffp-contract=off`.
- **The oracle runs the code of Balatro in the LuaJIT of Balatro.app**
  (`tools/oracle.sh`). The LuaJIT of Homebrew is built with the fused
  multiply-add in `math.randomseed`, and gives other numbers. The tests hold
  the numbers of the oracle. They do not read Balatro.love.

## The jokers

- **All 150 jokers score as in Balatro** (0.3, 2026-10-18). `bo_joker`
  ports `Card:calculate_joker` for each context, and `bo_eval` ports
  `G.FUNCS.evaluate_play`. `tools/fuzz.py` plays random hands with random
  jokers in this code and in the code of Balatro (`tools/headless.lua`).
  10000 of 10000 cases give the same chips, mult, and money.
- **The To Do List is not in the fuzzer.** Balatro picks its next hand with
  `pairs()` over a table with string keys. LuaJIT hashes strings with a new
  random value in each process, so this order changes from one start of
  Balatro to the next. Balatro4S uses one fixed order.
- **The tallies of the jokers do not change during a hand.** Balatro keeps
  the count of Steel cards and of Stone cards in `Card:update`, which runs
  one time in each frame. `bo_joker_update` does the same before a hand, so
  that Vampire does not change the count in the middle of the hand.
- **The money of a hand comes at the end.** Balatro adds money with events
  that run after the score. Balatro4S keeps the money in
  `pending_dollars` until the hand ends, so that Bull and Bootstraps see
  the same money as in Balatro.

## The consumables and the packs

- **Each Tarot, Planet, and Spectral card acts as in Balatro** (0.4,
  2026-10-18). `bo_consume` ports `Card:use_consumeable` and
  `Card:can_use_consumeable`. `tools/fuzz.py --use` uses random cards on
  random hands with random jokers, in this code and in the code of
  Balatro. 2000 of 2000 cases give the same hand, jokers, consumables,
  money, and hand levels.
- **Each booster pack opens as in Balatro.** `bo_pack` ports `Card:open`,
  the choice of a card in a pack, `G.FUNCS.skip_booster`, and
  `G.FUNCS.end_consumeable`. `tools/fuzz.py --pack` opens random packs. 2000
  of 2000 cases give the same cards and the same state after the choices.
- **The events of Balatro set the order of some steps.** Balatro runs many
  steps in events after the code that adds them. Balatro4S runs each step
  at once, in the order of the events. Some examples:
  - The Fool makes the card that was used before it. The new
    `last_tarot_planet` comes after the events of The Fool.
  - Hallucination tests its chance when a pack opens, but its Tarot card
    comes after the cards of the pack.
  - The jokers see the cards that The Hanged Man removes before the cards
    break, so Glass Joker gets no shattered card from them.
- **A bigger hand draws its new cards at once.** `CardArea:change_size`
  draws cards while the hand is on. For example, Hex can destroy Merry
  Andy, and then the hand gets one more card.
- **The headless Balatro of the tests removes a dissolved card in an
  event.** A joker that is destroyed in a loop over the jokers stays in the
  list until the loop ends, as in the game, where the card leaves after its
  animation.

## The vouchers, the blinds, and the tags

- **The vouchers, the boss blinds, and the tags act as in Balatro** (0.5,
  2026-10-18). `bo_voucher` ports `Card:redeem` and `Card:apply_to_run`.
  `bo_boss` ports the Blind object of `blind.lua`. `bo_tag` ports
  `Tag:apply_to_run` and `add_tag`. On the final code, `tools/fuzz.py`
  gave these results:
  - `--voucher`: 500 of 500 random vouchers agree.
  - `--blind`: 1000 of 1000 random plays with a random blind agree. The
    test covers `set_blind`, `press_play` (The Hook and The Tooth),
    `debuff_hand`, and `modify_hand`.
  - `--tag`: 1000 of 1000 random skips and the shops after them agree.
- **The fuzzer does not test some tags.** The tags that open a pack or
  reroll the boss use the screen of Balatro. The pack tests cover their
  packs. The hand of an Orbital Tag comes from `pairs()` over the poker
  hands, which LuaJIT orders in another way in each process, as for To Do
  List. Balatro4S uses one fixed order.
- **A debuffed joker leaves the deck.** `Card:set_debuff` calls
  `remove_from_deck`, so a joker that Crimson Heart debuffs or that
  perishes gives back its hand size, its slots, and its other effects.
- **The draws, the debuffs, and the checks of the blind come at the time of
  the events of Balatro.** For example, The Hook discards before the score,
  The Tooth takes the money before the score, and The Ox sets the money to
  $0 at once.
