
function make_paint(size, color)
    local paint = Sk.newPaint();
    paint:setAntiAlias(true)
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

gSlideIndex = 1

--------------------------------------------------------------------------------------

function onDrawContent(canvas)
    drawSlide(canvas, gSlides[gSlideIndex], gTemplate, gPaints)

    return false -- we're not animating
end

function onClickHandler(x, y)
    if x < 100 and y < 100 then
        onNextSlide()
        return true
    end
    return false
end

function onNextSlide()
    gSlideIndex = gSlideIndex + 1
    if gSlideIndex > #gSlides then
        gSlideIndex = 1
    end
end

