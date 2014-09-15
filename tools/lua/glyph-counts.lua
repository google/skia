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

local gCounts = {}  -- [fontID_pointsize] = [] unique glyphs
local gFirstGlyphs = {}
local gTotalCount = 0

function array_count(array)
    local n = 0
    for k in next, array do
        n = n + 1
    end
    return n
end

function sk_scrape_accumulate(t)
    verb = t.verb;
    if verb == "drawPosText" or verb == "drawPosTextH" then
        if t.glyphs then
            local key = array_count(t.glyphs)
            local n = gCounts[key]
            if n then
                gCounts[key] = n + 1
            else
                gCounts[key] = 1
            end
            
            if key == 1 then
                local first = t.glyphs[1];
                local n = gFirstGlyphs[first]
                if n then
                    n = n + 1
                else
                    n = 0
                end
                gFirstGlyphs[first] = n
            end

            gTotalCount = gTotalCount + 1
        end
    end
end

--[[
    lua_pictures will call this function after all of the pictures have been
    "accumulated".
]]
function sk_scrape_summarize()
    for k, v in next, gCounts do
        io.write("glyph_count ", k, ",frequency ", v * 100 / gTotalCount, "\n")
    end

--[[
    io.write("\n\nFirst glyph spread\n\n")
    for k, v in next, gFirstGlyphs do
        io.write("glyph, ", k, ",count, ", v, "\n")
    end
]]
end

function test_summary()
    io.write("just testing test_summary\n")
end

