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
               OpName %expectedB "expectedB"            ; id %31
               OpName %expectedC "expectedC"            ; id %33

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
               OpDecorate %expectedC RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %145 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
               OpDecorate %164 RelaxedPrecision
               OpDecorate %165 RelaxedPrecision
               OpDecorate %168 RelaxedPrecision
               OpDecorate %169 RelaxedPrecision
               OpDecorate %174 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %189 RelaxedPrecision
               OpDecorate %190 RelaxedPrecision
               OpDecorate %192 RelaxedPrecision
               OpDecorate %193 RelaxedPrecision
               OpDecorate %194 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
               OpDecorate %202 RelaxedPrecision
               OpDecorate %204 RelaxedPrecision
               OpDecorate %213 RelaxedPrecision
               OpDecorate %220 RelaxedPrecision
               OpDecorate %232 RelaxedPrecision
               OpDecorate %234 RelaxedPrecision
               OpDecorate %235 RelaxedPrecision

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
    %float_1 = OpConstant %float 1
         %30 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_1
         %32 = OpConstantComposite %v4float %float_1 %float_1 %float_0 %float_0
         %34 = OpConstantComposite %v4float %float_0 %float_1 %float_1 %float_1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
  %float_0_5 = OpConstant %float 0.5
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
         %48 = OpConstantComposite %v2float %float_0_5 %float_0_5
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
         %61 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
     %v3bool = OpTypeVector %bool 3
         %73 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
         %92 = OpConstantComposite %v3float %float_0 %float_0 %float_1
        %114 = OpConstantComposite %v2float %float_0 %float_1
        %125 = OpConstantComposite %v3float %float_0 %float_1 %float_0
        %135 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
        %144 = OpConstantComposite %v2float %float_1 %float_1
        %151 = OpConstantComposite %v3float %float_1 %float_1 %float_0
      %int_2 = OpConstant %int 2
      %int_1 = OpConstant %int 1
        %219 = OpConstantComposite %v3float %float_0 %float_1 %float_1


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
  %expectedC =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
        %227 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %expectedA %30
                 OpStore %expectedB %32
                 OpStore %expectedC %34
         %39 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %42 =   OpLoad %v4float %39                ; RelaxedPrecision
         %43 =   OpCompositeExtract %float %42 0    ; RelaxedPrecision
         %37 =   OpExtInst %float %5 Step %float_0_5 %43    ; RelaxedPrecision
         %44 =   OpFOrdEqual %bool %37 %float_0
                 OpSelectionMerge %46 None
                 OpBranchConditional %44 %45 %46

         %45 =     OpLabel
         %49 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %50 =       OpLoad %v4float %49            ; RelaxedPrecision
         %51 =       OpVectorShuffle %v2float %50 %50 0 1   ; RelaxedPrecision
         %47 =       OpExtInst %v2float %5 Step %48 %51     ; RelaxedPrecision
         %52 =       OpVectorShuffle %v2float %30 %30 0 1   ; RelaxedPrecision
         %53 =       OpFOrdEqual %v2bool %47 %52
         %55 =       OpAll %bool %53
                     OpBranch %46

         %46 = OpLabel
         %56 =   OpPhi %bool %false %26 %55 %45
                 OpSelectionMerge %58 None
                 OpBranchConditional %56 %57 %58

         %57 =     OpLabel
         %62 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %63 =       OpLoad %v4float %62            ; RelaxedPrecision
         %64 =       OpVectorShuffle %v3float %63 %63 0 1 2     ; RelaxedPrecision
         %59 =       OpExtInst %v3float %5 Step %61 %64         ; RelaxedPrecision
         %65 =       OpVectorShuffle %v3float %30 %30 0 1 2     ; RelaxedPrecision
         %66 =       OpFOrdEqual %v3bool %59 %65
         %68 =       OpAll %bool %66
                     OpBranch %58

         %58 = OpLabel
         %69 =   OpPhi %bool %false %46 %68 %57
                 OpSelectionMerge %71 None
                 OpBranchConditional %69 %70 %71

         %70 =     OpLabel
         %74 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %75 =       OpLoad %v4float %74            ; RelaxedPrecision
         %72 =       OpExtInst %v4float %5 Step %73 %75     ; RelaxedPrecision
         %76 =       OpFOrdEqual %v4bool %72 %30
         %78 =       OpAll %bool %76
                     OpBranch %71

         %71 = OpLabel
         %79 =   OpPhi %bool %false %58 %78 %70
                 OpSelectionMerge %81 None
                 OpBranchConditional %79 %80 %81

         %80 =     OpLabel
                     OpBranch %81

         %81 = OpLabel
         %83 =   OpPhi %bool %false %71 %true %80
                 OpSelectionMerge %85 None
                 OpBranchConditional %83 %84 %85

         %84 =     OpLabel
         %86 =       OpVectorShuffle %v2float %30 %30 0 1   ; RelaxedPrecision
         %87 =       OpFOrdEqual %v2bool %20 %86
         %88 =       OpAll %bool %87
                     OpBranch %85

         %85 = OpLabel
         %89 =   OpPhi %bool %false %81 %88 %84
                 OpSelectionMerge %91 None
                 OpBranchConditional %89 %90 %91

         %90 =     OpLabel
         %93 =       OpVectorShuffle %v3float %30 %30 0 1 2     ; RelaxedPrecision
         %94 =       OpFOrdEqual %v3bool %92 %93
         %95 =       OpAll %bool %94
                     OpBranch %91

         %91 = OpLabel
         %96 =   OpPhi %bool %false %85 %95 %90
                 OpSelectionMerge %98 None
                 OpBranchConditional %96 %97 %98

         %97 =     OpLabel
                     OpBranch %98

         %98 = OpLabel
         %99 =   OpPhi %bool %false %91 %true %97
                 OpSelectionMerge %101 None
                 OpBranchConditional %99 %100 %101

        %100 =     OpLabel
        %103 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %104 =       OpLoad %v4float %103           ; RelaxedPrecision
        %105 =       OpCompositeExtract %float %104 0   ; RelaxedPrecision
        %102 =       OpExtInst %float %5 Step %105 %float_0     ; RelaxedPrecision
        %106 =       OpFOrdEqual %bool %102 %float_1
                     OpBranch %101

        %101 = OpLabel
        %107 =   OpPhi %bool %false %98 %106 %100
                 OpSelectionMerge %109 None
                 OpBranchConditional %107 %108 %109

        %108 =     OpLabel
        %111 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %112 =       OpLoad %v4float %111           ; RelaxedPrecision
        %113 =       OpVectorShuffle %v2float %112 %112 0 1     ; RelaxedPrecision
        %110 =       OpExtInst %v2float %5 Step %113 %114       ; RelaxedPrecision
        %115 =       OpVectorShuffle %v2float %32 %32 0 1       ; RelaxedPrecision
        %116 =       OpFOrdEqual %v2bool %110 %115
        %117 =       OpAll %bool %116
                     OpBranch %109

        %109 = OpLabel
        %118 =   OpPhi %bool %false %101 %117 %108
                 OpSelectionMerge %120 None
                 OpBranchConditional %118 %119 %120

        %119 =     OpLabel
        %122 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %123 =       OpLoad %v4float %122           ; RelaxedPrecision
        %124 =       OpVectorShuffle %v3float %123 %123 0 1 2   ; RelaxedPrecision
        %121 =       OpExtInst %v3float %5 Step %124 %125       ; RelaxedPrecision
        %126 =       OpVectorShuffle %v3float %32 %32 0 1 2     ; RelaxedPrecision
        %127 =       OpFOrdEqual %v3bool %121 %126
        %128 =       OpAll %bool %127
                     OpBranch %120

        %120 = OpLabel
        %129 =   OpPhi %bool %false %109 %128 %119
                 OpSelectionMerge %131 None
                 OpBranchConditional %129 %130 %131

        %130 =     OpLabel
        %133 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %134 =       OpLoad %v4float %133           ; RelaxedPrecision
        %132 =       OpExtInst %v4float %5 Step %134 %135   ; RelaxedPrecision
        %136 =       OpFOrdEqual %v4bool %132 %32
        %137 =       OpAll %bool %136
                     OpBranch %131

        %131 = OpLabel
        %138 =   OpPhi %bool %false %120 %137 %130
                 OpSelectionMerge %140 None
                 OpBranchConditional %138 %139 %140

        %139 =     OpLabel
                     OpBranch %140

        %140 = OpLabel
        %141 =   OpPhi %bool %false %131 %true %139
                 OpSelectionMerge %143 None
                 OpBranchConditional %141 %142 %143

        %142 =     OpLabel
        %145 =       OpVectorShuffle %v2float %32 %32 0 1   ; RelaxedPrecision
        %146 =       OpFOrdEqual %v2bool %144 %145
        %147 =       OpAll %bool %146
                     OpBranch %143

        %143 = OpLabel
        %148 =   OpPhi %bool %false %140 %147 %142
                 OpSelectionMerge %150 None
                 OpBranchConditional %148 %149 %150

        %149 =     OpLabel
        %152 =       OpVectorShuffle %v3float %32 %32 0 1 2     ; RelaxedPrecision
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
        %164 =       OpLoad %v4float %162           ; RelaxedPrecision
        %165 =       OpCompositeExtract %float %164 0   ; RelaxedPrecision
        %166 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %168 =       OpLoad %v4float %166           ; RelaxedPrecision
        %169 =       OpCompositeExtract %float %168 0   ; RelaxedPrecision
        %161 =       OpExtInst %float %5 Step %165 %169     ; RelaxedPrecision
        %170 =       OpFOrdEqual %bool %161 %float_0
                     OpBranch %160

        %160 = OpLabel
        %171 =   OpPhi %bool %false %157 %170 %159
                 OpSelectionMerge %173 None
                 OpBranchConditional %171 %172 %173

        %172 =     OpLabel
        %175 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %176 =       OpLoad %v4float %175           ; RelaxedPrecision
        %177 =       OpVectorShuffle %v2float %176 %176 0 1     ; RelaxedPrecision
        %178 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %179 =       OpLoad %v4float %178           ; RelaxedPrecision
        %180 =       OpVectorShuffle %v2float %179 %179 0 1     ; RelaxedPrecision
        %174 =       OpExtInst %v2float %5 Step %177 %180       ; RelaxedPrecision
        %181 =       OpVectorShuffle %v2float %34 %34 0 1       ; RelaxedPrecision
        %182 =       OpFOrdEqual %v2bool %174 %181
        %183 =       OpAll %bool %182
                     OpBranch %173

        %173 = OpLabel
        %184 =   OpPhi %bool %false %160 %183 %172
                 OpSelectionMerge %186 None
                 OpBranchConditional %184 %185 %186

        %185 =     OpLabel
        %188 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %189 =       OpLoad %v4float %188           ; RelaxedPrecision
        %190 =       OpVectorShuffle %v3float %189 %189 0 1 2   ; RelaxedPrecision
        %191 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %192 =       OpLoad %v4float %191           ; RelaxedPrecision
        %193 =       OpVectorShuffle %v3float %192 %192 0 1 2   ; RelaxedPrecision
        %187 =       OpExtInst %v3float %5 Step %190 %193       ; RelaxedPrecision
        %194 =       OpVectorShuffle %v3float %34 %34 0 1 2     ; RelaxedPrecision
        %195 =       OpFOrdEqual %v3bool %187 %194
        %196 =       OpAll %bool %195
                     OpBranch %186

        %186 = OpLabel
        %197 =   OpPhi %bool %false %173 %196 %185
                 OpSelectionMerge %199 None
                 OpBranchConditional %197 %198 %199

        %198 =     OpLabel
        %201 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %202 =       OpLoad %v4float %201           ; RelaxedPrecision
        %203 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %204 =       OpLoad %v4float %203           ; RelaxedPrecision
        %200 =       OpExtInst %v4float %5 Step %202 %204   ; RelaxedPrecision
        %205 =       OpFOrdEqual %v4bool %200 %34
        %206 =       OpAll %bool %205
                     OpBranch %199

        %199 = OpLabel
        %207 =   OpPhi %bool %false %186 %206 %198
                 OpSelectionMerge %209 None
                 OpBranchConditional %207 %208 %209

        %208 =     OpLabel
                     OpBranch %209

        %209 = OpLabel
        %210 =   OpPhi %bool %false %199 %true %208
                 OpSelectionMerge %212 None
                 OpBranchConditional %210 %211 %212

        %211 =     OpLabel
        %213 =       OpVectorShuffle %v2float %34 %34 0 1   ; RelaxedPrecision
        %214 =       OpFOrdEqual %v2bool %114 %213
        %215 =       OpAll %bool %214
                     OpBranch %212

        %212 = OpLabel
        %216 =   OpPhi %bool %false %209 %215 %211
                 OpSelectionMerge %218 None
                 OpBranchConditional %216 %217 %218

        %217 =     OpLabel
        %220 =       OpVectorShuffle %v3float %34 %34 0 1 2     ; RelaxedPrecision
        %221 =       OpFOrdEqual %v3bool %219 %220
        %222 =       OpAll %bool %221
                     OpBranch %218

        %218 = OpLabel
        %223 =   OpPhi %bool %false %212 %222 %217
                 OpSelectionMerge %225 None
                 OpBranchConditional %223 %224 %225

        %224 =     OpLabel
                     OpBranch %225

        %225 = OpLabel
        %226 =   OpPhi %bool %false %218 %true %224
                 OpSelectionMerge %230 None
                 OpBranchConditional %226 %228 %229

        %228 =     OpLabel
        %231 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %232 =       OpLoad %v4float %231           ; RelaxedPrecision
                     OpStore %227 %232
                     OpBranch %230

        %229 =     OpLabel
        %233 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %234 =       OpLoad %v4float %233           ; RelaxedPrecision
                     OpStore %227 %234
                     OpBranch %230

        %230 = OpLabel
        %235 =   OpLoad %v4float %227               ; RelaxedPrecision
                 OpReturnValue %235
               OpFunctionEnd
