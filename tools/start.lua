-- start.lua: starts a run with Game:start_run of Balatro, and prints the
-- state of the start. tools/scenario.c does the same with bo_run_start_deck.
-- tools/scenario.lua calls it for a scenario file with a "start" line:
--   seed ABCDEFGH
--   back b_erratic
--   stake 3
--   start
-- start_run builds the screen too. The stubs below replace only the parts
-- of the screen that start_run calls. The rules stay the code of Balatro.

return function(B, lines, dir)
    local seed, back, stake, challenge = 'ABCDEFGH', 'b_red', 1, nil
    for _, w in ipairs(lines) do
        if w[1] == 'seed' then seed = w[2]
        elseif w[1] == 'back' then back = w[2]
        elseif w[1] == 'stake' then stake = tonumber(w[2])
        elseif w[1] == 'challenge' then challenge = tonumber(w[2]) end
    end
    if challenge then
        dofile(dir .. '/challenges.lua')
        challenge = G.CHALLENGES[challenge + 1]
    end

    -- An object that answers each index and each call with another one.
    local stub
    stub = function()
        return setmetatable({}, {__index = function() return stub() end, __call = function() return stub() end})
    end
    G.CONTROLLER = G.CONTROLLER or {}
    G.CONTROLLER.locks = G.CONTROLLER.locks or {}
    function G:prep_stage(new_stage, new_state) self.STAGE = new_stage; self.STATE = new_state end
    ease_background_colour_blind = function() end
    UIBox = function() return stub() end
    create_UIBox_HUD = function() return {} end
    create_UIBox_HUD_blind = function() return {} end
    G.FILE_HANDLER = {}
    G.save_settings = function() end
    G.real_dt = 1 / 60
    G.TIMERS.BACKGROUND = G.TIMERS.BACKGROUND or 0

    G.GAME = G:init_game_object()
    G.GAME.viewed_back = Back(G.P_CENTERS[back])
    G:start_run({seed = seed, stake = stake, challenge = challenge})
    B.manager:run_all()

    local letters = {['10'] = 'T', Jack = 'J', Queen = 'Q', King = 'K', Ace = 'A'}
    local function card_text(c)
        local text = c.base.suit:sub(1, 1) .. '_' .. (letters[c.base.value] or c.base.value)
        if c.config.center.key ~= 'c_base' then text = text .. ':' .. c.config.center.key end
        if c.edition then text = text .. ':' .. c.edition.type end
        if c.seal then text = text .. ':' .. c.seal end
        return text
    end
    local function rank_text(id)
        if not id then return '-' end
        return ({'2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A'})[id - 1] or '-'
    end

    local GAME = G.GAME
    print(string.format('start hands %d discards %d dollars %d hand_size %d joker_slots %d consumable_slots %d',
        GAME.round_resets.hands, GAME.round_resets.discards, GAME.dollars, G.hand.config.card_limit,
        G.jokers.config.card_limit, G.consumeables.config.card_limit))
    print('boss ' .. GAME.round_resets.blind_choices.Boss .. ' voucher ' .. GAME.current_round.voucher ..
          ' tags ' .. GAME.round_resets.blind_tags.Small .. ' ' .. GAME.round_resets.blind_tags.Big)
    local vouchers = {}
    for k, v in pairs(GAME.used_vouchers) do if v then vouchers[#vouchers + 1] = k end end
    table.sort(vouchers)
    print(table.concat({'vouchers', unpack(vouchers)}, ' '))
    local consumables = {}
    for i, c in ipairs(G.consumeables.cards) do consumables[i] = c.config.center.key end
    print(table.concat({'consumables', unpack(consumables)}, ' '))
    print(string.format('rates tarot %g planet %g spectral %g shop_slots %d', GAME.tarot_rate, GAME.planet_rate,
        GAME.spectral_rate, GAME.shop.joker_max))
    print(string.format('blind_scaling %g interest %s per_hand %d per_discard %d',
        GAME.starting_params.ante_scaling, GAME.modifiers.no_interest and 'no' or 'yes',
        GAME.modifiers.money_per_hand or 1, GAME.modifiers.money_per_discard or 0))
    -- get_blind_amount with the scaling of the stake, as Blind:set_blind.
    local amount = get_blind_amount(GAME.round_resets.ante) * GAME.starting_params.ante_scaling
    print(string.format('blinds %.17g %.17g %.17g', amount * G.P_BLINDS.bl_small.mult,
        amount * G.P_BLINDS.bl_big.mult, amount * G.P_BLINDS[GAME.round_resets.blind_choices.Boss].mult))
    -- The top of the deck is the end of the list.
    local deck = {}
    for i = #G.deck.cards, 1, -1 do deck[#deck + 1] = card_text(G.deck.cards[i]) end
    print('deck ' .. #G.deck.cards .. ' ' .. table.concat(deck, ' '))
    local jokers = {}
    for i, c in ipairs(G.jokers.cards) do
        jokers[i] = c.config.center.key .. (c.edition and (':' .. c.edition.type) or '') ..
                    (c.ability.eternal and ':E' or '') .. (c.pinned and ':P' or '')
    end
    print(table.concat({'jokers', unpack(jokers)}, ' '))
    local m = GAME.modifiers
    local reward = m.no_blind_reward or {}
    print(string.format('rules reward %d%d%d hand_money %s cap %s flip %d minus %d eternal_ante %d ' ..
        'slots_ante %d discard_cost %d joker_rate %g inflation %s debuff %s all_eternal %s',
        reward.Small and 0 or 1, reward.Big and 0 or 1, reward.Boss and 0 or 1,
        m.no_extra_hand_money and 'no' or 'yes', m.chips_dollar_cap and 'yes' or 'no', m.flipped_cards or 0,
        m.minus_hand_size_per_X_dollar or 0, m.set_eternal_ante or 0, m.set_joker_slots_ante or 0,
        m.discard_cost or 0, GAME.joker_rate, m.inflation and 'yes' or 'no',
        m.debuff_played_cards and 'yes' or 'no', m.all_eternal and 'yes' or 'no'))
    local banned = {}
    for k in pairs(GAME.banned_keys) do banned[#banned + 1] = k end
    table.sort(banned)
    print(table.concat({'banned', unpack(banned)}, ' '))
    local r = GAME.current_round
    print('round idol ' .. rank_text(r.idol_card.id) .. ' ' .. (r.idol_card.suit or '-') ..
          ' mail ' .. rank_text(r.mail_card.id) .. ' ancient ' .. (r.ancient_card.suit or '-') ..
          ' castle ' .. (r.castle_card.suit or '-'))
end
