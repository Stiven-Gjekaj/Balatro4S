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
