function tostr(t)
    local str = ""
    for k, v in next, t do
        str = str .. tostring(k) .. " "
        if type(v) == "table" then
            str = str .. "{ " .. tostr(v) .. "} "
        else
            str = str .. tostring(v) .. " "
        end
    end
    return str
end

canvas = {}
total = 0

function accumulate(t)
    local verb = t.verb
    t.verb = nil

    total = total + 1
    local n = canvas[verb] or 0
    n = n + 1
    canvas[verb] = n

    io.write(verb, " ")
    io.write(tostr(t), "\n")
end

function summarize()
    io.write("total ", total, "\n", tostr(canvas), "\n")
end

--[[
function drawsomething()
    local s = skia_newsurface(100, 100)
    local c = s:getcanvas();
    c:setColor(1, 0, 0, 1)
    c:drawRect(10, 10, 50, 50)
    s:saveImage("image.png")
end
--]]

