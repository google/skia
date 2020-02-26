STRINGIFY(

float dot(float2 a, float2 b) { return a.x*b.x + a.y*b.y; }
float dot(float3 a, float3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
float dot(float4 a, float4 b) { return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }

float  degrees(float  rad) { return rad * 57.2957795; }
float2 degrees(float2 rad) { return rad * 57.2957795; }
float3 degrees(float3 rad) { return rad * 57.2957795; }
float4 degrees(float4 rad) { return rad * 57.2957795; }

float  radians(float  deg) { return deg * 0.0174532925; }
float2 radians(float2 deg) { return deg * 0.0174532925; }
float3 radians(float3 deg) { return deg * 0.0174532925; }
float4 radians(float4 deg) { return deg * 0.0174532925; }

float length(float2 v) { return sqrt(dot(v, v)); }
float length(float3 v) { return sqrt(dot(v, v)); }
float length(float4 v) { return sqrt(dot(v, v)); }

float distance(float2 a, float2 b) { return length(a - b); }
float distance(float3 a, float3 b) { return length(a - b); }
float distance(float4 a, float4 b) { return length(a - b); }

float2 normalize(float2 v) { return v / length(v); }
float3 normalize(float3 v) { return v / length(v); }
float4 normalize(float4 v) { return v / length(v); }

float  mix(float  x, float  y, float t) { return x * (1 - t) + y * t; }
float2 mix(float2 x, float2 y, float t) { return x * (1 - t) + y * t; }
float3 mix(float3 x, float3 y, float t) { return x * (1 - t) + y * t; }
float4 mix(float4 x, float4 y, float t) { return x * (1 - t) + y * t; }

float2 mix(float2 x, float2 y, float2 t) { return x * (1 - t) + y * t; }
float3 mix(float3 x, float3 y, float3 t) { return x * (1 - t) + y * t; }
float4 mix(float4 x, float4 y, float4 t) { return x * (1 - t) + y * t; }

float3 cross(float3 a, float3 b) {
    return float3(a.y * b.z - a.z * b.y,
                  a.z * b.x - a.x * b.z,
                  a.x * b.y - a.y * b.x);
}

float  min(float  x, float  y) { return mix(y, x, x < y); }
float2 min(float2 x, float2 y) { return mix(y, x, lessThan(x, y)); }
float3 min(float3 x, float3 y) { return mix(y, x, lessThan(x, y)); }
float4 min(float4 x, float4 y) { return mix(y, x, lessThan(x, y)); }

float2 min(float2 x, float  y) { return mix(float2(y), x, lessThan(x, float2(y))); }
float3 min(float3 x, float  y) { return mix(float3(y), x, lessThan(x, float3(y))); }
float4 min(float4 x, float  y) { return mix(float4(y), x, lessThan(x, float4(y))); }

float  max(float  x, float  y) { return mix(y, x, x > y); }
float2 max(float2 x, float2 y) { return mix(y, x, greaterThan(x, y)); }
float3 max(float3 x, float3 y) { return mix(y, x, greaterThan(x, y)); }
float4 max(float4 x, float4 y) { return mix(y, x, greaterThan(x, y)); }

float2 max(float2 x, float  y) { return mix(float2(y), x, greaterThan(x, float2(y))); }
float3 max(float3 x, float  y) { return mix(float3(y), x, greaterThan(x, float3(y))); }
float4 max(float4 x, float  y) { return mix(float4(y), x, greaterThan(x, float4(y))); }

float  clamp(float  x, float  minVal, float  maxVal) { return min(max(x, minVal), maxVal); }
float2 clamp(float2 x, float2 minVal, float2 maxVal) { return min(max(x, minVal), maxVal); }
float3 clamp(float3 x, float3 minVal, float3 maxVal) { return min(max(x, minVal), maxVal); }
float4 clamp(float4 x, float4 minVal, float4 maxVal) { return min(max(x, minVal), maxVal); }

float2 clamp(float2 x, float  minVal, float  maxVal) { return min(max(x, minVal), maxVal); }
float3 clamp(float3 x, float  minVal, float  maxVal) { return min(max(x, minVal), maxVal); }
float4 clamp(float4 x, float  minVal, float  maxVal) { return min(max(x, minVal), maxVal); }

float  saturate(float  x) { return min(max(x, 0), 1); }
float2 saturate(float2 x) { return min(max(x, 0), 1); }
float3 saturate(float3 x) { return min(max(x, 0), 1); }
float4 saturate(float4 x) { return min(max(x, 0), 1); }

float  step(float  edge, float  x) { return mix(1, 0, x < edge); }
float2 step(float2 edge, float2 x) { return mix(float2(1), float2(0), lessThan(x, edge)); }
float3 step(float3 edge, float3 x) { return mix(float3(1), float3(0), lessThan(x, edge)); }
float4 step(float4 edge, float4 x) { return mix(float4(1), float4(0), lessThan(x, edge)); }

float2 step(float  edge, float2 x) { return mix(float2(1), float2(0), lessThan(x, float2(edge))); }
float3 step(float  edge, float3 x) { return mix(float3(1), float3(0), lessThan(x, float3(edge))); }
float4 step(float  edge, float4 x) { return mix(float4(1), float4(0), lessThan(x, float4(edge))); }

float  smoothstep(float  edge0, float  edge1, float  x) {
    float  t = saturate((x - edge0) / (edge1 - edge0));
    return t * t * (3.0 - 2.0 * t);
}

float2 smoothstep(float2 edge0, float2 edge1, float2 x) {
    float2 t = saturate((x - edge0) / (edge1 - edge0));
    return t * t * (3.0 - 2.0 * t);
}

float3 smoothstep(float3 edge0, float3 edge1, float3 x) {
    float3 t = saturate((x - edge0) / (edge1 - edge0));
    return t * t * (3.0 - 2.0 * t);
}

float4 smoothstep(float4 edge0, float4 edge1, float4 x) {
    float4 t = saturate((x - edge0) / (edge1 - edge0));
    return t * t * (3.0 - 2.0 * t);
}

float2 smoothstep(float  edge0, float  edge1, float2 x) {
    float2 t = saturate((x - edge0) / (edge1 - edge0));
    return t * t * (3.0 - 2.0 * t);
}

float3 smoothstep(float  edge0, float  edge1, float3 x) {
    float3 t = saturate((x - edge0) / (edge1 - edge0));
    return t * t * (3.0 - 2.0 * t);
}

float4 smoothstep(float  edge0, float  edge1, float4 x) {
    float4 t = saturate((x - edge0) / (edge1 - edge0));
    return t * t * (3.0 - 2.0 * t);
}

)
