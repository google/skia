cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorGreen : packoffset(c0);
    float4 _7_colorRed : packoffset(c1);
    float4 _7_colorBlack : packoffset(c2);
    float4 _7_colorWhite : packoffset(c3);
    float4 _7_testInputs : packoffset(c4);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    bool4 _40 = bool4(_7_colorGreen.x != 0.0f, _7_colorGreen.y != 0.0f, _7_colorGreen.z != 0.0f, _7_colorGreen.w != 0.0f);
    bool4 FTFT = _40;
    bool4 _42 = _40.wzyx;
    bool4 TFTF = _42;
    bool _89 = false;
    if ((_40.x ? _7_colorWhite.x : _7_colorBlack.x) == _7_colorBlack.x)
    {
        bool2 _81 = _40.xy;
        float2 _66 = float2(_81.x ? _7_colorWhite.xy.x : _7_colorBlack.xy.x, _81.y ? _7_colorWhite.xy.y : _7_colorBlack.xy.y);
        float2 _86 = float2(_7_colorBlack.x, 1.0f);
        _89 = all(bool2(_66.x == _86.x, _66.y == _86.y));
    }
    else
    {
        _89 = false;
    }
    bool _118 = false;
    if (_89)
    {
        bool3 _108 = _40.xyz;
        float3 _92 = float3(_108.x ? _7_colorWhite.xyz.x : _7_colorBlack.xyz.x, _108.y ? _7_colorWhite.xyz.y : _7_colorBlack.xyz.y, _108.z ? _7_colorWhite.xyz.z : _7_colorBlack.xyz.z);
        float3 _115 = float3(_7_colorBlack.x, 1.0f, _7_colorBlack.z);
        _118 = all(bool3(_92.x == _115.x, _92.y == _115.y, _92.z == _115.z));
    }
    else
    {
        _118 = false;
    }
    bool _139 = false;
    if (_118)
    {
        float4 _121 = float4(_40.x ? _7_colorWhite.x : _7_colorBlack.x, _40.y ? _7_colorWhite.y : _7_colorBlack.y, _40.z ? _7_colorWhite.z : _7_colorBlack.z, _40.w ? _7_colorWhite.w : _7_colorBlack.w);
        float4 _136 = float4(_7_colorBlack.x, 1.0f, _7_colorBlack.z, 1.0f);
        _139 = all(bool4(_121.x == _136.x, _121.y == _136.y, _121.z == _136.z, _121.w == _136.w));
    }
    else
    {
        _139 = false;
    }
    bool _161 = false;
    if (_139)
    {
        _161 = (_42.x ? _7_testInputs.x : _7_colorWhite.x) == _7_testInputs.x;
    }
    else
    {
        _161 = false;
    }
    bool _185 = false;
    if (_161)
    {
        bool2 _178 = _42.xy;
        float2 _164 = float2(_178.x ? _7_testInputs.xy.x : _7_colorWhite.xy.x, _178.y ? _7_testInputs.xy.y : _7_colorWhite.xy.y);
        float2 _182 = float2(_7_testInputs.x, 1.0f);
        _185 = all(bool2(_164.x == _182.x, _164.y == _182.y));
    }
    else
    {
        _185 = false;
    }
    bool _212 = false;
    if (_185)
    {
        bool3 _202 = _42.xyz;
        float3 _188 = float3(_202.x ? _7_testInputs.xyz.x : _7_colorWhite.xyz.x, _202.y ? _7_testInputs.xyz.y : _7_colorWhite.xyz.y, _202.z ? _7_testInputs.xyz.z : _7_colorWhite.xyz.z);
        float3 _209 = float3(_7_testInputs.x, 1.0f, _7_testInputs.z);
        _212 = all(bool3(_188.x == _209.x, _188.y == _209.y, _188.z == _209.z));
    }
    else
    {
        _212 = false;
    }
    bool _233 = false;
    if (_212)
    {
        float4 _215 = float4(_42.x ? _7_testInputs.x : _7_colorWhite.x, _42.y ? _7_testInputs.y : _7_colorWhite.y, _42.z ? _7_testInputs.z : _7_colorWhite.z, _42.w ? _7_testInputs.w : _7_colorWhite.w);
        float4 _230 = float4(_7_testInputs.x, 1.0f, _7_testInputs.z, 1.0f);
        _233 = all(bool4(_215.x == _230.x, _215.y == _230.y, _215.z == _230.z, _215.w == _230.w));
    }
    else
    {
        _233 = false;
    }
    float4 _234 = 0.0f.xxxx;
    if (_233)
    {
        _234 = _7_colorGreen;
    }
    else
    {
        _234 = _7_colorRed;
    }
    return _234;
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
