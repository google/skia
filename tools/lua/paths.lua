--
-- Copyright 2014 Google Inc.
--
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.
--

-- Path scraping script.
-- This script is designed to count the number of times we fall back to software
-- rendering for a path in a given SKP. However, this script does not count an exact
-- number of uploads, since there is some overlap with clipping: e.g. two clipped paths
-- may cause three uploads to the GPU (set clip 1, set clip 2, unset clip 2/reset clip 1),
-- but these cases are rare.

draws = 0
drawPaths = 0
drawPathsAnti = 0
drawPathsConvexAnti = 0

clips = 0
clipPaths = 0
clipPathsAnti = 0
clipPathsConvexAnti = 0

usedPath = false
usedSWPath = false

skpsTotal = 0
skpsWithPath = 0
skpsWithSWPath = 0

function sk_scrape_startcanvas(c, fileName)
   usedPath = false
   usedSWPath = false
end

function sk_scrape_endcanvas(c, fileName)
   skpsTotal = skpsTotal + 1
   if usedPath then
      skpsWithPath = skpsWithPath + 1
      if usedSWPath then
         skpsWithSWPath = skpsWithSWPath + 1
      end
   end
end

function string.starts(String,Start)
   return string.sub(String,1,string.len(Start))==Start
end

function isPathValid(path)
   if not path then
      return false
   end

   if path:isEmpty() then
      return false
   end

   if path:isRect() then
      return false
   end

   return true
end

function sk_scrape_accumulate(t)
   if (string.starts(t.verb, "draw")) then
      draws = draws + 1
   end

   if (string.starts(t.verb, "clip")) then
      clips = clips + 1
   end

   if t.verb == "clipPath" then
      local path = t.path
      if isPathValid(path) then
         clipPaths = clipPaths + 1
         usedPath = true
         if t.aa then
            clipPathsAnti = clipPathsAnti + 1
            if path:isConvex() then
               clipPathsConvexAnti = clipPathsConvexAnti + 1
            else
               usedSWPath = true
            end
         end
      end
   end

   if t.verb == "drawPath" then
      local path = t.path
      local paint = t.paint
      if paint and isPathValid(path) then
         drawPaths = drawPaths + 1
         usedPath = true
         if paint:isAntiAlias() then
            drawPathsAnti = drawPathsAnti + 1
            if path:isConvex() then
               drawPathsConvexAnti = drawPathsConvexAnti + 1
            else
               usedSWPath = true
            end
         end
      end
   end
end

function sk_scrape_summarize() 
   local swDrawPaths = drawPathsAnti - drawPathsConvexAnti
   local swClipPaths = clipPathsAnti - clipPathsConvexAnti

   io.write("clips = clips + ", clips, "\n");
   io.write("draws = draws + ", draws, "\n");
   io.write("clipPaths = clipPaths + ", clipPaths, "\n");
   io.write("drawPaths = drawPaths + ", drawPaths, "\n");
   io.write("swClipPaths = swClipPaths + ", swClipPaths, "\n");
   io.write("swDrawPaths = swDrawPaths + ", swDrawPaths, "\n");

   io.write("skpsTotal = skpsTotal + ", skpsTotal, "\n");
   io.write("skpsWithPath = skpsWithPath + ", skpsWithPath, "\n");
   io.write("skpsWithSWPath = skpsWithSWPath + ", skpsWithSWPath, "\n");
end
