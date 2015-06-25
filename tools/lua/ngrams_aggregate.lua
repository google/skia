-- Aggregate the output from ngrams.lua.

-- Get the data from all shards.
counts = {}
dofile("/tmp/lua-output")

-- Put the data into a sortable "array".
countArray = {}
for ngram, count in pairs(counts) do
  table.insert(countArray, {count, ngram})
end

-- Sort the data.
function compare(a, b)
  return a[1] > b[1]
end
table.sort(countArray, compare)

-- Write the result.
for i, countPair in ipairs(countArray) do
  io.write(countPair[1], "\t", countPair[2], "\n")
end
