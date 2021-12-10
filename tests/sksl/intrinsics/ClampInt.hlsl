cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_testInputs : packoffset(c0);
    float4 _10_colorGreen : packoffset(c1);
    float4 _10_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float4 _35 = _10_testInputs * 100.0f;
    int4 _44 = int4(int(_35.x), int(_35.y), int(_35.z), int(_35.w));
    int4 intValues = _44;
    int4 expectedA = int4(-100, 0, 75, 100);
    int4 expectedB = int4(-100, 0, 50, 225);
    int _56 = _44.x;
    bool _69 = false;
    if (clamp(_56, -100, 100) == (-100))
    {
        int2 _60 = clamp(_44.xy, int2(-100, -100), int2(100, 100));
        _69 = all(bool2(_60.x == int4(-100, 0, 75, 100).xy.x, _60.y == int4(-100, 0, 75, 100).xy.y));
    }
    else
    {
        _69 = false;
    }
    bool _81 = false;
    if (_69)
    {
        int3 _72 = clamp(_44.xyz, int3(-100, -100, -100), int3(100, 100, 100));
        _81 = all(bool3(_72.x == int4(-100, 0, 75, 100).xyz.x, _72.y == int4(-100, 0, 75, 100).xyz.y, _72.z == int4(-100, 0, 75, 100).xyz.z));
    }
    else
    {
        _81 = false;
    }
    bool _90 = false;
    if (_81)
    {
        int4 _84 = clamp(_44, int4(-100, -100, -100, -100), int4(100, 100, 100, 100));
        _90 = all(bool4(_84.x == int4(-100, 0, 75, 100).x, _84.y == int4(-100, 0, 75, 100).y, _84.z == int4(-100, 0, 75, 100).z, _84.w == int4(-100, 0, 75, 100).w));
    }
    else
    {
        _90 = false;
    }
    bool _94 = false;
    if (_90)
    {
        _94 = true;
    }
    else
    {
        _94 = false;
    }
    bool _101 = false;
    if (_94)
    {
        _101 = all(bool2(int2(-100, 0).x == int4(-100, 0, 75, 100).xy.x, int2(-100, 0).y == int4(-100, 0, 75, 100).xy.y));
    }
    else
    {
        _101 = false;
    }
    bool _108 = false;
    if (_101)
    {
        _108 = all(bool3(int3(-100, 0, 75).x == int4(-100, 0, 75, 100).xyz.x, int3(-100, 0, 75).y == int4(-100, 0, 75, 100).xyz.y, int3(-100, 0, 75).z == int4(-100, 0, 75, 100).xyz.z));
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
    bool _116 = false;
    if (_111)
    {
        _116 = clamp(_56, -100, 100) == (-100);
    }
    else
    {
        _116 = false;
    }
    bool _128 = false;
    if (_116)
    {
        int2 _119 = clamp(_44.xy, int2(-100, -200), int2(100, 200));
        _128 = all(bool2(_119.x == int4(-100, 0, 50, 225).xy.x, _119.y == int4(-100, 0, 50, 225).xy.y));
    }
    else
    {
        _128 = false;
    }
    bool _138 = false;
    if (_128)
    {
        int3 _131 = clamp(_44.xyz, int3(-100, -200, -200), int3(100, 200, 50));
        _138 = all(bool3(_131.x == int4(-100, 0, 50, 225).xyz.x, _131.y == int4(-100, 0, 50, 225).xyz.y, _131.z == int4(-100, 0, 50, 225).xyz.z));
    }
    else
    {
        _138 = false;
    }
    bool _147 = false;
    if (_138)
    {
        int4 _141 = clamp(_44, int4(-100, -200, -200, 100), int4(100, 200, 50, 300));
        _147 = all(bool4(_141.x == int4(-100, 0, 50, 225).x, _141.y == int4(-100, 0, 50, 225).y, _141.z == int4(-100, 0, 50, 225).z, _141.w == int4(-100, 0, 50, 225).w));
    }
    else
    {
        _147 = false;
    }
    bool _150 = false;
    if (_147)
    {
        _150 = true;
    }
    else
    {
        _150 = false;
    }
    bool _156 = false;
    if (_150)
    {
        _156 = all(bool2(int2(-100, 0).x == int4(-100, 0, 50, 225).xy.x, int2(-100, 0).y == int4(-100, 0, 50, 225).xy.y));
    }
    else
    {
        _156 = false;
    }
    bool _163 = false;
    if (_156)
    {
        _163 = all(bool3(int3(-100, 0, 50).x == int4(-100, 0, 50, 225).xyz.x, int3(-100, 0, 50).y == int4(-100, 0, 50, 225).xyz.y, int3(-100, 0, 50).z == int4(-100, 0, 50, 225).xyz.z));
    }
    else
    {
        _163 = false;
    }
    bool _166 = false;
    if (_163)
    {
        _166 = true;
    }
    else
    {
        _166 = false;
    }
    float4 _167 = 0.0f.xxxx;
    if (_166)
    {
        _167 = _10_colorGreen;
    }
    else
    {
        _167 = _10_colorRed;
    }
    return _167;
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
