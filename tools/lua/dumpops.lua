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
    -- dump the params in t, specifically showing the verb first, which we
    -- then nil out so it doesn't appear in tostr()
    io.write(t.verb, " ")
    t.verb = nil
    io.write(tostr(t), "\n")
end

function sk_scrape_summarize() end

