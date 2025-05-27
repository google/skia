               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
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
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %_arr_v3int_int_2 ArrayStride 16
               OpDecorate %_arr_mat2v2float_int_3 ArrayStride 32
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %_arr_S_int_3 ArrayStride 16
               OpDecorate %224 RelaxedPrecision
               OpDecorate %226 RelaxedPrecision
               OpDecorate %229 RelaxedPrecision
               OpDecorate %231 RelaxedPrecision
               OpDecorate %235 RelaxedPrecision
               OpDecorate %237 RelaxedPrecision
               OpDecorate %244 RelaxedPrecision
               OpDecorate %246 RelaxedPrecision
               OpDecorate %249 RelaxedPrecision
               OpDecorate %251 RelaxedPrecision
               OpDecorate %255 RelaxedPrecision
               OpDecorate %257 RelaxedPrecision
               OpDecorate %287 RelaxedPrecision
               OpDecorate %289 RelaxedPrecision
               OpDecorate %290 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
        %int = OpTypeInt 32 1
      %int_5 = OpConstant %int 5
%_arr_float_int_5 = OpTypeArray %float %int_5
%_UniformBuffer = OpTypeStruct %v4float %v4float %_arr_float_int_5 %_arr_float_int_5
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
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
         %45 = OpConstantComposite %v3int %int_1 %int_2 %int_3
      %int_4 = OpConstant %int 4
      %int_6 = OpConstant %int 6
         %48 = OpConstantComposite %v3int %int_4 %int_5 %int_6
     %int_n6 = OpConstant %int -6
         %53 = OpConstantComposite %v3int %int_4 %int_5 %int_n6
%mat2v2float = OpTypeMatrix %v2float 2
%_arr_mat2v2float_int_3 = OpTypeArray %mat2v2float %int_3
%_ptr_Function__arr_mat2v2float_int_3 = OpTypePointer Function %_arr_mat2v2float_int_3
         %59 = OpConstantComposite %v2float %float_1 %float_0
         %60 = OpConstantComposite %v2float %float_0 %float_1
         %61 = OpConstantComposite %mat2v2float %59 %60
         %62 = OpConstantComposite %v2float %float_2 %float_0
         %63 = OpConstantComposite %v2float %float_0 %float_2
         %64 = OpConstantComposite %mat2v2float %62 %63
    %float_6 = OpConstant %float 6
         %66 = OpConstantComposite %v2float %float_3 %float_4
         %67 = OpConstantComposite %v2float %float_5 %float_6
         %68 = OpConstantComposite %mat2v2float %66 %67
         %72 = OpConstantComposite %v2float %float_2 %float_3
         %73 = OpConstantComposite %v2float %float_4 %float_5
         %74 = OpConstantComposite %mat2v2float %72 %73
         %75 = OpConstantComposite %v2float %float_6 %float_0
         %76 = OpConstantComposite %v2float %float_0 %float_6
         %77 = OpConstantComposite %mat2v2float %75 %76
          %S = OpTypeStruct %int %int
%_arr_S_int_3 = OpTypeArray %S %int_3
%_ptr_Function__arr_S_int_3 = OpTypePointer Function %_arr_S_int_3
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
%_ptr_Uniform__arr_float_int_5 = OpTypePointer Uniform %_arr_float_int_5
     %v3bool = OpTypeVector %bool 3
     %v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %15
         %16 = OpLabel
         %20 = OpVariable %_ptr_Function_v2float Function
               OpStore %20 %19
         %22 = OpFunctionCall %v4float %main %20
               OpStore %sk_FragColor %22
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %23
         %24 = OpFunctionParameter %_ptr_Function_v2float
         %25 = OpLabel
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
        %280 = OpVariable %_ptr_Function_v4float Function
         %33 = OpCompositeConstruct %_arr_float_int_5 %float_1 %float_2 %float_3 %float_4 %float_5
               OpStore %f1 %33
               OpStore %f2 %33
         %37 = OpCompositeConstruct %_arr_float_int_5 %float_1 %float_2 %float_3 %float_n4 %float_5
               OpStore %f3 %37
         %49 = OpCompositeConstruct %_arr_v3int_int_2 %45 %48
               OpStore %v1 %49
               OpStore %v2 %49
         %54 = OpCompositeConstruct %_arr_v3int_int_2 %45 %53
               OpStore %v3 %54
         %69 = OpCompositeConstruct %_arr_mat2v2float_int_3 %61 %64 %68
               OpStore %m1 %69
               OpStore %m2 %69
         %78 = OpCompositeConstruct %_arr_mat2v2float_int_3 %61 %74 %77
               OpStore %m3 %78
         %83 = OpCompositeConstruct %S %int_1 %int_2
         %84 = OpCompositeConstruct %S %int_3 %int_4
         %85 = OpCompositeConstruct %S %int_5 %int_6
         %86 = OpCompositeConstruct %_arr_S_int_3 %83 %84 %85
               OpStore %s1 %86
         %89 = OpCompositeConstruct %S %int_0 %int_0
         %90 = OpCompositeConstruct %_arr_S_int_3 %83 %89 %85
               OpStore %s2 %90
               OpStore %s3 %86
         %95 = OpLogicalAnd %bool %true %true
         %96 = OpLogicalAnd %bool %true %95
         %97 = OpLogicalAnd %bool %true %96
         %98 = OpLogicalAnd %bool %true %97
               OpSelectionMerge %100 None
               OpBranchConditional %98 %99 %100
         %99 = OpLabel
        %101 = OpLogicalOr %bool %false %false
        %102 = OpLogicalOr %bool %false %101
        %103 = OpFUnordNotEqual %bool %float_4 %float_n4
        %104 = OpLogicalOr %bool %103 %102
        %105 = OpLogicalOr %bool %false %104
               OpBranch %100
        %100 = OpLabel
        %106 = OpPhi %bool %false %25 %105 %99
               OpSelectionMerge %108 None
               OpBranchConditional %106 %107 %108
        %107 = OpLabel
        %109 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %7 %int_2
        %111 = OpLoad %_arr_float_int_5 %109
        %112 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %7 %int_3
        %113 = OpLoad %_arr_float_int_5 %112
        %114 = OpCompositeExtract %float %111 0
        %115 = OpCompositeExtract %float %113 0
        %116 = OpFUnordNotEqual %bool %114 %115
        %117 = OpCompositeExtract %float %111 1
        %118 = OpCompositeExtract %float %113 1
        %119 = OpFUnordNotEqual %bool %117 %118
        %120 = OpLogicalOr %bool %119 %116
        %121 = OpCompositeExtract %float %111 2
        %122 = OpCompositeExtract %float %113 2
        %123 = OpFUnordNotEqual %bool %121 %122
        %124 = OpLogicalOr %bool %123 %120
        %125 = OpCompositeExtract %float %111 3
        %126 = OpCompositeExtract %float %113 3
        %127 = OpFUnordNotEqual %bool %125 %126
        %128 = OpLogicalOr %bool %127 %124
        %129 = OpCompositeExtract %float %111 4
        %130 = OpCompositeExtract %float %113 4
        %131 = OpFUnordNotEqual %bool %129 %130
        %132 = OpLogicalOr %bool %131 %128
               OpBranch %108
        %108 = OpLabel
        %133 = OpPhi %bool %false %100 %132 %107
               OpSelectionMerge %135 None
               OpBranchConditional %133 %134 %135
        %134 = OpLabel
        %136 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %7 %int_2
        %137 = OpLoad %_arr_float_int_5 %136
        %138 = OpCompositeExtract %float %137 0
        %139 = OpFOrdEqual %bool %138 %float_1
        %140 = OpCompositeExtract %float %137 1
        %141 = OpFOrdEqual %bool %140 %float_2
        %142 = OpLogicalAnd %bool %141 %139
        %143 = OpCompositeExtract %float %137 2
        %144 = OpFOrdEqual %bool %143 %float_3
        %145 = OpLogicalAnd %bool %144 %142
        %146 = OpCompositeExtract %float %137 3
        %147 = OpFOrdEqual %bool %146 %float_4
        %148 = OpLogicalAnd %bool %147 %145
        %149 = OpCompositeExtract %float %137 4
        %150 = OpFOrdEqual %bool %149 %float_5
        %151 = OpLogicalAnd %bool %150 %148
               OpBranch %135
        %135 = OpLabel
        %152 = OpPhi %bool %false %108 %151 %134
               OpSelectionMerge %154 None
               OpBranchConditional %152 %153 %154
        %153 = OpLabel
        %155 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %7 %int_2
        %156 = OpLoad %_arr_float_int_5 %155
        %157 = OpCompositeExtract %float %156 0
        %158 = OpFUnordNotEqual %bool %157 %float_1
        %159 = OpCompositeExtract %float %156 1
        %160 = OpFUnordNotEqual %bool %159 %float_2
        %161 = OpLogicalOr %bool %160 %158
        %162 = OpCompositeExtract %float %156 2
        %163 = OpFUnordNotEqual %bool %162 %float_3
        %164 = OpLogicalOr %bool %163 %161
        %165 = OpCompositeExtract %float %156 3
        %166 = OpFUnordNotEqual %bool %165 %float_n4
        %167 = OpLogicalOr %bool %166 %164
        %168 = OpCompositeExtract %float %156 4
        %169 = OpFUnordNotEqual %bool %168 %float_5
        %170 = OpLogicalOr %bool %169 %167
               OpBranch %154
        %154 = OpLabel
        %171 = OpPhi %bool %false %135 %170 %153
               OpSelectionMerge %173 None
               OpBranchConditional %171 %172 %173
        %172 = OpLabel
        %174 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %7 %int_2
        %175 = OpLoad %_arr_float_int_5 %174
        %176 = OpCompositeExtract %float %175 0
        %177 = OpFOrdEqual %bool %float_1 %176
        %178 = OpCompositeExtract %float %175 1
        %179 = OpFOrdEqual %bool %float_2 %178
        %180 = OpLogicalAnd %bool %179 %177
        %181 = OpCompositeExtract %float %175 2
        %182 = OpFOrdEqual %bool %float_3 %181
        %183 = OpLogicalAnd %bool %182 %180
        %184 = OpCompositeExtract %float %175 3
        %185 = OpFOrdEqual %bool %float_4 %184
        %186 = OpLogicalAnd %bool %185 %183
        %187 = OpCompositeExtract %float %175 4
        %188 = OpFOrdEqual %bool %float_5 %187
        %189 = OpLogicalAnd %bool %188 %186
               OpBranch %173
        %173 = OpLabel
        %190 = OpPhi %bool %false %154 %189 %172
               OpSelectionMerge %192 None
               OpBranchConditional %190 %191 %192
        %191 = OpLabel
        %193 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %7 %int_2
        %194 = OpLoad %_arr_float_int_5 %193
        %195 = OpCompositeExtract %float %194 0
        %196 = OpFUnordNotEqual %bool %float_1 %195
        %197 = OpCompositeExtract %float %194 1
        %198 = OpFUnordNotEqual %bool %float_2 %197
        %199 = OpLogicalOr %bool %198 %196
        %200 = OpCompositeExtract %float %194 2
        %201 = OpFUnordNotEqual %bool %float_3 %200
        %202 = OpLogicalOr %bool %201 %199
        %203 = OpCompositeExtract %float %194 3
        %204 = OpFUnordNotEqual %bool %float_n4 %203
        %205 = OpLogicalOr %bool %204 %202
        %206 = OpCompositeExtract %float %194 4
        %207 = OpFUnordNotEqual %bool %float_5 %206
        %208 = OpLogicalOr %bool %207 %205
               OpBranch %192
        %192 = OpLabel
        %209 = OpPhi %bool %false %173 %208 %191
               OpSelectionMerge %211 None
               OpBranchConditional %209 %210 %211
        %210 = OpLabel
        %212 = OpLogicalAnd %bool %true %true
               OpBranch %211
        %211 = OpLabel
        %213 = OpPhi %bool %false %192 %212 %210
               OpSelectionMerge %215 None
               OpBranchConditional %213 %214 %215
        %214 = OpLabel
        %216 = OpINotEqual %v3bool %48 %53
        %218 = OpAny %bool %216
        %219 = OpLogicalOr %bool %218 %false
               OpBranch %215
        %215 = OpLabel
        %220 = OpPhi %bool %false %211 %219 %214
               OpSelectionMerge %222 None
               OpBranchConditional %220 %221 %222
        %221 = OpLabel
        %224 = OpFOrdEqual %v2bool %59 %59
        %225 = OpAll %bool %224
        %226 = OpFOrdEqual %v2bool %60 %60
        %227 = OpAll %bool %226
        %228 = OpLogicalAnd %bool %225 %227
        %229 = OpFOrdEqual %v2bool %62 %62
        %230 = OpAll %bool %229
        %231 = OpFOrdEqual %v2bool %63 %63
        %232 = OpAll %bool %231
        %233 = OpLogicalAnd %bool %230 %232
        %234 = OpLogicalAnd %bool %233 %228
        %235 = OpFOrdEqual %v2bool %66 %66
        %236 = OpAll %bool %235
        %237 = OpFOrdEqual %v2bool %67 %67
        %238 = OpAll %bool %237
        %239 = OpLogicalAnd %bool %236 %238
        %240 = OpLogicalAnd %bool %239 %234
               OpBranch %222
        %222 = OpLabel
        %241 = OpPhi %bool %false %215 %240 %221
               OpSelectionMerge %243 None
               OpBranchConditional %241 %242 %243
        %242 = OpLabel
        %244 = OpFUnordNotEqual %v2bool %59 %59
        %245 = OpAny %bool %244
        %246 = OpFUnordNotEqual %v2bool %60 %60
        %247 = OpAny %bool %246
        %248 = OpLogicalOr %bool %245 %247
        %249 = OpFUnordNotEqual %v2bool %62 %72
        %250 = OpAny %bool %249
        %251 = OpFUnordNotEqual %v2bool %63 %73
        %252 = OpAny %bool %251
        %253 = OpLogicalOr %bool %250 %252
        %254 = OpLogicalOr %bool %253 %248
        %255 = OpFUnordNotEqual %v2bool %66 %75
        %256 = OpAny %bool %255
        %257 = OpFUnordNotEqual %v2bool %67 %76
        %258 = OpAny %bool %257
        %259 = OpLogicalOr %bool %256 %258
        %260 = OpLogicalOr %bool %259 %254
               OpBranch %243
        %243 = OpLabel
        %261 = OpPhi %bool %false %222 %260 %242
               OpSelectionMerge %263 None
               OpBranchConditional %261 %262 %263
        %262 = OpLabel
        %264 = OpLogicalOr %bool %false %false
        %265 = OpINotEqual %bool %int_3 %int_0
        %266 = OpINotEqual %bool %int_4 %int_0
        %267 = OpLogicalOr %bool %266 %265
        %268 = OpLogicalOr %bool %267 %264
        %269 = OpLogicalOr %bool %false %false
        %270 = OpLogicalOr %bool %269 %268
               OpBranch %263
        %263 = OpLabel
        %271 = OpPhi %bool %false %243 %270 %262
               OpSelectionMerge %273 None
               OpBranchConditional %271 %272 %273
        %272 = OpLabel
        %274 = OpLogicalAnd %bool %true %true
        %275 = OpLogicalAnd %bool %true %true
        %276 = OpLogicalAnd %bool %275 %274
        %277 = OpLogicalAnd %bool %true %true
        %278 = OpLogicalAnd %bool %277 %276
               OpBranch %273
        %273 = OpLabel
        %279 = OpPhi %bool %false %263 %278 %272
               OpSelectionMerge %284 None
               OpBranchConditional %279 %282 %283
        %282 = OpLabel
        %285 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %287 = OpLoad %v4float %285
               OpStore %280 %287
               OpBranch %284
        %283 = OpLabel
        %288 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %289 = OpLoad %v4float %288
               OpStore %280 %289
               OpBranch %284
        %284 = OpLabel
        %290 = OpLoad %v4float %280
               OpReturnValue %290
               OpFunctionEnd
