cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float _43 = (((((((((((((65503.8984375f * 65503.8984375f) * 65503.8984375f) * 65503.8984375f) * 65503.8984375f) * 65503.8984375f) * 65503.8984375f) * 65503.8984375f) * 65503.8984375f) * 65503.8984375f) * 65503.8984375f) * 65503.8984375f) * 65503.8984375f) * 65503.8984375f) * 65503.8984375f;
    float hugeH = _43;
    float _57 = ((((((((((9.9999996169031624536541560020822e+35f * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f;
    float hugeF = _57;
    int _81 = (((((((((((((((((((1073741824 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    int hugeI = _81;
    uint _105 = ((((((((((((((((((2147483648u * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    uint hugeU = _105;
    int _124 = ((((((((((((((((16384 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    int hugeS = _124;
    uint _142 = (((((((((((((((32768u * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    uint hugeUS = _142;
    int _163 = ((((((((((((((((((int(0x80000000) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    int hugeNI = _163;
    int _181 = ((((((((((((((((-32768) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    int hugeNS = _181;
    int4 _201 = ((((((((((((((int4(1073741824, 1073741824, 1073741824, 1073741824) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2);
    int4 hugeIvec = _201;
    uint4 _220 = (((((((((((((uint4(2147483648u, 2147483648u, 2147483648u, 2147483648u) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u);
    uint4 hugeUvec = _220;
    float4x4 hugeMxM = mul(float4x4(100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx), float4x4(100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx));
    float4 _230 = mul(100000002004087734272.0f.xxxx, float4x4(100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx));
    float4 hugeMxV = _230;
    float4 _232 = mul(float4x4(100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx), 100000002004087734272.0f.xxxx);
    float4 hugeVxM = _232;
    return ((((((((((((_11_colorGreen * clamp(_43, 0.0f, 1.0f)) * clamp(_57, 0.0f, 1.0f)) * clamp(float(_81), 0.0f, 1.0f)) * clamp(float(_105), 0.0f, 1.0f)) * clamp(float(_124), 0.0f, 1.0f)) * clamp(float(_142), 0.0f, 1.0f)) * clamp(float(_163), 0.0f, 1.0f)) * clamp(float(_181), 0.0f, 1.0f)) * clamp(float4(float(_201.x), float(_201.y), float(_201.z), float(_201.w)), 0.0f.xxxx, 1.0f.xxxx)) * clamp(float4(float(_220.x), float(_220.y), float(_220.z), float(_220.w)), 0.0f.xxxx, 1.0f.xxxx)) * clamp(hugeMxM[0], 0.0f.xxxx, 1.0f.xxxx)) * clamp(_230, 0.0f.xxxx, 1.0f.xxxx)) * clamp(_232, 0.0f.xxxx, 1.0f.xxxx);
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    sk_FragColor = main(_21);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
