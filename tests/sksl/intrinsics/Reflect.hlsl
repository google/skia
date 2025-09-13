cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_I : packoffset(c0);
    float4 _11_N : packoffset(c1);
    float4 _11_colorGreen : packoffset(c2);
    float4 _11_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float spvReflect(float i, float n)
{
    return i - 2.0 * dot(n, i) * n;
}

float4 main(float2 _25)
{
    float expectedX = spvReflect(996878592.0f, -1.9999999580429535907214788975919e+34f);
    expectedX = -49.0f;
    float2 expectedXY = float2(-169.0f, 202.0f);
    float3 expectedXYZ = float3(-379.0f, 454.0f, -529.0f);
    float4 expectedXYZW = float4(-699.0f, 838.0f, -977.0f, 1116.0f);
    bool _76 = false;
    if (spvReflect(_11_I.x, _11_N.x) == (-49.0f))
    {
        float2 _66 = reflect(_11_I.xy, _11_N.xy);
        _76 = all(bool2(_66.x == float2(-169.0f, 202.0f).x, _66.y == float2(-169.0f, 202.0f).y));
    }
    else
    {
        _76 = false;
    }
    bool _89 = false;
    if (_76)
    {
        float3 _79 = reflect(_11_I.xyz, _11_N.xyz);
        _89 = all(bool3(_79.x == float3(-379.0f, 454.0f, -529.0f).x, _79.y == float3(-379.0f, 454.0f, -529.0f).y, _79.z == float3(-379.0f, 454.0f, -529.0f).z));
    }
    else
    {
        _89 = false;
    }
    bool _100 = false;
    if (_89)
    {
        float4 _92 = reflect(_11_I, _11_N);
        _100 = all(bool4(_92.x == float4(-699.0f, 838.0f, -977.0f, 1116.0f).x, _92.y == float4(-699.0f, 838.0f, -977.0f, 1116.0f).y, _92.z == float4(-699.0f, 838.0f, -977.0f, 1116.0f).z, _92.w == float4(-699.0f, 838.0f, -977.0f, 1116.0f).w));
    }
    else
    {
        _100 = false;
    }
    bool _104 = false;
    if (_100)
    {
        _104 = true;
    }
    else
    {
        _104 = false;
    }
    bool _107 = false;
    if (_104)
    {
        _107 = true;
    }
    else
    {
        _107 = false;
    }
    bool _110 = false;
    if (_107)
    {
        _110 = true;
    }
    else
    {
        _110 = false;
    }
    bool _113 = false;
    if (_110)
    {
        _113 = true;
    }
    else
    {
        _113 = false;
    }
    float4 _114 = 0.0f.xxxx;
    if (_113)
    {
        _114 = _11_colorGreen;
    }
    else
    {
        _114 = _11_colorRed;
    }
    return _114;
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
