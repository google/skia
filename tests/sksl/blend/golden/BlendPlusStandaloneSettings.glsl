
out vec4 sk_FragColor;
in vec4 src, dst;
void main() {
    vec4 _0_blend_plus;
    {
        _0_blend_plus = min(src + dst, 1.0);
    }

    sk_FragColor = _0_blend_plus;

}
