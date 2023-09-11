cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _15_colorGreen : packoffset(c0);
    float4 _15_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static int scratchVar = 0;

bool test_flat_b()
{
    return true;
}

bool test_if_b()
{
    if (_15_colorGreen.y > 0.0f)
    {
        return true;
    }
    else
    {
        scratchVar++;
    }
    scratchVar++;
    return false;
}

bool test_else_b()
{
    if (_15_colorGreen.y == 0.0f)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool test_loop_if_b()
{
    for (int x = 0; x <= 1; x++)
    {
        if (_15_colorGreen.y == 0.0f)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    scratchVar++;
    return true;
}

float4 main(float2 _77)
{
    scratchVar = 0;
    bool _83 = false;
    if (test_flat_b())
    {
        bool _82 = test_if_b();
        _83 = _82;
    }
    else
    {
        _83 = false;
    }
    bool _87 = false;
    if (_83)
    {
        _87 = test_else_b();
    }
    else
    {
        _87 = false;
    }
    bool _91 = false;
    if (_87)
    {
        bool _90 = test_loop_if_b();
        _91 = _90;
    }
    else
    {
        _91 = false;
    }
    float4 _92 = 0.0f.xxxx;
    if (_91)
    {
        _92 = _15_colorGreen;
    }
    else
    {
        _92 = _15_colorRed;
    }
    return _92;
}

void frag_main()
{
    float2 _25 = 0.0f.xx;
    float4 _27 = main(_25);
    sk_FragColor = _27;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
