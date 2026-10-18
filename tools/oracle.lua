-- oracle.lua: runs functions of the original Balatro in LuaJIT, and prints
-- the results. The tests of Balatro4S hold these results as numbers.
-- It reads the Lua files of your own Balatro.love. It does not copy them.
-- Usage: luajit tools/oracle.lua DIR COMMAND [ARGS]
--   DIR is a directory with the unpacked Balatro.love.
--   rng SEED KEY N        print N values of pseudoseed(KEY) for SEED
--   hash TEXT             print pseudohash(TEXT)
--   random SEED N         print N values of math.random() after randomseed(SEED)
--   shuffle SEED N        print the shuffle of 1..N with pseudoshuffle

local dir, command = arg[1], arg[2]
G = {GAME = {pseudorandom = {}}, CONTROLLER = {}}
love = {graphics = {}, system = {}, filesystem = {}}
dofile(dir .. '/functions/misc_functions.lua')

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
elseif command == 'shuffle' then
    local list = {}
    for i = 1, tonumber(arg[4]) do list[i] = {v = i} end
    pseudoshuffle(list, tonumber(arg[3]))
    local out = {}
    for i, item in ipairs(list) do out[i] = item.v end
    print(table.concat(out, " "))
end
