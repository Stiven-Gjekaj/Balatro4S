-- headless.lua: loads the game code of your own Balatro.love with no
-- window, no sound, and no animation, so that a script can run the rules.
-- The Lua files stay in Balatro.love. This file only stubs the parts of
-- LOVE and of the screen that the rules call.
-- Usage, from another Lua file run by oracle-host:
--   local B = dofile('tools/headless.lua')(DIR)

return function(dir)
    local function load(name) return dofile(dir .. '/' .. name) end

    -- LOVE: the rules ask only for the OS and a few file calls.
    love = setmetatable({
        system = {getOS = function() return 'Linux' end},
        filesystem = {getInfo = function() return nil end, read = function() return nil end,
                      write = function() end, append = function() end, createDirectory = function() end},
        graphics = setmetatable({}, {__index = function() return function() end end}),
        timer = {getTime = function() return 0 end},
    }, {__index = function() return setmetatable({}, {__index = function() return function() end end}) end})

    load('engine/object.lua')
    -- Every object of the screen: the rules only need T and a few no-op calls.
    Node = Object:extend()
    function Node:init(args) self.T = {x = 0, y = 0, w = 1, h = 1, r = 0, scale = 1} self.children = {} end
    Moveable = Node:extend()
    function Moveable:init(X, Y, W, H)
        self.T = {x = X or 0, y = Y or 0, w = W or 1, h = H or 1, r = 0, scale = 1}
        self.VT = {x = 0, y = 0, w = 1, h = 1, r = 0, scale = 1}
        self.velocity = {x = 0, y = 0, r = 0, scale = 0}
        self.pinch = {x = false, y = false}
        self.role = {}
        self.alignment = {}
        self.states = {visible = true, collide = {}, focus = {}, hover = {}, click = {}, drag = {}, release_on = {}}
        self.children = {}
        self.ID = (G and G.ID or 0) + 1
        if G then G.ID = self.ID end
    end
    local noop_methods = {'juice_up', 'set_role', 'hard_set_T', 'align_to_major', 'remove', 'draw',
                          'set_alignment', 'move', 'calculate_parrallax'}
    for _, m in ipairs(noop_methods) do Moveable[m] = function() end end
    Sprite = Moveable:extend()
    AnimatedSprite = Sprite:extend()
    UIBox = Moveable:extend()
    function UIBox:init() Moveable.init(self) end
    function UIBox:get_UIE_by_ID() return setmetatable({config = {}}, {__index = function() return function() end end}) end
    DynaText = Moveable:extend()
    Particles = Moveable:extend()

    -- The events run at once, in order. The screen effects in them do nothing.
    Event = Object:extend()
    function Event:init(config)
        self.func = config.func or function() return true end
        -- An ease event moves a value. Here it jumps to the end value.
        if config.trigger == 'ease' or config.ease_to ~= nil then
            local f = config.func
            self.func = function()
                local value = config.ease_to
                if f then value = f(value) end
                config.ref_table[config.ref_value] = value
                return true
            end
        end
    end
    local queue = {}
    local manager = {}
    function manager:add_event(event)
        queue[#queue + 1] = event
    end
    function manager:clear_queue() queue = {} end
    -- Runs the events until none is left. An event that returns false runs again.
    function manager:run_all()
        local guard = 0
        while #queue > 0 and guard < 100000 do
            guard = guard + 1
            local event = table.remove(queue, 1)
            local ok, done = xpcall(event.func, debug.traceback)
            if not ok then error(done) end
            if done == false then queue[#queue + 1] = event end
        end
    end

    -- Game and its globals, with no window.
    load('game.lua')
    Game.init = function(self)
        self.ID = 0
        G = self
        self:set_globals()
    end
    load('engine/string_packer.lua')
    load('functions/misc_functions.lua')
    load('globals.lua')
    G.E_MANAGER = manager
    load('functions/common_events.lua')
    load('functions/state_events.lua')
    load('functions/button_callbacks.lua')
    load('functions/UI_definitions.lua')
    load('card.lua')
    load('cardarea.lua')
    load('blind.lua')
    load('back.lua')
    load('tag.lua')

    -- The screen calls of the rules do nothing.
    local function nothing() end
    -- The real functions that the stubs replace, for a script that needs one.
    local real = {}
    -- update_hand_text keeps the last chips and mult, which are the result.
    local shown = {}
    function update_hand_text(config, vals)
        for k, v in pairs(vals or {}) do shown[k] = v end
    end
    -- card_eval_status_text shows a message. Its one rule: new playing
    -- cards go to the jokers.
    function card_eval_status_text(card, eval_type, amt, percent, dir, extra)
        if extra and extra.playing_cards_created then
            playing_card_joker_effects(extra.playing_cards_created)
        end
    end
    for _, name in ipairs({'play_sound', 'delay', 'juice_card',
                           'juice_card_until', 'check_for_unlock', 'highlight_card', 'attention_text',
                           'discover_card', 'unlock_card', 'set_hand_usage', 'set_joker_usage',
                           'set_consumeable_usage', 'set_voucher_usage', 'check_and_set_high_score',
                           'inc_career_stat', 'ease_background_colour', 'ease_background_colour_blind',
                           'ease_colour', 'save_run', 'stop_use', 'play_area_status_text'}) do
        real[name] = _G[name]
        _G[name] = nothing
    end
    for _, name in ipairs({'start_materialize', 'juice_up', 'align_h_popup', 'hover', 'stop_hover', 'flip'}) do
        Card[name] = nothing
    end
    -- The sprites are empty objects, so that the code that sets their size runs.
    function Card:set_sprites(center, front)
        self.children.center = self.children.center or Moveable(0, 0, 0, 0)
        self.children.back = self.children.back or Moveable(0, 0, 0, 0)
        if front then self.children.front = self.children.front or Moveable(0, 0, 0, 0) end
    end
    -- A card that dissolves or breaks leaves in an event, as after its
    -- animation. A loop over the jokers that dissolves some of them sees
    -- each joker.
    local function remove_later(self)
        self.REMOVED = true
        G.E_MANAGER:add_event(Event({func = function() self:remove(); return true end}))
    end
    Card.start_dissolve = remove_later
    function Card:shatter() self.shattered = true; remove_later(self) end
    -- Card:remove of Balatro runs, with no screen objects to remove.
    remove_all = nothing
    function CardArea:init(X, Y, W, H, config)
        Moveable.init(self, X, Y, W, H)
        self.config = config or {}
        self.cards = {}
        self.children = {}
        self.highlighted = {}
        self.config.highlighted_limit = config and config.highlight_limit or 5
        self.config.card_limit = config and config.card_limit or 52
        self.config.temp_limit = self.config.card_limit
        self.config.card_count = 0
        self.config.type = config and config.type or 'deck'
        self.config.sort = 'desc'
    end
    for _, name in ipairs({'align_cards', 'hard_set_cards', 'set_ranks', 'draw'}) do CardArea[name] = nothing end
    -- An object that takes any call and any field: the parts of the screen
    -- that the rules touch, as G.HUD_blind.
    local any
    any = setmetatable({}, {__index = function() return any end, __call = function() return any end,
                            __newindex = function() end})
    for _, name in ipairs({'set_text', 'change_colour', 'alert_debuff', 'juice_up'}) do Blind[name] = nothing end
    local blind_init = Blind.init
    function Blind:init(...)
        Moveable.init(self, ...)
        self.children = {animatedSprite = any}
        self.config = {}
        self.tilt_var = {mx = 0, my = 0, amt = 0}
        self.loc_debuff_lines = {'', ''}
        self.shadow_height = 0
        self.ambient_tilt = 0.3
        self.chips = 0
        self.zoom = true
        self.states.collide.can = true
    end
    G.ARGS = G.ARGS or {}
    G.ARGS.spin = {amount = 0, real = 0, eased = 0}
    G.HUD_blind = any
    G.HUD = any
    G.ROOM = {jiggle = 0, T = {x = 0, y = 0, w = 20, h = 11.5}}
    G.ROOM_ATTACH = any
    G.I = {CARD = {}, CARDAREA = {}, NODE = {}, MOVEABLE = {}, SPRITE = {}, UIBOX = {}, POPUP = {}, ALERT = {}}
    localize = function(args) return type(args) == 'table' and (args.key or '') or args end

    -- The pools, as init_item_prototypes makes them.
    function G:headless_prototypes()
        self.P_CENTER_POOLS = {Booster = {}, Default = {}, Enhanced = {}, Edition = {}, Joker = {}, Tarot = {},
                               Planet = {}, Tarot_Planet = {}, Spectral = {}, Consumeables = {}, Voucher = {},
                               Back = {}, Tag = {}, Seal = {}, Stake = {}, Demo = {}}
        self.P_JOKER_RARITY_POOLS = {{}, {}, {}, {}}
        for k, v in pairs(self.P_TAGS) do
            v.key = k
            table.insert(self.P_CENTER_POOLS['Tag'], v)
        end
        for k, v in pairs(self.P_SEALS) do
            v.key = k
            table.insert(self.P_CENTER_POOLS['Seal'], v)
        end
        for k, v in pairs(self.P_STAKES) do
            v.key = k
            table.insert(self.P_CENTER_POOLS['Stake'], v)
        end
        for k, v in pairs(self.P_CENTERS) do
            v.key = k
            if v.set == 'Joker' then table.insert(self.P_CENTER_POOLS['Joker'], v) end
            if v.set and v.demo and v.pos then table.insert(self.P_CENTER_POOLS['Demo'], v) end
            if not v.wip then
                if v.set and v.set ~= 'Joker' and not v.skip_pool and not v.omit then table.insert(self.P_CENTER_POOLS[v.set], v) end
                if v.set == 'Tarot' or v.set == 'Planet' then table.insert(self.P_CENTER_POOLS['Tarot_Planet'], v) end
                if v.consumeable then table.insert(self.P_CENTER_POOLS['Consumeables'], v) end
                if v.rarity and v.set == 'Joker' and not v.demo then table.insert(self.P_JOKER_RARITY_POOLS[v.rarity], v) end
            end
        end
        for _, pool in pairs(self.P_CENTER_POOLS) do
            table.sort(pool, function(a, b) return (a.order or 0) < (b.order or 0) end)
        end
        for i = 1, 4 do
            table.sort(self.P_JOKER_RARITY_POOLS[i], function(a, b) return a.order < b.order end)
        end
        -- Each card is unlocked, as on a profile that has unlocked all.
        for k, v in pairs(self.P_CENTERS) do v.unlocked = true; v.discovered = true end
    end

    return {manager = manager, shown = shown, real = real}
end
