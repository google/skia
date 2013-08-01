
function sk_scrape_startcanvas(c, fileName) end

function sk_scrape_endcanvas(c, fileName) end

count3 = 0
count3sym = 0

function sk_scrape_accumulate(t)
    local p = t.paint
    if p then
        local s = p:getShader()
        if s then
            local g = s:asAGradient()
            if g then
                --io.write(g.type, " gradient with ", g.colorCount, " colors\n")
            
                if g.colorCount == 3 then
                   count3 = count3 + 1

                   if (g.midPos >= 0.499 and g.midPos <= 0.501) then
                      count3sym = count3sym + 1
                   end
                end    
            end
        end
    end
end

function sk_scrape_summarize() 
         io.write("Number of 3 color gradients:  ", count3, "\n");
         io.write("Number of 3 color symmetric gradients:  ", count3sym, "\n");
end

