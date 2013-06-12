local canvas

function sk_scrape_startcanvas(c, fileName)
    canvas = c
end

function sk_scrape_endcanvas(c, fileName)
    canvas = nil
end

local glyph_calls = 0
local unichar_calls = 0

local isTextVerbs = {
    drawPosText = true,
    drawPosTextH = true,
    drawText = true,
    drawTextOnPath = true,
}

function sk_scrape_accumulate(t)
    if isTextVerbs[t.verb] then
        if t.glyphs then
            glyph_calls = glyph_calls + 1
        else
            unichar_calls = unichar_calls + 1
        end
    end
end

function sk_scrape_summarize()
    io.write("glyph calls = ", glyph_calls,
             ", unichar calls = ", unichar_calls, "\n");
end

