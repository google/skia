
out vec4 sk_FragColor;
layout (binding = 1, set = 0) uniform texture2D aTexture;
layout (binding = 2, set = 0) uniform sampler2D aSampledTexture;
layout (location = 1) in vec2 c;
vec4 helpers_helper_h4ZT(sampler2D s, texture2D t) {
    return texture(s, c);
}
vec4 helper_h4TZ(texture2D t, sampler2D s) {
    return helpers_helper_h4ZT(s, t);
}
void main() {
    sk_FragColor = helper_h4TZ(aTexture, aSampledTexture);
}
