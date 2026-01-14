
out vec4 sk_FragColor;
uniform int a;
uniform uint b;
void main() {
    int b1 = findLSB(a) + findLSB(b);
    ivec2 b2 = findLSB(ivec2(a)) + findLSB(uvec2(b));
    ivec3 b3 = findLSB(ivec3(a)) + findLSB(uvec3(b));
    ivec4 b4 = findLSB(ivec4(a)) + findLSB(uvec4(b));
    sk_FragColor = vec4(((ivec4(b1) + b2.xyxy) + ivec4(b3, 1)) + b4);
}
