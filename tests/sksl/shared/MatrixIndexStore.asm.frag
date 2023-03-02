OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "testMatrix3x3"
OpMemberName %_UniformBuffer 3 "testMatrix4x4"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test4x4_b "test4x4_b"
OpName %matrix "matrix"
OpName %values "values"
OpName %index "index"
OpName %main "main"
OpName %_0_matrix "_0_matrix"
OpName %_1_values "_1_values"
OpName %_2_index "_2_index"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 Offset 80
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %139 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat3v3float %mat4v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%27 = OpTypeFunction %bool
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%37 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_4 = OpConstant %int 4
%54 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
%int_1 = OpConstant %int 1
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%int_3 = OpConstant %int 3
%v4bool = OpTypeVector %bool 4
%84 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
%91 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%104 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%false = OpConstantFalse %bool
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_2 = OpConstant %int 2
%v3bool = OpTypeVector %bool 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%24 = OpVariable %_ptr_Function_v2float Function
OpStore %24 %23
%26 = OpFunctionCall %v4float %main %24
OpStore %sk_FragColor %26
OpReturn
OpFunctionEnd
%test4x4_b = OpFunction %bool None %27
%28 = OpLabel
%matrix = OpVariable %_ptr_Function_mat4v4float Function
%values = OpVariable %_ptr_Function_v4float Function
%index = OpVariable %_ptr_Function_int Function
OpStore %values %37
OpStore %index %int_0
OpBranch %42
%42 = OpLabel
OpLoopMerge %46 %45 None
OpBranch %43
%43 = OpLabel
%47 = OpLoad %int %index
%49 = OpSLessThan %bool %47 %int_4
OpBranchConditional %49 %44 %46
%44 = OpLabel
%50 = OpLoad %v4float %values
%51 = OpLoad %int %index
%52 = OpAccessChain %_ptr_Function_v4float %matrix %51
OpStore %52 %50
%53 = OpLoad %v4float %values
%55 = OpFAdd %v4float %53 %54
OpStore %values %55
OpBranch %45
%45 = OpLabel
%57 = OpLoad %int %index
%58 = OpIAdd %int %57 %int_1
OpStore %index %58
OpBranch %42
%46 = OpLabel
%59 = OpLoad %mat4v4float %matrix
%60 = OpAccessChain %_ptr_Uniform_mat4v4float %11 %int_3
%63 = OpLoad %mat4v4float %60
%65 = OpCompositeExtract %v4float %59 0
%66 = OpCompositeExtract %v4float %63 0
%67 = OpFOrdEqual %v4bool %65 %66
%68 = OpAll %bool %67
%69 = OpCompositeExtract %v4float %59 1
%70 = OpCompositeExtract %v4float %63 1
%71 = OpFOrdEqual %v4bool %69 %70
%72 = OpAll %bool %71
%73 = OpLogicalAnd %bool %68 %72
%74 = OpCompositeExtract %v4float %59 2
%75 = OpCompositeExtract %v4float %63 2
%76 = OpFOrdEqual %v4bool %74 %75
%77 = OpAll %bool %76
%78 = OpLogicalAnd %bool %73 %77
%79 = OpCompositeExtract %v4float %59 3
%80 = OpCompositeExtract %v4float %63 3
%81 = OpFOrdEqual %v4bool %79 %80
%82 = OpAll %bool %81
%83 = OpLogicalAnd %bool %78 %82
OpReturnValue %83
OpFunctionEnd
%main = OpFunction %v4float None %84
%85 = OpFunctionParameter %_ptr_Function_v2float
%86 = OpLabel
%_0_matrix = OpVariable %_ptr_Function_mat3v3float Function
%_1_values = OpVariable %_ptr_Function_v3float Function
%_2_index = OpVariable %_ptr_Function_int Function
%133 = OpVariable %_ptr_Function_v4float Function
OpStore %_1_values %91
OpStore %_2_index %int_0
OpBranch %93
%93 = OpLabel
OpLoopMerge %97 %96 None
OpBranch %94
%94 = OpLabel
%98 = OpLoad %int %_2_index
%99 = OpSLessThan %bool %98 %int_3
OpBranchConditional %99 %95 %97
%95 = OpLabel
%100 = OpLoad %v3float %_1_values
%101 = OpLoad %int %_2_index
%102 = OpAccessChain %_ptr_Function_v3float %_0_matrix %101
OpStore %102 %100
%103 = OpLoad %v3float %_1_values
%105 = OpFAdd %v3float %103 %104
OpStore %_1_values %105
OpBranch %96
%96 = OpLabel
%106 = OpLoad %int %_2_index
%107 = OpIAdd %int %106 %int_1
OpStore %_2_index %107
OpBranch %93
%97 = OpLabel
%109 = OpLoad %mat3v3float %_0_matrix
%110 = OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_2
%113 = OpLoad %mat3v3float %110
%115 = OpCompositeExtract %v3float %109 0
%116 = OpCompositeExtract %v3float %113 0
%117 = OpFOrdEqual %v3bool %115 %116
%118 = OpAll %bool %117
%119 = OpCompositeExtract %v3float %109 1
%120 = OpCompositeExtract %v3float %113 1
%121 = OpFOrdEqual %v3bool %119 %120
%122 = OpAll %bool %121
%123 = OpLogicalAnd %bool %118 %122
%124 = OpCompositeExtract %v3float %109 2
%125 = OpCompositeExtract %v3float %113 2
%126 = OpFOrdEqual %v3bool %124 %125
%127 = OpAll %bool %126
%128 = OpLogicalAnd %bool %123 %127
OpSelectionMerge %130 None
OpBranchConditional %128 %129 %130
%129 = OpLabel
%131 = OpFunctionCall %bool %test4x4_b
OpBranch %130
%130 = OpLabel
%132 = OpPhi %bool %false %97 %131 %129
OpSelectionMerge %136 None
OpBranchConditional %132 %134 %135
%134 = OpLabel
%137 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%139 = OpLoad %v4float %137
OpStore %133 %139
OpBranch %136
%135 = OpLabel
%140 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%141 = OpLoad %v4float %140
OpStore %133 %141
OpBranch %136
%136 = OpLabel
%142 = OpLoad %v4float %133
OpReturnValue %142
OpFunctionEnd
