static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float userfunc_ff(float _24)
{
    return _24 + 1.0f;
}

float4 main(float2 _30)
{
    float b = 2.0f;
    float c = 3.0f;
    b = 2.0f;
    b = 3.0f + 77.0f;
    b = sin(3.0f + 77.0f);
    float _41 = 3.0f + 77.0f;
    float _44 = 3.0f + 77.0f;
    b = userfunc_ff(_44);
    float _46 = cos(3.0f);
    b = _46;
    b = _46;
    for (int x = 0; x < 1; x++)
    {
    }
    float d = c;
    b = 3.0f;
    float _63 = c + 1.0f;
    d = _63;
    return float4(float(3.0f == 2.0f), float(true), float(_63 == 5.0f), float(_63 == 4.0f));
}

void frag_main()
{
    float2 _19 = 0.0f.xx;
    sk_FragColor = main(_19);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
