
function make_paint(size, color)
    local paint = Sk.newPaint();
    paint:setAntiAlias(true)
    paint:setSubpixelText(true)
    paint:setTextSize(size)
    paint:setColor(color)
    return paint
end

function find_paint(paints, style)
    if not style then
        style = "child"
    end
    local paint = paints[style]
    return paint
end

function draw_node(canvas, node, x, y, paints)
    if node.text then
        local paint = find_paint(paints, node.style)
        canvas:drawText(node.text, x, y, paint)
    end
    if node.draw then
        node.draw(canvas)
    end
end

function drawSlide(canvas, slide, template, paints)
    draw_node(canvas, slide, template.title.x, template.title.y, paints)

    if slide.children then
        local x = template.child.x
        local y = template.child.y
        local dy = template.child.dy
        for i = 1, #slide.children do
            draw_node(canvas, slide.children[i], x, y, paints)
            y = y + dy
        end
    end
end

function slide_transition(prev, next, is_forward)
    local rec = {
        proc = function(self, canvas, drawSlideProc)
            if self:isDone() then
                drawSlideProc(canvas)
                return nil
            end
            canvas:drawImage(self.prevImage, self.curr_x, 0)
            canvas:drawImage(self.nextImage, self.curr_x + 640, 0)
            self.curr_x = self.curr_x + self.step_x
            return self
        end
    }
    if is_forward then
        rec.prevImage = prev
        rec.nextImage = next
        rec.curr_x = 0
        rec.step_x = -15
        rec.isDone = function (self) return self.curr_x <= -640 end
    else
        rec.prevImage = next
        rec.nextImage = prev
        rec.curr_x = -640
        rec.step_x = 15
        rec.isDone = function (self) return self.curr_x >= 0 end
    end
    return rec
end

--------------------------------------------------------------------------------------

gTemplate = {
    title = { x = 10, y = 64, textSize = 64 },
    child = { x = 40, y = 120, dy = 50, textSize = 40 },
}

gPaints = {}
gPaints.title = make_paint(gTemplate.title.textSize, { a=1, r=0, g=0, b=0 } )
gPaints.child = make_paint(gTemplate.child.textSize, { a=.75, r=0, g=0, b=0 } )

gRedPaint = Sk.newPaint()
gRedPaint:setAntiAlias(true)
gRedPaint:setColor{a=1, r=1, g=0, b=0 }

gSlides = {
    {   text = "Title1", style="title", color = { a=1, r=1, g=0, b=0 },
        children = {
            {   text = "bullet 1", style = "child" },
            {   text = "bullet 2", style = "child" },
            {   text = "bullet 3", style = "child" },
            {   draw = function (canvas)
                    canvas:drawOval({left=300, top=300, right=400, bottom=400}, gRedPaint)
            end },
        },
        transition = slide_transition
    },
    {   text = "Title2", style="title", color = { a=1, r=0, g=1, b=0 },
        children = {
            {   text = "bullet uno", style = "child" },
            {   text = "bullet 2", style = "child" },
            {   text = "bullet tres", style = "child" },
        },
        transition = fade_transition
    },
    {   text = "Title3", style="title",
        children = {
            {   text = "bullet 1", style = "child", },
            {   text = "bullet 2", style = "child", color = { r=0, g=0, b=1 } },
            {   text = "bullet 3", style = "child" },
        }
    }
}

--------------------------------------------------------------------------------------
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

gSurfaceFactory = function (w, h) return Sk.newRasterSurface(w, h) end

function spawn_transition(prevSlide, nextSlide, is_forward)
    local transition
    if is_forward then
        transition = prevSlide.transition
    else
        transition = nextSlide.transition
    end

    if not transition then
        return
    end

    local surf = gSurfaceFactory(640, 480)
    local canvas = surf:getCanvas()

    canvas:clear()
    drawSlide(canvas, prevSlide, gTemplate, gPaints)
    local prevImage = surf:newImageSnapshot()

    canvas:clear()
    drawSlide(canvas, nextSlide, gTemplate, gPaints)
    local nextImage = surf:newImageSnapshot()

    gCurrAnimation = transition(prevImage, nextImage, is_forward)
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
