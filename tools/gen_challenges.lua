-- gen_challenges.lua: writes src/core/bo_challenge_data.h and
-- src/core/bo_challenge_data.c from challenges.lua in your own
-- Balatro.love. The C files hold the rules of the 20 challenges: their
-- custom rules, their modifiers, their cards, their decks, and their
-- banned keys. They hold no text of the descriptions.
-- Usage: tools/oracle.sh genchallenges OUTPUT_DIR (oracle.lua runs this file)

local dir, out = arg[1], arg[3]
G = {}
dofile(dir .. '/challenges.lua')

local function c_string(s)
    if s == nil then return 'NULL' end
    return '"' .. tostring(s):gsub('\\', '\\\\'):gsub('"', '\\"') .. '"'
end

local function c_number(n)
    if n == nil then return '0' end
    if type(n) == 'boolean' then return n and '1' or '0' end
    return string.format('%.17g', n)
end

-- The keys of a set table, as {S = true, H = true}, in one sorted string.
local function key_letters(t)
    if not t then return nil end
    local keys = {}
    for k in pairs(t) do keys[#keys + 1] = k end
    table.sort(keys)
    return table.concat(keys)
end

local h = io.open(out .. '/bo_challenge_data.h', 'w')
h:write([[
// tools/gen_challenges.lua makes this file from challenges.lua. Do not
// change it by hand.

#ifndef BO_CHALLENGE_DATA_H
#define BO_CHALLENGE_DATA_H

#include <stdint.h>

// A rule of rules.custom or rules.modifiers: an id, and a number or a text.
typedef struct {
    const char *id;
    double number;
    const char *text;
} BOChallengeRule;

// A joker or a consumable of the start, with its edition and stickers.
typedef struct {
    const char *key;
    const char *edition;
    uint8_t eternal;
    uint8_t pinned;
} BOChallengeCard;

// A card of deck.cards: the letters of its suit and rank, as 'D' and 'T',
// and its enhancement, edition, and seal, or NULL.
typedef struct {
    char suit;
    char rank;
    const char *enhancement;
    const char *edition;
    const char *seal;
} BOChallengeDeckCard;

typedef struct {
    const char *id;
    const char *name;
    const BOChallengeRule *custom;
    uint8_t custom_count;
    const BOChallengeRule *modifiers;
    uint8_t modifier_count;
    const BOChallengeCard *jokers;
    uint8_t joker_count;
    const BOChallengeCard *consumables;
    uint8_t consumable_count;
    const char *const *vouchers;
    uint8_t voucher_count;
    // deck: its type, its cards, and the filters of the standard cards.
    // The filters are the letters of the ranks and suits, as "AK".
    const char *deck_type;
    const BOChallengeDeckCard *deck_cards;
    uint8_t deck_card_count;
    const char *yes_ranks, *no_ranks, *yes_suits, *no_suits;
    const char *deck_enhancement;
    const char *deck_edition;
    uint8_t deck_gold_seal;
    // restrictions: each banned key, with the ids of a banned group.
    const char *const *banned;
    uint8_t banned_count;
} BOChallengeInfo;

]])
h:write('#define BO_CHALLENGE_COUNT ' .. #G.CHALLENGES .. '\n\n')
h:write('const BOChallengeInfo *bo_challenge(int index);\n\n#endif\n')
h:close()

local c = io.open(out .. '/bo_challenge_data.c', 'w')
c:write('// tools/gen_challenges.lua makes this file from challenges.lua. Do not\n// change it by hand.\n\n')
c:write('#include "bo_challenge_data.h"\n\n#include <stddef.h>\n\n')
for i, ch in ipairs(G.CHALLENGES) do
    local rules = ch.rules or {}
    local function rule_list(name, list)
        if not list or #list == 0 then return end
        c:write('static const BOChallengeRule k' .. name .. i .. '[] = {')
        for _, r in ipairs(list) do
            local number = type(r.value) == 'number' and c_number(r.value) or '0'
            local text = type(r.value) == 'string' and c_string(r.value) or 'NULL'
            c:write('{' .. c_string(r.id) .. ', ' .. number .. ', ' .. text .. '}, ')
        end
        c:write('};\n')
    end
    rule_list('Custom', rules.custom)
    rule_list('Modifiers', rules.modifiers)
    local function card_list(name, list)
        if not list or #list == 0 then return end
        c:write('static const BOChallengeCard k' .. name .. i .. '[] = {')
        for _, r in ipairs(list) do
            c:write('{' .. c_string(r.id) .. ', ' .. c_string(r.edition) .. ', ' .. c_number(r.eternal) .. ', ' ..
                    c_number(r.pinned) .. '}, ')
        end
        c:write('};\n')
    end
    card_list('Jokers', ch.jokers)
    card_list('Consumables', ch.consumeables)
    if ch.vouchers and #ch.vouchers > 0 then
        c:write('static const char *const kVouchers' .. i .. '[] = {')
        for _, r in ipairs(ch.vouchers) do c:write(c_string(r.id) .. ', ') end
        c:write('};\n')
    end
    local deck = ch.deck or {}
    if deck.cards and #deck.cards > 0 then
        c:write('static const BOChallengeDeckCard kDeck' .. i .. '[] = {')
        for _, r in ipairs(deck.cards) do
            c:write("{'" .. r.s .. "', '" .. r.r .. "', " .. c_string(r.e) .. ', ' .. c_string(r.d) .. ', ' ..
                    c_string(r.g) .. '}, ')
        end
        c:write('};\n')
    end
    local banned = {}
    local restrictions = ch.restrictions or {}
    for _, group in ipairs({'banned_cards', 'banned_tags', 'banned_other'}) do
        for _, r in ipairs(restrictions[group] or {}) do
            banned[#banned + 1] = r.id
            for _, id in ipairs(r.ids or {}) do banned[#banned + 1] = id end
        end
    end
    if #banned > 0 then
        c:write('static const char *const kBanned' .. i .. '[] = {')
        for _, id in ipairs(banned) do c:write(c_string(id) .. ', ') end
        c:write('};\n')
    end
end
c:write('\nstatic const BOChallengeInfo kChallenges[BO_CHALLENGE_COUNT] = {\n')
for i, ch in ipairs(G.CHALLENGES) do
    local rules = ch.rules or {}
    local deck = ch.deck or {}
    local function ref(name, list)
        return (list and #list > 0) and ('k' .. name .. i .. ', ' .. #list) or 'NULL, 0'
    end
    local banned = 0
    for _, group in ipairs({'banned_cards', 'banned_tags', 'banned_other'}) do
        for _, r in ipairs((ch.restrictions or {})[group] or {}) do banned = banned + 1 + #(r.ids or {}) end
    end
    c:write(string.format('    {%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s},\n',
        c_string(ch.id), c_string(ch.name), ref('Custom', rules.custom), ref('Modifiers', rules.modifiers),
        ref('Jokers', ch.jokers), ref('Consumables', ch.consumeables), ref('Vouchers', ch.vouchers),
        c_string(deck.type), ref('Deck', deck.cards), c_string(key_letters(deck.yes_ranks)),
        c_string(key_letters(deck.no_ranks)), c_string(key_letters(deck.yes_suits)),
        c_string(key_letters(deck.no_suits)), c_string(deck.enhancement), c_string(deck.edition),
        c_number(deck.gold_seal), banned > 0 and ('kBanned' .. i .. ', ' .. banned) or 'NULL, 0'))
end
c:write([[
};

const BOChallengeInfo *bo_challenge(int index) {
    if (index < 0 || index >= BO_CHALLENGE_COUNT) return NULL;
    return &kChallenges[index];
}
]])
c:close()
print(#G.CHALLENGES .. ' challenges')
