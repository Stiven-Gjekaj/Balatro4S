-- describe.lua: prints the text of each card with the code of Balatro:
-- Card:generate_UIBox_ability_table, and localize with en-us.lua.
-- tools/describe.c prints the same with bo_describe of Balatro4S.
-- Usage: tools/oracle.sh describe SEED
-- Each line is: key | name | line 1 | line 2 ...

local dir, seed = arg[1], arg[3] or 'ABCDEFGH'
-- oracle.lua loads misc_functions.lua with the real localize. headless.lua
-- puts a stub in its place, so keep the real one.
local real_localize = localize
local B = dofile((arg[0]:gsub('oracle%.lua$', 'headless.lua')))(dir)
G.SETTINGS.profile = 1
G.PROFILES = {{career_stats = {}, high_scores = {}, challenge_progress = {completed = {}, unlocked = {}}}}
G:init_item_prototypes()
for k, v in pairs(G.P_CENTERS) do v.unlocked = true; v.discovered = true end
localize = real_localize
G.localization = dofile(dir .. '/localization/en-us.lua')
init_localization()
-- The font of the language: localize reads its scale.
G.LANG = {font = {DESCSCALE = 1, FONTSCALE = 1, squish = 1, TEXT_HEIGHT_SCALE = 1, TEXT_OFFSET = {x = 0, y = 0}}}

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

-- The text of the nodes of one line, in order.
local function line_text(nodes, out)
    for _, n in ipairs(nodes) do
        if n.config and n.config.text then out[#out + 1] = n.config.text end
        if n.config and n.config.object and n.config.object.string then
            local s = n.config.object.string
            out[#out + 1] = type(s) == 'table' and (s[1].string or tostring(s[1])) or tostring(s)
        end
        if n.nodes then line_text(n.nodes, out) end
    end
    return out
end

local keys = {}
for k, v in pairs(G.P_CENTERS) do
    if v.set == 'Joker' or v.set == 'Tarot' or v.set == 'Planet' or v.set == 'Spectral' or v.set == 'Voucher' or
        v.set == 'Booster' then
        keys[#keys + 1] = k
    end
end
table.sort(keys)
for _, k in ipairs(keys) do
    local ok, err = pcall(function()
        local card = Card(0, 0, 1, 1, nil, G.P_CENTERS[k])
        local ui = card:generate_UIBox_ability_table()
        local parts = {k}
        for _, line in ipairs(ui.main or {}) do
            parts[#parts + 1] = table.concat(line_text(line, {}))
        end
        print(table.concat(parts, ' | '))
    end)
    if not ok then print(k .. ' | ERROR ' .. tostring(err)) end
end
