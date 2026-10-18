<div align="center">

<img src="assets/brand/banner.png" alt="Balatro4S" width="640">

### A demake of Balatro for the iPhone 4S on iOS 6

_Native C and OpenGL ES 2.0, the rules of Balatro, your own copy of the game_

![iOS 6](https://img.shields.io/badge/ios-6.1-1f5fc4?style=for-the-badge&logo=apple&logoColor=ffffff)
![armv7](https://img.shields.io/badge/armv7-d8261c?style=for-the-badge)
![C](https://img.shields.io/badge/c-1a1a2e?style=for-the-badge&logo=c&logoColor=ffffff)
![OpenGL ES 2.0](https://img.shields.io/badge/opengl_es-2.0-5586a4?style=for-the-badge&logo=opengl&logoColor=ffffff)
![Cydia](https://img.shields.io/badge/cydia-6d83a2?style=for-the-badge)
[![MIT licence](https://img.shields.io/badge/mit_licence-3a3f47?style=for-the-badge)](LICENSE)
[![Release](https://img.shields.io/github/v/release/Stiven-Gjekaj/Balatro4S?style=for-the-badge&label=release&color=16181c)](https://github.com/Stiven-Gjekaj/Balatro4S/releases/latest)
[![Ko-fi](https://img.shields.io/badge/ko--fi-support_this_project-f26a1b?style=for-the-badge&logo=ko-fi&logoColor=ffffff)](https://ko-fi.com/stivengjekaj)

<p align="center">
  <a href="#overview"><b>Overview</b></a> |
  <a href="#you-need-a-copy-of-balatro"><b>Balatro.love</b></a> |
  <a href="#install"><b>Install</b></a> |
  <a href="#build"><b>Build</b></a> |
  <a href="docs/decisions.md"><b>Decisions</b></a> |
  <a href="TODO.md"><b>Roadmap</b></a>
</p>

</div>

---

> [!NOTE]
> **Version 0.10.0 is out.** It plays full runs with all 150 jokers, all
> decks and stakes, the 20 challenges, Endless Mode, and the sounds and the
> music of Balatro. Version 1.0 compares all the content with Balatro once
> more. [CHANGELOG.md](CHANGELOG.md) lists the changes of each release.
> [docs/decisions.md](docs/decisions.md) holds each decision and its reason.

> [!IMPORTANT]
> This is a fan project. It is not from LocalThunk or Playstack. The
> repository and the .deb hold no file of Balatro. You need your own copy.

---

<table>
<tr>
<td width="33%" valign="top">
<img src="assets/screenshots/table.png" alt="A round: the blind and its target in the side panel, eight cards in the hand, the Play Hand and Discard buttons, and the sort buttons">
<p align="center"><i>A round</i></p>
</td>
<td width="33%" valign="top">
<img src="assets/screenshots/jokers.png" alt="A hand scores: the King adds 10 chips and the Joker adds 4 Mult">
<p align="center"><i>A hand scores</i></p>
</td>
<td width="33%" valign="top">
<img src="assets/screenshots/shop.png" alt="The shop: Next Round, Reroll, two jokers for sale, a voucher, two packs, and the text of the Mime joker with a Buy button">
<p align="center"><i>The shop</i></p>
</td>
</tr>
<tr>
<td width="33%" valign="top">
<img src="assets/screenshots/blinds.png" alt="The blind select: the Small Blind is defeated, the Big Blind waits with a Select button, and the boss The Pillar is upcoming">
<p align="center"><i>The blind select</i></p>
</td>
<td width="33%" valign="top">
<img src="assets/screenshots/arcana.png" alt="An open Arcana Pack over the purple background, with the hand in a fan under the pack">
<p align="center"><i>An Arcana Pack</i></p>
</td>
<td width="33%" valign="top">
<img src="assets/screenshots/setup.png" alt="The New Run screen: the Magic Deck with its text, the White Stake, and the Play button">
<p align="center"><i>A new run</i></p>
</td>
</tr>
</table>

## Overview

**Balatro4S** plays [Balatro](https://www.playbalatro.com/) on an iPhone 4S
with iOS 6.1.3. Balatro itself does not run on iOS 6. Balatro4S is a new
engine in C, with the rules of Balatro 1.0.1o.

The app reads the art, the font, the text, the shaders, and the sounds from
the `Balatro.love` file of your own copy. It holds no file of Balatro.

The rules agree with the Lua code of Balatro itself. A test harness runs the
real code of Balatro on the Mac with the LuaJIT of the game, and compares it
with the C code over thousands of random seeds: the score of each hand, each
joker, each blind, each tag, pack, and voucher, the start of each deck,
stake, and challenge, and the money of each round. The same seed gives the
same run as in Balatro.

The game runs at 60 frames each second on the A5. The editions and the
background use the shaders of Balatro. After the first start, the app
starts in about 1 second.

## Features

- All 150 jokers, 22 Tarot cards, 12 Planet cards, 18 Spectral cards, 32
  vouchers, 24 tags, and all booster packs
- The 28 boss blinds and the 5 final boss blinds
- The 15 decks and the 8 stakes, with the Eternal, Perishable, and Rental
  stickers
- The 20 challenges, and Endless Mode after ante 8
- The seeds of Balatro
- The Foil, Holographic, Polychrome, and Negative editions and the moving
  background, with the shaders of Balatro
- The cards move, lean, and shake with the springs of Balatro
- The sounds and the five music tracks of Balatro, decoded on the phone
- The run saves itself and comes back after a restart
- A profile with the stats, the high scores, the wins of each deck, the
  unlocks, and the discoveries, as in Balatro
- The collection, the statistics, and the settings: game speed, screen
  shake, volume, music, and sounds

Version 0.10.0 was checked on an iPhone 4S with iOS 6.1.3.

## You need a copy of Balatro

The art, the sounds, the font, and the text of Balatro belong to their
owners. Balatro4S reads them from `Balatro.love`.

1. Buy Balatro, for example on Steam.
2. Find `Balatro.love`. On a Mac, it is in
   `Balatro.app/Contents/Game/Balatro.app/Contents/Resources/`. On Windows, `Balatro.exe` is a zip
   archive that holds the same files.
3. Copy it to `/var/mobile/Media/Balatro4S/Balatro.love` on the phone, with
   an AFC tool or with `scp`.

On its first start, the app decodes the textures and keeps them in
`/var/mobile/Media/Balatro4S/cache/`, about 31 MB. The run, the profile,
and the settings are in the same directory.

## Install

You need an iPhone, iPod touch, or iPad with iOS 6 and a jailbreak.

1. Download the `.deb` file from the
   [latest release](https://github.com/Stiven-Gjekaj/Balatro4S/releases/latest).
2. Copy it to the phone, for example with `scp` to `/tmp/`.
3. On the phone, as root: `dpkg -i /tmp/io.github.stivengjekaj.balatro4s_*.deb`.

The Balatro4S icon comes on the home screen. To remove the app:
`dpkg -r io.github.stivengjekaj.balatro4s`.

On iOS 7 or later, or with an APT that has TLS 1.2, you can also add the
Cydia source `https://stiven-gjekaj.github.io/cydia/`. Cydia then shows the
updates. Cydia on iOS 6 cannot read it, because its APT stops at TLS 1.0
and GitHub accepts only TLS 1.2. See
[the repository](https://github.com/Stiven-Gjekaj/cydia).

## Build

You need a Mac with Xcode and the iOS 9.3 SDK of Theos.

1. Download `iPhoneOS9.3.sdk.tar.xz` from the releases of
   [theos/sdks](https://github.com/theos/sdks/releases) and unpack it into
   `sdks/`. The result is `sdks/iPhoneOS9.3.sdk`. Git does not track `sdks/`.
2. Run `./fix-sdk.sh` one time. It changes the SDK stubs for the new Apple
   linker and for iOS 6.
3. Build:

| Command | Result |
| ------- | ------ |
| `scripts/app.sh` | `build/Balatro4S.app` |
| `scripts/deb.sh` | The Cydia package in `build/` |
| `scripts/test.sh` | The tests, on the Mac |
| `scripts/test.sh --phone` | The tests, on the phone |
| `scripts/install.sh` | Builds the app, copies it to the phone, and opens it |
| `scripts/install.sh --debug` | The same, with the debug commands of `src/game/bo_debug.c` |
| `scripts/tap.sh` | Sends debug commands, as `"tap X Y"`, to a debug build |
| `scripts/shot.sh` | Saves a screenshot of the phone |
| `scripts/icons.sh` | The icons of the app |
| `scripts/release.sh` | Tests, packs, tags, and publishes a release |

The phone scripts use `ssh iphone4s`. Set up that host in `~/.ssh/config`,
for example with a USB tunnel of `iproxy` on port 2222.

`build.sh` builds one armv7 binary. It compiles with ARC and links without
ARC, and all code is in ARM mode (`-marm`). The new Apple linker makes Thumb
code that crashes on iOS 6 before `main` starts.

The tools in `tools/` compare the C code with Balatro. They need Balatro in
`/Applications` on the Mac, or its path in `BALATRO_APP`:

| Command | Result |
| ------- | ------ |
| `tools/oracle.sh score FILE` | Runs a scenario with the code of Balatro |
| `tools/fuzz.py` | Compares random scenarios of the C code and of Balatro |
| `tools/oracle.sh gen src/core` | Makes the card data in `src/core/bo_data.c` |
| `tools/oracle.sh genchallenges src/core` | Makes the challenge data |

## Support

If you find Balatro4S useful, you can support its development here.

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/stivengjekaj)

## Licence

The code is under the MIT licence. See [LICENSE](LICENSE). Balatro4S uses
stb_vorbis by Sean Barrett under the MIT licence or the public domain, as
[src/third_party/README.md](src/third_party/README.md) tells. Balatro and
its art, sounds, font, and text belong to LocalThunk and Playstack.
