--[[
    This file is used as the aggregator file when using telemetry for
    scrape_dashing_full.lua
]]

dashCount = 0
dashTable = {}

function increment_inner(table, key, value)
    table[key] = (table[key] or 0) + value
end

function increment(table, tableKey, key, value)
    if (table[tableKey] == nil) then
        table[tableKey] = {}
    end
    increment_inner(table[tableKey], key, value)
end

dofile("/tmp/lua-output")

io.write("Total dashed effects is: ", dashCount, "\n")
for k1, v1 in next, dashTable do
    io.write("\nTable: ", k1, "\n") 
    for k, v in next, v1 do
        io.write("\"", k, "\": ", v, "\n")
    end
end

