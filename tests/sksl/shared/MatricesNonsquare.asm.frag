               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
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
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %8 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %m23 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %m24 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %m32 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %m34 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %m42 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %m43 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %m22 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %m33 RelaxedPrecision
               OpDecorate %164 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %188 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %203 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
               OpDecorate %205 RelaxedPrecision
               OpDecorate %206 RelaxedPrecision
               OpDecorate %214 RelaxedPrecision
               OpDecorate %216 RelaxedPrecision
               OpDecorate %219 RelaxedPrecision
               OpDecorate %224 RelaxedPrecision
               OpDecorate %231 RelaxedPrecision
               OpDecorate %232 RelaxedPrecision
               OpDecorate %234 RelaxedPrecision
               OpDecorate %235 RelaxedPrecision
               OpDecorate %335 RelaxedPrecision
               OpDecorate %338 RelaxedPrecision
               OpDecorate %339 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %8 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %22 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
    %v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
    %float_2 = OpConstant %float 2
         %32 = OpConstantComposite %v3float %float_2 %float_0 %float_0
         %33 = OpConstantComposite %v3float %float_0 %float_2 %float_0
         %34 = OpConstantComposite %mat2v3float %32 %33
      %false = OpConstantFalse %bool
     %v3bool = OpTypeVector %bool 3
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
    %float_3 = OpConstant %float 3
         %49 = OpConstantComposite %v4float %float_3 %float_0 %float_0 %float_0
         %50 = OpConstantComposite %v4float %float_0 %float_3 %float_0 %float_0
         %51 = OpConstantComposite %mat2v4float %49 %50
     %v4bool = OpTypeVector %bool 4
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
    %float_4 = OpConstant %float 4
         %65 = OpConstantComposite %v2float %float_4 %float_0
         %66 = OpConstantComposite %v2float %float_0 %float_4
         %67 = OpConstantComposite %mat3v2float %65 %66 %17
     %v2bool = OpTypeVector %bool 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
    %float_5 = OpConstant %float 5
         %84 = OpConstantComposite %v4float %float_5 %float_0 %float_0 %float_0
         %85 = OpConstantComposite %v4float %float_0 %float_5 %float_0 %float_0
         %86 = OpConstantComposite %v4float %float_0 %float_0 %float_5 %float_0
         %87 = OpConstantComposite %mat3v4float %84 %85 %86
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
    %float_6 = OpConstant %float 6
        %103 = OpConstantComposite %v2float %float_6 %float_0
        %104 = OpConstantComposite %v2float %float_0 %float_6
        %105 = OpConstantComposite %mat4v2float %103 %104 %17 %17
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
    %float_7 = OpConstant %float 7
        %124 = OpConstantComposite %v3float %float_7 %float_0 %float_0
        %125 = OpConstantComposite %v3float %float_0 %float_7 %float_0
        %126 = OpConstantComposite %v3float %float_0 %float_0 %float_7
        %127 = OpConstantComposite %v3float %float_0 %float_0 %float_0
        %128 = OpConstantComposite %mat4v3float %124 %125 %126 %127
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
    %float_8 = OpConstant %float 8
        %150 = OpConstantComposite %v2float %float_8 %float_0
        %151 = OpConstantComposite %v2float %float_0 %float_8
        %152 = OpConstantComposite %mat2v2float %150 %151
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
   %float_35 = OpConstant %float 35
        %168 = OpConstantComposite %v3float %float_35 %float_0 %float_0
        %169 = OpConstantComposite %v3float %float_0 %float_35 %float_0
        %170 = OpConstantComposite %v3float %float_0 %float_0 %float_35
        %171 = OpConstantComposite %mat3v3float %168 %169 %170
    %float_1 = OpConstant %float 1
        %185 = OpConstantComposite %v3float %float_1 %float_1 %float_1
        %186 = OpConstantComposite %mat2v3float %185 %185
        %192 = OpConstantComposite %v3float %float_3 %float_1 %float_1
        %193 = OpConstantComposite %v3float %float_1 %float_3 %float_1
        %194 = OpConstantComposite %mat2v3float %192 %193
        %201 = OpConstantComposite %v2float %float_2 %float_2
        %202 = OpConstantComposite %mat3v2float %201 %201 %201
   %float_n2 = OpConstant %float -2
        %210 = OpConstantComposite %v2float %float_2 %float_n2
        %211 = OpConstantComposite %v2float %float_n2 %float_2
        %212 = OpConstantComposite %v2float %float_n2 %float_n2
        %213 = OpConstantComposite %mat3v2float %210 %211 %212
 %float_0_25 = OpConstant %float 0.25
 %float_0_75 = OpConstant %float 0.75
        %228 = OpConstantComposite %v4float %float_0_75 %float_0 %float_0 %float_0
        %229 = OpConstantComposite %v4float %float_0 %float_0_75 %float_0 %float_0
        %230 = OpConstantComposite %mat2v4float %228 %229
        %239 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %13
         %14 = OpLabel
         %18 = OpVariable %_ptr_Function_v2float Function
               OpStore %18 %17
         %20 = OpFunctionCall %v4float %main %18
               OpStore %sk_FragColor %20
               OpReturn
               OpFunctionEnd
%test_half_b = OpFunction %bool None %22
         %23 = OpLabel
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
               OpStore %m23 %34
               OpSelectionMerge %37 None
               OpBranchConditional %true %36 %37
         %36 = OpLabel
         %39 = OpFOrdEqual %v3bool %32 %32
         %40 = OpAll %bool %39
         %41 = OpFOrdEqual %v3bool %33 %33
         %42 = OpAll %bool %41
         %43 = OpLogicalAnd %bool %40 %42
               OpBranch %37
         %37 = OpLabel
         %44 = OpPhi %bool %false %23 %43 %36
               OpStore %ok %44
               OpStore %m24 %51
               OpSelectionMerge %53 None
               OpBranchConditional %44 %52 %53
         %52 = OpLabel
         %55 = OpFOrdEqual %v4bool %49 %49
         %56 = OpAll %bool %55
         %57 = OpFOrdEqual %v4bool %50 %50
         %58 = OpAll %bool %57
         %59 = OpLogicalAnd %bool %56 %58
               OpBranch %53
         %53 = OpLabel
         %60 = OpPhi %bool %false %37 %59 %52
               OpStore %ok %60
               OpStore %m32 %67
               OpSelectionMerge %69 None
               OpBranchConditional %60 %68 %69
         %68 = OpLabel
         %71 = OpFOrdEqual %v2bool %65 %65
         %72 = OpAll %bool %71
         %73 = OpFOrdEqual %v2bool %66 %66
         %74 = OpAll %bool %73
         %75 = OpLogicalAnd %bool %72 %74
         %76 = OpFOrdEqual %v2bool %17 %17
         %77 = OpAll %bool %76
         %78 = OpLogicalAnd %bool %75 %77
               OpBranch %69
         %69 = OpLabel
         %79 = OpPhi %bool %false %53 %78 %68
               OpStore %ok %79
               OpStore %m34 %87
               OpSelectionMerge %89 None
               OpBranchConditional %79 %88 %89
         %88 = OpLabel
         %90 = OpFOrdEqual %v4bool %84 %84
         %91 = OpAll %bool %90
         %92 = OpFOrdEqual %v4bool %85 %85
         %93 = OpAll %bool %92
         %94 = OpLogicalAnd %bool %91 %93
         %95 = OpFOrdEqual %v4bool %86 %86
         %96 = OpAll %bool %95
         %97 = OpLogicalAnd %bool %94 %96
               OpBranch %89
         %89 = OpLabel
         %98 = OpPhi %bool %false %69 %97 %88
               OpStore %ok %98
               OpStore %m42 %105
               OpSelectionMerge %107 None
               OpBranchConditional %98 %106 %107
        %106 = OpLabel
        %108 = OpFOrdEqual %v2bool %103 %103
        %109 = OpAll %bool %108
        %110 = OpFOrdEqual %v2bool %104 %104
        %111 = OpAll %bool %110
        %112 = OpLogicalAnd %bool %109 %111
        %113 = OpFOrdEqual %v2bool %17 %17
        %114 = OpAll %bool %113
        %115 = OpLogicalAnd %bool %112 %114
        %116 = OpFOrdEqual %v2bool %17 %17
        %117 = OpAll %bool %116
        %118 = OpLogicalAnd %bool %115 %117
               OpBranch %107
        %107 = OpLabel
        %119 = OpPhi %bool %false %89 %118 %106
               OpStore %ok %119
               OpStore %m43 %128
               OpSelectionMerge %130 None
               OpBranchConditional %119 %129 %130
        %129 = OpLabel
        %131 = OpFOrdEqual %v3bool %124 %124
        %132 = OpAll %bool %131
        %133 = OpFOrdEqual %v3bool %125 %125
        %134 = OpAll %bool %133
        %135 = OpLogicalAnd %bool %132 %134
        %136 = OpFOrdEqual %v3bool %126 %126
        %137 = OpAll %bool %136
        %138 = OpLogicalAnd %bool %135 %137
        %139 = OpFOrdEqual %v3bool %127 %127
        %140 = OpAll %bool %139
        %141 = OpLogicalAnd %bool %138 %140
               OpBranch %130
        %130 = OpLabel
        %142 = OpPhi %bool %false %107 %141 %129
               OpStore %ok %142
        %146 = OpMatrixTimesMatrix %mat2v2float %67 %34
               OpStore %m22 %146
               OpSelectionMerge %148 None
               OpBranchConditional %142 %147 %148
        %147 = OpLabel
        %153 = OpCompositeExtract %v2float %146 0
        %154 = OpFOrdEqual %v2bool %153 %150
        %155 = OpAll %bool %154
        %156 = OpCompositeExtract %v2float %146 1
        %157 = OpFOrdEqual %v2bool %156 %151
        %158 = OpAll %bool %157
        %159 = OpLogicalAnd %bool %155 %158
               OpBranch %148
        %148 = OpLabel
        %160 = OpPhi %bool %false %130 %159 %147
               OpStore %ok %160
        %164 = OpMatrixTimesMatrix %mat3v3float %128 %87
               OpStore %m33 %164
               OpSelectionMerge %166 None
               OpBranchConditional %160 %165 %166
        %165 = OpLabel
        %172 = OpCompositeExtract %v3float %164 0
        %173 = OpFOrdEqual %v3bool %172 %168
        %174 = OpAll %bool %173
        %175 = OpCompositeExtract %v3float %164 1
        %176 = OpFOrdEqual %v3bool %175 %169
        %177 = OpAll %bool %176
        %178 = OpLogicalAnd %bool %174 %177
        %179 = OpCompositeExtract %v3float %164 2
        %180 = OpFOrdEqual %v3bool %179 %170
        %181 = OpAll %bool %180
        %182 = OpLogicalAnd %bool %178 %181
               OpBranch %166
        %166 = OpLabel
        %183 = OpPhi %bool %false %148 %182 %165
               OpStore %ok %183
        %187 = OpFAdd %v3float %32 %185
        %188 = OpFAdd %v3float %33 %185
        %189 = OpCompositeConstruct %mat2v3float %187 %188
               OpStore %m23 %189
               OpSelectionMerge %191 None
               OpBranchConditional %183 %190 %191
        %190 = OpLabel
        %195 = OpFOrdEqual %v3bool %187 %192
        %196 = OpAll %bool %195
        %197 = OpFOrdEqual %v3bool %188 %193
        %198 = OpAll %bool %197
        %199 = OpLogicalAnd %bool %196 %198
               OpBranch %191
        %191 = OpLabel
        %200 = OpPhi %bool %false %166 %199 %190
               OpStore %ok %200
        %203 = OpFSub %v2float %65 %201
        %204 = OpFSub %v2float %66 %201
        %205 = OpFSub %v2float %17 %201
        %206 = OpCompositeConstruct %mat3v2float %203 %204 %205
               OpStore %m32 %206
               OpSelectionMerge %208 None
               OpBranchConditional %200 %207 %208
        %207 = OpLabel
        %214 = OpFOrdEqual %v2bool %203 %210
        %215 = OpAll %bool %214
        %216 = OpFOrdEqual %v2bool %204 %211
        %217 = OpAll %bool %216
        %218 = OpLogicalAnd %bool %215 %217
        %219 = OpFOrdEqual %v2bool %205 %212
        %220 = OpAll %bool %219
        %221 = OpLogicalAnd %bool %218 %220
               OpBranch %208
        %208 = OpLabel
        %222 = OpPhi %bool %false %191 %221 %207
               OpStore %ok %222
        %224 = OpMatrixTimesScalar %mat2v4float %51 %float_0_25
               OpStore %m24 %224
               OpSelectionMerge %226 None
               OpBranchConditional %222 %225 %226
        %225 = OpLabel
        %231 = OpCompositeExtract %v4float %224 0
        %232 = OpFOrdEqual %v4bool %231 %228
        %233 = OpAll %bool %232
        %234 = OpCompositeExtract %v4float %224 1
        %235 = OpFOrdEqual %v4bool %234 %229
        %236 = OpAll %bool %235
        %237 = OpLogicalAnd %bool %233 %236
               OpBranch %226
        %226 = OpLabel
        %238 = OpPhi %bool %false %208 %237 %225
               OpStore %ok %238
               OpReturnValue %238
               OpFunctionEnd
       %main = OpFunction %v4float None %239
        %240 = OpFunctionParameter %_ptr_Function_v2float
        %241 = OpLabel
      %_0_ok = OpVariable %_ptr_Function_bool Function
     %_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
     %_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
     %_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
     %_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
        %326 = OpVariable %_ptr_Function_v4float Function
               OpStore %_0_ok %true
               OpStore %_1_m23 %34
               OpSelectionMerge %245 None
               OpBranchConditional %true %244 %245
        %244 = OpLabel
        %246 = OpFOrdEqual %v3bool %32 %32
        %247 = OpAll %bool %246
        %248 = OpFOrdEqual %v3bool %33 %33
        %249 = OpAll %bool %248
        %250 = OpLogicalAnd %bool %247 %249
               OpBranch %245
        %245 = OpLabel
        %251 = OpPhi %bool %false %241 %250 %244
               OpStore %_0_ok %251
               OpStore %_2_m24 %51
               OpSelectionMerge %254 None
               OpBranchConditional %251 %253 %254
        %253 = OpLabel
        %255 = OpFOrdEqual %v4bool %49 %49
        %256 = OpAll %bool %255
        %257 = OpFOrdEqual %v4bool %50 %50
        %258 = OpAll %bool %257
        %259 = OpLogicalAnd %bool %256 %258
               OpBranch %254
        %254 = OpLabel
        %260 = OpPhi %bool %false %245 %259 %253
               OpStore %_0_ok %260
               OpStore %_3_m32 %67
               OpSelectionMerge %263 None
               OpBranchConditional %260 %262 %263
        %262 = OpLabel
        %264 = OpFOrdEqual %v2bool %65 %65
        %265 = OpAll %bool %264
        %266 = OpFOrdEqual %v2bool %66 %66
        %267 = OpAll %bool %266
        %268 = OpLogicalAnd %bool %265 %267
        %269 = OpFOrdEqual %v2bool %17 %17
        %270 = OpAll %bool %269
        %271 = OpLogicalAnd %bool %268 %270
               OpBranch %263
        %263 = OpLabel
        %272 = OpPhi %bool %false %254 %271 %262
               OpStore %_0_ok %272
        %274 = OpMatrixTimesMatrix %mat2v2float %67 %34
               OpStore %_7_m22 %274
               OpSelectionMerge %276 None
               OpBranchConditional %272 %275 %276
        %275 = OpLabel
        %277 = OpCompositeExtract %v2float %274 0
        %278 = OpFOrdEqual %v2bool %277 %150
        %279 = OpAll %bool %278
        %280 = OpCompositeExtract %v2float %274 1
        %281 = OpFOrdEqual %v2bool %280 %151
        %282 = OpAll %bool %281
        %283 = OpLogicalAnd %bool %279 %282
               OpBranch %276
        %276 = OpLabel
        %284 = OpPhi %bool %false %263 %283 %275
               OpStore %_0_ok %284
        %285 = OpFAdd %v3float %32 %185
        %286 = OpFAdd %v3float %33 %185
        %287 = OpCompositeConstruct %mat2v3float %285 %286
               OpStore %_1_m23 %287
               OpSelectionMerge %289 None
               OpBranchConditional %284 %288 %289
        %288 = OpLabel
        %290 = OpFOrdEqual %v3bool %285 %192
        %291 = OpAll %bool %290
        %292 = OpFOrdEqual %v3bool %286 %193
        %293 = OpAll %bool %292
        %294 = OpLogicalAnd %bool %291 %293
               OpBranch %289
        %289 = OpLabel
        %295 = OpPhi %bool %false %276 %294 %288
               OpStore %_0_ok %295
        %296 = OpFSub %v2float %65 %201
        %297 = OpFSub %v2float %66 %201
        %298 = OpFSub %v2float %17 %201
        %299 = OpCompositeConstruct %mat3v2float %296 %297 %298
               OpStore %_3_m32 %299
               OpSelectionMerge %301 None
               OpBranchConditional %295 %300 %301
        %300 = OpLabel
        %302 = OpFOrdEqual %v2bool %296 %210
        %303 = OpAll %bool %302
        %304 = OpFOrdEqual %v2bool %297 %211
        %305 = OpAll %bool %304
        %306 = OpLogicalAnd %bool %303 %305
        %307 = OpFOrdEqual %v2bool %298 %212
        %308 = OpAll %bool %307
        %309 = OpLogicalAnd %bool %306 %308
               OpBranch %301
        %301 = OpLabel
        %310 = OpPhi %bool %false %289 %309 %300
               OpStore %_0_ok %310
        %311 = OpMatrixTimesScalar %mat2v4float %51 %float_0_25
               OpStore %_2_m24 %311
               OpSelectionMerge %313 None
               OpBranchConditional %310 %312 %313
        %312 = OpLabel
        %314 = OpCompositeExtract %v4float %311 0
        %315 = OpFOrdEqual %v4bool %314 %228
        %316 = OpAll %bool %315
        %317 = OpCompositeExtract %v4float %311 1
        %318 = OpFOrdEqual %v4bool %317 %229
        %319 = OpAll %bool %318
        %320 = OpLogicalAnd %bool %316 %319
               OpBranch %313
        %313 = OpLabel
        %321 = OpPhi %bool %false %301 %320 %312
               OpStore %_0_ok %321
               OpSelectionMerge %323 None
               OpBranchConditional %321 %322 %323
        %322 = OpLabel
        %324 = OpFunctionCall %bool %test_half_b
               OpBranch %323
        %323 = OpLabel
        %325 = OpPhi %bool %false %313 %324 %322
               OpSelectionMerge %330 None
               OpBranchConditional %325 %328 %329
        %328 = OpLabel
        %331 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %335 = OpLoad %v4float %331
               OpStore %326 %335
               OpBranch %330
        %329 = OpLabel
        %336 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
        %338 = OpLoad %v4float %336
               OpStore %326 %338
               OpBranch %330
        %330 = OpLabel
        %339 = OpLoad %v4float %326
               OpReturnValue %339
               OpFunctionEnd
