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

local effects = {}

function count_fields(t)
    for k, v in next, t do
        effects[k] = (effects[k] or 0) + 1
    end
end

local total_paints = 0;

function sk_scrape_accumulate(t)
    if (t.paint) then
        total_paints = total_paints + 1
        count_fields(t.paint:getEffects())
    end
end

function sk_scrape_summarize()
    io.write("total paints ", total_paints, " ", tostr(effects), "\n");
end

