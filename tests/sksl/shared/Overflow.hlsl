cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float _37 = ((((((((((9.9999996169031624536541560020822e+35f * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f;
    float hugeH = _37;
    float _49 = ((((((((((9.9999996169031624536541560020822e+35f * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f;
    float hugeF = _49;
    int _74 = (((((((((((((((((((1073741824 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    int hugeI = _74;
    uint _98 = ((((((((((((((((((2147483648u * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    uint hugeU = _98;
    int _117 = ((((((((((((((((16384 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    int hugeS = _117;
    uint _135 = (((((((((((((((32768u * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    uint hugeUS = _135;
    int _156 = ((((((((((((((((((int(0x80000000) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    int hugeNI = _156;
    int _174 = ((((((((((((((((-32768) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    int hugeNS = _174;
    int4 _194 = ((((((((((((((int4(1073741824, 1073741824, 1073741824, 1073741824) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2);
    int4 hugeIvec = _194;
    uint4 _213 = (((((((((((((uint4(2147483648u, 2147483648u, 2147483648u, 2147483648u) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u);
    uint4 hugeUvec = _213;
    float4x4 hugeMxM = mul(float4x4(100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx), float4x4(100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx));
    float4 _223 = mul(100000002004087734272.0f.xxxx, float4x4(100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx));
    float4 hugeMxV = _223;
    float4 _225 = mul(float4x4(100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx, 100000002004087734272.0f.xxxx), 100000002004087734272.0f.xxxx);
    float4 hugeVxM = _225;
    return ((((((((((((_7_colorGreen * clamp(_37, 0.0f, 1.0f)) * clamp(_49, 0.0f, 1.0f)) * clamp(float(_74), 0.0f, 1.0f)) * clamp(float(_98), 0.0f, 1.0f)) * clamp(float(_117), 0.0f, 1.0f)) * clamp(float(_135), 0.0f, 1.0f)) * clamp(float(_156), 0.0f, 1.0f)) * clamp(float(_174), 0.0f, 1.0f)) * clamp(float4(float(_194.x), float(_194.y), float(_194.z), float(_194.w)), 0.0f.xxxx, 1.0f.xxxx)) * clamp(float4(float(_213.x), float(_213.y), float(_213.z), float(_213.w)), 0.0f.xxxx, 1.0f.xxxx)) * clamp(hugeMxM[0], 0.0f.xxxx, 1.0f.xxxx)) * clamp(_223, 0.0f.xxxx, 1.0f.xxxx)) * clamp(_225, 0.0f.xxxx, 1.0f.xxxx);
}

void frag_main()
{
    float2 _17 = 0.0f.xx;
    sk_FragColor = main(_17);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
