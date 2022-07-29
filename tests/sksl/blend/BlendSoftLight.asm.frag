OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %soft_light_component_Qhh2h2 "soft_light_component_Qhh2h2"
OpName %DSqd "DSqd"
OpName %DCub "DCub"
OpName %DaSqd "DaSqd"
OpName %DaCub "DaCub"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
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
OpDecorate %DaSqd RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %DaCub RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
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
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %203 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%16 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_2 = OpConstant %float 2
%float_1 = OpConstant %float 1
%float_4 = OpConstant %float 4
%_ptr_Function_float = OpTypePointer Function %float
%float_3 = OpConstant %float 3
%float_6 = OpConstant %float 6
%float_12 = OpConstant %float 12
%float_16 = OpConstant %float 16
%void = OpTypeVoid
%175 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%float_0 = OpConstant %float 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_0 = OpConstant %int 0
%soft_light_component_Qhh2h2 = OpFunction %float None %16
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
%55 = OpLoad %v2float %17
%56 = OpCompositeExtract %float %55 1
%57 = OpFNegate %float %56
%58 = OpLoad %v2float %17
%59 = OpCompositeExtract %float %58 0
%60 = OpFMul %float %float_2 %59
%61 = OpFAdd %float %57 %60
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
%83 = OpLoad %v2float %18
%84 = OpCompositeExtract %float %83 0
%85 = OpFMul %float %81 %84
OpStore %DCub %85
%87 = OpLoad %v2float %18
%88 = OpCompositeExtract %float %87 1
%89 = OpLoad %v2float %18
%90 = OpCompositeExtract %float %89 1
%91 = OpFMul %float %88 %90
OpStore %DaSqd %91
%93 = OpLoad %v2float %18
%94 = OpCompositeExtract %float %93 1
%95 = OpFMul %float %91 %94
OpStore %DaCub %95
%96 = OpLoad %v2float %17
%97 = OpCompositeExtract %float %96 0
%98 = OpLoad %v2float %18
%99 = OpCompositeExtract %float %98 0
%101 = OpLoad %v2float %17
%102 = OpCompositeExtract %float %101 1
%103 = OpFMul %float %float_3 %102
%105 = OpLoad %v2float %17
%106 = OpCompositeExtract %float %105 0
%107 = OpFMul %float %float_6 %106
%108 = OpFSub %float %103 %107
%109 = OpFSub %float %108 %float_1
%110 = OpFMul %float %99 %109
%111 = OpFSub %float %97 %110
%112 = OpFMul %float %91 %111
%114 = OpLoad %v2float %18
%115 = OpCompositeExtract %float %114 1
%116 = OpFMul %float %float_12 %115
%117 = OpFMul %float %116 %81
%118 = OpLoad %v2float %17
%119 = OpCompositeExtract %float %118 1
%120 = OpLoad %v2float %17
%121 = OpCompositeExtract %float %120 0
%122 = OpFMul %float %float_2 %121
%123 = OpFSub %float %119 %122
%124 = OpFMul %float %117 %123
%125 = OpFAdd %float %112 %124
%127 = OpFMul %float %float_16 %85
%128 = OpLoad %v2float %17
%129 = OpCompositeExtract %float %128 1
%130 = OpLoad %v2float %17
%131 = OpCompositeExtract %float %130 0
%132 = OpFMul %float %float_2 %131
%133 = OpFSub %float %129 %132
%134 = OpFMul %float %127 %133
%135 = OpFSub %float %125 %134
%136 = OpLoad %v2float %17
%137 = OpCompositeExtract %float %136 0
%138 = OpFMul %float %95 %137
%139 = OpFSub %float %135 %138
%140 = OpFDiv %float %139 %91
OpReturnValue %140
%73 = OpLabel
%141 = OpLoad %v2float %18
%142 = OpCompositeExtract %float %141 0
%143 = OpLoad %v2float %17
%144 = OpCompositeExtract %float %143 1
%145 = OpLoad %v2float %17
%146 = OpCompositeExtract %float %145 0
%147 = OpFMul %float %float_2 %146
%148 = OpFSub %float %144 %147
%149 = OpFAdd %float %148 %float_1
%150 = OpFMul %float %142 %149
%151 = OpLoad %v2float %17
%152 = OpCompositeExtract %float %151 0
%153 = OpFAdd %float %150 %152
%155 = OpLoad %v2float %18
%156 = OpCompositeExtract %float %155 1
%157 = OpLoad %v2float %18
%158 = OpCompositeExtract %float %157 0
%159 = OpFMul %float %156 %158
%154 = OpExtInst %float %1 Sqrt %159
%160 = OpLoad %v2float %17
%161 = OpCompositeExtract %float %160 1
%162 = OpLoad %v2float %17
%163 = OpCompositeExtract %float %162 0
%164 = OpFMul %float %float_2 %163
%165 = OpFSub %float %161 %164
%166 = OpFMul %float %154 %165
%167 = OpFSub %float %153 %166
%168 = OpLoad %v2float %18
%169 = OpCompositeExtract %float %168 1
%170 = OpLoad %v2float %17
%171 = OpCompositeExtract %float %170 0
%172 = OpFMul %float %169 %171
%173 = OpFSub %float %167 %172
OpReturnValue %173
%74 = OpLabel
OpBranch %29
%29 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %175
%176 = OpLabel
%185 = OpVariable %_ptr_Function_v4float Function
%196 = OpVariable %_ptr_Function_v2float Function
%200 = OpVariable %_ptr_Function_v2float Function
%205 = OpVariable %_ptr_Function_v2float Function
%209 = OpVariable %_ptr_Function_v2float Function
%214 = OpVariable %_ptr_Function_v2float Function
%218 = OpVariable %_ptr_Function_v2float Function
%177 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%181 = OpLoad %v4float %177
%182 = OpCompositeExtract %float %181 3
%184 = OpFOrdEqual %bool %182 %float_0
OpSelectionMerge %189 None
OpBranchConditional %184 %187 %188
%187 = OpLabel
%190 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%192 = OpLoad %v4float %190
OpStore %185 %192
OpBranch %189
%188 = OpLabel
%193 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%194 = OpLoad %v4float %193
%195 = OpVectorShuffle %v2float %194 %194 0 3
OpStore %196 %195
%197 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%198 = OpLoad %v4float %197
%199 = OpVectorShuffle %v2float %198 %198 0 3
OpStore %200 %199
%201 = OpFunctionCall %float %soft_light_component_Qhh2h2 %196 %200
%202 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%203 = OpLoad %v4float %202
%204 = OpVectorShuffle %v2float %203 %203 1 3
OpStore %205 %204
%206 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%207 = OpLoad %v4float %206
%208 = OpVectorShuffle %v2float %207 %207 1 3
OpStore %209 %208
%210 = OpFunctionCall %float %soft_light_component_Qhh2h2 %205 %209
%211 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%212 = OpLoad %v4float %211
%213 = OpVectorShuffle %v2float %212 %212 2 3
OpStore %214 %213
%215 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%216 = OpLoad %v4float %215
%217 = OpVectorShuffle %v2float %216 %216 2 3
OpStore %218 %217
%219 = OpFunctionCall %float %soft_light_component_Qhh2h2 %214 %218
%220 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%221 = OpLoad %v4float %220
%222 = OpCompositeExtract %float %221 3
%223 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%224 = OpLoad %v4float %223
%225 = OpCompositeExtract %float %224 3
%226 = OpFSub %float %float_1 %225
%227 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%228 = OpLoad %v4float %227
%229 = OpCompositeExtract %float %228 3
%230 = OpFMul %float %226 %229
%231 = OpFAdd %float %222 %230
%232 = OpCompositeConstruct %v4float %201 %210 %219 %231
OpStore %185 %232
OpBranch %189
%189 = OpLabel
%233 = OpLoad %v4float %185
OpStore %sk_FragColor %233
OpReturn
OpFunctionEnd
