
out vec4 sk_FragColor;
void main() {
    {
        vec2 _5_3_color = sk_FragColor.xz;
        {
            _5_3_color.xy = _5_3_color.yx;
        }
        sk_FragColor.xz = _5_3_color;


        sk_FragColor.yw = vec2(3.0, 5.0);
    }

}
