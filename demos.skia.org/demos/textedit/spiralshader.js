
/*
 *  When calling makeShader, pass
 *      radius_scale    0.5
 *      center          x, y
 *      color0          r, g, b, a
 *      color1          r, g, b, a
 */
function MakeSpiralShaderEffect(CanvasKit) {
    const _spiralSkSL = `
    uniform float rad_scale;
    uniform float2 in_center;
    uniform float4 in_colors0;
    uniform float4 in_colors1;

    half4 main(float2 p) {
        float2 pp = p - in_center;
        float radius = sqrt(dot(pp, pp));
        radius = sqrt(radius);
        float angle = atan(pp.y / pp.x);
        float t = (angle + 3.1415926/2) / (3.1415926);
        t += radius * rad_scale;
        t = fract(t);
        return half4(mix(in_colors0, in_colors1, t));
    }`;

    return CanvasKit.RuntimeEffect.Make(_spiralSkSL);
}
