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
               OpName %uintValues "uintValues"          ; id %27
               OpName %expectedA "expectedA"            ; id %49
               OpName %expectedB "expectedB"            ; id %55

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
               OpDecorate %34 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %183 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision

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
       %uint = OpTypeInt 32 0
     %v4uint = OpTypeVector %uint 4
%_ptr_Function_v4uint = OpTypePointer Function %v4uint
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
  %float_100 = OpConstant %float 100
  %float_200 = OpConstant %float 200
         %38 = OpConstantComposite %v4float %float_200 %float_200 %float_200 %float_200
   %uint_100 = OpConstant %uint 100
   %uint_200 = OpConstant %uint 200
   %uint_275 = OpConstant %uint 275
   %uint_300 = OpConstant %uint 300
         %54 = OpConstantComposite %v4uint %uint_100 %uint_200 %uint_275 %uint_300
   %uint_250 = OpConstant %uint 250
   %uint_425 = OpConstant %uint 425
         %58 = OpConstantComposite %v4uint %uint_100 %uint_200 %uint_250 %uint_425
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
     %v2uint = OpTypeVector %uint 2
         %69 = OpConstantComposite %v2uint %uint_100 %uint_100
         %70 = OpConstantComposite %v2uint %uint_300 %uint_300
     %v2bool = OpTypeVector %bool 2
     %v3uint = OpTypeVector %uint 3
         %81 = OpConstantComposite %v3uint %uint_100 %uint_100 %uint_100
         %82 = OpConstantComposite %v3uint %uint_300 %uint_300 %uint_300
     %v3bool = OpTypeVector %bool 3
         %91 = OpConstantComposite %v4uint %uint_100 %uint_100 %uint_100 %uint_100
         %92 = OpConstantComposite %v4uint %uint_300 %uint_300 %uint_300 %uint_300
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
        %103 = OpConstantComposite %v2uint %uint_100 %uint_200
        %110 = OpConstantComposite %v3uint %uint_100 %uint_200 %uint_275
     %uint_0 = OpConstant %uint 0
        %128 = OpConstantComposite %v2uint %uint_100 %uint_0
   %uint_400 = OpConstant %uint 400
        %130 = OpConstantComposite %v2uint %uint_300 %uint_400
        %139 = OpConstantComposite %v3uint %uint_100 %uint_0 %uint_0
        %140 = OpConstantComposite %v3uint %uint_300 %uint_400 %uint_250
        %148 = OpConstantComposite %v4uint %uint_100 %uint_0 %uint_0 %uint_300
   %uint_500 = OpConstant %uint 500
        %150 = OpConstantComposite %v4uint %uint_300 %uint_400 %uint_250 %uint_500
        %165 = OpConstantComposite %v3uint %uint_100 %uint_200 %uint_250
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2


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
 %uintValues =   OpVariable %_ptr_Function_v4uint Function
  %expectedA =   OpVariable %_ptr_Function_v4uint Function
  %expectedB =   OpVariable %_ptr_Function_v4uint Function
        %173 =   OpVariable %_ptr_Function_v4float Function
         %31 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %34 =   OpLoad %v4float %31                ; RelaxedPrecision
         %36 =   OpVectorTimesScalar %v4float %34 %float_100    ; RelaxedPrecision
         %39 =   OpFAdd %v4float %36 %38                        ; RelaxedPrecision
         %40 =   OpCompositeExtract %float %39 0                ; RelaxedPrecision
         %41 =   OpConvertFToU %uint %40
         %42 =   OpCompositeExtract %float %39 1    ; RelaxedPrecision
         %43 =   OpConvertFToU %uint %42
         %44 =   OpCompositeExtract %float %39 2    ; RelaxedPrecision
         %45 =   OpConvertFToU %uint %44
         %46 =   OpCompositeExtract %float %39 3    ; RelaxedPrecision
         %47 =   OpConvertFToU %uint %46
         %48 =   OpCompositeConstruct %v4uint %41 %43 %45 %47
                 OpStore %uintValues %48
                 OpStore %expectedA %54
                 OpStore %expectedB %58
         %62 =   OpCompositeExtract %uint %48 0
         %61 =   OpExtInst %uint %5 UClamp %62 %uint_100 %uint_300
         %63 =   OpIEqual %bool %61 %uint_100
                 OpSelectionMerge %65 None
                 OpBranchConditional %63 %64 %65

         %64 =     OpLabel
         %67 =       OpVectorShuffle %v2uint %48 %48 0 1
         %66 =       OpExtInst %v2uint %5 UClamp %67 %69 %70
         %71 =       OpVectorShuffle %v2uint %54 %54 0 1
         %72 =       OpIEqual %v2bool %66 %71
         %74 =       OpAll %bool %72
                     OpBranch %65

         %65 = OpLabel
         %75 =   OpPhi %bool %false %26 %74 %64
                 OpSelectionMerge %77 None
                 OpBranchConditional %75 %76 %77

         %76 =     OpLabel
         %79 =       OpVectorShuffle %v3uint %48 %48 0 1 2
         %78 =       OpExtInst %v3uint %5 UClamp %79 %81 %82
         %83 =       OpVectorShuffle %v3uint %54 %54 0 1 2
         %84 =       OpIEqual %v3bool %78 %83
         %86 =       OpAll %bool %84
                     OpBranch %77

         %77 = OpLabel
         %87 =   OpPhi %bool %false %65 %86 %76
                 OpSelectionMerge %89 None
                 OpBranchConditional %87 %88 %89

         %88 =     OpLabel
         %90 =       OpExtInst %v4uint %5 UClamp %48 %91 %92
         %93 =       OpIEqual %v4bool %90 %54
         %95 =       OpAll %bool %93
                     OpBranch %89

         %89 = OpLabel
         %96 =   OpPhi %bool %false %77 %95 %88
                 OpSelectionMerge %98 None
                 OpBranchConditional %96 %97 %98

         %97 =     OpLabel
                     OpBranch %98

         %98 = OpLabel
        %100 =   OpPhi %bool %false %89 %true %97
                 OpSelectionMerge %102 None
                 OpBranchConditional %100 %101 %102

        %101 =     OpLabel
        %104 =       OpVectorShuffle %v2uint %54 %54 0 1
        %105 =       OpIEqual %v2bool %103 %104
        %106 =       OpAll %bool %105
                     OpBranch %102

        %102 = OpLabel
        %107 =   OpPhi %bool %false %98 %106 %101
                 OpSelectionMerge %109 None
                 OpBranchConditional %107 %108 %109

        %108 =     OpLabel
        %111 =       OpVectorShuffle %v3uint %54 %54 0 1 2
        %112 =       OpIEqual %v3bool %110 %111
        %113 =       OpAll %bool %112
                     OpBranch %109

        %109 = OpLabel
        %114 =   OpPhi %bool %false %102 %113 %108
                 OpSelectionMerge %116 None
                 OpBranchConditional %114 %115 %116

        %115 =     OpLabel
                     OpBranch %116

        %116 = OpLabel
        %117 =   OpPhi %bool %false %109 %true %115
                 OpSelectionMerge %119 None
                 OpBranchConditional %117 %118 %119

        %118 =     OpLabel
        %120 =       OpExtInst %uint %5 UClamp %62 %uint_100 %uint_300
        %121 =       OpIEqual %bool %120 %uint_100
                     OpBranch %119

        %119 = OpLabel
        %122 =   OpPhi %bool %false %116 %121 %118
                 OpSelectionMerge %124 None
                 OpBranchConditional %122 %123 %124

        %123 =     OpLabel
        %126 =       OpVectorShuffle %v2uint %48 %48 0 1
        %125 =       OpExtInst %v2uint %5 UClamp %126 %128 %130
        %131 =       OpVectorShuffle %v2uint %58 %58 0 1
        %132 =       OpIEqual %v2bool %125 %131
        %133 =       OpAll %bool %132
                     OpBranch %124

        %124 = OpLabel
        %134 =   OpPhi %bool %false %119 %133 %123
                 OpSelectionMerge %136 None
                 OpBranchConditional %134 %135 %136

        %135 =     OpLabel
        %138 =       OpVectorShuffle %v3uint %48 %48 0 1 2
        %137 =       OpExtInst %v3uint %5 UClamp %138 %139 %140
        %141 =       OpVectorShuffle %v3uint %58 %58 0 1 2
        %142 =       OpIEqual %v3bool %137 %141
        %143 =       OpAll %bool %142
                     OpBranch %136

        %136 = OpLabel
        %144 =   OpPhi %bool %false %124 %143 %135
                 OpSelectionMerge %146 None
                 OpBranchConditional %144 %145 %146

        %145 =     OpLabel
        %147 =       OpExtInst %v4uint %5 UClamp %48 %148 %150
        %151 =       OpIEqual %v4bool %147 %58
        %152 =       OpAll %bool %151
                     OpBranch %146

        %146 = OpLabel
        %153 =   OpPhi %bool %false %136 %152 %145
                 OpSelectionMerge %155 None
                 OpBranchConditional %153 %154 %155

        %154 =     OpLabel
                     OpBranch %155

        %155 = OpLabel
        %156 =   OpPhi %bool %false %146 %true %154
                 OpSelectionMerge %158 None
                 OpBranchConditional %156 %157 %158

        %157 =     OpLabel
        %159 =       OpVectorShuffle %v2uint %58 %58 0 1
        %160 =       OpIEqual %v2bool %103 %159
        %161 =       OpAll %bool %160
                     OpBranch %158

        %158 = OpLabel
        %162 =   OpPhi %bool %false %155 %161 %157
                 OpSelectionMerge %164 None
                 OpBranchConditional %162 %163 %164

        %163 =     OpLabel
        %166 =       OpVectorShuffle %v3uint %58 %58 0 1 2
        %167 =       OpIEqual %v3bool %165 %166
        %168 =       OpAll %bool %167
                     OpBranch %164

        %164 = OpLabel
        %169 =   OpPhi %bool %false %158 %168 %163
                 OpSelectionMerge %171 None
                 OpBranchConditional %169 %170 %171

        %170 =     OpLabel
                     OpBranch %171

        %171 = OpLabel
        %172 =   OpPhi %bool %false %164 %true %170
                 OpSelectionMerge %177 None
                 OpBranchConditional %172 %175 %176

        %175 =     OpLabel
        %178 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %180 =       OpLoad %v4float %178           ; RelaxedPrecision
                     OpStore %173 %180
                     OpBranch %177

        %176 =     OpLabel
        %181 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %183 =       OpLoad %v4float %181           ; RelaxedPrecision
                     OpStore %173 %183
                     OpBranch %177

        %177 = OpLabel
        %184 =   OpLoad %v4float %173               ; RelaxedPrecision
                 OpReturnValue %184
               OpFunctionEnd
