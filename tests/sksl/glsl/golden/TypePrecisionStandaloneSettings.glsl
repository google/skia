
out vec4 sk_FragColor;
float f = 1.0;
float h = 2.0;
vec2 f2 = vec2(1.0, 2.0);
vec3 h3 = vec3(1.0, 2.0, 3.0);
mat2 f22 = mat2(1.0, 2.0, 3.0, 4.0);
mat2x4 h24 = mat2x4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
void main() {
    sk_FragColor.x = ((((f + h) + f2.x) + h3.x) + f22[0].x) + h24[0].x;
}
