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
    float _39 = (((((((((9.0000007644071214079894667114892e+35f * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f) * 1000000000.0f;
    float huge = _39;
    int _64 = (((((((((((((((((((1073741824 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    int hugeI = _64;
    uint _88 = ((((((((((((((((((2147483648u * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    uint hugeU = _88;
    int _107 = ((((((((((((((((16384 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    int hugeS = _107;
    uint _125 = (((((((((((((((32768u * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    uint hugeUS = _125;
    int _146 = ((((((((((((((((((int(0x80000000) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    int hugeNI = _146;
    int _164 = ((((((((((((((((-32768) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    int hugeNS = _164;
    int4 _184 = ((((((((((((((int4(1073741824, 1073741824, 1073741824, 1073741824) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2)) * int4(2, 2, 2, 2);
    int4 hugeIvec = _184;
    uint4 _203 = (((((((((((((uint4(2147483648u, 2147483648u, 2147483648u, 2147483648u) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u)) * uint4(2u, 2u, 2u, 2u);
    uint4 hugeUvec = _203;
    return ((((((((_10_colorGreen * clamp(_39, 0.0f, 1.0f)) * clamp(float(_64), 0.0f, 1.0f)) * clamp(float(_88), 0.0f, 1.0f)) * clamp(float(_107), 0.0f, 1.0f)) * clamp(float(_125), 0.0f, 1.0f)) * clamp(float(_146), 0.0f, 1.0f)) * clamp(float(_164), 0.0f, 1.0f)) * clamp(float4(float(_184.x), float(_184.y), float(_184.z), float(_184.w)), 0.0f.xxxx, 1.0f.xxxx)) * clamp(float4(float(_203.x), float(_203.y), float(_203.z), float(_203.w)), 0.0f.xxxx, 1.0f.xxxx);
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
