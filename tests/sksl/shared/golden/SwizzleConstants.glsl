
out vec4 sk_FragColor;
void main() {
    vec4 v = vec4(sqrt(1.0));
    sk_FragColor = vec4(v.x, 1.0, 1.0, 1.0);
    sk_FragColor = vec4(v.xy, 1.0, 1.0);
    sk_FragColor = vec4(v.x, 1.0, 1.0, 1.0);
    sk_FragColor = vec4(0.0, v.y, 1.0, 1.0);
    sk_FragColor = vec4(v.xyz, 1.0);
    sk_FragColor = vec4(v.xy, 1.0, 1.0);
    sk_FragColor = vec4(v.x, 0.0, v.z, 1.0);
    sk_FragColor = vec4(v.x, 1.0, 0.0, 1.0);
    sk_FragColor = vec4(1.0, v.yz, 1.0);
    sk_FragColor = vec4(0.0, v.y, 1.0, 1.0);
    sk_FragColor = vec4(1.0, 1.0, v.z, 1.0);
    sk_FragColor = v;
    sk_FragColor = vec4(v.xyz, 1.0);
    sk_FragColor = vec4(v.xy, 0.0, v.w);
    sk_FragColor = vec4(v.xy, 1.0, 0.0);
    sk_FragColor = vec4(v.x, 1.0, v.zw);
    sk_FragColor = vec4(v.x, 0.0, v.z, 1.0);
    sk_FragColor = vec4(v.x, 1.0, 1.0, v.w);
    sk_FragColor = vec4(v.x, 1.0, 0.0, 1.0);
    sk_FragColor = vec4(1.0, v.yzw);
    sk_FragColor = vec4(0.0, v.yz, 1.0);
    sk_FragColor = vec4(0.0, v.y, 1.0, v.w);
    sk_FragColor = vec4(1.0, v.y, 1.0, 1.0);
    sk_FragColor = vec4(0.0, 0.0, v.zw);
    sk_FragColor = vec4(0.0, 0.0, v.z, 1.0);
    sk_FragColor = vec4(0.0, 1.0, 1.0, v.w);
}
