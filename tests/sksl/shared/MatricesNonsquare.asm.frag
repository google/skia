               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %8
               OpName %_UniformBuffer "_UniformBuffer"  ; id %13
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %15
               OpName %test_half_b "test_half_b"        ; id %6
               OpName %ok "ok"                          ; id %28
               OpName %m23 "m23"                        ; id %31
               OpName %m24 "m24"                        ; id %49
               OpName %m32 "m32"                        ; id %65
               OpName %m34 "m34"                        ; id %84
               OpName %m42 "m42"                        ; id %103
               OpName %m43 "m43"                        ; id %124
               OpName %m22 "m22"                        ; id %147
               OpName %m33 "m33"                        ; id %165
               OpName %main "main"                      ; id %7
               OpName %_0_ok "_0_ok"                    ; id %246
               OpName %_1_m23 "_1_m23"                  ; id %247
               OpName %_2_m24 "_2_m24"                  ; id %256
               OpName %_3_m32 "_3_m32"                  ; id %265
               OpName %_7_m22 "_7_m22"                  ; id %277

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
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %m23 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %m24 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %m32 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %m34 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %m42 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %m43 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %m22 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
               OpDecorate %m33 RelaxedPrecision
               OpDecorate %168 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %207 RelaxedPrecision
               OpDecorate %208 RelaxedPrecision
               OpDecorate %209 RelaxedPrecision
               OpDecorate %210 RelaxedPrecision
               OpDecorate %218 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %223 RelaxedPrecision
               OpDecorate %228 RelaxedPrecision
               OpDecorate %235 RelaxedPrecision
               OpDecorate %236 RelaxedPrecision
               OpDecorate %238 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %338 RelaxedPrecision
               OpDecorate %341 RelaxedPrecision
               OpDecorate %342 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %26 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
    %v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
    %float_2 = OpConstant %float 2
         %36 = OpConstantComposite %v3float %float_2 %float_0 %float_0
         %37 = OpConstantComposite %v3float %float_0 %float_2 %float_0
         %38 = OpConstantComposite %mat2v3float %36 %37
      %false = OpConstantFalse %bool
     %v3bool = OpTypeVector %bool 3
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
    %float_3 = OpConstant %float 3
         %53 = OpConstantComposite %v4float %float_3 %float_0 %float_0 %float_0
         %54 = OpConstantComposite %v4float %float_0 %float_3 %float_0 %float_0
         %55 = OpConstantComposite %mat2v4float %53 %54
     %v4bool = OpTypeVector %bool 4
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
    %float_4 = OpConstant %float 4
         %69 = OpConstantComposite %v2float %float_4 %float_0
         %70 = OpConstantComposite %v2float %float_0 %float_4
         %71 = OpConstantComposite %mat3v2float %69 %70 %21
     %v2bool = OpTypeVector %bool 2
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
    %float_5 = OpConstant %float 5
         %88 = OpConstantComposite %v4float %float_5 %float_0 %float_0 %float_0
         %89 = OpConstantComposite %v4float %float_0 %float_5 %float_0 %float_0
         %90 = OpConstantComposite %v4float %float_0 %float_0 %float_5 %float_0
         %91 = OpConstantComposite %mat3v4float %88 %89 %90
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
    %float_6 = OpConstant %float 6
        %107 = OpConstantComposite %v2float %float_6 %float_0
        %108 = OpConstantComposite %v2float %float_0 %float_6
        %109 = OpConstantComposite %mat4v2float %107 %108 %21 %21
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
    %float_7 = OpConstant %float 7
        %128 = OpConstantComposite %v3float %float_7 %float_0 %float_0
        %129 = OpConstantComposite %v3float %float_0 %float_7 %float_0
        %130 = OpConstantComposite %v3float %float_0 %float_0 %float_7
        %131 = OpConstantComposite %v3float %float_0 %float_0 %float_0
        %132 = OpConstantComposite %mat4v3float %128 %129 %130 %131
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
    %float_8 = OpConstant %float 8
        %154 = OpConstantComposite %v2float %float_8 %float_0
        %155 = OpConstantComposite %v2float %float_0 %float_8
        %156 = OpConstantComposite %mat2v2float %154 %155
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
   %float_35 = OpConstant %float 35
        %172 = OpConstantComposite %v3float %float_35 %float_0 %float_0
        %173 = OpConstantComposite %v3float %float_0 %float_35 %float_0
        %174 = OpConstantComposite %v3float %float_0 %float_0 %float_35
        %175 = OpConstantComposite %mat3v3float %172 %173 %174
    %float_1 = OpConstant %float 1
        %189 = OpConstantComposite %v3float %float_1 %float_1 %float_1
        %190 = OpConstantComposite %mat2v3float %189 %189
        %196 = OpConstantComposite %v3float %float_3 %float_1 %float_1
        %197 = OpConstantComposite %v3float %float_1 %float_3 %float_1
        %198 = OpConstantComposite %mat2v3float %196 %197
        %205 = OpConstantComposite %v2float %float_2 %float_2
        %206 = OpConstantComposite %mat3v2float %205 %205 %205
   %float_n2 = OpConstant %float -2
        %214 = OpConstantComposite %v2float %float_2 %float_n2
        %215 = OpConstantComposite %v2float %float_n2 %float_2
        %216 = OpConstantComposite %v2float %float_n2 %float_n2
        %217 = OpConstantComposite %mat3v2float %214 %215 %216
 %float_0_25 = OpConstant %float 0.25
 %float_0_75 = OpConstant %float 0.75
        %232 = OpConstantComposite %v4float %float_0_75 %float_0 %float_0 %float_0
        %233 = OpConstantComposite %v4float %float_0 %float_0_75 %float_0 %float_0
        %234 = OpConstantComposite %mat2v4float %232 %233
        %243 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %17

         %18 = OpLabel
         %22 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %22 %21
         %24 =   OpFunctionCall %v4float %main %22
                 OpStore %sk_FragColor %24
                 OpReturn
               OpFunctionEnd


               ; Function test_half_b
%test_half_b = OpFunction %bool None %26

         %27 = OpLabel
         %ok =   OpVariable %_ptr_Function_bool Function
        %m23 =   OpVariable %_ptr_Function_mat2v3float Function     ; RelaxedPrecision
        %m24 =   OpVariable %_ptr_Function_mat2v4float Function     ; RelaxedPrecision
        %m32 =   OpVariable %_ptr_Function_mat3v2float Function     ; RelaxedPrecision
        %m34 =   OpVariable %_ptr_Function_mat3v4float Function     ; RelaxedPrecision
        %m42 =   OpVariable %_ptr_Function_mat4v2float Function     ; RelaxedPrecision
        %m43 =   OpVariable %_ptr_Function_mat4v3float Function     ; RelaxedPrecision
        %m22 =   OpVariable %_ptr_Function_mat2v2float Function     ; RelaxedPrecision
        %m33 =   OpVariable %_ptr_Function_mat3v3float Function     ; RelaxedPrecision
                 OpStore %ok %true
                 OpStore %m23 %38
                 OpSelectionMerge %41 None
                 OpBranchConditional %true %40 %41

         %40 =     OpLabel
         %43 =       OpFOrdEqual %v3bool %36 %36    ; RelaxedPrecision
         %44 =       OpAll %bool %43
         %45 =       OpFOrdEqual %v3bool %37 %37    ; RelaxedPrecision
         %46 =       OpAll %bool %45
         %47 =       OpLogicalAnd %bool %44 %46
                     OpBranch %41

         %41 = OpLabel
         %48 =   OpPhi %bool %false %27 %47 %40
                 OpStore %ok %48
                 OpStore %m24 %55
                 OpSelectionMerge %57 None
                 OpBranchConditional %48 %56 %57

         %56 =     OpLabel
         %59 =       OpFOrdEqual %v4bool %53 %53    ; RelaxedPrecision
         %60 =       OpAll %bool %59
         %61 =       OpFOrdEqual %v4bool %54 %54    ; RelaxedPrecision
         %62 =       OpAll %bool %61
         %63 =       OpLogicalAnd %bool %60 %62
                     OpBranch %57

         %57 = OpLabel
         %64 =   OpPhi %bool %false %41 %63 %56
                 OpStore %ok %64
                 OpStore %m32 %71
                 OpSelectionMerge %73 None
                 OpBranchConditional %64 %72 %73

         %72 =     OpLabel
         %75 =       OpFOrdEqual %v2bool %69 %69    ; RelaxedPrecision
         %76 =       OpAll %bool %75
         %77 =       OpFOrdEqual %v2bool %70 %70    ; RelaxedPrecision
         %78 =       OpAll %bool %77
         %79 =       OpLogicalAnd %bool %76 %78
         %80 =       OpFOrdEqual %v2bool %21 %21    ; RelaxedPrecision
         %81 =       OpAll %bool %80
         %82 =       OpLogicalAnd %bool %79 %81
                     OpBranch %73

         %73 = OpLabel
         %83 =   OpPhi %bool %false %57 %82 %72
                 OpStore %ok %83
                 OpStore %m34 %91
                 OpSelectionMerge %93 None
                 OpBranchConditional %83 %92 %93

         %92 =     OpLabel
         %94 =       OpFOrdEqual %v4bool %88 %88    ; RelaxedPrecision
         %95 =       OpAll %bool %94
         %96 =       OpFOrdEqual %v4bool %89 %89    ; RelaxedPrecision
         %97 =       OpAll %bool %96
         %98 =       OpLogicalAnd %bool %95 %97
         %99 =       OpFOrdEqual %v4bool %90 %90    ; RelaxedPrecision
        %100 =       OpAll %bool %99
        %101 =       OpLogicalAnd %bool %98 %100
                     OpBranch %93

         %93 = OpLabel
        %102 =   OpPhi %bool %false %73 %101 %92
                 OpStore %ok %102
                 OpStore %m42 %109
                 OpSelectionMerge %111 None
                 OpBranchConditional %102 %110 %111

        %110 =     OpLabel
        %112 =       OpFOrdEqual %v2bool %107 %107  ; RelaxedPrecision
        %113 =       OpAll %bool %112
        %114 =       OpFOrdEqual %v2bool %108 %108  ; RelaxedPrecision
        %115 =       OpAll %bool %114
        %116 =       OpLogicalAnd %bool %113 %115
        %117 =       OpFOrdEqual %v2bool %21 %21    ; RelaxedPrecision
        %118 =       OpAll %bool %117
        %119 =       OpLogicalAnd %bool %116 %118
        %120 =       OpFOrdEqual %v2bool %21 %21    ; RelaxedPrecision
        %121 =       OpAll %bool %120
        %122 =       OpLogicalAnd %bool %119 %121
                     OpBranch %111

        %111 = OpLabel
        %123 =   OpPhi %bool %false %93 %122 %110
                 OpStore %ok %123
                 OpStore %m43 %132
                 OpSelectionMerge %134 None
                 OpBranchConditional %123 %133 %134

        %133 =     OpLabel
        %135 =       OpFOrdEqual %v3bool %128 %128  ; RelaxedPrecision
        %136 =       OpAll %bool %135
        %137 =       OpFOrdEqual %v3bool %129 %129  ; RelaxedPrecision
        %138 =       OpAll %bool %137
        %139 =       OpLogicalAnd %bool %136 %138
        %140 =       OpFOrdEqual %v3bool %130 %130  ; RelaxedPrecision
        %141 =       OpAll %bool %140
        %142 =       OpLogicalAnd %bool %139 %141
        %143 =       OpFOrdEqual %v3bool %131 %131  ; RelaxedPrecision
        %144 =       OpAll %bool %143
        %145 =       OpLogicalAnd %bool %142 %144
                     OpBranch %134

        %134 = OpLabel
        %146 =   OpPhi %bool %false %111 %145 %133
                 OpStore %ok %146
        %150 =   OpMatrixTimesMatrix %mat2v2float %71 %38   ; RelaxedPrecision
                 OpStore %m22 %150
                 OpSelectionMerge %152 None
                 OpBranchConditional %146 %151 %152

        %151 =     OpLabel
        %157 =       OpCompositeExtract %v2float %150 0     ; RelaxedPrecision
        %158 =       OpFOrdEqual %v2bool %157 %154          ; RelaxedPrecision
        %159 =       OpAll %bool %158
        %160 =       OpCompositeExtract %v2float %150 1     ; RelaxedPrecision
        %161 =       OpFOrdEqual %v2bool %160 %155          ; RelaxedPrecision
        %162 =       OpAll %bool %161
        %163 =       OpLogicalAnd %bool %159 %162
                     OpBranch %152

        %152 = OpLabel
        %164 =   OpPhi %bool %false %134 %163 %151
                 OpStore %ok %164
        %168 =   OpMatrixTimesMatrix %mat3v3float %132 %91  ; RelaxedPrecision
                 OpStore %m33 %168
                 OpSelectionMerge %170 None
                 OpBranchConditional %164 %169 %170

        %169 =     OpLabel
        %176 =       OpCompositeExtract %v3float %168 0     ; RelaxedPrecision
        %177 =       OpFOrdEqual %v3bool %176 %172          ; RelaxedPrecision
        %178 =       OpAll %bool %177
        %179 =       OpCompositeExtract %v3float %168 1     ; RelaxedPrecision
        %180 =       OpFOrdEqual %v3bool %179 %173          ; RelaxedPrecision
        %181 =       OpAll %bool %180
        %182 =       OpLogicalAnd %bool %178 %181
        %183 =       OpCompositeExtract %v3float %168 2     ; RelaxedPrecision
        %184 =       OpFOrdEqual %v3bool %183 %174          ; RelaxedPrecision
        %185 =       OpAll %bool %184
        %186 =       OpLogicalAnd %bool %182 %185
                     OpBranch %170

        %170 = OpLabel
        %187 =   OpPhi %bool %false %152 %186 %169
                 OpStore %ok %187
        %191 =   OpFAdd %v3float %36 %189           ; RelaxedPrecision
        %192 =   OpFAdd %v3float %37 %189           ; RelaxedPrecision
        %193 =   OpCompositeConstruct %mat2v3float %191 %192    ; RelaxedPrecision
                 OpStore %m23 %193
                 OpSelectionMerge %195 None
                 OpBranchConditional %187 %194 %195

        %194 =     OpLabel
        %199 =       OpFOrdEqual %v3bool %191 %196  ; RelaxedPrecision
        %200 =       OpAll %bool %199
        %201 =       OpFOrdEqual %v3bool %192 %197  ; RelaxedPrecision
        %202 =       OpAll %bool %201
        %203 =       OpLogicalAnd %bool %200 %202
                     OpBranch %195

        %195 = OpLabel
        %204 =   OpPhi %bool %false %170 %203 %194
                 OpStore %ok %204
        %207 =   OpFSub %v2float %69 %205           ; RelaxedPrecision
        %208 =   OpFSub %v2float %70 %205           ; RelaxedPrecision
        %209 =   OpFSub %v2float %21 %205           ; RelaxedPrecision
        %210 =   OpCompositeConstruct %mat3v2float %207 %208 %209   ; RelaxedPrecision
                 OpStore %m32 %210
                 OpSelectionMerge %212 None
                 OpBranchConditional %204 %211 %212

        %211 =     OpLabel
        %218 =       OpFOrdEqual %v2bool %207 %214  ; RelaxedPrecision
        %219 =       OpAll %bool %218
        %220 =       OpFOrdEqual %v2bool %208 %215  ; RelaxedPrecision
        %221 =       OpAll %bool %220
        %222 =       OpLogicalAnd %bool %219 %221
        %223 =       OpFOrdEqual %v2bool %209 %216  ; RelaxedPrecision
        %224 =       OpAll %bool %223
        %225 =       OpLogicalAnd %bool %222 %224
                     OpBranch %212

        %212 = OpLabel
        %226 =   OpPhi %bool %false %195 %225 %211
                 OpStore %ok %226
        %228 =   OpMatrixTimesScalar %mat2v4float %55 %float_0_25   ; RelaxedPrecision
                 OpStore %m24 %228
                 OpSelectionMerge %230 None
                 OpBranchConditional %226 %229 %230

        %229 =     OpLabel
        %235 =       OpCompositeExtract %v4float %228 0     ; RelaxedPrecision
        %236 =       OpFOrdEqual %v4bool %235 %232          ; RelaxedPrecision
        %237 =       OpAll %bool %236
        %238 =       OpCompositeExtract %v4float %228 1     ; RelaxedPrecision
        %239 =       OpFOrdEqual %v4bool %238 %233          ; RelaxedPrecision
        %240 =       OpAll %bool %239
        %241 =       OpLogicalAnd %bool %237 %240
                     OpBranch %230

        %230 = OpLabel
        %242 =   OpPhi %bool %false %212 %241 %229
                 OpStore %ok %242
                 OpReturnValue %242
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %243        ; RelaxedPrecision
        %244 = OpFunctionParameter %_ptr_Function_v2float

        %245 = OpLabel
      %_0_ok =   OpVariable %_ptr_Function_bool Function
     %_1_m23 =   OpVariable %_ptr_Function_mat2v3float Function
     %_2_m24 =   OpVariable %_ptr_Function_mat2v4float Function
     %_3_m32 =   OpVariable %_ptr_Function_mat3v2float Function
     %_7_m22 =   OpVariable %_ptr_Function_mat2v2float Function
        %330 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %_0_ok %true
                 OpStore %_1_m23 %38
                 OpSelectionMerge %249 None
                 OpBranchConditional %true %248 %249

        %248 =     OpLabel
        %250 =       OpFOrdEqual %v3bool %36 %36
        %251 =       OpAll %bool %250
        %252 =       OpFOrdEqual %v3bool %37 %37
        %253 =       OpAll %bool %252
        %254 =       OpLogicalAnd %bool %251 %253
                     OpBranch %249

        %249 = OpLabel
        %255 =   OpPhi %bool %false %245 %254 %248
                 OpStore %_0_ok %255
                 OpStore %_2_m24 %55
                 OpSelectionMerge %258 None
                 OpBranchConditional %255 %257 %258

        %257 =     OpLabel
        %259 =       OpFOrdEqual %v4bool %53 %53
        %260 =       OpAll %bool %259
        %261 =       OpFOrdEqual %v4bool %54 %54
        %262 =       OpAll %bool %261
        %263 =       OpLogicalAnd %bool %260 %262
                     OpBranch %258

        %258 = OpLabel
        %264 =   OpPhi %bool %false %249 %263 %257
                 OpStore %_0_ok %264
                 OpStore %_3_m32 %71
                 OpSelectionMerge %267 None
                 OpBranchConditional %264 %266 %267

        %266 =     OpLabel
        %268 =       OpFOrdEqual %v2bool %69 %69
        %269 =       OpAll %bool %268
        %270 =       OpFOrdEqual %v2bool %70 %70
        %271 =       OpAll %bool %270
        %272 =       OpLogicalAnd %bool %269 %271
        %273 =       OpFOrdEqual %v2bool %21 %21
        %274 =       OpAll %bool %273
        %275 =       OpLogicalAnd %bool %272 %274
                     OpBranch %267

        %267 = OpLabel
        %276 =   OpPhi %bool %false %258 %275 %266
                 OpStore %_0_ok %276
        %278 =   OpMatrixTimesMatrix %mat2v2float %71 %38
                 OpStore %_7_m22 %278
                 OpSelectionMerge %280 None
                 OpBranchConditional %276 %279 %280

        %279 =     OpLabel
        %281 =       OpCompositeExtract %v2float %278 0
        %282 =       OpFOrdEqual %v2bool %281 %154
        %283 =       OpAll %bool %282
        %284 =       OpCompositeExtract %v2float %278 1
        %285 =       OpFOrdEqual %v2bool %284 %155
        %286 =       OpAll %bool %285
        %287 =       OpLogicalAnd %bool %283 %286
                     OpBranch %280

        %280 = OpLabel
        %288 =   OpPhi %bool %false %267 %287 %279
                 OpStore %_0_ok %288
        %289 =   OpFAdd %v3float %36 %189
        %290 =   OpFAdd %v3float %37 %189
        %291 =   OpCompositeConstruct %mat2v3float %289 %290
                 OpStore %_1_m23 %291
                 OpSelectionMerge %293 None
                 OpBranchConditional %288 %292 %293

        %292 =     OpLabel
        %294 =       OpFOrdEqual %v3bool %289 %196
        %295 =       OpAll %bool %294
        %296 =       OpFOrdEqual %v3bool %290 %197
        %297 =       OpAll %bool %296
        %298 =       OpLogicalAnd %bool %295 %297
                     OpBranch %293

        %293 = OpLabel
        %299 =   OpPhi %bool %false %280 %298 %292
                 OpStore %_0_ok %299
        %300 =   OpFSub %v2float %69 %205
        %301 =   OpFSub %v2float %70 %205
        %302 =   OpFSub %v2float %21 %205
        %303 =   OpCompositeConstruct %mat3v2float %300 %301 %302
                 OpStore %_3_m32 %303
                 OpSelectionMerge %305 None
                 OpBranchConditional %299 %304 %305

        %304 =     OpLabel
        %306 =       OpFOrdEqual %v2bool %300 %214
        %307 =       OpAll %bool %306
        %308 =       OpFOrdEqual %v2bool %301 %215
        %309 =       OpAll %bool %308
        %310 =       OpLogicalAnd %bool %307 %309
        %311 =       OpFOrdEqual %v2bool %302 %216
        %312 =       OpAll %bool %311
        %313 =       OpLogicalAnd %bool %310 %312
                     OpBranch %305

        %305 = OpLabel
        %314 =   OpPhi %bool %false %293 %313 %304
                 OpStore %_0_ok %314
        %315 =   OpMatrixTimesScalar %mat2v4float %55 %float_0_25
                 OpStore %_2_m24 %315
                 OpSelectionMerge %317 None
                 OpBranchConditional %314 %316 %317

        %316 =     OpLabel
        %318 =       OpCompositeExtract %v4float %315 0
        %319 =       OpFOrdEqual %v4bool %318 %232
        %320 =       OpAll %bool %319
        %321 =       OpCompositeExtract %v4float %315 1
        %322 =       OpFOrdEqual %v4bool %321 %233
        %323 =       OpAll %bool %322
        %324 =       OpLogicalAnd %bool %320 %323
                     OpBranch %317

        %317 = OpLabel
        %325 =   OpPhi %bool %false %305 %324 %316
                 OpStore %_0_ok %325
                 OpSelectionMerge %327 None
                 OpBranchConditional %325 %326 %327

        %326 =     OpLabel
        %328 =       OpFunctionCall %bool %test_half_b
                     OpBranch %327

        %327 = OpLabel
        %329 =   OpPhi %bool %false %317 %328 %326
                 OpSelectionMerge %334 None
                 OpBranchConditional %329 %332 %333

        %332 =     OpLabel
        %335 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %338 =       OpLoad %v4float %335           ; RelaxedPrecision
                     OpStore %330 %338
                     OpBranch %334

        %333 =     OpLabel
        %339 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %341 =       OpLoad %v4float %339           ; RelaxedPrecision
                     OpStore %330 %341
                     OpBranch %334

        %334 = OpLabel
        %342 =   OpLoad %v4float %330               ; RelaxedPrecision
                 OpReturnValue %342
               OpFunctionEnd
