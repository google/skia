cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float _40 = ((((((((((9.9999996169031624536541560020822e+35f * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f;
    float hugeH = _40;
    float _52 = ((((((((((9.9999996169031624536541560020822e+35f * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f;
    float hugeF = _52;
    int _77 = (((((((((((((((((((1073741824 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    int hugeI = _77;
    uint _101 = ((((((((((((((((((2147483648u * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    uint hugeU = _101;
    int _120 = ((((((((((((((((16384 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    int hugeS = _120;
    uint _138 = (((((((((((((((32768u * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    uint hugeUS = _138;
    int _159 = ((((((((((((((((((int(0x80000000) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    int hugeNI = _159;
    int _177 = ((((((((((((((((-32768) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    int hugeNS = _177;
    int4 _197 = ((((((((((((((int4(1073741824, 1073741824, 1073741824, 1073741824) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2);
    int4 hugeIvec = _197;
    uint4 _216 = (((((((((((((uint4(2147483648u, 2147483648u, 2147483648u, 2147483648u) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u);
    uint4 hugeUvec = _216;
    float4x4 hugeMxM = mul(float4x4(100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx), float4x4(100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx));
    float4 _226 = mul(100000002004087734272.0f.xxxx, float4x4(100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx));
    float4 hugeMxV = _226;
    float4 _228 = mul(float4x4(100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx), 100000002004087734272.0f.xxxx);
    float4 hugeVxM = _228;
    return ((((((((((((_10_colorGreen * clamp(_40, 0.0f, 1.0f)) * clamp(_52, 0.0f, 1.0f)) * clamp(float(_77), 0.0f, 1.0f)) * clamp(float(_101), 0.0f, 1.0f)) * clamp(float(_120), 0.0f, 1.0f)) * clamp(float(_138), 0.0f, 1.0f)) * clamp(float(_159), 0.0f, 1.0f)) * clamp(float(_177), 0.0f, 1.0f)) * clamp(float4(float(_197.x), float(_197.y), float(_197.z), float(_197.w)), 0.0f.xxxx, 1.0f.xxxx)) * clamp(float4(float(_216.x), float(_216.y), float(_216.z), float(_216.w)), 0.0f.xxxx, 1.0f.xxxx)) * clamp(hugeMxM[0], 0.0f.xxxx, 1.0f.xxxx)) * clamp(_226, 0.0f.xxxx, 1.0f.xxxx)) * clamp(_228, 0.0f.xxxx, 1.0f.xxxx);
}

void frag_main()
{
    float2 _20 = 0.0f.xx;
    sk_FragColor = main(_20);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
