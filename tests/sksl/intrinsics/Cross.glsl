
out vec4 sk_FragColor;
uniform vec2 ah;
uniform vec2 bh;
uniform vec2 af;
uniform vec2 bf;
float cross_length_2d_ff2f2(vec2 a, vec2 b);
float cross_length_2d_hh2h2(vec2 a, vec2 b);
float cross_length_2d_ff2f2(vec2 a, vec2 b) {
    return true ? determinant(mat2(a, b)) : a.x * b.y - a.y * b.x;
}
float cross_length_2d_hh2h2(vec2 a, vec2 b) {
    return true ? determinant(mat2(a, b)) : a.x * b.y - a.y * b.x;
}
void main() {
    sk_FragColor.x = cross_length_2d_hh2h2(ah, bh);
    sk_FragColor.y = cross_length_2d_ff2f2(af, bf);
    sk_FragColor.z = cross_length_2d_hh2h2(vec2(3.0, 0.0), vec2(-1.0, 4.0));
    sk_FragColor.xyz = vec3(-8.0, -8.0, 12.0);
    sk_FragColor.yzw = vec3(9.0, -18.0, -9.0);
}
