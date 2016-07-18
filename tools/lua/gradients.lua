filename = ""

function sk_scrape_startcanvas(c, fileName)
    filename = fileName
end

function sk_scrape_endcanvas(c, fileName)

end

LuaDoubleNearlyZero = 1.0 / bit32.lshift(1.0, 12)

function LuaDoubleNearlyEqual(a, b)
    return math.abs(a-b) <= LuaDoubleNearlyZero
end

function bounds(rect)
    local width  = rect.right  - rect.left
    local height = rect.bottom - rect.top

    return width, height
end

gradients = {}

i = 1

function sk_scrape_accumulate(t)
    local p = t.paint
    if p then
        local s = p:getShader()
        if s then
            local g = s:asAGradient()
            if g then
                gradients[i] = {}

                gradients[i].filename = filename

                local width, height = -1, -1
                if t.rect then
                    width, height = bounds(t.rect)
                elseif t.rrect then
                    width, height = bounds(t.rrect:rect())
                elseif t.path then
                    width, height = bounds(t.path:getBounds())
                end
                gradients[i].boundsWidth  = width
                gradients[i].boundsHeight = height

                gradients[i].colorCount = g.colorCount
                gradients[i].type       = g.type
                gradients[i].tile       = g.tile

                isEvenlySpaced = true
                for j = 1, g.colorCount, 1 do
                    if not LuaDoubleNearlyEqual(g.positions[j], (j-1)/(g.colorCount-1)) then
                        isEvenlySpaced = false
                    end
                end
                gradients[i].isEvenlySpaced = isEvenlySpaced

                numHardStops = 0
                for j = 2, g.colorCount, 1 do
                    if LuaDoubleNearlyEqual(g.positions[j], g.positions[j-1]) then
                        numHardStops = numHardStops + 1
                    end
                end
                gradients[i].numHardStops = numHardStops

                gradients[i].verb = t.verb
                
                gradients[i].positions = {}
                for j = 1, g.colorCount, 1 do
                    gradients[i].positions[j] = g.positions[j]
                end

                i = i + 1
            end
        end
    end
end

function sk_scrape_summarize()
    for k, v in pairs(gradients) do
        local pos = ""
        for j = 1, v.colorCount , 1 do
            pos = pos .. v.positions[j]
            if j ~= v.colorCount then
                pos = pos .. ","
            end
        end

        io.write(string.format("%s %d %s %s %d %d %s %d %d %s\n",
                                v.filename,
                                v.colorCount,
                                v.type,
                                v.tile,
                                tonumber(v.isEvenlySpaced and 1 or 0),
                                v.numHardStops,
                                v.verb,
                                v.boundsWidth,
                                v.boundsHeight,
                                pos))
    end
end

