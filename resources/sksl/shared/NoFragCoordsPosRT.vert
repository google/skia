/*#pragma settings CannotUseFragCoord*/

layout(set=0) uniform float4 sk_RTAdjust;
in float4 pos;

void main() {
    sk_Position = pos;
}
