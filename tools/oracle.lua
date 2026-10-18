-- oracle.lua: runs functions of the original Balatro in LuaJIT, and prints
-- the results. The tests of Balatro4S hold these results as numbers.
-- It reads the Lua files of your own Balatro.love. It does not copy them.
-- Usage: luajit tools/oracle.lua DIR COMMAND [ARGS]
--   DIR is a directory with the unpacked Balatro.love.
--   rng SEED KEY N        print N values of pseudoseed(KEY) for SEED
--   hash TEXT             print pseudohash(TEXT)
--   random SEED N         print N values of math.random() after randomseed(SEED)
--   shuffle SEED N        print the shuffle of 1..N with pseudoshuffle
--   bosses SEED ANTES     print the boss of each ante from 1 to ANTES

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
elseif command == 'shuffle' then
    local list = {}
    for i = 1, tonumber(arg[4]) do list[i] = {v = i} end
    pseudoshuffle(list, tonumber(arg[3]))
    local out = {}
    for i, item in ipairs(list) do out[i] = item.v end
    print(table.concat(out, " "))
end
