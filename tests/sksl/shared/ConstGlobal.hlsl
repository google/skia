cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _18_colorGreen : packoffset(c0);
    float4 _18_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static float4x4 MATRIXFIVE = float4x4(0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx, 0.0f.xxxx);

bool verify_const_globals_biih44(int _35, int _36, float4x4 _37)
{
    bool _48 = false;
    if (_35 == 7)
    {
        _48 = _36 == 10;
    }
    else
    {
        _48 = false;
    }
    bool _68 = false;
    if (_48)
    {
        _68 = ((all(bool4(_37[0].x == float4(5.0f, 0.0f, 0.0f, 0.0f).x, _37[0].y == float4(5.0f, 0.0f, 0.0f, 0.0f).y, _37[0].z == float4(5.0f, 0.0f, 0.0f, 0.0f).z, _37[0].w == float4(5.0f, 0.0f, 0.0f, 0.0f).w)) && all(bool4(_37[1].x == float4(0.0f, 5.0f, 0.0f, 0.0f).x, _37[1].y == float4(0.0f, 5.0f, 0.0f, 0.0f).y, _37[1].z == float4(0.0f, 5.0f, 0.0f, 0.0f).z, _37[1].w == float4(0.0f, 5.0f, 0.0f, 0.0f).w))) && all(bool4(_37[2].x == float4(0.0f, 0.0f, 5.0f, 0.0f).x, _37[2].y == float4(0.0f, 0.0f, 5.0f, 0.0f).y, _37[2].z == float4(0.0f, 0.0f, 5.0f, 0.0f).z, _37[2].w == float4(0.0f, 0.0f, 5.0f, 0.0f).w))) && all(bool4(_37[3].x == float4(0.0f, 0.0f, 0.0f, 5.0f).x, _37[3].y == float4(0.0f, 0.0f, 0.0f, 5.0f).y, _37[3].z == float4(0.0f, 0.0f, 0.0f, 5.0f).z, _37[3].w == float4(0.0f, 0.0f, 0.0f, 5.0f).w));
    }
    else
    {
        _68 = false;
    }
    return _68;
}

float4 main(float2 _70)
{
    MATRIXFIVE = float4x4(float4(5.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 5.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 5.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 5.0f));
    int _72 = 7;
    int _73 = 10;
    float4x4 _74 = float4x4(float4(5.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 5.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 5.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 5.0f));
    float4 _76 = 0.0f.xxxx;
    if (verify_const_globals_biih44(_72, _73, _74))
    {
        _76 = _18_colorGreen;
    }
    else
    {
        _76 = _18_colorRed;
    }
    return _76;
}

void frag_main()
{
    float2 _27 = 0.0f.xx;
    float4 _29 = main(_27);
    sk_FragColor = _29;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
