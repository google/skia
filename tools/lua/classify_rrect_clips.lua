
function sk_scrape_startcanvas(c, fileName) end

function sk_scrape_endcanvas(c, fileName) end

function classify_rrect(rrect)
    if (rrect:type() == "simple") then
        local x, y = rrect:radii(0)
        if (x == y) then
            return "simple_circle"
        else
            return "simple_oval"
        end
    elseif (rrect:type() == "complex") then
        local numNotSquare = 0
        local rx, ry
        local same = true;
        local first_not_square_corner
        local last_not_square_corner
        for i = 1, 4 do
            local x, y = rrect:radii(i-1)
            if (x ~= 0 and y ~= 0) then
                if (numNotSquare == 0) then
                    rx = x
                    ry = y
                    first_not_square_corner = i
                else
                   last_not_square_corner = i
                   if (rx ~= x or ry ~=y) then
                       same = false
                   end
                end
                numNotSquare = numNotSquare + 1
            end
        end
        local numSquare = 4 - numNotSquare
        if (numSquare > 0 and same) then
            local corners = "corners"
            if (numSquare == 2) then
                if ((last_not_square_corner - 1 == first_not_square_corner) or
                    (1 == first_not_square_corner and 4 == last_not_square_corner )) then
                    corners = "adjacent_" .. corners
                else
                    corners = "opposite_" .. corners
                end
            elseif (1 == numSquare) then
                corners = "corner"
            end
            if (rx == ry) then
                return "circles_with_" .. numSquare .. "_square_" .. corners
            else
                return "ovals_with_" .. numSquare .. "_square_" .. corners
            end
        end
        return "complex_unclassified"
    elseif (rrect:type() == "rect") then
        return "rect"
    elseif (rrect:type() == "oval") then
        local x, y = rrect:radii(0)
        if (x == y) then
            return "circle"
        else
            return "oval"
        end
    elseif (rrect:type() == "empty") then
        return "empty"
    else
        return "unknown"
    end
end

function print_classes(class_table)
  function sort_classes(a, b)
     return a.count > b.count
  end
  array = {}
  for k, v in pairs(class_table) do
      if (type(v) == "number") then
          array[#array + 1] = {class = k, count = v};
      end
  end
  table.sort(array, sort_classes)
  local i
  for i = 1, #array do
      io.write(array[i].class, ": ", array[i].count, " (", array[i].count/class_table["total"] * 100, "%)\n");
  end
end

function sk_scrape_accumulate(t)
    if (t.verb == "clipRRect") then
        local rrect = t.rrect
        table["total"] = (table["total"] or 0) + 1
        local class = classify_rrect(rrect)
        table[class] = (table[class] or 0) + 1
    end
end

function sk_scrape_summarize()
  print_classes(table)
  --[[ To use the web scraper comment out the above call to print_classes, run the code below,
       and in the aggregator pass agg_table to print_classes.
  for k, v in pairs(table) do
      if (type(v) == "number") then
          local t = "agg_table[\"" .. k .. "\"]"
          io.write(t, " = (", t, " or 0) + ", table[k], "\n" );
      end
  end
  --]]
end
