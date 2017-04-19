#$ ./out/Release/fiddle --duration 1.5 --frame 0.1 |
#./tools/fiddle/parse-fiddle-output 
#/tmp/fiddle_Raster.png

mkdir -p /tmp/animation
for a in $(seq -f "%05g" 0 15)
do
    ./out/Release/fiddle --duration 1 --frame `bc -l <<< "$a/15"` | ./tools/fiddle/parse-fiddle-output
    cp /tmp/fiddle_Raster.png /tmp/animation/image-"$a".png
done

