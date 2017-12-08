function scale_text_delta(template, delta)
    template = template.slide
    for i = 1, #template do
        local paint = template[i].paint
        paint:setTextSize(paint:getTextSize() + delta)
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

function sqr(value) return value * value end

function set_blur(paint, alpha)
    local sigma = sqr(1 - alpha) * 20
    if gUseBlurInTransitions then
        paint:setImageFilter(Sk.newBlurImageFilter(sigma, sigma))
    end
    paint:setAlpha(alpha)
end

function fade_slide_transition(prev, next, is_forward)
    local rec = {
        paint = Sk.newPaint(),
        prevDrawable = prev,
        nextDrawable = next,
        proc = function(self, canvas, drawSlideProc)
            if self:isDone() then
                drawSlideProc(canvas)
                return nil
            end

            set_blur(self.paint, self.prev_a)
            self.prevDrawable:draw(canvas, self.prev_x, 0, self.paint)

            set_blur(self.paint, self.next_a)
            self.nextDrawable:draw(canvas, self.next_x, 0, self.paint)
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

function fade_transition(prev, next, is_forward)
    local rec = {
        paint = Sk.newPaint(),
        prevDrawable = prev,
        nextDrawable = next,
        proc = function(self, canvas, drawSlideProc)
            if self:isDone() then
                drawSlideProc(canvas)
                return nil
            end

            set_blur(self.paint, self.prev_a)
            self.prevDrawable:draw(canvas, 0, 0, self.paint)

            set_blur(self.paint, self.next_a)
            self.nextDrawable:draw(canvas, 0, 0, self.paint)
            self:step()
            return self
        end
    }
    rec.prev_a = 1
    rec.next_a = 0
    rec.isDone = function (self) return self.next_a >= 1 end
    rec.step = function (self)
        self.prev_a = math.max(self.prev_a - 0.025, 0)
        self.next_a = 1 - self.prev_a
    end

    return rec
end

function rotate_transition(prev, next, is_forward)
    local rec = {
        angle = 0,
        prevDrawable = prev,
        nextDrawable = next,
        activeDrawable = prev,
        proc = function(self, canvas, drawSlideProc)
            if self:isDone() then
                drawSlideProc(canvas)
                return nil
            end

            canvas:save()
            canvas:translate(320, 240)
            canvas:rotate(self.angle)
            canvas:translate(-320, -240)
            self.activeDrawable:draw(canvas, 0, 0)
            self:step()
            return self
        end,
        isDone = function (self) return self.angle >= 360 or self.angle <= -360 end
    }
    if is_forward then
        rec.step = function (self)
            self.angle = self.angle + 10
            if self.angle >= 180 then
                self.activeDrawable = self.nextDrawable
            end
        end
    else
        rec.step = function (self)
            self.angle = self.angle - 10
            if self.angle <= -180 then
                self.activeDrawable = self.nextDrawable
            end
        end
    end
    return rec
end

function zoom_transition(prev, next, is_forward)
    local rec = {
        scale = 1,
        scale_delta = .95,
        scale_limit = 0.2,
        pivot_x = 320,
        pivot_y = 240,
        prevDrawable = prev,
        nextDrawable = next,
        activeDrawable = prev,
        proc = function(self, canvas, drawSlideProc)
            if self:isDone() then
                drawSlideProc(canvas)
                return nil
            end

            canvas:translate(self.pivot_x, self.pivot_y)
            canvas:scale(self.scale, self.scale)
            canvas:translate(-self.pivot_x, -self.pivot_y)
            self.activeDrawable:draw(canvas, 0, 0)
            self:step()
            return self
        end,
        isDone = function (self) return self.scale > 1 end,
        step = function (self)
            if self.scale < self.scale_limit then
                self.scale = self.scale_limit
                self.scale_delta = 1 / self.scale_delta
                self.activeDrawable = self.nextDrawable
            end
            self.scale = self.scale * self.scale_delta
        end
    }
    return rec
end

gTransitionTable = {
    fade = fade_transition,
    slide = slide_transition,
    fade_slide = fade_slide_transition,
    rotate = rotate_transition,
    zoom = zoom_transition,
}

