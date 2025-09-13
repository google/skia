cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float2x2 _11_testMatrix2x2 : packoffset(c0);
    float4 _11_colorGreen : packoffset(c2);
    float4 _11_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _26)
{
    float4 _46 = float4(_11_testMatrix2x2[0].x, _11_testMatrix2x2[0].y, _11_testMatrix2x2[1].x, _11_testMatrix2x2[1].y) * (1.0f / _11_colorGreen.x);
    float4 infiniteValue = _46;
    float4 _59 = float4(_11_testMatrix2x2[0].x, _11_testMatrix2x2[0].y, _11_testMatrix2x2[1].x, _11_testMatrix2x2[1].y) * (1.0f / _11_colorGreen.y);
    float4 finiteValue = _59;
    bool _70 = false;
    if (isinf(_46.x))
    {
        _70 = all(isinf(_46.xy));
    }
    else
    {
        _70 = false;
    }
    bool _78 = false;
    if (_70)
    {
        _78 = all(isinf(_46.xyz));
    }
    else
    {
        _78 = false;
    }
    bool _84 = false;
    if (_78)
    {
        _84 = all(isinf(_46));
    }
    else
    {
        _84 = false;
    }
    bool _90 = false;
    if (_84)
    {
        _90 = !isinf(_59.x);
    }
    else
    {
        _90 = false;
    }
    bool _97 = false;
    if (_90)
    {
        _97 = !any(isinf(_59.xy));
    }
    else
    {
        _97 = false;
    }
    bool _104 = false;
    if (_97)
    {
        _104 = !any(isinf(_59.xyz));
    }
    else
    {
        _104 = false;
    }
    bool _110 = false;
    if (_104)
    {
        _110 = !any(isinf(_59));
    }
    else
    {
        _110 = false;
    }
    float4 _111 = 0.0f.xxxx;
    if (_110)
    {
        _111 = _11_colorGreen;
    }
    else
    {
        _111 = _11_colorRed;
    }
    return _111;
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
