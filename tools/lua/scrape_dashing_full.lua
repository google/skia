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
    Use to initialize all keys passed in keyTable to zero in table.
    Useful so that keys that are never get incremented still output zero at end
]]
function resetTableKeys(table, keyTable)
    for k, v in next, keyTable do
        table[v] = 0
    end
end

function increment(table, key)
    table[key] = (table[key] or 0) + 1
end

local dashCount = 0

local total_found = {}
local drawPoints_count = {}
local drawPoints_direction = {}
resetTableKeys(drawPoints_direction, {"hori", "vert", "other"})
local dashInterval_count = {}
local dashInterval_pattern = {}
resetTableKeys(dashInterval_pattern, {"one_one", "zero_on", "other"})
local dash_phase = {}
resetTableKeys(dash_phase, {"zero", "other"})
local dash_cap = {}
resetTableKeys(dash_cap, {"butt", "round", "square"})

local dashTable = {}
dashTable.total_found = total_found
dashTable.drawPoints_count = drawPoints_count
dashTable.drawPoints_direction = drawPoints_direction
dashTable.dashInterval_count = dashInterval_count
dashTable.dashInterval_pattern = dashInterval_pattern
dashTable.dash_phase = dash_phase
dashTable.dash_cap = dash_cap

function sk_scrape_accumulate(t)
    local p = t.paint
    if p then
        local pe = p:getPathEffect()
        if pe then
            local de = pe:asADash()
            if de then
                dashCount = dashCount + 1
                increment(total_found, t.verb);
                increment(dashInterval_count, #de.intervals)
                if 2 == #de.intervals then
		    if 1 == de.intervals[1] and 1 == de.intervals[2] then
                        increment(dashInterval_pattern, "one_one")
                    elseif 0 == de.intervals[1] then
                        increment(dashInterval_pattern, "zero_on")
                    else
                        increment(dashInterval_pattern, "other")
                    end
                end

                if 0 == de.phase then
                    increment(dash_phase, "zero")
                else
                    increment(dash_phase, "other")
                end

                local cap = p:getStrokeCap()
                if 0 == cap then
                    increment(dash_cap, "butt")
                elseif 1 == cap then
                    increment(dash_cap, "round")
                else
                    increment(dash_cap, "square")
                end

                if "drawPoints" == t.verb then
                    local points = t.points
                    increment(drawPoints_count, #points)
                    if 2 == #points then
                        if points[1].y == points[2].y then
                            increment(drawPoints_direction, "hori")
                        elseif points[1].x == points[2].x then
                            increment(drawPoints_direction, "vert")
                        else
                            increment(drawPoints_direction, "other")
                        end
                    end
                end

                --[[
                    eventually would like to print out info on drawPath verbs with dashed effect
                ]]
                if "drawPath" == t.verb then
                end

            end
        end
    end
end

--[[
    lua_pictures will call this function after all of the pictures have been
    "accumulated".
]]
function sk_scrape_summarize()
-- use for non telemetry
--[[
    io.write("Total dashed effects is: ", dashCount, "\n");
    for k1, v1 in next, dashTable do
        io.write("\nTable: ", k1, "\n") 
        for k, v in next, v1 do
            io.write("\"", k, "\": ", v, "\n")
        end
    end
]]

-- use for telemetry
    io.write("\ndashCount = dashCount + ", tostring(dashCount), "\n")
    for k1, v1 in next, dashTable do
        for k, v in next, v1 do
            io.write("\nincrement(dashTable, \"", k1, "\", \"", k, "\", ", v, ")\n")
        end
    end
end

