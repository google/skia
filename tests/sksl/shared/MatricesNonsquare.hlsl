cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _12_colorGreen : packoffset(c0);
    float4 _12_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool test_half_b()
{
    bool ok = true;
    float2x3 m23 = float2x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f));
    bool _48 = false;
    if (true)
    {
        _48 = all(bool3(float3(2.0f, 0.0f, 0.0f).x == float3(2.0f, 0.0f, 0.0f).x, float3(2.0f, 0.0f, 0.0f).y == float3(2.0f, 0.0f, 0.0f).y, float3(2.0f, 0.0f, 0.0f).z == float3(2.0f, 0.0f, 0.0f).z)) && all(bool3(float3(0.0f, 2.0f, 0.0f).x == float3(0.0f, 2.0f, 0.0f).x, float3(0.0f, 2.0f, 0.0f).y == float3(0.0f, 2.0f, 0.0f).y, float3(0.0f, 2.0f, 0.0f).z == float3(0.0f, 2.0f, 0.0f).z));
    }
    else
    {
        _48 = false;
    }
    ok = _48;
    float2x4 m24 = float2x4(float4(3.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 3.0f, 0.0f, 0.0f));
    bool _64 = false;
    if (_48)
    {
        _64 = all(bool4(float4(3.0f, 0.0f, 0.0f, 0.0f).x == float4(3.0f, 0.0f, 0.0f, 0.0f).x, float4(3.0f, 0.0f, 0.0f, 0.0f).y == float4(3.0f, 0.0f, 0.0f, 0.0f).y, float4(3.0f, 0.0f, 0.0f, 0.0f).z == float4(3.0f, 0.0f, 0.0f, 0.0f).z, float4(3.0f, 0.0f, 0.0f, 0.0f).w == float4(3.0f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(float4(0.0f, 3.0f, 0.0f, 0.0f).x == float4(0.0f, 3.0f, 0.0f, 0.0f).x, float4(0.0f, 3.0f, 0.0f, 0.0f).y == float4(0.0f, 3.0f, 0.0f, 0.0f).y, float4(0.0f, 3.0f, 0.0f, 0.0f).z == float4(0.0f, 3.0f, 0.0f, 0.0f).z, float4(0.0f, 3.0f, 0.0f, 0.0f).w == float4(0.0f, 3.0f, 0.0f, 0.0f).w));
    }
    else
    {
        _64 = false;
    }
    ok = _64;
    float3x2 m32 = float3x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f), 0.0f.xx);
    bool _83 = false;
    if (_64)
    {
        _83 = (all(bool2(float2(4.0f, 0.0f).x == float2(4.0f, 0.0f).x, float2(4.0f, 0.0f).y == float2(4.0f, 0.0f).y)) && all(bool2(float2(0.0f, 4.0f).x == float2(0.0f, 4.0f).x, float2(0.0f, 4.0f).y == float2(0.0f, 4.0f).y))) && all(bool2(0.0f.xx.x == 0.0f.xx.x, 0.0f.xx.y == 0.0f.xx.y));
    }
    else
    {
        _83 = false;
    }
    ok = _83;
    float3x4 m34 = float3x4(float4(5.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 5.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 5.0f, 0.0f));
    bool _102 = false;
    if (_83)
    {
        _102 = (all(bool4(float4(5.0f, 0.0f, 0.0f, 0.0f).x == float4(5.0f, 0.0f, 0.0f, 0.0f).x, float4(5.0f, 0.0f, 0.0f, 0.0f).y == float4(5.0f, 0.0f, 0.0f, 0.0f).y, float4(5.0f, 0.0f, 0.0f, 0.0f).z == float4(5.0f, 0.0f, 0.0f, 0.0f).z, float4(5.0f, 0.0f, 0.0f, 0.0f).w == float4(5.0f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(float4(0.0f, 5.0f, 0.0f, 0.0f).x == float4(0.0f, 5.0f, 0.0f, 0.0f).x, float4(0.0f, 5.0f, 0.0f, 0.0f).y == float4(0.0f, 5.0f, 0.0f, 0.0f).y, float4(0.0f, 5.0f, 0.0f, 0.0f).z == float4(0.0f, 5.0f, 0.0f, 0.0f).z, float4(0.0f, 5.0f, 0.0f, 0.0f).w == float4(0.0f, 5.0f, 0.0f, 0.0f).w))) && all(bool4(float4(0.0f, 0.0f, 5.0f, 0.0f).x == float4(0.0f, 0.0f, 5.0f, 0.0f).x, float4(0.0f, 0.0f, 5.0f, 0.0f).y == float4(0.0f, 0.0f, 5.0f, 0.0f).y, float4(0.0f, 0.0f, 5.0f, 0.0f).z == float4(0.0f, 0.0f, 5.0f, 0.0f).z, float4(0.0f, 0.0f, 5.0f, 0.0f).w == float4(0.0f, 0.0f, 5.0f, 0.0f).w));
    }
    else
    {
        _102 = false;
    }
    ok = _102;
    float4x2 m42 = float4x2(float2(6.0f, 0.0f), float2(0.0f, 6.0f), 0.0f.xx, 0.0f.xx);
    bool _123 = false;
    if (_102)
    {
        _123 = ((all(bool2(float2(6.0f, 0.0f).x == float2(6.0f, 0.0f).x, float2(6.0f, 0.0f).y == float2(6.0f, 0.0f).y)) && all(bool2(float2(0.0f, 6.0f).x == float2(0.0f, 6.0f).x, float2(0.0f, 6.0f).y == float2(0.0f, 6.0f).y))) && all(bool2(0.0f.xx.x == 0.0f.xx.x, 0.0f.xx.y == 0.0f.xx.y))) && all(bool2(0.0f.xx.x == 0.0f.xx.x, 0.0f.xx.y == 0.0f.xx.y));
    }
    else
    {
        _123 = false;
    }
    ok = _123;
    float4x3 m43 = float4x3(float3(7.0f, 0.0f, 0.0f), float3(0.0f, 7.0f, 0.0f), float3(0.0f, 0.0f, 7.0f), 0.0f.xxx);
    bool _146 = false;
    if (_123)
    {
        _146 = ((all(bool3(float3(7.0f, 0.0f, 0.0f).x == float3(7.0f, 0.0f, 0.0f).x, float3(7.0f, 0.0f, 0.0f).y == float3(7.0f, 0.0f, 0.0f).y, float3(7.0f, 0.0f, 0.0f).z == float3(7.0f, 0.0f, 0.0f).z)) && all(bool3(float3(0.0f, 7.0f, 0.0f).x == float3(0.0f, 7.0f, 0.0f).x, float3(0.0f, 7.0f, 0.0f).y == float3(0.0f, 7.0f, 0.0f).y, float3(0.0f, 7.0f, 0.0f).z == float3(0.0f, 7.0f, 0.0f).z))) && all(bool3(float3(0.0f, 0.0f, 7.0f).x == float3(0.0f, 0.0f, 7.0f).x, float3(0.0f, 0.0f, 7.0f).y == float3(0.0f, 0.0f, 7.0f).y, float3(0.0f, 0.0f, 7.0f).z == float3(0.0f, 0.0f, 7.0f).z))) && all(bool3(0.0f.xxx.x == 0.0f.xxx.x, 0.0f.xxx.y == 0.0f.xxx.y, 0.0f.xxx.z == 0.0f.xxx.z));
    }
    else
    {
        _146 = false;
    }
    ok = _146;
    float2x2 _150 = mul(float2x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f)), float3x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f), 0.0f.xx));
    float2x2 m22 = _150;
    bool _164 = false;
    if (_146)
    {
        float2 _157 = _150[0];
        float2 _160 = _150[1];
        _164 = all(bool2(_157.x == float2(8.0f, 0.0f).x, _157.y == float2(8.0f, 0.0f).y)) && all(bool2(_160.x == float2(0.0f, 8.0f).x, _160.y == float2(0.0f, 8.0f).y));
    }
    else
    {
        _164 = false;
    }
    ok = _164;
    float3x3 _168 = mul(float3x4(float4(5.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 5.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 5.0f, 0.0f)), float4x3(float3(7.0f, 0.0f, 0.0f), float3(0.0f, 7.0f, 0.0f), float3(0.0f, 0.0f, 7.0f), 0.0f.xxx));
    float3x3 m33 = _168;
    bool _187 = false;
    if (_164)
    {
        float3 _176 = _168[0];
        float3 _179 = _168[1];
        float3 _183 = _168[2];
        _187 = (all(bool3(_176.x == float3(35.0f, 0.0f, 0.0f).x, _176.y == float3(35.0f, 0.0f, 0.0f).y, _176.z == float3(35.0f, 0.0f, 0.0f).z)) && all(bool3(_179.x == float3(0.0f, 35.0f, 0.0f).x, _179.y == float3(0.0f, 35.0f, 0.0f).y, _179.z == float3(0.0f, 35.0f, 0.0f).z))) && all(bool3(_183.x == float3(0.0f, 0.0f, 35.0f).x, _183.y == float3(0.0f, 0.0f, 35.0f).y, _183.z == float3(0.0f, 0.0f, 35.0f).z));
    }
    else
    {
        _187 = false;
    }
    ok = _187;
    float3 _191 = float3(2.0f, 0.0f, 0.0f) + 1.0f.xxx;
    float3 _192 = float3(0.0f, 2.0f, 0.0f) + 1.0f.xxx;
    m23 = float2x3(_191, _192);
    bool _204 = false;
    if (_187)
    {
        _204 = all(bool3(_191.x == float3(3.0f, 1.0f, 1.0f).x, _191.y == float3(3.0f, 1.0f, 1.0f).y, _191.z == float3(3.0f, 1.0f, 1.0f).z)) && all(bool3(_192.x == float3(1.0f, 3.0f, 1.0f).x, _192.y == float3(1.0f, 3.0f, 1.0f).y, _192.z == float3(1.0f, 3.0f, 1.0f).z));
    }
    else
    {
        _204 = false;
    }
    ok = _204;
    float2 _207 = float2(4.0f, 0.0f) - 2.0f.xx;
    float2 _208 = float2(0.0f, 4.0f) - 2.0f.xx;
    float2 _209 = 0.0f.xx - 2.0f.xx;
    m32 = float3x2(_207, _208, _209);
    bool _226 = false;
    if (_204)
    {
        _226 = (all(bool2(_207.x == float2(2.0f, -2.0f).x, _207.y == float2(2.0f, -2.0f).y)) && all(bool2(_208.x == float2(-2.0f, 2.0f).x, _208.y == float2(-2.0f, 2.0f).y))) && all(bool2(_209.x == (-2.0f).xx.x, _209.y == (-2.0f).xx.y));
    }
    else
    {
        _226 = false;
    }
    ok = _226;
    float2x4 _228 = float2x4(float4(3.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 3.0f, 0.0f, 0.0f)) * 0.25f;
    m24 = _228;
    bool _242 = false;
    if (_226)
    {
        float4 _235 = _228[0];
        float4 _238 = _228[1];
        _242 = all(bool4(_235.x == float4(0.75f, 0.0f, 0.0f, 0.0f).x, _235.y == float4(0.75f, 0.0f, 0.0f, 0.0f).y, _235.z == float4(0.75f, 0.0f, 0.0f, 0.0f).z, _235.w == float4(0.75f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(_238.x == float4(0.0f, 0.75f, 0.0f, 0.0f).x, _238.y == float4(0.0f, 0.75f, 0.0f, 0.0f).y, _238.z == float4(0.0f, 0.75f, 0.0f, 0.0f).z, _238.w == float4(0.0f, 0.75f, 0.0f, 0.0f).w));
    }
    else
    {
        _242 = false;
    }
    ok = _242;
    return _242;
}

float4 main(float2 _244)
{
    bool _RESERVED_IDENTIFIER_FIXUP_0_ok = true;
    float2x3 _RESERVED_IDENTIFIER_FIXUP_1_m23 = float2x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f));
    bool _255 = false;
    if (true)
    {
        _255 = all(bool3(float3(2.0f, 0.0f, 0.0f).x == float3(2.0f, 0.0f, 0.0f).x, float3(2.0f, 0.0f, 0.0f).y == float3(2.0f, 0.0f, 0.0f).y, float3(2.0f, 0.0f, 0.0f).z == float3(2.0f, 0.0f, 0.0f).z)) && all(bool3(float3(0.0f, 2.0f, 0.0f).x == float3(0.0f, 2.0f, 0.0f).x, float3(0.0f, 2.0f, 0.0f).y == float3(0.0f, 2.0f, 0.0f).y, float3(0.0f, 2.0f, 0.0f).z == float3(0.0f, 2.0f, 0.0f).z));
    }
    else
    {
        _255 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _255;
    float2x4 _RESERVED_IDENTIFIER_FIXUP_2_m24 = float2x4(float4(3.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 3.0f, 0.0f, 0.0f));
    bool _264 = false;
    if (_255)
    {
        _264 = all(bool4(float4(3.0f, 0.0f, 0.0f, 0.0f).x == float4(3.0f, 0.0f, 0.0f, 0.0f).x, float4(3.0f, 0.0f, 0.0f, 0.0f).y == float4(3.0f, 0.0f, 0.0f, 0.0f).y, float4(3.0f, 0.0f, 0.0f, 0.0f).z == float4(3.0f, 0.0f, 0.0f, 0.0f).z, float4(3.0f, 0.0f, 0.0f, 0.0f).w == float4(3.0f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(float4(0.0f, 3.0f, 0.0f, 0.0f).x == float4(0.0f, 3.0f, 0.0f, 0.0f).x, float4(0.0f, 3.0f, 0.0f, 0.0f).y == float4(0.0f, 3.0f, 0.0f, 0.0f).y, float4(0.0f, 3.0f, 0.0f, 0.0f).z == float4(0.0f, 3.0f, 0.0f, 0.0f).z, float4(0.0f, 3.0f, 0.0f, 0.0f).w == float4(0.0f, 3.0f, 0.0f, 0.0f).w));
    }
    else
    {
        _264 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _264;
    float3x2 _RESERVED_IDENTIFIER_FIXUP_3_m32 = float3x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f), 0.0f.xx);
    bool _276 = false;
    if (_264)
    {
        _276 = (all(bool2(float2(4.0f, 0.0f).x == float2(4.0f, 0.0f).x, float2(4.0f, 0.0f).y == float2(4.0f, 0.0f).y)) && all(bool2(float2(0.0f, 4.0f).x == float2(0.0f, 4.0f).x, float2(0.0f, 4.0f).y == float2(0.0f, 4.0f).y))) && all(bool2(0.0f.xx.x == 0.0f.xx.x, 0.0f.xx.y == 0.0f.xx.y));
    }
    else
    {
        _276 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _276;
    float2x2 _278 = mul(float2x3(float3(2.0f, 0.0f, 0.0f), float3(0.0f, 2.0f, 0.0f)), float3x2(float2(4.0f, 0.0f), float2(0.0f, 4.0f), 0.0f.xx));
    float2x2 _RESERVED_IDENTIFIER_FIXUP_7_m22 = _278;
    bool _288 = false;
    if (_276)
    {
        float2 _281 = _278[0];
        float2 _284 = _278[1];
        _288 = all(bool2(_281.x == float2(8.0f, 0.0f).x, _281.y == float2(8.0f, 0.0f).y)) && all(bool2(_284.x == float2(0.0f, 8.0f).x, _284.y == float2(0.0f, 8.0f).y));
    }
    else
    {
        _288 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _288;
    float3 _289 = float3(2.0f, 0.0f, 0.0f) + 1.0f.xxx;
    float3 _290 = float3(0.0f, 2.0f, 0.0f) + 1.0f.xxx;
    _RESERVED_IDENTIFIER_FIXUP_1_m23 = float2x3(_289, _290);
    bool _299 = false;
    if (_288)
    {
        _299 = all(bool3(_289.x == float3(3.0f, 1.0f, 1.0f).x, _289.y == float3(3.0f, 1.0f, 1.0f).y, _289.z == float3(3.0f, 1.0f, 1.0f).z)) && all(bool3(_290.x == float3(1.0f, 3.0f, 1.0f).x, _290.y == float3(1.0f, 3.0f, 1.0f).y, _290.z == float3(1.0f, 3.0f, 1.0f).z));
    }
    else
    {
        _299 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _299;
    float2 _300 = float2(4.0f, 0.0f) - 2.0f.xx;
    float2 _301 = float2(0.0f, 4.0f) - 2.0f.xx;
    float2 _302 = 0.0f.xx - 2.0f.xx;
    _RESERVED_IDENTIFIER_FIXUP_3_m32 = float3x2(_300, _301, _302);
    bool _314 = false;
    if (_299)
    {
        _314 = (all(bool2(_300.x == float2(2.0f, -2.0f).x, _300.y == float2(2.0f, -2.0f).y)) && all(bool2(_301.x == float2(-2.0f, 2.0f).x, _301.y == float2(-2.0f, 2.0f).y))) && all(bool2(_302.x == (-2.0f).xx.x, _302.y == (-2.0f).xx.y));
    }
    else
    {
        _314 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _314;
    float2x4 _315 = float2x4(float4(3.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 3.0f, 0.0f, 0.0f)) * 0.25f;
    _RESERVED_IDENTIFIER_FIXUP_2_m24 = _315;
    bool _325 = false;
    if (_314)
    {
        float4 _318 = _315[0];
        float4 _321 = _315[1];
        _325 = all(bool4(_318.x == float4(0.75f, 0.0f, 0.0f, 0.0f).x, _318.y == float4(0.75f, 0.0f, 0.0f, 0.0f).y, _318.z == float4(0.75f, 0.0f, 0.0f, 0.0f).z, _318.w == float4(0.75f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(_321.x == float4(0.0f, 0.75f, 0.0f, 0.0f).x, _321.y == float4(0.0f, 0.75f, 0.0f, 0.0f).y, _321.z == float4(0.0f, 0.75f, 0.0f, 0.0f).z, _321.w == float4(0.0f, 0.75f, 0.0f, 0.0f).w));
    }
    else
    {
        _325 = false;
    }
    _RESERVED_IDENTIFIER_FIXUP_0_ok = _325;
    bool _329 = false;
    if (_325)
    {
        _329 = test_half_b();
    }
    else
    {
        _329 = false;
    }
    float4 _330 = 0.0f.xxxx;
    if (_329)
    {
        _330 = _12_colorGreen;
    }
    else
    {
        _330 = _12_colorRed;
    }
    return _330;
}

void frag_main()
{
    float2 _22 = 0.0f.xx;
    sk_FragColor = main(_22);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
