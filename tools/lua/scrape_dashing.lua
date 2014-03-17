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

local total_found = {}    -- accumulate() stores its data in here
local total_total = {}
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

function increment(table, key)
    table[key] = (table[key] or 0) + 1
end


local drawPointsTable = {}
local drawPointsTable_direction = {}

function sk_scrape_accumulate(t)
    increment(total_total, t.verb)

    local p = t.paint
    if p then
        local pe = p:getPathEffect();
        if pe then
            increment(total_found, t.verb)
        end
    end

    if "drawPoints" == t.verb then
        local points = t.points
        increment(drawPointsTable, #points)
        if 2 == #points then
            if points[1].y == points[2].y then
                increment(drawPointsTable_direction, "hori")
            elseif points[1].x == points[2].x then
                increment(drawPointsTable_direction, "vert")
            else
                increment(drawPointsTable_direction, "other")
            end
        end
    end
end

--[[
    lua_pictures will call this function after all of the pictures have been
    "accumulated".
]]
function sk_scrape_summarize()
    for k, v in next, total_found do
        io.write(k, " = ", v, "/", total_total[k], "\n")
    end
    print("histogram of point-counts for all drawPoints calls")
    print(tostr(drawPointsTable))
    print(tostr(drawPointsTable_direction))
end

