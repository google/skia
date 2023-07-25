               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %test_half_b "test_half_b"
               OpName %ok "ok"
               OpName %m23 "m23"
               OpName %m24 "m24"
               OpName %m32 "m32"
               OpName %m34 "m34"
               OpName %m42 "m42"
               OpName %m43 "m43"
               OpName %m22 "m22"
               OpName %m33 "m33"
               OpName %main "main"
               OpName %_0_ok "_0_ok"
               OpName %_1_m23 "_1_m23"
               OpName %_2_m24 "_2_m24"
               OpName %_3_m32 "_3_m32"
               OpName %_7_m22 "_7_m22"
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
               OpDecorate %m23 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %m24 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %m32 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %m34 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %m42 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %m43 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %m22 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %m33 RelaxedPrecision
               OpDecorate %166 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %190 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %205 RelaxedPrecision
               OpDecorate %206 RelaxedPrecision
               OpDecorate %207 RelaxedPrecision
               OpDecorate %208 RelaxedPrecision
               OpDecorate %216 RelaxedPrecision
               OpDecorate %218 RelaxedPrecision
               OpDecorate %221 RelaxedPrecision
               OpDecorate %226 RelaxedPrecision
               OpDecorate %233 RelaxedPrecision
               OpDecorate %234 RelaxedPrecision
               OpDecorate %236 RelaxedPrecision
               OpDecorate %237 RelaxedPrecision
               OpDecorate %337 RelaxedPrecision
               OpDecorate %340 RelaxedPrecision
               OpDecorate %341 RelaxedPrecision
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
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
    %v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
    %float_2 = OpConstant %float 2
         %34 = OpConstantComposite %v3float %float_2 %float_0 %float_0
         %35 = OpConstantComposite %v3float %float_0 %float_2 %float_0
         %36 = OpConstantComposite %mat2v3float %34 %35
      %false = OpConstantFalse %bool
     %v3bool = OpTypeVector %bool 3
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
    %float_3 = OpConstant %float 3
         %51 = OpConstantComposite %v4float %float_3 %float_0 %float_0 %float_0
         %52 = OpConstantComposite %v4float %float_0 %float_3 %float_0 %float_0
         %53 = OpConstantComposite %mat2v4float %51 %52
     %v4bool = OpTypeVector %bool 4
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
    %float_4 = OpConstant %float 4
         %67 = OpConstantComposite %v2float %float_4 %float_0
         %68 = OpConstantComposite %v2float %float_0 %float_4
         %69 = OpConstantComposite %mat3v2float %67 %68 %20
     %v2bool = OpTypeVector %bool 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
    %float_5 = OpConstant %float 5
         %86 = OpConstantComposite %v4float %float_5 %float_0 %float_0 %float_0
         %87 = OpConstantComposite %v4float %float_0 %float_5 %float_0 %float_0
         %88 = OpConstantComposite %v4float %float_0 %float_0 %float_5 %float_0
         %89 = OpConstantComposite %mat3v4float %86 %87 %88
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
    %float_6 = OpConstant %float 6
        %105 = OpConstantComposite %v2float %float_6 %float_0
        %106 = OpConstantComposite %v2float %float_0 %float_6
        %107 = OpConstantComposite %mat4v2float %105 %106 %20 %20
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
    %float_7 = OpConstant %float 7
        %126 = OpConstantComposite %v3float %float_7 %float_0 %float_0
        %127 = OpConstantComposite %v3float %float_0 %float_7 %float_0
        %128 = OpConstantComposite %v3float %float_0 %float_0 %float_7
        %129 = OpConstantComposite %v3float %float_0 %float_0 %float_0
        %130 = OpConstantComposite %mat4v3float %126 %127 %128 %129
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
    %float_8 = OpConstant %float 8
        %152 = OpConstantComposite %v2float %float_8 %float_0
        %153 = OpConstantComposite %v2float %float_0 %float_8
        %154 = OpConstantComposite %mat2v2float %152 %153
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
   %float_35 = OpConstant %float 35
        %170 = OpConstantComposite %v3float %float_35 %float_0 %float_0
        %171 = OpConstantComposite %v3float %float_0 %float_35 %float_0
        %172 = OpConstantComposite %v3float %float_0 %float_0 %float_35
        %173 = OpConstantComposite %mat3v3float %170 %171 %172
    %float_1 = OpConstant %float 1
        %187 = OpConstantComposite %v3float %float_1 %float_1 %float_1
        %188 = OpConstantComposite %mat2v3float %187 %187
        %194 = OpConstantComposite %v3float %float_3 %float_1 %float_1
        %195 = OpConstantComposite %v3float %float_1 %float_3 %float_1
        %196 = OpConstantComposite %mat2v3float %194 %195
        %203 = OpConstantComposite %v2float %float_2 %float_2
        %204 = OpConstantComposite %mat3v2float %203 %203 %203
   %float_n2 = OpConstant %float -2
        %212 = OpConstantComposite %v2float %float_2 %float_n2
        %213 = OpConstantComposite %v2float %float_n2 %float_2
        %214 = OpConstantComposite %v2float %float_n2 %float_n2
        %215 = OpConstantComposite %mat3v2float %212 %213 %214
 %float_0_25 = OpConstant %float 0.25
 %float_0_75 = OpConstant %float 0.75
        %230 = OpConstantComposite %v4float %float_0_75 %float_0 %float_0 %float_0
        %231 = OpConstantComposite %v4float %float_0 %float_0_75 %float_0 %float_0
        %232 = OpConstantComposite %mat2v4float %230 %231
        %241 = OpTypeFunction %v4float %_ptr_Function_v2float
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
        %m23 = OpVariable %_ptr_Function_mat2v3float Function
        %m24 = OpVariable %_ptr_Function_mat2v4float Function
        %m32 = OpVariable %_ptr_Function_mat3v2float Function
        %m34 = OpVariable %_ptr_Function_mat3v4float Function
        %m42 = OpVariable %_ptr_Function_mat4v2float Function
        %m43 = OpVariable %_ptr_Function_mat4v3float Function
        %m22 = OpVariable %_ptr_Function_mat2v2float Function
        %m33 = OpVariable %_ptr_Function_mat3v3float Function
               OpStore %ok %true
               OpStore %m23 %36
               OpSelectionMerge %39 None
               OpBranchConditional %true %38 %39
         %38 = OpLabel
         %41 = OpFOrdEqual %v3bool %34 %34
         %42 = OpAll %bool %41
         %43 = OpFOrdEqual %v3bool %35 %35
         %44 = OpAll %bool %43
         %45 = OpLogicalAnd %bool %42 %44
               OpBranch %39
         %39 = OpLabel
         %46 = OpPhi %bool %false %25 %45 %38
               OpStore %ok %46
               OpStore %m24 %53
               OpSelectionMerge %55 None
               OpBranchConditional %46 %54 %55
         %54 = OpLabel
         %57 = OpFOrdEqual %v4bool %51 %51
         %58 = OpAll %bool %57
         %59 = OpFOrdEqual %v4bool %52 %52
         %60 = OpAll %bool %59
         %61 = OpLogicalAnd %bool %58 %60
               OpBranch %55
         %55 = OpLabel
         %62 = OpPhi %bool %false %39 %61 %54
               OpStore %ok %62
               OpStore %m32 %69
               OpSelectionMerge %71 None
               OpBranchConditional %62 %70 %71
         %70 = OpLabel
         %73 = OpFOrdEqual %v2bool %67 %67
         %74 = OpAll %bool %73
         %75 = OpFOrdEqual %v2bool %68 %68
         %76 = OpAll %bool %75
         %77 = OpLogicalAnd %bool %74 %76
         %78 = OpFOrdEqual %v2bool %20 %20
         %79 = OpAll %bool %78
         %80 = OpLogicalAnd %bool %77 %79
               OpBranch %71
         %71 = OpLabel
         %81 = OpPhi %bool %false %55 %80 %70
               OpStore %ok %81
               OpStore %m34 %89
               OpSelectionMerge %91 None
               OpBranchConditional %81 %90 %91
         %90 = OpLabel
         %92 = OpFOrdEqual %v4bool %86 %86
         %93 = OpAll %bool %92
         %94 = OpFOrdEqual %v4bool %87 %87
         %95 = OpAll %bool %94
         %96 = OpLogicalAnd %bool %93 %95
         %97 = OpFOrdEqual %v4bool %88 %88
         %98 = OpAll %bool %97
         %99 = OpLogicalAnd %bool %96 %98
               OpBranch %91
         %91 = OpLabel
        %100 = OpPhi %bool %false %71 %99 %90
               OpStore %ok %100
               OpStore %m42 %107
               OpSelectionMerge %109 None
               OpBranchConditional %100 %108 %109
        %108 = OpLabel
        %110 = OpFOrdEqual %v2bool %105 %105
        %111 = OpAll %bool %110
        %112 = OpFOrdEqual %v2bool %106 %106
        %113 = OpAll %bool %112
        %114 = OpLogicalAnd %bool %111 %113
        %115 = OpFOrdEqual %v2bool %20 %20
        %116 = OpAll %bool %115
        %117 = OpLogicalAnd %bool %114 %116
        %118 = OpFOrdEqual %v2bool %20 %20
        %119 = OpAll %bool %118
        %120 = OpLogicalAnd %bool %117 %119
               OpBranch %109
        %109 = OpLabel
        %121 = OpPhi %bool %false %91 %120 %108
               OpStore %ok %121
               OpStore %m43 %130
               OpSelectionMerge %132 None
               OpBranchConditional %121 %131 %132
        %131 = OpLabel
        %133 = OpFOrdEqual %v3bool %126 %126
        %134 = OpAll %bool %133
        %135 = OpFOrdEqual %v3bool %127 %127
        %136 = OpAll %bool %135
        %137 = OpLogicalAnd %bool %134 %136
        %138 = OpFOrdEqual %v3bool %128 %128
        %139 = OpAll %bool %138
        %140 = OpLogicalAnd %bool %137 %139
        %141 = OpFOrdEqual %v3bool %129 %129
        %142 = OpAll %bool %141
        %143 = OpLogicalAnd %bool %140 %142
               OpBranch %132
        %132 = OpLabel
        %144 = OpPhi %bool %false %109 %143 %131
               OpStore %ok %144
        %148 = OpMatrixTimesMatrix %mat2v2float %69 %36
               OpStore %m22 %148
               OpSelectionMerge %150 None
               OpBranchConditional %144 %149 %150
        %149 = OpLabel
        %155 = OpCompositeExtract %v2float %148 0
        %156 = OpFOrdEqual %v2bool %155 %152
        %157 = OpAll %bool %156
        %158 = OpCompositeExtract %v2float %148 1
        %159 = OpFOrdEqual %v2bool %158 %153
        %160 = OpAll %bool %159
        %161 = OpLogicalAnd %bool %157 %160
               OpBranch %150
        %150 = OpLabel
        %162 = OpPhi %bool %false %132 %161 %149
               OpStore %ok %162
        %166 = OpMatrixTimesMatrix %mat3v3float %130 %89
               OpStore %m33 %166
               OpSelectionMerge %168 None
               OpBranchConditional %162 %167 %168
        %167 = OpLabel
        %174 = OpCompositeExtract %v3float %166 0
        %175 = OpFOrdEqual %v3bool %174 %170
        %176 = OpAll %bool %175
        %177 = OpCompositeExtract %v3float %166 1
        %178 = OpFOrdEqual %v3bool %177 %171
        %179 = OpAll %bool %178
        %180 = OpLogicalAnd %bool %176 %179
        %181 = OpCompositeExtract %v3float %166 2
        %182 = OpFOrdEqual %v3bool %181 %172
        %183 = OpAll %bool %182
        %184 = OpLogicalAnd %bool %180 %183
               OpBranch %168
        %168 = OpLabel
        %185 = OpPhi %bool %false %150 %184 %167
               OpStore %ok %185
        %189 = OpFAdd %v3float %34 %187
        %190 = OpFAdd %v3float %35 %187
        %191 = OpCompositeConstruct %mat2v3float %189 %190
               OpStore %m23 %191
               OpSelectionMerge %193 None
               OpBranchConditional %185 %192 %193
        %192 = OpLabel
        %197 = OpFOrdEqual %v3bool %189 %194
        %198 = OpAll %bool %197
        %199 = OpFOrdEqual %v3bool %190 %195
        %200 = OpAll %bool %199
        %201 = OpLogicalAnd %bool %198 %200
               OpBranch %193
        %193 = OpLabel
        %202 = OpPhi %bool %false %168 %201 %192
               OpStore %ok %202
        %205 = OpFSub %v2float %67 %203
        %206 = OpFSub %v2float %68 %203
        %207 = OpFSub %v2float %20 %203
        %208 = OpCompositeConstruct %mat3v2float %205 %206 %207
               OpStore %m32 %208
               OpSelectionMerge %210 None
               OpBranchConditional %202 %209 %210
        %209 = OpLabel
        %216 = OpFOrdEqual %v2bool %205 %212
        %217 = OpAll %bool %216
        %218 = OpFOrdEqual %v2bool %206 %213
        %219 = OpAll %bool %218
        %220 = OpLogicalAnd %bool %217 %219
        %221 = OpFOrdEqual %v2bool %207 %214
        %222 = OpAll %bool %221
        %223 = OpLogicalAnd %bool %220 %222
               OpBranch %210
        %210 = OpLabel
        %224 = OpPhi %bool %false %193 %223 %209
               OpStore %ok %224
        %226 = OpMatrixTimesScalar %mat2v4float %53 %float_0_25
               OpStore %m24 %226
               OpSelectionMerge %228 None
               OpBranchConditional %224 %227 %228
        %227 = OpLabel
        %233 = OpCompositeExtract %v4float %226 0
        %234 = OpFOrdEqual %v4bool %233 %230
        %235 = OpAll %bool %234
        %236 = OpCompositeExtract %v4float %226 1
        %237 = OpFOrdEqual %v4bool %236 %231
        %238 = OpAll %bool %237
        %239 = OpLogicalAnd %bool %235 %238
               OpBranch %228
        %228 = OpLabel
        %240 = OpPhi %bool %false %210 %239 %227
               OpStore %ok %240
               OpReturnValue %240
               OpFunctionEnd
       %main = OpFunction %v4float None %241
        %242 = OpFunctionParameter %_ptr_Function_v2float
        %243 = OpLabel
      %_0_ok = OpVariable %_ptr_Function_bool Function
     %_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
     %_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
     %_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
     %_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
        %328 = OpVariable %_ptr_Function_v4float Function
               OpStore %_0_ok %true
               OpStore %_1_m23 %36
               OpSelectionMerge %247 None
               OpBranchConditional %true %246 %247
        %246 = OpLabel
        %248 = OpFOrdEqual %v3bool %34 %34
        %249 = OpAll %bool %248
        %250 = OpFOrdEqual %v3bool %35 %35
        %251 = OpAll %bool %250
        %252 = OpLogicalAnd %bool %249 %251
               OpBranch %247
        %247 = OpLabel
        %253 = OpPhi %bool %false %243 %252 %246
               OpStore %_0_ok %253
               OpStore %_2_m24 %53
               OpSelectionMerge %256 None
               OpBranchConditional %253 %255 %256
        %255 = OpLabel
        %257 = OpFOrdEqual %v4bool %51 %51
        %258 = OpAll %bool %257
        %259 = OpFOrdEqual %v4bool %52 %52
        %260 = OpAll %bool %259
        %261 = OpLogicalAnd %bool %258 %260
               OpBranch %256
        %256 = OpLabel
        %262 = OpPhi %bool %false %247 %261 %255
               OpStore %_0_ok %262
               OpStore %_3_m32 %69
               OpSelectionMerge %265 None
               OpBranchConditional %262 %264 %265
        %264 = OpLabel
        %266 = OpFOrdEqual %v2bool %67 %67
        %267 = OpAll %bool %266
        %268 = OpFOrdEqual %v2bool %68 %68
        %269 = OpAll %bool %268
        %270 = OpLogicalAnd %bool %267 %269
        %271 = OpFOrdEqual %v2bool %20 %20
        %272 = OpAll %bool %271
        %273 = OpLogicalAnd %bool %270 %272
               OpBranch %265
        %265 = OpLabel
        %274 = OpPhi %bool %false %256 %273 %264
               OpStore %_0_ok %274
        %276 = OpMatrixTimesMatrix %mat2v2float %69 %36
               OpStore %_7_m22 %276
               OpSelectionMerge %278 None
               OpBranchConditional %274 %277 %278
        %277 = OpLabel
        %279 = OpCompositeExtract %v2float %276 0
        %280 = OpFOrdEqual %v2bool %279 %152
        %281 = OpAll %bool %280
        %282 = OpCompositeExtract %v2float %276 1
        %283 = OpFOrdEqual %v2bool %282 %153
        %284 = OpAll %bool %283
        %285 = OpLogicalAnd %bool %281 %284
               OpBranch %278
        %278 = OpLabel
        %286 = OpPhi %bool %false %265 %285 %277
               OpStore %_0_ok %286
        %287 = OpFAdd %v3float %34 %187
        %288 = OpFAdd %v3float %35 %187
        %289 = OpCompositeConstruct %mat2v3float %287 %288
               OpStore %_1_m23 %289
               OpSelectionMerge %291 None
               OpBranchConditional %286 %290 %291
        %290 = OpLabel
        %292 = OpFOrdEqual %v3bool %287 %194
        %293 = OpAll %bool %292
        %294 = OpFOrdEqual %v3bool %288 %195
        %295 = OpAll %bool %294
        %296 = OpLogicalAnd %bool %293 %295
               OpBranch %291
        %291 = OpLabel
        %297 = OpPhi %bool %false %278 %296 %290
               OpStore %_0_ok %297
        %298 = OpFSub %v2float %67 %203
        %299 = OpFSub %v2float %68 %203
        %300 = OpFSub %v2float %20 %203
        %301 = OpCompositeConstruct %mat3v2float %298 %299 %300
               OpStore %_3_m32 %301
               OpSelectionMerge %303 None
               OpBranchConditional %297 %302 %303
        %302 = OpLabel
        %304 = OpFOrdEqual %v2bool %298 %212
        %305 = OpAll %bool %304
        %306 = OpFOrdEqual %v2bool %299 %213
        %307 = OpAll %bool %306
        %308 = OpLogicalAnd %bool %305 %307
        %309 = OpFOrdEqual %v2bool %300 %214
        %310 = OpAll %bool %309
        %311 = OpLogicalAnd %bool %308 %310
               OpBranch %303
        %303 = OpLabel
        %312 = OpPhi %bool %false %291 %311 %302
               OpStore %_0_ok %312
        %313 = OpMatrixTimesScalar %mat2v4float %53 %float_0_25
               OpStore %_2_m24 %313
               OpSelectionMerge %315 None
               OpBranchConditional %312 %314 %315
        %314 = OpLabel
        %316 = OpCompositeExtract %v4float %313 0
        %317 = OpFOrdEqual %v4bool %316 %230
        %318 = OpAll %bool %317
        %319 = OpCompositeExtract %v4float %313 1
        %320 = OpFOrdEqual %v4bool %319 %231
        %321 = OpAll %bool %320
        %322 = OpLogicalAnd %bool %318 %321
               OpBranch %315
        %315 = OpLabel
        %323 = OpPhi %bool %false %303 %322 %314
               OpStore %_0_ok %323
               OpSelectionMerge %325 None
               OpBranchConditional %323 %324 %325
        %324 = OpLabel
        %326 = OpFunctionCall %bool %test_half_b
               OpBranch %325
        %325 = OpLabel
        %327 = OpPhi %bool %false %315 %326 %324
               OpSelectionMerge %332 None
               OpBranchConditional %327 %330 %331
        %330 = OpLabel
        %333 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %337 = OpLoad %v4float %333
               OpStore %328 %337
               OpBranch %332
        %331 = OpLabel
        %338 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %340 = OpLoad %v4float %338
               OpStore %328 %340
               OpBranch %332
        %332 = OpLabel
        %341 = OpLoad %v4float %328
               OpReturnValue %341
               OpFunctionEnd
