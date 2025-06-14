               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "colorBlack"
               OpMemberName %_UniformBuffer 3 "colorWhite"
               OpMemberName %_UniformBuffer 4 "testInputs"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %intGreen "intGreen"              ; id %27
               OpName %intRed "intRed"                  ; id %45

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
               OpMemberDecorate %_UniformBuffer 3 Offset 48
               OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 4 Offset 64
               OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %33 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %194 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %198 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %203 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
               OpDecorate %209 RelaxedPrecision
               OpDecorate %211 RelaxedPrecision
               OpDecorate %212 RelaxedPrecision
               OpDecorate %214 RelaxedPrecision
               OpDecorate %215 RelaxedPrecision
               OpDecorate %217 RelaxedPrecision
               OpDecorate %218 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %221 RelaxedPrecision
               OpDecorate %223 RelaxedPrecision
               OpDecorate %224 RelaxedPrecision
               OpDecorate %230 RelaxedPrecision
               OpDecorate %232 RelaxedPrecision
               OpDecorate %233 RelaxedPrecision
               OpDecorate %236 RelaxedPrecision
               OpDecorate %237 RelaxedPrecision
               OpDecorate %239 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision
               OpDecorate %242 RelaxedPrecision
               OpDecorate %243 RelaxedPrecision
               OpDecorate %245 RelaxedPrecision
               OpDecorate %246 RelaxedPrecision
               OpDecorate %252 RelaxedPrecision
               OpDecorate %254 RelaxedPrecision
               OpDecorate %256 RelaxedPrecision
               OpDecorate %258 RelaxedPrecision
               OpDecorate %260 RelaxedPrecision
               OpDecorate %262 RelaxedPrecision
               OpDecorate %268 RelaxedPrecision
               OpDecorate %270 RelaxedPrecision
               OpDecorate %271 RelaxedPrecision
               OpDecorate %273 RelaxedPrecision
               OpDecorate %274 RelaxedPrecision
               OpDecorate %276 RelaxedPrecision
               OpDecorate %277 RelaxedPrecision
               OpDecorate %279 RelaxedPrecision
               OpDecorate %280 RelaxedPrecision
               OpDecorate %282 RelaxedPrecision
               OpDecorate %283 RelaxedPrecision
               OpDecorate %288 RelaxedPrecision
               OpDecorate %290 RelaxedPrecision
               OpDecorate %291 RelaxedPrecision
               OpDecorate %293 RelaxedPrecision
               OpDecorate %294 RelaxedPrecision
               OpDecorate %296 RelaxedPrecision
               OpDecorate %297 RelaxedPrecision
               OpDecorate %299 RelaxedPrecision
               OpDecorate %300 RelaxedPrecision
               OpDecorate %302 RelaxedPrecision
               OpDecorate %303 RelaxedPrecision
               OpDecorate %309 RelaxedPrecision
               OpDecorate %311 RelaxedPrecision
               OpDecorate %312 RelaxedPrecision
               OpDecorate %314 RelaxedPrecision
               OpDecorate %315 RelaxedPrecision
               OpDecorate %317 RelaxedPrecision
               OpDecorate %318 RelaxedPrecision
               OpDecorate %320 RelaxedPrecision
               OpDecorate %321 RelaxedPrecision
               OpDecorate %323 RelaxedPrecision
               OpDecorate %324 RelaxedPrecision
               OpDecorate %330 RelaxedPrecision
               OpDecorate %332 RelaxedPrecision
               OpDecorate %334 RelaxedPrecision
               OpDecorate %336 RelaxedPrecision
               OpDecorate %338 RelaxedPrecision
               OpDecorate %340 RelaxedPrecision
               OpDecorate %347 RelaxedPrecision
               OpDecorate %348 RelaxedPrecision
               OpDecorate %356 RelaxedPrecision
               OpDecorate %357 RelaxedPrecision
               OpDecorate %365 RelaxedPrecision
               OpDecorate %366 RelaxedPrecision
               OpDecorate %374 RelaxedPrecision
               OpDecorate %381 RelaxedPrecision
               OpDecorate %382 RelaxedPrecision
               OpDecorate %389 RelaxedPrecision
               OpDecorate %390 RelaxedPrecision
               OpDecorate %398 RelaxedPrecision
               OpDecorate %399 RelaxedPrecision
               OpDecorate %407 RelaxedPrecision
               OpDecorate %417 RelaxedPrecision
               OpDecorate %419 RelaxedPrecision
               OpDecorate %420 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float %v4float     ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
  %float_100 = OpConstant %float 100
      %int_1 = OpConstant %int 1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
      %v2int = OpTypeVector %int 2
     %v2bool = OpTypeVector %bool 2
         %72 = OpConstantComposite %v2bool %false %false
      %v3int = OpTypeVector %int 3
     %v3bool = OpTypeVector %bool 3
         %86 = OpConstantComposite %v3bool %false %false %false
     %v4bool = OpTypeVector %bool 4
         %97 = OpConstantComposite %v4bool %false %false %false %false
       %true = OpConstantTrue %bool
        %112 = OpConstantComposite %v2bool %true %true
        %124 = OpConstantComposite %v3bool %true %true %true
        %134 = OpConstantComposite %v4bool %true %true %true %true
    %int_100 = OpConstant %int 100
        %145 = OpConstantComposite %v2int %int_0 %int_100
        %152 = OpConstantComposite %v3int %int_0 %int_100 %int_0
        %159 = OpConstantComposite %v4int %int_0 %int_100 %int_0 %int_100
        %169 = OpConstantComposite %v2int %int_100 %int_0
        %176 = OpConstantComposite %v3int %int_100 %int_0 %int_0
        %183 = OpConstantComposite %v4int %int_100 %int_0 %int_0 %int_100
    %v3float = OpTypeVector %float 3
    %float_1 = OpConstant %float 1
        %354 = OpConstantComposite %v2float %float_0 %float_1
        %363 = OpConstantComposite %v3float %float_0 %float_1 %float_0
        %372 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
        %387 = OpConstantComposite %v2float %float_1 %float_0
        %396 = OpConstantComposite %v3float %float_1 %float_0 %float_0
        %405 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%_ptr_Function_v4float = OpTypePointer Function %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %16

         %17 = OpLabel
         %21 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %21 %20
         %23 =   OpFunctionCall %v4float %main %21
                 OpStore %sk_FragColor %23
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %24         ; RelaxedPrecision
         %25 = OpFunctionParameter %_ptr_Function_v2float

         %26 = OpLabel
   %intGreen =   OpVariable %_ptr_Function_v4int Function
     %intRed =   OpVariable %_ptr_Function_v4int Function
        %411 =   OpVariable %_ptr_Function_v4float Function
         %30 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %33 =   OpLoad %v4float %30                ; RelaxedPrecision
         %35 =   OpVectorTimesScalar %v4float %33 %float_100    ; RelaxedPrecision
         %36 =   OpCompositeExtract %float %35 0                ; RelaxedPrecision
         %37 =   OpConvertFToS %int %36
         %38 =   OpCompositeExtract %float %35 1    ; RelaxedPrecision
         %39 =   OpConvertFToS %int %38
         %40 =   OpCompositeExtract %float %35 2    ; RelaxedPrecision
         %41 =   OpConvertFToS %int %40
         %42 =   OpCompositeExtract %float %35 3    ; RelaxedPrecision
         %43 =   OpConvertFToS %int %42
         %44 =   OpCompositeConstruct %v4int %37 %39 %41 %43
                 OpStore %intGreen %44
         %46 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %48 =   OpLoad %v4float %46                ; RelaxedPrecision
         %49 =   OpVectorTimesScalar %v4float %48 %float_100    ; RelaxedPrecision
         %50 =   OpCompositeExtract %float %49 0                ; RelaxedPrecision
         %51 =   OpConvertFToS %int %50
         %52 =   OpCompositeExtract %float %49 1    ; RelaxedPrecision
         %53 =   OpConvertFToS %int %52
         %54 =   OpCompositeExtract %float %49 2    ; RelaxedPrecision
         %55 =   OpConvertFToS %int %54
         %56 =   OpCompositeExtract %float %49 3    ; RelaxedPrecision
         %57 =   OpConvertFToS %int %56
         %58 =   OpCompositeConstruct %v4int %51 %53 %55 %57
                 OpStore %intRed %58
         %62 =   OpCompositeExtract %int %44 0
         %63 =   OpCompositeExtract %int %58 0
         %61 =   OpSelect %int %false %63 %62
         %64 =   OpIEqual %bool %61 %62
                 OpSelectionMerge %66 None
                 OpBranchConditional %64 %65 %66

         %65 =     OpLabel
         %68 =       OpVectorShuffle %v2int %44 %44 0 1
         %70 =       OpVectorShuffle %v2int %58 %58 0 1
         %73 =       OpVectorShuffle %v2int %44 %44 0 1
         %74 =       OpVectorShuffle %v2int %58 %58 0 1
         %67 =       OpSelect %v2int %72 %74 %73
         %75 =       OpVectorShuffle %v2int %44 %44 0 1
         %76 =       OpIEqual %v2bool %67 %75
         %77 =       OpAll %bool %76
                     OpBranch %66

         %66 = OpLabel
         %78 =   OpPhi %bool %false %26 %77 %65
                 OpSelectionMerge %80 None
                 OpBranchConditional %78 %79 %80

         %79 =     OpLabel
         %82 =       OpVectorShuffle %v3int %44 %44 0 1 2
         %84 =       OpVectorShuffle %v3int %58 %58 0 1 2
         %87 =       OpVectorShuffle %v3int %44 %44 0 1 2
         %88 =       OpVectorShuffle %v3int %58 %58 0 1 2
         %81 =       OpSelect %v3int %86 %88 %87
         %89 =       OpVectorShuffle %v3int %44 %44 0 1 2
         %90 =       OpIEqual %v3bool %81 %89
         %91 =       OpAll %bool %90
                     OpBranch %80

         %80 = OpLabel
         %92 =   OpPhi %bool %false %66 %91 %79
                 OpSelectionMerge %94 None
                 OpBranchConditional %92 %93 %94

         %93 =     OpLabel
         %95 =       OpSelect %v4int %97 %58 %44
         %98 =       OpIEqual %v4bool %95 %44
         %99 =       OpAll %bool %98
                     OpBranch %94

         %94 = OpLabel
        %100 =   OpPhi %bool %false %80 %99 %93
                 OpSelectionMerge %102 None
                 OpBranchConditional %100 %101 %102

        %101 =     OpLabel
        %103 =       OpSelect %int %true %63 %62
        %105 =       OpIEqual %bool %103 %63
                     OpBranch %102

        %102 = OpLabel
        %106 =   OpPhi %bool %false %94 %105 %101
                 OpSelectionMerge %108 None
                 OpBranchConditional %106 %107 %108

        %107 =     OpLabel
        %110 =       OpVectorShuffle %v2int %44 %44 0 1
        %111 =       OpVectorShuffle %v2int %58 %58 0 1
        %113 =       OpVectorShuffle %v2int %44 %44 0 1
        %114 =       OpVectorShuffle %v2int %58 %58 0 1
        %109 =       OpSelect %v2int %112 %114 %113
        %115 =       OpVectorShuffle %v2int %58 %58 0 1
        %116 =       OpIEqual %v2bool %109 %115
        %117 =       OpAll %bool %116
                     OpBranch %108

        %108 = OpLabel
        %118 =   OpPhi %bool %false %102 %117 %107
                 OpSelectionMerge %120 None
                 OpBranchConditional %118 %119 %120

        %119 =     OpLabel
        %122 =       OpVectorShuffle %v3int %44 %44 0 1 2
        %123 =       OpVectorShuffle %v3int %58 %58 0 1 2
        %125 =       OpVectorShuffle %v3int %44 %44 0 1 2
        %126 =       OpVectorShuffle %v3int %58 %58 0 1 2
        %121 =       OpSelect %v3int %124 %126 %125
        %127 =       OpVectorShuffle %v3int %58 %58 0 1 2
        %128 =       OpIEqual %v3bool %121 %127
        %129 =       OpAll %bool %128
                     OpBranch %120

        %120 = OpLabel
        %130 =   OpPhi %bool %false %108 %129 %119
                 OpSelectionMerge %132 None
                 OpBranchConditional %130 %131 %132

        %131 =     OpLabel
        %133 =       OpSelect %v4int %134 %58 %44
        %135 =       OpIEqual %v4bool %133 %58
        %136 =       OpAll %bool %135
                     OpBranch %132

        %132 = OpLabel
        %137 =   OpPhi %bool %false %120 %136 %131
                 OpSelectionMerge %139 None
                 OpBranchConditional %137 %138 %139

        %138 =     OpLabel
        %140 =       OpIEqual %bool %int_0 %62
                     OpBranch %139

        %139 = OpLabel
        %141 =   OpPhi %bool %false %132 %140 %138
                 OpSelectionMerge %143 None
                 OpBranchConditional %141 %142 %143

        %142 =     OpLabel
        %146 =       OpVectorShuffle %v2int %44 %44 0 1
        %147 =       OpIEqual %v2bool %145 %146
        %148 =       OpAll %bool %147
                     OpBranch %143

        %143 = OpLabel
        %149 =   OpPhi %bool %false %139 %148 %142
                 OpSelectionMerge %151 None
                 OpBranchConditional %149 %150 %151

        %150 =     OpLabel
        %153 =       OpVectorShuffle %v3int %44 %44 0 1 2
        %154 =       OpIEqual %v3bool %152 %153
        %155 =       OpAll %bool %154
                     OpBranch %151

        %151 = OpLabel
        %156 =   OpPhi %bool %false %143 %155 %150
                 OpSelectionMerge %158 None
                 OpBranchConditional %156 %157 %158

        %157 =     OpLabel
        %160 =       OpIEqual %v4bool %159 %44
        %161 =       OpAll %bool %160
                     OpBranch %158

        %158 = OpLabel
        %162 =   OpPhi %bool %false %151 %161 %157
                 OpSelectionMerge %164 None
                 OpBranchConditional %162 %163 %164

        %163 =     OpLabel
        %165 =       OpIEqual %bool %int_100 %63
                     OpBranch %164

        %164 = OpLabel
        %166 =   OpPhi %bool %false %158 %165 %163
                 OpSelectionMerge %168 None
                 OpBranchConditional %166 %167 %168

        %167 =     OpLabel
        %170 =       OpVectorShuffle %v2int %58 %58 0 1
        %171 =       OpIEqual %v2bool %169 %170
        %172 =       OpAll %bool %171
                     OpBranch %168

        %168 = OpLabel
        %173 =   OpPhi %bool %false %164 %172 %167
                 OpSelectionMerge %175 None
                 OpBranchConditional %173 %174 %175

        %174 =     OpLabel
        %177 =       OpVectorShuffle %v3int %58 %58 0 1 2
        %178 =       OpIEqual %v3bool %176 %177
        %179 =       OpAll %bool %178
                     OpBranch %175

        %175 = OpLabel
        %180 =   OpPhi %bool %false %168 %179 %174
                 OpSelectionMerge %182 None
                 OpBranchConditional %180 %181 %182

        %181 =     OpLabel
        %184 =       OpIEqual %v4bool %183 %58
        %185 =       OpAll %bool %184
                     OpBranch %182

        %182 = OpLabel
        %186 =   OpPhi %bool %false %175 %185 %181
                 OpSelectionMerge %188 None
                 OpBranchConditional %186 %187 %188

        %187 =     OpLabel
        %190 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %191 =       OpLoad %v4float %190           ; RelaxedPrecision
        %192 =       OpCompositeExtract %float %191 0   ; RelaxedPrecision
        %193 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %194 =       OpLoad %v4float %193           ; RelaxedPrecision
        %195 =       OpCompositeExtract %float %194 0   ; RelaxedPrecision
        %196 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %197 =       OpLoad %v4float %196           ; RelaxedPrecision
        %198 =       OpCompositeExtract %float %197 0   ; RelaxedPrecision
        %199 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %200 =       OpLoad %v4float %199           ; RelaxedPrecision
        %201 =       OpCompositeExtract %float %200 0   ; RelaxedPrecision
        %189 =       OpSelect %float %false %201 %198   ; RelaxedPrecision
        %202 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %203 =       OpLoad %v4float %202           ; RelaxedPrecision
        %204 =       OpCompositeExtract %float %203 0   ; RelaxedPrecision
        %205 =       OpFOrdEqual %bool %189 %204
                     OpBranch %188

        %188 = OpLabel
        %206 =   OpPhi %bool %false %182 %205 %187
                 OpSelectionMerge %208 None
                 OpBranchConditional %206 %207 %208

        %207 =     OpLabel
        %210 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %211 =       OpLoad %v4float %210           ; RelaxedPrecision
        %212 =       OpVectorShuffle %v2float %211 %211 0 1     ; RelaxedPrecision
        %213 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %214 =       OpLoad %v4float %213           ; RelaxedPrecision
        %215 =       OpVectorShuffle %v2float %214 %214 0 1     ; RelaxedPrecision
        %216 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %217 =       OpLoad %v4float %216           ; RelaxedPrecision
        %218 =       OpVectorShuffle %v2float %217 %217 0 1     ; RelaxedPrecision
        %219 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %220 =       OpLoad %v4float %219           ; RelaxedPrecision
        %221 =       OpVectorShuffle %v2float %220 %220 0 1     ; RelaxedPrecision
        %209 =       OpSelect %v2float %72 %221 %218            ; RelaxedPrecision
        %222 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %223 =       OpLoad %v4float %222           ; RelaxedPrecision
        %224 =       OpVectorShuffle %v2float %223 %223 0 1     ; RelaxedPrecision
        %225 =       OpFOrdEqual %v2bool %209 %224
        %226 =       OpAll %bool %225
                     OpBranch %208

        %208 = OpLabel
        %227 =   OpPhi %bool %false %188 %226 %207
                 OpSelectionMerge %229 None
                 OpBranchConditional %227 %228 %229

        %228 =     OpLabel
        %231 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %232 =       OpLoad %v4float %231           ; RelaxedPrecision
        %233 =       OpVectorShuffle %v3float %232 %232 0 1 2   ; RelaxedPrecision
        %235 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %236 =       OpLoad %v4float %235           ; RelaxedPrecision
        %237 =       OpVectorShuffle %v3float %236 %236 0 1 2   ; RelaxedPrecision
        %238 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %239 =       OpLoad %v4float %238           ; RelaxedPrecision
        %240 =       OpVectorShuffle %v3float %239 %239 0 1 2   ; RelaxedPrecision
        %241 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %242 =       OpLoad %v4float %241           ; RelaxedPrecision
        %243 =       OpVectorShuffle %v3float %242 %242 0 1 2   ; RelaxedPrecision
        %230 =       OpSelect %v3float %86 %243 %240            ; RelaxedPrecision
        %244 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %245 =       OpLoad %v4float %244           ; RelaxedPrecision
        %246 =       OpVectorShuffle %v3float %245 %245 0 1 2   ; RelaxedPrecision
        %247 =       OpFOrdEqual %v3bool %230 %246
        %248 =       OpAll %bool %247
                     OpBranch %229

        %229 = OpLabel
        %249 =   OpPhi %bool %false %208 %248 %228
                 OpSelectionMerge %251 None
                 OpBranchConditional %249 %250 %251

        %250 =     OpLabel
        %253 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %254 =       OpLoad %v4float %253           ; RelaxedPrecision
        %255 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %256 =       OpLoad %v4float %255           ; RelaxedPrecision
        %257 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %258 =       OpLoad %v4float %257           ; RelaxedPrecision
        %259 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %260 =       OpLoad %v4float %259           ; RelaxedPrecision
        %252 =       OpSelect %v4float %97 %260 %258    ; RelaxedPrecision
        %261 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %262 =       OpLoad %v4float %261           ; RelaxedPrecision
        %263 =       OpFOrdEqual %v4bool %252 %262
        %264 =       OpAll %bool %263
                     OpBranch %251

        %251 = OpLabel
        %265 =   OpPhi %bool %false %229 %264 %250
                 OpSelectionMerge %267 None
                 OpBranchConditional %265 %266 %267

        %266 =     OpLabel
        %269 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %270 =       OpLoad %v4float %269           ; RelaxedPrecision
        %271 =       OpCompositeExtract %float %270 0   ; RelaxedPrecision
        %272 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %273 =       OpLoad %v4float %272           ; RelaxedPrecision
        %274 =       OpCompositeExtract %float %273 0   ; RelaxedPrecision
        %275 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %276 =       OpLoad %v4float %275           ; RelaxedPrecision
        %277 =       OpCompositeExtract %float %276 0   ; RelaxedPrecision
        %278 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %279 =       OpLoad %v4float %278           ; RelaxedPrecision
        %280 =       OpCompositeExtract %float %279 0   ; RelaxedPrecision
        %268 =       OpSelect %float %true %280 %277    ; RelaxedPrecision
        %281 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %282 =       OpLoad %v4float %281           ; RelaxedPrecision
        %283 =       OpCompositeExtract %float %282 0   ; RelaxedPrecision
        %284 =       OpFOrdEqual %bool %268 %283
                     OpBranch %267

        %267 = OpLabel
        %285 =   OpPhi %bool %false %251 %284 %266
                 OpSelectionMerge %287 None
                 OpBranchConditional %285 %286 %287

        %286 =     OpLabel
        %289 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %290 =       OpLoad %v4float %289           ; RelaxedPrecision
        %291 =       OpVectorShuffle %v2float %290 %290 0 1     ; RelaxedPrecision
        %292 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %293 =       OpLoad %v4float %292           ; RelaxedPrecision
        %294 =       OpVectorShuffle %v2float %293 %293 0 1     ; RelaxedPrecision
        %295 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %296 =       OpLoad %v4float %295           ; RelaxedPrecision
        %297 =       OpVectorShuffle %v2float %296 %296 0 1     ; RelaxedPrecision
        %298 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %299 =       OpLoad %v4float %298           ; RelaxedPrecision
        %300 =       OpVectorShuffle %v2float %299 %299 0 1     ; RelaxedPrecision
        %288 =       OpSelect %v2float %112 %300 %297           ; RelaxedPrecision
        %301 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %302 =       OpLoad %v4float %301           ; RelaxedPrecision
        %303 =       OpVectorShuffle %v2float %302 %302 0 1     ; RelaxedPrecision
        %304 =       OpFOrdEqual %v2bool %288 %303
        %305 =       OpAll %bool %304
                     OpBranch %287

        %287 = OpLabel
        %306 =   OpPhi %bool %false %267 %305 %286
                 OpSelectionMerge %308 None
                 OpBranchConditional %306 %307 %308

        %307 =     OpLabel
        %310 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %311 =       OpLoad %v4float %310           ; RelaxedPrecision
        %312 =       OpVectorShuffle %v3float %311 %311 0 1 2   ; RelaxedPrecision
        %313 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %314 =       OpLoad %v4float %313           ; RelaxedPrecision
        %315 =       OpVectorShuffle %v3float %314 %314 0 1 2   ; RelaxedPrecision
        %316 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %317 =       OpLoad %v4float %316           ; RelaxedPrecision
        %318 =       OpVectorShuffle %v3float %317 %317 0 1 2   ; RelaxedPrecision
        %319 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %320 =       OpLoad %v4float %319           ; RelaxedPrecision
        %321 =       OpVectorShuffle %v3float %320 %320 0 1 2   ; RelaxedPrecision
        %309 =       OpSelect %v3float %124 %321 %318           ; RelaxedPrecision
        %322 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %323 =       OpLoad %v4float %322           ; RelaxedPrecision
        %324 =       OpVectorShuffle %v3float %323 %323 0 1 2   ; RelaxedPrecision
        %325 =       OpFOrdEqual %v3bool %309 %324
        %326 =       OpAll %bool %325
                     OpBranch %308

        %308 = OpLabel
        %327 =   OpPhi %bool %false %287 %326 %307
                 OpSelectionMerge %329 None
                 OpBranchConditional %327 %328 %329

        %328 =     OpLabel
        %331 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %332 =       OpLoad %v4float %331           ; RelaxedPrecision
        %333 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %334 =       OpLoad %v4float %333           ; RelaxedPrecision
        %335 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %336 =       OpLoad %v4float %335           ; RelaxedPrecision
        %337 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %338 =       OpLoad %v4float %337           ; RelaxedPrecision
        %330 =       OpSelect %v4float %134 %338 %336   ; RelaxedPrecision
        %339 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %340 =       OpLoad %v4float %339           ; RelaxedPrecision
        %341 =       OpFOrdEqual %v4bool %330 %340
        %342 =       OpAll %bool %341
                     OpBranch %329

        %329 = OpLabel
        %343 =   OpPhi %bool %false %308 %342 %328
                 OpSelectionMerge %345 None
                 OpBranchConditional %343 %344 %345

        %344 =     OpLabel
        %346 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %347 =       OpLoad %v4float %346           ; RelaxedPrecision
        %348 =       OpCompositeExtract %float %347 0   ; RelaxedPrecision
        %349 =       OpFOrdEqual %bool %float_0 %348
                     OpBranch %345

        %345 = OpLabel
        %350 =   OpPhi %bool %false %329 %349 %344
                 OpSelectionMerge %352 None
                 OpBranchConditional %350 %351 %352

        %351 =     OpLabel
        %355 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %356 =       OpLoad %v4float %355           ; RelaxedPrecision
        %357 =       OpVectorShuffle %v2float %356 %356 0 1     ; RelaxedPrecision
        %358 =       OpFOrdEqual %v2bool %354 %357
        %359 =       OpAll %bool %358
                     OpBranch %352

        %352 = OpLabel
        %360 =   OpPhi %bool %false %345 %359 %351
                 OpSelectionMerge %362 None
                 OpBranchConditional %360 %361 %362

        %361 =     OpLabel
        %364 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %365 =       OpLoad %v4float %364           ; RelaxedPrecision
        %366 =       OpVectorShuffle %v3float %365 %365 0 1 2   ; RelaxedPrecision
        %367 =       OpFOrdEqual %v3bool %363 %366
        %368 =       OpAll %bool %367
                     OpBranch %362

        %362 = OpLabel
        %369 =   OpPhi %bool %false %352 %368 %361
                 OpSelectionMerge %371 None
                 OpBranchConditional %369 %370 %371

        %370 =     OpLabel
        %373 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %374 =       OpLoad %v4float %373           ; RelaxedPrecision
        %375 =       OpFOrdEqual %v4bool %372 %374
        %376 =       OpAll %bool %375
                     OpBranch %371

        %371 = OpLabel
        %377 =   OpPhi %bool %false %362 %376 %370
                 OpSelectionMerge %379 None
                 OpBranchConditional %377 %378 %379

        %378 =     OpLabel
        %380 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %381 =       OpLoad %v4float %380           ; RelaxedPrecision
        %382 =       OpCompositeExtract %float %381 0   ; RelaxedPrecision
        %383 =       OpFOrdEqual %bool %float_1 %382
                     OpBranch %379

        %379 = OpLabel
        %384 =   OpPhi %bool %false %371 %383 %378
                 OpSelectionMerge %386 None
                 OpBranchConditional %384 %385 %386

        %385 =     OpLabel
        %388 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %389 =       OpLoad %v4float %388           ; RelaxedPrecision
        %390 =       OpVectorShuffle %v2float %389 %389 0 1     ; RelaxedPrecision
        %391 =       OpFOrdEqual %v2bool %387 %390
        %392 =       OpAll %bool %391
                     OpBranch %386

        %386 = OpLabel
        %393 =   OpPhi %bool %false %379 %392 %385
                 OpSelectionMerge %395 None
                 OpBranchConditional %393 %394 %395

        %394 =     OpLabel
        %397 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %398 =       OpLoad %v4float %397           ; RelaxedPrecision
        %399 =       OpVectorShuffle %v3float %398 %398 0 1 2   ; RelaxedPrecision
        %400 =       OpFOrdEqual %v3bool %396 %399
        %401 =       OpAll %bool %400
                     OpBranch %395

        %395 = OpLabel
        %402 =   OpPhi %bool %false %386 %401 %394
                 OpSelectionMerge %404 None
                 OpBranchConditional %402 %403 %404

        %403 =     OpLabel
        %406 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %407 =       OpLoad %v4float %406           ; RelaxedPrecision
        %408 =       OpFOrdEqual %v4bool %405 %407
        %409 =       OpAll %bool %408
                     OpBranch %404

        %404 = OpLabel
        %410 =   OpPhi %bool %false %395 %409 %403
                 OpSelectionMerge %415 None
                 OpBranchConditional %410 %413 %414

        %413 =     OpLabel
        %416 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %417 =       OpLoad %v4float %416           ; RelaxedPrecision
                     OpStore %411 %417
                     OpBranch %415

        %414 =     OpLabel
        %418 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %419 =       OpLoad %v4float %418           ; RelaxedPrecision
                     OpStore %411 %419
                     OpBranch %415

        %415 = OpLabel
        %420 =   OpLoad %v4float %411               ; RelaxedPrecision
                 OpReturnValue %420
               OpFunctionEnd
