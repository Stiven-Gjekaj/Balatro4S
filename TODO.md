# Roadmap

Each release adds one big part. The risky work comes first: the renderer,
the assets, and the speed on the A5. The reason for each choice is in
[docs/decisions.md](docs/decisions.md). Each release is a milestone on
GitHub, and each item below is an issue in that milestone.

The counts come from Balatro 1.0.1o: 150 jokers, 22 Tarot cards, 12 Planet
cards, 18 Spectral cards, 32 vouchers, 24 tags, 28 boss blinds, 15 decks,
8 stakes, and 20 challenges.

## 0.1: the foundation (done, 2026-10-18)

- Add the app, install, and package scripts, and the Cydia packaging.
- Read the textures and the font from `Balatro.love` on the phone.
- Draw sprites from the atlases with OpenGL ES 2.0, in landscape, at 60 frames per second.
- Draw text with the m6x11plus pixel font.
- Add the card model and the standard deck of 52 cards.
- Deal a hand of 8 cards, and select up to 5 cards with a tap.
- Find the poker hand of the selected cards, and score it with the base chips and mult.

## 0.2: a full run with the standard deck (done, 2026-10-18)

- Play and discard, with the counts of hands and discards.
- Add the blind select screen with the Small, Big, and Boss blinds.
- Add the end of a round and the cash out: the reward, the money for each hand left, and the interest.
- Add the antes 1 to 8 with the chip targets, the win screen, and the game over screen.
- Port the seeded random generator, so that a seed gives the same run as in Balatro.
- Sort the hand by rank or by suit.

## 0.3: the shop and the first jokers (done, 2026-10-18)

- Add the shop: the card slots, the voucher slot, the pack slots, and the reroll.
- Buy and sell cards, with the money and the limit of the interest.
- Add the joker engine with each trigger point of the original.
- Port the first 40 jokers.
- Move the jokers into a new order with a drag.

## 0.4: consumables and packs (done, 2026-10-18)

- Add the consumable slots and the Use button.
- Port the 22 Tarot cards.
- Port the 12 Planet cards and the hand levels.
- Port the 18 Spectral cards.
- Add the Arcana, Celestial, Spectral, Standard, and Buffoon packs in each size.
- Add the Run Info screen.

## 0.5: card modifiers, vouchers, tags, and all blinds (done, 2026-10-18)

- Add the 8 enhancements.
- Add the editions: Foil, Holographic, Polychrome, and Negative.
- Add the 4 seals.
- Port the 32 vouchers.
- Port the 24 tags and the skip of a blind.
- Port the 28 boss blinds, with the 5 final boss blinds.

## 0.6: all 150 jokers (done, 2026-10-18)

- Port the rest of the common jokers.
- Port the rest of the uncommon jokers.
- Port the rest of the rare jokers.
- Port the 5 legendary jokers.
- Compare each joker with the original in a test.

## 0.7: the feel (done, 2026-10-18)

- Draw the editions with shaders that the A5 can run at 60 frames per second.
- Add the animations: deal, draw, play, score, and the shake of a card.
- Add the moving background of the original.

## 0.7.1: sound and video

- Decode the sounds from Ogg Vorbis on the phone, and play them. This needs
  a Vorbis decoder, as stb_vorbis, in the repository.
- Play the 5 music tracks, with the changes between the shop, the packs, and the blinds.
- Record videos of the game with a screen recorder from Cydia. RecordMyScreen
  is on the phone. A person must start it with a tap.

## 0.8: progress (done, 2026-10-18)

- Port the 15 decks.
- Port the 8 stakes, with the Eternal, Perishable, and Rental stickers.
- Save the run, and resume it after a restart.
- Add the profile, the unlocks, and the discovery of the cards.
- Add the collection screens.

## 0.9: challenges and polish

- Port the 20 challenges.
- Add the statistics of the profile.
- Add the settings, with the game speed.
- Add the endless mode after ante 8.
- Measure the speed and the memory on the A5, and fix each slow screen.

## 1.0.0

- Compare all the content with Balatro 1.0.1o, and fix each difference.
- Publish the release on GitHub with the .deb, and add it to the Cydia repository.
