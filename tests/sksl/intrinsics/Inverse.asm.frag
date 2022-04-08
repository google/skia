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
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %inv3x3 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %inv4x4 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
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
%130 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%131 = OpConstantComposite %v3float %float_4 %float_5 %float_6
%132 = OpConstantComposite %v3float %float_7 %float_8 %float_9
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
%150 = OpVariable %_ptr_Function_v4float Function
%35 = OpCompositeConstruct %mat2v2float %33 %34
OpStore %inv2x2 %35
%51 = OpCompositeConstruct %mat3v3float %48 %49 %50
OpStore %inv3x3 %51
%64 = OpCompositeConstruct %mat4v4float %60 %61 %62 %63
OpStore %inv4x4 %64
%66 = OpCompositeConstruct %mat2v2float %33 %34
%67 = OpLoad %mat2v2float %inv2x2
%69 = OpCompositeExtract %v2float %66 0
%70 = OpCompositeExtract %v2float %67 0
%71 = OpFOrdEqual %v2bool %69 %70
%72 = OpAll %bool %71
%73 = OpCompositeExtract %v2float %66 1
%74 = OpCompositeExtract %v2float %67 1
%75 = OpFOrdEqual %v2bool %73 %74
%76 = OpAll %bool %75
%77 = OpLogicalAnd %bool %72 %76
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%80 = OpCompositeConstruct %mat3v3float %48 %49 %50
%81 = OpLoad %mat3v3float %inv3x3
%83 = OpCompositeExtract %v3float %80 0
%84 = OpCompositeExtract %v3float %81 0
%85 = OpFOrdEqual %v3bool %83 %84
%86 = OpAll %bool %85
%87 = OpCompositeExtract %v3float %80 1
%88 = OpCompositeExtract %v3float %81 1
%89 = OpFOrdEqual %v3bool %87 %88
%90 = OpAll %bool %89
%91 = OpLogicalAnd %bool %86 %90
%92 = OpCompositeExtract %v3float %80 2
%93 = OpCompositeExtract %v3float %81 2
%94 = OpFOrdEqual %v3bool %92 %93
%95 = OpAll %bool %94
%96 = OpLogicalAnd %bool %91 %95
OpBranch %79
%79 = OpLabel
%97 = OpPhi %bool %false %25 %96 %78
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%100 = OpCompositeConstruct %mat4v4float %60 %61 %62 %63
%101 = OpLoad %mat4v4float %inv4x4
%103 = OpCompositeExtract %v4float %100 0
%104 = OpCompositeExtract %v4float %101 0
%105 = OpFOrdEqual %v4bool %103 %104
%106 = OpAll %bool %105
%107 = OpCompositeExtract %v4float %100 1
%108 = OpCompositeExtract %v4float %101 1
%109 = OpFOrdEqual %v4bool %107 %108
%110 = OpAll %bool %109
%111 = OpLogicalAnd %bool %106 %110
%112 = OpCompositeExtract %v4float %100 2
%113 = OpCompositeExtract %v4float %101 2
%114 = OpFOrdEqual %v4bool %112 %113
%115 = OpAll %bool %114
%116 = OpLogicalAnd %bool %111 %115
%117 = OpCompositeExtract %v4float %100 3
%118 = OpCompositeExtract %v4float %101 3
%119 = OpFOrdEqual %v4bool %117 %118
%120 = OpAll %bool %119
%121 = OpLogicalAnd %bool %116 %120
OpBranch %99
%99 = OpLabel
%122 = OpPhi %bool %false %79 %121 %98
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%133 = OpCompositeConstruct %mat3v3float %130 %131 %132
%125 = OpExtInst %mat3v3float %1 MatrixInverse %133
%134 = OpLoad %mat3v3float %inv3x3
%135 = OpCompositeExtract %v3float %125 0
%136 = OpCompositeExtract %v3float %134 0
%137 = OpFUnordNotEqual %v3bool %135 %136
%138 = OpAny %bool %137
%139 = OpCompositeExtract %v3float %125 1
%140 = OpCompositeExtract %v3float %134 1
%141 = OpFUnordNotEqual %v3bool %139 %140
%142 = OpAny %bool %141
%143 = OpLogicalOr %bool %138 %142
%144 = OpCompositeExtract %v3float %125 2
%145 = OpCompositeExtract %v3float %134 2
%146 = OpFUnordNotEqual %v3bool %144 %145
%147 = OpAny %bool %146
%148 = OpLogicalOr %bool %143 %147
OpBranch %124
%124 = OpLabel
%149 = OpPhi %bool %false %99 %148 %123
OpSelectionMerge %154 None
OpBranchConditional %149 %152 %153
%152 = OpLabel
%155 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%159 = OpLoad %v4float %155
OpStore %150 %159
OpBranch %154
%153 = OpLabel
%160 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%162 = OpLoad %v4float %160
OpStore %150 %162
OpBranch %154
%154 = OpLabel
%163 = OpLoad %v4float %150
OpReturnValue %163
OpFunctionEnd
