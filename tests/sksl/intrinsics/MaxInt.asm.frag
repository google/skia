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
               OpName %expectedB "expectedB"            ; id %64

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
               OpDecorate %178 RelaxedPrecision
               OpDecorate %181 RelaxedPrecision
               OpDecorate %182 RelaxedPrecision

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
     %int_50 = OpConstant %int 50
     %int_75 = OpConstant %int 75
    %int_225 = OpConstant %int 225
         %63 = OpConstantComposite %v4int %int_50 %int_50 %int_75 %int_225
    %int_100 = OpConstant %int 100
         %66 = OpConstantComposite %v4int %int_0 %int_100 %int_75 %int_225
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
      %v2int = OpTypeVector %int 2
         %77 = OpConstantComposite %v2int %int_50 %int_50
     %v2bool = OpTypeVector %bool 2
      %v3int = OpTypeVector %int 3
         %88 = OpConstantComposite %v3int %int_50 %int_50 %int_50
     %v3bool = OpTypeVector %bool 3
         %97 = OpConstantComposite %v4int %int_50 %int_50 %int_50 %int_50
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
        %114 = OpConstantComposite %v3int %int_50 %int_50 %int_75
        %157 = OpConstantComposite %v2int %int_0 %int_100
        %164 = OpConstantComposite %v3int %int_0 %int_100 %int_75
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
        %172 =   OpVariable %_ptr_Function_v4float Function
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
                 OpStore %expectedA %63
                 OpStore %expectedB %66
         %70 =   OpCompositeExtract %int %44 0
         %69 =   OpExtInst %int %5 SMax %70 %int_50
         %71 =   OpIEqual %bool %69 %int_50
                 OpSelectionMerge %73 None
                 OpBranchConditional %71 %72 %73

         %72 =     OpLabel
         %75 =       OpVectorShuffle %v2int %44 %44 0 1
         %74 =       OpExtInst %v2int %5 SMax %75 %77
         %78 =       OpVectorShuffle %v2int %63 %63 0 1
         %79 =       OpIEqual %v2bool %74 %78
         %81 =       OpAll %bool %79
                     OpBranch %73

         %73 = OpLabel
         %82 =   OpPhi %bool %false %26 %81 %72
                 OpSelectionMerge %84 None
                 OpBranchConditional %82 %83 %84

         %83 =     OpLabel
         %86 =       OpVectorShuffle %v3int %44 %44 0 1 2
         %85 =       OpExtInst %v3int %5 SMax %86 %88
         %89 =       OpVectorShuffle %v3int %63 %63 0 1 2
         %90 =       OpIEqual %v3bool %85 %89
         %92 =       OpAll %bool %90
                     OpBranch %84

         %84 = OpLabel
         %93 =   OpPhi %bool %false %73 %92 %83
                 OpSelectionMerge %95 None
                 OpBranchConditional %93 %94 %95

         %94 =     OpLabel
         %96 =       OpExtInst %v4int %5 SMax %44 %97
         %98 =       OpIEqual %v4bool %96 %63
        %100 =       OpAll %bool %98
                     OpBranch %95

         %95 = OpLabel
        %101 =   OpPhi %bool %false %84 %100 %94
                 OpSelectionMerge %103 None
                 OpBranchConditional %101 %102 %103

        %102 =     OpLabel
                     OpBranch %103

        %103 = OpLabel
        %105 =   OpPhi %bool %false %95 %true %102
                 OpSelectionMerge %107 None
                 OpBranchConditional %105 %106 %107

        %106 =     OpLabel
        %108 =       OpVectorShuffle %v2int %63 %63 0 1
        %109 =       OpIEqual %v2bool %77 %108
        %110 =       OpAll %bool %109
                     OpBranch %107

        %107 = OpLabel
        %111 =   OpPhi %bool %false %103 %110 %106
                 OpSelectionMerge %113 None
                 OpBranchConditional %111 %112 %113

        %112 =     OpLabel
        %115 =       OpVectorShuffle %v3int %63 %63 0 1 2
        %116 =       OpIEqual %v3bool %114 %115
        %117 =       OpAll %bool %116
                     OpBranch %113

        %113 = OpLabel
        %118 =   OpPhi %bool %false %107 %117 %112
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
        %124 =       OpExtInst %int %5 SMax %70 %125
        %126 =       OpIEqual %bool %124 %int_0
                     OpBranch %123

        %123 = OpLabel
        %127 =   OpPhi %bool %false %120 %126 %122
                 OpSelectionMerge %129 None
                 OpBranchConditional %127 %128 %129

        %128 =     OpLabel
        %131 =       OpVectorShuffle %v2int %44 %44 0 1
        %132 =       OpVectorShuffle %v2int %58 %58 0 1
        %130 =       OpExtInst %v2int %5 SMax %131 %132
        %133 =       OpVectorShuffle %v2int %66 %66 0 1
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
        %139 =       OpExtInst %v3int %5 SMax %140 %141
        %142 =       OpVectorShuffle %v3int %66 %66 0 1 2
        %143 =       OpIEqual %v3bool %139 %142
        %144 =       OpAll %bool %143
                     OpBranch %138

        %138 = OpLabel
        %145 =   OpPhi %bool %false %129 %144 %137
                 OpSelectionMerge %147 None
                 OpBranchConditional %145 %146 %147

        %146 =     OpLabel
        %148 =       OpExtInst %v4int %5 SMax %44 %58
        %149 =       OpIEqual %v4bool %148 %66
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
        %158 =       OpVectorShuffle %v2int %66 %66 0 1
        %159 =       OpIEqual %v2bool %157 %158
        %160 =       OpAll %bool %159
                     OpBranch %156

        %156 = OpLabel
        %161 =   OpPhi %bool %false %153 %160 %155
                 OpSelectionMerge %163 None
                 OpBranchConditional %161 %162 %163

        %162 =     OpLabel
        %165 =       OpVectorShuffle %v3int %66 %66 0 1 2
        %166 =       OpIEqual %v3bool %164 %165
        %167 =       OpAll %bool %166
                     OpBranch %163

        %163 = OpLabel
        %168 =   OpPhi %bool %false %156 %167 %162
                 OpSelectionMerge %170 None
                 OpBranchConditional %168 %169 %170

        %169 =     OpLabel
                     OpBranch %170

        %170 = OpLabel
        %171 =   OpPhi %bool %false %163 %true %169
                 OpSelectionMerge %176 None
                 OpBranchConditional %171 %174 %175

        %174 =     OpLabel
        %177 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %178 =       OpLoad %v4float %177           ; RelaxedPrecision
                     OpStore %172 %178
                     OpBranch %176

        %175 =     OpLabel
        %179 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %181 =       OpLoad %v4float %179           ; RelaxedPrecision
                     OpStore %172 %181
                     OpBranch %176

        %176 = OpLabel
        %182 =   OpLoad %v4float %172               ; RelaxedPrecision
                 OpReturnValue %182
               OpFunctionEnd
