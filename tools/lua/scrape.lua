canvas = {}
total = 0
function accumulate(verb)
  total = total + 1
  n = canvas[verb] or 0
  n = n + 1
  canvas[verb] = n
end
function summarize()
  io.write('total='..total..' ')
  for k, v in next, canvas do
    io.write(k..'='..v..' ')
  end
end

