-- just a helper function to dump the parameters, for debugging
function tostr(t)
    local str = ""
    for k, v in next, t do
        if #str > 0 then
            str = str .. ", "
        end
        if type(k) == "number" then
            str = str .. "[" .. k .. "] = "
        else
            str = str .. tostring(k) .. " = "
        end
        if type(v) == "table" then
            str = str .. "{ " .. tostr(v) .. " }"
        else
            str = str .. tostring(v)
        end
    end
    return str
end

local total = {}    -- accumulate() stores its data in here
local canvas        -- holds the current canvas (from startcanvas())

--[[
    startcanvas() is called at the start of each picture file, passing the
    canvas that we will be drawing into, and the name of the file.
    
    Following this call, there will be some number of calls to accumulate(t)
    where t is a table of parameters that were passed to that draw-op.
    
        t.verb is a string holding the name of the draw-op (e.g. "drawRect")
    
    when a given picture is done, we call endcanvas(canvas, fileName)
]]
function sk_scrape_startcanvas(c, fileName)
    canvas = c
end

--[[
    Called when the current canvas is done drawing.
]]
function sk_scrape_endcanvas(c, fileName)
    canvas = nil
end

--[[
    Called with the parameters to each canvas.draw call, where canvas is the
    current canvas as set by startcanvas()
]]
function sk_scrape_accumulate(t)
    local n = total[t.verb] or 0
    total[t.verb] = n + 1

    if false and t.verb == "drawRect" then
        local m = canvas:getTotalMatrix()
        print("... ", tostr(m), "\n")
    end

    -- enable to dump all of the parameters we were sent
    if false then
        -- dump the params in t, specifically showing the verb first, which we
        -- then nil out so it doesn't appear in tostr()
        io.write(t.verb, " ")
        t.verb = nil
        io.write(tostr(t), "\n")
    end
end

--[[
    lua_pictures will call this function after all of the pictures have been
    "accumulated".
]]
function sk_scrape_summarize()
    io.write("\n{ ", tostr(total), " }\n")
end

