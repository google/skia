
out vec4 sk_FragColor;
void main() {
    vec4 v = vec4(sqrt(1.0));
    sk_FragColor = vec4(v.x, 1.0, 1.0, 1.0);
    sk_FragColor = vec4(v.xy, 1.0, 1.0);
    sk_FragColor = vec4(vec2(v.x, 1.0), 1.0, 1.0);
    sk_FragColor = vec4(vec2(v.y, 0.0).yx, 1.0, 1.0);
    sk_FragColor = vec4(v.xyz, 1.0);
    sk_FragColor = vec4(vec3(v.xy, 1.0), 1.0);
    sk_FragColor = vec4(vec3(v.xz, 0.0).xzy, 1.0);
    sk_FragColor = vec4(vec3(v.x, 1.0, 0.0), 1.0);
    sk_FragColor = vec4(vec3(v.yz, 1.0).zxy, 1.0);
    sk_FragColor = vec4(vec3(v.y, 0.0, 1.0).yxz, 1.0);
    sk_FragColor = vec4(vec3(v.z, 1.0, 1.0).yzx, 1.0);
    sk_FragColor = v;
    sk_FragColor = vec4(v.xyz, 1.0);
    sk_FragColor = vec4(v.xyw, 0.0).xywz;
    sk_FragColor = vec4(v.xy, 1.0, 0.0);
    sk_FragColor = vec4(v.xzw, 1.0).xwyz;
    sk_FragColor = vec4(v.xz, 0.0, 1.0).xzyw;
    sk_FragColor = vec4(v.xw, 1.0, 1.0).xzwy;
    sk_FragColor = vec4(v.x, 1.0, 0.0, 1.0);
    sk_FragColor = vec4(v.yzw, 1.0).wxyz;
    sk_FragColor = vec4(v.yz, 0.0, 1.0).zxyw;
    sk_FragColor = vec4(v.yw, 0.0, 1.0).zxwy;
    sk_FragColor = vec4(v.y, 1.0, 1.0, 1.0).yxzw;
    sk_FragColor = vec4(v.zw, 0.0, 0.0).zwxy;
    sk_FragColor = vec4(v.z, 0.0, 0.0, 1.0).yzxw;
    sk_FragColor = vec4(v.w, 0.0, 1.0, 1.0).yzwx;
}
