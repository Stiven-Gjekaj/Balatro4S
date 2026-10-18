-- scenario.lua: plays one hand from a scenario file with the code of
-- Balatro, and prints the result. tools/scenario.c does the same with the
-- code of Balatro4S. tools/fuzz.sh compares the two.
--
-- A scenario file has one setting on each line:
--   seed ABCDEFGH
--   ante 1
--   dollars 10
--   hands_left 4          (before the play; the play uses one)
--   discards_left 3
--   hands_played 0        (in this round)
--   discards_used 0
--   level Pair 2
--   played Pair 3         (G.GAME.hands[...].played)
--   joker j_key [edition=foil] [mult=1] [x_mult=1] [extra=1] [extra_chips=5]
--   consumable c_key [edition=negative]
--   deck N                (N cards in the deck)
--   idol 13 Hearts        (the card of The Idol; no line: no card)
--   mail 7                (the rank of Mail-In Rebate)
--   ancient Clubs         (the suit of Ancient Joker)
--   castle Hearts         (the suit of Castle)
--   play S_A S_K:m_glass:holo:Red
--   hand H_2:m_steel
--   use c_key 1 3         (use a consumable on cards 1 and 3 of the hand,
--                          and print the state after it, in place of a play)
--   last c_key            (G.GAME.last_tarot_planet)
--   eternal 2             (joker 2 is eternal)
-- A card is a key of P_CARDS, then an enhancement, an edition, and a seal.

local dir, file = arg[1], arg[3]
local B = dofile((arg[0]:gsub('oracle%.lua$', 'headless.lua')))(dir)
G.SETTINGS.profile = 1
G.PROFILES = {{career_stats = {}, high_scores = {}, challenge_progress = {completed = {}, unlocked = {}}}}
G:init_item_prototypes()
for k, v in pairs(G.P_CENTERS) do v.unlocked = true; v.discovered = true end

local lines = {}
for line in io.lines(file) do
    local words = {}
    for w in line:gmatch('%S+') do words[#words + 1] = w end
    if words[1] then lines[#lines + 1] = words end
end

local seed = 'ABCDEFGH'
for _, w in ipairs(lines) do if w[1] == 'seed' then seed = w[2] end end
G.GAME = G:init_game_object()
G.GAME.pseudorandom.seed = seed
G.GAME.pseudorandom.hashed_seed = pseudohash(seed)
G.GAME.selected_back = Back(G.P_CENTERS.b_red)
G.GAME.dollars = 4
local function area(name, limit, kind)
    G[name] = CardArea(0, 0, 1, 1, {card_limit = limit, type = kind})
end
area('jokers', 5, 'joker')
area('consumeables', 2, 'joker')
area('hand', 8, 'hand')
area('play', 5, 'play')
area('deck', 500, 'deck')
area('discard', 500, 'discard')
G.playing_cards = {}
G.GAME.blind = Blind(0, 0, 1, 1)
G.GAME.blind:set_blind(G.P_BLINDS.bl_small)
B.manager:run_all()

local editions = {foil = {foil = true}, holo = {holo = true}, polychrome = {polychrome = true},
                  negative = {negative = true}}

local function playing_card(token)
    local parts = {}
    for p in token:gmatch('[^:]+') do parts[#parts + 1] = p end
    local center = G.P_CENTERS.c_base
    local edition, seal
    for i = 2, #parts do
        local p = parts[i]
        if G.P_CENTERS[p] then center = G.P_CENTERS[p]
        elseif editions[p] then edition = editions[p]
        else seal = p end
    end
    G.playing_card = (G.playing_card or 0) + 1
    local card = Card(0, 0, 1, 1, G.P_CARDS[parts[1]], center, {playing_card = G.playing_card})
    if edition then card:set_edition(edition, true, true) end
    if seal then card:set_seal(seal, true, true) end
    table.insert(G.playing_cards, card)
    return card
end

local function set_ability(card, words)
    for i = 3, #words do
        local k, v = words[i]:match('([^=]+)=(.+)')
        if k == 'edition' then card:set_edition(editions[v], true, true)
        elseif k:sub(1, 6) == 'extra_' then card.ability.extra[k:sub(7)] = tonumber(v) or v
        elseif k == 'sell' then card.sell_cost = tonumber(v)
        else card.ability[k] = tonumber(v) or v end
    end
end

local plays, holds, use = {}, {}, nil
for _, w in ipairs(lines) do
    local key = w[1]
    if key == 'ante' then G.GAME.round_resets.ante = tonumber(w[2])
    elseif key == 'dollars' then G.GAME.dollars = tonumber(w[2])
    elseif key == 'hands_left' then G.GAME.current_round.hands_left = tonumber(w[2])
    elseif key == 'discards_left' then G.GAME.current_round.discards_left = tonumber(w[2])
    elseif key == 'hands_played' then G.GAME.current_round.hands_played = tonumber(w[2])
    elseif key == 'discards_used' then G.GAME.current_round.discards_used = tonumber(w[2])
    elseif key == 'level' then
        local name = w[2]:gsub('_', ' ')
        level_up_hand(nil, name, true, tonumber(w[3]) - 1)
    elseif key == 'played' then
        local name = w[2]:gsub('_', ' ')
        G.GAME.hands[name].played = tonumber(w[3])
    elseif key == 'joker' or key == 'consumable' then
        local card = Card(0, 0, 1, 1, nil, G.P_CENTERS[w[2]])
        set_ability(card, w)
        card:add_to_deck()
        if key == 'joker' then G.jokers:emplace(card) else G.consumeables:emplace(card) end
    elseif key == 'idol' then
        G.GAME.current_round.idol_card.id = tonumber(w[2])
        G.GAME.current_round.idol_card.suit = w[3]
    elseif key == 'mail' then G.GAME.current_round.mail_card.id = tonumber(w[2])
    elseif key == 'ancient' then G.GAME.current_round.ancient_card.suit = w[2]
    elseif key == 'castle' then G.GAME.current_round.castle_card.suit = w[2]
    elseif key == 'deck' then
        for i = 1, tonumber(w[2]) do G.deck:emplace(playing_card('C_2')) end
    elseif key == 'play' then
        for i = 2, #w do plays[#plays + 1] = playing_card(w[i]) end
    elseif key == 'hand' then
        for i = 2, #w do holds[#holds + 1] = playing_card(w[i]) end
    elseif key == 'use' then
        use = {key = w[2], picks = {}}
        for i = 3, #w do use.picks[#use.picks + 1] = tonumber(w[i]) end
    elseif key == 'last' then G.GAME.last_tarot_planet = w[2]
    elseif key == 'eternal' then G.jokers.cards[tonumber(w[2])]:set_eternal(true)
    end
end
B.manager:run_all()
-- Balatro sorts the scoring cards by their place on the screen, so each
-- card gets a place from left to right.
for i, c in ipairs(plays) do G.play:emplace(c); c.T.x = i end
for i, c in ipairs(holds) do G.hand:emplace(c); c.T.x = i end
local function print_jokers()
    for i, j in ipairs(G.jokers.cards) do
        local parts = {string.format('joker %d %s mult=%.17g x_mult=%.17g', i, j.config.center.key, j.ability.mult,
                                     j.ability.x_mult)}
        if type(j.ability.extra) == 'number' then
            parts[#parts + 1] = string.format('extra=%.17g', j.ability.extra)
        elseif type(j.ability.extra) == 'table' then
            local keys = {}
            for k, v in pairs(j.ability.extra) do if type(v) == 'number' then keys[#keys + 1] = k end end
            table.sort(keys)
            for _, k in ipairs(keys) do parts[#parts + 1] = string.format('extra_%s=%.17g', k, j.ability.extra[k]) end
        end
        print(table.concat(parts, ' '))
    end
end

local function print_levels()
    local levels = {}
    for k, v in pairs(G.GAME.hands) do if v.level ~= 1 then levels[#levels + 1] = k:gsub(' ', '_') .. '=' .. v.level end end
    table.sort(levels)
    print(#levels > 0 and ('levels ' .. table.concat(levels, ' ')) or 'levels')
end

local function card_text(c)
    local letters = {['10'] = 'T', Jack = 'J', Queen = 'Q', King = 'K', Ace = 'A'}
    local text = c.base.suit:sub(1, 1) .. '_' .. (letters[c.base.value] or c.base.value)
    if c.config.center.key ~= 'c_base' then text = text .. ':' .. c.config.center.key end
    if c.edition then text = text .. ':' .. c.edition.type end
    if c.seal then text = text .. ':' .. c.seal end
    return text
end

if use then
    -- G.FUNCS.use_card for a card in the consumable slots, with no screen.
    set_consumeable_usage = B.real.set_consumeable_usage
    G.PROFILES[1].consumeable_usage = {}
    G.save_settings = function() end
    G.CONTROLLER = {locks = {}}
    G.STATE = G.STATES.SELECTING_HAND
    G.STAGE = G.STAGES.RUN
    Card.update_alert = function() end
    get_joker_win_sticker = function() return nil end
    local card = Card(0, 0, 1, 1, nil, G.P_CENTERS[use.key])
    card:add_to_deck()
    G.consumeables:emplace(card)
    for _, p in ipairs(use.picks) do G.hand.highlighted[#G.hand.highlighted + 1] = G.hand.cards[p] end
    for _, c in ipairs(G.jokers.cards) do c:update(0) end
    for _, c in ipairs(G.consumeables.cards) do c:update(0) end
    local can = card:can_use_consumeable(nil, true)
    if can and card.ability.name == 'Ankh' and #G.jokers.cards >= G.jokers.config.card_limit then can = false end
    print('can_use ' .. tostring(can and true or false))
    if can then
        local area = card.area
        area:remove_card(card)
        card:use_consumeable(area)
        for i = 1, #G.jokers.cards do
            G.jokers.cards[i]:calculate_joker({using_consumeable = true, consumeable = card})
        end
        B.manager:run_all()
        card:remove()
        B.manager:run_all()
    end
    local hand = {}
    for _, c in ipairs(G.hand.cards) do hand[#hand + 1] = ' ' .. card_text(c) end
    print('hand' .. table.concat(hand))
    print('hand_size ' .. G.hand.config.card_limit)
    print('dollars ' .. G.GAME.dollars)
    print('joker_slots ' .. G.jokers.config.card_limit)
    for i, j in ipairs(G.jokers.cards) do
        print(string.format('joker_card %d %s%s%s', i, j.config.center.key, j.edition and (':' .. j.edition.type) or '',
                            j.ability.eternal and ':eternal' or ''))
    end
    print_jokers()
    print('consumable_slots ' .. G.consumeables.config.card_limit)
    for i, c in ipairs(G.consumeables.cards) do
        print(string.format('consumable %d %s%s', i, c.config.center.key, c.edition and (':' .. c.edition.type) or ''))
    end
    print_levels()
    os.exit(0)
end

-- play_cards_from_highlighted uses one hand before the score.
G.GAME.current_round.hands_left = G.GAME.current_round.hands_left - 1

-- Card:update runs each frame in the game. It keeps the tallies of some
-- jokers, as Steel Joker and Stone Joker.
G.STAGE = G.STAGES.RUN
Card.update_alert = function() end
get_joker_win_sticker = function() return nil end
for _, c in ipairs(G.jokers.cards) do c:update(0) end
for _, c in ipairs(G.consumeables.cards) do c:update(0) end

local last = {chips = 0, mult = 0}
local mod_chips_real, mod_mult_real = mod_chips, mod_mult
function mod_chips(v) last.chips = v; return mod_chips_real(v) end
function mod_mult(v) last.mult = v; return mod_mult_real(v) end
G.GAME.chips = 0
-- The name of the hand before the play: a Glass Card can break in it.
local text = G.FUNCS.get_poker_hand_info(G.play.cards)
G.FUNCS.evaluate_play()
B.manager:run_all()

print('hand ' .. text:gsub(' ', '_'))
print(string.format('chips %.17g', last.chips))
print(string.format('mult %.17g', last.mult))
print(string.format('total %.17g', G.GAME.chips))
print('dollars ' .. G.GAME.dollars)
print_jokers()
print('consumables ' .. #G.consumeables.cards)
print_levels()
