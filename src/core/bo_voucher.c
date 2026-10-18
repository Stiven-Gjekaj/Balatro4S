#include "bo_voucher.h"

#include <string.h>

#include "bo_joker.h"
#include "bo_pool.h"
#include "bo_rules.h"

#pragma STDC FP_CONTRACT OFF

int bo_voucher_can_redeem(const BORun *run) {
    if (run->state != BO_STATE_SHOP || !run->has_shop_voucher) return 0;
    int cost = run->shop_voucher.cost;
    return cost <= bo_run_spendable(run) || cost <= 0;
}

// change_shop_size: one more card for sale, now and in each shop.
static void change_shop_size(BORun *run, int delta) {
    run->shop_joker_max += delta;
    if (run->state != BO_STATE_SHOP) return;
    while (run->shop_count > run->shop_joker_max && run->shop_count > 0) {
        run->shop_count--;
        bo_run_forget_card(run, run->shop[run->shop_count].item.center);
    }
    while (run->shop_count < run->shop_joker_max && run->shop_count < BO_MAX_SHOP) {
        run->shop[run->shop_count++] = bo_pool_create_shop_card(run);
    }
}

void bo_voucher_apply(BORun *run, BOCenterId center) {
    const BOCenterInfo *c = bo_center(center);
    double extra = c->extra;
    switch (center) {
    case BO_V_OVERSTOCK_NORM:
    case BO_V_OVERSTOCK_PLUS: change_shop_size(run, 1); break;
    case BO_V_TAROT_MERCHANT:
    case BO_V_TAROT_TYCOON: run->tarot_rate = 4 * extra; break;
    case BO_V_PLANET_MERCHANT:
    case BO_V_PLANET_TYCOON: run->planet_rate = 4 * extra; break;
    case BO_V_HONE:
    case BO_V_GLOW_UP: run->edition_rate = extra; break;
    case BO_V_MAGIC_TRICK:
    case BO_V_ILLUSION: run->playing_card_rate = extra; break;
    case BO_V_CRYSTAL_BALL: run->consumable_slots++; break;
    case BO_V_CLEARANCE_SALE:
    case BO_V_LIQUIDATION:
        run->discount_percent = (int)extra;
        bo_rules_set_all_costs(run);
        break;
    case BO_V_REROLL_SURPLUS:
    case BO_V_REROLL_GLUT:
        run->reroll_cost_base -= (int)extra;
        run->reroll_cost = run->reroll_cost - (int)extra > 0 ? run->reroll_cost - (int)extra : 0;
        break;
    case BO_V_SEED_MONEY:
    case BO_V_MONEY_TREE: run->interest_cap = (int)extra; break;
    case BO_V_GRABBER:
    case BO_V_NACHO_TONG:
        run->hands_per_round += (int)extra;
        run->hands_left += (int)extra;
        break;
    case BO_V_PAINT_BRUSH:
    case BO_V_PALETTE: bo_rules_change_hand_size(run, 1); break;
    case BO_V_WASTEFUL:
    case BO_V_RECYCLOMANCY:
        run->discards_per_round += (int)extra;
        bo_rules_ease_discard(run, (int)extra);
        break;
    case BO_V_ANTIMATTER: run->joker_slots++; break;
    case BO_V_HIEROGLYPH:
    case BO_V_PETROGLYPH:
        run->ante -= (int)extra;
        if (center == BO_V_HIEROGLYPH) {
            run->hands_per_round -= (int)extra;
            run->hands_left -= (int)extra;
        } else {
            run->discards_per_round -= (int)extra;
            bo_rules_ease_discard(run, -(int)extra);
        }
        break;
    default: break;
    }
}

// Card:redeem: the voucher is used, paid, and applied.
static void redeem_item(BORun *run, BOItem voucher) {
    // Each redeem clears current_round.voucher, so the shop voucher does
    // not come back in this ante.
    run->voucher = BO_CENTER_NONE;
    run->used_vouchers[voucher.center] = 1;
    bo_run_ease_dollars(run, -voucher.cost);
    bo_voucher_apply(run, voucher.center);
    for (int i = 0; i < run->joker_count; i++) {
        BOContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.kind = BO_CTX_BUYING_CARD;
        ctx.card = &voucher;
        bo_joker_calculate(run, &run->jokers[i], &ctx);
    }
    bo_run_forget_card(run, voucher.center);
}

int bo_voucher_redeem(BORun *run) {
    if (!bo_voucher_can_redeem(run)) return 0;
    BOItem voucher = run->shop_voucher;
    run->has_shop_voucher = 0;
    redeem_item(run, voucher);
    return 1;
}

int bo_voucher_can_redeem_tag(const BORun *run, int index) {
    if (run->state != BO_STATE_SHOP || index < 0 || index >= run->tag_voucher_count) return 0;
    int cost = run->tag_vouchers[index].cost;
    return cost <= bo_run_spendable(run) || cost <= 0;
}

int bo_voucher_redeem_tag(BORun *run, int index) {
    if (!bo_voucher_can_redeem_tag(run, index)) return 0;
    BOItem voucher = run->tag_vouchers[index];
    memmove(&run->tag_vouchers[index], &run->tag_vouchers[index + 1],
            sizeof(BOItem) * (run->tag_voucher_count - index - 1));
    run->tag_voucher_count--;
    redeem_item(run, voucher);
    return 1;
}

int bo_voucher_can_reroll_boss(const BORun *run) {
    if (run->state != BO_STATE_BLIND_SELECT) return 0;
    if (bo_run_spendable(run) - 10 < 0) return 0;
    return run->used_vouchers[BO_V_RETCON] || (run->used_vouchers[BO_V_DIRECTORS_CUT] && !run->boss_rerolled);
}

int bo_voucher_reroll_boss(BORun *run) {
    if (!bo_voucher_can_reroll_boss(run)) return 0;
    run->boss_rerolled = 1;
    bo_run_ease_dollars(run, -10);
    run->boss = bo_blind_new_boss(&run->random, run->ante, run->win_ante, run->bosses_used, NULL);
    return 1;
}
