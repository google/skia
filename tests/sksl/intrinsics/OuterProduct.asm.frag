OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "h2"
OpMemberName %_UniformBuffer 1 "h3"
OpMemberName %_UniformBuffer 2 "h4"
OpMemberName %_UniformBuffer 3 "f2"
OpMemberName %_UniformBuffer 4 "f3"
OpMemberName %_UniformBuffer 5 "f4"
OpName %main "main"
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
OpMemberDecorate %_UniformBuffer 4 Offset 64
OpMemberDecorate %_UniformBuffer 5 Offset 80
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %38 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%v3float = OpTypeVector %float 3
%_UniformBuffer = OpTypeStruct %v2float %v3float %v4float %v2float %v3float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%int = OpTypeInt 32 1
%int_3 = OpConstant %int 3
%int_1 = OpConstant %int 1
%_ptr_Function_v2float = OpTypePointer Function %v2float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
%int_4 = OpConstant %int 4
%int_2 = OpConstant %int 2
%_ptr_Function_v3float = OpTypePointer Function %v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_5 = OpConstant %int 5
%_ptr_Function_v4float = OpTypePointer Function %v4float
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%int_0 = OpConstant %int 0
%main = OpFunction %void None %16
%17 = OpLabel
%18 = OpVariable %_ptr_Function_mat2v2float Function
%39 = OpVariable %_ptr_Function_mat3v3float Function
%59 = OpVariable %_ptr_Function_mat4v4float Function
%77 = OpVariable %_ptr_Function_mat2v3float Function
%93 = OpVariable %_ptr_Function_mat3v2float Function
%109 = OpVariable %_ptr_Function_mat2v4float Function
%124 = OpVariable %_ptr_Function_mat4v2float Function
%140 = OpVariable %_ptr_Function_mat3v4float Function
%155 = OpVariable %_ptr_Function_mat4v3float Function
%171 = OpVariable %_ptr_Function_mat2v2float Function
%181 = OpVariable %_ptr_Function_mat3v3float Function
%190 = OpVariable %_ptr_Function_mat4v4float Function
%198 = OpVariable %_ptr_Function_mat2v3float Function
%207 = OpVariable %_ptr_Function_mat3v2float Function
%216 = OpVariable %_ptr_Function_mat2v4float Function
%224 = OpVariable %_ptr_Function_mat4v2float Function
%233 = OpVariable %_ptr_Function_mat3v4float Function
%241 = OpVariable %_ptr_Function_mat4v3float Function
%22 = OpAccessChain %_ptr_Uniform_v2float %10 %int_3
%26 = OpLoad %v2float %22
%27 = OpAccessChain %_ptr_Uniform_v2float %10 %int_3
%28 = OpLoad %v2float %27
%21 = OpOuterProduct %mat2v2float %26 %28
OpStore %18 %21
%30 = OpAccessChain %_ptr_Function_v2float %18 %int_1
%32 = OpLoad %v2float %30
%33 = OpVectorShuffle %v4float %32 %32 0 1 1 1
%34 = OpCompositeExtract %float %33 0
%35 = OpCompositeExtract %float %33 1
%36 = OpCompositeExtract %float %33 2
%37 = OpCompositeExtract %float %33 3
%38 = OpCompositeConstruct %v4float %34 %35 %36 %37
OpStore %sk_FragColor %38
%43 = OpAccessChain %_ptr_Uniform_v3float %10 %int_4
%46 = OpLoad %v3float %43
%47 = OpAccessChain %_ptr_Uniform_v3float %10 %int_4
%48 = OpLoad %v3float %47
%42 = OpOuterProduct %mat3v3float %46 %48
OpStore %39 %42
%50 = OpAccessChain %_ptr_Function_v3float %39 %int_2
%52 = OpLoad %v3float %50
%53 = OpVectorShuffle %v4float %52 %52 0 1 2 2
%54 = OpCompositeExtract %float %53 0
%55 = OpCompositeExtract %float %53 1
%56 = OpCompositeExtract %float %53 2
%57 = OpCompositeExtract %float %53 3
%58 = OpCompositeConstruct %v4float %54 %55 %56 %57
OpStore %sk_FragColor %58
%63 = OpAccessChain %_ptr_Uniform_v4float %10 %int_5
%66 = OpLoad %v4float %63
%67 = OpAccessChain %_ptr_Uniform_v4float %10 %int_5
%68 = OpLoad %v4float %67
%62 = OpOuterProduct %mat4v4float %66 %68
OpStore %59 %62
%69 = OpAccessChain %_ptr_Function_v4float %59 %int_3
%71 = OpLoad %v4float %69
%72 = OpCompositeExtract %float %71 0
%73 = OpCompositeExtract %float %71 1
%74 = OpCompositeExtract %float %71 2
%75 = OpCompositeExtract %float %71 3
%76 = OpCompositeConstruct %v4float %72 %73 %74 %75
OpStore %sk_FragColor %76
%81 = OpAccessChain %_ptr_Uniform_v3float %10 %int_4
%82 = OpLoad %v3float %81
%83 = OpAccessChain %_ptr_Uniform_v2float %10 %int_3
%84 = OpLoad %v2float %83
%80 = OpOuterProduct %mat2v3float %82 %84
OpStore %77 %80
%85 = OpAccessChain %_ptr_Function_v3float %77 %int_1
%86 = OpLoad %v3float %85
%87 = OpVectorShuffle %v4float %86 %86 0 1 2 2
%88 = OpCompositeExtract %float %87 0
%89 = OpCompositeExtract %float %87 1
%90 = OpCompositeExtract %float %87 2
%91 = OpCompositeExtract %float %87 3
%92 = OpCompositeConstruct %v4float %88 %89 %90 %91
OpStore %sk_FragColor %92
%97 = OpAccessChain %_ptr_Uniform_v2float %10 %int_3
%98 = OpLoad %v2float %97
%99 = OpAccessChain %_ptr_Uniform_v3float %10 %int_4
%100 = OpLoad %v3float %99
%96 = OpOuterProduct %mat3v2float %98 %100
OpStore %93 %96
%101 = OpAccessChain %_ptr_Function_v2float %93 %int_2
%102 = OpLoad %v2float %101
%103 = OpVectorShuffle %v4float %102 %102 0 1 1 1
%104 = OpCompositeExtract %float %103 0
%105 = OpCompositeExtract %float %103 1
%106 = OpCompositeExtract %float %103 2
%107 = OpCompositeExtract %float %103 3
%108 = OpCompositeConstruct %v4float %104 %105 %106 %107
OpStore %sk_FragColor %108
%113 = OpAccessChain %_ptr_Uniform_v4float %10 %int_5
%114 = OpLoad %v4float %113
%115 = OpAccessChain %_ptr_Uniform_v2float %10 %int_3
%116 = OpLoad %v2float %115
%112 = OpOuterProduct %mat2v4float %114 %116
OpStore %109 %112
%117 = OpAccessChain %_ptr_Function_v4float %109 %int_1
%118 = OpLoad %v4float %117
%119 = OpCompositeExtract %float %118 0
%120 = OpCompositeExtract %float %118 1
%121 = OpCompositeExtract %float %118 2
%122 = OpCompositeExtract %float %118 3
%123 = OpCompositeConstruct %v4float %119 %120 %121 %122
OpStore %sk_FragColor %123
%128 = OpAccessChain %_ptr_Uniform_v2float %10 %int_3
%129 = OpLoad %v2float %128
%130 = OpAccessChain %_ptr_Uniform_v4float %10 %int_5
%131 = OpLoad %v4float %130
%127 = OpOuterProduct %mat4v2float %129 %131
OpStore %124 %127
%132 = OpAccessChain %_ptr_Function_v2float %124 %int_3
%133 = OpLoad %v2float %132
%134 = OpVectorShuffle %v4float %133 %133 0 1 1 1
%135 = OpCompositeExtract %float %134 0
%136 = OpCompositeExtract %float %134 1
%137 = OpCompositeExtract %float %134 2
%138 = OpCompositeExtract %float %134 3
%139 = OpCompositeConstruct %v4float %135 %136 %137 %138
OpStore %sk_FragColor %139
%144 = OpAccessChain %_ptr_Uniform_v4float %10 %int_5
%145 = OpLoad %v4float %144
%146 = OpAccessChain %_ptr_Uniform_v3float %10 %int_4
%147 = OpLoad %v3float %146
%143 = OpOuterProduct %mat3v4float %145 %147
OpStore %140 %143
%148 = OpAccessChain %_ptr_Function_v4float %140 %int_2
%149 = OpLoad %v4float %148
%150 = OpCompositeExtract %float %149 0
%151 = OpCompositeExtract %float %149 1
%152 = OpCompositeExtract %float %149 2
%153 = OpCompositeExtract %float %149 3
%154 = OpCompositeConstruct %v4float %150 %151 %152 %153
OpStore %sk_FragColor %154
%159 = OpAccessChain %_ptr_Uniform_v3float %10 %int_4
%160 = OpLoad %v3float %159
%161 = OpAccessChain %_ptr_Uniform_v4float %10 %int_5
%162 = OpLoad %v4float %161
%158 = OpOuterProduct %mat4v3float %160 %162
OpStore %155 %158
%163 = OpAccessChain %_ptr_Function_v3float %155 %int_3
%164 = OpLoad %v3float %163
%165 = OpVectorShuffle %v4float %164 %164 0 1 2 2
%166 = OpCompositeExtract %float %165 0
%167 = OpCompositeExtract %float %165 1
%168 = OpCompositeExtract %float %165 2
%169 = OpCompositeExtract %float %165 3
%170 = OpCompositeConstruct %v4float %166 %167 %168 %169
OpStore %sk_FragColor %170
%173 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%175 = OpLoad %v2float %173
%176 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%177 = OpLoad %v2float %176
%172 = OpOuterProduct %mat2v2float %175 %177
OpStore %171 %172
%178 = OpAccessChain %_ptr_Function_v2float %171 %int_1
%179 = OpLoad %v2float %178
%180 = OpVectorShuffle %v4float %179 %179 0 1 1 1
OpStore %sk_FragColor %180
%183 = OpAccessChain %_ptr_Uniform_v3float %10 %int_1
%184 = OpLoad %v3float %183
%185 = OpAccessChain %_ptr_Uniform_v3float %10 %int_1
%186 = OpLoad %v3float %185
%182 = OpOuterProduct %mat3v3float %184 %186
OpStore %181 %182
%187 = OpAccessChain %_ptr_Function_v3float %181 %int_2
%188 = OpLoad %v3float %187
%189 = OpVectorShuffle %v4float %188 %188 0 1 2 2
OpStore %sk_FragColor %189
%192 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%193 = OpLoad %v4float %192
%194 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%195 = OpLoad %v4float %194
%191 = OpOuterProduct %mat4v4float %193 %195
OpStore %190 %191
%196 = OpAccessChain %_ptr_Function_v4float %190 %int_3
%197 = OpLoad %v4float %196
OpStore %sk_FragColor %197
%200 = OpAccessChain %_ptr_Uniform_v3float %10 %int_1
%201 = OpLoad %v3float %200
%202 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%203 = OpLoad %v2float %202
%199 = OpOuterProduct %mat2v3float %201 %203
OpStore %198 %199
%204 = OpAccessChain %_ptr_Function_v3float %198 %int_1
%205 = OpLoad %v3float %204
%206 = OpVectorShuffle %v4float %205 %205 0 1 2 2
OpStore %sk_FragColor %206
%209 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%210 = OpLoad %v2float %209
%211 = OpAccessChain %_ptr_Uniform_v3float %10 %int_1
%212 = OpLoad %v3float %211
%208 = OpOuterProduct %mat3v2float %210 %212
OpStore %207 %208
%213 = OpAccessChain %_ptr_Function_v2float %207 %int_2
%214 = OpLoad %v2float %213
%215 = OpVectorShuffle %v4float %214 %214 0 1 1 1
OpStore %sk_FragColor %215
%218 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%219 = OpLoad %v4float %218
%220 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%221 = OpLoad %v2float %220
%217 = OpOuterProduct %mat2v4float %219 %221
OpStore %216 %217
%222 = OpAccessChain %_ptr_Function_v4float %216 %int_1
%223 = OpLoad %v4float %222
OpStore %sk_FragColor %223
%226 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%227 = OpLoad %v2float %226
%228 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%229 = OpLoad %v4float %228
%225 = OpOuterProduct %mat4v2float %227 %229
OpStore %224 %225
%230 = OpAccessChain %_ptr_Function_v2float %224 %int_3
%231 = OpLoad %v2float %230
%232 = OpVectorShuffle %v4float %231 %231 0 1 1 1
OpStore %sk_FragColor %232
%235 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%236 = OpLoad %v4float %235
%237 = OpAccessChain %_ptr_Uniform_v3float %10 %int_1
%238 = OpLoad %v3float %237
%234 = OpOuterProduct %mat3v4float %236 %238
OpStore %233 %234
%239 = OpAccessChain %_ptr_Function_v4float %233 %int_2
%240 = OpLoad %v4float %239
OpStore %sk_FragColor %240
%243 = OpAccessChain %_ptr_Uniform_v3float %10 %int_1
%244 = OpLoad %v3float %243
%245 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%246 = OpLoad %v4float %245
%242 = OpOuterProduct %mat4v3float %244 %246
OpStore %241 %242
%247 = OpAccessChain %_ptr_Function_v3float %241 %int_3
%248 = OpLoad %v3float %247
%249 = OpVectorShuffle %v4float %248 %248 0 1 2 2
OpStore %sk_FragColor %249
OpReturn
OpFunctionEnd
