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
               OpName %FTFT "FTFT"                      ; id %27
               OpName %TFTF "TFTF"                      ; id %44

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
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision
               OpDecorate %139 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
               OpDecorate %162 RelaxedPrecision
               OpDecorate %167 RelaxedPrecision
               OpDecorate %169 RelaxedPrecision
               OpDecorate %170 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %194 RelaxedPrecision
               OpDecorate %196 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %203 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
               OpDecorate %207 RelaxedPrecision
               OpDecorate %208 RelaxedPrecision
               OpDecorate %210 RelaxedPrecision
               OpDecorate %211 RelaxedPrecision
               OpDecorate %212 RelaxedPrecision
               OpDecorate %218 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %222 RelaxedPrecision
               OpDecorate %224 RelaxedPrecision
               OpDecorate %226 RelaxedPrecision
               OpDecorate %228 RelaxedPrecision
               OpDecorate %229 RelaxedPrecision
               OpDecorate %231 RelaxedPrecision
               OpDecorate %232 RelaxedPrecision
               OpDecorate %233 RelaxedPrecision
               OpDecorate %243 RelaxedPrecision
               OpDecorate %246 RelaxedPrecision
               OpDecorate %247 RelaxedPrecision

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
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %false = OpConstantFalse %bool
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
     %v2bool = OpTypeVector %bool 2
    %float_1 = OpConstant %float 1
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
      %int_4 = OpConstant %int 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1


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
       %FTFT =   OpVariable %_ptr_Function_v4bool Function
       %TFTF =   OpVariable %_ptr_Function_v4bool Function
        %237 =   OpVariable %_ptr_Function_v4float Function
         %31 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %34 =   OpLoad %v4float %31                ; RelaxedPrecision
         %35 =   OpCompositeExtract %float %34 0    ; RelaxedPrecision
         %36 =   OpFUnordNotEqual %bool %35 %float_0
         %37 =   OpCompositeExtract %float %34 1    ; RelaxedPrecision
         %38 =   OpFUnordNotEqual %bool %37 %float_0
         %39 =   OpCompositeExtract %float %34 2    ; RelaxedPrecision
         %40 =   OpFUnordNotEqual %bool %39 %float_0
         %41 =   OpCompositeExtract %float %34 3    ; RelaxedPrecision
         %42 =   OpFUnordNotEqual %bool %41 %float_0
         %43 =   OpCompositeConstruct %v4bool %36 %38 %40 %42
                 OpStore %FTFT %43
         %45 =   OpVectorShuffle %v4bool %43 %43 3 2 1 0
                 OpStore %TFTF %45
         %48 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %50 =   OpLoad %v4float %48                ; RelaxedPrecision
         %51 =   OpCompositeExtract %float %50 0    ; RelaxedPrecision
         %52 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_3
         %54 =   OpLoad %v4float %52                ; RelaxedPrecision
         %55 =   OpCompositeExtract %float %54 0    ; RelaxedPrecision
         %56 =   OpCompositeExtract %bool %43 0
         %57 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %58 =   OpLoad %v4float %57                ; RelaxedPrecision
         %59 =   OpCompositeExtract %float %58 0    ; RelaxedPrecision
         %60 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_3
         %61 =   OpLoad %v4float %60                ; RelaxedPrecision
         %62 =   OpCompositeExtract %float %61 0    ; RelaxedPrecision
         %47 =   OpSelect %float %56 %62 %59        ; RelaxedPrecision
         %63 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %64 =   OpLoad %v4float %63                ; RelaxedPrecision
         %65 =   OpCompositeExtract %float %64 0    ; RelaxedPrecision
         %66 =   OpFOrdEqual %bool %47 %65
                 OpSelectionMerge %68 None
                 OpBranchConditional %66 %67 %68

         %67 =     OpLabel
         %70 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %71 =       OpLoad %v4float %70            ; RelaxedPrecision
         %72 =       OpVectorShuffle %v2float %71 %71 0 1   ; RelaxedPrecision
         %73 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
         %74 =       OpLoad %v4float %73            ; RelaxedPrecision
         %75 =       OpVectorShuffle %v2float %74 %74 0 1   ; RelaxedPrecision
         %76 =       OpVectorShuffle %v2bool %43 %43 0 1
         %78 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %79 =       OpLoad %v4float %78            ; RelaxedPrecision
         %80 =       OpVectorShuffle %v2float %79 %79 0 1   ; RelaxedPrecision
         %81 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
         %82 =       OpLoad %v4float %81            ; RelaxedPrecision
         %83 =       OpVectorShuffle %v2float %82 %82 0 1   ; RelaxedPrecision
         %84 =       OpVectorShuffle %v2bool %43 %43 0 1
         %69 =       OpSelect %v2float %84 %83 %80  ; RelaxedPrecision
         %85 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %86 =       OpLoad %v4float %85            ; RelaxedPrecision
         %87 =       OpCompositeExtract %float %86 0    ; RelaxedPrecision
         %89 =       OpCompositeConstruct %v2float %87 %float_1     ; RelaxedPrecision
         %90 =       OpFOrdEqual %v2bool %69 %89
         %91 =       OpAll %bool %90
                     OpBranch %68

         %68 = OpLabel
         %92 =   OpPhi %bool %false %26 %91 %67
                 OpSelectionMerge %94 None
                 OpBranchConditional %92 %93 %94

         %93 =     OpLabel
         %96 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %97 =       OpLoad %v4float %96            ; RelaxedPrecision
         %98 =       OpVectorShuffle %v3float %97 %97 0 1 2     ; RelaxedPrecision
        %100 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %101 =       OpLoad %v4float %100           ; RelaxedPrecision
        %102 =       OpVectorShuffle %v3float %101 %101 0 1 2   ; RelaxedPrecision
        %103 =       OpVectorShuffle %v3bool %43 %43 0 1 2
        %105 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %106 =       OpLoad %v4float %105           ; RelaxedPrecision
        %107 =       OpVectorShuffle %v3float %106 %106 0 1 2   ; RelaxedPrecision
        %108 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %109 =       OpLoad %v4float %108           ; RelaxedPrecision
        %110 =       OpVectorShuffle %v3float %109 %109 0 1 2   ; RelaxedPrecision
        %111 =       OpVectorShuffle %v3bool %43 %43 0 1 2
         %95 =       OpSelect %v3float %111 %110 %107   ; RelaxedPrecision
        %112 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %113 =       OpLoad %v4float %112           ; RelaxedPrecision
        %114 =       OpCompositeExtract %float %113 0   ; RelaxedPrecision
        %115 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %116 =       OpLoad %v4float %115           ; RelaxedPrecision
        %117 =       OpCompositeExtract %float %116 2   ; RelaxedPrecision
        %118 =       OpCompositeConstruct %v3float %114 %float_1 %117   ; RelaxedPrecision
        %119 =       OpFOrdEqual %v3bool %95 %118
        %120 =       OpAll %bool %119
                     OpBranch %94

         %94 = OpLabel
        %121 =   OpPhi %bool %false %68 %120 %93
                 OpSelectionMerge %123 None
                 OpBranchConditional %121 %122 %123

        %122 =     OpLabel
        %125 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %126 =       OpLoad %v4float %125           ; RelaxedPrecision
        %127 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %128 =       OpLoad %v4float %127           ; RelaxedPrecision
        %129 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %130 =       OpLoad %v4float %129           ; RelaxedPrecision
        %131 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %132 =       OpLoad %v4float %131           ; RelaxedPrecision
        %124 =       OpSelect %v4float %43 %132 %130    ; RelaxedPrecision
        %133 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %134 =       OpLoad %v4float %133           ; RelaxedPrecision
        %135 =       OpCompositeExtract %float %134 0   ; RelaxedPrecision
        %136 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %137 =       OpLoad %v4float %136           ; RelaxedPrecision
        %138 =       OpCompositeExtract %float %137 2   ; RelaxedPrecision
        %139 =       OpCompositeConstruct %v4float %135 %float_1 %138 %float_1  ; RelaxedPrecision
        %140 =       OpFOrdEqual %v4bool %124 %139
        %141 =       OpAll %bool %140
                     OpBranch %123

        %123 = OpLabel
        %142 =   OpPhi %bool %false %94 %141 %122
                 OpSelectionMerge %144 None
                 OpBranchConditional %142 %143 %144

        %143 =     OpLabel
        %146 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %147 =       OpLoad %v4float %146           ; RelaxedPrecision
        %148 =       OpCompositeExtract %float %147 0   ; RelaxedPrecision
        %149 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %151 =       OpLoad %v4float %149           ; RelaxedPrecision
        %152 =       OpCompositeExtract %float %151 0   ; RelaxedPrecision
        %153 =       OpCompositeExtract %bool %45 0
        %154 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %155 =       OpLoad %v4float %154           ; RelaxedPrecision
        %156 =       OpCompositeExtract %float %155 0   ; RelaxedPrecision
        %157 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %158 =       OpLoad %v4float %157           ; RelaxedPrecision
        %159 =       OpCompositeExtract %float %158 0   ; RelaxedPrecision
        %145 =       OpSelect %float %153 %159 %156     ; RelaxedPrecision
        %160 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %161 =       OpLoad %v4float %160           ; RelaxedPrecision
        %162 =       OpCompositeExtract %float %161 0   ; RelaxedPrecision
        %163 =       OpFOrdEqual %bool %145 %162
                     OpBranch %144

        %144 = OpLabel
        %164 =   OpPhi %bool %false %123 %163 %143
                 OpSelectionMerge %166 None
                 OpBranchConditional %164 %165 %166

        %165 =     OpLabel
        %168 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %169 =       OpLoad %v4float %168           ; RelaxedPrecision
        %170 =       OpVectorShuffle %v2float %169 %169 0 1     ; RelaxedPrecision
        %171 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %172 =       OpLoad %v4float %171           ; RelaxedPrecision
        %173 =       OpVectorShuffle %v2float %172 %172 0 1     ; RelaxedPrecision
        %174 =       OpVectorShuffle %v2bool %45 %45 0 1
        %175 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %176 =       OpLoad %v4float %175           ; RelaxedPrecision
        %177 =       OpVectorShuffle %v2float %176 %176 0 1     ; RelaxedPrecision
        %178 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %179 =       OpLoad %v4float %178           ; RelaxedPrecision
        %180 =       OpVectorShuffle %v2float %179 %179 0 1     ; RelaxedPrecision
        %181 =       OpVectorShuffle %v2bool %45 %45 0 1
        %167 =       OpSelect %v2float %181 %180 %177   ; RelaxedPrecision
        %182 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %183 =       OpLoad %v4float %182           ; RelaxedPrecision
        %184 =       OpCompositeExtract %float %183 0   ; RelaxedPrecision
        %185 =       OpCompositeConstruct %v2float %184 %float_1    ; RelaxedPrecision
        %186 =       OpFOrdEqual %v2bool %167 %185
        %187 =       OpAll %bool %186
                     OpBranch %166

        %166 = OpLabel
        %188 =   OpPhi %bool %false %144 %187 %165
                 OpSelectionMerge %190 None
                 OpBranchConditional %188 %189 %190

        %189 =     OpLabel
        %192 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %193 =       OpLoad %v4float %192           ; RelaxedPrecision
        %194 =       OpVectorShuffle %v3float %193 %193 0 1 2   ; RelaxedPrecision
        %195 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %196 =       OpLoad %v4float %195           ; RelaxedPrecision
        %197 =       OpVectorShuffle %v3float %196 %196 0 1 2   ; RelaxedPrecision
        %198 =       OpVectorShuffle %v3bool %45 %45 0 1 2
        %199 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %200 =       OpLoad %v4float %199           ; RelaxedPrecision
        %201 =       OpVectorShuffle %v3float %200 %200 0 1 2   ; RelaxedPrecision
        %202 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %203 =       OpLoad %v4float %202           ; RelaxedPrecision
        %204 =       OpVectorShuffle %v3float %203 %203 0 1 2   ; RelaxedPrecision
        %205 =       OpVectorShuffle %v3bool %45 %45 0 1 2
        %191 =       OpSelect %v3float %205 %204 %201   ; RelaxedPrecision
        %206 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %207 =       OpLoad %v4float %206           ; RelaxedPrecision
        %208 =       OpCompositeExtract %float %207 0   ; RelaxedPrecision
        %209 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %210 =       OpLoad %v4float %209           ; RelaxedPrecision
        %211 =       OpCompositeExtract %float %210 2   ; RelaxedPrecision
        %212 =       OpCompositeConstruct %v3float %208 %float_1 %211   ; RelaxedPrecision
        %213 =       OpFOrdEqual %v3bool %191 %212
        %214 =       OpAll %bool %213
                     OpBranch %190

        %190 = OpLabel
        %215 =   OpPhi %bool %false %166 %214 %189
                 OpSelectionMerge %217 None
                 OpBranchConditional %215 %216 %217

        %216 =     OpLabel
        %219 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %220 =       OpLoad %v4float %219           ; RelaxedPrecision
        %221 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %222 =       OpLoad %v4float %221           ; RelaxedPrecision
        %223 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %224 =       OpLoad %v4float %223           ; RelaxedPrecision
        %225 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %226 =       OpLoad %v4float %225           ; RelaxedPrecision
        %218 =       OpSelect %v4float %45 %226 %224    ; RelaxedPrecision
        %227 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %228 =       OpLoad %v4float %227           ; RelaxedPrecision
        %229 =       OpCompositeExtract %float %228 0   ; RelaxedPrecision
        %230 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_4
        %231 =       OpLoad %v4float %230           ; RelaxedPrecision
        %232 =       OpCompositeExtract %float %231 2   ; RelaxedPrecision
        %233 =       OpCompositeConstruct %v4float %229 %float_1 %232 %float_1  ; RelaxedPrecision
        %234 =       OpFOrdEqual %v4bool %218 %233
        %235 =       OpAll %bool %234
                     OpBranch %217

        %217 = OpLabel
        %236 =   OpPhi %bool %false %190 %235 %216
                 OpSelectionMerge %241 None
                 OpBranchConditional %236 %239 %240

        %239 =     OpLabel
        %242 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %243 =       OpLoad %v4float %242           ; RelaxedPrecision
                     OpStore %237 %243
                     OpBranch %241

        %240 =     OpLabel
        %244 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %246 =       OpLoad %v4float %244           ; RelaxedPrecision
                     OpStore %237 %246
                     OpBranch %241

        %241 = OpLabel
        %247 =   OpLoad %v4float %237               ; RelaxedPrecision
                 OpReturnValue %247
               OpFunctionEnd
