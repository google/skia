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
OpMemberName %_UniformBuffer 4 "testInputs"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %c12 "c12"
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
OpMemberDecorate %_UniformBuffer 3 Offset 64
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpMemberDecorate %_UniformBuffer 4 Offset 112
OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %123 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
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
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float %mat3v3float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %v4float %_ptr_Function_v2float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%32 = OpConstantComposite %v2float %float_1 %float_2
%false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%int_0 = OpConstant %int 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%int_1 = OpConstant %int 1
%float_3 = OpConstant %float 3
%float_6 = OpConstant %float 6
%float_4 = OpConstant %float 4
%float_8 = OpConstant %float 8
%51 = OpConstantComposite %v2float %float_3 %float_6
%52 = OpConstantComposite %v2float %float_4 %float_8
%53 = OpConstantComposite %mat2v2float %51 %52
%v2bool = OpTypeVector %bool 2
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_3 = OpConstant %int 3
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
%float_12 = OpConstant %float 12
%float_5 = OpConstant %float 5
%float_10 = OpConstant %float 10
%float_15 = OpConstant %float 15
%float_18 = OpConstant %float 18
%79 = OpConstantComposite %v3float %float_4 %float_8 %float_12
%80 = OpConstantComposite %v3float %float_5 %float_10 %float_15
%81 = OpConstantComposite %v3float %float_6 %float_12 %float_18
%82 = OpConstantComposite %mat3v3float %79 %80 %81
%v3bool = OpTypeVector %bool 3
%mat3v2float = OpTypeMatrix %v2float 3
%106 = OpConstantComposite %v2float %float_5 %float_10
%107 = OpConstantComposite %v2float %float_6 %float_12
%108 = OpConstantComposite %mat3v2float %52 %106 %107
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_4 = OpConstant %int 4
%128 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_2
%mat4v4float = OpTypeMatrix %v4float 4
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%float_n2_5 = OpConstant %float -2.5
%float_1_5 = OpConstant %float 1.5
%float_4_5 = OpConstant %float 4.5
%136 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%137 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%138 = OpConstantComposite %v4float %float_n2_5 %float_0 %float_1_5 %float_4_5
%139 = OpConstantComposite %mat4v4float %136 %137 %137 %138
%v4bool = OpTypeVector %bool 4
%mat2v4float = OpTypeMatrix %v4float 2
%164 = OpConstantComposite %mat2v4float %136 %138
%mat4v2float = OpTypeMatrix %v2float 4
%180 = OpConstantComposite %v2float %float_n1_25 %float_n2_5
%181 = OpConstantComposite %v2float %float_0_75 %float_1_5
%182 = OpConstantComposite %v2float %float_2_25 %float_4_5
%183 = OpConstantComposite %mat4v2float %180 %22 %181 %182
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %26
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpLabel
%c12 = OpVariable %_ptr_Function_v2float Function
%200 = OpVariable %_ptr_Function_v4float Function
OpStore %c12 %32
%35 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%40 = OpAccessChain %_ptr_Uniform_v2float %35 %int_0
%42 = OpLoad %v2float %40
%43 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%45 = OpAccessChain %_ptr_Uniform_v2float %43 %int_1
%46 = OpLoad %v2float %45
%34 = OpOuterProduct %mat2v2float %42 %46
%55 = OpCompositeExtract %v2float %34 0
%56 = OpFOrdEqual %v2bool %55 %51
%57 = OpAll %bool %56
%58 = OpCompositeExtract %v2float %34 1
%59 = OpFOrdEqual %v2bool %58 %52
%60 = OpAll %bool %59
%61 = OpLogicalAnd %bool %57 %60
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%65 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%68 = OpAccessChain %_ptr_Uniform_v3float %65 %int_0
%70 = OpLoad %v3float %68
%71 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%72 = OpAccessChain %_ptr_Uniform_v3float %71 %int_1
%73 = OpLoad %v3float %72
%64 = OpOuterProduct %mat3v3float %70 %73
%84 = OpCompositeExtract %v3float %64 0
%85 = OpFOrdEqual %v3bool %84 %79
%86 = OpAll %bool %85
%87 = OpCompositeExtract %v3float %64 1
%88 = OpFOrdEqual %v3bool %87 %80
%89 = OpAll %bool %88
%90 = OpLogicalAnd %bool %86 %89
%91 = OpCompositeExtract %v3float %64 2
%92 = OpFOrdEqual %v3bool %91 %81
%93 = OpAll %bool %92
%94 = OpLogicalAnd %bool %90 %93
OpBranch %63
%63 = OpLabel
%95 = OpPhi %bool %false %28 %94 %62
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%99 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%100 = OpAccessChain %_ptr_Uniform_v2float %99 %int_0
%101 = OpLoad %v2float %100
%102 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%103 = OpAccessChain %_ptr_Uniform_v3float %102 %int_1
%104 = OpLoad %v3float %103
%98 = OpOuterProduct %mat3v2float %101 %104
%109 = OpCompositeExtract %v2float %98 0
%110 = OpFOrdEqual %v2bool %109 %52
%111 = OpAll %bool %110
%112 = OpCompositeExtract %v2float %98 1
%113 = OpFOrdEqual %v2bool %112 %106
%114 = OpAll %bool %113
%115 = OpLogicalAnd %bool %111 %114
%116 = OpCompositeExtract %v2float %98 2
%117 = OpFOrdEqual %v2bool %116 %107
%118 = OpAll %bool %117
%119 = OpLogicalAnd %bool %115 %118
OpBranch %97
%97 = OpLabel
%120 = OpPhi %bool %false %63 %119 %96
OpSelectionMerge %122 None
OpBranchConditional %120 %121 %122
%121 = OpLabel
%124 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%127 = OpLoad %v4float %124
%123 = OpOuterProduct %mat4v4float %127 %128
%141 = OpCompositeExtract %v4float %123 0
%142 = OpFOrdEqual %v4bool %141 %136
%143 = OpAll %bool %142
%144 = OpCompositeExtract %v4float %123 1
%145 = OpFOrdEqual %v4bool %144 %137
%146 = OpAll %bool %145
%147 = OpLogicalAnd %bool %143 %146
%148 = OpCompositeExtract %v4float %123 2
%149 = OpFOrdEqual %v4bool %148 %137
%150 = OpAll %bool %149
%151 = OpLogicalAnd %bool %147 %150
%152 = OpCompositeExtract %v4float %123 3
%153 = OpFOrdEqual %v4bool %152 %138
%154 = OpAll %bool %153
%155 = OpLogicalAnd %bool %151 %154
OpBranch %122
%122 = OpLabel
%156 = OpPhi %bool %false %97 %155 %121
OpSelectionMerge %158 None
OpBranchConditional %156 %157 %158
%157 = OpLabel
%160 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%161 = OpLoad %v4float %160
%162 = OpLoad %v2float %c12
%159 = OpOuterProduct %mat2v4float %161 %162
%165 = OpCompositeExtract %v4float %159 0
%166 = OpFOrdEqual %v4bool %165 %136
%167 = OpAll %bool %166
%168 = OpCompositeExtract %v4float %159 1
%169 = OpFOrdEqual %v4bool %168 %138
%170 = OpAll %bool %169
%171 = OpLogicalAnd %bool %167 %170
OpBranch %158
%158 = OpLabel
%172 = OpPhi %bool %false %122 %171 %157
OpSelectionMerge %174 None
OpBranchConditional %172 %173 %174
%173 = OpLabel
%176 = OpLoad %v2float %c12
%177 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%178 = OpLoad %v4float %177
%175 = OpOuterProduct %mat4v2float %176 %178
%184 = OpCompositeExtract %v2float %175 0
%185 = OpFOrdEqual %v2bool %184 %180
%186 = OpAll %bool %185
%187 = OpCompositeExtract %v2float %175 1
%188 = OpFOrdEqual %v2bool %187 %22
%189 = OpAll %bool %188
%190 = OpLogicalAnd %bool %186 %189
%191 = OpCompositeExtract %v2float %175 2
%192 = OpFOrdEqual %v2bool %191 %181
%193 = OpAll %bool %192
%194 = OpLogicalAnd %bool %190 %193
%195 = OpCompositeExtract %v2float %175 3
%196 = OpFOrdEqual %v2bool %195 %182
%197 = OpAll %bool %196
%198 = OpLogicalAnd %bool %194 %197
OpBranch %174
%174 = OpLabel
%199 = OpPhi %bool %false %158 %198 %173
OpSelectionMerge %204 None
OpBranchConditional %199 %202 %203
%202 = OpLabel
%205 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%206 = OpLoad %v4float %205
OpStore %200 %206
OpBranch %204
%203 = OpLabel
%207 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%208 = OpLoad %v4float %207
OpStore %200 %208
OpBranch %204
%204 = OpLabel
%209 = OpLoad %v4float %200
OpReturnValue %209
OpFunctionEnd
