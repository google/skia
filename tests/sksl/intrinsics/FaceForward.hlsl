cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_N : packoffset(c0);
    float4 _7_I : packoffset(c1);
    float4 _7_NRef : packoffset(c2);
    float4 _7_colorGreen : packoffset(c3);
    float4 _7_colorRed : packoffset(c4);
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

float4 main(float2 _21)
{
    float _25 = spvFaceForward(1.0f, 1000000015047466219876688855040.0f, 1000000015047466219876688855040.0f);
    float huge = _25;
    float2 _29 = faceforward(1.0f.xx, 1000000015047466219876688855040.0f.xx, 1000000015047466219876688855040.0f.xx);
    float2 huge2 = _29;
    float3 _35 = faceforward(1.0f.xxx, 1000000015047466219876688855040.0f.xxx, 1000000015047466219876688855040.0f.xxx);
    float3 huge3 = _35;
    float4 _40 = faceforward(1.0f.xxxx, 1000000015047466219876688855040.0f.xxxx, 1000000015047466219876688855040.0f.xxxx);
    float4 huge4 = _40;
    float4 expectedPos = _25.xxxx + _29.xxxx;
    float4 expectedNeg = _35.xxxx + _40.xxxx;
    expectedPos = float4(1.0f, 2.0f, 3.0f, 4.0f);
    expectedNeg = float4(-1.0f, -2.0f, -3.0f, -4.0f);
    bool _94 = false;
    if (spvFaceForward(_7_N.x, _7_I.x, _7_NRef.x) == (-1.0f))
    {
        float2 _80 = faceforward(_7_N.xy, _7_I.xy, _7_NRef.xy);
        _94 = all(bool2(_80.x == float4(-1.0f, -2.0f, -3.0f, -4.0f).xy.x, _80.y == float4(-1.0f, -2.0f, -3.0f, -4.0f).xy.y));
    }
    else
    {
        _94 = false;
    }
    bool _111 = false;
    if (_94)
    {
        float3 _97 = faceforward(_7_N.xyz, _7_I.xyz, _7_NRef.xyz);
        _111 = all(bool3(_97.x == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.x, _97.y == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.y, _97.z == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.z));
    }
    else
    {
        _111 = false;
    }
    bool _124 = false;
    if (_111)
    {
        float4 _114 = faceforward(_7_N, _7_I, _7_NRef);
        _124 = all(bool4(_114.x == float4(1.0f, 2.0f, 3.0f, 4.0f).x, _114.y == float4(1.0f, 2.0f, 3.0f, 4.0f).y, _114.z == float4(1.0f, 2.0f, 3.0f, 4.0f).z, _114.w == float4(1.0f, 2.0f, 3.0f, 4.0f).w));
    }
    else
    {
        _124 = false;
    }
    bool _128 = false;
    if (_124)
    {
        _128 = true;
    }
    else
    {
        _128 = false;
    }
    bool _135 = false;
    if (_128)
    {
        _135 = all(bool2(float2(-1.0f, -2.0f).x == float4(-1.0f, -2.0f, -3.0f, -4.0f).xy.x, float2(-1.0f, -2.0f).y == float4(-1.0f, -2.0f, -3.0f, -4.0f).xy.y));
    }
    else
    {
        _135 = false;
    }
    bool _142 = false;
    if (_135)
    {
        _142 = all(bool3(float3(1.0f, 2.0f, 3.0f).x == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.x, float3(1.0f, 2.0f, 3.0f).y == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.y, float3(1.0f, 2.0f, 3.0f).z == float4(1.0f, 2.0f, 3.0f, 4.0f).xyz.z));
    }
    else
    {
        _142 = false;
    }
    bool _145 = false;
    if (_142)
    {
        _145 = true;
    }
    else
    {
        _145 = false;
    }
    float4 _146 = 0.0f.xxxx;
    if (_145)
    {
        _146 = _7_colorGreen;
    }
    else
    {
        _146 = _7_colorRed;
    }
    return _146;
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
