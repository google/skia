OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %plus "plus"
OpName %minus "minus"
OpName %star "star"
OpName %slash "slash"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_bifffff22 "test_bifffff22"
OpName %one "one"
OpName %m2 "m2"
OpName %main "main"
OpName %f1 "f1"
OpName %f2 "f2"
OpName %f3 "f3"
OpName %f4 "f4"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %21 Binding 0
OpDecorate %21 DescriptorSet 0
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%int = OpTypeInt 32 1
%_ptr_Private_int = OpTypePointer Private %int
%plus = OpVariable %_ptr_Private_int Private
%int_1 = OpConstant %int 1
%minus = OpVariable %_ptr_Private_int Private
%int_2 = OpConstant %int 2
%star = OpVariable %_ptr_Private_int Private
%int_3 = OpConstant %int 3
%slash = OpVariable %_ptr_Private_int Private
%int_4 = OpConstant %int 4
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%21 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%26 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%30 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%38 = OpTypeFunction %bool %_ptr_Function_int %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_float %_ptr_Function_mat2v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%float_1 = OpConstant %float 1
%70 = OpConstantComposite %v2float %float_1 %float_1
%71 = OpConstantComposite %mat2v2float %70 %70
%float_2 = OpConstant %float 2
%85 = OpConstantComposite %v2float %float_2 %float_2
%86 = OpConstantComposite %mat2v2float %85 %85
%false = OpConstantFalse %bool
%int_0 = OpConstant %int 0
%131 = OpTypeFunction %v4float %_ptr_Function_v2float
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_0_5 = OpConstant %float 0.5
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %26
%27 = OpLabel
%31 = OpVariable %_ptr_Function_v2float Function
OpStore %31 %30
%33 = OpFunctionCall %v4float %main %31
OpStore %sk_FragColor %33
OpReturn
OpFunctionEnd
%test_bifffff22 = OpFunction %bool None %38
%39 = OpFunctionParameter %_ptr_Function_int
%40 = OpFunctionParameter %_ptr_Function_float
%41 = OpFunctionParameter %_ptr_Function_float
%42 = OpFunctionParameter %_ptr_Function_float
%43 = OpFunctionParameter %_ptr_Function_float
%44 = OpFunctionParameter %_ptr_Function_mat2v2float
%45 = OpLabel
%one = OpVariable %_ptr_Function_float Function
%m2 = OpVariable %_ptr_Function_mat2v2float Function
%47 = OpAccessChain %_ptr_Uniform_v4float %21 %int_1
%49 = OpLoad %v4float %47
%50 = OpCompositeExtract %float %49 0
OpStore %one %50
%52 = OpLoad %float %40
%53 = OpFMul %float %52 %50
%54 = OpLoad %float %41
%55 = OpFMul %float %54 %50
%56 = OpLoad %float %42
%57 = OpFMul %float %56 %50
%58 = OpLoad %float %43
%59 = OpFMul %float %58 %50
%60 = OpCompositeConstruct %v2float %53 %55
%61 = OpCompositeConstruct %v2float %57 %59
%62 = OpCompositeConstruct %mat2v2float %60 %61
OpStore %m2 %62
%63 = OpLoad %int %39
OpSelectionMerge %64 None
OpSwitch %63 %64 1 %65 2 %66 3 %67 4 %68
%65 = OpLabel
%72 = OpFAdd %v2float %60 %70
%73 = OpFAdd %v2float %61 %70
%74 = OpCompositeConstruct %mat2v2float %72 %73
OpStore %m2 %74
OpBranch %64
%66 = OpLabel
%75 = OpLoad %mat2v2float %m2
%76 = OpCompositeExtract %v2float %75 0
%77 = OpFSub %v2float %76 %70
%78 = OpCompositeExtract %v2float %75 1
%79 = OpFSub %v2float %78 %70
%80 = OpCompositeConstruct %mat2v2float %77 %79
OpStore %m2 %80
OpBranch %64
%67 = OpLabel
%81 = OpLoad %mat2v2float %m2
%83 = OpMatrixTimesScalar %mat2v2float %81 %float_2
OpStore %m2 %83
OpBranch %64
%68 = OpLabel
%84 = OpLoad %mat2v2float %m2
%87 = OpCompositeExtract %v2float %84 0
%88 = OpFDiv %v2float %87 %85
%89 = OpCompositeExtract %v2float %84 1
%90 = OpFDiv %v2float %89 %85
%91 = OpCompositeConstruct %mat2v2float %88 %90
OpStore %m2 %91
OpBranch %64
%64 = OpLabel
%94 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%95 = OpLoad %v2float %94
%96 = OpCompositeExtract %float %95 0
%97 = OpAccessChain %_ptr_Function_v2float %44 %int_0
%98 = OpLoad %v2float %97
%99 = OpCompositeExtract %float %98 0
%100 = OpFOrdEqual %bool %96 %99
OpSelectionMerge %102 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%103 = OpAccessChain %_ptr_Function_v2float %m2 %int_0
%104 = OpLoad %v2float %103
%105 = OpCompositeExtract %float %104 1
%106 = OpAccessChain %_ptr_Function_v2float %44 %int_0
%107 = OpLoad %v2float %106
%108 = OpCompositeExtract %float %107 1
%109 = OpFOrdEqual %bool %105 %108
OpBranch %102
%102 = OpLabel
%110 = OpPhi %bool %false %64 %109 %101
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%113 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%114 = OpLoad %v2float %113
%115 = OpCompositeExtract %float %114 0
%116 = OpAccessChain %_ptr_Function_v2float %44 %int_1
%117 = OpLoad %v2float %116
%118 = OpCompositeExtract %float %117 0
%119 = OpFOrdEqual %bool %115 %118
OpBranch %112
%112 = OpLabel
%120 = OpPhi %bool %false %102 %119 %111
OpSelectionMerge %122 None
OpBranchConditional %120 %121 %122
%121 = OpLabel
%123 = OpAccessChain %_ptr_Function_v2float %m2 %int_1
%124 = OpLoad %v2float %123
%125 = OpCompositeExtract %float %124 1
%126 = OpAccessChain %_ptr_Function_v2float %44 %int_1
%127 = OpLoad %v2float %126
%128 = OpCompositeExtract %float %127 1
%129 = OpFOrdEqual %bool %125 %128
OpBranch %122
%122 = OpLabel
%130 = OpPhi %bool %false %112 %129 %121
OpReturnValue %130
OpFunctionEnd
%main = OpFunction %v4float None %131
%132 = OpFunctionParameter %_ptr_Function_v2float
%133 = OpLabel
%f1 = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_float Function
%f3 = OpVariable %_ptr_Function_float Function
%f4 = OpVariable %_ptr_Function_float Function
%156 = OpVariable %_ptr_Function_int Function
%157 = OpVariable %_ptr_Function_float Function
%158 = OpVariable %_ptr_Function_float Function
%159 = OpVariable %_ptr_Function_float Function
%160 = OpVariable %_ptr_Function_float Function
%168 = OpVariable %_ptr_Function_mat2v2float Function
%173 = OpVariable %_ptr_Function_int Function
%174 = OpVariable %_ptr_Function_float Function
%175 = OpVariable %_ptr_Function_float Function
%176 = OpVariable %_ptr_Function_float Function
%177 = OpVariable %_ptr_Function_float Function
%185 = OpVariable %_ptr_Function_mat2v2float Function
%191 = OpVariable %_ptr_Function_int Function
%192 = OpVariable %_ptr_Function_float Function
%193 = OpVariable %_ptr_Function_float Function
%194 = OpVariable %_ptr_Function_float Function
%195 = OpVariable %_ptr_Function_float Function
%203 = OpVariable %_ptr_Function_mat2v2float Function
%209 = OpVariable %_ptr_Function_int Function
%210 = OpVariable %_ptr_Function_float Function
%211 = OpVariable %_ptr_Function_float Function
%212 = OpVariable %_ptr_Function_float Function
%213 = OpVariable %_ptr_Function_float Function
%222 = OpVariable %_ptr_Function_mat2v2float Function
%225 = OpVariable %_ptr_Function_v4float Function
OpStore %plus %int_1
OpStore %minus %int_2
OpStore %star %int_3
OpStore %slash %int_4
%135 = OpAccessChain %_ptr_Uniform_v4float %21 %int_0
%136 = OpLoad %v4float %135
%137 = OpCompositeExtract %float %136 1
OpStore %f1 %137
%139 = OpAccessChain %_ptr_Uniform_v4float %21 %int_0
%140 = OpLoad %v4float %139
%141 = OpCompositeExtract %float %140 1
%142 = OpFMul %float %float_2 %141
OpStore %f2 %142
%145 = OpAccessChain %_ptr_Uniform_v4float %21 %int_0
%146 = OpLoad %v4float %145
%147 = OpCompositeExtract %float %146 1
%148 = OpFMul %float %float_3 %147
OpStore %f3 %148
%151 = OpAccessChain %_ptr_Uniform_v4float %21 %int_0
%152 = OpLoad %v4float %151
%153 = OpCompositeExtract %float %152 1
%154 = OpFMul %float %float_4 %153
OpStore %f4 %154
%155 = OpLoad %int %plus
OpStore %156 %155
OpStore %157 %137
OpStore %158 %142
OpStore %159 %148
OpStore %160 %154
%161 = OpFAdd %float %137 %float_1
%162 = OpFAdd %float %142 %float_1
%163 = OpFAdd %float %148 %float_1
%164 = OpFAdd %float %154 %float_1
%165 = OpCompositeConstruct %v2float %161 %162
%166 = OpCompositeConstruct %v2float %163 %164
%167 = OpCompositeConstruct %mat2v2float %165 %166
OpStore %168 %167
%169 = OpFunctionCall %bool %test_bifffff22 %156 %157 %158 %159 %160 %168
OpSelectionMerge %171 None
OpBranchConditional %169 %170 %171
%170 = OpLabel
%172 = OpLoad %int %minus
OpStore %173 %172
OpStore %174 %137
OpStore %175 %142
OpStore %176 %148
OpStore %177 %154
%178 = OpFSub %float %137 %float_1
%179 = OpFSub %float %142 %float_1
%180 = OpFSub %float %148 %float_1
%181 = OpFSub %float %154 %float_1
%182 = OpCompositeConstruct %v2float %178 %179
%183 = OpCompositeConstruct %v2float %180 %181
%184 = OpCompositeConstruct %mat2v2float %182 %183
OpStore %185 %184
%186 = OpFunctionCall %bool %test_bifffff22 %173 %174 %175 %176 %177 %185
OpBranch %171
%171 = OpLabel
%187 = OpPhi %bool %false %133 %186 %170
OpSelectionMerge %189 None
OpBranchConditional %187 %188 %189
%188 = OpLabel
%190 = OpLoad %int %star
OpStore %191 %190
OpStore %192 %137
OpStore %193 %142
OpStore %194 %148
OpStore %195 %154
%196 = OpFMul %float %137 %float_2
%197 = OpFMul %float %142 %float_2
%198 = OpFMul %float %148 %float_2
%199 = OpFMul %float %154 %float_2
%200 = OpCompositeConstruct %v2float %196 %197
%201 = OpCompositeConstruct %v2float %198 %199
%202 = OpCompositeConstruct %mat2v2float %200 %201
OpStore %203 %202
%204 = OpFunctionCall %bool %test_bifffff22 %191 %192 %193 %194 %195 %203
OpBranch %189
%189 = OpLabel
%205 = OpPhi %bool %false %171 %204 %188
OpSelectionMerge %207 None
OpBranchConditional %205 %206 %207
%206 = OpLabel
%208 = OpLoad %int %slash
OpStore %209 %208
OpStore %210 %137
OpStore %211 %142
OpStore %212 %148
OpStore %213 %154
%215 = OpFMul %float %137 %float_0_5
%216 = OpFMul %float %142 %float_0_5
%217 = OpFMul %float %148 %float_0_5
%218 = OpFMul %float %154 %float_0_5
%219 = OpCompositeConstruct %v2float %215 %216
%220 = OpCompositeConstruct %v2float %217 %218
%221 = OpCompositeConstruct %mat2v2float %219 %220
OpStore %222 %221
%223 = OpFunctionCall %bool %test_bifffff22 %209 %210 %211 %212 %213 %222
OpBranch %207
%207 = OpLabel
%224 = OpPhi %bool %false %189 %223 %206
OpSelectionMerge %229 None
OpBranchConditional %224 %227 %228
%227 = OpLabel
%230 = OpAccessChain %_ptr_Uniform_v4float %21 %int_0
%231 = OpLoad %v4float %230
OpStore %225 %231
OpBranch %229
%228 = OpLabel
%232 = OpAccessChain %_ptr_Uniform_v4float %21 %int_1
%233 = OpLoad %v4float %232
OpStore %225 %233
OpBranch %229
%229 = OpLabel
%234 = OpLoad %v4float %225
OpReturnValue %234
OpFunctionEnd
