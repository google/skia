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
               OpMemberName %_UniformBuffer 2 "testInputs"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %ok "ok"                          ; id %27

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
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %141 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %146 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
               OpDecorate %163 RelaxedPrecision
               OpDecorate %170 RelaxedPrecision
               OpDecorate %171 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %188 RelaxedPrecision
               OpDecorate %190 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %194 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %203 RelaxedPrecision
               OpDecorate %205 RelaxedPrecision
               OpDecorate %206 RelaxedPrecision
               OpDecorate %207 RelaxedPrecision
               OpDecorate %208 RelaxedPrecision
               OpDecorate %209 RelaxedPrecision
               OpDecorate %210 RelaxedPrecision
               OpDecorate %211 RelaxedPrecision
               OpDecorate %212 RelaxedPrecision
               OpDecorate %213 RelaxedPrecision
               OpDecorate %214 RelaxedPrecision
               OpDecorate %216 RelaxedPrecision
               OpDecorate %223 RelaxedPrecision
               OpDecorate %226 RelaxedPrecision
               OpDecorate %227 RelaxedPrecision
               OpDecorate %228 RelaxedPrecision
               OpDecorate %229 RelaxedPrecision
               OpDecorate %230 RelaxedPrecision
               OpDecorate %231 RelaxedPrecision
               OpDecorate %232 RelaxedPrecision
               OpDecorate %233 RelaxedPrecision
               OpDecorate %234 RelaxedPrecision
               OpDecorate %238 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision
               OpDecorate %247 RelaxedPrecision
               OpDecorate %250 RelaxedPrecision
               OpDecorate %251 RelaxedPrecision
               OpDecorate %252 RelaxedPrecision
               OpDecorate %253 RelaxedPrecision
               OpDecorate %254 RelaxedPrecision
               OpDecorate %255 RelaxedPrecision
               OpDecorate %256 RelaxedPrecision
               OpDecorate %257 RelaxedPrecision
               OpDecorate %261 RelaxedPrecision
               OpDecorate %263 RelaxedPrecision
               OpDecorate %273 RelaxedPrecision
               OpDecorate %275 RelaxedPrecision
               OpDecorate %276 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_2 = OpConstant %int 2
%mat2v2float = OpTypeMatrix %v2float 2
%float_n1_25 = OpConstant %float -1.25
 %float_0_75 = OpConstant %float 0.75
 %float_2_25 = OpConstant %float 2.25
         %49 = OpConstantComposite %v2float %float_n1_25 %float_0
         %50 = OpConstantComposite %v2float %float_0_75 %float_2_25
         %51 = OpConstantComposite %mat2v2float %49 %50
     %v2bool = OpTypeVector %bool 2
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %89 = OpConstantComposite %v2float %float_0 %float_1
         %90 = OpConstantComposite %mat2v2float %89 %89
      %v4int = OpTypeVector %int 4
     %v4bool = OpTypeVector %bool 4
      %int_1 = OpConstant %int 1
   %float_n1 = OpConstant %float -1
        %236 = OpConstantComposite %v2float %float_n1 %float_1
        %237 = OpConstantComposite %mat2v2float %236 %20
    %float_5 = OpConstant %float 5
        %249 = OpConstantComposite %v4float %float_5 %float_5 %float_5 %float_5
    %float_6 = OpConstant %float 6
        %259 = OpConstantComposite %v2float %float_5 %float_6
        %260 = OpConstantComposite %mat2v2float %259 %259
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
         %ok =   OpVariable %_ptr_Function_bool Function
        %267 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %ok %true
                 OpSelectionMerge %33 None
                 OpBranchConditional %true %32 %33

         %32 =     OpLabel
         %34 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %37 =       OpLoad %v4float %34            ; RelaxedPrecision
         %38 =       OpCompositeExtract %float %37 0    ; RelaxedPrecision
         %39 =       OpCompositeExtract %float %37 1    ; RelaxedPrecision
         %40 =       OpCompositeExtract %float %37 2    ; RelaxedPrecision
         %41 =       OpCompositeExtract %float %37 3    ; RelaxedPrecision
         %42 =       OpCompositeConstruct %v2float %38 %39  ; RelaxedPrecision
         %43 =       OpCompositeConstruct %v2float %40 %41  ; RelaxedPrecision
         %45 =       OpCompositeConstruct %mat2v2float %42 %43  ; RelaxedPrecision
         %53 =       OpFOrdEqual %v2bool %42 %49                ; RelaxedPrecision
         %54 =       OpAll %bool %53
         %55 =       OpFOrdEqual %v2bool %43 %50    ; RelaxedPrecision
         %56 =       OpAll %bool %55
         %57 =       OpLogicalAnd %bool %54 %56
                     OpBranch %33

         %33 = OpLabel
         %58 =   OpPhi %bool %false %26 %57 %32
                 OpStore %ok %58
                 OpSelectionMerge %60 None
                 OpBranchConditional %58 %59 %60

         %59 =     OpLabel
         %61 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %62 =       OpLoad %v4float %61            ; RelaxedPrecision
         %63 =       OpCompositeExtract %float %62 0
         %64 =       OpCompositeExtract %float %62 1
         %65 =       OpCompositeExtract %float %62 2
         %66 =       OpCompositeExtract %float %62 3
         %67 =       OpCompositeConstruct %v2float %63 %64
         %68 =       OpCompositeConstruct %v2float %65 %66
         %69 =       OpCompositeConstruct %mat2v2float %67 %68
         %70 =       OpFOrdEqual %v2bool %67 %49
         %71 =       OpAll %bool %70
         %72 =       OpFOrdEqual %v2bool %68 %50
         %73 =       OpAll %bool %72
         %74 =       OpLogicalAnd %bool %71 %73
                     OpBranch %60

         %60 = OpLabel
         %75 =   OpPhi %bool %false %33 %74 %59
                 OpStore %ok %75
                 OpSelectionMerge %77 None
                 OpBranchConditional %75 %76 %77

         %76 =     OpLabel
         %78 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %80 =       OpLoad %v4float %78            ; RelaxedPrecision
         %81 =       OpCompositeExtract %float %80 0    ; RelaxedPrecision
         %82 =       OpCompositeExtract %float %80 1    ; RelaxedPrecision
         %83 =       OpCompositeExtract %float %80 2    ; RelaxedPrecision
         %84 =       OpCompositeExtract %float %80 3    ; RelaxedPrecision
         %85 =       OpCompositeConstruct %v2float %81 %82  ; RelaxedPrecision
         %86 =       OpCompositeConstruct %v2float %83 %84  ; RelaxedPrecision
         %87 =       OpCompositeConstruct %mat2v2float %85 %86  ; RelaxedPrecision
         %91 =       OpFOrdEqual %v2bool %85 %89                ; RelaxedPrecision
         %92 =       OpAll %bool %91
         %93 =       OpFOrdEqual %v2bool %86 %89    ; RelaxedPrecision
         %94 =       OpAll %bool %93
         %95 =       OpLogicalAnd %bool %92 %94
                     OpBranch %77

         %77 = OpLabel
         %96 =   OpPhi %bool %false %60 %95 %76
                 OpStore %ok %96
                 OpSelectionMerge %98 None
                 OpBranchConditional %96 %97 %98

         %97 =     OpLabel
         %99 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %100 =       OpLoad %v4float %99            ; RelaxedPrecision
        %101 =       OpCompositeExtract %float %100 0   ; RelaxedPrecision
        %102 =       OpCompositeExtract %float %100 1   ; RelaxedPrecision
        %103 =       OpCompositeExtract %float %100 2   ; RelaxedPrecision
        %104 =       OpCompositeExtract %float %100 3   ; RelaxedPrecision
        %105 =       OpCompositeConstruct %v2float %101 %102    ; RelaxedPrecision
        %106 =       OpCompositeConstruct %v2float %103 %104    ; RelaxedPrecision
        %107 =       OpCompositeConstruct %mat2v2float %105 %106    ; RelaxedPrecision
        %108 =       OpFOrdEqual %v2bool %105 %89                   ; RelaxedPrecision
        %109 =       OpAll %bool %108
        %110 =       OpFOrdEqual %v2bool %106 %89   ; RelaxedPrecision
        %111 =       OpAll %bool %110
        %112 =       OpLogicalAnd %bool %109 %111
                     OpBranch %98

         %98 = OpLabel
        %113 =   OpPhi %bool %false %77 %112 %97
                 OpStore %ok %113
                 OpSelectionMerge %115 None
                 OpBranchConditional %113 %114 %115

        %114 =     OpLabel
        %116 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %117 =       OpLoad %v4float %116           ; RelaxedPrecision
        %118 =       OpCompositeExtract %float %117 0   ; RelaxedPrecision
        %119 =       OpConvertFToS %int %118
        %120 =       OpCompositeExtract %float %117 1   ; RelaxedPrecision
        %121 =       OpConvertFToS %int %120
        %122 =       OpCompositeExtract %float %117 2   ; RelaxedPrecision
        %123 =       OpConvertFToS %int %122
        %124 =       OpCompositeExtract %float %117 3   ; RelaxedPrecision
        %125 =       OpConvertFToS %int %124
        %127 =       OpCompositeConstruct %v4int %119 %121 %123 %125
        %128 =       OpCompositeExtract %int %127 0
        %129 =       OpConvertSToF %float %128      ; RelaxedPrecision
        %130 =       OpCompositeExtract %int %127 1
        %131 =       OpConvertSToF %float %130      ; RelaxedPrecision
        %132 =       OpCompositeExtract %int %127 2
        %133 =       OpConvertSToF %float %132      ; RelaxedPrecision
        %134 =       OpCompositeExtract %int %127 3
        %135 =       OpConvertSToF %float %134      ; RelaxedPrecision
        %136 =       OpCompositeConstruct %v4float %129 %131 %133 %135  ; RelaxedPrecision
        %137 =       OpCompositeExtract %float %136 0                   ; RelaxedPrecision
        %138 =       OpCompositeExtract %float %136 1                   ; RelaxedPrecision
        %139 =       OpCompositeExtract %float %136 2                   ; RelaxedPrecision
        %140 =       OpCompositeExtract %float %136 3                   ; RelaxedPrecision
        %141 =       OpCompositeConstruct %v2float %137 %138            ; RelaxedPrecision
        %142 =       OpCompositeConstruct %v2float %139 %140            ; RelaxedPrecision
        %143 =       OpCompositeConstruct %mat2v2float %141 %142        ; RelaxedPrecision
        %144 =       OpFOrdEqual %v2bool %141 %89                       ; RelaxedPrecision
        %145 =       OpAll %bool %144
        %146 =       OpFOrdEqual %v2bool %142 %89   ; RelaxedPrecision
        %147 =       OpAll %bool %146
        %148 =       OpLogicalAnd %bool %145 %147
                     OpBranch %115

        %115 = OpLabel
        %149 =   OpPhi %bool %false %98 %148 %114
                 OpStore %ok %149
                 OpSelectionMerge %151 None
                 OpBranchConditional %149 %150 %151

        %150 =     OpLabel
        %152 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %153 =       OpLoad %v4float %152           ; RelaxedPrecision
        %154 =       OpCompositeExtract %float %153 0   ; RelaxedPrecision
        %155 =       OpCompositeExtract %float %153 1   ; RelaxedPrecision
        %156 =       OpCompositeExtract %float %153 2   ; RelaxedPrecision
        %157 =       OpCompositeExtract %float %153 3   ; RelaxedPrecision
        %158 =       OpCompositeConstruct %v2float %154 %155    ; RelaxedPrecision
        %159 =       OpCompositeConstruct %v2float %156 %157    ; RelaxedPrecision
        %160 =       OpCompositeConstruct %mat2v2float %158 %159    ; RelaxedPrecision
        %161 =       OpFOrdEqual %v2bool %158 %89                   ; RelaxedPrecision
        %162 =       OpAll %bool %161
        %163 =       OpFOrdEqual %v2bool %159 %89   ; RelaxedPrecision
        %164 =       OpAll %bool %163
        %165 =       OpLogicalAnd %bool %162 %164
                     OpBranch %151

        %151 = OpLabel
        %166 =   OpPhi %bool %false %115 %165 %150
                 OpStore %ok %166
                 OpSelectionMerge %168 None
                 OpBranchConditional %166 %167 %168

        %167 =     OpLabel
        %169 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %170 =       OpLoad %v4float %169           ; RelaxedPrecision
        %171 =       OpCompositeExtract %float %170 0   ; RelaxedPrecision
        %172 =       OpCompositeExtract %float %170 1   ; RelaxedPrecision
        %173 =       OpCompositeExtract %float %170 2   ; RelaxedPrecision
        %174 =       OpCompositeExtract %float %170 3   ; RelaxedPrecision
        %175 =       OpCompositeConstruct %v2float %171 %172    ; RelaxedPrecision
        %176 =       OpCompositeConstruct %v2float %173 %174    ; RelaxedPrecision
        %177 =       OpCompositeConstruct %mat2v2float %175 %176    ; RelaxedPrecision
        %178 =       OpFOrdEqual %v2bool %175 %89                   ; RelaxedPrecision
        %179 =       OpAll %bool %178
        %180 =       OpFOrdEqual %v2bool %176 %89   ; RelaxedPrecision
        %181 =       OpAll %bool %180
        %182 =       OpLogicalAnd %bool %179 %181
                     OpBranch %168

        %168 = OpLabel
        %183 =   OpPhi %bool %false %151 %182 %167
                 OpStore %ok %183
                 OpSelectionMerge %185 None
                 OpBranchConditional %183 %184 %185

        %184 =     OpLabel
        %186 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %187 =       OpLoad %v4float %186           ; RelaxedPrecision
        %188 =       OpCompositeExtract %float %187 0   ; RelaxedPrecision
        %189 =       OpFUnordNotEqual %bool %188 %float_0
        %190 =       OpCompositeExtract %float %187 1   ; RelaxedPrecision
        %191 =       OpFUnordNotEqual %bool %190 %float_0
        %192 =       OpCompositeExtract %float %187 2   ; RelaxedPrecision
        %193 =       OpFUnordNotEqual %bool %192 %float_0
        %194 =       OpCompositeExtract %float %187 3   ; RelaxedPrecision
        %195 =       OpFUnordNotEqual %bool %194 %float_0
        %197 =       OpCompositeConstruct %v4bool %189 %191 %193 %195
        %198 =       OpCompositeExtract %bool %197 0
        %199 =       OpSelect %float %198 %float_1 %float_0     ; RelaxedPrecision
        %200 =       OpCompositeExtract %bool %197 1
        %201 =       OpSelect %float %200 %float_1 %float_0     ; RelaxedPrecision
        %202 =       OpCompositeExtract %bool %197 2
        %203 =       OpSelect %float %202 %float_1 %float_0     ; RelaxedPrecision
        %204 =       OpCompositeExtract %bool %197 3
        %205 =       OpSelect %float %204 %float_1 %float_0     ; RelaxedPrecision
        %206 =       OpCompositeConstruct %v4float %199 %201 %203 %205  ; RelaxedPrecision
        %207 =       OpCompositeExtract %float %206 0                   ; RelaxedPrecision
        %208 =       OpCompositeExtract %float %206 1                   ; RelaxedPrecision
        %209 =       OpCompositeExtract %float %206 2                   ; RelaxedPrecision
        %210 =       OpCompositeExtract %float %206 3                   ; RelaxedPrecision
        %211 =       OpCompositeConstruct %v2float %207 %208            ; RelaxedPrecision
        %212 =       OpCompositeConstruct %v2float %209 %210            ; RelaxedPrecision
        %213 =       OpCompositeConstruct %mat2v2float %211 %212        ; RelaxedPrecision
        %214 =       OpFOrdEqual %v2bool %211 %89                       ; RelaxedPrecision
        %215 =       OpAll %bool %214
        %216 =       OpFOrdEqual %v2bool %212 %89   ; RelaxedPrecision
        %217 =       OpAll %bool %216
        %218 =       OpLogicalAnd %bool %215 %217
                     OpBranch %185

        %185 = OpLabel
        %219 =   OpPhi %bool %false %168 %218 %184
                 OpStore %ok %219
                 OpSelectionMerge %221 None
                 OpBranchConditional %219 %220 %221

        %220 =     OpLabel
        %222 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %223 =       OpLoad %v4float %222           ; RelaxedPrecision
        %224 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %226 =       OpLoad %v4float %224           ; RelaxedPrecision
        %227 =       OpFSub %v4float %223 %226      ; RelaxedPrecision
        %228 =       OpCompositeExtract %float %227 0   ; RelaxedPrecision
        %229 =       OpCompositeExtract %float %227 1   ; RelaxedPrecision
        %230 =       OpCompositeExtract %float %227 2   ; RelaxedPrecision
        %231 =       OpCompositeExtract %float %227 3   ; RelaxedPrecision
        %232 =       OpCompositeConstruct %v2float %228 %229    ; RelaxedPrecision
        %233 =       OpCompositeConstruct %v2float %230 %231    ; RelaxedPrecision
        %234 =       OpCompositeConstruct %mat2v2float %232 %233    ; RelaxedPrecision
        %238 =       OpFOrdEqual %v2bool %232 %236                  ; RelaxedPrecision
        %239 =       OpAll %bool %238
        %240 =       OpFOrdEqual %v2bool %233 %20   ; RelaxedPrecision
        %241 =       OpAll %bool %240
        %242 =       OpLogicalAnd %bool %239 %241
                     OpBranch %221

        %221 = OpLabel
        %243 =   OpPhi %bool %false %185 %242 %220
                 OpStore %ok %243
                 OpSelectionMerge %245 None
                 OpBranchConditional %243 %244 %245

        %244 =     OpLabel
        %246 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %247 =       OpLoad %v4float %246           ; RelaxedPrecision
        %250 =       OpFAdd %v4float %247 %249      ; RelaxedPrecision
        %251 =       OpCompositeExtract %float %250 0   ; RelaxedPrecision
        %252 =       OpCompositeExtract %float %250 1   ; RelaxedPrecision
        %253 =       OpCompositeExtract %float %250 2   ; RelaxedPrecision
        %254 =       OpCompositeExtract %float %250 3   ; RelaxedPrecision
        %255 =       OpCompositeConstruct %v2float %251 %252    ; RelaxedPrecision
        %256 =       OpCompositeConstruct %v2float %253 %254    ; RelaxedPrecision
        %257 =       OpCompositeConstruct %mat2v2float %255 %256    ; RelaxedPrecision
        %261 =       OpFOrdEqual %v2bool %255 %259                  ; RelaxedPrecision
        %262 =       OpAll %bool %261
        %263 =       OpFOrdEqual %v2bool %256 %259  ; RelaxedPrecision
        %264 =       OpAll %bool %263
        %265 =       OpLogicalAnd %bool %262 %264
                     OpBranch %245

        %245 = OpLabel
        %266 =   OpPhi %bool %false %221 %265 %244
                 OpStore %ok %266
                 OpSelectionMerge %271 None
                 OpBranchConditional %266 %269 %270

        %269 =     OpLabel
        %272 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %273 =       OpLoad %v4float %272           ; RelaxedPrecision
                     OpStore %267 %273
                     OpBranch %271

        %270 =     OpLabel
        %274 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %275 =       OpLoad %v4float %274           ; RelaxedPrecision
                     OpStore %267 %275
                     OpBranch %271

        %271 = OpLabel
        %276 =   OpLoad %v4float %267               ; RelaxedPrecision
                 OpReturnValue %276
               OpFunctionEnd
