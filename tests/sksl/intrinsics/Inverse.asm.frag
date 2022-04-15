OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %inv2x2 "inv2x2"
OpName %inv3x3 "inv3x3"
OpName %inv4x4 "inv4x4"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %inv2x2 RelaxedPrecision
OpDecorate %inv3x3 RelaxedPrecision
OpDecorate %inv4x4 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_n2 = OpConstant %float -2
%float_1 = OpConstant %float 1
%float_1_5 = OpConstant %float 1.5
%float_n0_5 = OpConstant %float -0.5
%33 = OpConstantComposite %v2float %float_n2 %float_1
%34 = OpConstantComposite %v2float %float_1_5 %float_n0_5
%35 = OpConstantComposite %mat2v2float %33 %34
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_n24 = OpConstant %float -24
%float_18 = OpConstant %float 18
%float_5 = OpConstant %float 5
%float_20 = OpConstant %float 20
%float_n15 = OpConstant %float -15
%float_n4 = OpConstant %float -4
%float_n5 = OpConstant %float -5
%float_4 = OpConstant %float 4
%48 = OpConstantComposite %v3float %float_n24 %float_18 %float_5
%49 = OpConstantComposite %v3float %float_20 %float_n15 %float_n4
%50 = OpConstantComposite %v3float %float_n5 %float_4 %float_1
%51 = OpConstantComposite %mat3v3float %48 %49 %50
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_0_5 = OpConstant %float 0.5
%float_n8 = OpConstant %float -8
%float_n1 = OpConstant %float -1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%60 = OpConstantComposite %v4float %float_n2 %float_n0_5 %float_1 %float_0_5
%61 = OpConstantComposite %v4float %float_1 %float_0_5 %float_0 %float_n0_5
%62 = OpConstantComposite %v4float %float_n8 %float_n1 %float_2 %float_2
%63 = OpConstantComposite %v4float %float_3 %float_0_5 %float_n1 %float_n0_5
%64 = OpConstantComposite %mat4v4float %60 %61 %62 %63
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%118 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%119 = OpConstantComposite %v3float %float_4 %float_5 %float_6
%120 = OpConstantComposite %v3float %float_7 %float_8 %float_9
%121 = OpConstantComposite %mat3v3float %118 %119 %120
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
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
%inv2x2 = OpVariable %_ptr_Function_mat2v2float Function
%inv3x3 = OpVariable %_ptr_Function_mat3v3float Function
%inv4x4 = OpVariable %_ptr_Function_mat4v4float Function
%138 = OpVariable %_ptr_Function_v4float Function
OpStore %inv2x2 %35
OpStore %inv3x3 %51
OpStore %inv4x4 %64
%66 = OpLoad %mat2v2float %inv2x2
%68 = OpCompositeExtract %v2float %66 0
%69 = OpFOrdEqual %v2bool %33 %68
%70 = OpAll %bool %69
%71 = OpCompositeExtract %v2float %66 1
%72 = OpFOrdEqual %v2bool %34 %71
%73 = OpAll %bool %72
%74 = OpLogicalAnd %bool %70 %73
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%77 = OpLoad %mat3v3float %inv3x3
%79 = OpCompositeExtract %v3float %77 0
%80 = OpFOrdEqual %v3bool %48 %79
%81 = OpAll %bool %80
%82 = OpCompositeExtract %v3float %77 1
%83 = OpFOrdEqual %v3bool %49 %82
%84 = OpAll %bool %83
%85 = OpLogicalAnd %bool %81 %84
%86 = OpCompositeExtract %v3float %77 2
%87 = OpFOrdEqual %v3bool %50 %86
%88 = OpAll %bool %87
%89 = OpLogicalAnd %bool %85 %88
OpBranch %76
%76 = OpLabel
%90 = OpPhi %bool %false %25 %89 %75
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%93 = OpLoad %mat4v4float %inv4x4
%95 = OpCompositeExtract %v4float %93 0
%96 = OpFOrdEqual %v4bool %60 %95
%97 = OpAll %bool %96
%98 = OpCompositeExtract %v4float %93 1
%99 = OpFOrdEqual %v4bool %61 %98
%100 = OpAll %bool %99
%101 = OpLogicalAnd %bool %97 %100
%102 = OpCompositeExtract %v4float %93 2
%103 = OpFOrdEqual %v4bool %62 %102
%104 = OpAll %bool %103
%105 = OpLogicalAnd %bool %101 %104
%106 = OpCompositeExtract %v4float %93 3
%107 = OpFOrdEqual %v4bool %63 %106
%108 = OpAll %bool %107
%109 = OpLogicalAnd %bool %105 %108
OpBranch %92
%92 = OpLabel
%110 = OpPhi %bool %false %76 %109 %91
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%113 = OpExtInst %mat3v3float %1 MatrixInverse %121
%122 = OpLoad %mat3v3float %inv3x3
%123 = OpCompositeExtract %v3float %113 0
%124 = OpCompositeExtract %v3float %122 0
%125 = OpFUnordNotEqual %v3bool %123 %124
%126 = OpAny %bool %125
%127 = OpCompositeExtract %v3float %113 1
%128 = OpCompositeExtract %v3float %122 1
%129 = OpFUnordNotEqual %v3bool %127 %128
%130 = OpAny %bool %129
%131 = OpLogicalOr %bool %126 %130
%132 = OpCompositeExtract %v3float %113 2
%133 = OpCompositeExtract %v3float %122 2
%134 = OpFUnordNotEqual %v3bool %132 %133
%135 = OpAny %bool %134
%136 = OpLogicalOr %bool %131 %135
OpBranch %112
%112 = OpLabel
%137 = OpPhi %bool %false %92 %136 %111
OpSelectionMerge %142 None
OpBranchConditional %137 %140 %141
%140 = OpLabel
%143 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%147 = OpLoad %v4float %143
OpStore %138 %147
OpBranch %142
%141 = OpLabel
%148 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%150 = OpLoad %v4float %148
OpStore %138 %150
OpBranch %142
%142 = OpLabel
%151 = OpLoad %v4float %138
OpReturnValue %151
OpFunctionEnd
