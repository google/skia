/*#pragma settings CannotUseFragCoord*/

uniform float4 sk_RTAdjust;
in float4 pos;

void main() {
    sk_Position = pos;
}
