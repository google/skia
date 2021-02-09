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
OpName %expectedA "expectedA"
OpName %expectedB "expectedB"
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
OpDecorate %35 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
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
%float_n1_25 = OpConstant %float -1.25
%float_0 = OpConstant %float 0
%float_0_5 = OpConstant %float 0.5
%25 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_5 %float_0_5
%float_1 = OpConstant %float 1
%28 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0 %float_1
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2float = OpTypeVector %float 2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
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
%expectedA = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4float Function
%137 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %25
OpStore %expectedB %28
%31 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%35 = OpLoad %v4float %31
%36 = OpCompositeExtract %float %35 0
%30 = OpExtInst %float %1 FMin %36 %float_0_5
%37 = OpLoad %v4float %expectedA
%38 = OpCompositeExtract %float %37 0
%39 = OpFOrdEqual %bool %30 %38
OpSelectionMerge %41 None
OpBranchConditional %39 %40 %41
%40 = OpLabel
%43 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%44 = OpLoad %v4float %43
%45 = OpVectorShuffle %v2float %44 %44 0 1
%47 = OpCompositeConstruct %v2float %float_0_5 %float_0_5
%42 = OpExtInst %v2float %1 FMin %45 %47
%48 = OpLoad %v4float %expectedA
%49 = OpVectorShuffle %v2float %48 %48 0 1
%50 = OpFOrdEqual %v2bool %42 %49
%52 = OpAll %bool %50
OpBranch %41
%41 = OpLabel
%53 = OpPhi %bool %false %19 %52 %40
OpSelectionMerge %55 None
OpBranchConditional %53 %54 %55
%54 = OpLabel
%57 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%58 = OpLoad %v4float %57
%59 = OpVectorShuffle %v3float %58 %58 0 1 2
%61 = OpCompositeConstruct %v3float %float_0_5 %float_0_5 %float_0_5
%56 = OpExtInst %v3float %1 FMin %59 %61
%62 = OpLoad %v4float %expectedA
%63 = OpVectorShuffle %v3float %62 %62 0 1 2
%64 = OpFOrdEqual %v3bool %56 %63
%66 = OpAll %bool %64
OpBranch %55
%55 = OpLabel
%67 = OpPhi %bool %false %41 %66 %54
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%71 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%72 = OpLoad %v4float %71
%73 = OpVectorShuffle %v4float %72 %72 0 1 2 3
%74 = OpCompositeConstruct %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%70 = OpExtInst %v4float %1 FMin %73 %74
%75 = OpLoad %v4float %expectedA
%76 = OpVectorShuffle %v4float %75 %75 0 1 2 3
%77 = OpFOrdEqual %v4bool %70 %76
%79 = OpAll %bool %77
OpBranch %69
%69 = OpLabel
%80 = OpPhi %bool %false %55 %79 %68
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%84 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%85 = OpLoad %v4float %84
%86 = OpCompositeExtract %float %85 0
%87 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%89 = OpLoad %v4float %87
%90 = OpCompositeExtract %float %89 0
%83 = OpExtInst %float %1 FMin %86 %90
%91 = OpLoad %v4float %expectedB
%92 = OpCompositeExtract %float %91 0
%93 = OpFOrdEqual %bool %83 %92
OpBranch %82
%82 = OpLabel
%94 = OpPhi %bool %false %69 %93 %81
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%98 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%99 = OpLoad %v4float %98
%100 = OpVectorShuffle %v2float %99 %99 0 1
%101 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%102 = OpLoad %v4float %101
%103 = OpVectorShuffle %v2float %102 %102 0 1
%97 = OpExtInst %v2float %1 FMin %100 %103
%104 = OpLoad %v4float %expectedB
%105 = OpVectorShuffle %v2float %104 %104 0 1
%106 = OpFOrdEqual %v2bool %97 %105
%107 = OpAll %bool %106
OpBranch %96
%96 = OpLabel
%108 = OpPhi %bool %false %82 %107 %95
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%112 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%113 = OpLoad %v4float %112
%114 = OpVectorShuffle %v3float %113 %113 0 1 2
%115 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%116 = OpLoad %v4float %115
%117 = OpVectorShuffle %v3float %116 %116 0 1 2
%111 = OpExtInst %v3float %1 FMin %114 %117
%118 = OpLoad %v4float %expectedB
%119 = OpVectorShuffle %v3float %118 %118 0 1 2
%120 = OpFOrdEqual %v3bool %111 %119
%121 = OpAll %bool %120
OpBranch %110
%110 = OpLabel
%122 = OpPhi %bool %false %96 %121 %109
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%126 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%127 = OpLoad %v4float %126
%128 = OpVectorShuffle %v4float %127 %127 0 1 2 3
%129 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%130 = OpLoad %v4float %129
%131 = OpVectorShuffle %v4float %130 %130 0 1 2 3
%125 = OpExtInst %v4float %1 FMin %128 %131
%132 = OpLoad %v4float %expectedB
%133 = OpVectorShuffle %v4float %132 %132 0 1 2 3
%134 = OpFOrdEqual %v4bool %125 %133
%135 = OpAll %bool %134
OpBranch %124
%124 = OpLabel
%136 = OpPhi %bool %false %110 %135 %123
OpSelectionMerge %140 None
OpBranchConditional %136 %138 %139
%138 = OpLabel
%141 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%142 = OpLoad %v4float %141
OpStore %137 %142
OpBranch %140
%139 = OpLabel
%143 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%145 = OpLoad %v4float %143
OpStore %137 %145
OpBranch %140
%140 = OpLabel
%146 = OpLoad %v4float %137
OpReturnValue %146
OpFunctionEnd
