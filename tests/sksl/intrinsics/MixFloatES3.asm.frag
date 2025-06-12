               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %_UniformBuffer "_UniformBuffer"  ; id %8
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "colorBlack"
               OpMemberName %_UniformBuffer 3 "colorWhite"
               OpMemberName %_UniformBuffer 4 "testInputs"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %10
               OpName %main "main"                      ; id %2
               OpName %FTFT "FTFT"                      ; id %23
               OpName %TFTF "TFTF"                      ; id %41

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
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %156 RelaxedPrecision
               OpDecorate %158 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %164 RelaxedPrecision
               OpDecorate %166 RelaxedPrecision
               OpDecorate %167 RelaxedPrecision
               OpDecorate %169 RelaxedPrecision
               OpDecorate %170 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision
               OpDecorate %188 RelaxedPrecision
               OpDecorate %190 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %194 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %198 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
               OpDecorate %201 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
               OpDecorate %205 RelaxedPrecision
               OpDecorate %207 RelaxedPrecision
               OpDecorate %208 RelaxedPrecision
               OpDecorate %209 RelaxedPrecision
               OpDecorate %215 RelaxedPrecision
               OpDecorate %217 RelaxedPrecision
               OpDecorate %219 RelaxedPrecision
               OpDecorate %221 RelaxedPrecision
               OpDecorate %223 RelaxedPrecision
               OpDecorate %225 RelaxedPrecision
               OpDecorate %226 RelaxedPrecision
               OpDecorate %228 RelaxedPrecision
               OpDecorate %229 RelaxedPrecision
               OpDecorate %230 RelaxedPrecision
               OpDecorate %240 RelaxedPrecision
               OpDecorate %243 RelaxedPrecision
               OpDecorate %244 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float %v4float     ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
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
%_entrypoint_v = OpFunction %void None %12

         %13 = OpLabel
         %17 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %17 %16
         %19 =   OpFunctionCall %v4float %main %17
                 OpStore %sk_FragColor %19
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %20         ; RelaxedPrecision
         %21 = OpFunctionParameter %_ptr_Function_v2float

         %22 = OpLabel
       %FTFT =   OpVariable %_ptr_Function_v4bool Function
       %TFTF =   OpVariable %_ptr_Function_v4bool Function
        %234 =   OpVariable %_ptr_Function_v4float Function
         %27 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %31 =   OpLoad %v4float %27                ; RelaxedPrecision
         %32 =   OpCompositeExtract %float %31 0    ; RelaxedPrecision
         %33 =   OpFUnordNotEqual %bool %32 %float_0
         %34 =   OpCompositeExtract %float %31 1    ; RelaxedPrecision
         %35 =   OpFUnordNotEqual %bool %34 %float_0
         %36 =   OpCompositeExtract %float %31 2    ; RelaxedPrecision
         %37 =   OpFUnordNotEqual %bool %36 %float_0
         %38 =   OpCompositeExtract %float %31 3    ; RelaxedPrecision
         %39 =   OpFUnordNotEqual %bool %38 %float_0
         %40 =   OpCompositeConstruct %v4bool %33 %35 %37 %39
                 OpStore %FTFT %40
         %42 =   OpVectorShuffle %v4bool %40 %40 3 2 1 0
                 OpStore %TFTF %42
         %45 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %47 =   OpLoad %v4float %45                ; RelaxedPrecision
         %48 =   OpCompositeExtract %float %47 0    ; RelaxedPrecision
         %49 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_3
         %51 =   OpLoad %v4float %49                ; RelaxedPrecision
         %52 =   OpCompositeExtract %float %51 0    ; RelaxedPrecision
         %53 =   OpCompositeExtract %bool %40 0
         %54 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %55 =   OpLoad %v4float %54                ; RelaxedPrecision
         %56 =   OpCompositeExtract %float %55 0    ; RelaxedPrecision
         %57 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_3
         %58 =   OpLoad %v4float %57                ; RelaxedPrecision
         %59 =   OpCompositeExtract %float %58 0    ; RelaxedPrecision
         %44 =   OpSelect %float %53 %59 %56        ; RelaxedPrecision
         %60 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %61 =   OpLoad %v4float %60                ; RelaxedPrecision
         %62 =   OpCompositeExtract %float %61 0    ; RelaxedPrecision
         %63 =   OpFOrdEqual %bool %44 %62
                 OpSelectionMerge %65 None
                 OpBranchConditional %63 %64 %65

         %64 =     OpLabel
         %67 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %68 =       OpLoad %v4float %67            ; RelaxedPrecision
         %69 =       OpVectorShuffle %v2float %68 %68 0 1   ; RelaxedPrecision
         %70 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_3
         %71 =       OpLoad %v4float %70            ; RelaxedPrecision
         %72 =       OpVectorShuffle %v2float %71 %71 0 1   ; RelaxedPrecision
         %73 =       OpVectorShuffle %v2bool %40 %40 0 1
         %75 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %76 =       OpLoad %v4float %75            ; RelaxedPrecision
         %77 =       OpVectorShuffle %v2float %76 %76 0 1   ; RelaxedPrecision
         %78 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_3
         %79 =       OpLoad %v4float %78            ; RelaxedPrecision
         %80 =       OpVectorShuffle %v2float %79 %79 0 1   ; RelaxedPrecision
         %81 =       OpVectorShuffle %v2bool %40 %40 0 1
         %66 =       OpSelect %v2float %81 %80 %77  ; RelaxedPrecision
         %82 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %83 =       OpLoad %v4float %82            ; RelaxedPrecision
         %84 =       OpCompositeExtract %float %83 0    ; RelaxedPrecision
         %86 =       OpCompositeConstruct %v2float %84 %float_1     ; RelaxedPrecision
         %87 =       OpFOrdEqual %v2bool %66 %86
         %88 =       OpAll %bool %87
                     OpBranch %65

         %65 = OpLabel
         %89 =   OpPhi %bool %false %22 %88 %64
                 OpSelectionMerge %91 None
                 OpBranchConditional %89 %90 %91

         %90 =     OpLabel
         %93 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %94 =       OpLoad %v4float %93            ; RelaxedPrecision
         %95 =       OpVectorShuffle %v3float %94 %94 0 1 2     ; RelaxedPrecision
         %97 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_3
         %98 =       OpLoad %v4float %97            ; RelaxedPrecision
         %99 =       OpVectorShuffle %v3float %98 %98 0 1 2     ; RelaxedPrecision
        %100 =       OpVectorShuffle %v3bool %40 %40 0 1 2
        %102 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %103 =       OpLoad %v4float %102           ; RelaxedPrecision
        %104 =       OpVectorShuffle %v3float %103 %103 0 1 2   ; RelaxedPrecision
        %105 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %106 =       OpLoad %v4float %105           ; RelaxedPrecision
        %107 =       OpVectorShuffle %v3float %106 %106 0 1 2   ; RelaxedPrecision
        %108 =       OpVectorShuffle %v3bool %40 %40 0 1 2
         %92 =       OpSelect %v3float %108 %107 %104   ; RelaxedPrecision
        %109 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %110 =       OpLoad %v4float %109           ; RelaxedPrecision
        %111 =       OpCompositeExtract %float %110 0   ; RelaxedPrecision
        %112 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %113 =       OpLoad %v4float %112           ; RelaxedPrecision
        %114 =       OpCompositeExtract %float %113 2   ; RelaxedPrecision
        %115 =       OpCompositeConstruct %v3float %111 %float_1 %114   ; RelaxedPrecision
        %116 =       OpFOrdEqual %v3bool %92 %115
        %117 =       OpAll %bool %116
                     OpBranch %91

         %91 = OpLabel
        %118 =   OpPhi %bool %false %65 %117 %90
                 OpSelectionMerge %120 None
                 OpBranchConditional %118 %119 %120

        %119 =     OpLabel
        %122 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %123 =       OpLoad %v4float %122           ; RelaxedPrecision
        %124 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %125 =       OpLoad %v4float %124           ; RelaxedPrecision
        %126 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %127 =       OpLoad %v4float %126           ; RelaxedPrecision
        %128 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %129 =       OpLoad %v4float %128           ; RelaxedPrecision
        %121 =       OpSelect %v4float %40 %129 %127    ; RelaxedPrecision
        %130 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %131 =       OpLoad %v4float %130           ; RelaxedPrecision
        %132 =       OpCompositeExtract %float %131 0   ; RelaxedPrecision
        %133 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %134 =       OpLoad %v4float %133           ; RelaxedPrecision
        %135 =       OpCompositeExtract %float %134 2   ; RelaxedPrecision
        %136 =       OpCompositeConstruct %v4float %132 %float_1 %135 %float_1  ; RelaxedPrecision
        %137 =       OpFOrdEqual %v4bool %121 %136
        %138 =       OpAll %bool %137
                     OpBranch %120

        %120 = OpLabel
        %139 =   OpPhi %bool %false %91 %138 %119
                 OpSelectionMerge %141 None
                 OpBranchConditional %139 %140 %141

        %140 =     OpLabel
        %143 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %144 =       OpLoad %v4float %143           ; RelaxedPrecision
        %145 =       OpCompositeExtract %float %144 0   ; RelaxedPrecision
        %146 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %148 =       OpLoad %v4float %146           ; RelaxedPrecision
        %149 =       OpCompositeExtract %float %148 0   ; RelaxedPrecision
        %150 =       OpCompositeExtract %bool %42 0
        %151 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %152 =       OpLoad %v4float %151           ; RelaxedPrecision
        %153 =       OpCompositeExtract %float %152 0   ; RelaxedPrecision
        %154 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %155 =       OpLoad %v4float %154           ; RelaxedPrecision
        %156 =       OpCompositeExtract %float %155 0   ; RelaxedPrecision
        %142 =       OpSelect %float %150 %156 %153     ; RelaxedPrecision
        %157 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %158 =       OpLoad %v4float %157           ; RelaxedPrecision
        %159 =       OpCompositeExtract %float %158 0   ; RelaxedPrecision
        %160 =       OpFOrdEqual %bool %142 %159
                     OpBranch %141

        %141 = OpLabel
        %161 =   OpPhi %bool %false %120 %160 %140
                 OpSelectionMerge %163 None
                 OpBranchConditional %161 %162 %163

        %162 =     OpLabel
        %165 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %166 =       OpLoad %v4float %165           ; RelaxedPrecision
        %167 =       OpVectorShuffle %v2float %166 %166 0 1     ; RelaxedPrecision
        %168 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %169 =       OpLoad %v4float %168           ; RelaxedPrecision
        %170 =       OpVectorShuffle %v2float %169 %169 0 1     ; RelaxedPrecision
        %171 =       OpVectorShuffle %v2bool %42 %42 0 1
        %172 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %173 =       OpLoad %v4float %172           ; RelaxedPrecision
        %174 =       OpVectorShuffle %v2float %173 %173 0 1     ; RelaxedPrecision
        %175 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %176 =       OpLoad %v4float %175           ; RelaxedPrecision
        %177 =       OpVectorShuffle %v2float %176 %176 0 1     ; RelaxedPrecision
        %178 =       OpVectorShuffle %v2bool %42 %42 0 1
        %164 =       OpSelect %v2float %178 %177 %174   ; RelaxedPrecision
        %179 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %180 =       OpLoad %v4float %179           ; RelaxedPrecision
        %181 =       OpCompositeExtract %float %180 0   ; RelaxedPrecision
        %182 =       OpCompositeConstruct %v2float %181 %float_1    ; RelaxedPrecision
        %183 =       OpFOrdEqual %v2bool %164 %182
        %184 =       OpAll %bool %183
                     OpBranch %163

        %163 = OpLabel
        %185 =   OpPhi %bool %false %141 %184 %162
                 OpSelectionMerge %187 None
                 OpBranchConditional %185 %186 %187

        %186 =     OpLabel
        %189 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %190 =       OpLoad %v4float %189           ; RelaxedPrecision
        %191 =       OpVectorShuffle %v3float %190 %190 0 1 2   ; RelaxedPrecision
        %192 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %193 =       OpLoad %v4float %192           ; RelaxedPrecision
        %194 =       OpVectorShuffle %v3float %193 %193 0 1 2   ; RelaxedPrecision
        %195 =       OpVectorShuffle %v3bool %42 %42 0 1 2
        %196 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %197 =       OpLoad %v4float %196           ; RelaxedPrecision
        %198 =       OpVectorShuffle %v3float %197 %197 0 1 2   ; RelaxedPrecision
        %199 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %200 =       OpLoad %v4float %199           ; RelaxedPrecision
        %201 =       OpVectorShuffle %v3float %200 %200 0 1 2   ; RelaxedPrecision
        %202 =       OpVectorShuffle %v3bool %42 %42 0 1 2
        %188 =       OpSelect %v3float %202 %201 %198   ; RelaxedPrecision
        %203 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %204 =       OpLoad %v4float %203           ; RelaxedPrecision
        %205 =       OpCompositeExtract %float %204 0   ; RelaxedPrecision
        %206 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %207 =       OpLoad %v4float %206           ; RelaxedPrecision
        %208 =       OpCompositeExtract %float %207 2   ; RelaxedPrecision
        %209 =       OpCompositeConstruct %v3float %205 %float_1 %208   ; RelaxedPrecision
        %210 =       OpFOrdEqual %v3bool %188 %209
        %211 =       OpAll %bool %210
                     OpBranch %187

        %187 = OpLabel
        %212 =   OpPhi %bool %false %163 %211 %186
                 OpSelectionMerge %214 None
                 OpBranchConditional %212 %213 %214

        %213 =     OpLabel
        %216 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %217 =       OpLoad %v4float %216           ; RelaxedPrecision
        %218 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %219 =       OpLoad %v4float %218           ; RelaxedPrecision
        %220 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %221 =       OpLoad %v4float %220           ; RelaxedPrecision
        %222 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %223 =       OpLoad %v4float %222           ; RelaxedPrecision
        %215 =       OpSelect %v4float %42 %223 %221    ; RelaxedPrecision
        %224 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %225 =       OpLoad %v4float %224           ; RelaxedPrecision
        %226 =       OpCompositeExtract %float %225 0   ; RelaxedPrecision
        %227 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_4
        %228 =       OpLoad %v4float %227           ; RelaxedPrecision
        %229 =       OpCompositeExtract %float %228 2   ; RelaxedPrecision
        %230 =       OpCompositeConstruct %v4float %226 %float_1 %229 %float_1  ; RelaxedPrecision
        %231 =       OpFOrdEqual %v4bool %215 %230
        %232 =       OpAll %bool %231
                     OpBranch %214

        %214 = OpLabel
        %233 =   OpPhi %bool %false %187 %232 %213
                 OpSelectionMerge %238 None
                 OpBranchConditional %233 %236 %237

        %236 =     OpLabel
        %239 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %240 =       OpLoad %v4float %239           ; RelaxedPrecision
                     OpStore %234 %240
                     OpBranch %238

        %237 =     OpLabel
        %241 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %243 =       OpLoad %v4float %241           ; RelaxedPrecision
                     OpStore %234 %243
                     OpBranch %238

        %238 = OpLabel
        %244 =   OpLoad %v4float %234               ; RelaxedPrecision
                 OpReturnValue %244
               OpFunctionEnd
