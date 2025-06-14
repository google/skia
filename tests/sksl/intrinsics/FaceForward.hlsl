cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_N : packoffset(c0);
    float4 _11_I : packoffset(c1);
    float4 _11_NRef : packoffset(c2);
    float4 _11_colorGreen : packoffset(c3);
    float4 _11_colorRed : packoffset(c4);
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

float4 main(float2 _25)
{
    float _29 = spvFaceForward(1.0f, 1000000015047466219876688855040.0f, 1000000015047466219876688855040.0f);
    float huge = _29;
    float2 _33 = faceforward(1.0f.xx, 1000000015047466219876688855040.0f.xx, 1000000015047466219876688855040.0f.xx);
    float2 huge2 = _33;
    float3 _39 = faceforward(1.0f.xxx, 1000000015047466219876688855040.0f.xxx, 1000000015047466219876688855040.0f.xxx);
    float3 huge3 = _39;
    float4 _44 = faceforward(1.0f.xxxx, 1000000015047466219876688855040.0f.xxxx, 1000000015047466219876688855040.0f.xxxx);
    float4 huge4 = _44;
    float4 expectedPos = _29.xxxx + _33.xxxx;
    float4 expectedNeg = _39.xxxx + _44.xxxx;
    expectedPos = float4(1.0f, 2.0f, 3.0f, 4.0f);
    expectedNeg = float4(-1.0f, -2.0f, -3.0f, -4.0f);
    bool _97 = false;
    if (spvFaceForward(_11_N.x, _11_I.x, _11_NRef.x) == (-1.0f))
    {
        float2 _83 = faceforward(_11_N.xy, _11_I.xy, _11_NRef.xy);
        _97 = all(bool2(_83.x == float4(-1.0f, -2.0f, -3.0f, -4.0f).xy.x, _83.y == float4(-1.0f, -2.0f, -3.0f, -4.0f).xy.y));
    }
    else
    {
        _97 = false;
    }
    bool _114 = false;
    if (_97)
    {
        float3 _100 = faceforward(_11_N.xyz, _11_I.xyz, _11_NRef.xyz);
        _114 = all(bool3(_100.x == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.x, _100.y == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.y, _100.z == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.z));
    }
    else
    {
        _114 = false;
    }
    bool _127 = false;
    if (_114)
    {
        float4 _117 = faceforward(_11_N, _11_I, _11_NRef);
        _127 = all(bool4(_117.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _117.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _117.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _117.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _127 = false;
    }
    bool _131 = false;
    if (_127)
    {
        _131 = true;
    }
    else
    {
        _131 = false;
    }
    bool _138 = false;
    if (_131)
    {
        _138 = all(bool2(float2(-1.0f, -2.0f).x == float4(-1.0f, -2.0f, -3.0f, -4.0f).xy.x, float2(-1.0f, -2.0f).y == float4(-1.0f, -2.0f, -3.0f, -4.0f).xy.y));
    }
    else
    {
        _138 = false;
    }
    bool _145 = false;
    if (_138)
    {
        _145 = all(bool3(float3(1.0f, 2.0f, 3.0f).x == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.x, float3(1.0f, 2.0f, 3.0f).y == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.y, float3(1.0f, 2.0f, 3.0f).z == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.z));
    }
    else
    {
        _145 = false;
    }
    bool _148 = false;
    if (_145)
    {
        _148 = true;
    }
    else
    {
        _148 = false;
    }
    float4 _149 = 0.0f.xxxx;
    if (_148)
    {
        _149 = _11_colorGreen;
    }
    else
    {
        _149 = _11_colorRed;
    }
    return _149;
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
