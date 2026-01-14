
out vec4 sk_FragColor;
uniform int a;
uniform uint b;
void main() {
    int b1 = bitCount(a) + bitCount(b);
    ivec2 b2 = bitCount(ivec2(a)) + bitCount(uvec2(b));
    ivec3 b3 = bitCount(ivec3(a)) + bitCount(uvec3(b));
    ivec4 b4 = bitCount(ivec4(a)) + bitCount(uvec4(b));
    sk_FragColor = vec4(((ivec4(b1) + b2.xyxy) + ivec4(b3, 1)) + b4);
}
