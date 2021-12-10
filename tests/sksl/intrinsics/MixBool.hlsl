cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
    float4 _10_colorBlack : packoffset(c2);
    float4 _10_colorWhite : packoffset(c3);
    float4 _10_testInputs : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float4 _35 = _10_colorGreen * 100.0f;
    int4 intGreen = int4(int(_35.x), int(_35.y), int(_35.z), int(_35.w));
    float4 _49 = _10_colorRed * 100.0f;
    int4 intRed = int4(int(_49.x), int(_49.y), int(_49.z), int(_49.w));
    bool _90 = false;
    if ((false ? intRed.x : intGreen.x) == intGreen.x)
    {
        int2 _74 = int2(bool2(false, false).x ? intRed.xy.x : intGreen.xy.x, bool2(false, false).y ? intRed.xy.y : intGreen.xy.y);
        _90 = all(bool2(_74.x == intGreen.xy.x, _74.y == intGreen.xy.y));
    }
    else
    {
        _90 = false;
    }
    bool _109 = false;
    if (_90)
    {
        int3 _93 = int3(bool3(false, false, false).x ? intRed.xyz.x : intGreen.xyz.x, bool3(false, false, false).y ? intRed.xyz.y : intGreen.xyz.y, bool3(false, false, false).z ? intRed.xyz.z : intGreen.xyz.z);
        _109 = all(bool3(_93.x == intGreen.xyz.x, _93.y == intGreen.xyz.y, _93.z == intGreen.xyz.z));
    }
    else
    {
        _109 = false;
    }
    bool _122 = false;
    if (_109)
    {
        int4 _112 = int4(bool4(false, false, false, false).x ? intRed.x : intGreen.x, bool4(false, false, false, false).y ? intRed.y : intGreen.y, bool4(false, false, false, false).z ? intRed.z : intGreen.z, bool4(false, false, false, false).w ? intRed.w : intGreen.w);
        _122 = all(bool4(_112.x == intGreen.x, _112.y == intGreen.y, _112.z == intGreen.z, _112.w == intGreen.w));
    }
    else
    {
        _122 = false;
    }
    bool _138 = false;
    if (_122)
    {
        _138 = (true ? intRed.x : intGreen.x) == intRed.x;
    }
    else
    {
        _138 = false;
    }
    bool _155 = false;
    if (_138)
    {
        int2 _141 = int2(bool2(true, true).x ? intRed.xy.x : intGreen.xy.x, bool2(true, true).y ? intRed.xy.y : intGreen.xy.y);
        _155 = all(bool2(_141.x == intRed.xy.x, _141.y == intRed.xy.y));
    }
    else
    {
        _155 = false;
    }
    bool _172 = false;
    if (_155)
    {
        int3 _158 = int3(bool3(true, true, true).x ? intRed.xyz.x : intGreen.xyz.x, bool3(true, true, true).y ? intRed.xyz.y : intGreen.xyz.y, bool3(true, true, true).z ? intRed.xyz.z : intGreen.xyz.z);
        _172 = all(bool3(_158.x == intRed.xyz.x, _158.y == intRed.xyz.y, _158.z == intRed.xyz.z));
    }
    else
    {
        _172 = false;
    }
    bool _184 = false;
    if (_172)
    {
        int4 _175 = int4(bool4(true, true, true, true).x ? intRed.x : intGreen.x, bool4(true, true, true, true).y ? intRed.y : intGreen.y, bool4(true, true, true, true).z ? intRed.z : intGreen.z, bool4(true, true, true, true).w ? intRed.w : intGreen.w);
        _184 = all(bool4(_175.x == intRed.x, _175.y == intRed.y, _175.z == intRed.z, _175.w == intRed.w));
    }
    else
    {
        _184 = false;
    }
    bool _190 = false;
    if (_184)
    {
        _190 = 0 == intGreen.x;
    }
    else
    {
        _190 = false;
    }
    bool _199 = false;
    if (_190)
    {
        _199 = all(bool2(int2(0, 100).x == intGreen.xy.x, int2(0, 100).y == intGreen.xy.y));
    }
    else
    {
        _199 = false;
    }
    bool _207 = false;
    if (_199)
    {
        _207 = all(bool3(int3(0, 100, 0).x == intGreen.xyz.x, int3(0, 100, 0).y == intGreen.xyz.y, int3(0, 100, 0).z == intGreen.xyz.z));
    }
    else
    {
        _207 = false;
    }
    bool _214 = false;
    if (_207)
    {
        _214 = all(bool4(int4(0, 100, 0, 100).x == intGreen.x, int4(0, 100, 0, 100).y == intGreen.y, int4(0, 100, 0, 100).z == intGreen.z, int4(0, 100, 0, 100).w == intGreen.w));
    }
    else
    {
        _214 = false;
    }
    bool _220 = false;
    if (_214)
    {
        _220 = 100 == intRed.x;
    }
    else
    {
        _220 = false;
    }
    bool _228 = false;
    if (_220)
    {
        _228 = all(bool2(int2(100, 0).x == intRed.xy.x, int2(100, 0).y == intRed.xy.y));
    }
    else
    {
        _228 = false;
    }
    bool _236 = false;
    if (_228)
    {
        _236 = all(bool3(int3(100, 0, 0).x == intRed.xyz.x, int3(100, 0, 0).y == intRed.xyz.y, int3(100, 0, 0).z == intRed.xyz.z));
    }
    else
    {
        _236 = false;
    }
    bool _243 = false;
    if (_236)
    {
        _243 = all(bool4(int4(100, 0, 0, 100).x == intRed.x, int4(100, 0, 0, 100).y == intRed.y, int4(100, 0, 0, 100).z == intRed.z, int4(100, 0, 0, 100).w == intRed.w));
    }
    else
    {
        _243 = false;
    }
    bool _263 = false;
    if (_243)
    {
        _263 = (false ? _10_colorRed.x : _10_colorGreen.x) == _10_colorGreen.x;
    }
    else
    {
        _263 = false;
    }
    bool _284 = false;
    if (_263)
    {
        float2 _266 = float2(bool2(false, false).x ? _10_colorRed.xy.x : _10_colorGreen.xy.x, bool2(false, false).y ? _10_colorRed.xy.y : _10_colorGreen.xy.y);
        _284 = all(bool2(_266.x == _10_colorGreen.xy.x, _266.y == _10_colorGreen.xy.y));
    }
    else
    {
        _284 = false;
    }
    bool _306 = false;
    if (_284)
    {
        float3 _287 = float3(bool3(false, false, false).x ? _10_colorRed.xyz.x : _10_colorGreen.xyz.x, bool3(false, false, false).y ? _10_colorRed.xyz.y : _10_colorGreen.xyz.y, bool3(false, false, false).z ? _10_colorRed.xyz.z : _10_colorGreen.xyz.z);
        _306 = all(bool3(_287.x == _10_colorGreen.xyz.x, _287.y == _10_colorGreen.xyz.y, _287.z == _10_colorGreen.xyz.z));
    }
    else
    {
        _306 = false;
    }
    bool _322 = false;
    if (_306)
    {
        float4 _309 = float4(bool4(false, false, false, false).x ? _10_colorRed.x : _10_colorGreen.x, bool4(false, false, false, false).y ? _10_colorRed.y : _10_colorGreen.y, bool4(false, false, false, false).z ? _10_colorRed.z : _10_colorGreen.z, bool4(false, false, false, false).w ? _10_colorRed.w : _10_colorGreen.w);
        _322 = all(bool4(_309.x == _10_colorGreen.x, _309.y == _10_colorGreen.y, _309.z == _10_colorGreen.z, _309.w == _10_colorGreen.w));
    }
    else
    {
        _322 = false;
    }
    bool _342 = false;
    if (_322)
    {
        _342 = (true ? _10_colorRed.x : _10_colorGreen.x) == _10_colorRed.x;
    }
    else
    {
        _342 = false;
    }
    bool _363 = false;
    if (_342)
    {
        float2 _345 = float2(bool2(true, true).x ? _10_colorRed.xy.x : _10_colorGreen.xy.x, bool2(true, true).y ? _10_colorRed.xy.y : _10_colorGreen.xy.y);
        _363 = all(bool2(_345.x == _10_colorRed.xy.x, _345.y == _10_colorRed.xy.y));
    }
    else
    {
        _363 = false;
    }
    bool _384 = false;
    if (_363)
    {
        float3 _366 = float3(bool3(true, true, true).x ? _10_colorRed.xyz.x : _10_colorGreen.xyz.x, bool3(true, true, true).y ? _10_colorRed.xyz.y : _10_colorGreen.xyz.y, bool3(true, true, true).z ? _10_colorRed.xyz.z : _10_colorGreen.xyz.z);
        _384 = all(bool3(_366.x == _10_colorRed.xyz.x, _366.y == _10_colorRed.xyz.y, _366.z == _10_colorRed.xyz.z));
    }
    else
    {
        _384 = false;
    }
    bool _400 = false;
    if (_384)
    {
        float4 _387 = float4(bool4(true, true, true, true).x ? _10_colorRed.x : _10_colorGreen.x, bool4(true, true, true, true).y ? _10_colorRed.y : _10_colorGreen.y, bool4(true, true, true, true).z ? _10_colorRed.z : _10_colorGreen.z, bool4(true, true, true, true).w ? _10_colorRed.w : _10_colorGreen.w);
        _400 = all(bool4(_387.x == _10_colorRed.x, _387.y == _10_colorRed.y, _387.z == _10_colorRed.z, _387.w == _10_colorRed.w));
    }
    else
    {
        _400 = false;
    }
    bool _407 = false;
    if (_400)
    {
        _407 = 0.0f == _10_colorGreen.x;
    }
    else
    {
        _407 = false;
    }
    bool _417 = false;
    if (_407)
    {
        _417 = all(bool2(float2(0.0f, 1.0f).x == _10_colorGreen.xy.x, float2(0.0f, 1.0f).y == _10_colorGreen.xy.y));
    }
    else
    {
        _417 = false;
    }
    bool _426 = false;
    if (_417)
    {
        _426 = all(bool3(float3(0.0f, 1.0f, 0.0f).x == _10_colorGreen.xyz.x, float3(0.0f, 1.0f, 0.0f).y == _10_colorGreen.xyz.y, float3(0.0f, 1.0f, 0.0f).z == _10_colorGreen.xyz.z));
    }
    else
    {
        _426 = false;
    }
    bool _434 = false;
    if (_426)
    {
        _434 = all(bool4(float4(0.0f, 1.0f, 0.0f, 1.0f).x == _10_colorGreen.x, float4(0.0f, 1.0f, 0.0f, 1.0f).y == _10_colorGreen.y, float4(0.0f, 1.0f, 0.0f, 1.0f).z == _10_colorGreen.z, float4(0.0f, 1.0f, 0.0f, 1.0f).w == _10_colorGreen.w));
    }
    else
    {
        _434 = false;
    }
    bool _441 = false;
    if (_434)
    {
        _441 = 1.0f == _10_colorRed.x;
    }
    else
    {
        _441 = false;
    }
    bool _450 = false;
    if (_441)
    {
        _450 = all(bool2(float2(1.0f, 0.0f).x == _10_colorRed.xy.x, float2(1.0f, 0.0f).y == _10_colorRed.xy.y));
    }
    else
    {
        _450 = false;
    }
    bool _459 = false;
    if (_450)
    {
        _459 = all(bool3(float3(1.0f, 0.0f, 0.0f).x == _10_colorRed.xyz.x, float3(1.0f, 0.0f, 0.0f).y == _10_colorRed.xyz.y, float3(1.0f, 0.0f, 0.0f).z == _10_colorRed.xyz.z));
    }
    else
    {
        _459 = false;
    }
    bool _467 = false;
    if (_459)
    {
        _467 = all(bool4(float4(1.0f, 0.0f, 0.0f, 1.0f).x == _10_colorRed.x, float4(1.0f, 0.0f, 0.0f, 1.0f).y == _10_colorRed.y, float4(1.0f, 0.0f, 0.0f, 1.0f).z == _10_colorRed.z, float4(1.0f, 0.0f, 0.0f, 1.0f).w == _10_colorRed.w));
    }
    else
    {
        _467 = false;
    }
    float4 _468 = 0.0f.xxxx;
    if (_467)
    {
        _468 = _10_colorGreen;
    }
    else
    {
        _468 = _10_colorRed;
    }
    return _468;
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
