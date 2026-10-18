// bo_profile: the profile of the player, as G.PROFILES of Balatro 1.0.1o:
// the career stats, the high scores, the wins of each deck at each stake,
// the cards that are unlocked and discovered, and check_for_unlock.
//
// The run calls the active profile at the places where Balatro calls
// inc_career_stat, check_and_set_high_score, discover_card, and
// check_for_unlock. With no active profile, as in the tests and the
// oracle, all cards are unlocked and discovered and nothing is counted.

#ifndef BO_PROFILE_H
#define BO_PROFILE_H

#include "bo_run.h"

// career_stats, in the order of game.lua. The names are the types of the
// unlock conditions, as "c_cards_played".
typedef enum {
    BO_STAT_ROUND_INTEREST_CAP_STREAK,
    BO_STAT_DOLLARS_EARNED,
    BO_STAT_SHOP_DOLLARS_SPENT,
    BO_STAT_TAROTS_BOUGHT,
    BO_STAT_PLANETS_BOUGHT,
    BO_STAT_PLAYING_CARDS_BOUGHT,
    BO_STAT_VOUCHERS_BOUGHT,
    BO_STAT_TAROT_READING_USED,
    BO_STAT_PLANETARIUM_USED,
    BO_STAT_SHOP_REROLLS,
    BO_STAT_CARDS_PLAYED,
    BO_STAT_CARDS_DISCARDED,
    BO_STAT_LOSSES,
    BO_STAT_WINS,
    BO_STAT_ROUNDS,
    BO_STAT_HANDS_PLAYED,
    BO_STAT_FACE_CARDS_PLAYED,
    BO_STAT_JOKERS_SOLD,
    BO_STAT_CARDS_SOLD,
    BO_STAT_SINGLE_HAND_ROUND_STREAK,
    BO_STAT_COUNT,
} BOStat;

// high_scores.
typedef enum {
    BO_HIGH_HAND,
    BO_HIGH_FURTHEST_ROUND,
    BO_HIGH_FURTHEST_ANTE,
    BO_HIGH_MOST_MONEY,
    BO_HIGH_BOSS_STREAK,
    BO_HIGH_WIN_STREAK,
    BO_HIGH_CURRENT_STREAK,
    BO_HIGH_COUNT,
} BOHighScore;

#define BO_DECK_COUNT 15
#define BO_STAKE_COUNT 8
#define BO_PROFILE_NOTICES 16

typedef struct {
    double stats[BO_STAT_COUNT];
    double high_scores[BO_HIGH_COUNT];
    uint8_t unlocked[BO_CENTER_COUNT];
    uint8_t discovered[BO_CENTER_COUNT];
    uint8_t blind_discovered[BO_BLIND_COUNT];
    uint8_t all_unlocked;
    // deck_usage: the runs, the wins, and the losses of each deck at each
    // stake. Index 0 is the Red Deck.
    struct {
        int count;
        int wins[BO_STAKE_COUNT];
        int losses[BO_STAKE_COUNT];
    } decks[BO_DECK_COUNT];
    int hand_usage[BO_HAND_COUNT];  // The plays of each poker hand.
    uint8_t challenges_completed[24];  // challenge_progress.completed, by index.
    int challenges_unlocked;           // The challenges that the player can start.
    int blank_redeems;              // voucher_usage.v_blank.count.
    // The cards that this profile unlocked, for the alerts of the screen.
    BOCenterId notices[BO_PROFILE_NOTICES];
    int notice_count;
} BOProfile;

// A new profile: the unlocked and discovered flags of game.lua.
void bo_profile_init(BOProfile *profile);

// The profile that the run calls, or NULL.
void bo_profile_set_active(BOProfile *profile);
BOProfile *bo_profile_active(void);

// unlock_all: all cards unlocked and discovered.
void bo_profile_unlock_all(BOProfile *profile);

// With no active profile, both give 1.
int bo_profile_unlocked(BOCenterId center);
int bo_profile_discovered(BOCenterId center);

// get_deck_win_stake: the highest stake that the deck won, or 0. With
// BO_CENTER_NONE, the highest stake of all decks.
int bo_profile_deck_win_stake(const BOProfile *profile, BOCenterId back);

// The highest stake that a new run with this deck can have: one over the
// highest win of the deck, as the run setup of Balatro.
int bo_profile_max_stake(const BOProfile *profile, BOCenterId back);

// The types of check_for_unlock.
typedef enum {
    BO_UNLOCK_CAREER_STAT,
    BO_UNLOCK_MONEY,
    BO_UNLOCK_ANTE_UP,
    BO_UNLOCK_ROUND_WIN,
    BO_UNLOCK_WIN,
    BO_UNLOCK_WIN_DECK,
    BO_UNLOCK_WIN_STAKE,
    BO_UNLOCK_WIN_NO_HAND,
    BO_UNLOCK_WIN_CUSTOM,
    BO_UNLOCK_DISCARD_CUSTOM,
    BO_UNLOCK_HAND_CONTENTS,
    BO_UNLOCK_PLAY_ALL_HEARTS,
    BO_UNLOCK_RUN_CARD_REPLAYS,
    BO_UNLOCK_CHIP_SCORE,
    BO_UNLOCK_MODIFY_DECK,
    BO_UNLOCK_MODIFY_JOKERS,
    BO_UNLOCK_HAVE_EDITION,
    BO_UNLOCK_MIN_HAND_SIZE,
    BO_UNLOCK_INTEREST_STREAK,
    BO_UNLOCK_RUN_REDEEM,
    BO_UNLOCK_BLANK_REDEEMS,
    BO_UNLOCK_DOUBLE_GOLD,
    BO_UNLOCK_CONTINUE_GAME,
    BO_UNLOCK_DISCOVER_AMOUNT,
    BO_UNLOCK_BLIND_DISCOVERIES,
} BOUnlockType;

typedef struct {
    BOUnlockType type;
    BOStat stat;           // BO_UNLOCK_CAREER_STAT.
    int ante;              // BO_UNLOCK_ANTE_UP.
    double chips;          // BO_UNLOCK_CHIP_SCORE.
    const BOCard *cards;   // BO_UNLOCK_DISCARD_CUSTOM and BO_UNLOCK_HAND_CONTENTS.
    int card_count;
} BOUnlockArgs;

// check_for_unlock: unlocks each locked card whose condition is true now.
void bo_profile_check(BORun *run, const BOUnlockArgs *args);

// The checks of the cards of the run, after a change to the cards: the
// deck (modify_deck), a Gold Card with a Gold Seal (double_gold), the
// jokers (modify_jokers and have_edition), and the hand size.
void bo_profile_check_cards(BORun *run);

// The checks and the stats of a play: the cards, the hand, and the score.
void bo_profile_play(BORun *run, const BOCard *cards, int count, BOHandType hand, double chips);

// The checks and the stats of a discard.
void bo_profile_discard(BORun *run, const BOCard *cards, int count);

// The end of a round that the player won: the blind, the streaks, and
// round_win. interest_steps is the interest of evaluate_round, or -1.
void bo_profile_round_won(BORun *run, int interest_steps);

// ease_dollars: the dollars earned, the high score, and the check of the
// money.
void bo_profile_dollars(BORun *run, int amount);

// inc_career_stat, with the check of the career stat.
void bo_profile_stat(BORun *run, BOStat stat, double amount);

// check_and_set_high_score.
void bo_profile_high_score(BOHighScore score, double amount);

// discover_card for a card, a tag, or a blind.
void bo_profile_discover(BORun *run, BOCenterId center);
void bo_profile_discover_blind(BORun *run, BOBlindId blind);

// The counts of discovered cards, as set_discover_tallies: all, Tarot,
// and Planet.
void bo_profile_discover_tallies(const BOProfile *profile, int *total, int *of, int *tarots, int *planets);

// The start of a run: the deck counts one more run.
void bo_profile_start_run(BORun *run);

// The challenges that the player can start: challenges_unlocked, or all
// with Unlock All.
int bo_profile_challenges_open(const BOProfile *profile);

// The end of a run: win_game or the loss of game_over.
void bo_profile_win(BORun *run);
void bo_profile_lose(BORun *run);

// Takes the oldest card that this profile unlocked and did not show yet.
// Returns BO_CENTER_NONE when there is none.
BOCenterId bo_profile_take_notice(BOProfile *profile);

// The file of a profile, with a header as bo_save. Returns 1 on success.
int bo_profile_write(const BOProfile *profile, const char *path);
int bo_profile_read(BOProfile *profile, const char *path);

#endif
