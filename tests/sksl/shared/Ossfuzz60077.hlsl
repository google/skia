static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void d_vi(int _24)
{
    int b = 4;
}

void c_vi(int _28)
{
    int _31 = _28;
    d_vi(_31);
}

void b_vi(int _33)
{
    int _36 = _33;
    c_vi(_36);
}

void a_vi(int _38)
{
    int _41 = _38;
    b_vi(_41);
    int _44 = _38;
    b_vi(_44);
}

float4 main(float2 _47)
{
    int i = 0;
    int _51 = i;
    a_vi(_51);
    return 0.0f.xxxx;
}

void frag_main()
{
    float2 _18 = 0.0f.xx;
    sk_FragColor = main(_18);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
