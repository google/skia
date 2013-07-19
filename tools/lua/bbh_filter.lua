-- bbh_filter.lua
--
-- This script outputs info about 'interesting' skp files,
-- where the definition of 'interesting' changes but is roughly:
-- "Interesting for bounding box hierarchy benchmarks."
--
-- Currently, the approach is to output, in equal ammounts, the names of the files that
-- have most commands, and the names of the files that use the least popular commands.

function count_entries(table)
    local count = 0
    for _,_ in pairs(table) do
        count = count + 1
    end
    return count
end

verbCounts = {}

function reset_current()
    -- Data about the skp in transit
    currentInfo = {
        fileName = '',
        verbs = {},
        numOps = 0
    }
end
reset_current()

numOutputFiles = 10  -- This is per measure.
globalInfo = {}  -- Saves currentInfo for each file to be used at the end.
output = {}  -- Stores {fileName, {verb, count}} tables.

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

function sk_scrape_endcanvas(c, fileName)
    globalInfo[fileName] = currentInfo
    globalInfo[fileName].fileName = fileName
    reset_current()
end

function sk_scrape_accumulate(t)
    -- dump the params in t, specifically showing the verb first, which we
    -- then nil out so it doesn't appear in tostr()
    --
    verbCounts[t.verb] = (verbCounts[t.verb] or 0) + 1
    currentInfo.verbs[t.verb] = (currentInfo.verbs[t.verb] or 0) + 1
    currentInfo.numOps = currentInfo.numOps + 1

    t.verb = nil
end

function sk_scrape_summarize()
    verbWeights = {}  -- {verb, weight}, where 0 < weight <= 1

    meta = {}
    for k,v in pairs(verbCounts) do
        table.insert(meta, {key=k, value=v})
    end
    table.sort(meta, function (a,b) return a.value > b.value; end)
    maxValue = meta[1].value
    io.write("-- ==================\n")
    io.write("------------------------------------------------------------------ \n")
    io.write("-- Command\t\t\tNumber of calls\t\tPopularity\n")
    io.write("------------------------------------------------------------------ \n")
    for k, v in pairs(meta) do
        verbWeights[v.key] = v.value / maxValue

        -- Poor man's formatting:
        local padding = "\t\t\t"
        if (#v.key + 3) < 8 then
            padding = "\t\t\t\t"
        end
        if (#v.key + 3) >= 16 then
            padding = "\t\t"
        end

        io.write ("-- ",v.key, padding, v.value, '\t\t\t', verbWeights[v.key], "\n")
    end

    meta = {}
    function calculate_weight(verbs)
        local weight = 0
        for name, count in pairs(verbs) do
            weight = weight + (1 / verbWeights[name]) * count
        end
        return weight
    end
    for n, info in pairs(globalInfo) do
        table.insert(meta, info)
    end

    local visitedFiles = {}

    -- Prints out information in lua readable format
    function output_with_metric(metric_func, description, numOutputFiles)
        table.sort(meta, metric_func)
        print(description)
        local iter = 0
        for i, t in pairs(meta) do
            if not visitedFiles[t.fileName] then
                visitedFiles[t.fileName] = true
                io.write ("{\nname = \"", t.fileName, "\", \nverbs = {\n")
                for verb,count in pairs(globalInfo[t.fileName].verbs) do
                    io.write('    ', verb, " = ", count, ",\n")
                end
                io.write("}\n},\n")

                iter = iter + 1
                if iter >= numOutputFiles then
                    break
                end
            end
        end
    end

    output_with_metric(
        function(a, b) return calculate_weight(a.verbs) > calculate_weight(b.verbs); end,
        "\n-- ================== skps with calling unpopular commands.", 10)
    output_with_metric(
        function(a, b) return a.numOps > b.numOps; end,
        "\n-- ================== skps with the most calls.", 50)

    local count = count_entries(visitedFiles)

    print ("-- Spat", count, "files")
end

