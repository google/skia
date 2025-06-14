               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %8
               OpName %_UniformBuffer "_UniformBuffer"  ; id %13
               OpMemberName %_UniformBuffer 0 "colorRed"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %15
               OpName %test_int_b "test_int_b"          ; id %6
               OpName %ok "ok"                          ; id %28
               OpName %inputRed "inputRed"              ; id %31
               OpName %inputGreen "inputGreen"          ; id %47
               OpName %x "x"                            ; id %60
               OpName %main "main"                      ; id %7
               OpName %_0_ok "_0_ok"                    ; id %210
               OpName %_1_inputRed "_1_inputRed"        ; id %211
               OpName %_2_inputGreen "_2_inputGreen"    ; id %215
               OpName %_3_x "_3_x"                      ; id %218

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %_1_inputRed RelaxedPrecision
               OpDecorate %214 RelaxedPrecision
               OpDecorate %_2_inputGreen RelaxedPrecision
               OpDecorate %217 RelaxedPrecision
               OpDecorate %_3_x RelaxedPrecision
               OpDecorate %221 RelaxedPrecision
               OpDecorate %229 RelaxedPrecision
               OpDecorate %230 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision
               OpDecorate %241 RelaxedPrecision
               OpDecorate %249 RelaxedPrecision
               OpDecorate %252 RelaxedPrecision
               OpDecorate %253 RelaxedPrecision
               OpDecorate %254 RelaxedPrecision
               OpDecorate %261 RelaxedPrecision
               OpDecorate %262 RelaxedPrecision
               OpDecorate %263 RelaxedPrecision
               OpDecorate %264 RelaxedPrecision
               OpDecorate %274 RelaxedPrecision
               OpDecorate %275 RelaxedPrecision
               OpDecorate %282 RelaxedPrecision
               OpDecorate %289 RelaxedPrecision
               OpDecorate %291 RelaxedPrecision
               OpDecorate %298 RelaxedPrecision
               OpDecorate %299 RelaxedPrecision
               OpDecorate %300 RelaxedPrecision
               OpDecorate %308 RelaxedPrecision
               OpDecorate %309 RelaxedPrecision
               OpDecorate %310 RelaxedPrecision
               OpDecorate %311 RelaxedPrecision
               OpDecorate %319 RelaxedPrecision
               OpDecorate %321 RelaxedPrecision
               OpDecorate %322 RelaxedPrecision
               OpDecorate %323 RelaxedPrecision
               OpDecorate %332 RelaxedPrecision
               OpDecorate %333 RelaxedPrecision
               OpDecorate %340 RelaxedPrecision
               OpDecorate %341 RelaxedPrecision
               OpDecorate %343 RelaxedPrecision
               OpDecorate %345 RelaxedPrecision
               OpDecorate %351 RelaxedPrecision
               OpDecorate %352 RelaxedPrecision
               OpDecorate %353 RelaxedPrecision
               OpDecorate %354 RelaxedPrecision
               OpDecorate %369 RelaxedPrecision
               OpDecorate %371 RelaxedPrecision
               OpDecorate %372 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %float     ; Block
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
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
         %62 = OpConstantComposite %v4int %int_2 %int_2 %int_2 %int_2
      %false = OpConstantFalse %bool
      %int_3 = OpConstant %int 3
         %68 = OpConstantComposite %v4int %int_3 %int_2 %int_2 %int_3
     %v4bool = OpTypeVector %bool 4
     %int_n1 = OpConstant %int -1
     %int_n2 = OpConstant %int -2
         %79 = OpConstantComposite %v4int %int_n1 %int_n1 %int_n2 %int_n2
         %88 = OpConstantComposite %v4int %int_2 %int_1 %int_1 %int_2
      %v3int = OpTypeVector %int 3
      %int_9 = OpConstant %int 9
         %95 = OpConstantComposite %v3int %int_9 %int_9 %int_9
        %101 = OpConstantComposite %v4int %int_9 %int_9 %int_9 %int_2
      %v2int = OpTypeVector %int 2
      %int_4 = OpConstant %int 4
        %108 = OpConstantComposite %v2int %int_4 %int_4
        %114 = OpConstantComposite %v4int %int_2 %int_0 %int_9 %int_2
      %int_5 = OpConstant %int 5
        %119 = OpConstantComposite %v4int %int_5 %int_5 %int_5 %int_5
        %124 = OpConstantComposite %v4int %int_0 %int_5 %int_5 %int_0
     %int_10 = OpConstant %int 10
        %136 = OpConstantComposite %v4int %int_10 %int_10 %int_10 %int_10
        %140 = OpConstantComposite %v4int %int_9 %int_9 %int_10 %int_10
        %149 = OpConstantComposite %v4int %int_1 %int_2 %int_1 %int_2
      %int_8 = OpConstant %int 8
        %155 = OpConstantComposite %v3int %int_8 %int_8 %int_8
        %161 = OpConstantComposite %v4int %int_8 %int_8 %int_8 %int_2
     %int_36 = OpConstant %int 36
        %167 = OpConstantComposite %v2int %int_36 %int_36
     %int_18 = OpConstant %int 18
        %174 = OpConstantComposite %v4int %int_4 %int_18 %int_8 %int_2
     %int_37 = OpConstant %int 37
        %179 = OpConstantComposite %v4int %int_37 %int_37 %int_37 %int_37
        %184 = OpConstantComposite %v4int %int_2 %int_9 %int_18 %int_4
        %190 = OpConstantComposite %v4int %int_4 %int_4 %int_4 %int_4
        %207 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_2 = OpConstant %float 2
        %220 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
    %float_3 = OpConstant %float 3
        %225 = OpConstantComposite %v4float %float_3 %float_2 %float_2 %float_3
   %float_n1 = OpConstant %float -1
   %float_n2 = OpConstant %float -2
        %235 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n2 %float_n2
    %float_1 = OpConstant %float 1
        %245 = OpConstantComposite %v4float %float_2 %float_1 %float_1 %float_2
    %v3float = OpTypeVector %float 3
    %float_9 = OpConstant %float 9
        %257 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_2
   %float_18 = OpConstant %float 18
    %float_4 = OpConstant %float 4
        %269 = OpConstantComposite %v4float %float_18 %float_4 %float_9 %float_2
    %float_5 = OpConstant %float 5
        %278 = OpConstantComposite %v4float %float_0 %float_5 %float_5 %float_0
   %float_10 = OpConstant %float 10
        %290 = OpConstantComposite %v4float %float_10 %float_10 %float_10 %float_10
        %294 = OpConstantComposite %v4float %float_9 %float_9 %float_10 %float_10
        %303 = OpConstantComposite %v4float %float_1 %float_2 %float_1 %float_2
    %float_8 = OpConstant %float 8
        %314 = OpConstantComposite %v4float %float_8 %float_8 %float_8 %float_2
   %float_32 = OpConstant %float 32
        %320 = OpConstantComposite %v2float %float_32 %float_32
   %float_16 = OpConstant %float 16
        %327 = OpConstantComposite %v4float %float_4 %float_16 %float_8 %float_2
        %331 = OpConstantComposite %v4float %float_32 %float_32 %float_32 %float_32
        %336 = OpConstantComposite %v4float %float_2 %float_8 %float_16 %float_4
        %342 = OpConstantComposite %v4float %float_4 %float_4 %float_4 %float_4
  %float_0_5 = OpConstant %float 0.5


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %17

         %18 = OpLabel
         %22 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %22 %21
         %24 =   OpFunctionCall %v4float %main %22
                 OpStore %sk_FragColor %24
                 OpReturn
               OpFunctionEnd


               ; Function test_int_b
 %test_int_b = OpFunction %bool None %26

         %27 = OpLabel
         %ok =   OpVariable %_ptr_Function_bool Function
   %inputRed =   OpVariable %_ptr_Function_v4int Function
 %inputGreen =   OpVariable %_ptr_Function_v4int Function
          %x =   OpVariable %_ptr_Function_v4int Function
                 OpStore %ok %true
         %34 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %37 =   OpLoad %v4float %34                ; RelaxedPrecision
         %38 =   OpCompositeExtract %float %37 0    ; RelaxedPrecision
         %39 =   OpConvertFToS %int %38
         %40 =   OpCompositeExtract %float %37 1    ; RelaxedPrecision
         %41 =   OpConvertFToS %int %40
         %42 =   OpCompositeExtract %float %37 2    ; RelaxedPrecision
         %43 =   OpConvertFToS %int %42
         %44 =   OpCompositeExtract %float %37 3    ; RelaxedPrecision
         %45 =   OpConvertFToS %int %44
         %46 =   OpCompositeConstruct %v4int %39 %41 %43 %45
                 OpStore %inputRed %46
         %48 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_1
         %50 =   OpLoad %v4float %48                ; RelaxedPrecision
         %51 =   OpCompositeExtract %float %50 0    ; RelaxedPrecision
         %52 =   OpConvertFToS %int %51
         %53 =   OpCompositeExtract %float %50 1    ; RelaxedPrecision
         %54 =   OpConvertFToS %int %53
         %55 =   OpCompositeExtract %float %50 2    ; RelaxedPrecision
         %56 =   OpConvertFToS %int %55
         %57 =   OpCompositeExtract %float %50 3    ; RelaxedPrecision
         %58 =   OpConvertFToS %int %57
         %59 =   OpCompositeConstruct %v4int %52 %54 %56 %58
                 OpStore %inputGreen %59
         %63 =   OpIAdd %v4int %46 %62
                 OpStore %x %63
                 OpSelectionMerge %66 None
                 OpBranchConditional %true %65 %66

         %65 =     OpLabel
         %69 =       OpIEqual %v4bool %63 %68
         %71 =       OpAll %bool %69
                     OpBranch %66

         %66 = OpLabel
         %72 =   OpPhi %bool %false %27 %71 %65
                 OpStore %ok %72
         %73 =   OpVectorShuffle %v4int %59 %59 1 3 0 2
         %74 =   OpISub %v4int %73 %62
                 OpStore %x %74
                 OpSelectionMerge %76 None
                 OpBranchConditional %72 %75 %76

         %75 =     OpLabel
         %80 =       OpIEqual %v4bool %74 %79
         %81 =       OpAll %bool %80
                     OpBranch %76

         %76 = OpLabel
         %82 =   OpPhi %bool %false %66 %81 %75
                 OpStore %ok %82
         %83 =   OpCompositeExtract %int %59 1
         %84 =   OpCompositeConstruct %v4int %83 %83 %83 %83
         %85 =   OpIAdd %v4int %46 %84
                 OpStore %x %85
                 OpSelectionMerge %87 None
                 OpBranchConditional %82 %86 %87

         %86 =     OpLabel
         %89 =       OpIEqual %v4bool %85 %88
         %90 =       OpAll %bool %89
                     OpBranch %87

         %87 = OpLabel
         %91 =   OpPhi %bool %false %76 %90 %86
                 OpStore %ok %91
         %92 =   OpVectorShuffle %v3int %59 %59 3 1 3
         %96 =   OpIMul %v3int %92 %95
         %97 =   OpLoad %v4int %x
         %98 =   OpVectorShuffle %v4int %97 %96 4 5 6 3
                 OpStore %x %98
                 OpSelectionMerge %100 None
                 OpBranchConditional %91 %99 %100

         %99 =     OpLabel
        %102 =       OpIEqual %v4bool %98 %101
        %103 =       OpAll %bool %102
                     OpBranch %100

        %100 = OpLabel
        %104 =   OpPhi %bool %false %87 %103 %99
                 OpStore %ok %104
        %105 =   OpVectorShuffle %v2int %98 %98 2 3
        %109 =   OpSDiv %v2int %105 %108
        %110 =   OpLoad %v4int %x
        %111 =   OpVectorShuffle %v4int %110 %109 4 5 2 3
                 OpStore %x %111
                 OpSelectionMerge %113 None
                 OpBranchConditional %104 %112 %113

        %112 =     OpLabel
        %115 =       OpIEqual %v4bool %111 %114
        %116 =       OpAll %bool %115
                     OpBranch %113

        %113 = OpLabel
        %117 =   OpPhi %bool %false %100 %116 %112
                 OpStore %ok %117
        %120 =   OpIMul %v4int %46 %119
        %121 =   OpVectorShuffle %v4int %120 %120 1 0 3 2
                 OpStore %x %121
                 OpSelectionMerge %123 None
                 OpBranchConditional %117 %122 %123

        %122 =     OpLabel
        %125 =       OpIEqual %v4bool %121 %124
        %126 =       OpAll %bool %125
                     OpBranch %123

        %123 = OpLabel
        %127 =   OpPhi %bool %false %113 %126 %122
                 OpStore %ok %127
        %128 =   OpIAdd %v4int %62 %46
                 OpStore %x %128
                 OpSelectionMerge %130 None
                 OpBranchConditional %127 %129 %130

        %129 =     OpLabel
        %131 =       OpIEqual %v4bool %128 %68
        %132 =       OpAll %bool %131
                     OpBranch %130

        %130 = OpLabel
        %133 =   OpPhi %bool %false %123 %132 %129
                 OpStore %ok %133
        %135 =   OpVectorShuffle %v4int %59 %59 1 3 0 2
        %137 =   OpISub %v4int %136 %135
                 OpStore %x %137
                 OpSelectionMerge %139 None
                 OpBranchConditional %133 %138 %139

        %138 =     OpLabel
        %141 =       OpIEqual %v4bool %137 %140
        %142 =       OpAll %bool %141
                     OpBranch %139

        %139 = OpLabel
        %143 =   OpPhi %bool %false %130 %142 %138
                 OpStore %ok %143
        %144 =   OpCompositeExtract %int %46 0
        %145 =   OpCompositeConstruct %v4int %144 %144 %144 %144
        %146 =   OpIAdd %v4int %145 %59
                 OpStore %x %146
                 OpSelectionMerge %148 None
                 OpBranchConditional %143 %147 %148

        %147 =     OpLabel
        %150 =       OpIEqual %v4bool %146 %149
        %151 =       OpAll %bool %150
                     OpBranch %148

        %148 = OpLabel
        %152 =   OpPhi %bool %false %139 %151 %147
                 OpStore %ok %152
        %154 =   OpVectorShuffle %v3int %59 %59 3 1 3
        %156 =   OpIMul %v3int %155 %154
        %157 =   OpLoad %v4int %x
        %158 =   OpVectorShuffle %v4int %157 %156 4 5 6 3
                 OpStore %x %158
                 OpSelectionMerge %160 None
                 OpBranchConditional %152 %159 %160

        %159 =     OpLabel
        %162 =       OpIEqual %v4bool %158 %161
        %163 =       OpAll %bool %162
                     OpBranch %160

        %160 = OpLabel
        %164 =   OpPhi %bool %false %148 %163 %159
                 OpStore %ok %164
        %166 =   OpVectorShuffle %v2int %158 %158 2 3
        %168 =   OpSDiv %v2int %167 %166
        %169 =   OpLoad %v4int %x
        %170 =   OpVectorShuffle %v4int %169 %168 4 5 2 3
                 OpStore %x %170
                 OpSelectionMerge %172 None
                 OpBranchConditional %164 %171 %172

        %171 =     OpLabel
        %175 =       OpIEqual %v4bool %170 %174
        %176 =       OpAll %bool %175
                     OpBranch %172

        %172 = OpLabel
        %177 =   OpPhi %bool %false %160 %176 %171
                 OpStore %ok %177
        %180 =   OpSDiv %v4int %179 %170
        %181 =   OpVectorShuffle %v4int %180 %180 1 0 3 2
                 OpStore %x %181
                 OpSelectionMerge %183 None
                 OpBranchConditional %177 %182 %183

        %182 =     OpLabel
        %185 =       OpIEqual %v4bool %181 %184
        %186 =       OpAll %bool %185
                     OpBranch %183

        %183 = OpLabel
        %187 =   OpPhi %bool %false %172 %186 %182
                 OpStore %ok %187
        %188 =   OpIAdd %v4int %181 %62
                 OpStore %x %188
        %189 =   OpIMul %v4int %188 %62
                 OpStore %x %189
        %191 =   OpISub %v4int %189 %190
                 OpStore %x %191
        %192 =   OpSDiv %v4int %191 %62
                 OpStore %x %192
                 OpSelectionMerge %194 None
                 OpBranchConditional %187 %193 %194

        %193 =     OpLabel
        %195 =       OpIEqual %v4bool %192 %184
        %196 =       OpAll %bool %195
                     OpBranch %194

        %194 = OpLabel
        %197 =   OpPhi %bool %false %183 %196 %193
                 OpStore %ok %197
        %198 =   OpIAdd %v4int %192 %62
                 OpStore %x %198
        %199 =   OpIMul %v4int %198 %62
                 OpStore %x %199
        %200 =   OpISub %v4int %199 %190
                 OpStore %x %200
        %201 =   OpSDiv %v4int %200 %62
                 OpStore %x %201
                 OpSelectionMerge %203 None
                 OpBranchConditional %197 %202 %203

        %202 =     OpLabel
        %204 =       OpIEqual %v4bool %201 %184
        %205 =       OpAll %bool %204
                     OpBranch %203

        %203 = OpLabel
        %206 =   OpPhi %bool %false %194 %205 %202
                 OpStore %ok %206
                 OpReturnValue %206
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %207        ; RelaxedPrecision
        %208 = OpFunctionParameter %_ptr_Function_v2float

        %209 = OpLabel
      %_0_ok =   OpVariable %_ptr_Function_bool Function
%_1_inputRed =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
%_2_inputGreen =   OpVariable %_ptr_Function_v4float Function   ; RelaxedPrecision
       %_3_x =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
        %364 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %_0_ok %true
        %213 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %214 =   OpLoad %v4float %213               ; RelaxedPrecision
                 OpStore %_1_inputRed %214
        %216 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %217 =   OpLoad %v4float %216               ; RelaxedPrecision
                 OpStore %_2_inputGreen %217
        %221 =   OpFAdd %v4float %214 %220          ; RelaxedPrecision
                 OpStore %_3_x %221
                 OpSelectionMerge %223 None
                 OpBranchConditional %true %222 %223

        %222 =     OpLabel
        %226 =       OpFOrdEqual %v4bool %221 %225
        %227 =       OpAll %bool %226
                     OpBranch %223

        %223 = OpLabel
        %228 =   OpPhi %bool %false %209 %227 %222
                 OpStore %_0_ok %228
        %229 =   OpVectorShuffle %v4float %217 %217 1 3 0 2     ; RelaxedPrecision
        %230 =   OpFSub %v4float %229 %220                      ; RelaxedPrecision
                 OpStore %_3_x %230
                 OpSelectionMerge %232 None
                 OpBranchConditional %228 %231 %232

        %231 =     OpLabel
        %236 =       OpFOrdEqual %v4bool %230 %235
        %237 =       OpAll %bool %236
                     OpBranch %232

        %232 = OpLabel
        %238 =   OpPhi %bool %false %223 %237 %231
                 OpStore %_0_ok %238
        %239 =   OpCompositeExtract %float %217 1   ; RelaxedPrecision
        %240 =   OpCompositeConstruct %v4float %239 %239 %239 %239  ; RelaxedPrecision
        %241 =   OpFAdd %v4float %214 %240                          ; RelaxedPrecision
                 OpStore %_3_x %241
                 OpSelectionMerge %243 None
                 OpBranchConditional %238 %242 %243

        %242 =     OpLabel
        %246 =       OpFOrdEqual %v4bool %241 %245
        %247 =       OpAll %bool %246
                     OpBranch %243

        %243 = OpLabel
        %248 =   OpPhi %bool %false %232 %247 %242
                 OpStore %_0_ok %248
        %249 =   OpVectorShuffle %v3float %217 %217 3 1 3   ; RelaxedPrecision
        %252 =   OpVectorTimesScalar %v3float %249 %float_9     ; RelaxedPrecision
        %253 =   OpLoad %v4float %_3_x                          ; RelaxedPrecision
        %254 =   OpVectorShuffle %v4float %253 %252 4 5 6 3     ; RelaxedPrecision
                 OpStore %_3_x %254
                 OpSelectionMerge %256 None
                 OpBranchConditional %248 %255 %256

        %255 =     OpLabel
        %258 =       OpFOrdEqual %v4bool %254 %257
        %259 =       OpAll %bool %258
                     OpBranch %256

        %256 = OpLabel
        %260 =   OpPhi %bool %false %243 %259 %255
                 OpStore %_0_ok %260
        %261 =   OpVectorShuffle %v2float %254 %254 2 3     ; RelaxedPrecision
        %262 =   OpVectorTimesScalar %v2float %261 %float_2     ; RelaxedPrecision
        %263 =   OpLoad %v4float %_3_x                          ; RelaxedPrecision
        %264 =   OpVectorShuffle %v4float %263 %262 4 5 2 3     ; RelaxedPrecision
                 OpStore %_3_x %264
                 OpSelectionMerge %266 None
                 OpBranchConditional %260 %265 %266

        %265 =     OpLabel
        %270 =       OpFOrdEqual %v4bool %264 %269
        %271 =       OpAll %bool %270
                     OpBranch %266

        %266 = OpLabel
        %272 =   OpPhi %bool %false %256 %271 %265
                 OpStore %_0_ok %272
        %274 =   OpVectorTimesScalar %v4float %214 %float_5     ; RelaxedPrecision
        %275 =   OpVectorShuffle %v4float %274 %274 1 0 3 2     ; RelaxedPrecision
                 OpStore %_3_x %275
                 OpSelectionMerge %277 None
                 OpBranchConditional %272 %276 %277

        %276 =     OpLabel
        %279 =       OpFOrdEqual %v4bool %275 %278
        %280 =       OpAll %bool %279
                     OpBranch %277

        %277 = OpLabel
        %281 =   OpPhi %bool %false %266 %280 %276
                 OpStore %_0_ok %281
        %282 =   OpFAdd %v4float %220 %214          ; RelaxedPrecision
                 OpStore %_3_x %282
                 OpSelectionMerge %284 None
                 OpBranchConditional %281 %283 %284

        %283 =     OpLabel
        %285 =       OpFOrdEqual %v4bool %282 %225
        %286 =       OpAll %bool %285
                     OpBranch %284

        %284 = OpLabel
        %287 =   OpPhi %bool %false %277 %286 %283
                 OpStore %_0_ok %287
        %289 =   OpVectorShuffle %v4float %217 %217 1 3 0 2     ; RelaxedPrecision
        %291 =   OpFSub %v4float %290 %289                      ; RelaxedPrecision
                 OpStore %_3_x %291
                 OpSelectionMerge %293 None
                 OpBranchConditional %287 %292 %293

        %292 =     OpLabel
        %295 =       OpFOrdEqual %v4bool %291 %294
        %296 =       OpAll %bool %295
                     OpBranch %293

        %293 = OpLabel
        %297 =   OpPhi %bool %false %284 %296 %292
                 OpStore %_0_ok %297
        %298 =   OpCompositeExtract %float %214 0   ; RelaxedPrecision
        %299 =   OpCompositeConstruct %v4float %298 %298 %298 %298  ; RelaxedPrecision
        %300 =   OpFAdd %v4float %299 %217                          ; RelaxedPrecision
                 OpStore %_3_x %300
                 OpSelectionMerge %302 None
                 OpBranchConditional %297 %301 %302

        %301 =     OpLabel
        %304 =       OpFOrdEqual %v4bool %300 %303
        %305 =       OpAll %bool %304
                     OpBranch %302

        %302 = OpLabel
        %306 =   OpPhi %bool %false %293 %305 %301
                 OpStore %_0_ok %306
        %308 =   OpVectorShuffle %v3float %217 %217 3 1 3   ; RelaxedPrecision
        %309 =   OpVectorTimesScalar %v3float %308 %float_8     ; RelaxedPrecision
        %310 =   OpLoad %v4float %_3_x                          ; RelaxedPrecision
        %311 =   OpVectorShuffle %v4float %310 %309 4 5 6 3     ; RelaxedPrecision
                 OpStore %_3_x %311
                 OpSelectionMerge %313 None
                 OpBranchConditional %306 %312 %313

        %312 =     OpLabel
        %315 =       OpFOrdEqual %v4bool %311 %314
        %316 =       OpAll %bool %315
                     OpBranch %313

        %313 = OpLabel
        %317 =   OpPhi %bool %false %302 %316 %312
                 OpStore %_0_ok %317
        %319 =   OpVectorShuffle %v2float %311 %311 2 3     ; RelaxedPrecision
        %321 =   OpFDiv %v2float %320 %319                  ; RelaxedPrecision
        %322 =   OpLoad %v4float %_3_x                      ; RelaxedPrecision
        %323 =   OpVectorShuffle %v4float %322 %321 4 5 2 3     ; RelaxedPrecision
                 OpStore %_3_x %323
                 OpSelectionMerge %325 None
                 OpBranchConditional %317 %324 %325

        %324 =     OpLabel
        %328 =       OpFOrdEqual %v4bool %323 %327
        %329 =       OpAll %bool %328
                     OpBranch %325

        %325 = OpLabel
        %330 =   OpPhi %bool %false %313 %329 %324
                 OpStore %_0_ok %330
        %332 =   OpFDiv %v4float %331 %323          ; RelaxedPrecision
        %333 =   OpVectorShuffle %v4float %332 %332 1 0 3 2     ; RelaxedPrecision
                 OpStore %_3_x %333
                 OpSelectionMerge %335 None
                 OpBranchConditional %330 %334 %335

        %334 =     OpLabel
        %337 =       OpFOrdEqual %v4bool %333 %336
        %338 =       OpAll %bool %337
                     OpBranch %335

        %335 = OpLabel
        %339 =   OpPhi %bool %false %325 %338 %334
                 OpStore %_0_ok %339
        %340 =   OpFAdd %v4float %333 %220          ; RelaxedPrecision
                 OpStore %_3_x %340
        %341 =   OpVectorTimesScalar %v4float %340 %float_2     ; RelaxedPrecision
                 OpStore %_3_x %341
        %343 =   OpFSub %v4float %341 %342          ; RelaxedPrecision
                 OpStore %_3_x %343
        %345 =   OpVectorTimesScalar %v4float %343 %float_0_5   ; RelaxedPrecision
                 OpStore %_3_x %345
                 OpSelectionMerge %347 None
                 OpBranchConditional %339 %346 %347

        %346 =     OpLabel
        %348 =       OpFOrdEqual %v4bool %345 %336
        %349 =       OpAll %bool %348
                     OpBranch %347

        %347 = OpLabel
        %350 =   OpPhi %bool %false %335 %349 %346
                 OpStore %_0_ok %350
        %351 =   OpFAdd %v4float %345 %220          ; RelaxedPrecision
                 OpStore %_3_x %351
        %352 =   OpVectorTimesScalar %v4float %351 %float_2     ; RelaxedPrecision
                 OpStore %_3_x %352
        %353 =   OpFSub %v4float %352 %342          ; RelaxedPrecision
                 OpStore %_3_x %353
        %354 =   OpVectorTimesScalar %v4float %353 %float_0_5   ; RelaxedPrecision
                 OpStore %_3_x %354
                 OpSelectionMerge %356 None
                 OpBranchConditional %350 %355 %356

        %355 =     OpLabel
        %357 =       OpFOrdEqual %v4bool %354 %336
        %358 =       OpAll %bool %357
                     OpBranch %356

        %356 = OpLabel
        %359 =   OpPhi %bool %false %347 %358 %355
                 OpStore %_0_ok %359
                 OpSelectionMerge %361 None
                 OpBranchConditional %359 %360 %361

        %360 =     OpLabel
        %362 =       OpFunctionCall %bool %test_int_b
                     OpBranch %361

        %361 = OpLabel
        %363 =   OpPhi %bool %false %356 %362 %360
                 OpSelectionMerge %367 None
                 OpBranchConditional %363 %365 %366

        %365 =     OpLabel
        %368 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %369 =       OpLoad %v4float %368           ; RelaxedPrecision
                     OpStore %364 %369
                     OpBranch %367

        %366 =     OpLabel
        %370 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %371 =       OpLoad %v4float %370           ; RelaxedPrecision
                     OpStore %364 %371
                     OpBranch %367

        %367 = OpLabel
        %372 =   OpLoad %v4float %364               ; RelaxedPrecision
                 OpReturnValue %372
               OpFunctionEnd
