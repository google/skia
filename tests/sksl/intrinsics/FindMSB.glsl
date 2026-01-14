
out vec4 sk_FragColor;
uniform int a;
uniform uint b;
void main() {
    int b1 = findMSB(a) + findMSB(b);
    ivec2 b2 = findMSB(ivec2(a)) + findMSB(uvec2(b));
    ivec3 b3 = findMSB(ivec3(a)) + findMSB(uvec3(b));
    ivec4 b4 = findMSB(ivec4(a)) + findMSB(uvec4(b));
    sk_FragColor = vec4(((ivec4(b1) + b2.xyxy) + ivec4(b3, 1)) + b4);
}
