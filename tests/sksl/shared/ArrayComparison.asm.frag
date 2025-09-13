               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %14
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testArray"
               OpMemberName %_UniformBuffer 3 "testArrayNegative"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %16
               OpName %main "main"                      ; id %6
               OpName %f1 "f1"                          ; id %29
               OpName %f2 "f2"                          ; id %37
               OpName %f3 "f3"                          ; id %38
               OpName %v1 "v1"                          ; id %41
               OpName %v2 "v2"                          ; id %53
               OpName %v3 "v3"                          ; id %54
               OpName %m1 "m1"                          ; id %58
               OpName %m2 "m2"                          ; id %73
               OpName %m3 "m3"                          ; id %74
               OpName %S "S"                            ; id %83
               OpMemberName %S 0 "x"
               OpMemberName %S 1 "y"
               OpName %s1 "s1"                      ; id %82
               OpName %s2 "s2"                      ; id %90
               OpName %s3 "s3"                      ; id %94

               ; Annotations
               OpDecorate %main RelaxedPrecision
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
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %_arr_v3int_int_2 ArrayStride 16
               OpDecorate %_arr_mat2v2float_int_3 ArrayStride 32
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 1 Offset 4
               OpDecorate %_arr_S_int_3 ArrayStride 16
               OpDecorate %227 RelaxedPrecision
               OpDecorate %229 RelaxedPrecision
               OpDecorate %232 RelaxedPrecision
               OpDecorate %234 RelaxedPrecision
               OpDecorate %238 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision
               OpDecorate %247 RelaxedPrecision
               OpDecorate %249 RelaxedPrecision
               OpDecorate %252 RelaxedPrecision
               OpDecorate %254 RelaxedPrecision
               OpDecorate %258 RelaxedPrecision
               OpDecorate %260 RelaxedPrecision
               OpDecorate %290 RelaxedPrecision
               OpDecorate %292 RelaxedPrecision
               OpDecorate %293 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
      %int_5 = OpConstant %int 5
%_arr_float_int_5 = OpTypeArray %float %int_5       ; ArrayStride 16
%_UniformBuffer = OpTypeStruct %v4float %v4float %_arr_float_int_5 %_arr_float_int_5    ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
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
%_arr_v3int_int_2 = OpTypeArray %v3int %int_2       ; ArrayStride 16
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
%_arr_mat2v2float_int_3 = OpTypeArray %mat2v2float %int_3   ; ArrayStride 32
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
%_arr_S_int_3 = OpTypeArray %S %int_3               ; ArrayStride 16
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


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %18

         %19 = OpLabel
         %23 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %23 %22
         %25 =   OpFunctionCall %v4float %main %23
                 OpStore %sk_FragColor %25
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %26         ; RelaxedPrecision
         %27 = OpFunctionParameter %_ptr_Function_v2float

         %28 = OpLabel
         %f1 =   OpVariable %_ptr_Function__arr_float_int_5 Function
         %f2 =   OpVariable %_ptr_Function__arr_float_int_5 Function
         %f3 =   OpVariable %_ptr_Function__arr_float_int_5 Function
         %v1 =   OpVariable %_ptr_Function__arr_v3int_int_2 Function
         %v2 =   OpVariable %_ptr_Function__arr_v3int_int_2 Function
         %v3 =   OpVariable %_ptr_Function__arr_v3int_int_2 Function
         %m1 =   OpVariable %_ptr_Function__arr_mat2v2float_int_3 Function
         %m2 =   OpVariable %_ptr_Function__arr_mat2v2float_int_3 Function
         %m3 =   OpVariable %_ptr_Function__arr_mat2v2float_int_3 Function
         %s1 =   OpVariable %_ptr_Function__arr_S_int_3 Function
         %s2 =   OpVariable %_ptr_Function__arr_S_int_3 Function
         %s3 =   OpVariable %_ptr_Function__arr_S_int_3 Function
        %283 =   OpVariable %_ptr_Function_v4float Function
         %36 =   OpCompositeConstruct %_arr_float_int_5 %float_1 %float_2 %float_3 %float_4 %float_5
                 OpStore %f1 %36
                 OpStore %f2 %36
         %40 =   OpCompositeConstruct %_arr_float_int_5 %float_1 %float_2 %float_3 %float_n4 %float_5
                 OpStore %f3 %40
         %52 =   OpCompositeConstruct %_arr_v3int_int_2 %48 %51
                 OpStore %v1 %52
                 OpStore %v2 %52
         %57 =   OpCompositeConstruct %_arr_v3int_int_2 %48 %56
                 OpStore %v3 %57
         %72 =   OpCompositeConstruct %_arr_mat2v2float_int_3 %64 %67 %71
                 OpStore %m1 %72
                 OpStore %m2 %72
         %81 =   OpCompositeConstruct %_arr_mat2v2float_int_3 %64 %77 %80
                 OpStore %m3 %81
         %86 =   OpCompositeConstruct %S %int_1 %int_2
         %87 =   OpCompositeConstruct %S %int_3 %int_4
         %88 =   OpCompositeConstruct %S %int_5 %int_6
         %89 =   OpCompositeConstruct %_arr_S_int_3 %86 %87 %88
                 OpStore %s1 %89
         %92 =   OpCompositeConstruct %S %int_0 %int_0
         %93 =   OpCompositeConstruct %_arr_S_int_3 %86 %92 %88
                 OpStore %s2 %93
                 OpStore %s3 %89
         %98 =   OpLogicalAnd %bool %true %true
         %99 =   OpLogicalAnd %bool %true %98
        %100 =   OpLogicalAnd %bool %true %99
        %101 =   OpLogicalAnd %bool %true %100
                 OpSelectionMerge %103 None
                 OpBranchConditional %101 %102 %103

        %102 =     OpLabel
        %104 =       OpLogicalOr %bool %false %false
        %105 =       OpLogicalOr %bool %false %104
        %106 =       OpFUnordNotEqual %bool %float_4 %float_n4
        %107 =       OpLogicalOr %bool %106 %105
        %108 =       OpLogicalOr %bool %false %107
                     OpBranch %103

        %103 = OpLabel
        %109 =   OpPhi %bool %false %28 %108 %102
                 OpSelectionMerge %111 None
                 OpBranchConditional %109 %110 %111

        %110 =     OpLabel
        %112 =       OpAccessChain %_ptr_Uniform__arr_float_int_5 %11 %int_2
        %114 =       OpLoad %_arr_float_int_5 %112
        %115 =       OpAccessChain %_ptr_Uniform__arr_float_int_5 %11 %int_3
        %116 =       OpLoad %_arr_float_int_5 %115
        %117 =       OpCompositeExtract %float %114 0
        %118 =       OpCompositeExtract %float %116 0
        %119 =       OpFUnordNotEqual %bool %117 %118
        %120 =       OpCompositeExtract %float %114 1
        %121 =       OpCompositeExtract %float %116 1
        %122 =       OpFUnordNotEqual %bool %120 %121
        %123 =       OpLogicalOr %bool %122 %119
        %124 =       OpCompositeExtract %float %114 2
        %125 =       OpCompositeExtract %float %116 2
        %126 =       OpFUnordNotEqual %bool %124 %125
        %127 =       OpLogicalOr %bool %126 %123
        %128 =       OpCompositeExtract %float %114 3
        %129 =       OpCompositeExtract %float %116 3
        %130 =       OpFUnordNotEqual %bool %128 %129
        %131 =       OpLogicalOr %bool %130 %127
        %132 =       OpCompositeExtract %float %114 4
        %133 =       OpCompositeExtract %float %116 4
        %134 =       OpFUnordNotEqual %bool %132 %133
        %135 =       OpLogicalOr %bool %134 %131
                     OpBranch %111

        %111 = OpLabel
        %136 =   OpPhi %bool %false %103 %135 %110
                 OpSelectionMerge %138 None
                 OpBranchConditional %136 %137 %138

        %137 =     OpLabel
        %139 =       OpAccessChain %_ptr_Uniform__arr_float_int_5 %11 %int_2
        %140 =       OpLoad %_arr_float_int_5 %139
        %141 =       OpCompositeExtract %float %140 0
        %142 =       OpFOrdEqual %bool %141 %float_1
        %143 =       OpCompositeExtract %float %140 1
        %144 =       OpFOrdEqual %bool %143 %float_2
        %145 =       OpLogicalAnd %bool %144 %142
        %146 =       OpCompositeExtract %float %140 2
        %147 =       OpFOrdEqual %bool %146 %float_3
        %148 =       OpLogicalAnd %bool %147 %145
        %149 =       OpCompositeExtract %float %140 3
        %150 =       OpFOrdEqual %bool %149 %float_4
        %151 =       OpLogicalAnd %bool %150 %148
        %152 =       OpCompositeExtract %float %140 4
        %153 =       OpFOrdEqual %bool %152 %float_5
        %154 =       OpLogicalAnd %bool %153 %151
                     OpBranch %138

        %138 = OpLabel
        %155 =   OpPhi %bool %false %111 %154 %137
                 OpSelectionMerge %157 None
                 OpBranchConditional %155 %156 %157

        %156 =     OpLabel
        %158 =       OpAccessChain %_ptr_Uniform__arr_float_int_5 %11 %int_2
        %159 =       OpLoad %_arr_float_int_5 %158
        %160 =       OpCompositeExtract %float %159 0
        %161 =       OpFUnordNotEqual %bool %160 %float_1
        %162 =       OpCompositeExtract %float %159 1
        %163 =       OpFUnordNotEqual %bool %162 %float_2
        %164 =       OpLogicalOr %bool %163 %161
        %165 =       OpCompositeExtract %float %159 2
        %166 =       OpFUnordNotEqual %bool %165 %float_3
        %167 =       OpLogicalOr %bool %166 %164
        %168 =       OpCompositeExtract %float %159 3
        %169 =       OpFUnordNotEqual %bool %168 %float_n4
        %170 =       OpLogicalOr %bool %169 %167
        %171 =       OpCompositeExtract %float %159 4
        %172 =       OpFUnordNotEqual %bool %171 %float_5
        %173 =       OpLogicalOr %bool %172 %170
                     OpBranch %157

        %157 = OpLabel
        %174 =   OpPhi %bool %false %138 %173 %156
                 OpSelectionMerge %176 None
                 OpBranchConditional %174 %175 %176

        %175 =     OpLabel
        %177 =       OpAccessChain %_ptr_Uniform__arr_float_int_5 %11 %int_2
        %178 =       OpLoad %_arr_float_int_5 %177
        %179 =       OpCompositeExtract %float %178 0
        %180 =       OpFOrdEqual %bool %float_1 %179
        %181 =       OpCompositeExtract %float %178 1
        %182 =       OpFOrdEqual %bool %float_2 %181
        %183 =       OpLogicalAnd %bool %182 %180
        %184 =       OpCompositeExtract %float %178 2
        %185 =       OpFOrdEqual %bool %float_3 %184
        %186 =       OpLogicalAnd %bool %185 %183
        %187 =       OpCompositeExtract %float %178 3
        %188 =       OpFOrdEqual %bool %float_4 %187
        %189 =       OpLogicalAnd %bool %188 %186
        %190 =       OpCompositeExtract %float %178 4
        %191 =       OpFOrdEqual %bool %float_5 %190
        %192 =       OpLogicalAnd %bool %191 %189
                     OpBranch %176

        %176 = OpLabel
        %193 =   OpPhi %bool %false %157 %192 %175
                 OpSelectionMerge %195 None
                 OpBranchConditional %193 %194 %195

        %194 =     OpLabel
        %196 =       OpAccessChain %_ptr_Uniform__arr_float_int_5 %11 %int_2
        %197 =       OpLoad %_arr_float_int_5 %196
        %198 =       OpCompositeExtract %float %197 0
        %199 =       OpFUnordNotEqual %bool %float_1 %198
        %200 =       OpCompositeExtract %float %197 1
        %201 =       OpFUnordNotEqual %bool %float_2 %200
        %202 =       OpLogicalOr %bool %201 %199
        %203 =       OpCompositeExtract %float %197 2
        %204 =       OpFUnordNotEqual %bool %float_3 %203
        %205 =       OpLogicalOr %bool %204 %202
        %206 =       OpCompositeExtract %float %197 3
        %207 =       OpFUnordNotEqual %bool %float_n4 %206
        %208 =       OpLogicalOr %bool %207 %205
        %209 =       OpCompositeExtract %float %197 4
        %210 =       OpFUnordNotEqual %bool %float_5 %209
        %211 =       OpLogicalOr %bool %210 %208
                     OpBranch %195

        %195 = OpLabel
        %212 =   OpPhi %bool %false %176 %211 %194
                 OpSelectionMerge %214 None
                 OpBranchConditional %212 %213 %214

        %213 =     OpLabel
        %215 =       OpLogicalAnd %bool %true %true
                     OpBranch %214

        %214 = OpLabel
        %216 =   OpPhi %bool %false %195 %215 %213
                 OpSelectionMerge %218 None
                 OpBranchConditional %216 %217 %218

        %217 =     OpLabel
        %219 =       OpINotEqual %v3bool %51 %56
        %221 =       OpAny %bool %219
        %222 =       OpLogicalOr %bool %221 %false
                     OpBranch %218

        %218 = OpLabel
        %223 =   OpPhi %bool %false %214 %222 %217
                 OpSelectionMerge %225 None
                 OpBranchConditional %223 %224 %225

        %224 =     OpLabel
        %227 =       OpFOrdEqual %v2bool %62 %62    ; RelaxedPrecision
        %228 =       OpAll %bool %227
        %229 =       OpFOrdEqual %v2bool %63 %63    ; RelaxedPrecision
        %230 =       OpAll %bool %229
        %231 =       OpLogicalAnd %bool %228 %230
        %232 =       OpFOrdEqual %v2bool %65 %65    ; RelaxedPrecision
        %233 =       OpAll %bool %232
        %234 =       OpFOrdEqual %v2bool %66 %66    ; RelaxedPrecision
        %235 =       OpAll %bool %234
        %236 =       OpLogicalAnd %bool %233 %235
        %237 =       OpLogicalAnd %bool %236 %231
        %238 =       OpFOrdEqual %v2bool %69 %69    ; RelaxedPrecision
        %239 =       OpAll %bool %238
        %240 =       OpFOrdEqual %v2bool %70 %70    ; RelaxedPrecision
        %241 =       OpAll %bool %240
        %242 =       OpLogicalAnd %bool %239 %241
        %243 =       OpLogicalAnd %bool %242 %237
                     OpBranch %225

        %225 = OpLabel
        %244 =   OpPhi %bool %false %218 %243 %224
                 OpSelectionMerge %246 None
                 OpBranchConditional %244 %245 %246

        %245 =     OpLabel
        %247 =       OpFUnordNotEqual %v2bool %62 %62   ; RelaxedPrecision
        %248 =       OpAny %bool %247
        %249 =       OpFUnordNotEqual %v2bool %63 %63   ; RelaxedPrecision
        %250 =       OpAny %bool %249
        %251 =       OpLogicalOr %bool %248 %250
        %252 =       OpFUnordNotEqual %v2bool %65 %75   ; RelaxedPrecision
        %253 =       OpAny %bool %252
        %254 =       OpFUnordNotEqual %v2bool %66 %76   ; RelaxedPrecision
        %255 =       OpAny %bool %254
        %256 =       OpLogicalOr %bool %253 %255
        %257 =       OpLogicalOr %bool %256 %251
        %258 =       OpFUnordNotEqual %v2bool %69 %78   ; RelaxedPrecision
        %259 =       OpAny %bool %258
        %260 =       OpFUnordNotEqual %v2bool %70 %79   ; RelaxedPrecision
        %261 =       OpAny %bool %260
        %262 =       OpLogicalOr %bool %259 %261
        %263 =       OpLogicalOr %bool %262 %257
                     OpBranch %246

        %246 = OpLabel
        %264 =   OpPhi %bool %false %225 %263 %245
                 OpSelectionMerge %266 None
                 OpBranchConditional %264 %265 %266

        %265 =     OpLabel
        %267 =       OpLogicalOr %bool %false %false
        %268 =       OpINotEqual %bool %int_3 %int_0
        %269 =       OpINotEqual %bool %int_4 %int_0
        %270 =       OpLogicalOr %bool %269 %268
        %271 =       OpLogicalOr %bool %270 %267
        %272 =       OpLogicalOr %bool %false %false
        %273 =       OpLogicalOr %bool %272 %271
                     OpBranch %266

        %266 = OpLabel
        %274 =   OpPhi %bool %false %246 %273 %265
                 OpSelectionMerge %276 None
                 OpBranchConditional %274 %275 %276

        %275 =     OpLabel
        %277 =       OpLogicalAnd %bool %true %true
        %278 =       OpLogicalAnd %bool %true %true
        %279 =       OpLogicalAnd %bool %278 %277
        %280 =       OpLogicalAnd %bool %true %true
        %281 =       OpLogicalAnd %bool %280 %279
                     OpBranch %276

        %276 = OpLabel
        %282 =   OpPhi %bool %false %266 %281 %275
                 OpSelectionMerge %287 None
                 OpBranchConditional %282 %285 %286

        %285 =     OpLabel
        %288 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %290 =       OpLoad %v4float %288           ; RelaxedPrecision
                     OpStore %283 %290
                     OpBranch %287

        %286 =     OpLabel
        %291 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %292 =       OpLoad %v4float %291           ; RelaxedPrecision
                     OpStore %283 %292
                     OpBranch %287

        %287 = OpLabel
        %293 =   OpLoad %v4float %283               ; RelaxedPrecision
                 OpReturnValue %293
               OpFunctionEnd
