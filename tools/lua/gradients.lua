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

function sk_scrape_startcanvas(c, fileName) end

function sk_scrape_endcanvas(c, fileName) end

function sk_scrape_accumulate(t)
    local p = t.paint
    if p then
        local s = p:getShader()
        if s then
            local g = s:asAGradient()
            if g then
                io.write(g.type, " gradient with ", g.colorCount, " colors\n")
            else
                local b = s:asABitmap()
                if b then
                    io.write("bitmap ", b.genID, " width=", b.width, " height=", b.height, "\n")
                end
            end
        end
    end
end

function sk_scrape_summarize() end

