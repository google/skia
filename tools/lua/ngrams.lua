-- Generate n-grams of Skia API calls from SKPs.

-- To test this locally, run:
-- $ GYP_DEFINES="skia_shared_lib=1" make lua_pictures
-- $ out/Debug/lua_pictures -q -r $SKP_DIR -l tools/lua/ngrams.lua > /tmp/lua-output
-- $ lua tools/lua/ngrams_aggregate.lua

-- To run on Cluster Telemetry, copy and paste the contents of this file into
-- the box at https://skia-tree-status.appspot.com/skia-telemetry/lua_script,
-- and paste the contents of ngrams_aggregate.lua into the "aggregator script"
-- box on the same page.

-- Change n as desired.
-- CHANGEME
local n = 3
-- CHANGEME

-- This algorithm uses a list-of-lists for each SKP. For API call, append a
-- list containing just the verb to the master list. Then, backtrack over the
-- last (n-1) sublists in the master list and append the verb to those
-- sublists. At the end of execution, the master list contains a sublist for
-- every verb in the SKP file. Each sublist has length n, with the exception of
-- the last n-1 sublists, which are discarded in the summarize() function,
-- which generates counts for each n-gram.

local ngrams = {}
local currentFile = ""

function sk_scrape_startcanvas(c, fileName)
  currentFile = fileName
  ngrams[currentFile] = {}
end

function sk_scrape_endcanvas(c, fileName)
end

function sk_scrape_accumulate(t)
  table.insert(ngrams[currentFile], {t.verb})
  for i = 1, n-1 do
    local idx = #ngrams[currentFile] - i
    if idx > 0 then
      table.insert(ngrams[currentFile][idx], t.verb)
    end
  end
end

function sk_scrape_summarize()
  -- Count the n-grams.
  local counts = {}
  for file, ngramsInFile in pairs(ngrams) do
    for i = 1, #ngramsInFile - (n-1) do
      local ngram = table.concat(ngramsInFile[i], " ")
      if counts[ngram] == nil then
        counts[ngram] = 1
      else
        counts[ngram] = counts[ngram] + 1
      end
    end
  end

  -- Write out code for aggregating.
  for ngram, count in pairs(counts) do
    io.write("if counts['", ngram, "'] == nil then counts['", ngram, "'] = ", count, " else counts['", ngram, "'] = counts['", ngram, "'] + ", count, " end\n")
  end
end
