mkdir -p /tmp/animation
for a in $(seq -f "%05g" 0 30)
do
    ./out/Release/fiddle --duration 1 --frame `bc -l <<< "$a/30"` | ./tools/fiddle/parse-fiddle-output
    cp /tmp/fiddle_Raster.png /tmp/animation/image-"$a".png
done
cd /tmp/animation; ffmpeg -r 15 -pattern_type glob -i '*.png' -c:v libvpx-vp9 -lossless 1 output.webm
