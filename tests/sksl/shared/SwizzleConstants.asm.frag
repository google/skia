OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %v "v"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %189 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %202 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%v2float = OpTypeVector %float 2
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%float_0 = OpConstant %float 0
%223 = OpConstantComposite %v4float %float_0 %float_1 %float_1 %float_1
%v4bool = OpTypeVector %bool 4
%int_2 = OpConstant %int 2
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%227 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
OpStore %v %26
%27 = OpLoad %v4float %v
%28 = OpCompositeExtract %float %27 0
%30 = OpCompositeConstruct %v4float %28 %float_1 %float_1 %float_1
OpStore %v %30
%31 = OpLoad %v4float %v
%32 = OpVectorShuffle %v2float %31 %31 0 1
%34 = OpCompositeExtract %float %32 0
%35 = OpCompositeExtract %float %32 1
%36 = OpCompositeConstruct %v4float %34 %35 %float_1 %float_1
OpStore %v %36
%37 = OpLoad %v4float %v
%38 = OpCompositeExtract %float %37 0
%40 = OpConvertSToF %float %int_1
%41 = OpCompositeConstruct %v2float %38 %40
%42 = OpCompositeExtract %float %41 0
%43 = OpCompositeExtract %float %41 1
%44 = OpCompositeConstruct %v4float %42 %43 %float_1 %float_1
OpStore %v %44
%45 = OpLoad %v4float %v
%46 = OpCompositeExtract %float %45 1
%47 = OpConvertSToF %float %int_0
%48 = OpCompositeConstruct %v2float %46 %47
%49 = OpVectorShuffle %v2float %48 %48 1 0
%50 = OpCompositeExtract %float %49 0
%51 = OpCompositeExtract %float %49 1
%52 = OpCompositeConstruct %v4float %50 %51 %float_1 %float_1
OpStore %v %52
%53 = OpLoad %v4float %v
%54 = OpVectorShuffle %v3float %53 %53 0 1 2
%56 = OpCompositeExtract %float %54 0
%57 = OpCompositeExtract %float %54 1
%58 = OpCompositeExtract %float %54 2
%59 = OpCompositeConstruct %v4float %56 %57 %58 %float_1
OpStore %v %59
%60 = OpLoad %v4float %v
%61 = OpVectorShuffle %v2float %60 %60 0 1
%62 = OpCompositeExtract %float %61 0
%63 = OpCompositeExtract %float %61 1
%64 = OpConvertSToF %float %int_1
%65 = OpCompositeConstruct %v3float %62 %63 %64
%66 = OpCompositeExtract %float %65 0
%67 = OpCompositeExtract %float %65 1
%68 = OpCompositeExtract %float %65 2
%69 = OpCompositeConstruct %v4float %66 %67 %68 %float_1
OpStore %v %69
%70 = OpLoad %v4float %v
%71 = OpVectorShuffle %v2float %70 %70 0 2
%72 = OpCompositeExtract %float %71 0
%73 = OpCompositeExtract %float %71 1
%74 = OpConvertSToF %float %int_0
%75 = OpCompositeConstruct %v3float %72 %73 %74
%76 = OpVectorShuffle %v3float %75 %75 0 2 1
%77 = OpCompositeExtract %float %76 0
%78 = OpCompositeExtract %float %76 1
%79 = OpCompositeExtract %float %76 2
%80 = OpCompositeConstruct %v4float %77 %78 %79 %float_1
OpStore %v %80
%81 = OpLoad %v4float %v
%82 = OpCompositeExtract %float %81 0
%83 = OpConvertSToF %float %int_1
%84 = OpConvertSToF %float %int_0
%85 = OpCompositeConstruct %v3float %82 %83 %84
%86 = OpCompositeExtract %float %85 0
%87 = OpCompositeExtract %float %85 1
%88 = OpCompositeExtract %float %85 2
%89 = OpCompositeConstruct %v4float %86 %87 %88 %float_1
OpStore %v %89
%90 = OpLoad %v4float %v
%91 = OpVectorShuffle %v2float %90 %90 1 2
%92 = OpCompositeExtract %float %91 0
%93 = OpCompositeExtract %float %91 1
%94 = OpConvertSToF %float %int_1
%95 = OpCompositeConstruct %v3float %92 %93 %94
%96 = OpVectorShuffle %v3float %95 %95 2 0 1
%97 = OpCompositeExtract %float %96 0
%98 = OpCompositeExtract %float %96 1
%99 = OpCompositeExtract %float %96 2
%100 = OpCompositeConstruct %v4float %97 %98 %99 %float_1
OpStore %v %100
%101 = OpLoad %v4float %v
%102 = OpCompositeExtract %float %101 1
%103 = OpConvertSToF %float %int_0
%104 = OpConvertSToF %float %int_1
%105 = OpCompositeConstruct %v3float %102 %103 %104
%106 = OpVectorShuffle %v3float %105 %105 1 0 2
%107 = OpCompositeExtract %float %106 0
%108 = OpCompositeExtract %float %106 1
%109 = OpCompositeExtract %float %106 2
%110 = OpCompositeConstruct %v4float %107 %108 %109 %float_1
OpStore %v %110
%111 = OpLoad %v4float %v
%112 = OpCompositeExtract %float %111 2
%113 = OpConvertSToF %float %int_1
%114 = OpCompositeConstruct %v2float %112 %113
%115 = OpVectorShuffle %v3float %114 %114 1 1 0
%116 = OpCompositeExtract %float %115 0
%117 = OpCompositeExtract %float %115 1
%118 = OpCompositeExtract %float %115 2
%119 = OpCompositeConstruct %v4float %116 %117 %118 %float_1
OpStore %v %119
%120 = OpLoad %v4float %v
%121 = OpVectorShuffle %v4float %120 %120 0 1 2 3
OpStore %v %121
%122 = OpLoad %v4float %v
%123 = OpVectorShuffle %v3float %122 %122 0 1 2
%124 = OpCompositeExtract %float %123 0
%125 = OpCompositeExtract %float %123 1
%126 = OpCompositeExtract %float %123 2
%127 = OpConvertSToF %float %int_1
%128 = OpCompositeConstruct %v4float %124 %125 %126 %127
OpStore %v %128
%129 = OpLoad %v4float %v
%130 = OpVectorShuffle %v3float %129 %129 0 1 3
%131 = OpCompositeExtract %float %130 0
%132 = OpCompositeExtract %float %130 1
%133 = OpCompositeExtract %float %130 2
%134 = OpConvertSToF %float %int_0
%135 = OpCompositeConstruct %v4float %131 %132 %133 %134
%136 = OpVectorShuffle %v4float %135 %135 0 1 3 2
OpStore %v %136
%137 = OpLoad %v4float %v
%138 = OpVectorShuffle %v2float %137 %137 0 1
%139 = OpCompositeExtract %float %138 0
%140 = OpCompositeExtract %float %138 1
%141 = OpConvertSToF %float %int_1
%142 = OpConvertSToF %float %int_0
%143 = OpCompositeConstruct %v4float %139 %140 %141 %142
OpStore %v %143
%144 = OpLoad %v4float %v
%145 = OpVectorShuffle %v3float %144 %144 0 2 3
%146 = OpCompositeExtract %float %145 0
%147 = OpCompositeExtract %float %145 1
%148 = OpCompositeExtract %float %145 2
%149 = OpConvertSToF %float %int_1
%150 = OpCompositeConstruct %v4float %146 %147 %148 %149
%151 = OpVectorShuffle %v4float %150 %150 0 3 1 2
OpStore %v %151
%152 = OpLoad %v4float %v
%153 = OpVectorShuffle %v2float %152 %152 0 2
%154 = OpCompositeExtract %float %153 0
%155 = OpCompositeExtract %float %153 1
%156 = OpConvertSToF %float %int_0
%157 = OpConvertSToF %float %int_1
%158 = OpCompositeConstruct %v4float %154 %155 %156 %157
%159 = OpVectorShuffle %v4float %158 %158 0 2 1 3
OpStore %v %159
%160 = OpLoad %v4float %v
%161 = OpVectorShuffle %v2float %160 %160 0 3
%162 = OpCompositeExtract %float %161 0
%163 = OpCompositeExtract %float %161 1
%164 = OpConvertSToF %float %int_1
%165 = OpCompositeConstruct %v3float %162 %163 %164
%166 = OpVectorShuffle %v4float %165 %165 0 2 2 1
OpStore %v %166
%167 = OpLoad %v4float %v
%168 = OpCompositeExtract %float %167 0
%169 = OpConvertSToF %float %int_1
%170 = OpConvertSToF %float %int_0
%171 = OpCompositeConstruct %v3float %168 %169 %170
%172 = OpVectorShuffle %v4float %171 %171 0 1 2 1
OpStore %v %172
%173 = OpLoad %v4float %v
%174 = OpVectorShuffle %v3float %173 %173 1 2 3
%175 = OpCompositeExtract %float %174 0
%176 = OpCompositeExtract %float %174 1
%177 = OpCompositeExtract %float %174 2
%178 = OpConvertSToF %float %int_1
%179 = OpCompositeConstruct %v4float %175 %176 %177 %178
%180 = OpVectorShuffle %v4float %179 %179 3 0 1 2
OpStore %v %180
%181 = OpLoad %v4float %v
%182 = OpVectorShuffle %v2float %181 %181 1 2
%183 = OpCompositeExtract %float %182 0
%184 = OpCompositeExtract %float %182 1
%185 = OpConvertSToF %float %int_0
%186 = OpConvertSToF %float %int_1
%187 = OpCompositeConstruct %v4float %183 %184 %185 %186
%188 = OpVectorShuffle %v4float %187 %187 2 0 1 3
OpStore %v %188
%189 = OpLoad %v4float %v
%190 = OpVectorShuffle %v2float %189 %189 1 3
%191 = OpCompositeExtract %float %190 0
%192 = OpCompositeExtract %float %190 1
%193 = OpConvertSToF %float %int_0
%194 = OpConvertSToF %float %int_1
%195 = OpCompositeConstruct %v4float %191 %192 %193 %194
%196 = OpVectorShuffle %v4float %195 %195 2 0 3 1
OpStore %v %196
%197 = OpLoad %v4float %v
%198 = OpCompositeExtract %float %197 1
%199 = OpConvertSToF %float %int_1
%200 = OpCompositeConstruct %v2float %198 %199
%201 = OpVectorShuffle %v4float %200 %200 1 0 1 1
OpStore %v %201
%202 = OpLoad %v4float %v
%203 = OpVectorShuffle %v2float %202 %202 2 3
%204 = OpCompositeExtract %float %203 0
%205 = OpCompositeExtract %float %203 1
%206 = OpConvertSToF %float %int_0
%207 = OpCompositeConstruct %v3float %204 %205 %206
%208 = OpVectorShuffle %v4float %207 %207 2 2 0 1
OpStore %v %208
%209 = OpLoad %v4float %v
%210 = OpCompositeExtract %float %209 2
%211 = OpConvertSToF %float %int_0
%212 = OpConvertSToF %float %int_1
%213 = OpCompositeConstruct %v3float %210 %211 %212
%214 = OpVectorShuffle %v4float %213 %213 1 1 0 2
OpStore %v %214
%215 = OpLoad %v4float %v
%216 = OpCompositeExtract %float %215 3
%217 = OpConvertSToF %float %int_0
%218 = OpConvertSToF %float %int_1
%219 = OpCompositeConstruct %v3float %216 %217 %218
%220 = OpVectorShuffle %v4float %219 %219 1 2 2 0
OpStore %v %220
%221 = OpLoad %v4float %v
%224 = OpFOrdEqual %v4bool %221 %223
%226 = OpAll %bool %224
OpSelectionMerge %230 None
OpBranchConditional %226 %228 %229
%228 = OpLabel
%231 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%232 = OpLoad %v4float %231
OpStore %227 %232
OpBranch %230
%229 = OpLabel
%233 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%235 = OpLoad %v4float %233
OpStore %227 %235
OpBranch %230
%230 = OpLabel
%236 = OpLoad %v4float %227
OpReturnValue %236
OpFunctionEnd
