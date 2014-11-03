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
        elseif type(v) == "string" then
            str = str .. '"' .. v .. '"'
        else
            str = str .. tostring(v)
        end
    end
    return str
end

function trim_ws(s)
    return s:match("^%s*(.*)")
end

function count_hypens(s)
    local leftover = s:match("^-*(.*)")
    return string.len(s) - string.len(leftover)
end

function pretty_print_slide(slide)
    io.write("{\n")
    if slide.transition then
        io.write("   transition = \"", slide.transition, "\",\n")
    end
    for i = 1, #slide do
        local node = slide[i]
        for j = 0, node.indent do
            io.write("   ")
        end
        io.write("{ ")
        io.write(tostr(node))
        io.write(" },\n")
    end
    io.write("},\n")
end

function pretty_print_slides(slides)
    io.write("gSlides = {\n")
    for i = 1, #slides do
        pretty_print_slide(slides[i])
    end
    io.write("}\n")
end

function parse_transition_type(s)
    return s:match("^<%s*transition%s*=%s*(%a+)%s*>$")
end

function parse_blockstyle_type(s)
    return s:match("^<%s*blockstyle%s*=%s*(%a+)%s*>$")
end

function parse_file(file)
    local slides = {}
    local block = {}

    for line in file:lines() do
        local s = trim_ws(line)
        if #s == 0 then   -- done with a block
            if #block > 0 then
                slides[#slides + 1] = block
                block = {}
            end
        else
            local transition_type = parse_transition_type(s)
            local blockstyle = parse_blockstyle_type(s)
            if transition_type then
                block["transition"] = transition_type
            elseif blockstyle then
                block["blockstyle"] = blockstyle
            else
                if block.blockstyle == "code" then
                    block[#block + 1] = { text = line }
                else
                    local n = count_hypens(s)
                    block[#block + 1] = {
                        indent = n,
                        text = trim_ws(s:sub(n + 1, -1))
                    }
                end
            end
        end
    end
--    pretty_print_slides(slides)
    return slides
end

