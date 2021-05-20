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
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
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
%false = OpConstantFalse %bool
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%int_1 = OpConstant %int 1
%52 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
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
%35 = OpAccessChain %_ptr_Function_v2float %29 %int_0
%36 = OpLoad %v2float %35
%37 = OpAccessChain %_ptr_Function_v2float %30 %int_0
%38 = OpLoad %v2float %37
%39 = OpFOrdEqual %v2bool %36 %38
%41 = OpAll %bool %39
OpSelectionMerge %43 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%45 = OpAccessChain %_ptr_Function_v2float %29 %int_1
%46 = OpLoad %v2float %45
%47 = OpAccessChain %_ptr_Function_v2float %30 %int_1
%48 = OpLoad %v2float %47
%49 = OpFOrdEqual %v2bool %46 %48
%50 = OpAll %bool %49
OpBranch %43
%43 = OpLabel
%51 = OpPhi %bool %false %31 %50 %42
OpReturnValue %51
OpFunctionEnd
%main = OpFunction %v4float None %52
%53 = OpFunctionParameter %_ptr_Function_v2float
%54 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%65 = OpVariable %_ptr_Function_mat2v2float Function
%73 = OpVariable %_ptr_Function_mat2v2float Function
%114 = OpVariable %_ptr_Function_mat2v2float Function
%119 = OpVariable %_ptr_Function_mat2v2float Function
%147 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%58 = OpLoad %bool %_0_ok
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%61 = OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
%64 = OpLoad %mat2v2float %61
OpStore %65 %64
%71 = OpCompositeConstruct %v2float %float_1 %float_2
%72 = OpCompositeConstruct %v2float %float_3 %float_4
%70 = OpCompositeConstruct %mat2v2float %71 %72
OpStore %73 %70
%74 = OpFunctionCall %bool %_mat2x2_equal_bh22h22 %65 %73
OpBranch %60
%60 = OpLabel
%75 = OpPhi %bool %false %54 %74 %59
OpStore %_0_ok %75
%76 = OpLoad %bool %_0_ok
OpSelectionMerge %78 None
OpBranchConditional %76 %77 %78
%77 = OpLabel
%79 = OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_3
%82 = OpLoad %mat3v3float %79
%89 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%90 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%91 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%88 = OpCompositeConstruct %mat3v3float %89 %90 %91
%93 = OpCompositeExtract %v3float %82 0
%94 = OpCompositeExtract %v3float %88 0
%95 = OpFOrdEqual %v3bool %93 %94
%96 = OpAll %bool %95
%97 = OpCompositeExtract %v3float %82 1
%98 = OpCompositeExtract %v3float %88 1
%99 = OpFOrdEqual %v3bool %97 %98
%100 = OpAll %bool %99
%101 = OpLogicalAnd %bool %96 %100
%102 = OpCompositeExtract %v3float %82 2
%103 = OpCompositeExtract %v3float %88 2
%104 = OpFOrdEqual %v3bool %102 %103
%105 = OpAll %bool %104
%106 = OpLogicalAnd %bool %101 %105
OpBranch %78
%78 = OpLabel
%107 = OpPhi %bool %false %60 %106 %77
OpStore %_0_ok %107
%108 = OpLoad %bool %_0_ok
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%112 = OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
%113 = OpLoad %mat2v2float %112
OpStore %114 %113
%117 = OpCompositeConstruct %v2float %float_100 %float_0
%118 = OpCompositeConstruct %v2float %float_0 %float_100
%116 = OpCompositeConstruct %mat2v2float %117 %118
OpStore %119 %116
%120 = OpFunctionCall %bool %_mat2x2_equal_bh22h22 %114 %119
%111 = OpLogicalNot %bool %120
OpBranch %110
%110 = OpLabel
%121 = OpPhi %bool %false %78 %111 %109
OpStore %_0_ok %121
%122 = OpLoad %bool %_0_ok
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%125 = OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_3
%126 = OpLoad %mat3v3float %125
%128 = OpCompositeConstruct %v3float %float_9 %float_8 %float_7
%129 = OpCompositeConstruct %v3float %float_6 %float_5 %float_4
%130 = OpCompositeConstruct %v3float %float_3 %float_2 %float_1
%127 = OpCompositeConstruct %mat3v3float %128 %129 %130
%131 = OpCompositeExtract %v3float %126 0
%132 = OpCompositeExtract %v3float %127 0
%133 = OpFOrdNotEqual %v3bool %131 %132
%134 = OpAny %bool %133
%135 = OpCompositeExtract %v3float %126 1
%136 = OpCompositeExtract %v3float %127 1
%137 = OpFOrdNotEqual %v3bool %135 %136
%138 = OpAny %bool %137
%139 = OpLogicalOr %bool %134 %138
%140 = OpCompositeExtract %v3float %126 2
%141 = OpCompositeExtract %v3float %127 2
%142 = OpFOrdNotEqual %v3bool %140 %141
%143 = OpAny %bool %142
%144 = OpLogicalOr %bool %139 %143
OpBranch %124
%124 = OpLabel
%145 = OpPhi %bool %false %110 %144 %123
OpStore %_0_ok %145
%146 = OpLoad %bool %_0_ok
OpSelectionMerge %151 None
OpBranchConditional %146 %149 %150
%149 = OpLabel
%152 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%154 = OpLoad %v4float %152
OpStore %147 %154
OpBranch %151
%150 = OpLabel
%155 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%156 = OpLoad %v4float %155
OpStore %147 %156
OpBranch %151
%151 = OpLabel
%157 = OpLoad %v4float %147
OpReturnValue %157
OpFunctionEnd
