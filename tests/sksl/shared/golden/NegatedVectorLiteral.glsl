
out vec4 sk_FragColor;
void main() {
    {
        sk_FragColor.x = 1.0;
        sk_FragColor.y = 1.0;
        sk_FragColor.z = 1.0;
        sk_FragColor.w = 1.0;
    }

    {
        sk_FragColor.x = float(ivec4(-1) == ivec4(ivec2(-1), ivec2(-1)) ? 1 : 0);
        sk_FragColor.y = float(ivec4(1) != ivec4(-1) ? 1 : 0);
        sk_FragColor.z = float(ivec4(-2) == ivec4(-2, ivec3(-2)) ? 1 : 0);
        sk_FragColor.w = float(ivec2(1, -2) == ivec2(1, -2) ? 1 : 0);
    }

}
