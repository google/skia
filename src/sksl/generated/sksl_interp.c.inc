R"SKSL(
$genType atan($genType y_over_x);
$genHType atan($genHType y_over_x);
$genType cos($genType y);
$genHType cos($genHType y);
float dot($genType x, $genType y);
half dot($genHType x, $genHType y);
$genType fract($genType x);
$genHType fract($genHType x);
float2x2 inverse(float2x2 m);
float3x3 inverse(float3x3 m);
float4x4 inverse(float4x4 m);
half2x2 inverse(half2x2 m);
half3x3 inverse(half3x3 m);
half4x4 inverse(half4x4 m);
float length($genType x);
half length($genHType x);
$genType normalize($genType x);
$genHType normalize($genHType x);
$genType pow($genType x, $genType y);
$genHType pow($genHType x, $genHType y);
$genType sin($genType x);
$genHType sin($genHType x);
$genType sqrt($genType x);
$genHType sqrt($genHType x);
$genType tan($genType x);
$genHType tan($genHType x);

$genType min($genType x, $genType y);
$genType min($genType x, float y);
$genHType min($genHType x, $genHType y);
$genHType min($genHType x, half y);
$genIType min($genIType x, $genIType y);
$genIType min($genIType x, int y);
$genType max($genType x, $genType y);
$genType max($genType x, float y);
$genHType max($genHType x, $genHType y);
$genHType max($genHType x, half y);
$genIType max($genIType x, $genIType y);
$genIType max($genIType x, int y);
$genType clamp($genType x, $genType minVal, $genType maxVal);
$genType clamp($genType x, float minVal, float maxVal);
$genHType clamp($genHType x, $genHType minVal, $genHType maxVal);
$genHType clamp($genHType x, half minVal, half maxVal);
$genIType clamp($genIType x, $genIType minVal, $genIType maxVal);
$genIType clamp($genIType x, int minVal, int maxVal);
$genType saturate($genType x);
$genHType saturate($genHType x);

$genType  mix($genType  x, $genType  y, $genBType a);
$genHType mix($genHType x, $genHType y, $genBType a);
$genIType mix($genIType x, $genIType y, $genBType a);
$genBType mix($genBType x, $genBType y, $genBType a);

$genType  mix($genType  x, $genType  y, $genType a);
$genType  mix($genType  x, $genType  y, float a);
$genHType mix($genHType x, $genHType y, $genHType a);
$genHType mix($genHType x, $genHType y, half a);

$bvec lessThan($vec  x, $vec  y);
$bvec lessThan($hvec x, $hvec y);
$bvec lessThan($ivec x, $ivec y);
$bvec lessThan($uvec x, $uvec y);
$bvec lessThanEqual($vec  x, $vec  y);
$bvec lessThanEqual($hvec x, $hvec y);
$bvec lessThanEqual($ivec x, $ivec y);
$bvec lessThanEqual($uvec x, $uvec y);
$bvec greaterThan($vec  x, $vec  y);
$bvec greaterThan($hvec x, $hvec y);
$bvec greaterThan($ivec x, $ivec y);
$bvec greaterThan($uvec x, $uvec y);
$bvec greaterThanEqual($vec  x, $vec  y);
$bvec greaterThanEqual($hvec x, $hvec y);
$bvec greaterThanEqual($ivec x, $ivec y);
$bvec greaterThanEqual($uvec x, $uvec y);
$bvec equal($vec  x, $vec  y);
$bvec equal($hvec x, $hvec y);
$bvec equal($ivec x, $ivec y);
$bvec equal($uvec x, $uvec y);
$bvec equal($bvec x, $bvec y);
$bvec notEqual($vec  x, $vec  y);
$bvec notEqual($hvec x, $hvec y);
$bvec notEqual($ivec x, $ivec y);
$bvec notEqual($uvec x, $uvec y);
$bvec notEqual($bvec x, $bvec y);

bool  any($bvec x);
bool  all($bvec x);
$bvec not($bvec x);

float  degrees(float  rad) { return rad * 57.2957795; }
float2 degrees(float2 rad) { return rad * 57.2957795; }
float3 degrees(float3 rad) { return rad * 57.2957795; }
float4 degrees(float4 rad) { return rad * 57.2957795; }

float  radians(float  deg) { return deg * 0.0174532925; }
float2 radians(float2 deg) { return deg * 0.0174532925; }
float3 radians(float3 deg) { return deg * 0.0174532925; }
float4 radians(float4 deg) { return deg * 0.0174532925; }

float distance(float2 a, float2 b) { return length(a - b); }
float distance(float3 a, float3 b) { return length(a - b); }
float distance(float4 a, float4 b) { return length(a - b); }

float3 cross(float3 a, float3 b) {
    return float3(a.y * b.z - a.z * b.y,
                  a.z * b.x - a.x * b.z,
                  a.x * b.y - a.y * b.x);
}
)SKSL";