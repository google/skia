gShowBounds = false
gUseBlurInTransitions = false

gPath = "resources/lua/"

function load_file(file)
    local prev_path = package.path
    package.path = package.path .. ";" .. gPath .. file .. ".lua"
    require(file)
    package.path = prev_path
end

load_file("slides_utils")

gSlides = parse_file(io.open("resources/lua/slides_content2.lua", "r"))

function make_rect(l, t, r, b)
    return { left = l, top = t, right = r, bottom = b }
end

function make_paint(typefacename, style, size, color)
    local paint = Sk.newPaint();
    paint:setAntiAlias(true)
    paint:setSubpixelText(true)
    paint:setTypeface(Sk.newTypeface(typefacename, style))
    paint:setTextSize(size)
    paint:setColor(color)
    return paint
end

function draw_bullet(canvas, x, y, paint, indent)
    if 0 == indent then
        return
    end
    local ps = paint:getTextSize()
    local cx = x - ps * .8
    local cy = y - ps * .4
    local radius = ps * .2
    canvas:drawCircle(cx, cy, radius, paint)
end

function stroke_rect(canvas, rect, color)
    local paint = Sk.newPaint()
    paint:setStroke(true);
    paint:setColor(color)
    canvas:drawRect(rect, paint)
end

function drawSlide(canvas, slide, master_template)

    if #slide == 1 then
        template = master_template.title
        canvas:drawText(slide[1].text, 320, 240, template[1])
        return
    end

    template = master_template.slide

    local x = template.margin_x
    local y = template.margin_y
    local scale = 1.25

    if slide.blockstyle == "code" then
        local paint = master_template.codePaint
        local fm = paint:getFontMetrics()
        local height = #slide * (fm.descent - fm.ascent)
        y = (480 - height) / 2
        for i = 1, #slide do
            local node = slide[i]
            y = y - fm.ascent * scale
            canvas:drawText(node.text, x, y, paint)
            y = y + fm.descent * scale
        end
        return
    end

    for i = 1, #slide do
        local node = slide[i]
        local paint = template[node.indent + 1].paint
        local extra_dy = template[node.indent + 1].extra_dy
        local fm = paint:getFontMetrics()
        local x_offset = -fm.ascent * node.indent * 1.25

        local bounds = make_rect(x + x_offset, y, 620, 640)
        local blob, newBottom = Sk.newTextBlob(node.text, bounds, paint)
        draw_bullet(canvas, x + x_offset, y - fm.ascent, paint, node.indent)
        canvas:drawTextBlob(blob, 0, 0, paint)
        y = newBottom + paint:getTextSize() * .5 + extra_dy

        if gShowBounds then
            bounds.bottom = newBottom
            stroke_rect(canvas, bounds, {a=1,r=0,g=1,b=0})
            stroke_rect(canvas, blob:bounds(), {a=1,r=1,g=0,b=0})
        end

    end
end

--------------------------------------------------------------------------------------
function make_tmpl(paint, extra_dy)
    return { paint = paint, extra_dy = extra_dy }
end

function SkiaPoint_make_template()
    normal = Sk.newFontStyle()
    bold = Sk.newFontStyle(700)
    local title = {
        margin_x = 30,
        margin_y = 100,
    }
    title[1] = make_paint("Arial", bold, 45, { a=1, r=1, g=1, b=1 })
    title[1]:setTextAlign("center")
    title[2] = make_paint("Arial", bold, 25, { a=1, r=.75, g=.75, b=.75 })
    title[2]:setTextAlign("center")

    local slide = {
        margin_x = 20,
        margin_y = 25,
    }
    slide[1] = make_tmpl(make_paint("Arial", bold, 35, { a=1, r=1, g=1, b=1 }), 18)
    slide[2] = make_tmpl(make_paint("Arial", normal, 25, { a=1, r=1, g=1, b=1 }), 10)
    slide[3] = make_tmpl(make_paint("Arial", normal, 20, { a=1, r=.9, g=.9, b=.9 }), 5)

    return {
        title = title,
        slide = slide,
        codePaint = make_paint("Courier", normal, 20, { a=1, r=.9, g=.9, b=.9 }),
    }
end

gTemplate = SkiaPoint_make_template()

gRedPaint = Sk.newPaint()
gRedPaint:setAntiAlias(true)
gRedPaint:setColor{a=1, r=1, g=0, b=0 }

-- animation.proc is passed the canvas before drawing.
-- The animation.proc returns itself or another animation (which means keep animating)
-- or it returns nil, which stops the animation.
--
local gCurrAnimation

gSlideIndex = 1

-----------------------------------------------------------------------------

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

function convert_to_picture_drawable(slide)
    local rec = Sk.newPictureRecorder()
    drawSlide(rec:beginRecording(640, 480), slide, gTemplate)
    return new_drawable_picture(rec:endRecording())
end

function convert_to_image_drawable(slide)
    local surf = Sk.newRasterSurface(640, 480)
    drawSlide(surf:getCanvas(), slide, gTemplate)
    return new_drawable_image(surf:newImageSnapshot())
end

function new_drawable_slide(slide)
    return {
        slide = slide,
        draw = function (self, canvas, x, y, paint)
            if (nil == paint or ("number" == type(paint) and (1 == paint))) then
                canvas:save()
            else
                canvas:saveLayer(paint)
            end
            canvas:translate(x, y)
            drawSlide(canvas, self.slide, gTemplate)
            canvas:restore()
        end
    }
end

gNewDrawableFactory = {
    default = new_drawable_slide,
    picture = convert_to_picture_drawable,
    image = convert_to_image_drawable,
}

-----------------------------------------------------------------------------

function next_slide()
    local prev = gSlides[gSlideIndex]

    if gSlideIndex < #gSlides then
        gSlideIndex = gSlideIndex + 1
        spawn_transition(prev, gSlides[gSlideIndex], true)
    end
end

function prev_slide()
    local prev = gSlides[gSlideIndex]

    if gSlideIndex > 1 then
        gSlideIndex = gSlideIndex - 1
        spawn_transition(prev, gSlides[gSlideIndex], false)
    end
end

gDrawableType = "default"

load_file("slides_transitions")

function spawn_transition(prevSlide, nextSlide, is_forward)
    local transition
    if is_forward then
        transition = gTransitionTable[nextSlide.transition]
    else
        transition = gTransitionTable[prevSlide.transition]
    end

    if not transition then
        transition = fade_slide_transition
    end

    local prevDrawable = gNewDrawableFactory[gDrawableType](prevSlide)
    local nextDrawable = gNewDrawableFactory[gDrawableType](nextSlide)
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

local bgPaint = nil

function draw_bg(canvas)
    if not bgPaint then
        bgPaint = Sk.newPaint()
        local grad = Sk.newLinearGradient(  0,   0, { a=1, r=0, g=0, b=.3 },
                                          640, 480, { a=1, r=0, g=0, b=.8 })
        bgPaint:setShader(grad)
        bgPaint:setDither(true)
    end

    canvas:drawPaint(bgPaint)
end

function onDrawContent(canvas, width, height)
    local matrix = Sk.newMatrix()
    matrix:setRectToRect(make_rect(0, 0, 640, 480), make_rect(0, 0, width, height), "center")
    canvas:concat(matrix)

    draw_bg(canvas)

    local drawSlideProc = function(canvas)
        drawSlide(canvas, gSlides[gSlideIndex], gTemplate)
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
    ["="] = function () scale_text_delta(gTemplate, 1) end,
    ["-"] = function () scale_text_delta(gTemplate, -1) end,

    b = function () gShowBounds = not gShowBounds end,
    B = function () gUseBlurInTransitions = not gUseBlurInTransitions end,

    ["1"] = function () gDrawableType = "default" end,
    ["2"] = function () gDrawableType = "picture" end,
    ["3"] = function () gDrawableType = "image" end,
}

function onCharHandler(uni)
    local proc = keyProcs[uni]
    if proc then
        proc()
        return true
    end
    return false
end
