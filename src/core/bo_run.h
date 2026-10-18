// bo_run: the state of one run, and its steps: select a blind, play,
// discard, cash out, and shop. The screen calls these steps. The order of
// the random numbers is the order of Balatro 1.0.1o, so a seed gives the
// same cards as in the original.

#ifndef BO_RUN_H
#define BO_RUN_H

#include "bo_blind.h"
#include "bo_card.h"
#include "bo_data.h"
#include "bo_item.h"
#include "bo_random.h"
#include "bo_score.h"

#define BO_MAX_CARDS 256
#define BO_MAX_HAND 64
#define BO_MAX_JOKERS 32
#define BO_MAX_CONSUMABLES 32
#define BO_MAX_SHOP 8

typedef enum {
    BO_STATE_BLIND_SELECT,
    BO_STATE_SELECTING,   // A blind is on. The player selects cards.
    BO_STATE_ROUND_EVAL,  // The blind is beaten. The cash out waits.
    BO_STATE_SHOP,
    BO_STATE_GAME_OVER,
    BO_STATE_PACK,        // A booster pack is open.
} BORunState;

typedef enum {
    BO_SLOT_SMALL,
    BO_SLOT_BIG,
    BO_SLOT_BOSS,
} BOBlindSlot;

#define BO_EVAL_MAX_ROWS 16

typedef enum {
    BO_EVAL_BLIND,     // The reward of the blind.
    BO_EVAL_HANDS,     // $1 for each hand that is left.
    BO_EVAL_DISCARDS,  // Green Deck: $1 for each discard that is left.
    BO_EVAL_JOKER,     // A joker: Golden Joker, Rocket, and others.
    BO_EVAL_INTEREST,
    BO_EVAL_TAG,       // Investment Tag.
} BOEvalKind;

typedef struct {
    uint8_t kind;
    uint8_t index;  // The joker, for BO_EVAL_JOKER.
    int count;      // The hands or the discards.
    int dollars;
} BOEvalRow;

typedef struct {
    int row_count;
    BOEvalRow rows[BO_EVAL_MAX_ROWS];
    int total;
} BORoundEval;

// A card in the shop: a joker, a consumable, or a playing card.
typedef struct {
    int is_playing_card;
    BOItem item;
    BOCard card;  // When is_playing_card. item then has the cost.
} BOShopCard;

// A tag that waits (G.GAME.tags).
typedef struct {
    BOCenterId center;
    int orbital_hand;  // Orbital Tag: a BOHandType.
    int triggered;
} BOTag;

// The data of each poker hand in the run (G.GAME.hands).
typedef struct {
    int level;
    int played;
    int played_this_round;
    int visible;
} BOHandStats;

typedef struct BORun {
    BORandom random;
    BORunState state;
    int stake;
    int win_ante;
    int won;
    int ante;
    int round;
    BOBlindSlot slot;
    BOBlindId boss;
    BOBlindId blind;
    double blind_chips;
    int blind_dollars;
    int blind_disabled;
    int blind_triggered;
    int blind_active;      // A blind is on: from its choice to its defeat.
    int boss_prepped;      // Blind.prepped: The Fish and Crimson Heart.
    int water_sub;         // The discards that The Water took.
    int needle_sub;        // The hands that The Needle took.
    uint8_t eye_hands[BO_HAND_COUNT];  // The Eye: the hands played.
    int mouth_hand;        // The Mouth: the one hand, or BO_HAND_NONE.
    double chips;          // The score of this round.

    // round_resets and current_round.
    int hands_per_round;
    int discards_per_round;
    int hands_left;
    int discards_left;
    int hands_played;      // In this round.
    int discards_used;     // In this round.
    int hand_size;         // G.hand.config.real_card_limit. It can be below 0.
    int reroll_cost_base;  // round_resets.reroll_cost.
    int reroll_cost;       // current_round.reroll_cost.
    int reroll_cost_increase;
    int free_rerolls;
    int jokers_purchased;
    int round_bonus_hands;     // round_bonus.next_hands.
    int round_bonus_discards;  // round_bonus.discards.
    int idol_rank, idol_suit;  // The Idol.
    int mail_rank;             // Mail-In Rebate.
    int ancient_suit;          // Ancient Joker.
    BOCenterId last_tarot_planet;  // The Fool makes this card.
    int ecto_minus;            // The hand size that the next Ectoplasm takes.
    int castle_suit;           // Castle.
    BOHandType most_played_hand;
    BOHandType last_hand_played;

    // G.GAME.
    int dollars;
    int bankrupt_at;
    int interest_cap;
    int interest_amount;
    int inflation;
    int discount_percent;
    double probabilities;  // G.GAME.probabilities.normal.
    double edition_rate;
    double joker_rate, tarot_rate, planet_rate, spectral_rate, playing_card_rate;
    int consumeable_buffer;
    int joker_buffer;
    int dollar_buffer;
    // ease_dollars of Balatro changes the money in an event, after the
    // score. While defer_dollars is set, the money waits in pending_dollars.
    int defer_dollars;
    int pending_dollars;
    int hands_played_total;  // G.GAME.hands_played.
    int unused_discards;
    int skips;
    int tarots_used;         // consumeable_usage_total.tarot.
    int planets_used_unique; // The kinds of planet cards used, for Satellite.
    int starting_deck_size;
    int shop_joker_max;
    int gros_michel_extinct; // G.GAME.pool_flags.gros_michel_extinct.
    int first_shop_buffoon;
    uint8_t used_jokers[BO_CENTER_COUNT];   // The cards on the table or in the shop.
    uint8_t used_vouchers[BO_CENTER_COUNT];
    uint8_t planet_used[BO_CENTER_COUNT];
    int bosses_used[BO_BLIND_COUNT];
    BOHandStats hands[BO_HAND_COUNT];
    BOScoreContext context;  // The levels and the rules of the hands.
    int sort_by_suit;
    uint32_t next_sort_id;   // G.sort_id.
    uint32_t next_id;        // The node IDs.
    BORoundEval eval;
    BOCenterId voucher;      // current_round.voucher.
    int boss_rerolled;       // round_resets.boss_rerolled: Director's Cut.

    // The cards.
    BOCard deck[BO_MAX_CARDS];  // The top is the last card.
    int deck_count;
    BOCard hand[BO_MAX_HAND];
    int hand_count;
    BOCard discard[BO_MAX_CARDS];
    int discard_count;
    BOCard play[BO_MAX_PLAY + 8];  // The played cards while they score.
    int play_count;
    BOItem jokers[BO_MAX_JOKERS];
    int joker_count;
    int joker_slots;
    BOItem consumables[BO_MAX_CONSUMABLES];
    int consumable_count;
    int consumable_slots;

    // The shop.
    BOShopCard shop[BO_MAX_SHOP];
    int shop_count;
    BOItem shop_voucher;
    int has_shop_voucher;
    BOItem shop_packs[2];
    int shop_pack_count;
    BOCenterId used_packs[2];  // current_round.used_packs, BO_CENTER_NONE for USED.
    int used_packs_set;

    // The open booster pack (BO_STATE_PACK).
    BOCenterId pack;
    BOShopCard pack_cards[5];
    int pack_count;
    int pack_choices;        // G.GAME.pack_choices.
    int pending_hallucinations;  // Tarot cards of Hallucination that wait.
    BORunState pack_return;  // G.GAME.PACK_INTERRUPT.

    // The tags that wait (G.GAME.tags), and the tags of the blinds.
    BOTag tags[32];
    int tag_count;
    BOCenterId blind_tags[2];  // round_resets.blind_tags: Small and Big.
    int orbital_choices[3];    // The hand of an Orbital Tag of each blind.
    int orbital_ante;          // The ante of orbital_choices.
    int temp_handsize;         // Juggle Tag: the hand size of this round.
    int temp_reroll_cost;      // D6 Tag: the base reroll cost, or -1.
    int shop_free;             // Coupon Tag acted in this shop.
    int shop_d6ed;             // D6 Tag acted in this shop.
    BOItem tag_vouchers[4];    // Voucher Tag: more vouchers in this shop.
    int tag_voucher_count;
} BORun;

// Starts a run of the Red Deck at stake 1 with seed, for example
// "ABCDEFGH".
void bo_run_start(BORun *run, const char *seed);

// The blind of a slot in this ante.
BOBlindId bo_run_slot_blind(const BORun *run, BOBlindSlot slot);

// The chip target of the blind of a slot in this ante.
double bo_run_slot_chips(const BORun *run, BOBlindSlot slot);

// Starts the blind on deck: the deck shuffles, and the hand fills.
void bo_run_select_blind(BORun *run);

// Plays the cards of the hand that mask selects (bit i is hand[i]). score
// gets the result. Returns 0 when the play is not allowed.
int bo_run_play(BORun *run, uint64_t mask, BOScore *score);

// Discards the cards that mask selects. Returns 0 when it is not allowed.
int bo_run_discard(BORun *run, uint64_t mask);
// The discard of G.FUNCS.discard_cards_from_highlighted. With hook (The
// Hook), it uses no discard.
void bo_run_discard_cards(BORun *run, uint64_t mask, int hook);
// G.FUNCS.draw_from_deck_to_hand(count): draws count cards.
void bo_run_draw_cards(BORun *run, int count);

// Takes the money of the round and opens the shop.
void bo_run_cash_out(BORun *run);

// Sorts the hand by rank, or by suit, from high to low.
void bo_run_sort_hand(BORun *run, int by_suit);

// The shop.
int bo_run_reroll(BORun *run);
// Buys shop card index. Returns 0 when there is no money or no room.
int bo_run_buy(BORun *run, int index);
// Sells joker index. Returns 0 when the joker is eternal.
int bo_run_sell_joker(BORun *run, int index);
int bo_run_sell_consumable(BORun *run, int index);
// Makes the cards of a new shop, with the effects of the tags.
void bo_run_fill_shop(BORun *run);
// Buys and opens shop pack index. See bo_pack.h for the open pack.
int bo_run_open_pack(BORun *run, int index);
// Uses consumable index on the cards of the hand that highlighted selects.
// Returns 0 when the card cannot be used now.
int bo_run_use_consumable(BORun *run, int index, uint64_t highlighted);
// can_use_consumeable and check_use of consumable index.
int bo_run_can_use_consumable(const BORun *run, int index, uint64_t highlighted);
// Leaves the shop and goes to the next blind select.
void bo_run_end_shop(BORun *run);
// Moves joker from to the place to, as a drag on the screen does.
void bo_run_move_joker(BORun *run, int from, int to);

// Card:remove: the center is not used any more when no joker or
// consumable has it.
void bo_run_forget_card(BORun *run, BOCenterId center);
// Removes the jokers that an effect destroyed.
void bo_run_remove_marked_jokers(BORun *run);

// The number of dollars that the player can spend: dollars - bankrupt_at.
int bo_run_spendable(const BORun *run);

// Makes a card of a set, as create_card does. type is a set, as
// BO_SET_JOKER, or -1 for "Base" playing cards. rarity is 0 for a random
// rarity. forced is a center or BO_CENTER_NONE. key_append is the text
// that Balatro adds to the key of the random state, as "sho".
BOItem bo_run_create_item(BORun *run, int type, int rarity, int legendary, int soulable, BOCenterId forced,
                          const char *key_append, int in_shop);

// Adds a joker or a consumable that was made, with add_to_deck.
void bo_run_add_joker(BORun *run, const BOItem *item);
void bo_run_add_consumable(BORun *run, const BOItem *item);

// Removes joker index, with remove_from_deck.
void bo_run_remove_joker(BORun *run, int index);

void bo_run_ease_dollars(BORun *run, int amount);
void bo_run_level_up_hand(BORun *run, BOHandType type, int amount);

// The count of jokers with this center that are not debuffed.
int bo_run_find_joker(const BORun *run, BOCenterId center);

// A new id for a card, as the node IDs of Balatro.
uint32_t bo_run_new_id(BORun *run);
uint32_t bo_run_new_sort_id(BORun *run);

#endif
