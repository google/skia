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
OpDecorate %31 RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0_5 = OpConstant %float 0.5
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%31 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_75 %float_2_25
%float_1 = OpConstant %float 1
%34 = OpConstantComposite %v4float %float_0 %float_1 %float_0_75 %float_2_25
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
%expectedB = OpVariable %_ptr_Function_v4float Function
%137 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedA %31
OpStore %expectedB %34
%37 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%41 = OpLoad %v4float %37
%42 = OpCompositeExtract %float %41 0
%36 = OpExtInst %float %1 FMax %42 %float_0_5
%43 = OpLoad %v4float %expectedA
%44 = OpCompositeExtract %float %43 0
%45 = OpFOrdEqual %bool %36 %44
OpSelectionMerge %47 None
OpBranchConditional %45 %46 %47
%46 = OpLabel
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%50 = OpLoad %v4float %49
%51 = OpVectorShuffle %v2float %50 %50 0 1
%52 = OpCompositeConstruct %v2float %float_0_5 %float_0_5
%48 = OpExtInst %v2float %1 FMax %51 %52
%53 = OpLoad %v4float %expectedA
%54 = OpVectorShuffle %v2float %53 %53 0 1
%55 = OpFOrdEqual %v2bool %48 %54
%57 = OpAll %bool %55
OpBranch %47
%47 = OpLabel
%58 = OpPhi %bool %false %25 %57 %46
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%62 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%63 = OpLoad %v4float %62
%64 = OpVectorShuffle %v3float %63 %63 0 1 2
%66 = OpCompositeConstruct %v3float %float_0_5 %float_0_5 %float_0_5
%61 = OpExtInst %v3float %1 FMax %64 %66
%67 = OpLoad %v4float %expectedA
%68 = OpVectorShuffle %v3float %67 %67 0 1 2
%69 = OpFOrdEqual %v3bool %61 %68
%71 = OpAll %bool %69
OpBranch %60
%60 = OpLabel
%72 = OpPhi %bool %false %47 %71 %59
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%77 = OpLoad %v4float %76
%78 = OpCompositeConstruct %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%75 = OpExtInst %v4float %1 FMax %77 %78
%79 = OpLoad %v4float %expectedA
%80 = OpFOrdEqual %v4bool %75 %79
%82 = OpAll %bool %80
OpBranch %74
%74 = OpLabel
%83 = OpPhi %bool %false %60 %82 %73
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%87 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%88 = OpLoad %v4float %87
%89 = OpCompositeExtract %float %88 0
%90 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%92 = OpLoad %v4float %90
%93 = OpCompositeExtract %float %92 0
%86 = OpExtInst %float %1 FMax %89 %93
%94 = OpLoad %v4float %expectedB
%95 = OpCompositeExtract %float %94 0
%96 = OpFOrdEqual %bool %86 %95
OpBranch %85
%85 = OpLabel
%97 = OpPhi %bool %false %74 %96 %84
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%101 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%102 = OpLoad %v4float %101
%103 = OpVectorShuffle %v2float %102 %102 0 1
%104 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%105 = OpLoad %v4float %104
%106 = OpVectorShuffle %v2float %105 %105 0 1
%100 = OpExtInst %v2float %1 FMax %103 %106
%107 = OpLoad %v4float %expectedB
%108 = OpVectorShuffle %v2float %107 %107 0 1
%109 = OpFOrdEqual %v2bool %100 %108
%110 = OpAll %bool %109
OpBranch %99
%99 = OpLabel
%111 = OpPhi %bool %false %85 %110 %98
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%115 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%116 = OpLoad %v4float %115
%117 = OpVectorShuffle %v3float %116 %116 0 1 2
%118 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%119 = OpLoad %v4float %118
%120 = OpVectorShuffle %v3float %119 %119 0 1 2
%114 = OpExtInst %v3float %1 FMax %117 %120
%121 = OpLoad %v4float %expectedB
%122 = OpVectorShuffle %v3float %121 %121 0 1 2
%123 = OpFOrdEqual %v3bool %114 %122
%124 = OpAll %bool %123
OpBranch %113
%113 = OpLabel
%125 = OpPhi %bool %false %99 %124 %112
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%129 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%130 = OpLoad %v4float %129
%131 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%132 = OpLoad %v4float %131
%128 = OpExtInst %v4float %1 FMax %130 %132
%133 = OpLoad %v4float %expectedB
%134 = OpFOrdEqual %v4bool %128 %133
%135 = OpAll %bool %134
OpBranch %127
%127 = OpLabel
%136 = OpPhi %bool %false %113 %135 %126
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
