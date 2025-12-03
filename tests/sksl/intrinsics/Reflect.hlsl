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
    float expectedX = spvReflect((65504.0f * 222.0f) * 2.0f, -65504.0f);
    expectedX = -49.0f;
    float2 expectedXY = float2(-169.0f, 202.0f);
    float3 expectedXYZ = float3(-379.0f, 454.0f, -529.0f);
    float4 expectedXYZW = float4(-699.0f, 838.0f, -977.0f, 1116.0f);
    bool _80 = false;
    if (spvReflect(_11_I.x, _11_N.x) == (-49.0f))
    {
        float2 _70 = reflect(_11_I.xy, _11_N.xy);
        _80 = all(bool2(_70.x == float2(-169.0f, 202.0f).x, _70.y == float2(-169.0f, 202.0f).y));
    }
    else
    {
        _80 = false;
    }
    bool _93 = false;
    if (_80)
    {
        float3 _83 = reflect(_11_I.xyz, _11_N.xyz);
        _93 = all(bool3(_83.x == float3(-379.0f, 454.0f, -529.0f).x, _83.y == float3(-379.0f, 454.0f, -529.0f).y, _83.z == float3(-379.0f, 454.0f, -529.0f).z));
    }
    else
    {
        _93 = false;
    }
    bool _104 = false;
    if (_93)
    {
        float4 _96 = reflect(_11_I, _11_N);
        _104 = all(bool4(_96.x == float4(-699.0f, 838.0f, -977.0f, 1116.0f).x, _96.y == float4(-699.0f, 838.0f, -977.0f, 1116.0f).y, _96.z == float4(-699.0f, 838.0f, -977.0f, 1116.0f).z, _96.w == float4(-699.0f, 838.0f, -977.0f, 1116.0f).w));
    }
    else
    {
        _104 = false;
    }
    bool _108 = false;
    if (_104)
    {
        _108 = true;
    }
    else
    {
        _108 = false;
    }
    bool _111 = false;
    if (_108)
    {
        _111 = true;
    }
    else
    {
        _111 = false;
    }
    bool _114 = false;
    if (_111)
    {
        _114 = true;
    }
    else
    {
        _114 = false;
    }
    bool _117 = false;
    if (_114)
    {
        _117 = true;
    }
    else
    {
        _117 = false;
    }
    float4 _118 = 0.0f.xxxx;
    if (_117)
    {
        _118 = _11_colorGreen;
    }
    else
    {
        _118 = _11_colorRed;
    }
    return _118;
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
