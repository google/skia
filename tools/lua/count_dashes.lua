--
-- Copyright 2016 Google Inc.
--
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.
--

-- Dashed path scraping script.
-- This script is designed to count the total number of dashes in a dashed path
-- by computing the fill path and then counting how many individual segments are
-- inside the resulting fill path.

dashes = 0

pathPieces = {}

function sk_scrape_startcanvas(c, fileName)
end

function sk_scrape_endcanvas(c, fileName)
end

function sk_scrape_accumulate(t)
    local paint = t.paint
    if paint then
        local pe = paint:getPathEffect()
        if pe then
            if t.verb == "drawPath" and pe:asADash() then
                dashes = dashes + 1
                pathPieces[dashes] = 0

                local path = t.path
                local fillpath = paint:getFillPath(path)
                local verbs = fillpath:getVerbs()
                for _, verb in ipairs(verbs) do
                    if verb == "move" then
                       pathPieces[dashes] = pathPieces[dashes] + 1
                    end
                end
            end
        end
    end
end

-- We mulitply by two because for each segment of the dash, we do two measurements:
-- One for the beginning and one for the end of each dash.
function sk_scrape_summarize() 
    local pieces5 = 0;
    local pieces10 = 0;
    for _, p in ipairs(pathPieces) do
        local pieces = 2*p
        if pieces < 5 then
            pieces5 = pieces5 + 1
        end
        if pieces > 5 and pieces < 10 then
            pieces10 = pieces10 + 1
        end
    end
    io.write(string.format("%d %d %d\n", 2*dashes, pieces5, pieces10))
end
