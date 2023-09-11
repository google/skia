cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_N : packoffset(c0);
    float4 _10_I : packoffset(c1);
    float4 _10_NRef : packoffset(c2);
    float4 _10_colorGreen : packoffset(c3);
    float4 _10_colorRed : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float spvFaceForward(float n, float i, float nref)
{
    return i * nref < 0.0 ? n : -n;
}

float4 main(float2 _24)
{
    float _28 = spvFaceForward(1.0f, 1000000015047466219876688855040.0f, 1000000015047466219876688855040.0f);
    float huge = _28;
    float2 _32 = faceforward(1.0f.xx, 1000000015047466219876688855040.0f.xx, 1000000015047466219876688855040.0f.xx);
    float2 huge2 = _32;
    float3 _38 = faceforward(1.0f.xxx, 1000000015047466219876688855040.0f.xxx, 1000000015047466219876688855040.0f.xxx);
    float3 huge3 = _38;
    float4 _43 = faceforward(1.0f.xxxx, 1000000015047466219876688855040.0f.xxxx, 1000000015047466219876688855040.0f.xxxx);
    float4 huge4 = _43;
    float4 expectedPos = _28.xxxx + _32.xxxx;
    float4 expectedNeg = _38.xxxx + _43.xxxx;
    expectedPos = float4(1.0f, 2.0f, 3.0f, 4.0f);
    expectedNeg = float4(-1.0f, -2.0f, -3.0f, -4.0f);
    bool _96 = false;
    if (spvFaceForward(_10_N.x, _10_I.x, _10_NRef.x) == (-1.0f))
    {
        float2 _82 = faceforward(_10_N.xy, _10_I.xy, _10_NRef.xy);
        _96 = all(bool2(_82.x == float4(-1.0f, -2.0f, -3.0f, -4.0f).xy.x, _82.y == float4(-1.0f, -2.0f, -3.0f, -4.0f).xy.y));
    }
    else
    {
        _96 = false;
    }
    bool _113 = false;
    if (_96)
    {
        float3 _99 = faceforward(_10_N.xyz, _10_I.xyz, _10_NRef.xyz);
        _113 = all(bool3(_99.x == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.x, _99.y == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.y, _99.z == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.z));
    }
    else
    {
        _113 = false;
    }
    bool _126 = false;
    if (_113)
    {
        float4 _116 = faceforward(_10_N, _10_I, _10_NRef);
        _126 = all(bool4(_116.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _116.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _116.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _116.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _126 = false;
    }
    bool _130 = false;
    if (_126)
    {
        _130 = true;
    }
    else
    {
        _130 = false;
    }
    bool _137 = false;
    if (_130)
    {
        _137 = all(bool2(float2(-1.0f, -2.0f).x == float4(-1.0f, -2.0f, -3.0f, -4.0f).xy.x, float2(-1.0f, -2.0f).y == float4(-1.0f, -2.0f, -3.0f, -4.0f).xy.y));
    }
    else
    {
        _137 = false;
    }
    bool _144 = false;
    if (_137)
    {
        _144 = all(bool3(float3(1.0f, 2.0f, 3.0f).x == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.x, float3(1.0f, 2.0f, 3.0f).y == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.y, float3(1.0f, 2.0f, 3.0f).z == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.z));
    }
    else
    {
        _144 = false;
    }
    bool _147 = false;
    if (_144)
    {
        _147 = true;
    }
    else
    {
        _147 = false;
    }
    float4 _148 = 0.0f.xxxx;
    if (_147)
    {
        _148 = _10_colorGreen;
    }
    else
    {
        _148 = _10_colorRed;
    }
    return _148;
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
