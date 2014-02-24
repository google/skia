function sk_scrape_startcanvas(c, fileName)
    canvas = c
    clipstack = {}
    restoreCount = 0
end

function sk_scrape_endcanvas(c, fileName)
    canvas = nil
end

function sk_scrape_accumulate(t)
    if (t.verb == "restore") then
        restoreCount = restoreCount + 1;
        io.write("Clip Stack at restore #", restoreCount, ":\n")
        for i = 1, #clipstack do
            local element = clipstack[i];
            io.write("\t", element["op"], ", ", element["type"], ", aa:", tostring(element["aa"]), "\n")
        end
        io.write("\n")
    else
        clipstack = canvas:getClipStack()
    end
end

function sk_scrape_summarize() end
