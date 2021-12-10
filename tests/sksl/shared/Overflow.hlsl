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
    float huge = (((((((((9.0000007644071214079894667114892e+35f * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f;
    int hugeI = (((((((((((((((((((1073741824 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    uint hugeU = ((((((((((((((((((2147483648u * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    int hugeS = ((((((((((((((((16384 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    uint hugeUS = (((((((((((((((32768u * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    int hugeNI = (((((((((((((((((((-2147483648) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    int hugeNS = ((((((((((((((((-32768) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    int4 i4 = int4(2, 2, 2, 2);
    int4 hugeIvec = ((((((((((((((int4(1073741824, 1073741824, 1073741824, 1073741824) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4;
    uint4 u4 = uint4(2u, 2u, 2u, 2u);
    uint4 hugeUvec = (((((((((((((uint4(2147483648u, 2147483648u, 2147483648u, 2147483648u) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4;
    return ((((((((_10_colorGreen * clamp(huge, 0.0f, 1.0f)) * clamp(float(hugeI), 0.0f, 1.0f)) * clamp(float(hugeU), 0.0f, 1.0f)) * clamp(float(hugeS), 0.0f, 1.0f)) * clamp(float(hugeUS), 0.0f, 1.0f)) * clamp(float(hugeNI), 0.0f, 1.0f)) * clamp(float(hugeNS), 0.0f, 1.0f)) * clamp(float4(float(hugeIvec.x), float(hugeIvec.y), float(hugeIvec.z), float(hugeIvec.w)), 0.0f.xxxx, 1.0f.xxxx)) * clamp(float4(float(hugeUvec.x), float(hugeUvec.y), float(hugeUvec.z), float(hugeUvec.w)), 0.0f.xxxx, 1.0f.xxxx);
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
