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
    bool4 _42 = bool4(_10_colorGreen.x != 0.0f, _10_colorGreen.y != 0.0f, _10_colorGreen.z != 0.0f, _10_colorGreen.w != 0.0f);
    bool4 FTFT = _42;
    bool4 _44 = _42.wzyx;
    bool4 TFTF = _44;
    bool _91 = false;
    if ((_42.x ? _10_colorWhite.x : _10_colorBlack.x) == _10_colorBlack.x)
    {
        bool2 _83 = _42.xy;
        float2 _68 = float2(_83.x ? _10_colorWhite.xy.x : _10_colorBlack.xy.x, _83.y ? _10_colorWhite.xy.y : _10_colorBlack.xy.y);
        float2 _88 = float2(_10_colorBlack.x, 1.0f);
        _91 = all(bool2(_68.x == _88.x, _68.y == _88.y));
    }
    else
    {
        _91 = false;
    }
    bool _120 = false;
    if (_91)
    {
        bool3 _110 = _42.xyz;
        float3 _94 = float3(_110.x ? _10_colorWhite.xyz.x : _10_colorBlack.xyz.x, _110.y ? _10_colorWhite.xyz.y : _10_colorBlack.xyz.y, _110.z ? _10_colorWhite.xyz.z : _10_colorBlack.xyz.z);
        float3 _117 = float3(_10_colorBlack.x, 1.0f, _10_colorBlack.z);
        _120 = all(bool3(_94.x == _117.x, _94.y == _117.y, _94.z == _117.z));
    }
    else
    {
        _120 = false;
    }
    bool _141 = false;
    if (_120)
    {
        float4 _123 = float4(_42.x ? _10_colorWhite.x : _10_colorBlack.x, _42.y ? _10_colorWhite.y : _10_colorBlack.y, _42.z ? _10_colorWhite.z : _10_colorBlack.z, _42.w ? _10_colorWhite.w : _10_colorBlack.w);
        float4 _138 = float4(_10_colorBlack.x, 1.0f, _10_colorBlack.z, 1.0f);
        _141 = all(bool4(_123.x == _138.x, _123.y == _138.y, _123.z == _138.z, _123.w == _138.w));
    }
    else
    {
        _141 = false;
    }
    bool _163 = false;
    if (_141)
    {
        _163 = (_44.x ? _10_testInputs.x : _10_colorWhite.x) == _10_testInputs.x;
    }
    else
    {
        _163 = false;
    }
    bool _187 = false;
    if (_163)
    {
        bool2 _180 = _44.xy;
        float2 _166 = float2(_180.x ? _10_testInputs.xy.x : _10_colorWhite.xy.x, _180.y ? _10_testInputs.xy.y : _10_colorWhite.xy.y);
        float2 _184 = float2(_10_testInputs.x, 1.0f);
        _187 = all(bool2(_166.x == _184.x, _166.y == _184.y));
    }
    else
    {
        _187 = false;
    }
    bool _214 = false;
    if (_187)
    {
        bool3 _204 = _44.xyz;
        float3 _190 = float3(_204.x ? _10_testInputs.xyz.x : _10_colorWhite.xyz.x, _204.y ? _10_testInputs.xyz.y : _10_colorWhite.xyz.y, _204.z ? _10_testInputs.xyz.z : _10_colorWhite.xyz.z);
        float3 _211 = float3(_10_testInputs.x, 1.0f, _10_testInputs.z);
        _214 = all(bool3(_190.x == _211.x, _190.y == _211.y, _190.z == _211.z));
    }
    else
    {
        _214 = false;
    }
    bool _235 = false;
    if (_214)
    {
        float4 _217 = float4(_44.x ? _10_testInputs.x : _10_colorWhite.x, _44.y ? _10_testInputs.y : _10_colorWhite.y, _44.z ? _10_testInputs.z : _10_colorWhite.z, _44.w ? _10_testInputs.w : _10_colorWhite.w);
        float4 _232 = float4(_10_testInputs.x, 1.0f, _10_testInputs.z, 1.0f);
        _235 = all(bool4(_217.x == _232.x, _217.y == _232.y, _217.z == _232.z, _217.w == _232.w));
    }
    else
    {
        _235 = false;
    }
    float4 _236 = 0.0f.xxxx;
    if (_235)
    {
        _236 = _10_colorGreen;
    }
    else
    {
        _236 = _10_colorRed;
    }
    return _236;
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
