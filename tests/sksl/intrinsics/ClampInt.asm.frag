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
               OpName %intValues "intValues"            ; id %27
               OpName %expectedA "expectedA"            ; id %45
               OpName %expectedB "expectedB"            ; id %50

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
               OpDecorate %33 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %178 RelaxedPrecision
               OpDecorate %179 RelaxedPrecision

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
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
  %float_100 = OpConstant %float 100
   %int_n100 = OpConstant %int -100
     %int_75 = OpConstant %int 75
    %int_100 = OpConstant %int 100
         %49 = OpConstantComposite %v4int %int_n100 %int_0 %int_75 %int_100
     %int_50 = OpConstant %int 50
    %int_225 = OpConstant %int 225
         %53 = OpConstantComposite %v4int %int_n100 %int_0 %int_50 %int_225
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
      %v2int = OpTypeVector %int 2
         %64 = OpConstantComposite %v2int %int_n100 %int_n100
         %65 = OpConstantComposite %v2int %int_100 %int_100
     %v2bool = OpTypeVector %bool 2
      %v3int = OpTypeVector %int 3
         %76 = OpConstantComposite %v3int %int_n100 %int_n100 %int_n100
         %77 = OpConstantComposite %v3int %int_100 %int_100 %int_100
     %v3bool = OpTypeVector %bool 3
         %86 = OpConstantComposite %v4int %int_n100 %int_n100 %int_n100 %int_n100
         %87 = OpConstantComposite %v4int %int_100 %int_100 %int_100 %int_100
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
         %98 = OpConstantComposite %v2int %int_n100 %int_0
        %105 = OpConstantComposite %v3int %int_n100 %int_0 %int_75
   %int_n200 = OpConstant %int -200
        %123 = OpConstantComposite %v2int %int_n100 %int_n200
    %int_200 = OpConstant %int 200
        %125 = OpConstantComposite %v2int %int_100 %int_200
        %134 = OpConstantComposite %v3int %int_n100 %int_n200 %int_n200
        %135 = OpConstantComposite %v3int %int_100 %int_200 %int_50
        %143 = OpConstantComposite %v4int %int_n100 %int_n200 %int_n200 %int_100
    %int_300 = OpConstant %int 300
        %145 = OpConstantComposite %v4int %int_100 %int_200 %int_50 %int_300
        %160 = OpConstantComposite %v3int %int_n100 %int_0 %int_50
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
  %intValues =   OpVariable %_ptr_Function_v4int Function
  %expectedA =   OpVariable %_ptr_Function_v4int Function
  %expectedB =   OpVariable %_ptr_Function_v4int Function
        %168 =   OpVariable %_ptr_Function_v4float Function
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
                 OpStore %intValues %44
                 OpStore %expectedA %49
                 OpStore %expectedB %53
         %57 =   OpCompositeExtract %int %44 0
         %56 =   OpExtInst %int %5 SClamp %57 %int_n100 %int_100
         %58 =   OpIEqual %bool %56 %int_n100
                 OpSelectionMerge %60 None
                 OpBranchConditional %58 %59 %60

         %59 =     OpLabel
         %62 =       OpVectorShuffle %v2int %44 %44 0 1
         %61 =       OpExtInst %v2int %5 SClamp %62 %64 %65
         %66 =       OpVectorShuffle %v2int %49 %49 0 1
         %67 =       OpIEqual %v2bool %61 %66
         %69 =       OpAll %bool %67
                     OpBranch %60

         %60 = OpLabel
         %70 =   OpPhi %bool %false %26 %69 %59
                 OpSelectionMerge %72 None
                 OpBranchConditional %70 %71 %72

         %71 =     OpLabel
         %74 =       OpVectorShuffle %v3int %44 %44 0 1 2
         %73 =       OpExtInst %v3int %5 SClamp %74 %76 %77
         %78 =       OpVectorShuffle %v3int %49 %49 0 1 2
         %79 =       OpIEqual %v3bool %73 %78
         %81 =       OpAll %bool %79
                     OpBranch %72

         %72 = OpLabel
         %82 =   OpPhi %bool %false %60 %81 %71
                 OpSelectionMerge %84 None
                 OpBranchConditional %82 %83 %84

         %83 =     OpLabel
         %85 =       OpExtInst %v4int %5 SClamp %44 %86 %87
         %88 =       OpIEqual %v4bool %85 %49
         %90 =       OpAll %bool %88
                     OpBranch %84

         %84 = OpLabel
         %91 =   OpPhi %bool %false %72 %90 %83
                 OpSelectionMerge %93 None
                 OpBranchConditional %91 %92 %93

         %92 =     OpLabel
                     OpBranch %93

         %93 = OpLabel
         %95 =   OpPhi %bool %false %84 %true %92
                 OpSelectionMerge %97 None
                 OpBranchConditional %95 %96 %97

         %96 =     OpLabel
         %99 =       OpVectorShuffle %v2int %49 %49 0 1
        %100 =       OpIEqual %v2bool %98 %99
        %101 =       OpAll %bool %100
                     OpBranch %97

         %97 = OpLabel
        %102 =   OpPhi %bool %false %93 %101 %96
                 OpSelectionMerge %104 None
                 OpBranchConditional %102 %103 %104

        %103 =     OpLabel
        %106 =       OpVectorShuffle %v3int %49 %49 0 1 2
        %107 =       OpIEqual %v3bool %105 %106
        %108 =       OpAll %bool %107
                     OpBranch %104

        %104 = OpLabel
        %109 =   OpPhi %bool %false %97 %108 %103
                 OpSelectionMerge %111 None
                 OpBranchConditional %109 %110 %111

        %110 =     OpLabel
                     OpBranch %111

        %111 = OpLabel
        %112 =   OpPhi %bool %false %104 %true %110
                 OpSelectionMerge %114 None
                 OpBranchConditional %112 %113 %114

        %113 =     OpLabel
        %115 =       OpExtInst %int %5 SClamp %57 %int_n100 %int_100
        %116 =       OpIEqual %bool %115 %int_n100
                     OpBranch %114

        %114 = OpLabel
        %117 =   OpPhi %bool %false %111 %116 %113
                 OpSelectionMerge %119 None
                 OpBranchConditional %117 %118 %119

        %118 =     OpLabel
        %121 =       OpVectorShuffle %v2int %44 %44 0 1
        %120 =       OpExtInst %v2int %5 SClamp %121 %123 %125
        %126 =       OpVectorShuffle %v2int %53 %53 0 1
        %127 =       OpIEqual %v2bool %120 %126
        %128 =       OpAll %bool %127
                     OpBranch %119

        %119 = OpLabel
        %129 =   OpPhi %bool %false %114 %128 %118
                 OpSelectionMerge %131 None
                 OpBranchConditional %129 %130 %131

        %130 =     OpLabel
        %133 =       OpVectorShuffle %v3int %44 %44 0 1 2
        %132 =       OpExtInst %v3int %5 SClamp %133 %134 %135
        %136 =       OpVectorShuffle %v3int %53 %53 0 1 2
        %137 =       OpIEqual %v3bool %132 %136
        %138 =       OpAll %bool %137
                     OpBranch %131

        %131 = OpLabel
        %139 =   OpPhi %bool %false %119 %138 %130
                 OpSelectionMerge %141 None
                 OpBranchConditional %139 %140 %141

        %140 =     OpLabel
        %142 =       OpExtInst %v4int %5 SClamp %44 %143 %145
        %146 =       OpIEqual %v4bool %142 %53
        %147 =       OpAll %bool %146
                     OpBranch %141

        %141 = OpLabel
        %148 =   OpPhi %bool %false %131 %147 %140
                 OpSelectionMerge %150 None
                 OpBranchConditional %148 %149 %150

        %149 =     OpLabel
                     OpBranch %150

        %150 = OpLabel
        %151 =   OpPhi %bool %false %141 %true %149
                 OpSelectionMerge %153 None
                 OpBranchConditional %151 %152 %153

        %152 =     OpLabel
        %154 =       OpVectorShuffle %v2int %53 %53 0 1
        %155 =       OpIEqual %v2bool %98 %154
        %156 =       OpAll %bool %155
                     OpBranch %153

        %153 = OpLabel
        %157 =   OpPhi %bool %false %150 %156 %152
                 OpSelectionMerge %159 None
                 OpBranchConditional %157 %158 %159

        %158 =     OpLabel
        %161 =       OpVectorShuffle %v3int %53 %53 0 1 2
        %162 =       OpIEqual %v3bool %160 %161
        %163 =       OpAll %bool %162
                     OpBranch %159

        %159 = OpLabel
        %164 =   OpPhi %bool %false %153 %163 %158
                 OpSelectionMerge %166 None
                 OpBranchConditional %164 %165 %166

        %165 =     OpLabel
                     OpBranch %166

        %166 = OpLabel
        %167 =   OpPhi %bool %false %159 %true %165
                 OpSelectionMerge %172 None
                 OpBranchConditional %167 %170 %171

        %170 =     OpLabel
        %173 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %175 =       OpLoad %v4float %173           ; RelaxedPrecision
                     OpStore %168 %175
                     OpBranch %172

        %171 =     OpLabel
        %176 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %178 =       OpLoad %v4float %176           ; RelaxedPrecision
                     OpStore %168 %178
                     OpBranch %172

        %172 = OpLabel
        %179 =   OpLoad %v4float %168               ; RelaxedPrecision
                 OpReturnValue %179
               OpFunctionEnd
