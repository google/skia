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
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpMemberName %_UniformBuffer 3 "testMatrix3x3"
OpName %_entrypoint_v "_entrypoint_v"
OpName %_mat2x2_equal_bh22h22 "_mat2x2_equal_bh22h22"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 64
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float %mat3v3float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%20 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%27 = OpTypeFunction %bool %_ptr_Function_mat2v2float %_ptr_Function_mat2v2float
%v2bool = OpTypeVector %bool 2
%44 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_3 = OpConstant %int 3
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%v3bool = OpTypeVector %bool 3
%float_100 = OpConstant %float 100
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %20
%21 = OpLabel
%24 = OpVariable %_ptr_Function_v2float Function
OpStore %24 %23
%26 = OpFunctionCall %v4float %main %24
OpStore %sk_FragColor %26
OpReturn
OpFunctionEnd
%_mat2x2_equal_bh22h22 = OpFunction %bool None %27
%29 = OpFunctionParameter %_ptr_Function_mat2v2float
%30 = OpFunctionParameter %_ptr_Function_mat2v2float
%31 = OpLabel
%32 = OpLoad %mat2v2float %29
%33 = OpLoad %mat2v2float %30
%35 = OpCompositeExtract %v2float %32 0
%36 = OpCompositeExtract %v2float %33 0
%37 = OpFOrdEqual %v2bool %35 %36
%38 = OpAll %bool %37
%39 = OpCompositeExtract %v2float %32 1
%40 = OpCompositeExtract %v2float %33 1
%41 = OpFOrdEqual %v2bool %39 %40
%42 = OpAll %bool %41
%43 = OpLogicalAnd %bool %38 %42
OpReturnValue %43
OpFunctionEnd
%main = OpFunction %v4float None %44
%45 = OpFunctionParameter %_ptr_Function_v2float
%46 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%59 = OpVariable %_ptr_Function_mat2v2float Function
%67 = OpVariable %_ptr_Function_mat2v2float Function
%108 = OpVariable %_ptr_Function_mat2v2float Function
%113 = OpVariable %_ptr_Function_mat2v2float Function
%141 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%51 = OpLoad %bool %_0_ok
OpSelectionMerge %53 None
OpBranchConditional %51 %52 %53
%52 = OpLabel
%54 = OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
%58 = OpLoad %mat2v2float %54
OpStore %59 %58
%65 = OpCompositeConstruct %v2float %float_1 %float_2
%66 = OpCompositeConstruct %v2float %float_3 %float_4
%64 = OpCompositeConstruct %mat2v2float %65 %66
OpStore %67 %64
%68 = OpFunctionCall %bool %_mat2x2_equal_bh22h22 %59 %67
OpBranch %53
%53 = OpLabel
%69 = OpPhi %bool %false %46 %68 %52
OpStore %_0_ok %69
%70 = OpLoad %bool %_0_ok
OpSelectionMerge %72 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
%73 = OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_3
%76 = OpLoad %mat3v3float %73
%83 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%84 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%85 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%82 = OpCompositeConstruct %mat3v3float %83 %84 %85
%87 = OpCompositeExtract %v3float %76 0
%88 = OpCompositeExtract %v3float %82 0
%89 = OpFOrdEqual %v3bool %87 %88
%90 = OpAll %bool %89
%91 = OpCompositeExtract %v3float %76 1
%92 = OpCompositeExtract %v3float %82 1
%93 = OpFOrdEqual %v3bool %91 %92
%94 = OpAll %bool %93
%95 = OpLogicalAnd %bool %90 %94
%96 = OpCompositeExtract %v3float %76 2
%97 = OpCompositeExtract %v3float %82 2
%98 = OpFOrdEqual %v3bool %96 %97
%99 = OpAll %bool %98
%100 = OpLogicalAnd %bool %95 %99
OpBranch %72
%72 = OpLabel
%101 = OpPhi %bool %false %53 %100 %71
OpStore %_0_ok %101
%102 = OpLoad %bool %_0_ok
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%106 = OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
%107 = OpLoad %mat2v2float %106
OpStore %108 %107
%111 = OpCompositeConstruct %v2float %float_100 %float_0
%112 = OpCompositeConstruct %v2float %float_0 %float_100
%110 = OpCompositeConstruct %mat2v2float %111 %112
OpStore %113 %110
%114 = OpFunctionCall %bool %_mat2x2_equal_bh22h22 %108 %113
%105 = OpLogicalNot %bool %114
OpBranch %104
%104 = OpLabel
%115 = OpPhi %bool %false %72 %105 %103
OpStore %_0_ok %115
%116 = OpLoad %bool %_0_ok
OpSelectionMerge %118 None
OpBranchConditional %116 %117 %118
%117 = OpLabel
%119 = OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_3
%120 = OpLoad %mat3v3float %119
%122 = OpCompositeConstruct %v3float %float_9 %float_8 %float_7
%123 = OpCompositeConstruct %v3float %float_6 %float_5 %float_4
%124 = OpCompositeConstruct %v3float %float_3 %float_2 %float_1
%121 = OpCompositeConstruct %mat3v3float %122 %123 %124
%125 = OpCompositeExtract %v3float %120 0
%126 = OpCompositeExtract %v3float %121 0
%127 = OpFOrdNotEqual %v3bool %125 %126
%128 = OpAny %bool %127
%129 = OpCompositeExtract %v3float %120 1
%130 = OpCompositeExtract %v3float %121 1
%131 = OpFOrdNotEqual %v3bool %129 %130
%132 = OpAny %bool %131
%133 = OpLogicalOr %bool %128 %132
%134 = OpCompositeExtract %v3float %120 2
%135 = OpCompositeExtract %v3float %121 2
%136 = OpFOrdNotEqual %v3bool %134 %135
%137 = OpAny %bool %136
%138 = OpLogicalOr %bool %133 %137
OpBranch %118
%118 = OpLabel
%139 = OpPhi %bool %false %104 %138 %117
OpStore %_0_ok %139
%140 = OpLoad %bool %_0_ok
OpSelectionMerge %145 None
OpBranchConditional %140 %143 %144
%143 = OpLabel
%146 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%149 = OpLoad %v4float %146
OpStore %141 %149
OpBranch %145
%144 = OpLabel
%150 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%152 = OpLoad %v4float %150
OpStore %141 %152
OpBranch %145
%145 = OpLabel
%153 = OpLoad %v4float %141
OpReturnValue %153
OpFunctionEnd
