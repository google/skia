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

    if false and t.verb == "drawRect" and t.paint:isAntiAlias() then
        local r = t.rect;
        local p = t.paint;
        local c = p:getColor();
        print("drawRect ", tostr(r), tostr(c), "\n")
    end

    if false and t.verb == "drawPath" then
        local pred, r1, r2, d1, d2 = t.path:isNestedFillRects()
        
        if pred then
            print("drawRect_Nested", tostr(r1), tostr(r2), d1, d2)
        else
            print("drawPath", "isEmpty", tostring(t.path:isEmpty()),
                    "isRect", tostring(t.path:isRect()), tostr(t.path:getBounds()))
        end
    end
end

--[[
    lua_pictures will call this function after all of the pictures have been
    "accumulated".
]]
function sk_scrape_summarize()
    io.write("\n{ ", tostr(total), " }\n")
end

