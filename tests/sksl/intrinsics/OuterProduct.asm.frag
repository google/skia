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
OpDecorate %132 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
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
%v2bool = OpTypeVector %bool 2
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_3 = OpConstant %int 3
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
%float_12 = OpConstant %float 12
%float_5 = OpConstant %float 5
%float_10 = OpConstant %float 10
%float_15 = OpConstant %float 15
%float_18 = OpConstant %float 18
%v3bool = OpTypeVector %bool 3
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_4 = OpConstant %int 4
%137 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_2
%mat4v4float = OpTypeMatrix %v4float 4
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%float_n2_5 = OpConstant %float -2.5
%float_1_5 = OpConstant %float 1.5
%float_4_5 = OpConstant %float 4.5
%v4bool = OpTypeVector %bool 4
%mat2v4float = OpTypeMatrix %v4float 2
%mat4v2float = OpTypeMatrix %v2float 4
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
%223 = OpVariable %_ptr_Function_v4float Function
OpStore %c12 %32
%35 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%40 = OpAccessChain %_ptr_Uniform_v2float %35 %int_0
%42 = OpLoad %v2float %40
%43 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%45 = OpAccessChain %_ptr_Uniform_v2float %43 %int_1
%46 = OpLoad %v2float %45
%34 = OpOuterProduct %mat2v2float %42 %46
%51 = OpCompositeConstruct %v2float %float_3 %float_6
%52 = OpCompositeConstruct %v2float %float_4 %float_8
%53 = OpCompositeConstruct %mat2v2float %51 %52
%55 = OpCompositeExtract %v2float %34 0
%56 = OpCompositeExtract %v2float %53 0
%57 = OpFOrdEqual %v2bool %55 %56
%58 = OpAll %bool %57
%59 = OpCompositeExtract %v2float %34 1
%60 = OpCompositeExtract %v2float %53 1
%61 = OpFOrdEqual %v2bool %59 %60
%62 = OpAll %bool %61
%63 = OpLogicalAnd %bool %58 %62
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%67 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%70 = OpAccessChain %_ptr_Uniform_v3float %67 %int_0
%72 = OpLoad %v3float %70
%73 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%74 = OpAccessChain %_ptr_Uniform_v3float %73 %int_1
%75 = OpLoad %v3float %74
%66 = OpOuterProduct %mat3v3float %72 %75
%81 = OpCompositeConstruct %v3float %float_4 %float_8 %float_12
%82 = OpCompositeConstruct %v3float %float_5 %float_10 %float_15
%83 = OpCompositeConstruct %v3float %float_6 %float_12 %float_18
%84 = OpCompositeConstruct %mat3v3float %81 %82 %83
%86 = OpCompositeExtract %v3float %66 0
%87 = OpCompositeExtract %v3float %84 0
%88 = OpFOrdEqual %v3bool %86 %87
%89 = OpAll %bool %88
%90 = OpCompositeExtract %v3float %66 1
%91 = OpCompositeExtract %v3float %84 1
%92 = OpFOrdEqual %v3bool %90 %91
%93 = OpAll %bool %92
%94 = OpLogicalAnd %bool %89 %93
%95 = OpCompositeExtract %v3float %66 2
%96 = OpCompositeExtract %v3float %84 2
%97 = OpFOrdEqual %v3bool %95 %96
%98 = OpAll %bool %97
%99 = OpLogicalAnd %bool %94 %98
OpBranch %65
%65 = OpLabel
%100 = OpPhi %bool %false %28 %99 %64
OpSelectionMerge %102 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%104 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%105 = OpAccessChain %_ptr_Uniform_v2float %104 %int_0
%106 = OpLoad %v2float %105
%107 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%108 = OpAccessChain %_ptr_Uniform_v3float %107 %int_1
%109 = OpLoad %v3float %108
%103 = OpOuterProduct %mat3v2float %106 %109
%111 = OpCompositeConstruct %v2float %float_4 %float_8
%112 = OpCompositeConstruct %v2float %float_5 %float_10
%113 = OpCompositeConstruct %v2float %float_6 %float_12
%114 = OpCompositeConstruct %mat3v2float %111 %112 %113
%115 = OpCompositeExtract %v2float %103 0
%116 = OpCompositeExtract %v2float %114 0
%117 = OpFOrdEqual %v2bool %115 %116
%118 = OpAll %bool %117
%119 = OpCompositeExtract %v2float %103 1
%120 = OpCompositeExtract %v2float %114 1
%121 = OpFOrdEqual %v2bool %119 %120
%122 = OpAll %bool %121
%123 = OpLogicalAnd %bool %118 %122
%124 = OpCompositeExtract %v2float %103 2
%125 = OpCompositeExtract %v2float %114 2
%126 = OpFOrdEqual %v2bool %124 %125
%127 = OpAll %bool %126
%128 = OpLogicalAnd %bool %123 %127
OpBranch %102
%102 = OpLabel
%129 = OpPhi %bool %false %65 %128 %101
OpSelectionMerge %131 None
OpBranchConditional %129 %130 %131
%130 = OpLabel
%133 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%136 = OpLoad %v4float %133
%132 = OpOuterProduct %mat4v4float %136 %137
%145 = OpCompositeConstruct %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%146 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%147 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%148 = OpCompositeConstruct %v4float %float_n2_5 %float_0 %float_1_5 %float_4_5
%149 = OpCompositeConstruct %mat4v4float %145 %146 %147 %148
%151 = OpCompositeExtract %v4float %132 0
%152 = OpCompositeExtract %v4float %149 0
%153 = OpFOrdEqual %v4bool %151 %152
%154 = OpAll %bool %153
%155 = OpCompositeExtract %v4float %132 1
%156 = OpCompositeExtract %v4float %149 1
%157 = OpFOrdEqual %v4bool %155 %156
%158 = OpAll %bool %157
%159 = OpLogicalAnd %bool %154 %158
%160 = OpCompositeExtract %v4float %132 2
%161 = OpCompositeExtract %v4float %149 2
%162 = OpFOrdEqual %v4bool %160 %161
%163 = OpAll %bool %162
%164 = OpLogicalAnd %bool %159 %163
%165 = OpCompositeExtract %v4float %132 3
%166 = OpCompositeExtract %v4float %149 3
%167 = OpFOrdEqual %v4bool %165 %166
%168 = OpAll %bool %167
%169 = OpLogicalAnd %bool %164 %168
OpBranch %131
%131 = OpLabel
%170 = OpPhi %bool %false %102 %169 %130
OpSelectionMerge %172 None
OpBranchConditional %170 %171 %172
%171 = OpLabel
%174 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%175 = OpLoad %v4float %174
%176 = OpLoad %v2float %c12
%173 = OpOuterProduct %mat2v4float %175 %176
%178 = OpCompositeConstruct %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%179 = OpCompositeConstruct %v4float %float_n2_5 %float_0 %float_1_5 %float_4_5
%180 = OpCompositeConstruct %mat2v4float %178 %179
%181 = OpCompositeExtract %v4float %173 0
%182 = OpCompositeExtract %v4float %180 0
%183 = OpFOrdEqual %v4bool %181 %182
%184 = OpAll %bool %183
%185 = OpCompositeExtract %v4float %173 1
%186 = OpCompositeExtract %v4float %180 1
%187 = OpFOrdEqual %v4bool %185 %186
%188 = OpAll %bool %187
%189 = OpLogicalAnd %bool %184 %188
OpBranch %172
%172 = OpLabel
%190 = OpPhi %bool %false %131 %189 %171
OpSelectionMerge %192 None
OpBranchConditional %190 %191 %192
%191 = OpLabel
%194 = OpLoad %v2float %c12
%195 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%196 = OpLoad %v4float %195
%193 = OpOuterProduct %mat4v2float %194 %196
%198 = OpCompositeConstruct %v2float %float_n1_25 %float_n2_5
%199 = OpCompositeConstruct %v2float %float_0 %float_0
%200 = OpCompositeConstruct %v2float %float_0_75 %float_1_5
%201 = OpCompositeConstruct %v2float %float_2_25 %float_4_5
%202 = OpCompositeConstruct %mat4v2float %198 %199 %200 %201
%203 = OpCompositeExtract %v2float %193 0
%204 = OpCompositeExtract %v2float %202 0
%205 = OpFOrdEqual %v2bool %203 %204
%206 = OpAll %bool %205
%207 = OpCompositeExtract %v2float %193 1
%208 = OpCompositeExtract %v2float %202 1
%209 = OpFOrdEqual %v2bool %207 %208
%210 = OpAll %bool %209
%211 = OpLogicalAnd %bool %206 %210
%212 = OpCompositeExtract %v2float %193 2
%213 = OpCompositeExtract %v2float %202 2
%214 = OpFOrdEqual %v2bool %212 %213
%215 = OpAll %bool %214
%216 = OpLogicalAnd %bool %211 %215
%217 = OpCompositeExtract %v2float %193 3
%218 = OpCompositeExtract %v2float %202 3
%219 = OpFOrdEqual %v2bool %217 %218
%220 = OpAll %bool %219
%221 = OpLogicalAnd %bool %216 %220
OpBranch %192
%192 = OpLabel
%222 = OpPhi %bool %false %172 %221 %191
OpSelectionMerge %227 None
OpBranchConditional %222 %225 %226
%225 = OpLabel
%228 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%229 = OpLoad %v4float %228
OpStore %223 %229
OpBranch %227
%226 = OpLabel
%230 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%231 = OpLoad %v4float %230
OpStore %223 %231
OpBranch %227
%227 = OpLabel
%232 = OpLoad %v4float %223
OpReturnValue %232
OpFunctionEnd
