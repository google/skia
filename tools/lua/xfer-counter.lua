
function sk_scrape_startcanvas(c, fileName)
end

function sk_scrape_endcanvas(c, fileName)
end

local gXM_Count = 0
local gXferModeTab = {}

function sk_scrape_accumulate(t)
    if not t.paint then
        return
    end

    local xferMode = t.paint:getXfermode()

    if xferMode then
        local modeName = xferMode:getTypeName()

        if gXferModeTab[modeName] == nil then
            gXferModeTab[modeName] = 1;
        else
            gXferModeTab[modeName] = gXferModeTab[modeName] + 1
        end
        gXM_Count = gXM_Count + 1
    end
end

function sk_scrape_summarize()
    for key,value in pairs(gXferModeTab) do
        io.write(key, ": ", value, "\n")
    end
    io.write("total: ", gXM_Count)
end

function test_summary()
    io.write("just testing test_summary\n")
end

