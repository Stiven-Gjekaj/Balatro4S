-- gen_data.lua: writes src/core/bo_data.h and src/core/bo_data.c from the
-- tables P_CENTERS and P_TAGS of game.lua in your own Balatro.love.
-- The C files hold the numbers of the rules: the keys, the names, the
-- costs, the places in the atlases, and the config values. They hold no
-- text of the descriptions. The app reads that text from Balatro.love.
-- Usage: tools/oracle.sh gen OUTPUT_DIR (oracle.lua runs this file)

local dir, out = arg[1], arg[3]

local function game_table(name)
    local file = io.open(dir .. '/game.lua')
    local text = file:read('*a')
    file:close()
    local start = text:find('self.' .. name .. ' = {', 1, true)
    local depth, i = 0, text:find('{', start, true)
    local first = i
    repeat
        local c = text:sub(i, i)
        if c == '{' then depth = depth + 1 elseif c == '}' then depth = depth - 1 end
        i = i + 1
    until depth == 0
    HEX = function(hex) return hex end
    localize = function(key) return key end
    return assert(loadstring('return ' .. text:sub(first, i - 1)))()
end

local centers = game_table('P_CENTERS')
for k, v in pairs(game_table('P_TAGS')) do v.set = 'Tag'; centers[k] = v end

-- The sets in a fixed order, then the order field, then the key.
local set_order = {Default = 1, Enhanced = 2, Edition = 3, Joker = 4, Tarot = 5, Planet = 6, Spectral = 7,
                   Voucher = 8, Back = 9, Booster = 10, Tag = 11}
local list = {}
for k, v in pairs(centers) do
    if set_order[v.set] then
        v.key = k
        list[#list + 1] = v
    end
end
table.sort(list, function(a, b)
    if a.set ~= b.set then return set_order[a.set] < set_order[b.set] end
    if (a.order or 0) ~= (b.order or 0) then return (a.order or 0) < (b.order or 0) end
    return a.key < b.key
end)

-- The standard config fields of set_ability. The other fields go into a
-- list of (field, value) pairs.
local standard = {mult = 'mult', h_mult = 'h_mult', h_x_mult = 'h_x_mult', h_dollars = 'h_dollars',
                  p_dollars = 'p_dollars', t_mult = 't_mult', t_chips = 't_chips', Xmult = 'x_mult',
                  h_size = 'h_size', d_size = 'd_size', bonus = 'bonus'}
local fields, field_list = {}, {}
local function field(name)
    if not fields[name] then
        fields[name] = true
        field_list[#field_list + 1] = name
    end
end

local function c_string(s)
    if s == nil then return 'NULL' end
    return '"' .. tostring(s):gsub('\\', '\\\\'):gsub('"', '\\"') .. '"'
end

local function c_number(n)
    if n == nil then return '0' end
    if type(n) == 'boolean' then return n and '1' or '0' end
    return string.format('%.17g', n)
end

-- Each entry of the generic list: the field, a number, and a string.
local function pairs_of(center)
    local items = {}
    local function add(name, value)
        if type(value) == 'table' then
            -- One more level, as {extra = {odds = 6}} or {requires = {'v_x'}}.
            for k, v in pairs(value) do
                if type(v) ~= 'table' then
                    local sub = type(k) == 'number' and (name .. '_' .. k) or (name .. '_' .. k)
                    field(sub)
                    items[#items + 1] = {sub, v}
                end
            end
        else
            field(name)
            items[#items + 1] = {name, value}
        end
    end
    for k, v in pairs(center.config or {}) do
        if not standard[k] and k ~= 'type' then add(k, v) end
    end
    if center.requires then add('requires', center.requires) end
    table.sort(items, function(a, b) return a[1] < b[1] end)
    return items
end

local entries = {}
for i, c in ipairs(list) do entries[i] = pairs_of(c) end
table.sort(field_list)

-- Two fields can differ only in case, as "extra_Xmult" and "extra_xmult".
-- The field with no capital letter then gets the suffix "_LC".
local upper_seen = {}
for _, f in ipairs(field_list) do
    local u = f:upper()
    upper_seen[u] = (upper_seen[u] or 0) + 1
end
local function enum_name(prefix, key)
    local name = key:upper():gsub('[^%w]', '_')
    if prefix == 'BO_F_' and upper_seen[key:upper()] and upper_seen[key:upper()] > 1 and key == key:lower() then
        name = name .. '_LC'
    end
    return prefix .. name
end

local h = io.open(out .. '/bo_data.h', 'w')
h:write([[
// bo_data: the cards of Balatro 1.0.1o as data.
// tools/gen_data.lua makes this file from game.lua. Do not change it by
// hand.

#ifndef BO_DATA_H
#define BO_DATA_H

#include <stdint.h>

typedef enum {
    BO_SET_DEFAULT,
    BO_SET_ENHANCED,
    BO_SET_EDITION,
    BO_SET_JOKER,
    BO_SET_TAROT,
    BO_SET_PLANET,
    BO_SET_SPECTRAL,
    BO_SET_VOUCHER,
    BO_SET_BACK,
    BO_SET_BOOSTER,
    BO_SET_TAG,
} BOSet;

// Each center, in the order of its set, then its order field.
typedef enum {
]])
for _, c in ipairs(list) do h:write('    ' .. enum_name('BO_', c.key) .. ',\n') end
h:write('    BO_CENTER_COUNT,\n    BO_CENTER_NONE = -1,\n} BOCenterId;\n\n')
h:write('// The config fields that are not standard. "extra_odds" is config.extra.odds.\ntypedef enum {\n')
for _, f in ipairs(field_list) do h:write('    ' .. enum_name('BO_F_', f) .. ',\n') end
h:write('    BO_FIELD_COUNT,\n} BOField;\n\n')
h:write([[
typedef struct {
    uint8_t field;
    double number;
    const char *text;
} BOConfigValue;

typedef struct {
    const char *key;
    const char *name;
    uint8_t set;
    int16_t order;
    uint8_t rarity;
    int8_t cost;
    int8_t pos_x, pos_y;
    int8_t soul_x, soul_y;     // -1 when the card has no second layer.
    uint8_t unlocked;
    uint8_t blueprint_compat;
    uint8_t eternal_compat;
    uint8_t perishable_compat;
    int16_t min_ante;          // For a tag.
    uint8_t consumeable;
    uint8_t hidden;            // The Soul and Black Hole: not in the normal pools.
    uint8_t omit;              // Not in any pool.
    double weight;             // The weight of a booster pack in get_pack.
    const char *kind;          // The kind of a booster pack, as "Arcana".
    const char *effect;
    const char *enhancement_gate;
    const char *no_pool_flag;
    const char *yes_pool_flag;
    const char *hand_type;     // config.type, as "Pair".
    double mult, h_mult, h_x_mult, h_dollars, p_dollars, t_mult, t_chips, x_mult, h_size, d_size, bonus;
    double extra;              // config.extra when it is a number.
    uint8_t config_count;
    const BOConfigValue *config;
} BOCenterInfo;

const BOCenterInfo *bo_center(BOCenterId id);
BOCenterId bo_center_by_key(const char *key);

// A config value by field. has gets 0 when the center has no such field.
double bo_center_number(const BOCenterInfo *center, BOField field, int *has);
const char *bo_center_text(const BOCenterInfo *center, BOField field);

#endif
]])
h:close()

local c = io.open(out .. '/bo_data.c', 'w')
c:write('// tools/gen_data.lua makes this file from game.lua. Do not change it by hand.\n\n')
c:write('#include "bo_data.h"\n\n#include <stddef.h>\n#include <string.h>\n\n')
for i, center in ipairs(list) do
    if #entries[i] > 0 then
        c:write('static const BOConfigValue k' .. i .. '[] = {')
        for _, e in ipairs(entries[i]) do
            local value = e[2]
            local number = type(value) == 'string' and '0' or c_number(value)
            local text = type(value) == 'string' and c_string(value) or 'NULL'
            c:write('{' .. enum_name('BO_F_', e[1]) .. ', ' .. number .. ', ' .. text .. '}, ')
        end
        c:write('};\n')
    end
end
c:write('\nstatic const BOCenterInfo kCenters[BO_CENTER_COUNT] = {\n')
local set_enum = {Default = 'BO_SET_DEFAULT', Enhanced = 'BO_SET_ENHANCED', Edition = 'BO_SET_EDITION',
                  Joker = 'BO_SET_JOKER', Tarot = 'BO_SET_TAROT', Planet = 'BO_SET_PLANET',
                  Spectral = 'BO_SET_SPECTRAL', Voucher = 'BO_SET_VOUCHER', Back = 'BO_SET_BACK',
                  Booster = 'BO_SET_BOOSTER', Tag = 'BO_SET_TAG'}
for i, v in ipairs(list) do
    local cfg = v.config or {}
    local extra = type(cfg.extra) == 'number' and cfg.extra or nil
    local soul = v.soul_pos
    c:write(string.format('    {%s, %s, %s, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %s, %s, %s, %s, %s, %s, %s, ',
        c_string(v.key), c_string(v.name), set_enum[v.set], v.order or 0, v.rarity or 0, v.cost or 0,
        v.pos and v.pos.x or -1, v.pos and v.pos.y or -1, soul and soul.x or -1, soul and soul.y or -1,
        v.unlocked == false and 0 or 1, v.blueprint_compat and 1 or 0, v.eternal_compat and 1 or 0,
        v.perishable_compat and 1 or 0, v.min_ante or 0, v.consumeable and 1 or 0, v.hidden and 1 or 0,
        v.omit and 1 or 0, c_number(v.weight), c_string(v.kind), c_string(v.effect), c_string(v.enhancement_gate),
        c_string(v.no_pool_flag), c_string(v.yes_pool_flag), c_string(type(cfg.type) == 'string' and cfg.type or nil)))
    c:write(string.format('%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %d, %s},\n',
        c_number(cfg.mult), c_number(cfg.h_mult), c_number(cfg.h_x_mult), c_number(cfg.h_dollars),
        c_number(cfg.p_dollars), c_number(cfg.t_mult), c_number(cfg.t_chips), c_number(cfg.Xmult or 1),
        c_number(cfg.h_size), c_number(cfg.d_size), c_number(cfg.bonus), c_number(extra),
        #entries[i], #entries[i] > 0 and ('k' .. i) or 'NULL'))
end
c:write([[
};

const BOCenterInfo *bo_center(BOCenterId id) {
    if (id < 0 || id >= BO_CENTER_COUNT) return NULL;
    return &kCenters[id];
}

BOCenterId bo_center_by_key(const char *key) {
    for (int i = 0; i < BO_CENTER_COUNT; i++) {
        if (strcmp(kCenters[i].key, key) == 0) return (BOCenterId)i;
    }
    return BO_CENTER_NONE;
}

double bo_center_number(const BOCenterInfo *center, BOField field, int *has) {
    for (int i = 0; i < center->config_count; i++) {
        if (center->config[i].field == field) {
            if (has) *has = 1;
            return center->config[i].number;
        }
    }
    if (has) *has = 0;
    return 0;
}

const char *bo_center_text(const BOCenterInfo *center, BOField field) {
    for (int i = 0; i < center->config_count; i++) {
        if (center->config[i].field == field) return center->config[i].text;
    }
    return NULL;
}
]])
c:close()
print(#list .. ' centers, ' .. #field_list .. ' fields')
