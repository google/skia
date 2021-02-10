OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %v "v"
OpName %result "result"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %27 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0 = OpConstant %float 0
%true = OpConstantTrue %bool
%v2bool = OpTypeVector %bool 2
%false = OpConstantFalse %bool
%int_1 = OpConstant %int 1
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%v = OpVariable %_ptr_Function_v4bool Function
%result = OpVariable %_ptr_Function_v4bool Function
%230 = OpVariable %_ptr_Function_v4float Function
%23 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%27 = OpLoad %v4float %23
%28 = OpCompositeExtract %float %27 1
%29 = OpFUnordNotEqual %bool %28 %float_0
%31 = OpCompositeConstruct %v4bool %29 %29 %29 %29
OpStore %v %31
%33 = OpLoad %v4bool %v
%34 = OpCompositeExtract %bool %33 0
%36 = OpCompositeConstruct %v4bool %34 %true %true %true
OpStore %result %36
%37 = OpLoad %v4bool %v
%38 = OpVectorShuffle %v2bool %37 %37 0 1
%40 = OpCompositeExtract %bool %38 0
%41 = OpCompositeExtract %bool %38 1
%43 = OpCompositeConstruct %v4bool %40 %41 %false %true
OpStore %result %43
%44 = OpLoad %v4bool %v
%45 = OpCompositeExtract %bool %44 0
%47 = OpINotEqual %bool %int_1 %int_0
%48 = OpCompositeConstruct %v2bool %45 %47
%49 = OpCompositeExtract %bool %48 0
%50 = OpCompositeExtract %bool %48 1
%51 = OpCompositeConstruct %v4bool %49 %50 %true %false
OpStore %result %51
%52 = OpLoad %v4bool %v
%53 = OpCompositeExtract %bool %52 1
%54 = OpINotEqual %bool %int_0 %int_0
%55 = OpCompositeConstruct %v2bool %53 %54
%56 = OpVectorShuffle %v2bool %55 %55 1 0
%57 = OpCompositeExtract %bool %56 0
%58 = OpCompositeExtract %bool %56 1
%59 = OpCompositeConstruct %v4bool %57 %58 %true %true
OpStore %result %59
%60 = OpLoad %v4bool %v
%61 = OpVectorShuffle %v3bool %60 %60 0 1 2
%63 = OpCompositeExtract %bool %61 0
%64 = OpCompositeExtract %bool %61 1
%65 = OpCompositeExtract %bool %61 2
%66 = OpCompositeConstruct %v4bool %63 %64 %65 %true
OpStore %result %66
%67 = OpLoad %v4bool %v
%68 = OpVectorShuffle %v2bool %67 %67 0 1
%69 = OpCompositeExtract %bool %68 0
%70 = OpCompositeExtract %bool %68 1
%71 = OpINotEqual %bool %int_1 %int_0
%72 = OpCompositeConstruct %v3bool %69 %70 %71
%73 = OpCompositeExtract %bool %72 0
%74 = OpCompositeExtract %bool %72 1
%75 = OpCompositeExtract %bool %72 2
%76 = OpCompositeConstruct %v4bool %73 %74 %75 %true
OpStore %result %76
%77 = OpLoad %v4bool %v
%78 = OpVectorShuffle %v2bool %77 %77 0 2
%79 = OpCompositeExtract %bool %78 0
%80 = OpCompositeExtract %bool %78 1
%81 = OpINotEqual %bool %int_0 %int_0
%82 = OpCompositeConstruct %v3bool %79 %80 %81
%83 = OpVectorShuffle %v3bool %82 %82 0 2 1
%84 = OpCompositeExtract %bool %83 0
%85 = OpCompositeExtract %bool %83 1
%86 = OpCompositeExtract %bool %83 2
%87 = OpCompositeConstruct %v4bool %84 %85 %86 %true
OpStore %result %87
%88 = OpLoad %v4bool %v
%89 = OpCompositeExtract %bool %88 0
%90 = OpINotEqual %bool %int_1 %int_0
%91 = OpINotEqual %bool %int_0 %int_0
%92 = OpCompositeConstruct %v3bool %89 %90 %91
%93 = OpCompositeExtract %bool %92 0
%94 = OpCompositeExtract %bool %92 1
%95 = OpCompositeExtract %bool %92 2
%96 = OpCompositeConstruct %v4bool %93 %94 %95 %false
OpStore %result %96
%97 = OpLoad %v4bool %v
%98 = OpVectorShuffle %v2bool %97 %97 1 2
%99 = OpCompositeExtract %bool %98 0
%100 = OpCompositeExtract %bool %98 1
%101 = OpINotEqual %bool %int_1 %int_0
%102 = OpCompositeConstruct %v3bool %99 %100 %101
%103 = OpVectorShuffle %v3bool %102 %102 2 0 1
%104 = OpCompositeExtract %bool %103 0
%105 = OpCompositeExtract %bool %103 1
%106 = OpCompositeExtract %bool %103 2
%107 = OpCompositeConstruct %v4bool %104 %105 %106 %false
OpStore %result %107
%108 = OpLoad %v4bool %v
%109 = OpCompositeExtract %bool %108 1
%110 = OpINotEqual %bool %int_0 %int_0
%111 = OpINotEqual %bool %int_1 %int_0
%112 = OpCompositeConstruct %v3bool %109 %110 %111
%113 = OpVectorShuffle %v3bool %112 %112 1 0 2
%114 = OpCompositeExtract %bool %113 0
%115 = OpCompositeExtract %bool %113 1
%116 = OpCompositeExtract %bool %113 2
%117 = OpCompositeConstruct %v4bool %114 %115 %116 %false
OpStore %result %117
%118 = OpLoad %v4bool %v
%119 = OpCompositeExtract %bool %118 2
%120 = OpINotEqual %bool %int_1 %int_0
%121 = OpCompositeConstruct %v2bool %119 %120
%122 = OpVectorShuffle %v3bool %121 %121 1 1 0
%123 = OpCompositeExtract %bool %122 0
%124 = OpCompositeExtract %bool %122 1
%125 = OpCompositeExtract %bool %122 2
%126 = OpCompositeConstruct %v4bool %123 %124 %125 %false
OpStore %result %126
%127 = OpLoad %v4bool %v
%128 = OpVectorShuffle %v4bool %127 %127 0 1 2 3
OpStore %result %128
%129 = OpLoad %v4bool %v
%130 = OpVectorShuffle %v3bool %129 %129 0 1 2
%131 = OpCompositeExtract %bool %130 0
%132 = OpCompositeExtract %bool %130 1
%133 = OpCompositeExtract %bool %130 2
%134 = OpINotEqual %bool %int_1 %int_0
%135 = OpCompositeConstruct %v4bool %131 %132 %133 %134
OpStore %result %135
%136 = OpLoad %v4bool %v
%137 = OpVectorShuffle %v3bool %136 %136 0 1 3
%138 = OpCompositeExtract %bool %137 0
%139 = OpCompositeExtract %bool %137 1
%140 = OpCompositeExtract %bool %137 2
%141 = OpINotEqual %bool %int_0 %int_0
%142 = OpCompositeConstruct %v4bool %138 %139 %140 %141
%143 = OpVectorShuffle %v4bool %142 %142 0 1 3 2
OpStore %result %143
%144 = OpLoad %v4bool %v
%145 = OpVectorShuffle %v2bool %144 %144 0 1
%146 = OpCompositeExtract %bool %145 0
%147 = OpCompositeExtract %bool %145 1
%148 = OpINotEqual %bool %int_1 %int_0
%149 = OpINotEqual %bool %int_0 %int_0
%150 = OpCompositeConstruct %v4bool %146 %147 %148 %149
OpStore %result %150
%151 = OpLoad %v4bool %v
%152 = OpVectorShuffle %v3bool %151 %151 0 2 3
%153 = OpCompositeExtract %bool %152 0
%154 = OpCompositeExtract %bool %152 1
%155 = OpCompositeExtract %bool %152 2
%156 = OpINotEqual %bool %int_1 %int_0
%157 = OpCompositeConstruct %v4bool %153 %154 %155 %156
%158 = OpVectorShuffle %v4bool %157 %157 0 3 1 2
OpStore %result %158
%159 = OpLoad %v4bool %v
%160 = OpVectorShuffle %v2bool %159 %159 0 2
%161 = OpCompositeExtract %bool %160 0
%162 = OpCompositeExtract %bool %160 1
%163 = OpINotEqual %bool %int_0 %int_0
%164 = OpINotEqual %bool %int_1 %int_0
%165 = OpCompositeConstruct %v4bool %161 %162 %163 %164
%166 = OpVectorShuffle %v4bool %165 %165 0 2 1 3
OpStore %result %166
%167 = OpLoad %v4bool %v
%168 = OpVectorShuffle %v2bool %167 %167 0 3
%169 = OpCompositeExtract %bool %168 0
%170 = OpCompositeExtract %bool %168 1
%171 = OpINotEqual %bool %int_1 %int_0
%172 = OpCompositeConstruct %v3bool %169 %170 %171
%173 = OpVectorShuffle %v4bool %172 %172 0 2 2 1
OpStore %result %173
%174 = OpLoad %v4bool %v
%175 = OpCompositeExtract %bool %174 0
%176 = OpINotEqual %bool %int_1 %int_0
%177 = OpINotEqual %bool %int_0 %int_0
%178 = OpCompositeConstruct %v3bool %175 %176 %177
%179 = OpVectorShuffle %v4bool %178 %178 0 1 2 1
OpStore %result %179
%180 = OpLoad %v4bool %v
%181 = OpVectorShuffle %v3bool %180 %180 1 2 3
%182 = OpCompositeExtract %bool %181 0
%183 = OpCompositeExtract %bool %181 1
%184 = OpCompositeExtract %bool %181 2
%185 = OpINotEqual %bool %int_1 %int_0
%186 = OpCompositeConstruct %v4bool %182 %183 %184 %185
%187 = OpVectorShuffle %v4bool %186 %186 3 0 1 2
OpStore %result %187
%188 = OpLoad %v4bool %v
%189 = OpVectorShuffle %v2bool %188 %188 1 2
%190 = OpCompositeExtract %bool %189 0
%191 = OpCompositeExtract %bool %189 1
%192 = OpINotEqual %bool %int_0 %int_0
%193 = OpINotEqual %bool %int_1 %int_0
%194 = OpCompositeConstruct %v4bool %190 %191 %192 %193
%195 = OpVectorShuffle %v4bool %194 %194 2 0 1 3
OpStore %result %195
%196 = OpLoad %v4bool %v
%197 = OpVectorShuffle %v2bool %196 %196 1 3
%198 = OpCompositeExtract %bool %197 0
%199 = OpCompositeExtract %bool %197 1
%200 = OpINotEqual %bool %int_0 %int_0
%201 = OpINotEqual %bool %int_1 %int_0
%202 = OpCompositeConstruct %v4bool %198 %199 %200 %201
%203 = OpVectorShuffle %v4bool %202 %202 2 0 3 1
OpStore %result %203
%204 = OpLoad %v4bool %v
%205 = OpCompositeExtract %bool %204 1
%206 = OpINotEqual %bool %int_1 %int_0
%207 = OpCompositeConstruct %v2bool %205 %206
%208 = OpVectorShuffle %v4bool %207 %207 1 0 1 1
OpStore %result %208
%209 = OpLoad %v4bool %v
%210 = OpVectorShuffle %v2bool %209 %209 2 3
%211 = OpCompositeExtract %bool %210 0
%212 = OpCompositeExtract %bool %210 1
%213 = OpINotEqual %bool %int_0 %int_0
%214 = OpCompositeConstruct %v3bool %211 %212 %213
%215 = OpVectorShuffle %v4bool %214 %214 2 2 0 1
OpStore %result %215
%216 = OpLoad %v4bool %v
%217 = OpCompositeExtract %bool %216 2
%218 = OpINotEqual %bool %int_0 %int_0
%219 = OpINotEqual %bool %int_1 %int_0
%220 = OpCompositeConstruct %v3bool %217 %218 %219
%221 = OpVectorShuffle %v4bool %220 %220 1 1 0 2
OpStore %result %221
%222 = OpLoad %v4bool %v
%223 = OpCompositeExtract %bool %222 3
%224 = OpINotEqual %bool %int_0 %int_0
%225 = OpINotEqual %bool %int_1 %int_0
%226 = OpCompositeConstruct %v3bool %223 %224 %225
%227 = OpVectorShuffle %v4bool %226 %226 1 2 2 0
OpStore %result %227
%229 = OpLoad %v4bool %result
%228 = OpAny %bool %229
OpSelectionMerge %234 None
OpBranchConditional %228 %232 %233
%232 = OpLabel
%235 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%236 = OpLoad %v4float %235
OpStore %230 %236
OpBranch %234
%233 = OpLabel
%237 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%238 = OpLoad %v4float %237
OpStore %230 %238
OpBranch %234
%234 = OpLabel
%239 = OpLoad %v4float %230
OpReturnValue %239
OpFunctionEnd
