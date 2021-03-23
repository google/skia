OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %_soft_light_component_hh2h2 "_soft_light_component_hh2h2"
OpName %DSqd "DSqd"
OpName %DCub "DCub"
OpName %DaSqd "DaSqd"
OpName %DaCub "DaCub"
OpName %main "main"
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
OpDecorate %21 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %DSqd RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %DCub RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %DaSqd RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %DaCub RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %151 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %166 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %175 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %201 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%15 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_2 = OpConstant %float 2
%float_1 = OpConstant %float 1
%float_4 = OpConstant %float 4
%_ptr_Function_float = OpTypePointer Function %float
%float_3 = OpConstant %float 3
%float_6 = OpConstant %float 6
%float_12 = OpConstant %float 12
%float_16 = OpConstant %float 16
%void = OpTypeVoid
%182 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%float_0 = OpConstant %float 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_0 = OpConstant %int 0
%_soft_light_component_hh2h2 = OpFunction %float None %15
%17 = OpFunctionParameter %_ptr_Function_v2float
%18 = OpFunctionParameter %_ptr_Function_v2float
%19 = OpLabel
%DSqd = OpVariable %_ptr_Function_float Function
%DCub = OpVariable %_ptr_Function_float Function
%DaSqd = OpVariable %_ptr_Function_float Function
%DaCub = OpVariable %_ptr_Function_float Function
%21 = OpLoad %v2float %17
%22 = OpCompositeExtract %float %21 0
%23 = OpFMul %float %float_2 %22
%24 = OpLoad %v2float %17
%25 = OpCompositeExtract %float %24 1
%26 = OpFOrdLessThanEqual %bool %23 %25
OpSelectionMerge %29 None
OpBranchConditional %26 %27 %28
%27 = OpLabel
%30 = OpLoad %v2float %18
%31 = OpCompositeExtract %float %30 0
%32 = OpLoad %v2float %18
%33 = OpCompositeExtract %float %32 0
%34 = OpFMul %float %31 %33
%35 = OpLoad %v2float %17
%36 = OpCompositeExtract %float %35 1
%37 = OpLoad %v2float %17
%38 = OpCompositeExtract %float %37 0
%39 = OpFMul %float %float_2 %38
%40 = OpFSub %float %36 %39
%41 = OpFMul %float %34 %40
%42 = OpLoad %v2float %18
%43 = OpCompositeExtract %float %42 1
%44 = OpFDiv %float %41 %43
%46 = OpLoad %v2float %18
%47 = OpCompositeExtract %float %46 1
%48 = OpFSub %float %float_1 %47
%49 = OpLoad %v2float %17
%50 = OpCompositeExtract %float %49 0
%51 = OpFMul %float %48 %50
%52 = OpFAdd %float %44 %51
%53 = OpLoad %v2float %18
%54 = OpCompositeExtract %float %53 0
%56 = OpLoad %v2float %17
%57 = OpCompositeExtract %float %56 1
%55 = OpFNegate %float %57
%58 = OpLoad %v2float %17
%59 = OpCompositeExtract %float %58 0
%60 = OpFMul %float %float_2 %59
%61 = OpFAdd %float %55 %60
%62 = OpFAdd %float %61 %float_1
%63 = OpFMul %float %54 %62
%64 = OpFAdd %float %52 %63
OpReturnValue %64
%28 = OpLabel
%66 = OpLoad %v2float %18
%67 = OpCompositeExtract %float %66 0
%68 = OpFMul %float %float_4 %67
%69 = OpLoad %v2float %18
%70 = OpCompositeExtract %float %69 1
%71 = OpFOrdLessThanEqual %bool %68 %70
OpSelectionMerge %74 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%77 = OpLoad %v2float %18
%78 = OpCompositeExtract %float %77 0
%79 = OpLoad %v2float %18
%80 = OpCompositeExtract %float %79 0
%81 = OpFMul %float %78 %80
OpStore %DSqd %81
%83 = OpLoad %float %DSqd
%84 = OpLoad %v2float %18
%85 = OpCompositeExtract %float %84 0
%86 = OpFMul %float %83 %85
OpStore %DCub %86
%88 = OpLoad %v2float %18
%89 = OpCompositeExtract %float %88 1
%90 = OpLoad %v2float %18
%91 = OpCompositeExtract %float %90 1
%92 = OpFMul %float %89 %91
OpStore %DaSqd %92
%94 = OpLoad %float %DaSqd
%95 = OpLoad %v2float %18
%96 = OpCompositeExtract %float %95 1
%97 = OpFMul %float %94 %96
OpStore %DaCub %97
%98 = OpLoad %float %DaSqd
%99 = OpLoad %v2float %17
%100 = OpCompositeExtract %float %99 0
%101 = OpLoad %v2float %18
%102 = OpCompositeExtract %float %101 0
%104 = OpLoad %v2float %17
%105 = OpCompositeExtract %float %104 1
%106 = OpFMul %float %float_3 %105
%108 = OpLoad %v2float %17
%109 = OpCompositeExtract %float %108 0
%110 = OpFMul %float %float_6 %109
%111 = OpFSub %float %106 %110
%112 = OpFSub %float %111 %float_1
%113 = OpFMul %float %102 %112
%114 = OpFSub %float %100 %113
%115 = OpFMul %float %98 %114
%117 = OpLoad %v2float %18
%118 = OpCompositeExtract %float %117 1
%119 = OpFMul %float %float_12 %118
%120 = OpLoad %float %DSqd
%121 = OpFMul %float %119 %120
%122 = OpLoad %v2float %17
%123 = OpCompositeExtract %float %122 1
%124 = OpLoad %v2float %17
%125 = OpCompositeExtract %float %124 0
%126 = OpFMul %float %float_2 %125
%127 = OpFSub %float %123 %126
%128 = OpFMul %float %121 %127
%129 = OpFAdd %float %115 %128
%131 = OpLoad %float %DCub
%132 = OpFMul %float %float_16 %131
%133 = OpLoad %v2float %17
%134 = OpCompositeExtract %float %133 1
%135 = OpLoad %v2float %17
%136 = OpCompositeExtract %float %135 0
%137 = OpFMul %float %float_2 %136
%138 = OpFSub %float %134 %137
%139 = OpFMul %float %132 %138
%140 = OpFSub %float %129 %139
%141 = OpLoad %float %DaCub
%142 = OpLoad %v2float %17
%143 = OpCompositeExtract %float %142 0
%144 = OpFMul %float %141 %143
%145 = OpFSub %float %140 %144
%146 = OpLoad %float %DaSqd
%147 = OpFDiv %float %145 %146
OpReturnValue %147
%73 = OpLabel
%148 = OpLoad %v2float %18
%149 = OpCompositeExtract %float %148 0
%150 = OpLoad %v2float %17
%151 = OpCompositeExtract %float %150 1
%152 = OpLoad %v2float %17
%153 = OpCompositeExtract %float %152 0
%154 = OpFMul %float %float_2 %153
%155 = OpFSub %float %151 %154
%156 = OpFAdd %float %155 %float_1
%157 = OpFMul %float %149 %156
%158 = OpLoad %v2float %17
%159 = OpCompositeExtract %float %158 0
%160 = OpFAdd %float %157 %159
%162 = OpLoad %v2float %18
%163 = OpCompositeExtract %float %162 1
%164 = OpLoad %v2float %18
%165 = OpCompositeExtract %float %164 0
%166 = OpFMul %float %163 %165
%161 = OpExtInst %float %1 Sqrt %166
%167 = OpLoad %v2float %17
%168 = OpCompositeExtract %float %167 1
%169 = OpLoad %v2float %17
%170 = OpCompositeExtract %float %169 0
%171 = OpFMul %float %float_2 %170
%172 = OpFSub %float %168 %171
%173 = OpFMul %float %161 %172
%174 = OpFSub %float %160 %173
%175 = OpLoad %v2float %18
%176 = OpCompositeExtract %float %175 1
%177 = OpLoad %v2float %17
%178 = OpCompositeExtract %float %177 0
%179 = OpFMul %float %176 %178
%180 = OpFSub %float %174 %179
OpReturnValue %180
%74 = OpLabel
OpBranch %29
%29 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %182
%183 = OpLabel
%192 = OpVariable %_ptr_Function_v4float Function
%203 = OpVariable %_ptr_Function_v2float Function
%207 = OpVariable %_ptr_Function_v2float Function
%212 = OpVariable %_ptr_Function_v2float Function
%216 = OpVariable %_ptr_Function_v2float Function
%221 = OpVariable %_ptr_Function_v2float Function
%225 = OpVariable %_ptr_Function_v2float Function
%184 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%188 = OpLoad %v4float %184
%189 = OpCompositeExtract %float %188 3
%191 = OpFOrdEqual %bool %189 %float_0
OpSelectionMerge %196 None
OpBranchConditional %191 %194 %195
%194 = OpLabel
%197 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%199 = OpLoad %v4float %197
OpStore %192 %199
OpBranch %196
%195 = OpLabel
%200 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%201 = OpLoad %v4float %200
%202 = OpVectorShuffle %v2float %201 %201 0 3
OpStore %203 %202
%204 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%205 = OpLoad %v4float %204
%206 = OpVectorShuffle %v2float %205 %205 0 3
OpStore %207 %206
%208 = OpFunctionCall %float %_soft_light_component_hh2h2 %203 %207
%209 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%210 = OpLoad %v4float %209
%211 = OpVectorShuffle %v2float %210 %210 1 3
OpStore %212 %211
%213 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%214 = OpLoad %v4float %213
%215 = OpVectorShuffle %v2float %214 %214 1 3
OpStore %216 %215
%217 = OpFunctionCall %float %_soft_light_component_hh2h2 %212 %216
%218 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%219 = OpLoad %v4float %218
%220 = OpVectorShuffle %v2float %219 %219 2 3
OpStore %221 %220
%222 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%223 = OpLoad %v4float %222
%224 = OpVectorShuffle %v2float %223 %223 2 3
OpStore %225 %224
%226 = OpFunctionCall %float %_soft_light_component_hh2h2 %221 %225
%227 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%228 = OpLoad %v4float %227
%229 = OpCompositeExtract %float %228 3
%230 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%231 = OpLoad %v4float %230
%232 = OpCompositeExtract %float %231 3
%233 = OpFSub %float %float_1 %232
%234 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%235 = OpLoad %v4float %234
%236 = OpCompositeExtract %float %235 3
%237 = OpFMul %float %233 %236
%238 = OpFAdd %float %229 %237
%239 = OpCompositeConstruct %v4float %208 %217 %226 %238
OpStore %192 %239
OpBranch %196
%196 = OpLabel
%240 = OpLoad %v4float %192
OpStore %sk_FragColor %240
OpReturn
OpFunctionEnd
