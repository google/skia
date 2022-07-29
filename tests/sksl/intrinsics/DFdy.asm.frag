OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpMemberName %_UniformBuffer 3 "u_skRTFlip"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %expected "expected"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 16384
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %expected RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%v2float = OpTypeVector %float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%29 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_3 = OpConstant %int 3
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%float_1 = OpConstant %float 1
%118 = OpConstantComposite %v2float %float_1 %float_1
%132 = OpConstantComposite %v2float %float_0 %float_1
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %24
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%expected = OpVariable %_ptr_Function_v4float Function
%136 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %29
%32 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%36 = OpLoad %v4float %32
%37 = OpCompositeExtract %float %36 0
%31 = OpDPdy %float %37
%39 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%41 = OpLoad %v2float %39
%42 = OpCompositeExtract %float %41 1
%43 = OpFMul %float %31 %42
%44 = OpFOrdEqual %bool %43 %float_0
OpSelectionMerge %46 None
OpBranchConditional %44 %45 %46
%45 = OpLabel
%48 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%49 = OpLoad %v4float %48
%50 = OpVectorShuffle %v2float %49 %49 0 1
%47 = OpDPdy %v2float %50
%51 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%52 = OpLoad %v2float %51
%53 = OpCompositeExtract %float %52 1
%54 = OpCompositeConstruct %v2float %53 %53
%55 = OpFMul %v2float %47 %54
%56 = OpVectorShuffle %v2float %29 %29 0 1
%57 = OpFOrdEqual %v2bool %55 %56
%59 = OpAll %bool %57
OpBranch %46
%46 = OpLabel
%60 = OpPhi %bool %false %26 %59 %45
OpSelectionMerge %62 None
OpBranchConditional %60 %61 %62
%61 = OpLabel
%64 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%65 = OpLoad %v4float %64
%66 = OpVectorShuffle %v3float %65 %65 0 1 2
%63 = OpDPdy %v3float %66
%68 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%69 = OpLoad %v2float %68
%70 = OpCompositeExtract %float %69 1
%71 = OpCompositeConstruct %v3float %70 %70 %70
%72 = OpFMul %v3float %63 %71
%73 = OpVectorShuffle %v3float %29 %29 0 1 2
%74 = OpFOrdEqual %v3bool %72 %73
%76 = OpAll %bool %74
OpBranch %62
%62 = OpLabel
%77 = OpPhi %bool %false %46 %76 %61
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%81 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%82 = OpLoad %v4float %81
%80 = OpDPdy %v4float %82
%83 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%84 = OpLoad %v2float %83
%85 = OpCompositeExtract %float %84 1
%86 = OpCompositeConstruct %v4float %85 %85 %85 %85
%87 = OpFMul %v4float %80 %86
%88 = OpFOrdEqual %v4bool %87 %29
%90 = OpAll %bool %88
OpBranch %79
%79 = OpLabel
%91 = OpPhi %bool %false %62 %90 %78
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%96 = OpLoad %v2float %25
%97 = OpVectorShuffle %v2float %96 %96 0 0
%95 = OpDPdy %v2float %97
%98 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%99 = OpLoad %v2float %98
%100 = OpCompositeExtract %float %99 1
%101 = OpCompositeConstruct %v2float %100 %100
%102 = OpFMul %v2float %95 %101
%94 = OpExtInst %v2float %1 FSign %102
%103 = OpFOrdEqual %v2bool %94 %20
%104 = OpAll %bool %103
OpBranch %93
%93 = OpLabel
%105 = OpPhi %bool %false %79 %104 %92
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%110 = OpLoad %v2float %25
%111 = OpVectorShuffle %v2float %110 %110 1 1
%109 = OpDPdy %v2float %111
%112 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%113 = OpLoad %v2float %112
%114 = OpCompositeExtract %float %113 1
%115 = OpCompositeConstruct %v2float %114 %114
%116 = OpFMul %v2float %109 %115
%108 = OpExtInst %v2float %1 FSign %116
%119 = OpFOrdEqual %v2bool %108 %118
%120 = OpAll %bool %119
OpBranch %107
%107 = OpLabel
%121 = OpPhi %bool %false %93 %120 %106
OpSelectionMerge %123 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%126 = OpLoad %v2float %25
%125 = OpDPdy %v2float %126
%127 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%128 = OpLoad %v2float %127
%129 = OpCompositeExtract %float %128 1
%130 = OpCompositeConstruct %v2float %129 %129
%131 = OpFMul %v2float %125 %130
%124 = OpExtInst %v2float %1 FSign %131
%133 = OpFOrdEqual %v2bool %124 %132
%134 = OpAll %bool %133
OpBranch %123
%123 = OpLabel
%135 = OpPhi %bool %false %107 %134 %122
OpSelectionMerge %139 None
OpBranchConditional %135 %137 %138
%137 = OpLabel
%140 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%142 = OpLoad %v4float %140
OpStore %136 %142
OpBranch %139
%138 = OpLabel
%143 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%145 = OpLoad %v4float %143
OpStore %136 %145
OpBranch %139
%139 = OpLabel
%146 = OpLoad %v4float %136
OpReturnValue %146
OpFunctionEnd
