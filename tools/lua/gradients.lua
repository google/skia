function sk_scrape_startcanvas(c, fileName) end
function sk_scrape_endcanvas(c, fileName) end

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
                gradients[i].colorCount        = g.colorCount
                gradients[i].type              = g.type
                gradients[i].tile              = g.tile
                gradients[i].isEvenlySpaced    = g.isEvenlySpaced
                gradients[i].containsHardStops = g.containsHardStops

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

        io.write(string.format("%d %s %s %d %d %s\n",
                                v.colorCount,
                                v.type,
                                v.tile,
                                tonumber(v.isEvenlySpaced and 1 or 0),
                                tonumber(v.containsHardStops and 1 or 0),
                                pos))
    end
end

