![Balatro4S](assets/brand/banner.png)

# Balatro4S

Balatro4S is a demake of [Balatro](https://www.playbalatro.com/) for the
iPhone 4S on iOS 6.1.3 (jailbroken). It is a native rewrite in C and
Objective-C, with OpenGL ES 2.0. You install it as a .deb package.

This is a fan project. It is not from LocalThunk or Playstack.

**The status:** early work. Version 0.8.0 plays a full run of 8 antes with
each of the 15 decks at each of the 8 stakes, and the seeds of Balatro. It has the shop, all 150
jokers, all Tarot, Planet, and Spectral cards, all booster packs, all
vouchers, all tags, and all boss blinds. Each joker agrees with Balatro in
its own tests. The editions and the background use the shaders of
Balatro, and the cards move as in Balatro. The run saves itself, and the
profile unlocks the cards as Balatro does. The sounds and the challenges
come in the next versions. The plan is in
[TODO.md](TODO.md), and the decisions and their reasons are in
[docs/decisions.md](docs/decisions.md).

<img src="assets/screenshots/blinds.png" width="400" alt="The blind select: the Small Blind is defeated, the Big Blind waits with a Select button, and the boss The Pillar is upcoming"> <img src="assets/screenshots/table.png" width="400" alt="A round: the blind and its target in the side panel, eight cards in the hand, the Play Hand and Discard buttons, and the sort buttons">
<img src="assets/screenshots/shop.png" width="400" alt="The shop: Next Round, Reroll, two jokers for sale, a voucher, two packs, and the text of the Mime joker with a Buy button"> <img src="assets/screenshots/jokers.png" width="400" alt="A hand scores: the King adds 10 chips and the Joker adds 4 Mult">
<img src="assets/screenshots/tarot.png" width="400" alt="The Magician with its text and the Use and Sell buttons, over a hand with two selected cards"> <img src="assets/screenshots/pack.png" width="400" alt="An open Spectral Pack: the text of Talisman with a Use button, and the hand under the pack">
<img src="assets/screenshots/tags.png" width="400" alt="The blind select: the Small Blind is skipped, the Big Blind has its tag and a Skip Blind button, and the text of The Manacle shows under the boss"> <img src="assets/screenshots/arcana.png" width="400" alt="An open Arcana Pack over the purple background, with the hand in a fan under the pack">
<img src="assets/screenshots/setup.png" width="400" alt="The New Run screen: the Magic Deck with its text, the White Stake, and the Play button"> <img src="assets/screenshots/collection.png" width="400" alt="The collection: the Jokers tab with the Joker discovered and the other jokers not discovered, and the text of the Joker">

## You need a copy of Balatro

This repository and the .deb hold no file of Balatro. The art, the sounds,
the font, and the text belong to their owners. Balatro4S reads them from
the `Balatro.love` file of your own copy.

1. Buy Balatro, for example on Steam.
2. Find `Balatro.love`. On a Mac, it is in
   `Balatro.app/Contents/Game/Balatro.app/Contents/Resources/`. On
   Windows, `Balatro.exe` is a zip archive that holds the same files.
3. Copy it to `/var/mobile/Media/Balatro4S/Balatro.love` on the phone,
   with an AFC tool or with `scp`.

Balatro4S follows Balatro 1.0.1o.

## Install

1. Download the .deb from the latest
   [release](https://github.com/Stiven-Gjekaj/Balatro4S/releases).
2. Copy it to the phone, for example with `scp`.
3. Install it as root: `dpkg -i io.github.stivengjekaj.balatro4s_0.2.0_iphoneos-arm.deb`.

The APT of Cydia on iOS 6 speaks only TLS 1.0, and GitHub accepts only TLS
1.2, so `dpkg -i` is the way on iOS 6.

## Build

You need a Mac with Xcode, the iPhoneOS 9.3 SDK in `sdks/`, and a
jailbroken iPhone at `ssh iphone4s`.

    ./fix-sdk.sh           # one time, after you unpack the SDK
    scripts/test.sh        # the tests on the Mac
    scripts/test.sh --phone
    scripts/install.sh     # build, install, and open the app on the phone
    scripts/deb.sh         # build the .deb

## Licence

The code is under the MIT licence. See [LICENSE](LICENSE).
