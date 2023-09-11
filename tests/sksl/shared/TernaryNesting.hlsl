cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_colorWhite : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float4 _33 = float4(0.0f, 0.0f, _7_colorWhite.zw);
    float4 colorBlue = _33;
    float4 _41 = float4(0.0f, _7_colorWhite.y, 0.0f, _7_colorWhite.w);
    float4 colorGreen = _41;
    float4 _49 = float4(_7_colorWhite.x, 0.0f, 0.0f, _7_colorWhite.w);
    float4 colorRed = _49;
    float4 _57 = 0.0f.xxxx;
    if (any(bool4(_7_colorWhite.x != _33.x, _7_colorWhite.y != _33.y, _7_colorWhite.z != _33.z, _7_colorWhite.w != _33.w)))
    {
        float4 _63 = 0.0f.xxxx;
        if (all(bool4(_41.x == _49.x, _41.y == _49.y, _41.z == _49.z, _41.w == _49.w)))
        {
            _63 = _49;
        }
        else
        {
            _63 = _41;
        }
        _57 = _63;
    }
    else
    {
        float4 _70 = 0.0f.xxxx;
        if (any(bool4(_49.x != _41.x, _49.y != _41.y, _49.z != _41.z, _49.w != _41.w)))
        {
            _70 = _33;
        }
        else
        {
            _70 = _7_colorWhite;
        }
        _57 = _70;
    }
    float4 result = _57;
    float4 _80 = 0.0f.xxxx;
    if (all(bool4(_49.x == _33.x, _49.y == _33.y, _49.z == _33.z, _49.w == _33.w)))
    {
        _80 = _7_colorWhite;
    }
    else
    {
        float4 _88 = 0.0f.xxxx;
        if (any(bool4(_49.x != _41.x, _49.y != _41.y, _49.z != _41.z, _49.w != _41.w)))
        {
            _88 = _57;
        }
        else
        {
            float4 _96 = 0.0f.xxxx;
            if (all(bool4(_49.x == _7_colorWhite.x, _49.y == _7_colorWhite.y, _49.z == _7_colorWhite.z, _49.w == _7_colorWhite.w)))
            {
                _96 = _33;
            }
            else
            {
                _96 = _49;
            }
            _88 = _96;
        }
        _80 = _88;
    }
    return _80;
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
