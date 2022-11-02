OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorWhite"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpMemberName %_UniformBuffer 3 "testMatrix2x2"
OpMemberName %_UniformBuffer 4 "testMatrix3x3"
OpMemberName %_UniformBuffer 5 "testMatrix4x4"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_iscalar_b "test_iscalar_b"
OpName %x "x"
OpName %test_fvec_b "test_fvec_b"
OpName %x_0 "x"
OpName %test_ivec_b "test_ivec_b"
OpName %x_1 "x"
OpName %test_mat2_b "test_mat2_b"
OpName %x_2 "x"
OpName %test_mat3_b "test_mat3_b"
OpName %x_3 "x"
OpName %test_mat4_b "test_mat4_b"
OpName %x_4 "x"
OpName %main "main"
OpName %_0_x "_0_x"
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
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpMemberDecorate %_UniformBuffer 4 Offset 80
OpMemberDecorate %_UniformBuffer 4 ColMajor
OpMemberDecorate %_UniformBuffer 4 MatrixStride 16
OpMemberDecorate %_UniformBuffer 5 Offset 128
OpMemberDecorate %_UniformBuffer 5 ColMajor
OpMemberDecorate %_UniformBuffer 5 MatrixStride 16
OpDecorate %_UniformBuffer Block
OpDecorate %16 Binding 0
OpDecorate %16 DescriptorSet 0
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %x_0 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %mat2v2float %mat3v3float %mat4v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%16 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%26 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%29 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%33 = OpTypeFunction %bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_n1 = OpConstant %int -1
%float_n1 = OpConstant %float -1
%54 = OpConstantComposite %v2float %float_n1 %float_n1
%v2bool = OpTypeVector %bool 2
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%68 = OpConstantComposite %v2int %int_n1 %int_n1
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int_3 = OpConstant %int 3
%float_n2 = OpConstant %float -2
%float_n3 = OpConstant %float -3
%float_n4 = OpConstant %float -4
%86 = OpConstantComposite %v2float %float_n1 %float_n2
%87 = OpConstantComposite %v2float %float_n3 %float_n4
%88 = OpConstantComposite %mat2v2float %86 %87
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_4 = OpConstant %int 4
%float_n5 = OpConstant %float -5
%float_n6 = OpConstant %float -6
%float_n7 = OpConstant %float -7
%float_n8 = OpConstant %float -8
%float_n9 = OpConstant %float -9
%113 = OpConstantComposite %v3float %float_n1 %float_n2 %float_n3
%114 = OpConstantComposite %v3float %float_n4 %float_n5 %float_n6
%115 = OpConstantComposite %v3float %float_n7 %float_n8 %float_n9
%116 = OpConstantComposite %mat3v3float %113 %114 %115
%v3bool = OpTypeVector %bool 3
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%int_5 = OpConstant %int 5
%float_n10 = OpConstant %float -10
%float_n11 = OpConstant %float -11
%float_n12 = OpConstant %float -12
%float_n13 = OpConstant %float -13
%float_n14 = OpConstant %float -14
%float_n15 = OpConstant %float -15
%float_n16 = OpConstant %float -16
%149 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n3 %float_n4
%150 = OpConstantComposite %v4float %float_n5 %float_n6 %float_n7 %float_n8
%151 = OpConstantComposite %v4float %float_n9 %float_n10 %float_n11 %float_n12
%152 = OpConstantComposite %v4float %float_n13 %float_n14 %float_n15 %float_n16
%153 = OpConstantComposite %mat4v4float %149 %150 %151 %152
%v4bool = OpTypeVector %bool 4
%166 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%false = OpConstantFalse %bool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %26
%27 = OpLabel
%30 = OpVariable %_ptr_Function_v2float Function
OpStore %30 %29
%32 = OpFunctionCall %v4float %main %30
OpStore %sk_FragColor %32
OpReturn
OpFunctionEnd
%test_iscalar_b = OpFunction %bool None %33
%34 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%38 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%41 = OpLoad %v4float %38
%42 = OpCompositeExtract %float %41 0
%43 = OpConvertFToS %int %42
OpStore %x %43
%44 = OpSNegate %int %43
OpStore %x %44
%46 = OpIEqual %bool %44 %int_n1
OpReturnValue %46
OpFunctionEnd
%test_fvec_b = OpFunction %bool None %33
%47 = OpLabel
%x_0 = OpVariable %_ptr_Function_v2float Function
%49 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%50 = OpLoad %v4float %49
%51 = OpVectorShuffle %v2float %50 %50 0 1
OpStore %x_0 %51
%52 = OpFNegate %v2float %51
OpStore %x_0 %52
%55 = OpFOrdEqual %v2bool %52 %54
%57 = OpAll %bool %55
OpReturnValue %57
OpFunctionEnd
%test_ivec_b = OpFunction %bool None %33
%58 = OpLabel
%x_1 = OpVariable %_ptr_Function_v2int Function
%62 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%63 = OpLoad %v4float %62
%64 = OpCompositeExtract %float %63 0
%65 = OpConvertFToS %int %64
%66 = OpCompositeConstruct %v2int %65 %65
OpStore %x_1 %66
%67 = OpSNegate %v2int %66
OpStore %x_1 %67
%69 = OpIEqual %v2bool %67 %68
%70 = OpAll %bool %69
OpReturnValue %70
OpFunctionEnd
%test_mat2_b = OpFunction %bool None %33
%71 = OpLabel
%x_2 = OpVariable %_ptr_Function_mat2v2float Function
%74 = OpAccessChain %_ptr_Uniform_mat2v2float %16 %int_3
%77 = OpLoad %mat2v2float %74
OpStore %x_2 %77
%78 = OpCompositeExtract %v2float %77 0
%79 = OpFNegate %v2float %78
%80 = OpCompositeExtract %v2float %77 1
%81 = OpFNegate %v2float %80
%82 = OpCompositeConstruct %mat2v2float %79 %81
OpStore %x_2 %82
%89 = OpFOrdEqual %v2bool %79 %86
%90 = OpAll %bool %89
%91 = OpFOrdEqual %v2bool %81 %87
%92 = OpAll %bool %91
%93 = OpLogicalAnd %bool %90 %92
OpReturnValue %93
OpFunctionEnd
%test_mat3_b = OpFunction %bool None %33
%94 = OpLabel
%x_3 = OpVariable %_ptr_Function_mat3v3float Function
%97 = OpAccessChain %_ptr_Uniform_mat3v3float %16 %int_4
%100 = OpLoad %mat3v3float %97
OpStore %x_3 %100
%101 = OpCompositeExtract %v3float %100 0
%102 = OpFNegate %v3float %101
%103 = OpCompositeExtract %v3float %100 1
%104 = OpFNegate %v3float %103
%105 = OpCompositeExtract %v3float %100 2
%106 = OpFNegate %v3float %105
%107 = OpCompositeConstruct %mat3v3float %102 %104 %106
OpStore %x_3 %107
%118 = OpFOrdEqual %v3bool %102 %113
%119 = OpAll %bool %118
%120 = OpFOrdEqual %v3bool %104 %114
%121 = OpAll %bool %120
%122 = OpLogicalAnd %bool %119 %121
%123 = OpFOrdEqual %v3bool %106 %115
%124 = OpAll %bool %123
%125 = OpLogicalAnd %bool %122 %124
OpReturnValue %125
OpFunctionEnd
%test_mat4_b = OpFunction %bool None %33
%126 = OpLabel
%x_4 = OpVariable %_ptr_Function_mat4v4float Function
%129 = OpAccessChain %_ptr_Uniform_mat4v4float %16 %int_5
%132 = OpLoad %mat4v4float %129
OpStore %x_4 %132
%133 = OpCompositeExtract %v4float %132 0
%134 = OpFNegate %v4float %133
%135 = OpCompositeExtract %v4float %132 1
%136 = OpFNegate %v4float %135
%137 = OpCompositeExtract %v4float %132 2
%138 = OpFNegate %v4float %137
%139 = OpCompositeExtract %v4float %132 3
%140 = OpFNegate %v4float %139
%141 = OpCompositeConstruct %mat4v4float %134 %136 %138 %140
OpStore %x_4 %141
%155 = OpFOrdEqual %v4bool %134 %149
%156 = OpAll %bool %155
%157 = OpFOrdEqual %v4bool %136 %150
%158 = OpAll %bool %157
%159 = OpLogicalAnd %bool %156 %158
%160 = OpFOrdEqual %v4bool %138 %151
%161 = OpAll %bool %160
%162 = OpLogicalAnd %bool %159 %161
%163 = OpFOrdEqual %v4bool %140 %152
%164 = OpAll %bool %163
%165 = OpLogicalAnd %bool %162 %164
OpReturnValue %165
OpFunctionEnd
%main = OpFunction %v4float None %166
%167 = OpFunctionParameter %_ptr_Function_v2float
%168 = OpLabel
%_0_x = OpVariable %_ptr_Function_float Function
%201 = OpVariable %_ptr_Function_v4float Function
%171 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%172 = OpLoad %v4float %171
%173 = OpCompositeExtract %float %172 0
OpStore %_0_x %173
%174 = OpFNegate %float %173
OpStore %_0_x %174
%176 = OpFOrdEqual %bool %174 %float_n1
OpSelectionMerge %178 None
OpBranchConditional %176 %177 %178
%177 = OpLabel
%179 = OpFunctionCall %bool %test_iscalar_b
OpBranch %178
%178 = OpLabel
%180 = OpPhi %bool %false %168 %179 %177
OpSelectionMerge %182 None
OpBranchConditional %180 %181 %182
%181 = OpLabel
%183 = OpFunctionCall %bool %test_fvec_b
OpBranch %182
%182 = OpLabel
%184 = OpPhi %bool %false %178 %183 %181
OpSelectionMerge %186 None
OpBranchConditional %184 %185 %186
%185 = OpLabel
%187 = OpFunctionCall %bool %test_ivec_b
OpBranch %186
%186 = OpLabel
%188 = OpPhi %bool %false %182 %187 %185
OpSelectionMerge %190 None
OpBranchConditional %188 %189 %190
%189 = OpLabel
%191 = OpFunctionCall %bool %test_mat2_b
OpBranch %190
%190 = OpLabel
%192 = OpPhi %bool %false %186 %191 %189
OpSelectionMerge %194 None
OpBranchConditional %192 %193 %194
%193 = OpLabel
%195 = OpFunctionCall %bool %test_mat3_b
OpBranch %194
%194 = OpLabel
%196 = OpPhi %bool %false %190 %195 %193
OpSelectionMerge %198 None
OpBranchConditional %196 %197 %198
%197 = OpLabel
%199 = OpFunctionCall %bool %test_mat4_b
OpBranch %198
%198 = OpLabel
%200 = OpPhi %bool %false %194 %199 %197
OpSelectionMerge %205 None
OpBranchConditional %200 %203 %204
%203 = OpLabel
%206 = OpAccessChain %_ptr_Uniform_v4float %16 %int_1
%208 = OpLoad %v4float %206
OpStore %201 %208
OpBranch %205
%204 = OpLabel
%209 = OpAccessChain %_ptr_Uniform_v4float %16 %int_2
%211 = OpLoad %v4float %209
OpStore %201 %211
OpBranch %205
%205 = OpLabel
%212 = OpLoad %v4float %201
OpReturnValue %212
OpFunctionEnd
