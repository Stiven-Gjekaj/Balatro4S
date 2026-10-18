-- oracle.lua: runs functions of the original Balatro in LuaJIT, and prints
-- the results. The tests of Balatro4S hold these results as numbers.
-- It reads the Lua files of your own Balatro.love. It does not copy them.
-- Usage: tools/oracle.sh COMMAND [ARGS], or with any LuaJIT of Balatro:
--        oracle-host tools/oracle.lua DIR COMMAND [ARGS]
--   DIR is a directory with the unpacked Balatro.love.
--   rng SEED KEY N        print N values of pseudoseed(KEY) for SEED
--   hash TEXT             print pseudohash(TEXT)
--   random SEED N         print N values of math.random() after randomseed(SEED)
--   shuffle SEED N        print the shuffle of 1..N with pseudoshuffle
--   bosses SEED ANTES     print the boss of each ante from 1 to ANTES
--   string SEED           print random_string(8, SEED), a new seed of a run
--   gen OUTPUT_DIR        write bo_data.h and bo_data.c (tools/gen_data.lua)
--   genchallenges DIR     write bo_challenge_data.h and .c (tools/gen_challenges.lua)
--   shop SEED REROLLS     print the first shop of a run and its rerolls
--   score FILE            play the hand of a scenario file (tools/scenario.lua)
--   deck SEED KEY...      print the deck from the top after each shuffle KEY
--   describe SEED         print the text of each card (tools/describe.lua)

local dir, command = arg[1], arg[2]
G = {GAME = {pseudorandom = {}}, CONTROLLER = {}}
love = {graphics = {}, system = {}, filesystem = {}}
dofile(dir .. '/functions/misc_functions.lua')

-- Reads one table of game.lua, for example P_BLINDS, as Lua data.
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

local function start(seed)
    G.GAME.pseudorandom = {seed = seed, hashed_seed = pseudohash(seed)}
end

if command == 'hash' then
    print(string.format('%.17g', pseudohash(arg[3])))
elseif command == 'rng' then
    start(arg[3])
    for i = 1, tonumber(arg[5]) do print(string.format('%.17g', pseudoseed(arg[4]))) end
elseif command == 'random' then
    math.randomseed(tonumber(arg[3]))
    for i = 1, tonumber(arg[4]) do print(string.format('%.17g', math.random())) end
elseif command == 'bosses' then
    dofile(dir .. '/functions/common_events.lua')
    start(arg[3])
    G.P_BLINDS = game_table('P_BLINDS')
    G.GAME.bosses_used, G.GAME.banned_keys, G.GAME.win_ante = {}, {}, 8
    G.GAME.round_resets = {ante = 1}
    for k, v in pairs(G.P_BLINDS) do if v.boss then G.GAME.bosses_used[k] = 0 end end
    local out = {}
    for ante = 1, tonumber(arg[4]) do
        G.GAME.round_resets.ante = ante
        out[#out + 1] = get_new_boss()
    end
    print(table.concat(out, ' '))
elseif command == 'shop' then
    local B = dofile((arg[0]:gsub('oracle%.lua$', 'headless.lua')))(dir)
    G.SETTINGS.profile = 1
    G.PROFILES = {{career_stats = {}, high_scores = {}}}
    G:init_item_prototypes()
    for k, v in pairs(G.P_CENTERS) do v.unlocked = true; v.discovered = true end
    G.GAME = G:init_game_object()
    G.GAME.pseudorandom.seed = arg[3]
    G.GAME.pseudorandom.hashed_seed = pseudohash(arg[3])
    G.GAME.selected_back = Back(G.P_CENTERS.b_red)
    for _, n in ipairs({'jokers', 'consumeables', 'hand', 'deck', 'play', 'discard'}) do
        G[n] = CardArea(0, 0, 1, 1, {card_limit = 5, type = n == 'jokers' and 'joker' or n})
    end
    G.playing_cards = {}
    G.shop_jokers = CardArea(0, 0, 1, 1, {card_limit = 2, type = 'shop'})
    create_shop_card_ui = function() end
    -- start_run: the boss, the voucher, and the tags of the first ante.
    G.GAME.round_resets.blind_choices.Boss = get_new_boss()
    G.GAME.current_round.voucher = get_next_voucher_key()
    get_next_tag_key()
    get_next_tag_key()
    local function show()
        local out = {}
        for _, c in ipairs(G.shop_jokers.cards) do
            local e = c.edition and c.edition.type or ''
            out[#out + 1] = c.config.center.key .. (e ~= '' and (':' .. e) or '') ..
                (c.base and c.base.value and c.ability.set ~= 'Joker' and c.playing_card ~= nil and (':' .. c.base.value) or '')
        end
        print(table.concat(out, ' '))
    end
    for i = 1, 2 do G.shop_jokers:emplace(create_card_for_shop(G.shop_jokers)) end
    show()
    for r = 1, tonumber(arg[4]) do
        for i = #G.shop_jokers.cards, 1, -1 do
            local c = G.shop_jokers:remove_card(G.shop_jokers.cards[i])
            c:remove()
        end
        for i = 1, 2 do G.shop_jokers:emplace(create_card_for_shop(G.shop_jokers)) end
        show()
    end
    print('voucher ' .. G.GAME.current_round.voucher)
    print('packs ' .. get_pack('shop_pack').key .. ' ' .. get_pack('shop_pack').key)
elseif command == 'score' then
    dofile((arg[0]:gsub('oracle%.lua$', 'scenario.lua')))
elseif command == 'describe' then
    dofile((arg[0]:gsub('oracle%.lua$', 'describe.lua')))
elseif command == 'gen' then
    dofile((arg[0]:gsub('oracle%.lua$', 'gen_data.lua')))
elseif command == 'genchallenges' then
    dofile((arg[0]:gsub('oracle%.lua$', 'gen_challenges.lua')))
elseif command == 'string' then
    print(random_string(8, tonumber(arg[3])))
elseif command == 'deck' then
    -- The cards in the order of start_run: sorted by suit, then rank, as
    -- strings. Each card gets the next sort_id.
    start(arg[3])
    local protos = {}
    for k, v in pairs(game_table('P_CARDS')) do protos[#protos + 1] = {s = k:sub(1, 1), r = k:sub(3, 3)} end
    table.sort(protos, function(a, b) return a.s .. a.r < b.s .. b.r end)
    local deck = {}
    for i, p in ipairs(protos) do deck[i] = {sort_id = i, name = p.r .. p.s} end
    for i = 4, #arg do
        pseudoshuffle(deck, pseudoseed(arg[i]))
        local out = {}
        -- The top of the deck is the end of the list.
        for j = #deck, 1, -1 do out[#out + 1] = deck[j].name end
        print(table.concat(out, ' '))
    end
elseif command == 'shuffle' then
    local list = {}
    for i = 1, tonumber(arg[4]) do list[i] = {v = i} end
    pseudoshuffle(list, tonumber(arg[3]))
    local out = {}
    for i, item in ipairs(list) do out[i] = item.v end
    print(table.concat(out, " "))
end
