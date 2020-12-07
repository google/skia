
out vec4 sk_FragColor;
float test1[] = float[](1.0, 2.0, 3.0, 4.0);
vec2 test2[] = vec2[](vec2(1.0, 2.0), vec2(3.0, 4.0));
mat4 test3[] = mat4[]();
void main() {
    sk_FragColor.x = (test1[0] + test2[0].x) + test3[0][0].x;
}
