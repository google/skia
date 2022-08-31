
out vec4 sk_FragColor;
uniform vec2 ah;
uniform vec2 bh;
uniform vec2 af;
uniform vec2 bf;
void main() {
    sk_FragColor.x = determinant(mat2(ah, bh));
    sk_FragColor.y = determinant(mat2(af, bf));
    sk_FragColor.z = 12.0;
    sk_FragColor.xyz = vec3(-8.0, -8.0, 12.0);
    sk_FragColor.yzw = vec3(9.0, -18.0, -9.0);
}
