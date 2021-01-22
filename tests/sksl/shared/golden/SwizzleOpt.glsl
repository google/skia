
out vec4 sk_FragColor;
void main() {
    float v = sqrt(1.0);
    sk_FragColor = vec4(v);
    sk_FragColor = vec4(vec4(v).xyz, 0.0).wzyx;
    sk_FragColor = vec3(vec4(v).xw, 0.0).zzxy;
    sk_FragColor = vec3(vec3(vec4(v).xw, 0.0).yx, 1.0).zzxy;
    sk_FragColor = vec3(vec4(v).zy, 1.0).xyzz;
    sk_FragColor = vec4(v);
    sk_FragColor = vec4(vec4(v).xx, 1.0, 1.0);
    sk_FragColor = vec4(v).wzwz;
    sk_FragColor.xyzw = sk_FragColor;
    sk_FragColor.wzyx = sk_FragColor;
    sk_FragColor.xyzw.xw = sk_FragColor.yz;
    sk_FragColor.wzyx.yzw = vec3(sk_FragColor.ww, 1.0);
}
