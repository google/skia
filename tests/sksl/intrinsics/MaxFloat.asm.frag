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
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
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
%float_0_5 = OpConstant %float 0.5
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%25 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_75 %float_2_25
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%29 = OpConstantComposite %v4float %float_0 %float_1 %float_0_75 %float_2_25
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
%133 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %25
OpStore %expectedB %29
%32 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%36 = OpLoad %v4float %32
%37 = OpCompositeExtract %float %36 0
%31 = OpExtInst %float %1 FMax %37 %float_0_5
%38 = OpLoad %v4float %expectedA
%39 = OpCompositeExtract %float %38 0
%40 = OpFOrdEqual %bool %31 %39
OpSelectionMerge %42 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
%44 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%45 = OpLoad %v4float %44
%46 = OpVectorShuffle %v2float %45 %45 0 1
%48 = OpCompositeConstruct %v2float %float_0_5 %float_0_5
%43 = OpExtInst %v2float %1 FMax %46 %48
%49 = OpLoad %v4float %expectedA
%50 = OpVectorShuffle %v2float %49 %49 0 1
%51 = OpFOrdEqual %v2bool %43 %50
%53 = OpAll %bool %51
OpBranch %42
%42 = OpLabel
%54 = OpPhi %bool %false %19 %53 %41
OpSelectionMerge %56 None
OpBranchConditional %54 %55 %56
%55 = OpLabel
%58 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%59 = OpLoad %v4float %58
%60 = OpVectorShuffle %v3float %59 %59 0 1 2
%62 = OpCompositeConstruct %v3float %float_0_5 %float_0_5 %float_0_5
%57 = OpExtInst %v3float %1 FMax %60 %62
%63 = OpLoad %v4float %expectedA
%64 = OpVectorShuffle %v3float %63 %63 0 1 2
%65 = OpFOrdEqual %v3bool %57 %64
%67 = OpAll %bool %65
OpBranch %56
%56 = OpLabel
%68 = OpPhi %bool %false %42 %67 %55
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%72 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%73 = OpLoad %v4float %72
%74 = OpCompositeConstruct %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%71 = OpExtInst %v4float %1 FMax %73 %74
%75 = OpLoad %v4float %expectedA
%76 = OpFOrdEqual %v4bool %71 %75
%78 = OpAll %bool %76
OpBranch %70
%70 = OpLabel
%79 = OpPhi %bool %false %56 %78 %69
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%83 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%84 = OpLoad %v4float %83
%85 = OpCompositeExtract %float %84 0
%86 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%88 = OpLoad %v4float %86
%89 = OpCompositeExtract %float %88 0
%82 = OpExtInst %float %1 FMax %85 %89
%90 = OpLoad %v4float %expectedB
%91 = OpCompositeExtract %float %90 0
%92 = OpFOrdEqual %bool %82 %91
OpBranch %81
%81 = OpLabel
%93 = OpPhi %bool %false %70 %92 %80
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%97 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%98 = OpLoad %v4float %97
%99 = OpVectorShuffle %v2float %98 %98 0 1
%100 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%101 = OpLoad %v4float %100
%102 = OpVectorShuffle %v2float %101 %101 0 1
%96 = OpExtInst %v2float %1 FMax %99 %102
%103 = OpLoad %v4float %expectedB
%104 = OpVectorShuffle %v2float %103 %103 0 1
%105 = OpFOrdEqual %v2bool %96 %104
%106 = OpAll %bool %105
OpBranch %95
%95 = OpLabel
%107 = OpPhi %bool %false %81 %106 %94
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%111 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%112 = OpLoad %v4float %111
%113 = OpVectorShuffle %v3float %112 %112 0 1 2
%114 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%115 = OpLoad %v4float %114
%116 = OpVectorShuffle %v3float %115 %115 0 1 2
%110 = OpExtInst %v3float %1 FMax %113 %116
%117 = OpLoad %v4float %expectedB
%118 = OpVectorShuffle %v3float %117 %117 0 1 2
%119 = OpFOrdEqual %v3bool %110 %118
%120 = OpAll %bool %119
OpBranch %109
%109 = OpLabel
%121 = OpPhi %bool %false %95 %120 %108
OpSelectionMerge %123 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%125 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%126 = OpLoad %v4float %125
%127 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%128 = OpLoad %v4float %127
%124 = OpExtInst %v4float %1 FMax %126 %128
%129 = OpLoad %v4float %expectedB
%130 = OpFOrdEqual %v4bool %124 %129
%131 = OpAll %bool %130
OpBranch %123
%123 = OpLabel
%132 = OpPhi %bool %false %109 %131 %122
OpSelectionMerge %136 None
OpBranchConditional %132 %134 %135
%134 = OpLabel
%137 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%138 = OpLoad %v4float %137
OpStore %133 %138
OpBranch %136
%135 = OpLabel
%139 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%141 = OpLoad %v4float %139
OpStore %133 %141
OpBranch %136
%136 = OpLabel
%142 = OpLoad %v4float %133
OpReturnValue %142
OpFunctionEnd
