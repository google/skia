stats = {}

-- switch this to run on the automated scraper system
newline = "\n"
-- newline = "\\n"

function sk_scrape_startcanvas(c, fileName)
    canvas = c
    oldstackstr = "<invalid>"
end

function sk_scrape_endcanvas(c, fileName)
    canvas = nil
end

function string.starts(String,Start)
   return string.sub(String,1,string.len(Start))==Start
end

function build_stack_string(stack)
    local info = ""
    for i = 1, #stack do
        local element = stack[i];
        info = info .. element["op"] .. ", " .. element["type"] .. ", aa:" .. tostring(element["aa"])
        if (element["type"] == "path") then
            if (element["path"]:getSegmentTypes() == "line" and element["path"]:isConvex()) then
                info = info .. ", convex_poly " .. element["path"]:countPoints() .. " points"
            else
                info = info .. ", fill: " .. element["path"]:getFillType()
                info = info .. ", segments: (" .. element["path"]:getSegmentTypes() .. ")"
                info = info .. ", convex:" .. tostring(element["path"]:isConvex())
            end
        end
        info = info .. newline
    end
    return info
end

function sk_scrape_accumulate(t)
    if (string.starts(t.verb, "draw")) then
        local stack = canvas:getReducedClipStack()
        local stackstr = build_stack_string(stack)
        if (stackstr ~= "") then
            if (stats[stackstr] == nil) then
                stats[stackstr] = {}
                stats[stackstr].drawCnt = 0
                stats[stackstr].instanceCnt = 0
            end
            stats[stackstr].drawCnt = stats[stackstr].drawCnt + 1
            if (stackstr ~= oldstackstr) then
                stats[stackstr].instanceCnt = stats[stackstr].instanceCnt + 1
            end
        end
        oldstackstr = stackstr
    end
end

function print_stats(stats)
    function sort_by_draw_cnt(a, b)
        return a.data.drawCnt > b.data.drawCnt
    end
    array = {}
    for k,v in pairs(stats) do
        array[#array + 1] = { name = k, data = v }
    end
    table.sort(array, sort_by_draw_cnt)
    for i = 1, #array do
        io.write("\n-------\n", array[i].name, tostring(array[i].data.drawCnt), " draws, ", tostring(array[i].data.instanceCnt), " instances.\n")
    end
end

function sk_scrape_summarize()
    print_stats(stats)
    --[[ To use the web scraper comment out the print above, run the code below to generate an
         aggregate table on the automated scraper system. Then use the print_stats function on
         agg_stats in the aggregator step.
    for k,v in pairs(stats) do
        if (v.drawCnt ~= nil) then
             -- io.write("\n-------\n", k, tostring(v.drawCnt), " draws, ", tostring(v.instanceCnt), " instances.\n")
             local tableEntry = 'agg_stats["' .. k .. '"]'
             io.write(tableEntry, " = ", tableEntry, " or {}\n")
             io.write(tableEntry, ".drawCnt = (", tableEntry, ".drawCnt or 0 ) + ", v.drawCnt, "\n")
             io.write(tableEntry, ".instanceCnt = (", tableEntry, ".instanceCnt or 0 ) + ", v.instanceCnt, "\n")
        end
    end
    --]]
end
