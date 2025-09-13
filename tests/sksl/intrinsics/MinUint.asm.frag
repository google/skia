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
               OpName %uintGreen "uintGreen"            ; id %47
               OpName %expectedA "expectedA"            ; id %61
               OpName %expectedB "expectedB"            ; id %65

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
               OpDecorate %31 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
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
      %int_1 = OpConstant %int 1
    %uint_50 = OpConstant %uint 50
     %uint_0 = OpConstant %uint 0
         %64 = OpConstantComposite %v4uint %uint_50 %uint_0 %uint_50 %uint_50
   %uint_100 = OpConstant %uint 100
         %67 = OpConstantComposite %v4uint %uint_0 %uint_0 %uint_0 %uint_100
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
     %v2uint = OpTypeVector %uint 2
         %78 = OpConstantComposite %v2uint %uint_50 %uint_50
     %v2bool = OpTypeVector %bool 2
     %v3uint = OpTypeVector %uint 3
         %89 = OpConstantComposite %v3uint %uint_50 %uint_50 %uint_50
     %v3bool = OpTypeVector %bool 3
         %98 = OpConstantComposite %v4uint %uint_50 %uint_50 %uint_50 %uint_50
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
        %109 = OpConstantComposite %v2uint %uint_50 %uint_0
        %116 = OpConstantComposite %v3uint %uint_50 %uint_0 %uint_50
        %159 = OpConstantComposite %v2uint %uint_0 %uint_0
        %166 = OpConstantComposite %v3uint %uint_0 %uint_0 %uint_0
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
 %uintValues =   OpVariable %_ptr_Function_v4uint Function
  %uintGreen =   OpVariable %_ptr_Function_v4uint Function
  %expectedA =   OpVariable %_ptr_Function_v4uint Function
  %expectedB =   OpVariable %_ptr_Function_v4uint Function
        %174 =   OpVariable %_ptr_Function_v4float Function
         %32 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %35 =   OpLoad %v4float %32                ; RelaxedPrecision
         %31 =   OpExtInst %v4float %5 FAbs %35     ; RelaxedPrecision
         %37 =   OpVectorTimesScalar %v4float %31 %float_100    ; RelaxedPrecision
         %38 =   OpCompositeExtract %float %37 0                ; RelaxedPrecision
         %39 =   OpConvertFToU %uint %38
         %40 =   OpCompositeExtract %float %37 1    ; RelaxedPrecision
         %41 =   OpConvertFToU %uint %40
         %42 =   OpCompositeExtract %float %37 2    ; RelaxedPrecision
         %43 =   OpConvertFToU %uint %42
         %44 =   OpCompositeExtract %float %37 3    ; RelaxedPrecision
         %45 =   OpConvertFToU %uint %44
         %46 =   OpCompositeConstruct %v4uint %39 %41 %43 %45
                 OpStore %uintValues %46
         %48 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %50 =   OpLoad %v4float %48                ; RelaxedPrecision
         %51 =   OpVectorTimesScalar %v4float %50 %float_100    ; RelaxedPrecision
         %52 =   OpCompositeExtract %float %51 0                ; RelaxedPrecision
         %53 =   OpConvertFToU %uint %52
         %54 =   OpCompositeExtract %float %51 1    ; RelaxedPrecision
         %55 =   OpConvertFToU %uint %54
         %56 =   OpCompositeExtract %float %51 2    ; RelaxedPrecision
         %57 =   OpConvertFToU %uint %56
         %58 =   OpCompositeExtract %float %51 3    ; RelaxedPrecision
         %59 =   OpConvertFToU %uint %58
         %60 =   OpCompositeConstruct %v4uint %53 %55 %57 %59
                 OpStore %uintGreen %60
                 OpStore %expectedA %64
                 OpStore %expectedB %67
         %71 =   OpCompositeExtract %uint %46 0
         %70 =   OpExtInst %uint %5 UMin %71 %uint_50
         %72 =   OpIEqual %bool %70 %uint_50
                 OpSelectionMerge %74 None
                 OpBranchConditional %72 %73 %74

         %73 =     OpLabel
         %76 =       OpVectorShuffle %v2uint %46 %46 0 1
         %75 =       OpExtInst %v2uint %5 UMin %76 %78
         %79 =       OpVectorShuffle %v2uint %64 %64 0 1
         %80 =       OpIEqual %v2bool %75 %79
         %82 =       OpAll %bool %80
                     OpBranch %74

         %74 = OpLabel
         %83 =   OpPhi %bool %false %26 %82 %73
                 OpSelectionMerge %85 None
                 OpBranchConditional %83 %84 %85

         %84 =     OpLabel
         %87 =       OpVectorShuffle %v3uint %46 %46 0 1 2
         %86 =       OpExtInst %v3uint %5 UMin %87 %89
         %90 =       OpVectorShuffle %v3uint %64 %64 0 1 2
         %91 =       OpIEqual %v3bool %86 %90
         %93 =       OpAll %bool %91
                     OpBranch %85

         %85 = OpLabel
         %94 =   OpPhi %bool %false %74 %93 %84
                 OpSelectionMerge %96 None
                 OpBranchConditional %94 %95 %96

         %95 =     OpLabel
         %97 =       OpExtInst %v4uint %5 UMin %46 %98
         %99 =       OpIEqual %v4bool %97 %64
        %101 =       OpAll %bool %99
                     OpBranch %96

         %96 = OpLabel
        %102 =   OpPhi %bool %false %85 %101 %95
                 OpSelectionMerge %104 None
                 OpBranchConditional %102 %103 %104

        %103 =     OpLabel
                     OpBranch %104

        %104 = OpLabel
        %106 =   OpPhi %bool %false %96 %true %103
                 OpSelectionMerge %108 None
                 OpBranchConditional %106 %107 %108

        %107 =     OpLabel
        %110 =       OpVectorShuffle %v2uint %64 %64 0 1
        %111 =       OpIEqual %v2bool %109 %110
        %112 =       OpAll %bool %111
                     OpBranch %108

        %108 = OpLabel
        %113 =   OpPhi %bool %false %104 %112 %107
                 OpSelectionMerge %115 None
                 OpBranchConditional %113 %114 %115

        %114 =     OpLabel
        %117 =       OpVectorShuffle %v3uint %64 %64 0 1 2
        %118 =       OpIEqual %v3bool %116 %117
        %119 =       OpAll %bool %118
                     OpBranch %115

        %115 = OpLabel
        %120 =   OpPhi %bool %false %108 %119 %114
                 OpSelectionMerge %122 None
                 OpBranchConditional %120 %121 %122

        %121 =     OpLabel
                     OpBranch %122

        %122 = OpLabel
        %123 =   OpPhi %bool %false %115 %true %121
                 OpSelectionMerge %125 None
                 OpBranchConditional %123 %124 %125

        %124 =     OpLabel
        %127 =       OpCompositeExtract %uint %60 0
        %126 =       OpExtInst %uint %5 UMin %71 %127
        %128 =       OpIEqual %bool %126 %uint_0
                     OpBranch %125

        %125 = OpLabel
        %129 =   OpPhi %bool %false %122 %128 %124
                 OpSelectionMerge %131 None
                 OpBranchConditional %129 %130 %131

        %130 =     OpLabel
        %133 =       OpVectorShuffle %v2uint %46 %46 0 1
        %134 =       OpVectorShuffle %v2uint %60 %60 0 1
        %132 =       OpExtInst %v2uint %5 UMin %133 %134
        %135 =       OpVectorShuffle %v2uint %67 %67 0 1
        %136 =       OpIEqual %v2bool %132 %135
        %137 =       OpAll %bool %136
                     OpBranch %131

        %131 = OpLabel
        %138 =   OpPhi %bool %false %125 %137 %130
                 OpSelectionMerge %140 None
                 OpBranchConditional %138 %139 %140

        %139 =     OpLabel
        %142 =       OpVectorShuffle %v3uint %46 %46 0 1 2
        %143 =       OpVectorShuffle %v3uint %60 %60 0 1 2
        %141 =       OpExtInst %v3uint %5 UMin %142 %143
        %144 =       OpVectorShuffle %v3uint %67 %67 0 1 2
        %145 =       OpIEqual %v3bool %141 %144
        %146 =       OpAll %bool %145
                     OpBranch %140

        %140 = OpLabel
        %147 =   OpPhi %bool %false %131 %146 %139
                 OpSelectionMerge %149 None
                 OpBranchConditional %147 %148 %149

        %148 =     OpLabel
        %150 =       OpExtInst %v4uint %5 UMin %46 %60
        %151 =       OpIEqual %v4bool %150 %67
        %152 =       OpAll %bool %151
                     OpBranch %149

        %149 = OpLabel
        %153 =   OpPhi %bool %false %140 %152 %148
                 OpSelectionMerge %155 None
                 OpBranchConditional %153 %154 %155

        %154 =     OpLabel
                     OpBranch %155

        %155 = OpLabel
        %156 =   OpPhi %bool %false %149 %true %154
                 OpSelectionMerge %158 None
                 OpBranchConditional %156 %157 %158

        %157 =     OpLabel
        %160 =       OpVectorShuffle %v2uint %67 %67 0 1
        %161 =       OpIEqual %v2bool %159 %160
        %162 =       OpAll %bool %161
                     OpBranch %158

        %158 = OpLabel
        %163 =   OpPhi %bool %false %155 %162 %157
                 OpSelectionMerge %165 None
                 OpBranchConditional %163 %164 %165

        %164 =     OpLabel
        %167 =       OpVectorShuffle %v3uint %67 %67 0 1 2
        %168 =       OpIEqual %v3bool %166 %167
        %169 =       OpAll %bool %168
                     OpBranch %165

        %165 = OpLabel
        %170 =   OpPhi %bool %false %158 %169 %164
                 OpSelectionMerge %172 None
                 OpBranchConditional %170 %171 %172

        %171 =     OpLabel
                     OpBranch %172

        %172 = OpLabel
        %173 =   OpPhi %bool %false %165 %true %171
                 OpSelectionMerge %178 None
                 OpBranchConditional %173 %176 %177

        %176 =     OpLabel
        %179 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %180 =       OpLoad %v4float %179           ; RelaxedPrecision
                     OpStore %174 %180
                     OpBranch %178

        %177 =     OpLabel
        %181 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %183 =       OpLoad %v4float %181           ; RelaxedPrecision
                     OpStore %174 %183
                     OpBranch %178

        %178 = OpLabel
        %184 =   OpLoad %v4float %174               ; RelaxedPrecision
                 OpReturnValue %184
               OpFunctionEnd
