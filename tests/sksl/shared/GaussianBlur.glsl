
out vec4 sk_FragColor;
layout (binding = 0) uniform sampler2D uTextureSampler_0_Stage1;
layout (binding = 0) uniform uniformBuffer {
    layout (offset = 0) vec4 sk_RTAdjust;
    layout (offset = 16) vec2 uIncrement_Stage1_c0;
    layout (offset = 32) vec4[7] uKernel_Stage1_c0;
    layout (offset = 144) mat3 umatrix_Stage1_c0_c0;
    layout (offset = 192) vec4 uborder_Stage1_c0_c0_c0;
    layout (offset = 208) vec4 usubset_Stage1_c0_c0_c0;
    layout (offset = 224) vec4 unorm_Stage1_c0_c0_c0;
};
layout (location = 0) in vec2 vLocalCoord_Stage0;
vec4 TextureEffect_Stage1_c0_c0_c0_h4h4f2(vec4 _input, vec2 _coords) {
    vec2 inCoord = _coords;
    inCoord *= unorm_Stage1_c0_c0_c0.xy;
    vec2 subsetCoord;
    subsetCoord.x = inCoord.x;
    subsetCoord.y = inCoord.y;
    vec2 clampedCoord;
    clampedCoord = subsetCoord;
    vec4 textureColor = texture(uTextureSampler_0_Stage1, clampedCoord * unorm_Stage1_c0_c0_c0.zw);
    float snappedX = floor(inCoord.x + 0.0010000000474974513) + 0.5;
    if (snappedX < usubset_Stage1_c0_c0_c0.x || snappedX > usubset_Stage1_c0_c0_c0.z) {
        textureColor = uborder_Stage1_c0_c0_c0;
    }
    return textureColor;
}
vec4 MatrixEffect_Stage1_c0_c0_h4h4f2(vec4 _input, vec2 _coords) {
    return TextureEffect_Stage1_c0_c0_c0_h4h4f2(_input, (umatrix_Stage1_c0_c0 * vec3(_coords, 1.0)).xy);
}
vec4 GaussianConvolution_Stage1_c0_h4h4(vec4 _input) {
    vec4 _output;
    _output = vec4(0.0, 0.0, 0.0, 0.0);
    vec2 coord = vLocalCoord_Stage0 - 12.0 * uIncrement_Stage1_c0;
    vec2 coordSampled = vec2(0.0, 0.0);
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[0].x;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[0].y;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[0].z;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[0].w;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[1].x;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[1].y;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[1].z;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[1].w;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[2].x;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[2].y;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[2].z;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[2].w;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[3].x;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[3].y;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[3].z;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[3].w;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[4].x;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[4].y;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[4].z;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[4].w;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[5].x;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[5].y;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[5].z;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[5].w;
    coord += uIncrement_Stage1_c0;
    coordSampled = coord;
    _output += MatrixEffect_Stage1_c0_c0_h4h4f2(_input, coordSampled) * uKernel_Stage1_c0[6].x;
    coord += uIncrement_Stage1_c0;
    _output *= _input;
    return _output;
}
void main() {
    vec4 outputColor_Stage0;
    vec4 outputCoverage_Stage0;
    {
        outputColor_Stage0 = vec4(1.0);
        outputCoverage_Stage0 = vec4(1.0);
    }
    vec4 output_Stage1;
    output_Stage1 = GaussianConvolution_Stage1_c0_h4h4(outputColor_Stage0);
    {
        sk_FragColor = output_Stage1 * outputCoverage_Stage0;
    }
}
