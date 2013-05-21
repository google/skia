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

canvas = {}
total = 0

-- called with the parameters to each canvas.draw call
function accumulate(t)
    total = total + 1
    local n = canvas[t.verb] or 0
    canvas[t.verb] = n + 1

    -- enable to dump all of the parameters we were sent
    if false then
        -- dump the params in t, specifically showing the verb first, which we
        -- then nil out so it doesn't appear in tostr()
        io.write(t.verb, " ")
        t.verb = nil
        io.write(tostr(t), "\n")
    end
end

-- lua_pictures will call this function after all of the files have been
-- "accumulated"
function summarize()
    io.write("total ", total, "\n", tostr(canvas), "\n")
end

