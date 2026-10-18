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

## Each joker against Balatro

- **Each of the 149 jokers agrees with Balatro in its own runs** (0.6,
  2026-10-18). `tools/fuzz.py --each N` puts each joker in N scenarios with
  random other jokers. `--round` adds a discard before the play and the
  end of the round after it. On the final code:
  - `--each 10`: 1490 of 1490 plays agree.
  - `--round --each 10`: 1490 of 1490 rounds agree.
  - `--blind --round --each 6`: 894 of 894 rounds with a random blind
    agree.
  To Do List is the 150th joker. Its hand comes from `pairs()` over
  strings, so the fuzzer does not test it (see "The jokers").
- **The fuzzer found faults that the earlier tests did not see.** A
  discard of no card used a discard, the blind was defeated before the
  held Gold cards paid, and an empty draw sorted the hand. Each fix has its
  own commit and test.

## The shaders

- **The editions and the shines use the shaders of Balatro.love** (0.7,
  early part, 2026-10-18). The app reads `resources/shaders/*.fs` from the
  Balatro.love of the owner, and compiles them for OpenGL ES 2.0 with a
  short header that maps the LOVE names to GLSL ES. The repository holds
  no shader of Balatro.
- **A cache keeps the shaders at 60 frames per second.** On the A5, one
  card with the foil shader takes about 7 ms, so 5 cards gave 18 frames
  per second. Each card with an effect renders into a slot of a 1024 x
  1024 texture: the shines at half size, and the Negative cards at full
  size because they replace the card. Each frame renders again the two
  slots that waited longest. With 10 cards with editions and a Spectral
  card, the game runs at 60 frames per second.
- **The background renders at 120 x 80 before the screen clears** (0.7,
  2026-10-18). The background shader of Balatro over the full screen of
  960 x 640 is too slow for the A5. At 240 x 160 it took about 10 ms each
  frame, and the game ran at 37 frames per second. At 120 x 80 the game
  runs at 60 frames per second. The pass into the small texture comes
  before the clear of the screen: a change of framebuffer after the clear
  makes the GPU store the screen and load it again. The colours follow
  `ease_background_colour_blind`.

## The motion

- **The cards move with the springs of Balatro** (0.7, 2026-10-18).
  `bo_motion` ports `move_xy`, `move_r`, `move_scale`, and `juice_up` of
  `Moveable` with the same constants, in the units of the game. A card
  leans in the direction that it moves, and it shakes when it scores. The
  hand fans and sways as `align_cards` puts it. The screen shake follows
  `update_canvas_juice` with the screen shake setting at 50, as a new
  profile starts.
- **The turn of a card is a transform in `bo_gfx`.** All the layers of a
  card, its edition, its seal, and its shadow turn as one. The effect
  cache renders its slots upright, and the transform applies when the
  slot draws.

## The decks, the stakes, the saves, and the profile

- **The oracle runs the real `Game:start_run`** (0.8, 2026-10-18).
  `tools/start.lua` replaces only the parts of the screen that start_run
  calls, as the HUD and the background, with stubs. The deck, the vouchers,
  the consumables, the Erratic cards, the stake, the boss, and the tags come
  from the code of Balatro. `tools/fuzz.py --start` compares random seeds,
  decks, and stakes. The test found a fault that every run had: the first
  pick of the suit of Ancient Joker did not allow Spades.
- **The round fuzz compares `evaluate_round` too.** After `end_round`, the
  oracle runs the money of the round. This found two faults: the money of
  two Investment Tags, and the blind money when Mr. Bones saves the run.
- **A save is the bytes of `BORun` with a header.** `BORun` holds no
  pointer, so the save writes it as it is. The header holds a fingerprint
  of the layout. A save from a build with another layout does not load, and
  the player starts a new run. The game writes the save when the run
  changes and no animation is on, and removes it when the run is lost.
- **The profile is an active pointer in the core.** The run calls it where
  Balatro calls `inc_career_stat`, `discover_card`, and `check_for_unlock`.
  With no active profile, as in the tests and the oracle, all cards are
  unlocked and nothing is counted, so the fuzz results do not change.
- **Unlock All is on the setup screen, with a second tap.** Balatro has it
  in the profile menu. The demake has no profile menu yet.

## The challenges, the settings, and the speed

- **The challenges come from `challenges.lua` in a generator** (0.9,
  2026-10-18). `tools/gen_challenges.lua` writes the rules, the cards, and
  the banned keys of the 20 challenges into C tables. It writes no text of
  the descriptions. The start of each challenge agrees with the real
  `Game:start_run` in `tools/fuzz.py --start`, and the custom rules agree
  in the round fuzz with `rules N`.
- **A challenge counts no stats, unlocks, or discoveries.** This is as
  Balatro does. The high scores still count. A win marks the challenge as
  complete. Five deck wins open five challenges, and each completion opens
  one more.
- **The settings are a file of their own, `settings.b4t`.** They hold the
  game speed (0.5, 1, 2, or 4) and the screen shake. They are not part of
  the profile, so a new profile keeps them.
- **The decoded textures stay in a cache beside `Balatro.love`.** The
  ImageIO decode of the PNG files took 3.5 s of a 3.95 s start on the A5.
  The cache holds the resized RGBA pixels of each atlas, about 32 MB, in
  `/var/mobile/Media/Balatro4S/cache/`. The size and the time of
  `Balatro.love` are in the header of each file, so a new `Balatro.love`
  makes the cache old. The first start takes 5.8 s and writes the cache.
  A later start takes 0.83 s.

## The sound

- **stb_vorbis decodes the Ogg files of Balatro** (2026-10-18). It is one
  C file with the MIT or the public domain license, at a fixed commit in
  `src/third_party`. The mixer includes it, so the tests on the Mac build
  it too.
- **The mixer follows `engine/sound_manager.lua`.** Each play is a voice
  with a pitch and a volume, as a LOVE source. The pitch changes the speed
  of the samples with a linear filter. The five music tracks play at the
  same time at pitch 0.7 and volume 0.6. Each frame fades the track of the
  state in and the others out, at the speed of `dt * 3`, as `MODULATE`
  does. A game over slows the music to half speed.
- **The music streams, and the short sounds are in memory.** A track that
  nobody can hear decodes nothing. When it fades in again, the decoder
  seeks to the place of the other tracks. The short sounds use about
  10 MB. The thread that loads them takes 2.6 s on the A5, after the
  start, so the start does not wait for it.
- **The audio session is "ambient sound".** The ring/silent switch mutes
  the game, and the music of another app goes on under it.
- **The sounds play at the events of the game, with the pitch and the
  volume of Balatro.** The ambient sounds of the fire and the organ are not
  in the game, because the game has no flames on the score yet.

## The comparison for 1.0

- **The text of each card agrees with the code of Balatro** (1.0,
  2026-10-18). `tools/oracle.sh describe SEED` prints the text of the 266
  jokers, consumables, vouchers, and packs with
  `Card:generate_UIBox_ability_table` of Balatro. `tools/describe.c` prints
  the same with `bo_describe`. The comparison found three faults: the
  spaces in an X box, the last card of The Fool, and the number of
  Misprint. The other differences come from the state of the run in the
  harness, as the cards in the deck.
- **All the fuzz modes agree before the release.** 300 cases of each of the
  eight modes, 2400 cases, agree with Balatro.
