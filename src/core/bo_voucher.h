// bo_voucher: the vouchers, as Card:redeem and Card:apply_to_run of
// Balatro 1.0.1o, and the boss reroll of Director's Cut and Retcon.

#ifndef BO_VOUCHER_H
#define BO_VOUCHER_H

#include "bo_run.h"

// The voucher of the shop can be bought now.
int bo_voucher_can_redeem(const BORun *run);

// Buys the voucher of the shop and applies it to the run.
int bo_voucher_redeem(BORun *run);

// Card:apply_to_run: the effect of a voucher on the run.
void bo_voucher_apply(BORun *run, BOCenterId center);

// G.FUNCS.reroll_boss_button: a new boss for $10.
int bo_voucher_can_reroll_boss(const BORun *run);
int bo_voucher_reroll_boss(BORun *run);

#endif
