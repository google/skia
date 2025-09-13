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
               OpName %intGreen "intGreen"              ; id %45
               OpName %expectedA "expectedA"            ; id %59
               OpName %expectedB "expectedB"            ; id %63

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
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision
               OpDecorate %180 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision

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
      %int_1 = OpConstant %int 1
   %int_n125 = OpConstant %int -125
     %int_50 = OpConstant %int 50
         %62 = OpConstantComposite %v4int %int_n125 %int_0 %int_50 %int_50
    %int_100 = OpConstant %int 100
         %65 = OpConstantComposite %v4int %int_n125 %int_0 %int_0 %int_100
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
      %v2int = OpTypeVector %int 2
         %76 = OpConstantComposite %v2int %int_50 %int_50
     %v2bool = OpTypeVector %bool 2
      %v3int = OpTypeVector %int 3
         %87 = OpConstantComposite %v3int %int_50 %int_50 %int_50
     %v3bool = OpTypeVector %bool 3
         %96 = OpConstantComposite %v4int %int_50 %int_50 %int_50 %int_50
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
        %107 = OpConstantComposite %v2int %int_n125 %int_0
        %114 = OpConstantComposite %v3int %int_n125 %int_0 %int_50
        %163 = OpConstantComposite %v3int %int_n125 %int_0 %int_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
   %intGreen =   OpVariable %_ptr_Function_v4int Function
  %expectedA =   OpVariable %_ptr_Function_v4int Function
  %expectedB =   OpVariable %_ptr_Function_v4int Function
        %171 =   OpVariable %_ptr_Function_v4float Function
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
                 OpStore %intGreen %58
                 OpStore %expectedA %62
                 OpStore %expectedB %65
         %69 =   OpCompositeExtract %int %44 0
         %68 =   OpExtInst %int %5 SMin %69 %int_50
         %70 =   OpIEqual %bool %68 %int_n125
                 OpSelectionMerge %72 None
                 OpBranchConditional %70 %71 %72

         %71 =     OpLabel
         %74 =       OpVectorShuffle %v2int %44 %44 0 1
         %73 =       OpExtInst %v2int %5 SMin %74 %76
         %77 =       OpVectorShuffle %v2int %62 %62 0 1
         %78 =       OpIEqual %v2bool %73 %77
         %80 =       OpAll %bool %78
                     OpBranch %72

         %72 = OpLabel
         %81 =   OpPhi %bool %false %26 %80 %71
                 OpSelectionMerge %83 None
                 OpBranchConditional %81 %82 %83

         %82 =     OpLabel
         %85 =       OpVectorShuffle %v3int %44 %44 0 1 2
         %84 =       OpExtInst %v3int %5 SMin %85 %87
         %88 =       OpVectorShuffle %v3int %62 %62 0 1 2
         %89 =       OpIEqual %v3bool %84 %88
         %91 =       OpAll %bool %89
                     OpBranch %83

         %83 = OpLabel
         %92 =   OpPhi %bool %false %72 %91 %82
                 OpSelectionMerge %94 None
                 OpBranchConditional %92 %93 %94

         %93 =     OpLabel
         %95 =       OpExtInst %v4int %5 SMin %44 %96
         %97 =       OpIEqual %v4bool %95 %62
         %99 =       OpAll %bool %97
                     OpBranch %94

         %94 = OpLabel
        %100 =   OpPhi %bool %false %83 %99 %93
                 OpSelectionMerge %102 None
                 OpBranchConditional %100 %101 %102

        %101 =     OpLabel
                     OpBranch %102

        %102 = OpLabel
        %104 =   OpPhi %bool %false %94 %true %101
                 OpSelectionMerge %106 None
                 OpBranchConditional %104 %105 %106

        %105 =     OpLabel
        %108 =       OpVectorShuffle %v2int %62 %62 0 1
        %109 =       OpIEqual %v2bool %107 %108
        %110 =       OpAll %bool %109
                     OpBranch %106

        %106 = OpLabel
        %111 =   OpPhi %bool %false %102 %110 %105
                 OpSelectionMerge %113 None
                 OpBranchConditional %111 %112 %113

        %112 =     OpLabel
        %115 =       OpVectorShuffle %v3int %62 %62 0 1 2
        %116 =       OpIEqual %v3bool %114 %115
        %117 =       OpAll %bool %116
                     OpBranch %113

        %113 = OpLabel
        %118 =   OpPhi %bool %false %106 %117 %112
                 OpSelectionMerge %120 None
                 OpBranchConditional %118 %119 %120

        %119 =     OpLabel
                     OpBranch %120

        %120 = OpLabel
        %121 =   OpPhi %bool %false %113 %true %119
                 OpSelectionMerge %123 None
                 OpBranchConditional %121 %122 %123

        %122 =     OpLabel
        %125 =       OpCompositeExtract %int %58 0
        %124 =       OpExtInst %int %5 SMin %69 %125
        %126 =       OpIEqual %bool %124 %int_n125
                     OpBranch %123

        %123 = OpLabel
        %127 =   OpPhi %bool %false %120 %126 %122
                 OpSelectionMerge %129 None
                 OpBranchConditional %127 %128 %129

        %128 =     OpLabel
        %131 =       OpVectorShuffle %v2int %44 %44 0 1
        %132 =       OpVectorShuffle %v2int %58 %58 0 1
        %130 =       OpExtInst %v2int %5 SMin %131 %132
        %133 =       OpVectorShuffle %v2int %65 %65 0 1
        %134 =       OpIEqual %v2bool %130 %133
        %135 =       OpAll %bool %134
                     OpBranch %129

        %129 = OpLabel
        %136 =   OpPhi %bool %false %123 %135 %128
                 OpSelectionMerge %138 None
                 OpBranchConditional %136 %137 %138

        %137 =     OpLabel
        %140 =       OpVectorShuffle %v3int %44 %44 0 1 2
        %141 =       OpVectorShuffle %v3int %58 %58 0 1 2
        %139 =       OpExtInst %v3int %5 SMin %140 %141
        %142 =       OpVectorShuffle %v3int %65 %65 0 1 2
        %143 =       OpIEqual %v3bool %139 %142
        %144 =       OpAll %bool %143
                     OpBranch %138

        %138 = OpLabel
        %145 =   OpPhi %bool %false %129 %144 %137
                 OpSelectionMerge %147 None
                 OpBranchConditional %145 %146 %147

        %146 =     OpLabel
        %148 =       OpExtInst %v4int %5 SMin %44 %58
        %149 =       OpIEqual %v4bool %148 %65
        %150 =       OpAll %bool %149
                     OpBranch %147

        %147 = OpLabel
        %151 =   OpPhi %bool %false %138 %150 %146
                 OpSelectionMerge %153 None
                 OpBranchConditional %151 %152 %153

        %152 =     OpLabel
                     OpBranch %153

        %153 = OpLabel
        %154 =   OpPhi %bool %false %147 %true %152
                 OpSelectionMerge %156 None
                 OpBranchConditional %154 %155 %156

        %155 =     OpLabel
        %157 =       OpVectorShuffle %v2int %65 %65 0 1
        %158 =       OpIEqual %v2bool %107 %157
        %159 =       OpAll %bool %158
                     OpBranch %156

        %156 = OpLabel
        %160 =   OpPhi %bool %false %153 %159 %155
                 OpSelectionMerge %162 None
                 OpBranchConditional %160 %161 %162

        %161 =     OpLabel
        %164 =       OpVectorShuffle %v3int %65 %65 0 1 2
        %165 =       OpIEqual %v3bool %163 %164
        %166 =       OpAll %bool %165
                     OpBranch %162

        %162 = OpLabel
        %167 =   OpPhi %bool %false %156 %166 %161
                 OpSelectionMerge %169 None
                 OpBranchConditional %167 %168 %169

        %168 =     OpLabel
                     OpBranch %169

        %169 = OpLabel
        %170 =   OpPhi %bool %false %162 %true %168
                 OpSelectionMerge %175 None
                 OpBranchConditional %170 %173 %174

        %173 =     OpLabel
        %176 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %177 =       OpLoad %v4float %176           ; RelaxedPrecision
                     OpStore %171 %177
                     OpBranch %175

        %174 =     OpLabel
        %178 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %180 =       OpLoad %v4float %178           ; RelaxedPrecision
                     OpStore %171 %180
                     OpBranch %175

        %175 = OpLabel
        %181 =   OpLoad %v4float %171               ; RelaxedPrecision
                 OpReturnValue %181
               OpFunctionEnd
