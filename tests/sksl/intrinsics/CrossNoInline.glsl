
out vec4 sk_FragColor;
uniform vec2 ah;
uniform vec2 bh;
uniform vec2 af;
uniform vec2 bf;
float cross_length_2d_hh2h2(vec2 a, vec2 b) {
    return a.x * b.y - a.y * b.x;
}
float cross_length_2d_ff2f2(vec2 a, vec2 b) {
    return a.x * b.y - a.y * b.x;
}
void main() {
    sk_FragColor.x = cross_length_2d_hh2h2(ah, bh);
    sk_FragColor.y = cross_length_2d_ff2f2(af, bf);
}
