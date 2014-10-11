
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
    },
    {   text = "Title2", style="title", color = { a=1, r=0, g=1, b=0 },
        children = {
            {   text = "bullet uno", style = "child" },
            {   text = "bullet 2", style = "child" },
            {   text = "bullet tres", style = "child" },
        }
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

gSlideIndex = 1

function next_slide()
    gSlideIndex = gSlideIndex + 1
    if gSlideIndex > #gSlides then
        gSlideIndex = 1
    end
end

function prev_slide()
    gSlideIndex = gSlideIndex - 1
    if gSlideIndex < 1 then
        gSlideIndex = #gSlides
    end
end

--------------------------------------------------------------------------------------

-- animation.proc is passed the canvas before drawing.
-- The animation.proc returns itself or another animation (which means keep animating)
-- or it returns nil, which stops the animation.
--
local gCurrAnimation

function spawn_rotate_animation()
    gCurrAnimation = {
        angle = 0,
        angle_delta = 5,
        pivot_x = 320,
        pivot_y = 240,
        proc = function (this, canvas)
            if this.angle >= 360 then
                return nil
            end
            canvas:translate(this.pivot_x, this.pivot_y)
            canvas:rotate(this.angle)
            canvas:translate(-this.pivot_x, -this.pivot_y)

            this.angle = this.angle + this.angle_delta
            return this
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
        proc = function (this, canvas)
            if this.scale < this.scale_limit then
                this.scale = this.scale_limit
                this.scale_delta = 1 / this.scale_delta
            end
            if this.scale > 1 then
                return nil
            end
            canvas:translate(this.pivot_x, this.pivot_y)
            canvas:scale(this.scale, this.scale)
            canvas:translate(-this.pivot_x, -this.pivot_y)

            this.scale = this.scale * this.scale_delta
            return this
        end
    }
end

function onDrawContent(canvas)
    if gCurrAnimation then
        gCurrAnimation = gCurrAnimation:proc(canvas)
    end

    drawSlide(canvas, gSlides[gSlideIndex], gTemplate, gPaints)

    if gCurrAnimation then
        return true
    else
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
