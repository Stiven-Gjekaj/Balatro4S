// bo_joker: the effects of the jokers, a port of Card:calculate_joker,
// add_to_deck, remove_from_deck, and calculate_dollar_bonus of Balatro
// 1.0.1o.

#ifndef BO_JOKER_H
#define BO_JOKER_H

#include "bo_hand.h"
#include "bo_item.h"

struct BORun;

// The moments when Balatro asks the jokers. Each is a field of the context
// table of calculate_joker.
typedef enum {
    BO_CTX_OPEN_BOOSTER,
    BO_CTX_BUYING_CARD,
    BO_CTX_SELLING_SELF,
    BO_CTX_SELLING_CARD,
    BO_CTX_REROLL_SHOP,
    BO_CTX_ENDING_SHOP,
    BO_CTX_SKIP_BLIND,
    BO_CTX_SKIPPING_BOOSTER,
    BO_CTX_PLAYING_CARD_ADDED,
    BO_CTX_FIRST_HAND_DRAWN,
    BO_CTX_SETTING_BLIND,
    BO_CTX_DESTROYING_CARD,
    BO_CTX_CARDS_DESTROYED,
    BO_CTX_REMOVE_PLAYING_CARDS,
    BO_CTX_USING_CONSUMEABLE,
    BO_CTX_DEBUFFED_HAND,
    BO_CTX_PRE_DISCARD,
    BO_CTX_DISCARD,
    BO_CTX_END_OF_ROUND,           // With individual or repetition, or alone.
    BO_CTX_INDIVIDUAL,             // A played card or a held card.
    BO_CTX_REPETITION,
    BO_CTX_OTHER_JOKER,
    BO_CTX_BEFORE,
    BO_CTX_AFTER,
    BO_CTX_JOKER_MAIN,
} BOContextKind;

enum {
    BO_IN_PLAY = 1,    // context.cardarea == G.play
    BO_IN_HAND = 2,    // context.cardarea == G.hand
    BO_IN_JOKERS = 3,  // context.cardarea == G.jokers
};

typedef struct {
    BOContextKind kind;
    int cardarea;
    int individual;   // With BO_CTX_END_OF_ROUND.
    int repetition;   // With BO_CTX_END_OF_ROUND.
    int game_over;    // With BO_CTX_END_OF_ROUND.
    int blueprint;    // The depth of Blueprint and Brainstorm copies.
    BOItem *blueprint_card;
    // The play.
    BOCard *full_hand;
    int full_count;
    BOCard **scoring;
    int scoring_count;
    BOHandType scoring_name;
    const BOHandResult *poker_hands;
    BOCard *other_card;
    BOItem *other_joker;
    int card_effects_any;  // For Mime: the held card had an effect.
    // Other moments.
    int blind_boss;        // setting_blind: context.blind.boss.
    BOItem *consumeable;   // using_consumeable.
    BOItem *card;          // buying_card.
    int hook;              // pre_discard: a discard by The Hook.
    int glass_shattered;   // cards_destroyed and remove_playing_cards.
    int faces_removed;
    int highlighted_glass; // using_consumeable of The Hanged Man.
} BOContext;

// The table that calculate_joker returns.
typedef struct {
    int any;  // 1 when calculate_joker returned a table.
    int has_mult_mod, has_chip_mod, has_xmult_mod;
    double mult_mod, chip_mod, xmult_mod;
    int has_chips, has_mult, has_x_mult, has_h_mult;
    double chips, mult, x_mult, h_mult;
    int dollars;
    int repetitions;
    int saved;
    int remove;
    int level_up;
    int message;      // The effect showed a message, as "Upgrade!".
    int extra;        // An extra table (Hiker, Lucky Cat, 8 Ball).
    int destroyed;    // destroying_card returned true.
    int playing_cards_created;
    BOItem *card;     // The card that shows the effect.
} BOEffect;

// Card:calculate_joker. self is a joker in run->jokers or a consumable.
BOEffect bo_joker_calculate(struct BORun *run, BOItem *self, BOContext *context);

// Card:add_to_deck and Card:remove_from_deck for a joker or a consumable.
void bo_joker_add_to_deck(struct BORun *run, BOItem *item);
void bo_joker_remove_from_deck(struct BORun *run, BOItem *item);

// Card:calculate_dollar_bonus. Returns 0 when the joker gives nothing.
int bo_joker_dollar_bonus(struct BORun *run, BOItem *item, int *has);

// To Do List: set_ability picks the poker hand of the list. title is 1
// for the card on the title screen, which uses another key.
void bo_joker_set_to_do(struct BORun *run, BOItem *item, int title);

// The tallies that Card:update keeps for some jokers each frame.
void bo_joker_update(struct BORun *run, BOItem *item);

#endif
