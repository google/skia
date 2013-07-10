-- Experimental helpers for skia --

function string.startsWith(String,Start)
   return string.sub(String,1,string.len(Start))==Start
end

function string.endsWith(String,End)
   return End=='' or string.sub(String,-string.len(End))==End
end


Sk = {}

function Sk.isFinite(x)
    return x * 0 == 0
end

-------------------------------------------------------------------------------

Sk.Rect = { left = 0, top = 0, right = 0, bottom = 0 }
Sk.Rect.__index = Sk.Rect

function Sk.Rect.new(l, t, r, b)
    local rect
    if r then
        -- 4 arguments
        rect = { left = l, top = t, right = r, bottom = b }
    elseif l then
        -- 2 arguments
        rect = { right = l, bottom = t }
    else
        -- 0 arguments
        rect = {}
    end
    setmetatable(rect, Sk.Rect)
    return rect;
end

function Sk.Rect:width()
    return self.right - self.left
end

function Sk.Rect:height()
    return self.bottom - self.top
end

function Sk.Rect:isEmpty()
    return self:width() <= 0 or self:height() <= 0
end

function Sk.Rect:isFinite()
    local value = self.left * 0
    value = value * self.top
    value = value * self.right
    value = value * self.bottom
    return 0 == value
end

function Sk.Rect:setEmpty()
    self.left = 0
    self.top = 0
    self.right = 0
    self.bottom = 0
end

function Sk.Rect:set(l, t, r, b)
    self.left = l
    self.top = t
    self.right = r
    self.bottom = b
end

function Sk.Rect:offset(dx, dy)
    dy = dy or dx

    self.left = self.left + dx
    self.top = self.top + dy
    self.right = self.right + dx
    self.bottom = self.bottom + dy
end

function Sk.Rect:inset(dx, dy)
    dy = dy or dx

    self.left = self.left + dx
    self.top = self.top + dy
    self.right = self.right - dx
    self.bottom = self.bottom - dy
end

-------------------------------------------------------------------------------
