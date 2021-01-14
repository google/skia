
out vec4 sk_FragColor;
float test1[4] = float[4](1.0, 2.0, 3.0, 4.0);
vec2 test2[2] = vec2[2](vec2(1.0, 2.0), vec2(3.0, 4.0));
mat4 test3[1] = mat4[1]();
void main() {
    sk_FragColor.x = (test1[0] + test2[0].x) + test3[0][0].x;
}
