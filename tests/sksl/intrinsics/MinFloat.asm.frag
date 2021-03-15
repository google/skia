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
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
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
%132 = OpVariable %_ptr_Function_v4float Function
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
%73 = OpCompositeConstruct %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%70 = OpExtInst %v4float %1 FMin %72 %73
%74 = OpLoad %v4float %expectedA
%75 = OpFOrdEqual %v4bool %70 %74
%77 = OpAll %bool %75
OpBranch %69
%69 = OpLabel
%78 = OpPhi %bool %false %55 %77 %68
OpSelectionMerge %80 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
%82 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%83 = OpLoad %v4float %82
%84 = OpCompositeExtract %float %83 0
%85 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%87 = OpLoad %v4float %85
%88 = OpCompositeExtract %float %87 0
%81 = OpExtInst %float %1 FMin %84 %88
%89 = OpLoad %v4float %expectedB
%90 = OpCompositeExtract %float %89 0
%91 = OpFOrdEqual %bool %81 %90
OpBranch %80
%80 = OpLabel
%92 = OpPhi %bool %false %69 %91 %79
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%96 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%97 = OpLoad %v4float %96
%98 = OpVectorShuffle %v2float %97 %97 0 1
%99 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%100 = OpLoad %v4float %99
%101 = OpVectorShuffle %v2float %100 %100 0 1
%95 = OpExtInst %v2float %1 FMin %98 %101
%102 = OpLoad %v4float %expectedB
%103 = OpVectorShuffle %v2float %102 %102 0 1
%104 = OpFOrdEqual %v2bool %95 %103
%105 = OpAll %bool %104
OpBranch %94
%94 = OpLabel
%106 = OpPhi %bool %false %80 %105 %93
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%110 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%111 = OpLoad %v4float %110
%112 = OpVectorShuffle %v3float %111 %111 0 1 2
%113 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%114 = OpLoad %v4float %113
%115 = OpVectorShuffle %v3float %114 %114 0 1 2
%109 = OpExtInst %v3float %1 FMin %112 %115
%116 = OpLoad %v4float %expectedB
%117 = OpVectorShuffle %v3float %116 %116 0 1 2
%118 = OpFOrdEqual %v3bool %109 %117
%119 = OpAll %bool %118
OpBranch %108
%108 = OpLabel
%120 = OpPhi %bool %false %94 %119 %107
OpSelectionMerge %122 None
OpBranchConditional %120 %121 %122
%121 = OpLabel
%124 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%125 = OpLoad %v4float %124
%126 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%127 = OpLoad %v4float %126
%123 = OpExtInst %v4float %1 FMin %125 %127
%128 = OpLoad %v4float %expectedB
%129 = OpFOrdEqual %v4bool %123 %128
%130 = OpAll %bool %129
OpBranch %122
%122 = OpLabel
%131 = OpPhi %bool %false %108 %130 %121
OpSelectionMerge %135 None
OpBranchConditional %131 %133 %134
%133 = OpLabel
%136 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%137 = OpLoad %v4float %136
OpStore %132 %137
OpBranch %135
%134 = OpLabel
%138 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%140 = OpLoad %v4float %138
OpStore %132 %140
OpBranch %135
%135 = OpLabel
%141 = OpLoad %v4float %132
OpReturnValue %141
OpFunctionEnd
