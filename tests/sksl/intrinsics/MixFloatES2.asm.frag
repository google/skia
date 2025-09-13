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
               OpName %expectedBW "expectedBW"          ; id %27
               OpName %expectedWT "expectedWT"          ; id %32

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
               OpDecorate %expectedBW RelaxedPrecision
               OpDecorate %expectedWT RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %164 RelaxedPrecision
               OpDecorate %166 RelaxedPrecision
               OpDecorate %167 RelaxedPrecision
               OpDecorate %170 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %190 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %198 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
               OpDecorate %206 RelaxedPrecision
               OpDecorate %208 RelaxedPrecision
               OpDecorate %219 RelaxedPrecision
               OpDecorate %226 RelaxedPrecision
               OpDecorate %238 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision
               OpDecorate %241 RelaxedPrecision

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
%_ptr_Function_v4float = OpTypePointer Function %v4float
  %float_0_5 = OpConstant %float 0.5
    %float_1 = OpConstant %float 1
         %31 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_1
 %float_2_25 = OpConstant %float 2.25
         %34 = OpConstantComposite %v4float %float_1 %float_0_5 %float_1 %float_2_25
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
         %45 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
         %46 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
     %v4bool = OpTypeVector %bool 4
 %float_0_25 = OpConstant %float 0.25
         %58 = OpConstantComposite %v4float %float_0_25 %float_0_25 %float_0_25 %float_0_25
 %float_0_75 = OpConstant %float 0.75
         %60 = OpConstantComposite %v4float %float_0_25 %float_0_75 %float_0 %float_1
         %71 = OpConstantComposite %v4float %float_0_75 %float_0_75 %float_0_75 %float_0_75
         %72 = OpConstantComposite %v4float %float_0_75 %float_0_25 %float_0 %float_1
         %83 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
         %84 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
        %110 = OpConstantComposite %v2float %float_0_5 %float_0_5
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
        %126 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
     %v3bool = OpTypeVector %bool 3
        %139 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
       %true = OpConstantTrue %bool
      %int_4 = OpConstant %int 4
        %183 = OpConstantComposite %v2float %float_0 %float_0_5
        %197 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
        %209 = OpConstantComposite %v4float %float_0 %float_0_5 %float_0 %float_1
        %218 = OpConstantComposite %v2float %float_1 %float_0_5
        %225 = OpConstantComposite %v3float %float_1 %float_0_5 %float_1


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
 %expectedBW =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
 %expectedWT =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
        %233 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %expectedBW %31
                 OpStore %expectedWT %34
         %38 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %41 =   OpLoad %v4float %38                ; RelaxedPrecision
         %42 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %44 =   OpLoad %v4float %42                ; RelaxedPrecision
         %37 =   OpExtInst %v4float %5 FMix %41 %44 %45     ; RelaxedPrecision
         %47 =   OpFOrdEqual %v4bool %37 %46
         %49 =   OpAll %bool %47
                 OpSelectionMerge %51 None
                 OpBranchConditional %49 %50 %51

         %50 =     OpLabel
         %53 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %54 =       OpLoad %v4float %53            ; RelaxedPrecision
         %55 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %56 =       OpLoad %v4float %55            ; RelaxedPrecision
         %52 =       OpExtInst %v4float %5 FMix %54 %56 %58     ; RelaxedPrecision
         %61 =       OpFOrdEqual %v4bool %52 %60
         %62 =       OpAll %bool %61
                     OpBranch %51

         %51 = OpLabel
         %63 =   OpPhi %bool %false %26 %62 %50
                 OpSelectionMerge %65 None
                 OpBranchConditional %63 %64 %65

         %64 =     OpLabel
         %67 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %68 =       OpLoad %v4float %67            ; RelaxedPrecision
         %69 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %70 =       OpLoad %v4float %69            ; RelaxedPrecision
         %66 =       OpExtInst %v4float %5 FMix %68 %70 %71     ; RelaxedPrecision
         %73 =       OpFOrdEqual %v4bool %66 %72
         %74 =       OpAll %bool %73
                     OpBranch %65

         %65 = OpLabel
         %75 =   OpPhi %bool %false %51 %74 %64
                 OpSelectionMerge %77 None
                 OpBranchConditional %75 %76 %77

         %76 =     OpLabel
         %79 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %80 =       OpLoad %v4float %79            ; RelaxedPrecision
         %81 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %82 =       OpLoad %v4float %81            ; RelaxedPrecision
         %78 =       OpExtInst %v4float %5 FMix %80 %82 %83     ; RelaxedPrecision
         %85 =       OpFOrdEqual %v4bool %78 %84
         %86 =       OpAll %bool %85
                     OpBranch %77

         %77 = OpLabel
         %87 =   OpPhi %bool %false %65 %86 %76
                 OpSelectionMerge %89 None
                 OpBranchConditional %87 %88 %89

         %88 =     OpLabel
         %91 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %93 =       OpLoad %v4float %91            ; RelaxedPrecision
         %94 =       OpCompositeExtract %float %93 0    ; RelaxedPrecision
         %95 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
         %97 =       OpLoad %v4float %95            ; RelaxedPrecision
         %98 =       OpCompositeExtract %float %97 0    ; RelaxedPrecision
         %90 =       OpExtInst %float %5 FMix %94 %98 %float_0_5    ; RelaxedPrecision
         %99 =       OpFOrdEqual %bool %90 %float_0_5
                     OpBranch %89

         %89 = OpLabel
        %100 =   OpPhi %bool %false %77 %99 %88
                 OpSelectionMerge %102 None
                 OpBranchConditional %100 %101 %102

        %101 =     OpLabel
        %104 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %105 =       OpLoad %v4float %104           ; RelaxedPrecision
        %106 =       OpVectorShuffle %v2float %105 %105 0 1     ; RelaxedPrecision
        %107 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %108 =       OpLoad %v4float %107           ; RelaxedPrecision
        %109 =       OpVectorShuffle %v2float %108 %108 0 1     ; RelaxedPrecision
        %103 =       OpExtInst %v2float %5 FMix %106 %109 %110  ; RelaxedPrecision
        %111 =       OpVectorShuffle %v2float %31 %31 0 1       ; RelaxedPrecision
        %112 =       OpFOrdEqual %v2bool %103 %111
        %114 =       OpAll %bool %112
                     OpBranch %102

        %102 = OpLabel
        %115 =   OpPhi %bool %false %89 %114 %101
                 OpSelectionMerge %117 None
                 OpBranchConditional %115 %116 %117

        %116 =     OpLabel
        %119 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %120 =       OpLoad %v4float %119           ; RelaxedPrecision
        %121 =       OpVectorShuffle %v3float %120 %120 0 1 2   ; RelaxedPrecision
        %123 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %124 =       OpLoad %v4float %123           ; RelaxedPrecision
        %125 =       OpVectorShuffle %v3float %124 %124 0 1 2   ; RelaxedPrecision
        %118 =       OpExtInst %v3float %5 FMix %121 %125 %126  ; RelaxedPrecision
        %127 =       OpVectorShuffle %v3float %31 %31 0 1 2     ; RelaxedPrecision
        %128 =       OpFOrdEqual %v3bool %118 %127
        %130 =       OpAll %bool %128
                     OpBranch %117

        %117 = OpLabel
        %131 =   OpPhi %bool %false %102 %130 %116
                 OpSelectionMerge %133 None
                 OpBranchConditional %131 %132 %133

        %132 =     OpLabel
        %135 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %136 =       OpLoad %v4float %135           ; RelaxedPrecision
        %137 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %138 =       OpLoad %v4float %137           ; RelaxedPrecision
        %134 =       OpExtInst %v4float %5 FMix %136 %138 %139  ; RelaxedPrecision
        %140 =       OpFOrdEqual %v4bool %134 %31
        %141 =       OpAll %bool %140
                     OpBranch %133

        %133 = OpLabel
        %142 =   OpPhi %bool %false %117 %141 %132
                 OpSelectionMerge %144 None
                 OpBranchConditional %142 %143 %144

        %143 =     OpLabel
                     OpBranch %144

        %144 = OpLabel
        %146 =   OpPhi %bool %false %133 %true %143
                 OpSelectionMerge %148 None
                 OpBranchConditional %146 %147 %148

        %147 =     OpLabel
        %149 =       OpVectorShuffle %v2float %31 %31 0 1   ; RelaxedPrecision
        %150 =       OpFOrdEqual %v2bool %110 %149
        %151 =       OpAll %bool %150
                     OpBranch %148

        %148 = OpLabel
        %152 =   OpPhi %bool %false %144 %151 %147
                 OpSelectionMerge %154 None
                 OpBranchConditional %152 %153 %154

        %153 =     OpLabel
        %155 =       OpVectorShuffle %v3float %31 %31 0 1 2     ; RelaxedPrecision
        %156 =       OpFOrdEqual %v3bool %126 %155
        %157 =       OpAll %bool %156
                     OpBranch %154

        %154 = OpLabel
        %158 =   OpPhi %bool %false %148 %157 %153
                 OpSelectionMerge %160 None
                 OpBranchConditional %158 %159 %160

        %159 =     OpLabel
                     OpBranch %160

        %160 = OpLabel
        %161 =   OpPhi %bool %false %154 %true %159
                 OpSelectionMerge %163 None
                 OpBranchConditional %161 %162 %163

        %162 =     OpLabel
        %165 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %166 =       OpLoad %v4float %165           ; RelaxedPrecision
        %167 =       OpCompositeExtract %float %166 0   ; RelaxedPrecision
        %168 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %170 =       OpLoad %v4float %168           ; RelaxedPrecision
        %171 =       OpCompositeExtract %float %170 0   ; RelaxedPrecision
        %164 =       OpExtInst %float %5 FMix %167 %171 %float_0    ; RelaxedPrecision
        %172 =       OpFOrdEqual %bool %164 %float_1
                     OpBranch %163

        %163 = OpLabel
        %173 =   OpPhi %bool %false %160 %172 %162
                 OpSelectionMerge %175 None
                 OpBranchConditional %173 %174 %175

        %174 =     OpLabel
        %177 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %178 =       OpLoad %v4float %177           ; RelaxedPrecision
        %179 =       OpVectorShuffle %v2float %178 %178 0 1     ; RelaxedPrecision
        %180 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %181 =       OpLoad %v4float %180           ; RelaxedPrecision
        %182 =       OpVectorShuffle %v2float %181 %181 0 1     ; RelaxedPrecision
        %176 =       OpExtInst %v2float %5 FMix %179 %182 %183  ; RelaxedPrecision
        %184 =       OpVectorShuffle %v2float %34 %34 0 1       ; RelaxedPrecision
        %185 =       OpFOrdEqual %v2bool %176 %184
        %186 =       OpAll %bool %185
                     OpBranch %175

        %175 = OpLabel
        %187 =   OpPhi %bool %false %163 %186 %174
                 OpSelectionMerge %189 None
                 OpBranchConditional %187 %188 %189

        %188 =     OpLabel
        %191 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %192 =       OpLoad %v4float %191           ; RelaxedPrecision
        %193 =       OpVectorShuffle %v3float %192 %192 0 1 2   ; RelaxedPrecision
        %194 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %195 =       OpLoad %v4float %194           ; RelaxedPrecision
        %196 =       OpVectorShuffle %v3float %195 %195 0 1 2   ; RelaxedPrecision
        %190 =       OpExtInst %v3float %5 FMix %193 %196 %197  ; RelaxedPrecision
        %198 =       OpVectorShuffle %v3float %34 %34 0 1 2     ; RelaxedPrecision
        %199 =       OpFOrdEqual %v3bool %190 %198
        %200 =       OpAll %bool %199
                     OpBranch %189

        %189 = OpLabel
        %201 =   OpPhi %bool %false %175 %200 %188
                 OpSelectionMerge %203 None
                 OpBranchConditional %201 %202 %203

        %202 =     OpLabel
        %205 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %206 =       OpLoad %v4float %205           ; RelaxedPrecision
        %207 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %208 =       OpLoad %v4float %207           ; RelaxedPrecision
        %204 =       OpExtInst %v4float %5 FMix %206 %208 %209  ; RelaxedPrecision
        %210 =       OpFOrdEqual %v4bool %204 %34
        %211 =       OpAll %bool %210
                     OpBranch %203

        %203 = OpLabel
        %212 =   OpPhi %bool %false %189 %211 %202
                 OpSelectionMerge %214 None
                 OpBranchConditional %212 %213 %214

        %213 =     OpLabel
                     OpBranch %214

        %214 = OpLabel
        %215 =   OpPhi %bool %false %203 %true %213
                 OpSelectionMerge %217 None
                 OpBranchConditional %215 %216 %217

        %216 =     OpLabel
        %219 =       OpVectorShuffle %v2float %34 %34 0 1   ; RelaxedPrecision
        %220 =       OpFOrdEqual %v2bool %218 %219
        %221 =       OpAll %bool %220
                     OpBranch %217

        %217 = OpLabel
        %222 =   OpPhi %bool %false %214 %221 %216
                 OpSelectionMerge %224 None
                 OpBranchConditional %222 %223 %224

        %223 =     OpLabel
        %226 =       OpVectorShuffle %v3float %34 %34 0 1 2     ; RelaxedPrecision
        %227 =       OpFOrdEqual %v3bool %225 %226
        %228 =       OpAll %bool %227
                     OpBranch %224

        %224 = OpLabel
        %229 =   OpPhi %bool %false %217 %228 %223
                 OpSelectionMerge %231 None
                 OpBranchConditional %229 %230 %231

        %230 =     OpLabel
                     OpBranch %231

        %231 = OpLabel
        %232 =   OpPhi %bool %false %224 %true %230
                 OpSelectionMerge %236 None
                 OpBranchConditional %232 %234 %235

        %234 =     OpLabel
        %237 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %238 =       OpLoad %v4float %237           ; RelaxedPrecision
                     OpStore %233 %238
                     OpBranch %236

        %235 =     OpLabel
        %239 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %240 =       OpLoad %v4float %239           ; RelaxedPrecision
                     OpStore %233 %240
                     OpBranch %236

        %236 = OpLabel
        %241 =   OpLoad %v4float %233               ; RelaxedPrecision
                 OpReturnValue %241
               OpFunctionEnd
