               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %9
               OpName %_UniformBuffer "_UniformBuffer"  ; id %14
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %16
               OpName %test_half_b "test_half_b"        ; id %6
               OpName %ok "ok"                          ; id %29
               OpName %m1 "m1"                          ; id %32
               OpName %m3 "m3"                          ; id %52
               OpName %m4 "m4"                          ; id %61
               OpName %m5 "m5"                          ; id %91
               OpName %m7 "m7"                          ; id %126
               OpName %m9 "m9"                          ; id %139
               OpName %m10 "m10"                        ; id %160
               OpName %m11 "m11"                        ; id %184
               OpName %test_comma_b "test_comma_b"      ; id %7
               OpName %x "x"                            ; id %213
               OpName %y "y"                            ; id %214
               OpName %main "main"                      ; id %8
               OpName %_0_ok "_0_ok"                    ; id %223
               OpName %_1_m1 "_1_m1"                    ; id %224
               OpName %_2_m3 "_2_m3"                    ; id %233
               OpName %_3_m4 "_3_m4"                    ; id %242
               OpName %_4_m5 "_4_m5"                    ; id %254
               OpName %_7_m10 "_7_m10"                  ; id %280
               OpName %_8_m11 "_8_m11"                  ; id %281

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %13 Binding 0
               OpDecorate %13 DescriptorSet 0
               OpDecorate %m1 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %m3 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %m4 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %m5 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %m7 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %m9 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision
               OpDecorate %m10 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %m11 RelaxedPrecision
               OpDecorate %188 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %190 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
               OpDecorate %202 RelaxedPrecision
               OpDecorate %205 RelaxedPrecision
               OpDecorate %208 RelaxedPrecision
               OpDecorate %317 RelaxedPrecision
               OpDecorate %319 RelaxedPrecision
               OpDecorate %320 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %27 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
         %39 = OpConstantComposite %v2float %float_1 %float_2
         %40 = OpConstantComposite %v2float %float_3 %float_4
         %41 = OpConstantComposite %mat2v2float %39 %40
      %false = OpConstantFalse %bool
     %v2bool = OpTypeVector %bool 2
    %float_6 = OpConstant %float 6
         %63 = OpConstantComposite %v2float %float_6 %float_0
         %64 = OpConstantComposite %v2float %float_0 %float_6
         %65 = OpConstantComposite %mat2v2float %63 %64
   %float_12 = OpConstant %float 12
   %float_18 = OpConstant %float 18
   %float_24 = OpConstant %float 24
         %80 = OpConstantComposite %v2float %float_6 %float_12
         %81 = OpConstantComposite %v2float %float_18 %float_24
         %82 = OpConstantComposite %mat2v2float %80 %81
      %int_1 = OpConstant %int 1
        %101 = OpConstantComposite %v2float %float_4 %float_0
        %102 = OpConstantComposite %v2float %float_0 %float_4
        %103 = OpConstantComposite %mat2v2float %101 %102
    %float_5 = OpConstant %float 5
    %float_8 = OpConstant %float 8
        %117 = OpConstantComposite %v2float %float_5 %float_2
        %118 = OpConstantComposite %v2float %float_3 %float_8
        %119 = OpConstantComposite %mat2v2float %117 %118
    %float_7 = OpConstant %float 7
        %128 = OpConstantComposite %v2float %float_5 %float_6
        %129 = OpConstantComposite %v2float %float_7 %float_8
        %130 = OpConstantComposite %mat2v2float %128 %129
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
    %float_9 = OpConstant %float 9
        %144 = OpConstantComposite %v3float %float_9 %float_0 %float_0
        %145 = OpConstantComposite %v3float %float_0 %float_9 %float_0
        %146 = OpConstantComposite %v3float %float_0 %float_0 %float_9
        %147 = OpConstantComposite %mat3v3float %144 %145 %146
     %v3bool = OpTypeVector %bool 3
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
   %float_11 = OpConstant %float 11
        %164 = OpConstantComposite %v4float %float_11 %float_0 %float_0 %float_0
        %165 = OpConstantComposite %v4float %float_0 %float_11 %float_0 %float_0
        %166 = OpConstantComposite %v4float %float_0 %float_0 %float_11 %float_0
        %167 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_11
        %168 = OpConstantComposite %mat4v4float %164 %165 %166 %167
     %v4bool = OpTypeVector %bool 4
   %float_20 = OpConstant %float 20
        %186 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_20
        %187 = OpConstantComposite %mat4v4float %186 %186 %186 %186
        %195 = OpConstantComposite %v4float %float_9 %float_20 %float_20 %float_20
        %196 = OpConstantComposite %v4float %float_20 %float_9 %float_20 %float_20
        %197 = OpConstantComposite %v4float %float_20 %float_20 %float_9 %float_20
        %198 = OpConstantComposite %v4float %float_20 %float_20 %float_20 %float_9
        %199 = OpConstantComposite %mat4v4float %195 %196 %197 %198
        %220 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %18

         %19 = OpLabel
         %23 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %23 %22
         %25 =   OpFunctionCall %v4float %main %23
                 OpStore %sk_FragColor %25
                 OpReturn
               OpFunctionEnd


               ; Function test_half_b
%test_half_b = OpFunction %bool None %27

         %28 = OpLabel
         %ok =   OpVariable %_ptr_Function_bool Function
         %m1 =   OpVariable %_ptr_Function_mat2v2float Function     ; RelaxedPrecision
         %m3 =   OpVariable %_ptr_Function_mat2v2float Function     ; RelaxedPrecision
         %m4 =   OpVariable %_ptr_Function_mat2v2float Function     ; RelaxedPrecision
         %m5 =   OpVariable %_ptr_Function_mat2v2float Function     ; RelaxedPrecision
         %m7 =   OpVariable %_ptr_Function_mat2v2float Function     ; RelaxedPrecision
         %m9 =   OpVariable %_ptr_Function_mat3v3float Function     ; RelaxedPrecision
        %m10 =   OpVariable %_ptr_Function_mat4v4float Function     ; RelaxedPrecision
        %m11 =   OpVariable %_ptr_Function_mat4v4float Function     ; RelaxedPrecision
                 OpStore %ok %true
                 OpStore %m1 %41
                 OpSelectionMerge %44 None
                 OpBranchConditional %true %43 %44

         %43 =     OpLabel
         %46 =       OpFOrdEqual %v2bool %39 %39    ; RelaxedPrecision
         %47 =       OpAll %bool %46
         %48 =       OpFOrdEqual %v2bool %40 %40    ; RelaxedPrecision
         %49 =       OpAll %bool %48
         %50 =       OpLogicalAnd %bool %47 %49
                     OpBranch %44

         %44 = OpLabel
         %51 =   OpPhi %bool %false %28 %50 %43
                 OpStore %ok %51
                 OpStore %m3 %41
                 OpSelectionMerge %54 None
                 OpBranchConditional %51 %53 %54

         %53 =     OpLabel
         %55 =       OpFOrdEqual %v2bool %39 %39    ; RelaxedPrecision
         %56 =       OpAll %bool %55
         %57 =       OpFOrdEqual %v2bool %40 %40    ; RelaxedPrecision
         %58 =       OpAll %bool %57
         %59 =       OpLogicalAnd %bool %56 %58
                     OpBranch %54

         %54 = OpLabel
         %60 =   OpPhi %bool %false %44 %59 %53
                 OpStore %ok %60
                 OpStore %m4 %65
                 OpSelectionMerge %67 None
                 OpBranchConditional %60 %66 %67

         %66 =     OpLabel
         %68 =       OpFOrdEqual %v2bool %63 %63    ; RelaxedPrecision
         %69 =       OpAll %bool %68
         %70 =       OpFOrdEqual %v2bool %64 %64    ; RelaxedPrecision
         %71 =       OpAll %bool %70
         %72 =       OpLogicalAnd %bool %69 %71
                     OpBranch %67

         %67 = OpLabel
         %73 =   OpPhi %bool %false %54 %72 %66
                 OpStore %ok %73
         %74 =   OpMatrixTimesMatrix %mat2v2float %41 %65   ; RelaxedPrecision
                 OpStore %m3 %74
                 OpSelectionMerge %76 None
                 OpBranchConditional %73 %75 %76

         %75 =     OpLabel
         %83 =       OpCompositeExtract %v2float %74 0  ; RelaxedPrecision
         %84 =       OpFOrdEqual %v2bool %83 %80        ; RelaxedPrecision
         %85 =       OpAll %bool %84
         %86 =       OpCompositeExtract %v2float %74 1  ; RelaxedPrecision
         %87 =       OpFOrdEqual %v2bool %86 %81        ; RelaxedPrecision
         %88 =       OpAll %bool %87
         %89 =       OpLogicalAnd %bool %85 %88
                     OpBranch %76

         %76 = OpLabel
         %90 =   OpPhi %bool %false %67 %89 %75
                 OpStore %ok %90
         %93 =   OpAccessChain %_ptr_Function_v2float %m1 %int_1
         %94 =   OpLoad %v2float %93                ; RelaxedPrecision
         %95 =   OpCompositeExtract %float %94 1    ; RelaxedPrecision
         %96 =   OpCompositeConstruct %v2float %95 %float_0     ; RelaxedPrecision
         %97 =   OpCompositeConstruct %v2float %float_0 %95     ; RelaxedPrecision
         %98 =   OpCompositeConstruct %mat2v2float %96 %97      ; RelaxedPrecision
                 OpStore %m5 %98
                 OpSelectionMerge %100 None
                 OpBranchConditional %90 %99 %100

         %99 =     OpLabel
        %104 =       OpFOrdEqual %v2bool %96 %101   ; RelaxedPrecision
        %105 =       OpAll %bool %104
        %106 =       OpFOrdEqual %v2bool %97 %102   ; RelaxedPrecision
        %107 =       OpAll %bool %106
        %108 =       OpLogicalAnd %bool %105 %107
                     OpBranch %100

        %100 = OpLabel
        %109 =   OpPhi %bool %false %76 %108 %99
                 OpStore %ok %109
        %110 =   OpFAdd %v2float %39 %96            ; RelaxedPrecision
        %111 =   OpFAdd %v2float %40 %97            ; RelaxedPrecision
        %112 =   OpCompositeConstruct %mat2v2float %110 %111    ; RelaxedPrecision
                 OpStore %m1 %112
                 OpSelectionMerge %114 None
                 OpBranchConditional %109 %113 %114

        %113 =     OpLabel
        %120 =       OpFOrdEqual %v2bool %110 %117  ; RelaxedPrecision
        %121 =       OpAll %bool %120
        %122 =       OpFOrdEqual %v2bool %111 %118  ; RelaxedPrecision
        %123 =       OpAll %bool %122
        %124 =       OpLogicalAnd %bool %121 %123
                     OpBranch %114

        %114 = OpLabel
        %125 =   OpPhi %bool %false %100 %124 %113
                 OpStore %ok %125
                 OpStore %m7 %130
                 OpSelectionMerge %132 None
                 OpBranchConditional %125 %131 %132

        %131 =     OpLabel
        %133 =       OpFOrdEqual %v2bool %128 %128  ; RelaxedPrecision
        %134 =       OpAll %bool %133
        %135 =       OpFOrdEqual %v2bool %129 %129  ; RelaxedPrecision
        %136 =       OpAll %bool %135
        %137 =       OpLogicalAnd %bool %134 %136
                     OpBranch %132

        %132 = OpLabel
        %138 =   OpPhi %bool %false %114 %137 %131
                 OpStore %ok %138
                 OpStore %m9 %147
                 OpSelectionMerge %149 None
                 OpBranchConditional %138 %148 %149

        %148 =     OpLabel
        %151 =       OpFOrdEqual %v3bool %144 %144  ; RelaxedPrecision
        %152 =       OpAll %bool %151
        %153 =       OpFOrdEqual %v3bool %145 %145  ; RelaxedPrecision
        %154 =       OpAll %bool %153
        %155 =       OpLogicalAnd %bool %152 %154
        %156 =       OpFOrdEqual %v3bool %146 %146  ; RelaxedPrecision
        %157 =       OpAll %bool %156
        %158 =       OpLogicalAnd %bool %155 %157
                     OpBranch %149

        %149 = OpLabel
        %159 =   OpPhi %bool %false %132 %158 %148
                 OpStore %ok %159
                 OpStore %m10 %168
                 OpSelectionMerge %170 None
                 OpBranchConditional %159 %169 %170

        %169 =     OpLabel
        %172 =       OpFOrdEqual %v4bool %164 %164  ; RelaxedPrecision
        %173 =       OpAll %bool %172
        %174 =       OpFOrdEqual %v4bool %165 %165  ; RelaxedPrecision
        %175 =       OpAll %bool %174
        %176 =       OpLogicalAnd %bool %173 %175
        %177 =       OpFOrdEqual %v4bool %166 %166  ; RelaxedPrecision
        %178 =       OpAll %bool %177
        %179 =       OpLogicalAnd %bool %176 %178
        %180 =       OpFOrdEqual %v4bool %167 %167  ; RelaxedPrecision
        %181 =       OpAll %bool %180
        %182 =       OpLogicalAnd %bool %179 %181
                     OpBranch %170

        %170 = OpLabel
        %183 =   OpPhi %bool %false %149 %182 %169
                 OpStore %ok %183
                 OpStore %m11 %187
        %188 =   OpFSub %v4float %186 %164          ; RelaxedPrecision
        %189 =   OpFSub %v4float %186 %165          ; RelaxedPrecision
        %190 =   OpFSub %v4float %186 %166          ; RelaxedPrecision
        %191 =   OpFSub %v4float %186 %167          ; RelaxedPrecision
        %192 =   OpCompositeConstruct %mat4v4float %188 %189 %190 %191  ; RelaxedPrecision
                 OpStore %m11 %192
                 OpSelectionMerge %194 None
                 OpBranchConditional %183 %193 %194

        %193 =     OpLabel
        %200 =       OpFOrdEqual %v4bool %188 %195  ; RelaxedPrecision
        %201 =       OpAll %bool %200
        %202 =       OpFOrdEqual %v4bool %189 %196  ; RelaxedPrecision
        %203 =       OpAll %bool %202
        %204 =       OpLogicalAnd %bool %201 %203
        %205 =       OpFOrdEqual %v4bool %190 %197  ; RelaxedPrecision
        %206 =       OpAll %bool %205
        %207 =       OpLogicalAnd %bool %204 %206
        %208 =       OpFOrdEqual %v4bool %191 %198  ; RelaxedPrecision
        %209 =       OpAll %bool %208
        %210 =       OpLogicalAnd %bool %207 %209
                     OpBranch %194

        %194 = OpLabel
        %211 =   OpPhi %bool %false %170 %210 %193
                 OpStore %ok %211
                 OpReturnValue %211
               OpFunctionEnd


               ; Function test_comma_b
%test_comma_b = OpFunction %bool None %27

        %212 = OpLabel
          %x =   OpVariable %_ptr_Function_mat2v2float Function
          %y =   OpVariable %_ptr_Function_mat2v2float Function
                 OpStore %x %41
                 OpStore %y %41
        %215 =   OpFOrdEqual %v2bool %39 %39
        %216 =   OpAll %bool %215
        %217 =   OpFOrdEqual %v2bool %40 %40
        %218 =   OpAll %bool %217
        %219 =   OpLogicalAnd %bool %216 %218
                 OpReturnValue %219
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %220        ; RelaxedPrecision
        %221 = OpFunctionParameter %_ptr_Function_v2float

        %222 = OpLabel
      %_0_ok =   OpVariable %_ptr_Function_bool Function
      %_1_m1 =   OpVariable %_ptr_Function_mat2v2float Function
      %_2_m3 =   OpVariable %_ptr_Function_mat2v2float Function
      %_3_m4 =   OpVariable %_ptr_Function_mat2v2float Function
      %_4_m5 =   OpVariable %_ptr_Function_mat2v2float Function
     %_7_m10 =   OpVariable %_ptr_Function_mat4v4float Function
     %_8_m11 =   OpVariable %_ptr_Function_mat4v4float Function
        %309 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %_0_ok %true
                 OpStore %_1_m1 %41
                 OpSelectionMerge %226 None
                 OpBranchConditional %true %225 %226

        %225 =     OpLabel
        %227 =       OpFOrdEqual %v2bool %39 %39
        %228 =       OpAll %bool %227
        %229 =       OpFOrdEqual %v2bool %40 %40
        %230 =       OpAll %bool %229
        %231 =       OpLogicalAnd %bool %228 %230
                     OpBranch %226

        %226 = OpLabel
        %232 =   OpPhi %bool %false %222 %231 %225
                 OpStore %_0_ok %232
                 OpStore %_2_m3 %41
                 OpSelectionMerge %235 None
                 OpBranchConditional %232 %234 %235

        %234 =     OpLabel
        %236 =       OpFOrdEqual %v2bool %39 %39
        %237 =       OpAll %bool %236
        %238 =       OpFOrdEqual %v2bool %40 %40
        %239 =       OpAll %bool %238
        %240 =       OpLogicalAnd %bool %237 %239
                     OpBranch %235

        %235 = OpLabel
        %241 =   OpPhi %bool %false %226 %240 %234
                 OpStore %_0_ok %241
                 OpStore %_3_m4 %65
        %243 =   OpMatrixTimesMatrix %mat2v2float %41 %65
                 OpStore %_2_m3 %243
                 OpSelectionMerge %245 None
                 OpBranchConditional %241 %244 %245

        %244 =     OpLabel
        %246 =       OpCompositeExtract %v2float %243 0
        %247 =       OpFOrdEqual %v2bool %246 %80
        %248 =       OpAll %bool %247
        %249 =       OpCompositeExtract %v2float %243 1
        %250 =       OpFOrdEqual %v2bool %249 %81
        %251 =       OpAll %bool %250
        %252 =       OpLogicalAnd %bool %248 %251
                     OpBranch %245

        %245 = OpLabel
        %253 =   OpPhi %bool %false %235 %252 %244
                 OpStore %_0_ok %253
        %255 =   OpAccessChain %_ptr_Function_v2float %_1_m1 %int_1
        %256 =   OpLoad %v2float %255
        %257 =   OpCompositeExtract %float %256 1
        %258 =   OpCompositeConstruct %v2float %257 %float_0
        %259 =   OpCompositeConstruct %v2float %float_0 %257
        %260 =   OpCompositeConstruct %mat2v2float %258 %259
                 OpStore %_4_m5 %260
                 OpSelectionMerge %262 None
                 OpBranchConditional %253 %261 %262

        %261 =     OpLabel
        %263 =       OpFOrdEqual %v2bool %258 %101
        %264 =       OpAll %bool %263
        %265 =       OpFOrdEqual %v2bool %259 %102
        %266 =       OpAll %bool %265
        %267 =       OpLogicalAnd %bool %264 %266
                     OpBranch %262

        %262 = OpLabel
        %268 =   OpPhi %bool %false %245 %267 %261
                 OpStore %_0_ok %268
        %269 =   OpFAdd %v2float %39 %258
        %270 =   OpFAdd %v2float %40 %259
        %271 =   OpCompositeConstruct %mat2v2float %269 %270
                 OpStore %_1_m1 %271
                 OpSelectionMerge %273 None
                 OpBranchConditional %268 %272 %273

        %272 =     OpLabel
        %274 =       OpFOrdEqual %v2bool %269 %117
        %275 =       OpAll %bool %274
        %276 =       OpFOrdEqual %v2bool %270 %118
        %277 =       OpAll %bool %276
        %278 =       OpLogicalAnd %bool %275 %277
                     OpBranch %273

        %273 = OpLabel
        %279 =   OpPhi %bool %false %262 %278 %272
                 OpStore %_0_ok %279
                 OpStore %_7_m10 %168
                 OpStore %_8_m11 %187
        %282 =   OpFSub %v4float %186 %164
        %283 =   OpFSub %v4float %186 %165
        %284 =   OpFSub %v4float %186 %166
        %285 =   OpFSub %v4float %186 %167
        %286 =   OpCompositeConstruct %mat4v4float %282 %283 %284 %285
                 OpStore %_8_m11 %286
                 OpSelectionMerge %288 None
                 OpBranchConditional %279 %287 %288

        %287 =     OpLabel
        %289 =       OpFOrdEqual %v4bool %282 %195
        %290 =       OpAll %bool %289
        %291 =       OpFOrdEqual %v4bool %283 %196
        %292 =       OpAll %bool %291
        %293 =       OpLogicalAnd %bool %290 %292
        %294 =       OpFOrdEqual %v4bool %284 %197
        %295 =       OpAll %bool %294
        %296 =       OpLogicalAnd %bool %293 %295
        %297 =       OpFOrdEqual %v4bool %285 %198
        %298 =       OpAll %bool %297
        %299 =       OpLogicalAnd %bool %296 %298
                     OpBranch %288

        %288 = OpLabel
        %300 =   OpPhi %bool %false %273 %299 %287
                 OpStore %_0_ok %300
                 OpSelectionMerge %302 None
                 OpBranchConditional %300 %301 %302

        %301 =     OpLabel
        %303 =       OpFunctionCall %bool %test_half_b
                     OpBranch %302

        %302 = OpLabel
        %304 =   OpPhi %bool %false %288 %303 %301
                 OpSelectionMerge %306 None
                 OpBranchConditional %304 %305 %306

        %305 =     OpLabel
        %307 =       OpFunctionCall %bool %test_comma_b
                     OpBranch %306

        %306 = OpLabel
        %308 =   OpPhi %bool %false %302 %307 %305
                 OpSelectionMerge %313 None
                 OpBranchConditional %308 %311 %312

        %311 =     OpLabel
        %314 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_0
        %317 =       OpLoad %v4float %314           ; RelaxedPrecision
                     OpStore %309 %317
                     OpBranch %313

        %312 =     OpLabel
        %318 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_1
        %319 =       OpLoad %v4float %318           ; RelaxedPrecision
                     OpStore %309 %319
                     OpBranch %313

        %313 = OpLabel
        %320 =   OpLoad %v4float %309               ; RelaxedPrecision
                 OpReturnValue %320
               OpFunctionEnd
