
const float sk_PrivkGuardedDivideEpsilon = false ? 1e-08 : 0.0;
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
float color_burn_component_Qhh2h2(vec2 s, vec2 d);
float color_burn_component_Qhh2h2(vec2 s, vec2 d) {
    if (d.y == d.x) {
        return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    } else if (s.x == 0.0) {
        return d.x * (1.0 - s.y);
    } else {
        float delta = max(0.0, d.y - ((d.y - d.x) * s.y) / (s.x + sk_PrivkGuardedDivideEpsilon));
        return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    }
}
void main() {
    sk_FragColor = vec4(color_burn_component_Qhh2h2(src.xw, dst.xw), color_burn_component_Qhh2h2(src.yw, dst.yw), color_burn_component_Qhh2h2(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
}
