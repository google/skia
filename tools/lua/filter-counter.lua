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

local gCF_Count = 0
local gIF_Count = 0
local gBOTH_Count = 0

function sk_scrape_accumulate(t)
    if not t.paint then
        return
    end

    local colorFilter = t.paint:getColorFilter()
    local imageFilter = t.paint:getImageFilter()

    if colorFilter then
        gCF_Count = gCF_Count + 1
    end
    if imageFilter then
        gIF_Count = gIF_Count + 1
    end
    if colorFilter and imageFilter then
        gBOTH_Count = gBOTH_Count + 1
    end
end

--[[
    lua_pictures will call this function after all of the pictures have been
    "accumulated".
]]
function sk_scrape_summarize()
    io.write("colorfilters ", gCF_Count, ", imagefilters ", gIF_Count, ", both_filters ", gBOTH_Count, "\n")

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

