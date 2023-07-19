
out vec4 sk_FragColor;
layout (binding = 0) uniform sampler uSampler;
layout (binding = 1) uniform texture2D uTexture;
vec4 squaredSample_h4f2Z(vec2 p, sampler2D s) {
    return texture(s, p) * texture(s, p);
}
vec4 main() {
    return texture(makeSampler2D(uTexture, uSampler), p) * squaredSample_h4f2Z(p, makeSampler2D(uTexture, uSampler));
}
