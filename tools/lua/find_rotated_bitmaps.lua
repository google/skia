function string.startsWith(String,Start)
   return string.sub(String,1,string.len(Start))==Start
end

function string.endsWith(String,End)
   return End=='' or string.sub(String,-string.len(End))==End
end

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

local canvas = nil
local num_perspective_bitmaps = 0
local num_affine_bitmaps = 0
local num_scaled_bitmaps = 0
local num_translated_bitmaps = 0
local num_identity_bitmaps = 0

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
            ", scaled = ", num_scaled_bitmaps,
            ", affine = ", num_affine_bitmaps,
            ", perspective = ", num_perspective_bitmaps,
            "\n")
end

