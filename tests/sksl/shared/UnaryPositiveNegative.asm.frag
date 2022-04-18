OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
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
OpName %negated "negated"
OpName %x_2 "x"
OpName %test_mat3_b "test_mat3_b"
OpName %negated_0 "negated"
OpName %x_3 "x"
OpName %test_mat4_b "test_mat4_b"
OpName %negated_1 "negated"
OpName %x_4 "x"
OpName %main "main"
OpName %_0_x "_0_x"
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
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
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
%float_n2 = OpConstant %float -2
%float_n3 = OpConstant %float -3
%float_n4 = OpConstant %float -4
%77 = OpConstantComposite %v2float %float_n1 %float_n2
%78 = OpConstantComposite %v2float %float_n3 %float_n4
%79 = OpConstantComposite %mat2v2float %77 %78
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int_3 = OpConstant %int 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_n5 = OpConstant %float -5
%float_n6 = OpConstant %float -6
%float_n7 = OpConstant %float -7
%float_n8 = OpConstant %float -8
%float_n9 = OpConstant %float -9
%103 = OpConstantComposite %v3float %float_n1 %float_n2 %float_n3
%104 = OpConstantComposite %v3float %float_n4 %float_n5 %float_n6
%105 = OpConstantComposite %v3float %float_n7 %float_n8 %float_n9
%106 = OpConstantComposite %mat3v3float %103 %104 %105
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_4 = OpConstant %int 4
%v3bool = OpTypeVector %bool 3
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_n10 = OpConstant %float -10
%float_n11 = OpConstant %float -11
%float_n12 = OpConstant %float -12
%float_n13 = OpConstant %float -13
%float_n14 = OpConstant %float -14
%float_n15 = OpConstant %float -15
%float_n16 = OpConstant %float -16
%138 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n3 %float_n4
%139 = OpConstantComposite %v4float %float_n5 %float_n6 %float_n7 %float_n8
%140 = OpConstantComposite %v4float %float_n9 %float_n10 %float_n11 %float_n12
%141 = OpConstantComposite %v4float %float_n13 %float_n14 %float_n15 %float_n16
%142 = OpConstantComposite %mat4v4float %138 %139 %140 %141
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%int_5 = OpConstant %int 5
%v4bool = OpTypeVector %bool 4
%169 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%negated = OpVariable %_ptr_Function_mat2v2float Function
%x_2 = OpVariable %_ptr_Function_mat2v2float Function
OpStore %negated %79
%81 = OpAccessChain %_ptr_Uniform_mat2v2float %16 %int_3
%84 = OpLoad %mat2v2float %81
OpStore %x_2 %84
%85 = OpCompositeExtract %v2float %84 0
%86 = OpFNegate %v2float %85
%87 = OpCompositeExtract %v2float %84 1
%88 = OpFNegate %v2float %87
%89 = OpCompositeConstruct %mat2v2float %86 %88
OpStore %x_2 %89
%90 = OpFOrdEqual %v2bool %86 %77
%91 = OpAll %bool %90
%92 = OpFOrdEqual %v2bool %88 %78
%93 = OpAll %bool %92
%94 = OpLogicalAnd %bool %91 %93
OpReturnValue %94
OpFunctionEnd
%test_mat3_b = OpFunction %bool None %33
%95 = OpLabel
%negated_0 = OpVariable %_ptr_Function_mat3v3float Function
%x_3 = OpVariable %_ptr_Function_mat3v3float Function
OpStore %negated_0 %106
%108 = OpAccessChain %_ptr_Uniform_mat3v3float %16 %int_4
%111 = OpLoad %mat3v3float %108
OpStore %x_3 %111
%112 = OpCompositeExtract %v3float %111 0
%113 = OpFNegate %v3float %112
%114 = OpCompositeExtract %v3float %111 1
%115 = OpFNegate %v3float %114
%116 = OpCompositeExtract %v3float %111 2
%117 = OpFNegate %v3float %116
%118 = OpCompositeConstruct %mat3v3float %113 %115 %117
OpStore %x_3 %118
%120 = OpFOrdEqual %v3bool %113 %103
%121 = OpAll %bool %120
%122 = OpFOrdEqual %v3bool %115 %104
%123 = OpAll %bool %122
%124 = OpLogicalAnd %bool %121 %123
%125 = OpFOrdEqual %v3bool %117 %105
%126 = OpAll %bool %125
%127 = OpLogicalAnd %bool %124 %126
OpReturnValue %127
OpFunctionEnd
%test_mat4_b = OpFunction %bool None %33
%128 = OpLabel
%negated_1 = OpVariable %_ptr_Function_mat4v4float Function
%x_4 = OpVariable %_ptr_Function_mat4v4float Function
OpStore %negated_1 %142
%144 = OpAccessChain %_ptr_Uniform_mat4v4float %16 %int_5
%147 = OpLoad %mat4v4float %144
OpStore %x_4 %147
%148 = OpCompositeExtract %v4float %147 0
%149 = OpFNegate %v4float %148
%150 = OpCompositeExtract %v4float %147 1
%151 = OpFNegate %v4float %150
%152 = OpCompositeExtract %v4float %147 2
%153 = OpFNegate %v4float %152
%154 = OpCompositeExtract %v4float %147 3
%155 = OpFNegate %v4float %154
%156 = OpCompositeConstruct %mat4v4float %149 %151 %153 %155
OpStore %x_4 %156
%158 = OpFOrdEqual %v4bool %149 %138
%159 = OpAll %bool %158
%160 = OpFOrdEqual %v4bool %151 %139
%161 = OpAll %bool %160
%162 = OpLogicalAnd %bool %159 %161
%163 = OpFOrdEqual %v4bool %153 %140
%164 = OpAll %bool %163
%165 = OpLogicalAnd %bool %162 %164
%166 = OpFOrdEqual %v4bool %155 %141
%167 = OpAll %bool %166
%168 = OpLogicalAnd %bool %165 %167
OpReturnValue %168
OpFunctionEnd
%main = OpFunction %v4float None %169
%170 = OpFunctionParameter %_ptr_Function_v2float
%171 = OpLabel
%_0_x = OpVariable %_ptr_Function_float Function
%204 = OpVariable %_ptr_Function_v4float Function
%174 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%175 = OpLoad %v4float %174
%176 = OpCompositeExtract %float %175 0
OpStore %_0_x %176
%177 = OpFNegate %float %176
OpStore %_0_x %177
%179 = OpFOrdEqual %bool %177 %float_n1
OpSelectionMerge %181 None
OpBranchConditional %179 %180 %181
%180 = OpLabel
%182 = OpFunctionCall %bool %test_iscalar_b
OpBranch %181
%181 = OpLabel
%183 = OpPhi %bool %false %171 %182 %180
OpSelectionMerge %185 None
OpBranchConditional %183 %184 %185
%184 = OpLabel
%186 = OpFunctionCall %bool %test_fvec_b
OpBranch %185
%185 = OpLabel
%187 = OpPhi %bool %false %181 %186 %184
OpSelectionMerge %189 None
OpBranchConditional %187 %188 %189
%188 = OpLabel
%190 = OpFunctionCall %bool %test_ivec_b
OpBranch %189
%189 = OpLabel
%191 = OpPhi %bool %false %185 %190 %188
OpSelectionMerge %193 None
OpBranchConditional %191 %192 %193
%192 = OpLabel
%194 = OpFunctionCall %bool %test_mat2_b
OpBranch %193
%193 = OpLabel
%195 = OpPhi %bool %false %189 %194 %192
OpSelectionMerge %197 None
OpBranchConditional %195 %196 %197
%196 = OpLabel
%198 = OpFunctionCall %bool %test_mat3_b
OpBranch %197
%197 = OpLabel
%199 = OpPhi %bool %false %193 %198 %196
OpSelectionMerge %201 None
OpBranchConditional %199 %200 %201
%200 = OpLabel
%202 = OpFunctionCall %bool %test_mat4_b
OpBranch %201
%201 = OpLabel
%203 = OpPhi %bool %false %197 %202 %200
OpSelectionMerge %208 None
OpBranchConditional %203 %206 %207
%206 = OpLabel
%209 = OpAccessChain %_ptr_Uniform_v4float %16 %int_1
%211 = OpLoad %v4float %209
OpStore %204 %211
OpBranch %208
%207 = OpLabel
%212 = OpAccessChain %_ptr_Uniform_v4float %16 %int_2
%214 = OpLoad %v4float %212
OpStore %204 %214
OpBranch %208
%208 = OpLabel
%215 = OpLoad %v4float %204
OpReturnValue %215
OpFunctionEnd
