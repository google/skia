
local r = { left = 10, top = 10, right = 100, bottom = 80 }
local x = 0;

local paint = Sk.newPaint();
paint:setAntiAlias(true);

local image     -- = Sk.loadImage('/skia/sailboat.jpg');
function setImageFilename(filename)
    image = Sk.loadImage(filename)
end


local color = {a = 1, r = 1, g = 0, b = 0};

function rnd(range) 
   return math.random() * range;
end 

rndX = function () return rnd(640) end 
rndY = function () return rnd(480) end 

function draw_rand_path(canvas);
   if not path_paint then 
       path_paint = Sk.newPaint();
       path_paint:setAntiAlias(true);
   end 
   path_paint:setColor({a = 1, r = math.random(), g = math.random(), b = math.random() });

   local path = Sk.newPath();
   path:moveTo(rndX(), rndY());
   for i = 0, 50 do 
       path:quadTo(rndX(), rndY(), rndX(), rndY());
   end 
   canvas:drawPath(path, path_paint);

   paint:setColor{a=1,r=0,g=0,b=1};
   local align = { 'left', 'center', 'right' };
   paint:setTextSize(30);
   for k, v in next, align do 
       paint:setTextAlign(v);
       canvas:drawText('Hamburgefons', 320, 200 + 30*k, paint);
   end 
end 

function onStartup()
    local paint = Sk.newPaint();
    paint:setColor{a=1, r=1, g=0, b=0};
    if false then
        local doc = Sk.newDocumentPDF('/skia/trunk/test.pdf');
        local canvas = doc:beginPage(72*8.5, 72*11);
        canvas:drawText('Hello Lua', 300, 300, paint);
        doc:close();
        doc = nil;
    end
end 

function onDrawContent(canvas)
    draw_rand_path(canvas);
    color.g = x / 100;
    paint:setColor(color) 
    canvas:translate(x, 0);
    canvas:drawOval(r, paint) 
    x = x + 1;
    local r2 = {}
    r2.left = x;
    r2.top = r.bottom + 50;
    r2.right = r2.left + image:width() * 1;
    r2.bottom = r2.top + image:height() * 1;
    canvas:drawImageRect(image, nil, r2, 0.75);
    if x > 200 then x = 0 end;

    return true -- so we can animate
end

onStartup()
