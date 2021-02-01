/*#pragma settings NoInline*/

// Verify that all the basic ES2 types are supported as FP helper function return types.
float    returns_float()             { return float(1); }
float2   returns_float2()            { return float2(2); }
float3   returns_float3()            { return float3(3); }
float4   returns_float4()            { return float4(4); }
float2x2 returns_float2x2()          { return float2x2(2); }
float3x3 returns_float3x3()          { return float3x3(3); }
float4x4 returns_float4x4()          { return float4x4(4); }
half     returns_half()              { return half(1); }
half2    returns_half2()             { return half2(2); }
half3    returns_half3()             { return half3(3); }
half4    returns_half4()             { return half4(4); }
half2x2  returns_half2x2()           { return half2x2(2); }
half3x3  returns_half3x3()           { return half3x3(3); }
half4x4  returns_half4x4()           { return half4x4(4); }
bool     returns_bool()              { return bool(true); }
bool2    returns_bool2()             { return bool2(true); }
bool3    returns_bool3()             { return bool3(true); }
bool4    returns_bool4()             { return bool4(true); }
int      returns_int()               { return int(1); }
int2     returns_int2()              { return int2(2); }
int3     returns_int3()              { return int3(3); }
int4     returns_int4()              { return int4(4); }

half4 main() {
    returns_float();
    returns_float2();
    returns_float3();
    returns_float4();
    returns_float2x2();
    returns_float3x3();
    returns_float4x4();
    returns_half();
    returns_half2();
    returns_half3();
    returns_half4();
    returns_half2x2();
    returns_half3x3();
    returns_half4x4();
    returns_bool();
    returns_bool2();
    returns_bool3();
    returns_bool4();
    returns_int();
    returns_int2();
    returns_int3();
    returns_int4();
    return half4(1);
}
