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
OpName %test_half_b "test_half_b"
OpName %ok "ok"
OpName %m10 "m10"
OpName %m11 "m11"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m10 "_1_m10"
OpName %_2_m11 "_2_m11"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %m10 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %m11 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_11 = OpConstant %float 11
%false = OpConstantFalse %bool
%v4bool = OpTypeVector %bool 4
%float_20 = OpConstant %float 20
%float_9 = OpConstant %float 9
%122 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %24
%25 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%m10 = OpVariable %_ptr_Function_mat4v4float Function
%m11 = OpVariable %_ptr_Function_mat4v4float Function
OpStore %ok %true
%34 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%35 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%36 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%37 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%33 = OpCompositeConstruct %mat4v4float %34 %35 %36 %37
OpStore %m10 %33
%39 = OpLoad %bool %ok
OpSelectionMerge %41 None
OpBranchConditional %39 %40 %41
%40 = OpLabel
%42 = OpLoad %mat4v4float %m10
%44 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%45 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%46 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%47 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%43 = OpCompositeConstruct %mat4v4float %44 %45 %46 %47
%49 = OpCompositeExtract %v4float %42 0
%50 = OpCompositeExtract %v4float %43 0
%51 = OpFOrdEqual %v4bool %49 %50
%52 = OpAll %bool %51
%53 = OpCompositeExtract %v4float %42 1
%54 = OpCompositeExtract %v4float %43 1
%55 = OpFOrdEqual %v4bool %53 %54
%56 = OpAll %bool %55
%57 = OpLogicalAnd %bool %52 %56
%58 = OpCompositeExtract %v4float %42 2
%59 = OpCompositeExtract %v4float %43 2
%60 = OpFOrdEqual %v4bool %58 %59
%61 = OpAll %bool %60
%62 = OpLogicalAnd %bool %57 %61
%63 = OpCompositeExtract %v4float %42 3
%64 = OpCompositeExtract %v4float %43 3
%65 = OpFOrdEqual %v4bool %63 %64
%66 = OpAll %bool %65
%67 = OpLogicalAnd %bool %62 %66
OpBranch %41
%41 = OpLabel
%68 = OpPhi %bool %false %25 %67 %40
OpStore %ok %68
%72 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%73 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%74 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%75 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%71 = OpCompositeConstruct %mat4v4float %72 %73 %74 %75
OpStore %m11 %71
%76 = OpLoad %mat4v4float %m11
%77 = OpLoad %mat4v4float %m10
%78 = OpCompositeExtract %v4float %76 0
%79 = OpCompositeExtract %v4float %77 0
%80 = OpFSub %v4float %78 %79
%81 = OpCompositeExtract %v4float %76 1
%82 = OpCompositeExtract %v4float %77 1
%83 = OpFSub %v4float %81 %82
%84 = OpCompositeExtract %v4float %76 2
%85 = OpCompositeExtract %v4float %77 2
%86 = OpFSub %v4float %84 %85
%87 = OpCompositeExtract %v4float %76 3
%88 = OpCompositeExtract %v4float %77 3
%89 = OpFSub %v4float %87 %88
%90 = OpCompositeConstruct %mat4v4float %80 %83 %86 %89
OpStore %m11 %90
%91 = OpLoad %bool %ok
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%94 = OpLoad %mat4v4float %m11
%97 = OpCompositeConstruct %v4float %float_9 %float_20 %float_20 %float_20
%98 = OpCompositeConstruct %v4float %float_20 %float_9 %float_20 %float_20
%99 = OpCompositeConstruct %v4float %float_20 %float_20 %float_9 %float_20
%100 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_9
%96 = OpCompositeConstruct %mat4v4float %97 %98 %99 %100
%101 = OpCompositeExtract %v4float %94 0
%102 = OpCompositeExtract %v4float %96 0
%103 = OpFOrdEqual %v4bool %101 %102
%104 = OpAll %bool %103
%105 = OpCompositeExtract %v4float %94 1
%106 = OpCompositeExtract %v4float %96 1
%107 = OpFOrdEqual %v4bool %105 %106
%108 = OpAll %bool %107
%109 = OpLogicalAnd %bool %104 %108
%110 = OpCompositeExtract %v4float %94 2
%111 = OpCompositeExtract %v4float %96 2
%112 = OpFOrdEqual %v4bool %110 %111
%113 = OpAll %bool %112
%114 = OpLogicalAnd %bool %109 %113
%115 = OpCompositeExtract %v4float %94 3
%116 = OpCompositeExtract %v4float %96 3
%117 = OpFOrdEqual %v4bool %115 %116
%118 = OpAll %bool %117
%119 = OpLogicalAnd %bool %114 %118
OpBranch %93
%93 = OpLabel
%120 = OpPhi %bool %false %41 %119 %92
OpStore %ok %120
%121 = OpLoad %bool %ok
OpReturnValue %121
OpFunctionEnd
%main = OpFunction %v4float None %122
%123 = OpFunctionParameter %_ptr_Function_v2float
%124 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m10 = OpVariable %_ptr_Function_mat4v4float Function
%_2_m11 = OpVariable %_ptr_Function_mat4v4float Function
%216 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%128 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%129 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%130 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%131 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%127 = OpCompositeConstruct %mat4v4float %128 %129 %130 %131
OpStore %_1_m10 %127
%132 = OpLoad %bool %_0_ok
OpSelectionMerge %134 None
OpBranchConditional %132 %133 %134
%133 = OpLabel
%135 = OpLoad %mat4v4float %_1_m10
%137 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%138 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%139 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%140 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%136 = OpCompositeConstruct %mat4v4float %137 %138 %139 %140
%141 = OpCompositeExtract %v4float %135 0
%142 = OpCompositeExtract %v4float %136 0
%143 = OpFOrdEqual %v4bool %141 %142
%144 = OpAll %bool %143
%145 = OpCompositeExtract %v4float %135 1
%146 = OpCompositeExtract %v4float %136 1
%147 = OpFOrdEqual %v4bool %145 %146
%148 = OpAll %bool %147
%149 = OpLogicalAnd %bool %144 %148
%150 = OpCompositeExtract %v4float %135 2
%151 = OpCompositeExtract %v4float %136 2
%152 = OpFOrdEqual %v4bool %150 %151
%153 = OpAll %bool %152
%154 = OpLogicalAnd %bool %149 %153
%155 = OpCompositeExtract %v4float %135 3
%156 = OpCompositeExtract %v4float %136 3
%157 = OpFOrdEqual %v4bool %155 %156
%158 = OpAll %bool %157
%159 = OpLogicalAnd %bool %154 %158
OpBranch %134
%134 = OpLabel
%160 = OpPhi %bool %false %124 %159 %133
OpStore %_0_ok %160
%163 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%164 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%165 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%166 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_20
%162 = OpCompositeConstruct %mat4v4float %163 %164 %165 %166
OpStore %_2_m11 %162
%167 = OpLoad %mat4v4float %_2_m11
%168 = OpLoad %mat4v4float %_1_m10
%169 = OpCompositeExtract %v4float %167 0
%170 = OpCompositeExtract %v4float %168 0
%171 = OpFSub %v4float %169 %170
%172 = OpCompositeExtract %v4float %167 1
%173 = OpCompositeExtract %v4float %168 1
%174 = OpFSub %v4float %172 %173
%175 = OpCompositeExtract %v4float %167 2
%176 = OpCompositeExtract %v4float %168 2
%177 = OpFSub %v4float %175 %176
%178 = OpCompositeExtract %v4float %167 3
%179 = OpCompositeExtract %v4float %168 3
%180 = OpFSub %v4float %178 %179
%181 = OpCompositeConstruct %mat4v4float %171 %174 %177 %180
OpStore %_2_m11 %181
%182 = OpLoad %bool %_0_ok
OpSelectionMerge %184 None
OpBranchConditional %182 %183 %184
%183 = OpLabel
%185 = OpLoad %mat4v4float %_2_m11
%187 = OpCompositeConstruct %v4float %float_9 %float_20 %float_20 %float_20
%188 = OpCompositeConstruct %v4float %float_20 %float_9 %float_20 %float_20
%189 = OpCompositeConstruct %v4float %float_20 %float_20 %float_9 %float_20
%190 = OpCompositeConstruct %v4float %float_20 %float_20 %float_20 %float_9
%186 = OpCompositeConstruct %mat4v4float %187 %188 %189 %190
%191 = OpCompositeExtract %v4float %185 0
%192 = OpCompositeExtract %v4float %186 0
%193 = OpFOrdEqual %v4bool %191 %192
%194 = OpAll %bool %193
%195 = OpCompositeExtract %v4float %185 1
%196 = OpCompositeExtract %v4float %186 1
%197 = OpFOrdEqual %v4bool %195 %196
%198 = OpAll %bool %197
%199 = OpLogicalAnd %bool %194 %198
%200 = OpCompositeExtract %v4float %185 2
%201 = OpCompositeExtract %v4float %186 2
%202 = OpFOrdEqual %v4bool %200 %201
%203 = OpAll %bool %202
%204 = OpLogicalAnd %bool %199 %203
%205 = OpCompositeExtract %v4float %185 3
%206 = OpCompositeExtract %v4float %186 3
%207 = OpFOrdEqual %v4bool %205 %206
%208 = OpAll %bool %207
%209 = OpLogicalAnd %bool %204 %208
OpBranch %184
%184 = OpLabel
%210 = OpPhi %bool %false %134 %209 %183
OpStore %_0_ok %210
%211 = OpLoad %bool %_0_ok
OpSelectionMerge %213 None
OpBranchConditional %211 %212 %213
%212 = OpLabel
%214 = OpFunctionCall %bool %test_half_b
OpBranch %213
%213 = OpLabel
%215 = OpPhi %bool %false %184 %214 %212
OpSelectionMerge %220 None
OpBranchConditional %215 %218 %219
%218 = OpLabel
%221 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%225 = OpLoad %v4float %221
OpStore %216 %225
OpBranch %220
%219 = OpLabel
%226 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%228 = OpLoad %v4float %226
OpStore %216 %228
OpBranch %220
%220 = OpLabel
%229 = OpLoad %v4float %216
OpReturnValue %229
OpFunctionEnd
