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
            local n = count_hypens(s)
            block[#block + 1] = {
                indent = n,
                text = trim_ws(s:sub(n + 1, -1))
            }
        end
    end
    return slides
end

function pretty_print_slide(slide)
    io.write("{\n")
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

gSlides = parse_file(io.open("/skia/trunk/resources/slides_content.lua", "r"))

function make_paint(size, color)
    local paint = Sk.newPaint();
    paint:setAntiAlias(true)
    paint:setSubpixelText(true)
    paint:setTextSize(size)
    paint:setColor(color)
    return paint
end

function drawSlide(canvas, slide, template, paints)
    local scale = 1.15
    local y = 0
    for i = 1, #slide do
        local node = slide[i]
        local temp = template[node.indent + 1]
        local paint = paints[node.indent + 1]
        local fm = paint:getFontMetrics()
        y = y - fm.ascent * scale
        canvas:drawText(node.text, temp.x, y, paint)
        y = y + fm.descent * scale
    end
end

function slide_transition(prev, next, is_forward)
    local rec = {
        proc = function(self, canvas, drawSlideProc)
            if self:isDone() then
                drawSlideProc(canvas)
                return nil
            end
            self.prevDrawable:draw(canvas, self.curr_x, 0)
            self.nextDrawable:draw(canvas, self.curr_x + 640, 0)
            self.curr_x = self.curr_x + self.step_x
            return self
        end
    }
    if is_forward then
        rec.prevDrawable = prev
        rec.nextDrawable = next
        rec.curr_x = 0
        rec.step_x = -15
        rec.isDone = function (self) return self.curr_x <= -640 end
    else
        rec.prevDrawable = next
        rec.nextDrawable = prev
        rec.curr_x = -640
        rec.step_x = 15
        rec.isDone = function (self) return self.curr_x >= 0 end
    end
    return rec
end

function fade_slide_transition(prev, next, is_forward)
    local rec = {
        prevDrawable = prev,
        nextDrawable = next,
        proc = function(self, canvas, drawSlideProc)
            if self:isDone() then
                drawSlideProc(canvas)
                return nil
            end
            self.prevDrawable:draw(canvas, self.prev_x, 0, self.prev_a)
            self.nextDrawable:draw(canvas, self.next_x, 0, self.next_a)
            self:step()
            return self
        end
    }
    if is_forward then
        rec.prev_x = 0
        rec.prev_a = 1
        rec.next_x = 640
        rec.next_a = 0
        rec.isDone = function (self) return self.next_x <= 0 end
        rec.step = function (self)
            self.next_x = self.next_x - 20
            self.next_a = (640 - self.next_x) / 640
            self.prev_a = 1 - self.next_a
        end
    else
        rec.prev_x = 0
        rec.prev_a = 1
        rec.next_x = 0
        rec.next_a = 0
        rec.isDone = function (self) return self.prev_x >= 640 end
        rec.step = function (self)
            self.prev_x = self.prev_x + 20
            self.prev_a = (640 - self.prev_x) / 640
            self.next_a = 1 - self.prev_a
        end
    end
    return rec
end

--------------------------------------------------------------------------------------

gTemplate = {
    { x = 10, textSize = 40, bullet = "" },
    { x = 40, textSize = 30, bullet = "\xE2\x80\xA2" },
    { x = 70, textSize = 20, bullet = "\xE2\x97\xA6" },
}

gPaints = {
    make_paint(gTemplate[1].textSize, { a=1, r=0, g=0, b=0 } ),
    make_paint(gTemplate[2].textSize, { a=1, r=1, g=0, b=0 } ),
    make_paint(gTemplate[3].textSize, { a=1, r=0, g=1, b=0 } ),
}

gRedPaint = Sk.newPaint()
gRedPaint:setAntiAlias(true)
gRedPaint:setColor{a=1, r=1, g=0, b=0 }

-- animation.proc is passed the canvas before drawing.
-- The animation.proc returns itself or another animation (which means keep animating)
-- or it returns nil, which stops the animation.
--
local gCurrAnimation

gSlideIndex = 1

function next_slide()
    local prev = gSlides[gSlideIndex]

    gSlideIndex = gSlideIndex + 1
    if gSlideIndex > #gSlides then
        gSlideIndex = 1
    end

    spawn_transition(prev, gSlides[gSlideIndex], true)
end

function prev_slide()
    local prev = gSlides[gSlideIndex]

    gSlideIndex = gSlideIndex - 1
    if gSlideIndex < 1 then
        gSlideIndex = #gSlides
    end

    spawn_transition(prev, gSlides[gSlideIndex], false)
end

function new_drawable_picture(pic)
    return {
        picture = pic,
        width = pic:width(),
        height = pic:height(),
        draw = function (self, canvas, x, y, paint)
            canvas:drawPicture(self.picture, x, y, paint)
        end
    }
end

function new_drawable_image(img)
    return {
        image = img,
        width = img:width(),
        height = img:height(),
        draw = function (self, canvas, x, y, paint)
            canvas:drawImage(self.image, x, y, paint)
        end
    }
end

function spawn_transition(prevSlide, nextSlide, is_forward)
    local transition
    if is_forward then
        transition = prevSlide.transition
    else
        transition = nextSlide.transition
    end

    if not transition then
        transition = fade_slide_transition
    end

    local rec = Sk.newPictureRecorder()

    drawSlide(rec:beginRecording(640, 480), prevSlide, gTemplate, gPaints)
    local prevDrawable = new_drawable_picture(rec:endRecording())

    drawSlide(rec:beginRecording(640, 480), nextSlide, gTemplate, gPaints)
    local nextDrawable = new_drawable_picture(rec:endRecording())

    gCurrAnimation = transition(prevDrawable, nextDrawable, is_forward)
end

--------------------------------------------------------------------------------------

function spawn_rotate_animation()
    gCurrAnimation = {
        angle = 0,
        angle_delta = 5,
        pivot_x = 320,
        pivot_y = 240,
        proc = function (self, canvas, drawSlideProc)
            if self.angle >= 360 then
                drawSlideProc(canvas)
                return nil
            end
            canvas:translate(self.pivot_x, self.pivot_y)
            canvas:rotate(self.angle)
            canvas:translate(-self.pivot_x, -self.pivot_y)
            drawSlideProc(canvas)

            self.angle = self.angle + self.angle_delta
            return self
        end
    }
end

function spawn_scale_animation()
    gCurrAnimation = {
        scale = 1,
        scale_delta = .95,
        scale_limit = 0.2,
        pivot_x = 320,
        pivot_y = 240,
        proc = function (self, canvas, drawSlideProc)
            if self.scale < self.scale_limit then
                self.scale = self.scale_limit
                self.scale_delta = 1 / self.scale_delta
            end
            if self.scale > 1 then
                drawSlideProc(canvas)
                return nil
            end
            canvas:translate(self.pivot_x, self.pivot_y)
            canvas:scale(self.scale, self.scale)
            canvas:translate(-self.pivot_x, -self.pivot_y)
            drawSlideProc(canvas)

            self.scale = self.scale * self.scale_delta
            return self
        end
    }
end

function onDrawContent(canvas)
    local drawSlideProc = function(canvas)
        drawSlide(canvas, gSlides[gSlideIndex], gTemplate, gPaints)
    end

    if gCurrAnimation then
        gCurrAnimation = gCurrAnimation:proc(canvas, drawSlideProc)
        return true
    else
        drawSlideProc(canvas)
        return false
    end
end

function onClickHandler(x, y)
    return false
end

local keyProcs = {
    n = next_slide,
    p = prev_slide,
    r = spawn_rotate_animation,
    s = spawn_scale_animation,
}

function onCharHandler(uni)
    local proc = keyProcs[uni]
    if proc then
        proc()
        return true
    end
    return false
end
