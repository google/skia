function string.startsWith(String,Start)
   return string.sub(String,1,string.len(Start))==Start
end

function string.endsWith(String,End)
   return End=='' or string.sub(String,-string.len(End))==End
end

local canvas = nil
local num_perspective_bitmaps = 0
local num_affine_bitmaps = 0
local num_scaled_bitmaps = 0
local num_translated_bitmaps = 0
local num_identity_bitmaps = 0
local num_scaled_up = 0
local num_scaled_down = 0

function sk_scrape_startcanvas(c, fileName) 
  canvas = c
end

function sk_scrape_endcanvas(c, fileName)
  canvas = nil
end

function sk_scrape_accumulate(t)
    -- dump the params in t, specifically showing the verb first, which we
    -- then nil out so it doesn't appear in tostr()
    if (string.startsWith(t.verb,"drawBitmap")) then
      matrix = canvas:getTotalMatrix()
      matrixType = matrix:getType()
      if matrixType.perspective then
        num_perspective_bitmaps = num_perspective_bitmaps + 1
      elseif matrixType.affine then
        num_affine_bitmaps = num_affine_bitmaps + 1
      elseif matrixType.scale then 
        num_scaled_bitmaps = num_scaled_bitmaps + 1
        if matrix:getScaleX() > 1 or matrix:getScaleY() > 1 then
          num_scaled_up = num_scaled_up + 1
        else
          num_scaled_down = num_scaled_down + 1
        end
      elseif matrixType.translate then
        num_translated_bitmaps = num_translated_bitmaps + 1
      else
        num_identity_bitmaps = num_identity_bitmaps + 1
      end
    end
end

function sk_scrape_summarize()
  io.write( "identity = ", num_identity_bitmaps,
            ", translated = ", num_translated_bitmaps, 
            ", scaled = ", num_scaled_bitmaps, " (up = ", num_scaled_up, "; down = ", num_scaled_down, ")",
            ", affine = ", num_affine_bitmaps,
            ", perspective = ", num_perspective_bitmaps,
            "\n")
end

