
out vec4 sk_FragColor;
uniform vec4 colorWhite;
vec4 main() {
    vec4 color = vec4(0.0);
    if (colorWhite.x == 1.0) color.y = 1.0;
    if (colorWhite.x == 2.0) ; else color.w = 1.0;
    while (colorWhite.x == 2.0) ;
    do ; while (colorWhite.x == 2.0);
    return color;
}
