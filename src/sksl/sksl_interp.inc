STRINGIFY(

sk_has_side_effects void print(float f);

$genType cos($genType y);
$genHType cos($genHType y);
float dot($genType x, $genType y);
float2x2 inverse(float2x2 m);
float3x3 inverse(float3x3 m);
float4x4 inverse(float4x4 m);
$genType sin($genType x);
$genHType sin($genHType x);
$genType sqrt($genType x);
$genHType sqrt($genHType x);
$genType tan($genType x);
$genHType tan($genHType x);

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

)
