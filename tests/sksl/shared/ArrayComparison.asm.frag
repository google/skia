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
               OpMemberName %_UniformBuffer 2 "testArray"
               OpMemberName %_UniformBuffer 3 "testArrayNegative"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %f1 "f1"
               OpName %f2 "f2"
               OpName %f3 "f3"
               OpName %v1 "v1"
               OpName %v2 "v2"
               OpName %v3 "v3"
               OpName %m1 "m1"
               OpName %m2 "m2"
               OpName %m3 "m3"
               OpName %S "S"
               OpMemberName %S 0 "x"
               OpMemberName %S 1 "y"
               OpName %s1 "s1"
               OpName %s2 "s2"
               OpName %s3 "s3"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %_arr_float_int_5 ArrayStride 16
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 3 Offset 112
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %_arr_v3int_int_2 ArrayStride 16
               OpDecorate %_arr_mat2v2float_int_3 ArrayStride 32
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %_arr_S_int_3 ArrayStride 16
               OpDecorate %226 RelaxedPrecision
               OpDecorate %228 RelaxedPrecision
               OpDecorate %231 RelaxedPrecision
               OpDecorate %233 RelaxedPrecision
               OpDecorate %237 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %246 RelaxedPrecision
               OpDecorate %248 RelaxedPrecision
               OpDecorate %251 RelaxedPrecision
               OpDecorate %253 RelaxedPrecision
               OpDecorate %257 RelaxedPrecision
               OpDecorate %259 RelaxedPrecision
               OpDecorate %289 RelaxedPrecision
               OpDecorate %291 RelaxedPrecision
               OpDecorate %292 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
        %int = OpTypeInt 32 1
      %int_5 = OpConstant %int 5
%_arr_float_int_5 = OpTypeArray %float %int_5
%_UniformBuffer = OpTypeStruct %v4float %v4float %_arr_float_int_5 %_arr_float_int_5
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %26 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function__arr_float_int_5 = OpTypePointer Function %_arr_float_int_5
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
    %float_5 = OpConstant %float 5
   %float_n4 = OpConstant %float -4
      %v3int = OpTypeVector %int 3
      %int_2 = OpConstant %int 2
%_arr_v3int_int_2 = OpTypeArray %v3int %int_2
%_ptr_Function__arr_v3int_int_2 = OpTypePointer Function %_arr_v3int_int_2
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
         %48 = OpConstantComposite %v3int %int_1 %int_2 %int_3
      %int_4 = OpConstant %int 4
      %int_6 = OpConstant %int 6
         %51 = OpConstantComposite %v3int %int_4 %int_5 %int_6
     %int_n6 = OpConstant %int -6
         %56 = OpConstantComposite %v3int %int_4 %int_5 %int_n6
%mat2v2float = OpTypeMatrix %v2float 2
%_arr_mat2v2float_int_3 = OpTypeArray %mat2v2float %int_3
%_ptr_Function__arr_mat2v2float_int_3 = OpTypePointer Function %_arr_mat2v2float_int_3
         %62 = OpConstantComposite %v2float %float_1 %float_0
         %63 = OpConstantComposite %v2float %float_0 %float_1
         %64 = OpConstantComposite %mat2v2float %62 %63
         %65 = OpConstantComposite %v2float %float_2 %float_0
         %66 = OpConstantComposite %v2float %float_0 %float_2
         %67 = OpConstantComposite %mat2v2float %65 %66
    %float_6 = OpConstant %float 6
         %69 = OpConstantComposite %v2float %float_3 %float_4
         %70 = OpConstantComposite %v2float %float_5 %float_6
         %71 = OpConstantComposite %mat2v2float %69 %70
         %75 = OpConstantComposite %v2float %float_2 %float_3
         %76 = OpConstantComposite %v2float %float_4 %float_5
         %77 = OpConstantComposite %mat2v2float %75 %76
         %78 = OpConstantComposite %v2float %float_6 %float_0
         %79 = OpConstantComposite %v2float %float_0 %float_6
         %80 = OpConstantComposite %mat2v2float %78 %79
          %S = OpTypeStruct %int %int
%_arr_S_int_3 = OpTypeArray %S %int_3
%_ptr_Function__arr_S_int_3 = OpTypePointer Function %_arr_S_int_3
      %int_0 = OpConstant %int 0
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
%_ptr_Uniform__arr_float_int_5 = OpTypePointer Uniform %_arr_float_int_5
     %v3bool = OpTypeVector %bool 3
     %v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %18
         %19 = OpLabel
         %23 = OpVariable %_ptr_Function_v2float Function
               OpStore %23 %22
         %25 = OpFunctionCall %v4float %main %23
               OpStore %sk_FragColor %25
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %26
         %27 = OpFunctionParameter %_ptr_Function_v2float
         %28 = OpLabel
         %f1 = OpVariable %_ptr_Function__arr_float_int_5 Function
         %f2 = OpVariable %_ptr_Function__arr_float_int_5 Function
         %f3 = OpVariable %_ptr_Function__arr_float_int_5 Function
         %v1 = OpVariable %_ptr_Function__arr_v3int_int_2 Function
         %v2 = OpVariable %_ptr_Function__arr_v3int_int_2 Function
         %v3 = OpVariable %_ptr_Function__arr_v3int_int_2 Function
         %m1 = OpVariable %_ptr_Function__arr_mat2v2float_int_3 Function
         %m2 = OpVariable %_ptr_Function__arr_mat2v2float_int_3 Function
         %m3 = OpVariable %_ptr_Function__arr_mat2v2float_int_3 Function
         %s1 = OpVariable %_ptr_Function__arr_S_int_3 Function
         %s2 = OpVariable %_ptr_Function__arr_S_int_3 Function
         %s3 = OpVariable %_ptr_Function__arr_S_int_3 Function
        %282 = OpVariable %_ptr_Function_v4float Function
         %36 = OpCompositeConstruct %_arr_float_int_5 %float_1 %float_2 %float_3 %float_4 %float_5
               OpStore %f1 %36
               OpStore %f2 %36
         %40 = OpCompositeConstruct %_arr_float_int_5 %float_1 %float_2 %float_3 %float_n4 %float_5
               OpStore %f3 %40
         %52 = OpCompositeConstruct %_arr_v3int_int_2 %48 %51
               OpStore %v1 %52
               OpStore %v2 %52
         %57 = OpCompositeConstruct %_arr_v3int_int_2 %48 %56
               OpStore %v3 %57
         %72 = OpCompositeConstruct %_arr_mat2v2float_int_3 %64 %67 %71
               OpStore %m1 %72
               OpStore %m2 %72
         %81 = OpCompositeConstruct %_arr_mat2v2float_int_3 %64 %77 %80
               OpStore %m3 %81
         %86 = OpCompositeConstruct %S %int_1 %int_2
         %87 = OpCompositeConstruct %S %int_3 %int_4
         %88 = OpCompositeConstruct %S %int_5 %int_6
         %89 = OpCompositeConstruct %_arr_S_int_3 %86 %87 %88
               OpStore %s1 %89
         %92 = OpCompositeConstruct %S %int_0 %int_0
         %93 = OpCompositeConstruct %_arr_S_int_3 %86 %92 %88
               OpStore %s2 %93
               OpStore %s3 %89
         %97 = OpLogicalAnd %bool %true %true
         %98 = OpLogicalAnd %bool %true %97
         %99 = OpLogicalAnd %bool %true %98
        %100 = OpLogicalAnd %bool %true %99
               OpSelectionMerge %102 None
               OpBranchConditional %100 %101 %102
        %101 = OpLabel
        %103 = OpLogicalOr %bool %false %false
        %104 = OpLogicalOr %bool %false %103
        %105 = OpFUnordNotEqual %bool %float_4 %float_n4
        %106 = OpLogicalOr %bool %105 %104
        %107 = OpLogicalOr %bool %false %106
               OpBranch %102
        %102 = OpLabel
        %108 = OpPhi %bool %false %28 %107 %101
               OpSelectionMerge %110 None
               OpBranchConditional %108 %109 %110
        %109 = OpLabel
        %111 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %10 %int_2
        %113 = OpLoad %_arr_float_int_5 %111
        %114 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %10 %int_3
        %115 = OpLoad %_arr_float_int_5 %114
        %116 = OpCompositeExtract %float %113 0
        %117 = OpCompositeExtract %float %115 0
        %118 = OpFUnordNotEqual %bool %116 %117
        %119 = OpCompositeExtract %float %113 1
        %120 = OpCompositeExtract %float %115 1
        %121 = OpFUnordNotEqual %bool %119 %120
        %122 = OpLogicalOr %bool %121 %118
        %123 = OpCompositeExtract %float %113 2
        %124 = OpCompositeExtract %float %115 2
        %125 = OpFUnordNotEqual %bool %123 %124
        %126 = OpLogicalOr %bool %125 %122
        %127 = OpCompositeExtract %float %113 3
        %128 = OpCompositeExtract %float %115 3
        %129 = OpFUnordNotEqual %bool %127 %128
        %130 = OpLogicalOr %bool %129 %126
        %131 = OpCompositeExtract %float %113 4
        %132 = OpCompositeExtract %float %115 4
        %133 = OpFUnordNotEqual %bool %131 %132
        %134 = OpLogicalOr %bool %133 %130
               OpBranch %110
        %110 = OpLabel
        %135 = OpPhi %bool %false %102 %134 %109
               OpSelectionMerge %137 None
               OpBranchConditional %135 %136 %137
        %136 = OpLabel
        %138 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %10 %int_2
        %139 = OpLoad %_arr_float_int_5 %138
        %140 = OpCompositeExtract %float %139 0
        %141 = OpFOrdEqual %bool %140 %float_1
        %142 = OpCompositeExtract %float %139 1
        %143 = OpFOrdEqual %bool %142 %float_2
        %144 = OpLogicalAnd %bool %143 %141
        %145 = OpCompositeExtract %float %139 2
        %146 = OpFOrdEqual %bool %145 %float_3
        %147 = OpLogicalAnd %bool %146 %144
        %148 = OpCompositeExtract %float %139 3
        %149 = OpFOrdEqual %bool %148 %float_4
        %150 = OpLogicalAnd %bool %149 %147
        %151 = OpCompositeExtract %float %139 4
        %152 = OpFOrdEqual %bool %151 %float_5
        %153 = OpLogicalAnd %bool %152 %150
               OpBranch %137
        %137 = OpLabel
        %154 = OpPhi %bool %false %110 %153 %136
               OpSelectionMerge %156 None
               OpBranchConditional %154 %155 %156
        %155 = OpLabel
        %157 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %10 %int_2
        %158 = OpLoad %_arr_float_int_5 %157
        %159 = OpCompositeExtract %float %158 0
        %160 = OpFUnordNotEqual %bool %159 %float_1
        %161 = OpCompositeExtract %float %158 1
        %162 = OpFUnordNotEqual %bool %161 %float_2
        %163 = OpLogicalOr %bool %162 %160
        %164 = OpCompositeExtract %float %158 2
        %165 = OpFUnordNotEqual %bool %164 %float_3
        %166 = OpLogicalOr %bool %165 %163
        %167 = OpCompositeExtract %float %158 3
        %168 = OpFUnordNotEqual %bool %167 %float_n4
        %169 = OpLogicalOr %bool %168 %166
        %170 = OpCompositeExtract %float %158 4
        %171 = OpFUnordNotEqual %bool %170 %float_5
        %172 = OpLogicalOr %bool %171 %169
               OpBranch %156
        %156 = OpLabel
        %173 = OpPhi %bool %false %137 %172 %155
               OpSelectionMerge %175 None
               OpBranchConditional %173 %174 %175
        %174 = OpLabel
        %176 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %10 %int_2
        %177 = OpLoad %_arr_float_int_5 %176
        %178 = OpCompositeExtract %float %177 0
        %179 = OpFOrdEqual %bool %float_1 %178
        %180 = OpCompositeExtract %float %177 1
        %181 = OpFOrdEqual %bool %float_2 %180
        %182 = OpLogicalAnd %bool %181 %179
        %183 = OpCompositeExtract %float %177 2
        %184 = OpFOrdEqual %bool %float_3 %183
        %185 = OpLogicalAnd %bool %184 %182
        %186 = OpCompositeExtract %float %177 3
        %187 = OpFOrdEqual %bool %float_4 %186
        %188 = OpLogicalAnd %bool %187 %185
        %189 = OpCompositeExtract %float %177 4
        %190 = OpFOrdEqual %bool %float_5 %189
        %191 = OpLogicalAnd %bool %190 %188
               OpBranch %175
        %175 = OpLabel
        %192 = OpPhi %bool %false %156 %191 %174
               OpSelectionMerge %194 None
               OpBranchConditional %192 %193 %194
        %193 = OpLabel
        %195 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %10 %int_2
        %196 = OpLoad %_arr_float_int_5 %195
        %197 = OpCompositeExtract %float %196 0
        %198 = OpFUnordNotEqual %bool %float_1 %197
        %199 = OpCompositeExtract %float %196 1
        %200 = OpFUnordNotEqual %bool %float_2 %199
        %201 = OpLogicalOr %bool %200 %198
        %202 = OpCompositeExtract %float %196 2
        %203 = OpFUnordNotEqual %bool %float_3 %202
        %204 = OpLogicalOr %bool %203 %201
        %205 = OpCompositeExtract %float %196 3
        %206 = OpFUnordNotEqual %bool %float_n4 %205
        %207 = OpLogicalOr %bool %206 %204
        %208 = OpCompositeExtract %float %196 4
        %209 = OpFUnordNotEqual %bool %float_5 %208
        %210 = OpLogicalOr %bool %209 %207
               OpBranch %194
        %194 = OpLabel
        %211 = OpPhi %bool %false %175 %210 %193
               OpSelectionMerge %213 None
               OpBranchConditional %211 %212 %213
        %212 = OpLabel
        %214 = OpLogicalAnd %bool %true %true
               OpBranch %213
        %213 = OpLabel
        %215 = OpPhi %bool %false %194 %214 %212
               OpSelectionMerge %217 None
               OpBranchConditional %215 %216 %217
        %216 = OpLabel
        %218 = OpINotEqual %v3bool %51 %56
        %220 = OpAny %bool %218
        %221 = OpLogicalOr %bool %220 %false
               OpBranch %217
        %217 = OpLabel
        %222 = OpPhi %bool %false %213 %221 %216
               OpSelectionMerge %224 None
               OpBranchConditional %222 %223 %224
        %223 = OpLabel
        %226 = OpFOrdEqual %v2bool %62 %62
        %227 = OpAll %bool %226
        %228 = OpFOrdEqual %v2bool %63 %63
        %229 = OpAll %bool %228
        %230 = OpLogicalAnd %bool %227 %229
        %231 = OpFOrdEqual %v2bool %65 %65
        %232 = OpAll %bool %231
        %233 = OpFOrdEqual %v2bool %66 %66
        %234 = OpAll %bool %233
        %235 = OpLogicalAnd %bool %232 %234
        %236 = OpLogicalAnd %bool %235 %230
        %237 = OpFOrdEqual %v2bool %69 %69
        %238 = OpAll %bool %237
        %239 = OpFOrdEqual %v2bool %70 %70
        %240 = OpAll %bool %239
        %241 = OpLogicalAnd %bool %238 %240
        %242 = OpLogicalAnd %bool %241 %236
               OpBranch %224
        %224 = OpLabel
        %243 = OpPhi %bool %false %217 %242 %223
               OpSelectionMerge %245 None
               OpBranchConditional %243 %244 %245
        %244 = OpLabel
        %246 = OpFUnordNotEqual %v2bool %62 %62
        %247 = OpAny %bool %246
        %248 = OpFUnordNotEqual %v2bool %63 %63
        %249 = OpAny %bool %248
        %250 = OpLogicalOr %bool %247 %249
        %251 = OpFUnordNotEqual %v2bool %65 %75
        %252 = OpAny %bool %251
        %253 = OpFUnordNotEqual %v2bool %66 %76
        %254 = OpAny %bool %253
        %255 = OpLogicalOr %bool %252 %254
        %256 = OpLogicalOr %bool %255 %250
        %257 = OpFUnordNotEqual %v2bool %69 %78
        %258 = OpAny %bool %257
        %259 = OpFUnordNotEqual %v2bool %70 %79
        %260 = OpAny %bool %259
        %261 = OpLogicalOr %bool %258 %260
        %262 = OpLogicalOr %bool %261 %256
               OpBranch %245
        %245 = OpLabel
        %263 = OpPhi %bool %false %224 %262 %244
               OpSelectionMerge %265 None
               OpBranchConditional %263 %264 %265
        %264 = OpLabel
        %266 = OpLogicalOr %bool %false %false
        %267 = OpINotEqual %bool %int_3 %int_0
        %268 = OpINotEqual %bool %int_4 %int_0
        %269 = OpLogicalOr %bool %268 %267
        %270 = OpLogicalOr %bool %269 %266
        %271 = OpLogicalOr %bool %false %false
        %272 = OpLogicalOr %bool %271 %270
               OpBranch %265
        %265 = OpLabel
        %273 = OpPhi %bool %false %245 %272 %264
               OpSelectionMerge %275 None
               OpBranchConditional %273 %274 %275
        %274 = OpLabel
        %276 = OpLogicalAnd %bool %true %true
        %277 = OpLogicalAnd %bool %true %true
        %278 = OpLogicalAnd %bool %277 %276
        %279 = OpLogicalAnd %bool %true %true
        %280 = OpLogicalAnd %bool %279 %278
               OpBranch %275
        %275 = OpLabel
        %281 = OpPhi %bool %false %265 %280 %274
               OpSelectionMerge %286 None
               OpBranchConditional %281 %284 %285
        %284 = OpLabel
        %287 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %289 = OpLoad %v4float %287
               OpStore %282 %289
               OpBranch %286
        %285 = OpLabel
        %290 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %291 = OpLoad %v4float %290
               OpStore %282 %291
               OpBranch %286
        %286 = OpLabel
        %292 = OpLoad %v4float %282
               OpReturnValue %292
               OpFunctionEnd
