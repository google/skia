cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_I : packoffset(c0);
    float4 _10_N : packoffset(c1);
    float4 _10_colorGreen : packoffset(c2);
    float4 _10_colorRed : packoffset(c3);
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

float4 main(float2 _24)
{
    float expectedX = -49.0f;
    float2 expectedXY = float2(-169.0f, 202.0f);
    float3 expectedXYZ = float3(-379.0f, 454.0f, -529.0f);
    float4 expectedXYZW = float4(-699.0f, 838.0f, -977.0f, 1116.0f);
    bool _74 = false;
    if (spvReflect(_10_I.x, _10_N.x) == expectedX)
    {
        float2 _63 = reflect(_10_I.xy, _10_N.xy);
        _74 = all(bool2(_63.x == expectedXY.x, _63.y == expectedXY.y));
    }
    else
    {
        _74 = false;
    }
    bool _88 = false;
    if (_74)
    {
        float3 _77 = reflect(_10_I.xyz, _10_N.xyz);
        _88 = all(bool3(_77.x == expectedXYZ.x, _77.y == expectedXYZ.y, _77.z == expectedXYZ.z));
    }
    else
    {
        _88 = false;
    }
    bool _100 = false;
    if (_88)
    {
        float4 _91 = reflect(_10_I, _10_N);
        _100 = all(bool4(_91.x == expectedXYZW.x, _91.y == expectedXYZW.y, _91.z == expectedXYZW.z, _91.w == expectedXYZW.w));
    }
    else
    {
        _100 = false;
    }
    bool _105 = false;
    if (_100)
    {
        _105 = (-49.0f) == expectedX;
    }
    else
    {
        _105 = false;
    }
    bool _111 = false;
    if (_105)
    {
        _111 = all(bool2(float2(-169.0f, 202.0f).x == expectedXY.x, float2(-169.0f, 202.0f).y == expectedXY.y));
    }
    else
    {
        _111 = false;
    }
    bool _117 = false;
    if (_111)
    {
        _117 = all(bool3(float3(-379.0f, 454.0f, -529.0f).x == expectedXYZ.x, float3(-379.0f, 454.0f, -529.0f).y == expectedXYZ.y, float3(-379.0f, 454.0f, -529.0f).z == expectedXYZ.z));
    }
    else
    {
        _117 = false;
    }
    bool _123 = false;
    if (_117)
    {
        _123 = all(bool4(float4(-699.0f, 838.0f, -977.0f, 1116.0f).x == expectedXYZW.x, float4(-699.0f, 838.0f, -977.0f, 1116.0f).y == expectedXYZW.y, float4(-699.0f, 838.0f, -977.0f, 1116.0f).z == expectedXYZW.z, float4(-699.0f, 838.0f, -977.0f, 1116.0f).w == expectedXYZW.w));
    }
    else
    {
        _123 = false;
    }
    float4 _124 = 0.0f.xxxx;
    if (_123)
    {
        _124 = _10_colorGreen;
    }
    else
    {
        _124 = _10_colorRed;
    }
    return _124;
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
