static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _18)
{
    float4 result = 0.0f.xxxx;
    float a = 0.0f;
    float b = 0.0f;
    for (;;)
    {
        bool _40 = false;
        if (a < 10.0f)
        {
            _40 = b < 10.0f;
        }
        else
        {
            _40 = false;
        }
        if (_40)
        {
            result.x += a;
            result.y += b;
            a += 1.0f;
            b += 1.0f;
            continue;
        }
        else
        {
            break;
        }
    }
    for (int c = 0; c < 10; c++)
    {
        result.z += 1.0f;
    }
    float _76[2] = { 0.0f, 10.0f };
    float d[2] = _76;
    float _84[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
    float e[4] = _84;
    float f = 9.0f;
    for (; d[0] < d[1]; d[0] += 1.0f)
    {
        result.w = e[0] * f;
    }
    for (;;)
    {
        break;
    }
    for (;;)
    {
        break;
    }
    return result;
}

void frag_main()
{
    float2 _14 = 0.0f.xx;
    sk_FragColor = main(_14);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
