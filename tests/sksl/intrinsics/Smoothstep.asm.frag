               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "testInputs"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %expectedA "expectedA"            ; id %27
               OpName %expectedB "expectedB"            ; id %32

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
               OpDecorate %expectedA RelaxedPrecision
               OpDecorate %expectedB RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %131 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
               OpDecorate %163 RelaxedPrecision
               OpDecorate %164 RelaxedPrecision
               OpDecorate %166 RelaxedPrecision
               OpDecorate %167 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %188 RelaxedPrecision
               OpDecorate %190 RelaxedPrecision
               OpDecorate %191 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %198 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
               OpDecorate %202 RelaxedPrecision
               OpDecorate %211 RelaxedPrecision
               OpDecorate %213 RelaxedPrecision
               OpDecorate %214 RelaxedPrecision

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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0_84375 = OpConstant %float 0.84375
    %float_1 = OpConstant %float 1
         %31 = OpConstantComposite %v4float %float_0 %float_0 %float_0_84375 %float_1
         %33 = OpConstantComposite %v4float %float_1 %float_0 %float_1 %float_1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
         %47 = OpConstantComposite %v3float %float_0 %float_0 %float_0_84375
     %v3bool = OpTypeVector %bool 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_2 = OpConstant %int 2
      %int_1 = OpConstant %int 1
%float_n1_25 = OpConstant %float -1.25
        %100 = OpConstantComposite %v2float %float_n1_25 %float_0
 %float_0_75 = OpConstant %float 0.75
        %117 = OpConstantComposite %v3float %float_n1_25 %float_0 %float_0_75
 %float_2_25 = OpConstant %float 2.25
        %134 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
     %v4bool = OpTypeVector %bool 4
        %144 = OpConstantComposite %v2float %float_1 %float_0
        %151 = OpConstantComposite %v3float %float_1 %float_0 %float_1


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
  %expectedA =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
  %expectedB =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
        %206 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %expectedA %31
                 OpStore %expectedB %33
                 OpSelectionMerge %38 None
                 OpBranchConditional %true %37 %38

         %37 =     OpLabel
         %39 =       OpVectorShuffle %v2float %31 %31 0 1   ; RelaxedPrecision
         %40 =       OpFOrdEqual %v2bool %20 %39
         %42 =       OpAll %bool %40
                     OpBranch %38

         %38 = OpLabel
         %43 =   OpPhi %bool %false %26 %42 %37
                 OpSelectionMerge %45 None
                 OpBranchConditional %43 %44 %45

         %44 =     OpLabel
         %48 =       OpVectorShuffle %v3float %31 %31 0 1 2     ; RelaxedPrecision
         %49 =       OpFOrdEqual %v3bool %47 %48
         %51 =       OpAll %bool %49
                     OpBranch %45

         %45 = OpLabel
         %52 =   OpPhi %bool %false %38 %51 %44
                 OpSelectionMerge %54 None
                 OpBranchConditional %52 %53 %54

         %53 =     OpLabel
                     OpBranch %54

         %54 = OpLabel
         %55 =   OpPhi %bool %false %45 %true %53
                 OpSelectionMerge %57 None
                 OpBranchConditional %55 %56 %57

         %56 =     OpLabel
                     OpBranch %57

         %57 = OpLabel
         %58 =   OpPhi %bool %false %54 %true %56
                 OpSelectionMerge %60 None
                 OpBranchConditional %58 %59 %60

         %59 =     OpLabel
         %61 =       OpVectorShuffle %v2float %31 %31 0 1   ; RelaxedPrecision
         %62 =       OpFOrdEqual %v2bool %20 %61
         %63 =       OpAll %bool %62
                     OpBranch %60

         %60 = OpLabel
         %64 =   OpPhi %bool %false %57 %63 %59
                 OpSelectionMerge %66 None
                 OpBranchConditional %64 %65 %66

         %65 =     OpLabel
         %67 =       OpVectorShuffle %v3float %31 %31 0 1 2     ; RelaxedPrecision
         %68 =       OpFOrdEqual %v3bool %47 %67
         %69 =       OpAll %bool %68
                     OpBranch %66

         %66 = OpLabel
         %70 =   OpPhi %bool %false %60 %69 %65
                 OpSelectionMerge %72 None
                 OpBranchConditional %70 %71 %72

         %71 =     OpLabel
                     OpBranch %72

         %72 = OpLabel
         %73 =   OpPhi %bool %false %66 %true %71
                 OpSelectionMerge %75 None
                 OpBranchConditional %73 %74 %75

         %74 =     OpLabel
         %77 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %80 =       OpLoad %v4float %77            ; RelaxedPrecision
         %81 =       OpCompositeExtract %float %80 1    ; RelaxedPrecision
         %82 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %84 =       OpLoad %v4float %82            ; RelaxedPrecision
         %85 =       OpCompositeExtract %float %84 1    ; RelaxedPrecision
         %76 =       OpExtInst %float %5 SmoothStep %81 %85 %float_n1_25    ; RelaxedPrecision
         %87 =       OpFOrdEqual %bool %76 %float_0
                     OpBranch %75

         %75 = OpLabel
         %88 =   OpPhi %bool %false %72 %87 %74
                 OpSelectionMerge %90 None
                 OpBranchConditional %88 %89 %90

         %89 =     OpLabel
         %92 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %93 =       OpLoad %v4float %92            ; RelaxedPrecision
         %94 =       OpCompositeExtract %float %93 1    ; RelaxedPrecision
         %95 =       OpCompositeConstruct %v2float %94 %94  ; RelaxedPrecision
         %96 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %97 =       OpLoad %v4float %96            ; RelaxedPrecision
         %98 =       OpCompositeExtract %float %97 1    ; RelaxedPrecision
         %99 =       OpCompositeConstruct %v2float %98 %98  ; RelaxedPrecision
         %91 =       OpExtInst %v2float %5 SmoothStep %95 %99 %100  ; RelaxedPrecision
        %101 =       OpVectorShuffle %v2float %31 %31 0 1           ; RelaxedPrecision
        %102 =       OpFOrdEqual %v2bool %91 %101
        %103 =       OpAll %bool %102
                     OpBranch %90

         %90 = OpLabel
        %104 =   OpPhi %bool %false %75 %103 %89
                 OpSelectionMerge %106 None
                 OpBranchConditional %104 %105 %106

        %105 =     OpLabel
        %108 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %109 =       OpLoad %v4float %108           ; RelaxedPrecision
        %110 =       OpCompositeExtract %float %109 1   ; RelaxedPrecision
        %111 =       OpCompositeConstruct %v3float %110 %110 %110   ; RelaxedPrecision
        %112 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %113 =       OpLoad %v4float %112           ; RelaxedPrecision
        %114 =       OpCompositeExtract %float %113 1   ; RelaxedPrecision
        %115 =       OpCompositeConstruct %v3float %114 %114 %114   ; RelaxedPrecision
        %107 =       OpExtInst %v3float %5 SmoothStep %111 %115 %117    ; RelaxedPrecision
        %118 =       OpVectorShuffle %v3float %31 %31 0 1 2             ; RelaxedPrecision
        %119 =       OpFOrdEqual %v3bool %107 %118
        %120 =       OpAll %bool %119
                     OpBranch %106

        %106 = OpLabel
        %121 =   OpPhi %bool %false %90 %120 %105
                 OpSelectionMerge %123 None
                 OpBranchConditional %121 %122 %123

        %122 =     OpLabel
        %125 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %126 =       OpLoad %v4float %125           ; RelaxedPrecision
        %127 =       OpCompositeExtract %float %126 1   ; RelaxedPrecision
        %128 =       OpCompositeConstruct %v4float %127 %127 %127 %127  ; RelaxedPrecision
        %129 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %130 =       OpLoad %v4float %129           ; RelaxedPrecision
        %131 =       OpCompositeExtract %float %130 1   ; RelaxedPrecision
        %132 =       OpCompositeConstruct %v4float %131 %131 %131 %131  ; RelaxedPrecision
        %124 =       OpExtInst %v4float %5 SmoothStep %128 %132 %134    ; RelaxedPrecision
        %135 =       OpFOrdEqual %v4bool %124 %31
        %137 =       OpAll %bool %135
                     OpBranch %123

        %123 = OpLabel
        %138 =   OpPhi %bool %false %106 %137 %122
                 OpSelectionMerge %140 None
                 OpBranchConditional %138 %139 %140

        %139 =     OpLabel
                     OpBranch %140

        %140 = OpLabel
        %141 =   OpPhi %bool %false %123 %true %139
                 OpSelectionMerge %143 None
                 OpBranchConditional %141 %142 %143

        %142 =     OpLabel
        %145 =       OpVectorShuffle %v2float %33 %33 0 1   ; RelaxedPrecision
        %146 =       OpFOrdEqual %v2bool %144 %145
        %147 =       OpAll %bool %146
                     OpBranch %143

        %143 = OpLabel
        %148 =   OpPhi %bool %false %140 %147 %142
                 OpSelectionMerge %150 None
                 OpBranchConditional %148 %149 %150

        %149 =     OpLabel
        %152 =       OpVectorShuffle %v3float %33 %33 0 1 2     ; RelaxedPrecision
        %153 =       OpFOrdEqual %v3bool %151 %152
        %154 =       OpAll %bool %153
                     OpBranch %150

        %150 = OpLabel
        %155 =   OpPhi %bool %false %143 %154 %149
                 OpSelectionMerge %157 None
                 OpBranchConditional %155 %156 %157

        %156 =     OpLabel
                     OpBranch %157

        %157 = OpLabel
        %158 =   OpPhi %bool %false %150 %true %156
                 OpSelectionMerge %160 None
                 OpBranchConditional %158 %159 %160

        %159 =     OpLabel
        %162 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %163 =       OpLoad %v4float %162           ; RelaxedPrecision
        %164 =       OpCompositeExtract %float %163 0   ; RelaxedPrecision
        %165 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %166 =       OpLoad %v4float %165           ; RelaxedPrecision
        %167 =       OpCompositeExtract %float %166 0   ; RelaxedPrecision
        %161 =       OpExtInst %float %5 SmoothStep %164 %167 %float_n1_25  ; RelaxedPrecision
        %168 =       OpFOrdEqual %bool %161 %float_1
                     OpBranch %160

        %160 = OpLabel
        %169 =   OpPhi %bool %false %157 %168 %159
                 OpSelectionMerge %171 None
                 OpBranchConditional %169 %170 %171

        %170 =     OpLabel
        %173 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %174 =       OpLoad %v4float %173           ; RelaxedPrecision
        %175 =       OpVectorShuffle %v2float %174 %174 0 1     ; RelaxedPrecision
        %176 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %177 =       OpLoad %v4float %176           ; RelaxedPrecision
        %178 =       OpVectorShuffle %v2float %177 %177 0 1     ; RelaxedPrecision
        %172 =       OpExtInst %v2float %5 SmoothStep %175 %178 %100    ; RelaxedPrecision
        %179 =       OpVectorShuffle %v2float %33 %33 0 1               ; RelaxedPrecision
        %180 =       OpFOrdEqual %v2bool %172 %179
        %181 =       OpAll %bool %180
                     OpBranch %171

        %171 = OpLabel
        %182 =   OpPhi %bool %false %160 %181 %170
                 OpSelectionMerge %184 None
                 OpBranchConditional %182 %183 %184

        %183 =     OpLabel
        %186 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %187 =       OpLoad %v4float %186           ; RelaxedPrecision
        %188 =       OpVectorShuffle %v3float %187 %187 0 1 2   ; RelaxedPrecision
        %189 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %190 =       OpLoad %v4float %189           ; RelaxedPrecision
        %191 =       OpVectorShuffle %v3float %190 %190 0 1 2   ; RelaxedPrecision
        %185 =       OpExtInst %v3float %5 SmoothStep %188 %191 %117    ; RelaxedPrecision
        %192 =       OpVectorShuffle %v3float %33 %33 0 1 2             ; RelaxedPrecision
        %193 =       OpFOrdEqual %v3bool %185 %192
        %194 =       OpAll %bool %193
                     OpBranch %184

        %184 = OpLabel
        %195 =   OpPhi %bool %false %171 %194 %183
                 OpSelectionMerge %197 None
                 OpBranchConditional %195 %196 %197

        %196 =     OpLabel
        %199 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %200 =       OpLoad %v4float %199           ; RelaxedPrecision
        %201 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %202 =       OpLoad %v4float %201           ; RelaxedPrecision
        %198 =       OpExtInst %v4float %5 SmoothStep %200 %202 %134    ; RelaxedPrecision
        %203 =       OpFOrdEqual %v4bool %198 %33
        %204 =       OpAll %bool %203
                     OpBranch %197

        %197 = OpLabel
        %205 =   OpPhi %bool %false %184 %204 %196
                 OpSelectionMerge %209 None
                 OpBranchConditional %205 %207 %208

        %207 =     OpLabel
        %210 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %211 =       OpLoad %v4float %210           ; RelaxedPrecision
                     OpStore %206 %211
                     OpBranch %209

        %208 =     OpLabel
        %212 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %213 =       OpLoad %v4float %212           ; RelaxedPrecision
                     OpStore %206 %213
                     OpBranch %209

        %209 = OpLabel
        %214 =   OpLoad %v4float %206               ; RelaxedPrecision
                 OpReturnValue %214
               OpFunctionEnd
