#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    uint3 sk_ThreadPosition;
};
struct Globals {
    texture2d<half, access::write> dest;
};
kernel void computeMain(texture2d<half, access::write> dest [[texture(0)]], uint3 sk_ThreadPosition [[thread_position_in_grid]]) {
    Globals _globals{dest};
    (void)_globals;
    Inputs _in = { sk_ThreadPosition };
    half4 pixel = half4(0.0h, 0.0h, 0.0h, 1.0h);
    float max_x = 5.0;
    float max_y = 5.0;
    float x = float(_in.sk_ThreadPosition.x * 2u - _globals.dest.get_width()) / float(_globals.dest.get_width());
    float y = float(_in.sk_ThreadPosition.y * 2u - _globals.dest.get_height()) / float(_globals.dest.get_height());
    float3 ray_origin = float3(0.0, 0.0, -1.0);
    float3 ray_target = float3(x * max_x, y * max_y, 0.0);
    float3 sphere_center = float3(0.0, 0.0, -10.0);
    float sphere_radius = 1.0;
    float3 t_minus_c = ray_target - sphere_center;
    float b = dot(ray_origin, t_minus_c);
    float c = dot(t_minus_c, t_minus_c) - sphere_radius * sphere_radius;
    float bsqmc = b * b - c;
    if (bsqmc >= 0.0) {
        pixel = half4(0.40000000596046448h, 0.40000000596046448h, 1.0h, 1.0h);
    }
    _globals.dest.write(pixel, _in.sk_ThreadPosition.xy);
    return;
}
