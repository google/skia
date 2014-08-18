
clips = 0
draws = 0
clipPaths = 0
drawPaths = 0
swClipPaths = 0
swDrawPaths = 0

skpsTotal = 0
skpsWithPath = 0
skpsWithSWPath = 0

dofile("/tmp/lua-output")

io.write("Number of clips: ", clips, "\n");
io.write("Number of draws: ", draws, "\n");
io.write("Number of clipped paths: ", clipPaths, "\n");
io.write("Number of drawn paths: ", drawPaths, "\n");
io.write("Number of clipped software paths: ", swClipPaths, "\n");
io.write("Number of drawn software paths: ", swDrawPaths, "\n");

io.write("\n")

io.write("Number of SKPs total: ", skpsTotal, "\n")
io.write("Number of SKPs that draw paths: ", skpsWithPath, "\n")
io.write("Number of SKPs that draw SW paths: ", skpsWithSWPath, "\n")

io.write("\n")
io.write("\n")

totalSWPaths = swDrawPaths + swClipPaths
totalPaths = drawPaths + clipPaths

io.write("Percentage of paths needing software: ", (100*(totalSWPaths / totalPaths)), "\n")
io.write("Percentage of draws/clips needing software: ",
         (100*(totalSWPaths / (draws + clips))), "\n")

io.write("\n")

io.write("Percentage of SKPs that draw paths: ", (100*(skpsWithPath / skpsTotal)), "\n")
io.write("Percentage of SKPs that draw SW paths: ", (100*(skpsWithSWPath / skpsTotal)), "\n")
