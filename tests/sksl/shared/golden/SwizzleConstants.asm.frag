OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %v "v"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %18 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%float_1_0 = OpConstant %float 1
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%v3float = OpTypeVector %float 3
%main = OpFunction %void None %11
%12 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%15 = OpExtInst %float %1 Sqrt %float_1
%17 = OpCompositeConstruct %v4float %15 %15 %15 %15
OpStore %v %17
%18 = OpLoad %v4float %v
%19 = OpCompositeExtract %float %18 0
%21 = OpCompositeConstruct %v4float %19 %float_1_0 %float_1_0 %float_1_0
OpStore %sk_FragColor %21
%22 = OpLoad %v4float %v
%23 = OpVectorShuffle %v2float %22 %22 0 1
%25 = OpCompositeExtract %float %23 0
%26 = OpCompositeExtract %float %23 1
%27 = OpCompositeConstruct %v4float %25 %26 %float_1_0 %float_1_0
OpStore %sk_FragColor %27
%28 = OpLoad %v4float %v
%29 = OpCompositeExtract %float %28 0
%30 = OpCompositeConstruct %v2float %29 %float_1_0
%31 = OpCompositeExtract %float %30 0
%32 = OpCompositeExtract %float %30 1
%33 = OpCompositeConstruct %v4float %31 %32 %float_1_0 %float_1_0
OpStore %sk_FragColor %33
%35 = OpLoad %v4float %v
%36 = OpCompositeExtract %float %35 1
%37 = OpCompositeConstruct %v2float %float_0 %36
%38 = OpCompositeExtract %float %37 0
%39 = OpCompositeExtract %float %37 1
%40 = OpCompositeConstruct %v4float %38 %39 %float_1_0 %float_1_0
OpStore %sk_FragColor %40
%41 = OpLoad %v4float %v
%42 = OpVectorShuffle %v3float %41 %41 0 1 2
%44 = OpCompositeExtract %float %42 0
%45 = OpCompositeExtract %float %42 1
%46 = OpCompositeExtract %float %42 2
%47 = OpCompositeConstruct %v4float %44 %45 %46 %float_1_0
OpStore %sk_FragColor %47
%48 = OpLoad %v4float %v
%49 = OpVectorShuffle %v2float %48 %48 0 1
%50 = OpCompositeExtract %float %49 0
%51 = OpCompositeExtract %float %49 1
%52 = OpCompositeConstruct %v3float %50 %51 %float_1_0
%53 = OpCompositeExtract %float %52 0
%54 = OpCompositeExtract %float %52 1
%55 = OpCompositeExtract %float %52 2
%56 = OpCompositeConstruct %v4float %53 %54 %55 %float_1_0
OpStore %sk_FragColor %56
%57 = OpLoad %v4float %v
%58 = OpCompositeExtract %float %57 0
%59 = OpLoad %v4float %v
%60 = OpCompositeExtract %float %59 2
%61 = OpCompositeConstruct %v3float %58 %float_0 %60
%62 = OpCompositeExtract %float %61 0
%63 = OpCompositeExtract %float %61 1
%64 = OpCompositeExtract %float %61 2
%65 = OpCompositeConstruct %v4float %62 %63 %64 %float_1_0
OpStore %sk_FragColor %65
%66 = OpLoad %v4float %v
%67 = OpCompositeExtract %float %66 0
%68 = OpCompositeConstruct %v3float %67 %float_1_0 %float_0
%69 = OpCompositeExtract %float %68 0
%70 = OpCompositeExtract %float %68 1
%71 = OpCompositeExtract %float %68 2
%72 = OpCompositeConstruct %v4float %69 %70 %71 %float_1_0
OpStore %sk_FragColor %72
%73 = OpLoad %v4float %v
%74 = OpVectorShuffle %v2float %73 %73 1 2
%75 = OpCompositeExtract %float %74 0
%76 = OpCompositeExtract %float %74 1
%77 = OpCompositeConstruct %v3float %float_1_0 %75 %76
%78 = OpCompositeExtract %float %77 0
%79 = OpCompositeExtract %float %77 1
%80 = OpCompositeExtract %float %77 2
%81 = OpCompositeConstruct %v4float %78 %79 %80 %float_1_0
OpStore %sk_FragColor %81
%82 = OpLoad %v4float %v
%83 = OpCompositeExtract %float %82 1
%84 = OpCompositeConstruct %v3float %float_0 %83 %float_1_0
%85 = OpCompositeExtract %float %84 0
%86 = OpCompositeExtract %float %84 1
%87 = OpCompositeExtract %float %84 2
%88 = OpCompositeConstruct %v4float %85 %86 %87 %float_1_0
OpStore %sk_FragColor %88
%89 = OpLoad %v4float %v
%90 = OpCompositeExtract %float %89 2
%91 = OpCompositeConstruct %v3float %float_1_0 %float_1_0 %90
%92 = OpCompositeExtract %float %91 0
%93 = OpCompositeExtract %float %91 1
%94 = OpCompositeExtract %float %91 2
%95 = OpCompositeConstruct %v4float %92 %93 %94 %float_1_0
OpStore %sk_FragColor %95
%96 = OpLoad %v4float %v
OpStore %sk_FragColor %96
%97 = OpLoad %v4float %v
%98 = OpVectorShuffle %v3float %97 %97 0 1 2
%99 = OpCompositeExtract %float %98 0
%100 = OpCompositeExtract %float %98 1
%101 = OpCompositeExtract %float %98 2
%102 = OpCompositeConstruct %v4float %99 %100 %101 %float_1_0
OpStore %sk_FragColor %102
%103 = OpLoad %v4float %v
%104 = OpVectorShuffle %v2float %103 %103 0 1
%105 = OpCompositeExtract %float %104 0
%106 = OpCompositeExtract %float %104 1
%107 = OpLoad %v4float %v
%108 = OpCompositeExtract %float %107 3
%109 = OpCompositeConstruct %v4float %105 %106 %float_0 %108
OpStore %sk_FragColor %109
%110 = OpLoad %v4float %v
%111 = OpVectorShuffle %v2float %110 %110 0 1
%112 = OpCompositeExtract %float %111 0
%113 = OpCompositeExtract %float %111 1
%114 = OpCompositeConstruct %v4float %112 %113 %float_1_0 %float_0
OpStore %sk_FragColor %114
%115 = OpLoad %v4float %v
%116 = OpCompositeExtract %float %115 0
%117 = OpLoad %v4float %v
%118 = OpVectorShuffle %v2float %117 %117 2 3
%119 = OpCompositeExtract %float %118 0
%120 = OpCompositeExtract %float %118 1
%121 = OpCompositeConstruct %v4float %116 %float_1_0 %119 %120
OpStore %sk_FragColor %121
%122 = OpLoad %v4float %v
%123 = OpCompositeExtract %float %122 0
%124 = OpLoad %v4float %v
%125 = OpCompositeExtract %float %124 2
%126 = OpCompositeConstruct %v4float %123 %float_0 %125 %float_1_0
OpStore %sk_FragColor %126
%127 = OpLoad %v4float %v
%128 = OpCompositeExtract %float %127 0
%129 = OpLoad %v4float %v
%130 = OpCompositeExtract %float %129 3
%131 = OpCompositeConstruct %v4float %128 %float_1_0 %float_1_0 %130
OpStore %sk_FragColor %131
%132 = OpLoad %v4float %v
%133 = OpCompositeExtract %float %132 0
%134 = OpCompositeConstruct %v4float %133 %float_1_0 %float_0 %float_1_0
OpStore %sk_FragColor %134
%135 = OpLoad %v4float %v
%136 = OpVectorShuffle %v3float %135 %135 1 2 3
%137 = OpCompositeExtract %float %136 0
%138 = OpCompositeExtract %float %136 1
%139 = OpCompositeExtract %float %136 2
%140 = OpCompositeConstruct %v4float %float_1_0 %137 %138 %139
OpStore %sk_FragColor %140
%141 = OpLoad %v4float %v
%142 = OpVectorShuffle %v2float %141 %141 1 2
%143 = OpCompositeExtract %float %142 0
%144 = OpCompositeExtract %float %142 1
%145 = OpCompositeConstruct %v4float %float_0 %143 %144 %float_1_0
OpStore %sk_FragColor %145
%146 = OpLoad %v4float %v
%147 = OpCompositeExtract %float %146 1
%148 = OpLoad %v4float %v
%149 = OpCompositeExtract %float %148 3
%150 = OpCompositeConstruct %v4float %float_0 %147 %float_1_0 %149
OpStore %sk_FragColor %150
%151 = OpLoad %v4float %v
%152 = OpCompositeExtract %float %151 1
%153 = OpCompositeConstruct %v4float %float_1_0 %152 %float_1_0 %float_1_0
OpStore %sk_FragColor %153
%154 = OpLoad %v4float %v
%155 = OpVectorShuffle %v2float %154 %154 2 3
%156 = OpCompositeExtract %float %155 0
%157 = OpCompositeExtract %float %155 1
%158 = OpCompositeConstruct %v4float %float_0 %float_0 %156 %157
OpStore %sk_FragColor %158
%159 = OpLoad %v4float %v
%160 = OpCompositeExtract %float %159 2
%161 = OpCompositeConstruct %v4float %float_0 %float_0 %160 %float_1_0
OpStore %sk_FragColor %161
%162 = OpLoad %v4float %v
%163 = OpCompositeExtract %float %162 3
%164 = OpCompositeConstruct %v4float %float_0 %float_1_0 %float_1_0 %163
OpStore %sk_FragColor %164
OpReturn
OpFunctionEnd
