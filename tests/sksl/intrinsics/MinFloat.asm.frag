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
OpDecorate %expectedA RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
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
%float_n1_25 = OpConstant %float -1.25
%float_0_5 = OpConstant %float 0.5
%30 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_5 %float_0_5
%float_1 = OpConstant %float 1
%33 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0 %float_1
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%51 = OpConstantComposite %v2float %float_0_5 %float_0_5
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%65 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%v3bool = OpTypeVector %bool 3
%77 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
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
%expectedB = OpVariable %_ptr_Function_v4float Function
%136 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %30
OpStore %expectedB %33
%36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%40 = OpLoad %v4float %36
%41 = OpCompositeExtract %float %40 0
%35 = OpExtInst %float %1 FMin %41 %float_0_5
%42 = OpLoad %v4float %expectedA
%43 = OpCompositeExtract %float %42 0
%44 = OpFOrdEqual %bool %35 %43
OpSelectionMerge %46 None
OpBranchConditional %44 %45 %46
%45 = OpLabel
%48 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%49 = OpLoad %v4float %48
%50 = OpVectorShuffle %v2float %49 %49 0 1
%47 = OpExtInst %v2float %1 FMin %50 %51
%52 = OpLoad %v4float %expectedA
%53 = OpVectorShuffle %v2float %52 %52 0 1
%54 = OpFOrdEqual %v2bool %47 %53
%56 = OpAll %bool %54
OpBranch %46
%46 = OpLabel
%57 = OpPhi %bool %false %25 %56 %45
OpSelectionMerge %59 None
OpBranchConditional %57 %58 %59
%58 = OpLabel
%61 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%62 = OpLoad %v4float %61
%63 = OpVectorShuffle %v3float %62 %62 0 1 2
%60 = OpExtInst %v3float %1 FMin %63 %65
%66 = OpLoad %v4float %expectedA
%67 = OpVectorShuffle %v3float %66 %66 0 1 2
%68 = OpFOrdEqual %v3bool %60 %67
%70 = OpAll %bool %68
OpBranch %59
%59 = OpLabel
%71 = OpPhi %bool %false %46 %70 %58
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%75 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%76 = OpLoad %v4float %75
%74 = OpExtInst %v4float %1 FMin %76 %77
%78 = OpLoad %v4float %expectedA
%79 = OpFOrdEqual %v4bool %74 %78
%81 = OpAll %bool %79
OpBranch %73
%73 = OpLabel
%82 = OpPhi %bool %false %59 %81 %72
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%86 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%87 = OpLoad %v4float %86
%88 = OpCompositeExtract %float %87 0
%89 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%91 = OpLoad %v4float %89
%92 = OpCompositeExtract %float %91 0
%85 = OpExtInst %float %1 FMin %88 %92
%93 = OpLoad %v4float %expectedB
%94 = OpCompositeExtract %float %93 0
%95 = OpFOrdEqual %bool %85 %94
OpBranch %84
%84 = OpLabel
%96 = OpPhi %bool %false %73 %95 %83
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%100 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%101 = OpLoad %v4float %100
%102 = OpVectorShuffle %v2float %101 %101 0 1
%103 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%104 = OpLoad %v4float %103
%105 = OpVectorShuffle %v2float %104 %104 0 1
%99 = OpExtInst %v2float %1 FMin %102 %105
%106 = OpLoad %v4float %expectedB
%107 = OpVectorShuffle %v2float %106 %106 0 1
%108 = OpFOrdEqual %v2bool %99 %107
%109 = OpAll %bool %108
OpBranch %98
%98 = OpLabel
%110 = OpPhi %bool %false %84 %109 %97
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%114 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%115 = OpLoad %v4float %114
%116 = OpVectorShuffle %v3float %115 %115 0 1 2
%117 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%118 = OpLoad %v4float %117
%119 = OpVectorShuffle %v3float %118 %118 0 1 2
%113 = OpExtInst %v3float %1 FMin %116 %119
%120 = OpLoad %v4float %expectedB
%121 = OpVectorShuffle %v3float %120 %120 0 1 2
%122 = OpFOrdEqual %v3bool %113 %121
%123 = OpAll %bool %122
OpBranch %112
%112 = OpLabel
%124 = OpPhi %bool %false %98 %123 %111
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%128 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%129 = OpLoad %v4float %128
%130 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%131 = OpLoad %v4float %130
%127 = OpExtInst %v4float %1 FMin %129 %131
%132 = OpLoad %v4float %expectedB
%133 = OpFOrdEqual %v4bool %127 %132
%134 = OpAll %bool %133
OpBranch %126
%126 = OpLabel
%135 = OpPhi %bool %false %112 %134 %125
OpSelectionMerge %139 None
OpBranchConditional %135 %137 %138
%137 = OpLabel
%140 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%141 = OpLoad %v4float %140
OpStore %136 %141
OpBranch %139
%138 = OpLabel
%142 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%144 = OpLoad %v4float %142
OpStore %136 %144
OpBranch %139
%139 = OpLabel
%145 = OpLoad %v4float %136
OpReturnValue %145
OpFunctionEnd
