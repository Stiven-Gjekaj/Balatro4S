# Changelog

## 0.3.0 (2026-10-18)

- The shop: two cards for sale, the voucher, two packs, the reroll, and
  Next Round. The cards of the shop come from the seed as in Balatro.
- Buy a joker or a consumable. Sell a joker or a consumable for its sell
  value.
- All 150 jokers score as in Balatro 1.0.1o. A test plays 10000 random
  hands in this code and in the code of Balatro, and all of them agree.
- The jokers and the consumables show above the table. Tap a card to see
  its name, its text, and its rarity. The text comes from the text file of
  Balatro.
- Drag a joker to put it in a new place.
- Each joker shows its chips, mult, or money when it scores.
- The vouchers and the packs show in the shop, but you cannot buy them yet.
  The packs come in 0.4, and the vouchers come in 0.5.
- The consumables cannot be used yet. They come in 0.4.
- The editions show as a colour over the card until the shaders of 0.7.

## 0.2.0 (2026-10-18)

- A full run with the Red Deck: the blind select, the Small, Big, and Boss
  blinds, and the antes 1 to 8, with the chip targets of Balatro.
- The cash out: the reward of the blind, $1 for each hand that is left, and
  the interest.
- The win screen after the boss of ante 8, the endless mode, and the game
  over screen.
- The random numbers of Balatro. A seed gives the same deck, the same
  hands, and the same bosses as in Balatro 1.0.1o.
- The seed of the run shows in the side panel.
- Sort the hand by rank or by suit.
- The boss blinds have no effect yet. Their effects come in 0.5.

## 0.1.0 (2026-10-18)

- The app reads the art and the font from `Balatro.love` on the phone. The
  .deb holds no file of Balatro.
- The table in landscape at 60 frames per second, with the cards of
  Balatro, the m6x11plus font, the side panel, and the deck.
- Deal 8 cards from a shuffled standard deck. Select up to 5 cards with a
  tap.
- The poker hand of the selected cards, with all 12 hands and the rules of
  Balatro 1.0.1o.
- Play Hand scores the hand with the base chips and mult and the chips of
  each scoring card, and shows each step. Discard changes the selected
  cards.
