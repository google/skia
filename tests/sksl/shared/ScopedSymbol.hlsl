struct S
{
    int i;
};

cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _17_colorGreen : packoffset(c0);
    float4 _17_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

static int glob = 0;

bool block_variable_hides_global_variable_b()
{
    return glob == 2;
}

bool local_variable_hides_struct_b()
{
    bool S_1 = true;
    return true;
}

bool local_struct_variable_hides_struct_type_b()
{
    S _44 = { 1 };
    S S_1 = _44;
    return S_1.i == 1;
}

bool local_variable_hides_global_variable_b()
{
    int glob_1 = 1;
    return true;
}

float4 main(float2 _53)
{
    glob = 2;
    bool _59 = false;
    if (true)
    {
        _59 = block_variable_hides_global_variable_b();
    }
    else
    {
        _59 = false;
    }
    bool _63 = false;
    if (_59)
    {
        _63 = local_variable_hides_struct_b();
    }
    else
    {
        _63 = false;
    }
    bool _67 = false;
    if (_63)
    {
        _67 = local_struct_variable_hides_struct_type_b();
    }
    else
    {
        _67 = false;
    }
    bool _71 = false;
    if (_67)
    {
        _71 = local_variable_hides_global_variable_b();
    }
    else
    {
        _71 = false;
    }
    float4 _72 = 0.0f.xxxx;
    if (_71)
    {
        _72 = _17_colorGreen;
    }
    else
    {
        _72 = _17_colorRed;
    }
    return _72;
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
