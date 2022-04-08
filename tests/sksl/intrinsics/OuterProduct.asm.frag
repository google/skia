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
OpDecorate %131 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
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
%v2bool = OpTypeVector %bool 2
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_3 = OpConstant %int 3
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
%float_12 = OpConstant %float 12
%float_5 = OpConstant %float 5
%float_10 = OpConstant %float 10
%float_15 = OpConstant %float 15
%float_18 = OpConstant %float 18
%81 = OpConstantComposite %v3float %float_4 %float_8 %float_12
%82 = OpConstantComposite %v3float %float_5 %float_10 %float_15
%83 = OpConstantComposite %v3float %float_6 %float_12 %float_18
%v3bool = OpTypeVector %bool 3
%mat3v2float = OpTypeMatrix %v2float 3
%111 = OpConstantComposite %v2float %float_5 %float_10
%112 = OpConstantComposite %v2float %float_6 %float_12
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_4 = OpConstant %int 4
%136 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_2
%mat4v4float = OpTypeMatrix %v4float 4
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%float_n2_5 = OpConstant %float -2.5
%float_1_5 = OpConstant %float 1.5
%float_4_5 = OpConstant %float 4.5
%144 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%145 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%146 = OpConstantComposite %v4float %float_n2_5 %float_0 %float_1_5 %float_4_5
%v4bool = OpTypeVector %bool 4
%mat2v4float = OpTypeMatrix %v4float 2
%mat4v2float = OpTypeMatrix %v2float 4
%194 = OpConstantComposite %v2float %float_n1_25 %float_n2_5
%195 = OpConstantComposite %v2float %float_0_75 %float_1_5
%196 = OpConstantComposite %v2float %float_2_25 %float_4_5
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
%218 = OpVariable %_ptr_Function_v4float Function
OpStore %c12 %32
%35 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%40 = OpAccessChain %_ptr_Uniform_v2float %35 %int_0
%42 = OpLoad %v2float %40
%43 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%45 = OpAccessChain %_ptr_Uniform_v2float %43 %int_1
%46 = OpLoad %v2float %45
%34 = OpOuterProduct %mat2v2float %42 %46
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
%113 = OpCompositeConstruct %mat3v2float %52 %111 %112
%114 = OpCompositeExtract %v2float %103 0
%115 = OpCompositeExtract %v2float %113 0
%116 = OpFOrdEqual %v2bool %114 %115
%117 = OpAll %bool %116
%118 = OpCompositeExtract %v2float %103 1
%119 = OpCompositeExtract %v2float %113 1
%120 = OpFOrdEqual %v2bool %118 %119
%121 = OpAll %bool %120
%122 = OpLogicalAnd %bool %117 %121
%123 = OpCompositeExtract %v2float %103 2
%124 = OpCompositeExtract %v2float %113 2
%125 = OpFOrdEqual %v2bool %123 %124
%126 = OpAll %bool %125
%127 = OpLogicalAnd %bool %122 %126
OpBranch %102
%102 = OpLabel
%128 = OpPhi %bool %false %65 %127 %101
OpSelectionMerge %130 None
OpBranchConditional %128 %129 %130
%129 = OpLabel
%132 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%135 = OpLoad %v4float %132
%131 = OpOuterProduct %mat4v4float %135 %136
%147 = OpCompositeConstruct %mat4v4float %144 %145 %145 %146
%149 = OpCompositeExtract %v4float %131 0
%150 = OpCompositeExtract %v4float %147 0
%151 = OpFOrdEqual %v4bool %149 %150
%152 = OpAll %bool %151
%153 = OpCompositeExtract %v4float %131 1
%154 = OpCompositeExtract %v4float %147 1
%155 = OpFOrdEqual %v4bool %153 %154
%156 = OpAll %bool %155
%157 = OpLogicalAnd %bool %152 %156
%158 = OpCompositeExtract %v4float %131 2
%159 = OpCompositeExtract %v4float %147 2
%160 = OpFOrdEqual %v4bool %158 %159
%161 = OpAll %bool %160
%162 = OpLogicalAnd %bool %157 %161
%163 = OpCompositeExtract %v4float %131 3
%164 = OpCompositeExtract %v4float %147 3
%165 = OpFOrdEqual %v4bool %163 %164
%166 = OpAll %bool %165
%167 = OpLogicalAnd %bool %162 %166
OpBranch %130
%130 = OpLabel
%168 = OpPhi %bool %false %102 %167 %129
OpSelectionMerge %170 None
OpBranchConditional %168 %169 %170
%169 = OpLabel
%172 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%173 = OpLoad %v4float %172
%174 = OpLoad %v2float %c12
%171 = OpOuterProduct %mat2v4float %173 %174
%176 = OpCompositeConstruct %mat2v4float %144 %146
%177 = OpCompositeExtract %v4float %171 0
%178 = OpCompositeExtract %v4float %176 0
%179 = OpFOrdEqual %v4bool %177 %178
%180 = OpAll %bool %179
%181 = OpCompositeExtract %v4float %171 1
%182 = OpCompositeExtract %v4float %176 1
%183 = OpFOrdEqual %v4bool %181 %182
%184 = OpAll %bool %183
%185 = OpLogicalAnd %bool %180 %184
OpBranch %170
%170 = OpLabel
%186 = OpPhi %bool %false %130 %185 %169
OpSelectionMerge %188 None
OpBranchConditional %186 %187 %188
%187 = OpLabel
%190 = OpLoad %v2float %c12
%191 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%192 = OpLoad %v4float %191
%189 = OpOuterProduct %mat4v2float %190 %192
%197 = OpCompositeConstruct %mat4v2float %194 %22 %195 %196
%198 = OpCompositeExtract %v2float %189 0
%199 = OpCompositeExtract %v2float %197 0
%200 = OpFOrdEqual %v2bool %198 %199
%201 = OpAll %bool %200
%202 = OpCompositeExtract %v2float %189 1
%203 = OpCompositeExtract %v2float %197 1
%204 = OpFOrdEqual %v2bool %202 %203
%205 = OpAll %bool %204
%206 = OpLogicalAnd %bool %201 %205
%207 = OpCompositeExtract %v2float %189 2
%208 = OpCompositeExtract %v2float %197 2
%209 = OpFOrdEqual %v2bool %207 %208
%210 = OpAll %bool %209
%211 = OpLogicalAnd %bool %206 %210
%212 = OpCompositeExtract %v2float %189 3
%213 = OpCompositeExtract %v2float %197 3
%214 = OpFOrdEqual %v2bool %212 %213
%215 = OpAll %bool %214
%216 = OpLogicalAnd %bool %211 %215
OpBranch %188
%188 = OpLabel
%217 = OpPhi %bool %false %170 %216 %187
OpSelectionMerge %222 None
OpBranchConditional %217 %220 %221
%220 = OpLabel
%223 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%224 = OpLoad %v4float %223
OpStore %218 %224
OpBranch %222
%221 = OpLabel
%225 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%226 = OpLoad %v4float %225
OpStore %218 %226
OpBranch %222
%222 = OpLabel
%227 = OpLoad %v4float %218
OpReturnValue %227
OpFunctionEnd
