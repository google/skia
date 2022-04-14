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
OpName %expected "expected"
OpName %sksl_synthetic_uniforms "sksl_synthetic_uniforms"
OpMemberName %sksl_synthetic_uniforms 0 "u_skRTFlip"
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
OpDecorate %expected RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpMemberDecorate %sksl_synthetic_uniforms 0 Offset 16384
OpDecorate %sksl_synthetic_uniforms Block
OpDecorate %37 Binding 0
OpDecorate %37 DescriptorSet 0
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%28 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%sksl_synthetic_uniforms = OpTypeStruct %v2float
%_ptr_Uniform_sksl_synthetic_uniforms = OpTypePointer Uniform %sksl_synthetic_uniforms
%37 = OpVariable %_ptr_Uniform_sksl_synthetic_uniforms Uniform
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%float_1 = OpConstant %float 1
%124 = OpConstantComposite %v2float %float_1 %float_1
%138 = OpConstantComposite %v2float %float_0 %float_1
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
%expected = OpVariable %_ptr_Function_v4float Function
%142 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %28
%31 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%35 = OpLoad %v4float %31
%36 = OpCompositeExtract %float %35 0
%30 = OpDPdy %float %36
%40 = OpAccessChain %_ptr_Uniform_v2float %37 %int_0
%42 = OpLoad %v2float %40
%43 = OpCompositeExtract %float %42 1
%44 = OpFMul %float %30 %43
%45 = OpLoad %v4float %expected
%46 = OpCompositeExtract %float %45 0
%47 = OpFOrdEqual %bool %44 %46
OpSelectionMerge %49 None
OpBranchConditional %47 %48 %49
%48 = OpLabel
%51 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%52 = OpLoad %v4float %51
%53 = OpVectorShuffle %v2float %52 %52 0 1
%50 = OpDPdy %v2float %53
%54 = OpAccessChain %_ptr_Uniform_v2float %37 %int_0
%55 = OpLoad %v2float %54
%56 = OpCompositeExtract %float %55 1
%57 = OpCompositeConstruct %v2float %56 %56
%58 = OpFMul %v2float %50 %57
%59 = OpLoad %v4float %expected
%60 = OpVectorShuffle %v2float %59 %59 0 1
%61 = OpFOrdEqual %v2bool %58 %60
%63 = OpAll %bool %61
OpBranch %49
%49 = OpLabel
%64 = OpPhi %bool %false %25 %63 %48
OpSelectionMerge %66 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%68 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%69 = OpLoad %v4float %68
%70 = OpVectorShuffle %v3float %69 %69 0 1 2
%67 = OpDPdy %v3float %70
%72 = OpAccessChain %_ptr_Uniform_v2float %37 %int_0
%73 = OpLoad %v2float %72
%74 = OpCompositeExtract %float %73 1
%75 = OpCompositeConstruct %v3float %74 %74 %74
%76 = OpFMul %v3float %67 %75
%77 = OpLoad %v4float %expected
%78 = OpVectorShuffle %v3float %77 %77 0 1 2
%79 = OpFOrdEqual %v3bool %76 %78
%81 = OpAll %bool %79
OpBranch %66
%66 = OpLabel
%82 = OpPhi %bool %false %49 %81 %65
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%86 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%87 = OpLoad %v4float %86
%85 = OpDPdy %v4float %87
%88 = OpAccessChain %_ptr_Uniform_v2float %37 %int_0
%89 = OpLoad %v2float %88
%90 = OpCompositeExtract %float %89 1
%91 = OpCompositeConstruct %v4float %90 %90 %90 %90
%92 = OpFMul %v4float %85 %91
%93 = OpLoad %v4float %expected
%94 = OpFOrdEqual %v4bool %92 %93
%96 = OpAll %bool %94
OpBranch %84
%84 = OpLabel
%97 = OpPhi %bool %false %66 %96 %83
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%102 = OpLoad %v2float %24
%103 = OpVectorShuffle %v2float %102 %102 0 0
%101 = OpDPdy %v2float %103
%104 = OpAccessChain %_ptr_Uniform_v2float %37 %int_0
%105 = OpLoad %v2float %104
%106 = OpCompositeExtract %float %105 1
%107 = OpCompositeConstruct %v2float %106 %106
%108 = OpFMul %v2float %101 %107
%100 = OpExtInst %v2float %1 FSign %108
%109 = OpFOrdEqual %v2bool %100 %19
%110 = OpAll %bool %109
OpBranch %99
%99 = OpLabel
%111 = OpPhi %bool %false %84 %110 %98
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%116 = OpLoad %v2float %24
%117 = OpVectorShuffle %v2float %116 %116 1 1
%115 = OpDPdy %v2float %117
%118 = OpAccessChain %_ptr_Uniform_v2float %37 %int_0
%119 = OpLoad %v2float %118
%120 = OpCompositeExtract %float %119 1
%121 = OpCompositeConstruct %v2float %120 %120
%122 = OpFMul %v2float %115 %121
%114 = OpExtInst %v2float %1 FSign %122
%125 = OpFOrdEqual %v2bool %114 %124
%126 = OpAll %bool %125
OpBranch %113
%113 = OpLabel
%127 = OpPhi %bool %false %99 %126 %112
OpSelectionMerge %129 None
OpBranchConditional %127 %128 %129
%128 = OpLabel
%132 = OpLoad %v2float %24
%131 = OpDPdy %v2float %132
%133 = OpAccessChain %_ptr_Uniform_v2float %37 %int_0
%134 = OpLoad %v2float %133
%135 = OpCompositeExtract %float %134 1
%136 = OpCompositeConstruct %v2float %135 %135
%137 = OpFMul %v2float %131 %136
%130 = OpExtInst %v2float %1 FSign %137
%139 = OpFOrdEqual %v2bool %130 %138
%140 = OpAll %bool %139
OpBranch %129
%129 = OpLabel
%141 = OpPhi %bool %false %113 %140 %128
OpSelectionMerge %145 None
OpBranchConditional %141 %143 %144
%143 = OpLabel
%146 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%148 = OpLoad %v4float %146
OpStore %142 %148
OpBranch %145
%144 = OpLabel
%149 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%151 = OpLoad %v4float %149
OpStore %142 %151
OpBranch %145
%145 = OpLabel
%152 = OpLoad %v4float %142
OpReturnValue %152
OpFunctionEnd
