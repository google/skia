/*#pragma settings CannotUseFragCoord*/

layout(set=0) uniform float4 sk_RTAdjust;
layout(location=0) in float4 pos;

void main() {
    sk_Position = pos;
}
