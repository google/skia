
uniform vec2 u_skRTFlip;
out vec4 sk_FragColor;
layout (binding = 0) uniform sampler2D t;
vec4 main() {
    return textureGrad(t, coords, dFdx(coords), (u_skRTFlip.y * dFdy(coords)));
}
