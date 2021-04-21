OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %expectedA "expectedA"
OpName %clampLow "clampLow"
OpName %expectedB "expectedB"
OpName %clampHigh "clampHigh"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
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
OpDecorate %expectedA RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %clampLow RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %clampHigh RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n1 = OpConstant %float -1
%float_0_75 = OpConstant %float 0.75
%float_1 = OpConstant %float 1
%31 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_75 %float_1
%float_n2 = OpConstant %float -2
%34 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n2 %float_1
%float_0_5 = OpConstant %float 0.5
%float_2_25 = OpConstant %float 2.25
%38 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_5 %float_2_25
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%42 = OpConstantComposite %v4float %float_1 %float_2 %float_0_5 %float_3
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%expectedA = OpVariable %_ptr_Function_v4float Function
%clampLow = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4float Function
%clampHigh = OpVariable %_ptr_Function_v4float Function
%150 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %31
OpStore %clampLow %34
OpStore %expectedB %38
OpStore %clampHigh %42
%45 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%49 = OpLoad %v4float %45
%50 = OpCompositeExtract %float %49 0
%44 = OpExtInst %float %1 FClamp %50 %float_n1 %float_1
%51 = OpLoad %v4float %expectedA
%52 = OpCompositeExtract %float %51 0
%53 = OpFOrdEqual %bool %44 %52
OpSelectionMerge %55 None
OpBranchConditional %53 %54 %55
%54 = OpLabel
%57 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%58 = OpLoad %v4float %57
%59 = OpVectorShuffle %v2float %58 %58 0 1
%60 = OpCompositeConstruct %v2float %float_n1 %float_n1
%61 = OpCompositeConstruct %v2float %float_1 %float_1
%56 = OpExtInst %v2float %1 FClamp %59 %60 %61
%62 = OpLoad %v4float %expectedA
%63 = OpVectorShuffle %v2float %62 %62 0 1
%64 = OpFOrdEqual %v2bool %56 %63
%66 = OpAll %bool %64
OpBranch %55
%55 = OpLabel
%67 = OpPhi %bool %false %25 %66 %54
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%71 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%72 = OpLoad %v4float %71
%73 = OpVectorShuffle %v3float %72 %72 0 1 2
%75 = OpCompositeConstruct %v3float %float_n1 %float_n1 %float_n1
%76 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%70 = OpExtInst %v3float %1 FClamp %73 %75 %76
%77 = OpLoad %v4float %expectedA
%78 = OpVectorShuffle %v3float %77 %77 0 1 2
%79 = OpFOrdEqual %v3bool %70 %78
%81 = OpAll %bool %79
OpBranch %69
%69 = OpLabel
%82 = OpPhi %bool %false %55 %81 %68
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%86 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%87 = OpLoad %v4float %86
%88 = OpCompositeConstruct %v4float %float_n1 %float_n1 %float_n1 %float_n1
%89 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%85 = OpExtInst %v4float %1 FClamp %87 %88 %89
%90 = OpLoad %v4float %expectedA
%91 = OpFOrdEqual %v4bool %85 %90
%93 = OpAll %bool %91
OpBranch %84
%84 = OpLabel
%94 = OpPhi %bool %false %69 %93 %83
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%98 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%99 = OpLoad %v4float %98
%100 = OpCompositeExtract %float %99 0
%101 = OpLoad %v4float %clampLow
%102 = OpCompositeExtract %float %101 0
%103 = OpLoad %v4float %clampHigh
%104 = OpCompositeExtract %float %103 0
%97 = OpExtInst %float %1 FClamp %100 %102 %104
%105 = OpLoad %v4float %expectedB
%106 = OpCompositeExtract %float %105 0
%107 = OpFOrdEqual %bool %97 %106
OpBranch %96
%96 = OpLabel
%108 = OpPhi %bool %false %84 %107 %95
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%112 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%113 = OpLoad %v4float %112
%114 = OpVectorShuffle %v2float %113 %113 0 1
%115 = OpLoad %v4float %clampLow
%116 = OpVectorShuffle %v2float %115 %115 0 1
%117 = OpLoad %v4float %clampHigh
%118 = OpVectorShuffle %v2float %117 %117 0 1
%111 = OpExtInst %v2float %1 FClamp %114 %116 %118
%119 = OpLoad %v4float %expectedB
%120 = OpVectorShuffle %v2float %119 %119 0 1
%121 = OpFOrdEqual %v2bool %111 %120
%122 = OpAll %bool %121
OpBranch %110
%110 = OpLabel
%123 = OpPhi %bool %false %96 %122 %109
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%127 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%128 = OpLoad %v4float %127
%129 = OpVectorShuffle %v3float %128 %128 0 1 2
%130 = OpLoad %v4float %clampLow
%131 = OpVectorShuffle %v3float %130 %130 0 1 2
%132 = OpLoad %v4float %clampHigh
%133 = OpVectorShuffle %v3float %132 %132 0 1 2
%126 = OpExtInst %v3float %1 FClamp %129 %131 %133
%134 = OpLoad %v4float %expectedB
%135 = OpVectorShuffle %v3float %134 %134 0 1 2
%136 = OpFOrdEqual %v3bool %126 %135
%137 = OpAll %bool %136
OpBranch %125
%125 = OpLabel
%138 = OpPhi %bool %false %110 %137 %124
OpSelectionMerge %140 None
OpBranchConditional %138 %139 %140
%139 = OpLabel
%142 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%143 = OpLoad %v4float %142
%144 = OpLoad %v4float %clampLow
%145 = OpLoad %v4float %clampHigh
%141 = OpExtInst %v4float %1 FClamp %143 %144 %145
%146 = OpLoad %v4float %expectedB
%147 = OpFOrdEqual %v4bool %141 %146
%148 = OpAll %bool %147
OpBranch %140
%140 = OpLabel
%149 = OpPhi %bool %false %125 %148 %139
OpSelectionMerge %153 None
OpBranchConditional %149 %151 %152
%151 = OpLabel
%154 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%156 = OpLoad %v4float %154
OpStore %150 %156
OpBranch %153
%152 = OpLabel
%157 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%159 = OpLoad %v4float %157
OpStore %150 %159
OpBranch %153
%153 = OpLabel
%160 = OpLoad %v4float %150
OpReturnValue %160
OpFunctionEnd
