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
OpName %check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3 "check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3"
OpName %main "main"
OpName %v1 "v1"
OpName %v2 "v2"
OpName %v3 "v3"
OpName %v4 "v4"
OpName %v5 "v5"
OpName %v6 "v6"
OpName %v7 "v7"
OpName %v8 "v8"
OpName %v9 "v9"
OpName %v10 "v10"
OpName %v11 "v11"
OpName %v12 "v12"
OpName %v13 "v13"
OpName %v14 "v14"
OpName %v15 "v15"
OpName %v16 "v16"
OpName %v17 "v17"
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
OpDecorate %56 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
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
%void = OpTypeVoid
%16 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%v3float = OpTypeVector %float 3
%int = OpTypeInt 32 1
%v2int = OpTypeVector %int 2
%v4bool = OpTypeVector %bool 4
%v2bool = OpTypeVector %bool 2
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v3float = OpTypePointer Function %v3float
%_ptr_Function_v2int = OpTypePointer Function %v2int
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%26 = OpTypeFunction %bool %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v3float %_ptr_Function_v2int %_ptr_Function_v2int %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v4float %_ptr_Function_v2int %_ptr_Function_v4bool %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v2bool %_ptr_Function_v2bool %_ptr_Function_v3bool
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%float_17 = OpConstant %float 17
%113 = OpTypeFunction %v4float
%float_2 = OpConstant %float 2
%119 = OpConstantComposite %v2float %float_1 %float_2
%123 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%135 = OpConstantComposite %v2int %int_1 %int_2
%int_3 = OpConstant %int 3
%int_4 = OpConstant %int 4
%155 = OpConstantComposite %v2int %int_3 %int_4
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%172 = OpConstantComposite %v4bool %true %false %true %false
%174 = OpConstantComposite %v2float %float_1 %float_0
%194 = OpConstantComposite %v3bool %true %true %true
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%18 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %18
OpReturn
OpFunctionEnd
%check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3 = OpFunction %bool None %26
%34 = OpFunctionParameter %_ptr_Function_v2float
%35 = OpFunctionParameter %_ptr_Function_v2float
%36 = OpFunctionParameter %_ptr_Function_v2float
%37 = OpFunctionParameter %_ptr_Function_v3float
%38 = OpFunctionParameter %_ptr_Function_v2int
%39 = OpFunctionParameter %_ptr_Function_v2int
%40 = OpFunctionParameter %_ptr_Function_v2float
%41 = OpFunctionParameter %_ptr_Function_v2float
%42 = OpFunctionParameter %_ptr_Function_v4float
%43 = OpFunctionParameter %_ptr_Function_v2int
%44 = OpFunctionParameter %_ptr_Function_v4bool
%45 = OpFunctionParameter %_ptr_Function_v2float
%46 = OpFunctionParameter %_ptr_Function_v2float
%47 = OpFunctionParameter %_ptr_Function_v2float
%48 = OpFunctionParameter %_ptr_Function_v2bool
%49 = OpFunctionParameter %_ptr_Function_v2bool
%50 = OpFunctionParameter %_ptr_Function_v3bool
%51 = OpLabel
%52 = OpLoad %v2float %34
%53 = OpCompositeExtract %float %52 0
%54 = OpLoad %v2float %35
%55 = OpCompositeExtract %float %54 0
%56 = OpFAdd %float %53 %55
%57 = OpLoad %v2float %36
%58 = OpCompositeExtract %float %57 0
%59 = OpFAdd %float %56 %58
%60 = OpLoad %v3float %37
%61 = OpCompositeExtract %float %60 0
%62 = OpFAdd %float %59 %61
%63 = OpLoad %v2int %38
%64 = OpCompositeExtract %int %63 0
%65 = OpConvertSToF %float %64
%66 = OpFAdd %float %62 %65
%67 = OpLoad %v2int %39
%68 = OpCompositeExtract %int %67 0
%69 = OpConvertSToF %float %68
%70 = OpFAdd %float %66 %69
%71 = OpLoad %v2float %40
%72 = OpCompositeExtract %float %71 0
%73 = OpFAdd %float %70 %72
%74 = OpLoad %v2float %41
%75 = OpCompositeExtract %float %74 0
%76 = OpFAdd %float %73 %75
%77 = OpLoad %v4float %42
%78 = OpCompositeExtract %float %77 0
%79 = OpFAdd %float %76 %78
%80 = OpLoad %v2int %43
%81 = OpCompositeExtract %int %80 0
%82 = OpConvertSToF %float %81
%83 = OpFAdd %float %79 %82
%84 = OpLoad %v4bool %44
%85 = OpCompositeExtract %bool %84 0
%86 = OpSelect %float %85 %float_1 %float_0
%89 = OpFAdd %float %83 %86
%90 = OpLoad %v2float %45
%91 = OpCompositeExtract %float %90 0
%92 = OpFAdd %float %89 %91
%93 = OpLoad %v2float %46
%94 = OpCompositeExtract %float %93 0
%95 = OpFAdd %float %92 %94
%96 = OpLoad %v2float %47
%97 = OpCompositeExtract %float %96 0
%98 = OpFAdd %float %95 %97
%99 = OpLoad %v2bool %48
%100 = OpCompositeExtract %bool %99 0
%101 = OpSelect %float %100 %float_1 %float_0
%102 = OpFAdd %float %98 %101
%103 = OpLoad %v2bool %49
%104 = OpCompositeExtract %bool %103 0
%105 = OpSelect %float %104 %float_1 %float_0
%106 = OpFAdd %float %102 %105
%107 = OpLoad %v3bool %50
%108 = OpCompositeExtract %bool %107 0
%109 = OpSelect %float %108 %float_1 %float_0
%110 = OpFAdd %float %106 %109
%112 = OpFOrdEqual %bool %110 %float_17
OpReturnValue %112
OpFunctionEnd
%main = OpFunction %v4float None %113
%114 = OpLabel
%v1 = OpVariable %_ptr_Function_v2float Function
%v2 = OpVariable %_ptr_Function_v2float Function
%v3 = OpVariable %_ptr_Function_v2float Function
%v4 = OpVariable %_ptr_Function_v3float Function
%v5 = OpVariable %_ptr_Function_v2int Function
%v6 = OpVariable %_ptr_Function_v2int Function
%v7 = OpVariable %_ptr_Function_v2float Function
%v8 = OpVariable %_ptr_Function_v2float Function
%v9 = OpVariable %_ptr_Function_v4float Function
%v10 = OpVariable %_ptr_Function_v2int Function
%v11 = OpVariable %_ptr_Function_v4bool Function
%v12 = OpVariable %_ptr_Function_v2float Function
%v13 = OpVariable %_ptr_Function_v2float Function
%v14 = OpVariable %_ptr_Function_v2float Function
%v15 = OpVariable %_ptr_Function_v2bool Function
%v16 = OpVariable %_ptr_Function_v2bool Function
%v17 = OpVariable %_ptr_Function_v3bool Function
%196 = OpVariable %_ptr_Function_v2float Function
%198 = OpVariable %_ptr_Function_v2float Function
%200 = OpVariable %_ptr_Function_v2float Function
%202 = OpVariable %_ptr_Function_v3float Function
%204 = OpVariable %_ptr_Function_v2int Function
%206 = OpVariable %_ptr_Function_v2int Function
%208 = OpVariable %_ptr_Function_v2float Function
%210 = OpVariable %_ptr_Function_v2float Function
%212 = OpVariable %_ptr_Function_v4float Function
%214 = OpVariable %_ptr_Function_v2int Function
%216 = OpVariable %_ptr_Function_v4bool Function
%218 = OpVariable %_ptr_Function_v2float Function
%220 = OpVariable %_ptr_Function_v2float Function
%222 = OpVariable %_ptr_Function_v2float Function
%224 = OpVariable %_ptr_Function_v2bool Function
%226 = OpVariable %_ptr_Function_v2bool Function
%228 = OpVariable %_ptr_Function_v3bool Function
%230 = OpVariable %_ptr_Function_v4float Function
%116 = OpCompositeConstruct %v2float %float_1 %float_1
OpStore %v1 %116
OpStore %v2 %119
%121 = OpCompositeConstruct %v2float %float_1 %float_1
OpStore %v3 %121
OpStore %v4 %123
%126 = OpCompositeConstruct %v2int %int_1 %int_1
OpStore %v5 %126
%128 = OpCompositeExtract %float %119 0
%129 = OpConvertFToS %int %128
%130 = OpCompositeExtract %float %119 1
%131 = OpConvertFToS %int %130
%132 = OpCompositeConstruct %v2int %129 %131
OpStore %v6 %132
%136 = OpCompositeExtract %int %135 0
%137 = OpConvertSToF %float %136
%138 = OpCompositeExtract %int %135 1
%139 = OpConvertSToF %float %138
%140 = OpCompositeConstruct %v2float %137 %139
OpStore %v7 %140
%142 = OpLoad %v2int %v5
%143 = OpCompositeExtract %int %142 0
%144 = OpConvertSToF %float %143
%145 = OpCompositeExtract %int %142 1
%146 = OpConvertSToF %float %145
%147 = OpCompositeConstruct %v2float %144 %146
OpStore %v8 %147
%149 = OpLoad %v2int %v6
%150 = OpCompositeExtract %int %149 0
%151 = OpConvertSToF %float %150
%152 = OpExtInst %float %1 Sqrt %float_2
%156 = OpCompositeExtract %int %155 0
%157 = OpConvertSToF %float %156
%158 = OpCompositeExtract %int %155 1
%159 = OpConvertSToF %float %158
%160 = OpCompositeConstruct %v2float %157 %159
%161 = OpCompositeExtract %float %160 0
%162 = OpCompositeExtract %float %160 1
%163 = OpCompositeConstruct %v4float %151 %152 %161 %162
OpStore %v9 %163
%165 = OpLoad %v2float %v1
%166 = OpCompositeExtract %float %165 0
%167 = OpConvertFToS %int %166
%168 = OpCompositeConstruct %v2int %int_3 %167
OpStore %v10 %168
OpStore %v11 %172
OpStore %v12 %174
%176 = OpCompositeConstruct %v2float %float_0 %float_0
OpStore %v13 %176
%178 = OpCompositeConstruct %v2bool %false %false
%179 = OpCompositeExtract %bool %178 0
%180 = OpSelect %float %179 %float_1 %float_0
%181 = OpCompositeExtract %bool %178 1
%182 = OpSelect %float %181 %float_1 %float_0
%183 = OpCompositeConstruct %v2float %180 %182
OpStore %v14 %183
%185 = OpCompositeConstruct %v2bool %true %true
OpStore %v15 %185
%187 = OpCompositeConstruct %v2float %float_1 %float_1
%188 = OpCompositeExtract %float %187 0
%189 = OpFUnordNotEqual %bool %188 %float_0
%190 = OpCompositeExtract %float %187 1
%191 = OpFUnordNotEqual %bool %190 %float_0
%192 = OpCompositeConstruct %v2bool %189 %191
OpStore %v16 %192
OpStore %v17 %194
%195 = OpLoad %v2float %v1
OpStore %196 %195
%197 = OpLoad %v2float %v2
OpStore %198 %197
%199 = OpLoad %v2float %v3
OpStore %200 %199
%201 = OpLoad %v3float %v4
OpStore %202 %201
%203 = OpLoad %v2int %v5
OpStore %204 %203
%205 = OpLoad %v2int %v6
OpStore %206 %205
%207 = OpLoad %v2float %v7
OpStore %208 %207
%209 = OpLoad %v2float %v8
OpStore %210 %209
%211 = OpLoad %v4float %v9
OpStore %212 %211
%213 = OpLoad %v2int %v10
OpStore %214 %213
%215 = OpLoad %v4bool %v11
OpStore %216 %215
%217 = OpLoad %v2float %v12
OpStore %218 %217
%219 = OpLoad %v2float %v13
OpStore %220 %219
%221 = OpLoad %v2float %v14
OpStore %222 %221
%223 = OpLoad %v2bool %v15
OpStore %224 %223
%225 = OpLoad %v2bool %v16
OpStore %226 %225
%227 = OpLoad %v3bool %v17
OpStore %228 %227
%229 = OpFunctionCall %bool %check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3 %196 %198 %200 %202 %204 %206 %208 %210 %212 %214 %216 %218 %220 %222 %224 %226 %228
OpSelectionMerge %233 None
OpBranchConditional %229 %231 %232
%231 = OpLabel
%234 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%237 = OpLoad %v4float %234
OpStore %230 %237
OpBranch %233
%232 = OpLabel
%238 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%239 = OpLoad %v4float %238
OpStore %230 %239
OpBranch %233
%233 = OpLabel
%240 = OpLoad %v4float %230
OpReturnValue %240
OpFunctionEnd
