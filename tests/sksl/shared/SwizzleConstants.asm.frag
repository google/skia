OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %v "v"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%v3float = OpTypeVector %float 3
%145 = OpConstantComposite %v4float %float_0 %float_1 %float_1 %float_1
%v4bool = OpTypeVector %bool 4
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%149 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
OpStore %v %26
%27 = OpLoad %v4float %v
%28 = OpCompositeExtract %float %27 0
%30 = OpCompositeConstruct %v4float %28 %float_1 %float_1 %float_1
OpStore %v %30
%31 = OpLoad %v4float %v
%32 = OpVectorShuffle %v2float %31 %31 0 1
%34 = OpCompositeExtract %float %32 0
%35 = OpCompositeExtract %float %32 1
%36 = OpCompositeConstruct %v4float %34 %35 %float_1 %float_1
OpStore %v %36
%37 = OpLoad %v4float %v
%38 = OpCompositeExtract %float %37 0
%39 = OpCompositeConstruct %v4float %38 %float_1 %float_1 %float_1
OpStore %v %39
%41 = OpLoad %v4float %v
%42 = OpCompositeExtract %float %41 1
%43 = OpCompositeConstruct %v4float %float_0 %42 %float_1 %float_1
OpStore %v %43
%44 = OpLoad %v4float %v
%45 = OpVectorShuffle %v3float %44 %44 0 1 2
%47 = OpCompositeExtract %float %45 0
%48 = OpCompositeExtract %float %45 1
%49 = OpCompositeExtract %float %45 2
%50 = OpCompositeConstruct %v4float %47 %48 %49 %float_1
OpStore %v %50
%51 = OpLoad %v4float %v
%52 = OpVectorShuffle %v2float %51 %51 0 1
%53 = OpCompositeExtract %float %52 0
%54 = OpCompositeExtract %float %52 1
%55 = OpCompositeConstruct %v4float %53 %54 %float_1 %float_1
OpStore %v %55
%56 = OpLoad %v4float %v
%57 = OpCompositeExtract %float %56 0
%58 = OpLoad %v4float %v
%59 = OpCompositeExtract %float %58 2
%60 = OpCompositeConstruct %v4float %57 %float_0 %59 %float_1
OpStore %v %60
%61 = OpLoad %v4float %v
%62 = OpCompositeExtract %float %61 0
%63 = OpCompositeConstruct %v4float %62 %float_1 %float_0 %float_1
OpStore %v %63
%64 = OpLoad %v4float %v
%65 = OpVectorShuffle %v2float %64 %64 1 2
%66 = OpCompositeExtract %float %65 0
%67 = OpCompositeExtract %float %65 1
%68 = OpCompositeConstruct %v4float %float_1 %66 %67 %float_1
OpStore %v %68
%69 = OpLoad %v4float %v
%70 = OpCompositeExtract %float %69 1
%71 = OpCompositeConstruct %v4float %float_0 %70 %float_1 %float_1
OpStore %v %71
%72 = OpLoad %v4float %v
%73 = OpCompositeExtract %float %72 2
%74 = OpCompositeConstruct %v4float %float_1 %float_1 %73 %float_1
OpStore %v %74
%75 = OpLoad %v4float %v
OpStore %v %75
%76 = OpLoad %v4float %v
%77 = OpVectorShuffle %v3float %76 %76 0 1 2
%78 = OpCompositeExtract %float %77 0
%79 = OpCompositeExtract %float %77 1
%80 = OpCompositeExtract %float %77 2
%81 = OpCompositeConstruct %v4float %78 %79 %80 %float_1
OpStore %v %81
%82 = OpLoad %v4float %v
%83 = OpVectorShuffle %v2float %82 %82 0 1
%84 = OpCompositeExtract %float %83 0
%85 = OpCompositeExtract %float %83 1
%86 = OpLoad %v4float %v
%87 = OpCompositeExtract %float %86 3
%88 = OpCompositeConstruct %v4float %84 %85 %float_0 %87
OpStore %v %88
%89 = OpLoad %v4float %v
%90 = OpVectorShuffle %v2float %89 %89 0 1
%91 = OpCompositeExtract %float %90 0
%92 = OpCompositeExtract %float %90 1
%93 = OpCompositeConstruct %v4float %91 %92 %float_1 %float_0
OpStore %v %93
%94 = OpLoad %v4float %v
%95 = OpCompositeExtract %float %94 0
%96 = OpLoad %v4float %v
%97 = OpVectorShuffle %v2float %96 %96 2 3
%98 = OpCompositeExtract %float %97 0
%99 = OpCompositeExtract %float %97 1
%100 = OpCompositeConstruct %v4float %95 %float_1 %98 %99
OpStore %v %100
%101 = OpLoad %v4float %v
%102 = OpCompositeExtract %float %101 0
%103 = OpLoad %v4float %v
%104 = OpCompositeExtract %float %103 2
%105 = OpCompositeConstruct %v4float %102 %float_0 %104 %float_1
OpStore %v %105
%106 = OpLoad %v4float %v
%107 = OpCompositeExtract %float %106 0
%108 = OpLoad %v4float %v
%109 = OpCompositeExtract %float %108 3
%110 = OpCompositeConstruct %v4float %107 %float_1 %float_1 %109
OpStore %v %110
%111 = OpLoad %v4float %v
%112 = OpCompositeExtract %float %111 0
%113 = OpCompositeConstruct %v4float %112 %float_1 %float_0 %float_1
OpStore %v %113
%114 = OpLoad %v4float %v
%115 = OpVectorShuffle %v3float %114 %114 1 2 3
%116 = OpCompositeExtract %float %115 0
%117 = OpCompositeExtract %float %115 1
%118 = OpCompositeExtract %float %115 2
%119 = OpCompositeConstruct %v4float %float_1 %116 %117 %118
OpStore %v %119
%120 = OpLoad %v4float %v
%121 = OpVectorShuffle %v2float %120 %120 1 2
%122 = OpCompositeExtract %float %121 0
%123 = OpCompositeExtract %float %121 1
%124 = OpCompositeConstruct %v4float %float_0 %122 %123 %float_1
OpStore %v %124
%125 = OpLoad %v4float %v
%126 = OpCompositeExtract %float %125 1
%127 = OpLoad %v4float %v
%128 = OpCompositeExtract %float %127 3
%129 = OpCompositeConstruct %v4float %float_0 %126 %float_1 %128
OpStore %v %129
%130 = OpLoad %v4float %v
%131 = OpCompositeExtract %float %130 1
%132 = OpCompositeConstruct %v4float %float_1 %131 %float_1 %float_1
OpStore %v %132
%133 = OpLoad %v4float %v
%134 = OpVectorShuffle %v2float %133 %133 2 3
%135 = OpCompositeExtract %float %134 0
%136 = OpCompositeExtract %float %134 1
%137 = OpCompositeConstruct %v4float %float_0 %float_0 %135 %136
OpStore %v %137
%138 = OpLoad %v4float %v
%139 = OpCompositeExtract %float %138 2
%140 = OpCompositeConstruct %v4float %float_0 %float_0 %139 %float_1
OpStore %v %140
%141 = OpLoad %v4float %v
%142 = OpCompositeExtract %float %141 3
%143 = OpCompositeConstruct %v4float %float_0 %float_1 %float_1 %142
OpStore %v %143
%144 = OpLoad %v4float %v
%146 = OpFOrdEqual %v4bool %144 %145
%148 = OpAll %bool %146
OpSelectionMerge %152 None
OpBranchConditional %148 %150 %151
%150 = OpLabel
%153 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%155 = OpLoad %v4float %153
OpStore %149 %155
OpBranch %152
%151 = OpLabel
%156 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%158 = OpLoad %v4float %156
OpStore %149 %158
OpBranch %152
%152 = OpLabel
%159 = OpLoad %v4float %149
OpReturnValue %159
OpFunctionEnd
