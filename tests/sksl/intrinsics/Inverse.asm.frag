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
OpDecorate %35 RelaxedPrecision
OpDecorate %inv3x3 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %inv4x4 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
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
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%127 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%128 = OpConstantComposite %v3float %float_4 %float_5 %float_6
%129 = OpConstantComposite %v3float %float_7 %float_8 %float_9
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
%147 = OpVariable %_ptr_Function_v4float Function
%35 = OpCompositeConstruct %mat2v2float %33 %34
OpStore %inv2x2 %35
%51 = OpCompositeConstruct %mat3v3float %48 %49 %50
OpStore %inv3x3 %51
%64 = OpCompositeConstruct %mat4v4float %60 %61 %62 %63
OpStore %inv4x4 %64
%66 = OpLoad %mat2v2float %inv2x2
%68 = OpCompositeExtract %v2float %35 0
%69 = OpCompositeExtract %v2float %66 0
%70 = OpFOrdEqual %v2bool %68 %69
%71 = OpAll %bool %70
%72 = OpCompositeExtract %v2float %35 1
%73 = OpCompositeExtract %v2float %66 1
%74 = OpFOrdEqual %v2bool %72 %73
%75 = OpAll %bool %74
%76 = OpLogicalAnd %bool %71 %75
OpSelectionMerge %78 None
OpBranchConditional %76 %77 %78
%77 = OpLabel
%79 = OpLoad %mat3v3float %inv3x3
%81 = OpCompositeExtract %v3float %51 0
%82 = OpCompositeExtract %v3float %79 0
%83 = OpFOrdEqual %v3bool %81 %82
%84 = OpAll %bool %83
%85 = OpCompositeExtract %v3float %51 1
%86 = OpCompositeExtract %v3float %79 1
%87 = OpFOrdEqual %v3bool %85 %86
%88 = OpAll %bool %87
%89 = OpLogicalAnd %bool %84 %88
%90 = OpCompositeExtract %v3float %51 2
%91 = OpCompositeExtract %v3float %79 2
%92 = OpFOrdEqual %v3bool %90 %91
%93 = OpAll %bool %92
%94 = OpLogicalAnd %bool %89 %93
OpBranch %78
%78 = OpLabel
%95 = OpPhi %bool %false %25 %94 %77
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%98 = OpLoad %mat4v4float %inv4x4
%100 = OpCompositeExtract %v4float %64 0
%101 = OpCompositeExtract %v4float %98 0
%102 = OpFOrdEqual %v4bool %100 %101
%103 = OpAll %bool %102
%104 = OpCompositeExtract %v4float %64 1
%105 = OpCompositeExtract %v4float %98 1
%106 = OpFOrdEqual %v4bool %104 %105
%107 = OpAll %bool %106
%108 = OpLogicalAnd %bool %103 %107
%109 = OpCompositeExtract %v4float %64 2
%110 = OpCompositeExtract %v4float %98 2
%111 = OpFOrdEqual %v4bool %109 %110
%112 = OpAll %bool %111
%113 = OpLogicalAnd %bool %108 %112
%114 = OpCompositeExtract %v4float %64 3
%115 = OpCompositeExtract %v4float %98 3
%116 = OpFOrdEqual %v4bool %114 %115
%117 = OpAll %bool %116
%118 = OpLogicalAnd %bool %113 %117
OpBranch %97
%97 = OpLabel
%119 = OpPhi %bool %false %78 %118 %96
OpSelectionMerge %121 None
OpBranchConditional %119 %120 %121
%120 = OpLabel
%130 = OpCompositeConstruct %mat3v3float %127 %128 %129
%122 = OpExtInst %mat3v3float %1 MatrixInverse %130
%131 = OpLoad %mat3v3float %inv3x3
%132 = OpCompositeExtract %v3float %122 0
%133 = OpCompositeExtract %v3float %131 0
%134 = OpFUnordNotEqual %v3bool %132 %133
%135 = OpAny %bool %134
%136 = OpCompositeExtract %v3float %122 1
%137 = OpCompositeExtract %v3float %131 1
%138 = OpFUnordNotEqual %v3bool %136 %137
%139 = OpAny %bool %138
%140 = OpLogicalOr %bool %135 %139
%141 = OpCompositeExtract %v3float %122 2
%142 = OpCompositeExtract %v3float %131 2
%143 = OpFUnordNotEqual %v3bool %141 %142
%144 = OpAny %bool %143
%145 = OpLogicalOr %bool %140 %144
OpBranch %121
%121 = OpLabel
%146 = OpPhi %bool %false %97 %145 %120
OpSelectionMerge %151 None
OpBranchConditional %146 %149 %150
%149 = OpLabel
%152 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%156 = OpLoad %v4float %152
OpStore %147 %156
OpBranch %151
%150 = OpLabel
%157 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%159 = OpLoad %v4float %157
OpStore %147 %159
OpBranch %151
%151 = OpLabel
%160 = OpLoad %v4float %147
OpReturnValue %160
OpFunctionEnd
