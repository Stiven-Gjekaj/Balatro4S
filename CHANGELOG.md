# Changelog

## 1.0.0 (2026-10-18)

- The text of each card agrees with Balatro. A new tool compares the text of
  the 266 jokers, consumables, vouchers, and packs with the code of
  Balatro.
- The Fool shows the last Tarot or Planet card, or None.
- Misprint shows a Mult number that changes, as in Balatro.
- Fix: an X Mult box has no spaces in it.

## 0.10.1 (2026-10-18)

- The icon has the rounded corners and the gloss of iOS 6, also in Cydia.

## 0.10.0 (2026-10-18)

- The sounds and the music of Balatro. The five music tracks change with
  the blind select, the boss blinds, the shop, and the packs, and the music
  slows at a game over. The cards, the buttons, the score, the shop, the
  packs, and the blinds play their sounds at the pitch of Balatro.
- The settings set the volume, the music, and the sounds.
- The ring/silent switch mutes the game.
- Fix: a tap on an empty place in the collection does not show the text of
  a card.

## 0.9.0 (2026-10-18)

- The 20 challenges, with their rules, cards, and banned cards. The start
  of each challenge agrees with Balatro. Five deck wins open the first five
  challenges.
- The statistics screen shows the high scores, the progress, and the
  career stats of the profile.
- The settings screen sets the game speed and the screen shake.
- Endless Mode goes on after the win at ante 8. The blind targets agree
  with Balatro up to ante 40.
- The app keeps the decoded textures in a cache. A start takes 0.83 s in
  place of 3.95 s, after the first start.
- Fix: the win screen shows the ante of the win.

## 0.8.0 (2026-10-18)

- The 15 decks and the 8 stakes, with the Eternal, Perishable, and Rental
  stickers. A setup screen chooses the deck and the stake of a new run.
  The start of a run with each deck and stake agrees with Balatro.
- The run saves itself and goes on after a restart.
- The profile counts the stats and the wins of each deck, and unlocks the
  decks, the stakes, the jokers, and the vouchers as Balatro does. A banner
  shows each unlock. Unlock All needs a second tap.
- The collection shows the jokers, decks, vouchers, Tarot, Planet, and
  Spectral cards, packs, tags, and blinds that the profile discovered.
- Fix: the first suit of Ancient Joker can be Spades.
- Fix: each Investment Tag pays its own row of money.
- Fix: the blind pays no money when Mr. Bones saves the run.

## 0.7.0 (2026-10-18)

- The moving background of Balatro, in the colours of the blind and the
  pack, at 60 frames per second.
- The cards move with the springs of Balatro. A card leans when it moves
  and shakes when it scores. The hand fans and sways, and the cards come
  from the deck one after another.
- The room and the chips and mult boxes shake when a card scores.
- The sounds and the music move to 0.7.1. They need a Vorbis decoder.

## 0.6.0 (2026-10-18)

- Each of the 149 jokers agrees with Balatro in its own tests: 1490 plays,
  1490 rounds with a discard, and 894 rounds with a random blind. To Do
  List cannot be compared, because Balatro itself picks its hand in another
  order in each start of the game.
- Fix: a discard of no card uses no discard.
- Fix: the blind is defeated after the held Gold cards and the jokers pay,
  so a debuffed Gold card gives no money.
- Fix: a draw of no card does not sort the hand.
- The Foil, Holographic, Polychrome, and Negative editions, the shine of
  the vouchers, the packs, the Spectral cards, and the Gold Seal use the
  shaders of Balatro, at 60 frames per second.

## 0.5.0 (2026-10-18)

- The 28 boss blinds and the 5 final boss blinds act as in Balatro. The
  blind select and the round show the text of the boss.
- Buy the 32 vouchers in the shop. Director's Cut and Retcon add a Reroll
  Boss button to the blind select.
- Skip the Small Blind or the Big Blind for its tag. The 24 tags act as in
  Balatro: free jokers, free packs, editions, money, hand levels, and more.
  Tap a tag to see its text.
- Tests compare 1000 plays with random blinds, 500 vouchers, and 1000 skips
  with random tags with the code of Balatro, and all of them agree.
- Fix: a debuffed or perished joker leaves the deck, as in Balatro, so its
  effects stop.
- Fix: the discards left never go below 0 when a joker leaves.

## 0.4.0 (2026-10-18)

- Use the 22 Tarot cards, the 12 Planet cards, and the 18 Spectral cards.
  Tap a card in the consumable slots, select the cards of the hand, and tap
  Use. A test uses 2000 random cards in this code and in the code of
  Balatro, and all of them agree.
- Open the Arcana, Celestial, Spectral, Standard, and Buffoon packs in each
  size from the shop. Choose the cards of the pack, or skip it. A test opens
  2000 random packs, and all of them agree with Balatro.
- The Planet cards level up the poker hands. Run Info shows the level, the
  chips, the mult, and the plays of each hand.
- The text of each Tarot, Planet, Spectral, pack, and voucher card.
- A larger hand draws its new cards at once, as in Balatro.
- Fix: a playing card in the shop takes one sort_id, as in Balatro.
- The vouchers show in the shop, but you cannot buy them yet. They come in
  0.5.

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
