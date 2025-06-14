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
               OpName %expectedB "expectedB"            ; id %66

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
               OpDecorate %182 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
               OpDecorate %186 RelaxedPrecision

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
   %uint_125 = OpConstant %uint 125
    %uint_80 = OpConstant %uint 80
   %uint_225 = OpConstant %uint 225
         %65 = OpConstantComposite %v4uint %uint_125 %uint_80 %uint_80 %uint_225
   %uint_100 = OpConstant %uint 100
    %uint_75 = OpConstant %uint 75
         %69 = OpConstantComposite %v4uint %uint_125 %uint_100 %uint_75 %uint_225
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
     %v2uint = OpTypeVector %uint 2
         %80 = OpConstantComposite %v2uint %uint_80 %uint_80
     %v2bool = OpTypeVector %bool 2
     %v3uint = OpTypeVector %uint 3
         %91 = OpConstantComposite %v3uint %uint_80 %uint_80 %uint_80
     %v3bool = OpTypeVector %bool 3
        %100 = OpConstantComposite %v4uint %uint_80 %uint_80 %uint_80 %uint_80
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
        %111 = OpConstantComposite %v2uint %uint_125 %uint_80
        %118 = OpConstantComposite %v3uint %uint_125 %uint_80 %uint_80
        %161 = OpConstantComposite %v2uint %uint_125 %uint_100
        %168 = OpConstantComposite %v3uint %uint_125 %uint_100 %uint_75
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
        %176 =   OpVariable %_ptr_Function_v4float Function
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
                 OpStore %expectedA %65
                 OpStore %expectedB %69
         %73 =   OpCompositeExtract %uint %46 0
         %72 =   OpExtInst %uint %5 UMax %73 %uint_80
         %74 =   OpIEqual %bool %72 %uint_125
                 OpSelectionMerge %76 None
                 OpBranchConditional %74 %75 %76

         %75 =     OpLabel
         %78 =       OpVectorShuffle %v2uint %46 %46 0 1
         %77 =       OpExtInst %v2uint %5 UMax %78 %80
         %81 =       OpVectorShuffle %v2uint %65 %65 0 1
         %82 =       OpIEqual %v2bool %77 %81
         %84 =       OpAll %bool %82
                     OpBranch %76

         %76 = OpLabel
         %85 =   OpPhi %bool %false %26 %84 %75
                 OpSelectionMerge %87 None
                 OpBranchConditional %85 %86 %87

         %86 =     OpLabel
         %89 =       OpVectorShuffle %v3uint %46 %46 0 1 2
         %88 =       OpExtInst %v3uint %5 UMax %89 %91
         %92 =       OpVectorShuffle %v3uint %65 %65 0 1 2
         %93 =       OpIEqual %v3bool %88 %92
         %95 =       OpAll %bool %93
                     OpBranch %87

         %87 = OpLabel
         %96 =   OpPhi %bool %false %76 %95 %86
                 OpSelectionMerge %98 None
                 OpBranchConditional %96 %97 %98

         %97 =     OpLabel
         %99 =       OpExtInst %v4uint %5 UMax %46 %100
        %101 =       OpIEqual %v4bool %99 %65
        %103 =       OpAll %bool %101
                     OpBranch %98

         %98 = OpLabel
        %104 =   OpPhi %bool %false %87 %103 %97
                 OpSelectionMerge %106 None
                 OpBranchConditional %104 %105 %106

        %105 =     OpLabel
                     OpBranch %106

        %106 = OpLabel
        %108 =   OpPhi %bool %false %98 %true %105
                 OpSelectionMerge %110 None
                 OpBranchConditional %108 %109 %110

        %109 =     OpLabel
        %112 =       OpVectorShuffle %v2uint %65 %65 0 1
        %113 =       OpIEqual %v2bool %111 %112
        %114 =       OpAll %bool %113
                     OpBranch %110

        %110 = OpLabel
        %115 =   OpPhi %bool %false %106 %114 %109
                 OpSelectionMerge %117 None
                 OpBranchConditional %115 %116 %117

        %116 =     OpLabel
        %119 =       OpVectorShuffle %v3uint %65 %65 0 1 2
        %120 =       OpIEqual %v3bool %118 %119
        %121 =       OpAll %bool %120
                     OpBranch %117

        %117 = OpLabel
        %122 =   OpPhi %bool %false %110 %121 %116
                 OpSelectionMerge %124 None
                 OpBranchConditional %122 %123 %124

        %123 =     OpLabel
                     OpBranch %124

        %124 = OpLabel
        %125 =   OpPhi %bool %false %117 %true %123
                 OpSelectionMerge %127 None
                 OpBranchConditional %125 %126 %127

        %126 =     OpLabel
        %129 =       OpCompositeExtract %uint %60 0
        %128 =       OpExtInst %uint %5 UMax %73 %129
        %130 =       OpIEqual %bool %128 %uint_125
                     OpBranch %127

        %127 = OpLabel
        %131 =   OpPhi %bool %false %124 %130 %126
                 OpSelectionMerge %133 None
                 OpBranchConditional %131 %132 %133

        %132 =     OpLabel
        %135 =       OpVectorShuffle %v2uint %46 %46 0 1
        %136 =       OpVectorShuffle %v2uint %60 %60 0 1
        %134 =       OpExtInst %v2uint %5 UMax %135 %136
        %137 =       OpVectorShuffle %v2uint %69 %69 0 1
        %138 =       OpIEqual %v2bool %134 %137
        %139 =       OpAll %bool %138
                     OpBranch %133

        %133 = OpLabel
        %140 =   OpPhi %bool %false %127 %139 %132
                 OpSelectionMerge %142 None
                 OpBranchConditional %140 %141 %142

        %141 =     OpLabel
        %144 =       OpVectorShuffle %v3uint %46 %46 0 1 2
        %145 =       OpVectorShuffle %v3uint %60 %60 0 1 2
        %143 =       OpExtInst %v3uint %5 UMax %144 %145
        %146 =       OpVectorShuffle %v3uint %69 %69 0 1 2
        %147 =       OpIEqual %v3bool %143 %146
        %148 =       OpAll %bool %147
                     OpBranch %142

        %142 = OpLabel
        %149 =   OpPhi %bool %false %133 %148 %141
                 OpSelectionMerge %151 None
                 OpBranchConditional %149 %150 %151

        %150 =     OpLabel
        %152 =       OpExtInst %v4uint %5 UMax %46 %60
        %153 =       OpIEqual %v4bool %152 %69
        %154 =       OpAll %bool %153
                     OpBranch %151

        %151 = OpLabel
        %155 =   OpPhi %bool %false %142 %154 %150
                 OpSelectionMerge %157 None
                 OpBranchConditional %155 %156 %157

        %156 =     OpLabel
                     OpBranch %157

        %157 = OpLabel
        %158 =   OpPhi %bool %false %151 %true %156
                 OpSelectionMerge %160 None
                 OpBranchConditional %158 %159 %160

        %159 =     OpLabel
        %162 =       OpVectorShuffle %v2uint %69 %69 0 1
        %163 =       OpIEqual %v2bool %161 %162
        %164 =       OpAll %bool %163
                     OpBranch %160

        %160 = OpLabel
        %165 =   OpPhi %bool %false %157 %164 %159
                 OpSelectionMerge %167 None
                 OpBranchConditional %165 %166 %167

        %166 =     OpLabel
        %169 =       OpVectorShuffle %v3uint %69 %69 0 1 2
        %170 =       OpIEqual %v3bool %168 %169
        %171 =       OpAll %bool %170
                     OpBranch %167

        %167 = OpLabel
        %172 =   OpPhi %bool %false %160 %171 %166
                 OpSelectionMerge %174 None
                 OpBranchConditional %172 %173 %174

        %173 =     OpLabel
                     OpBranch %174

        %174 = OpLabel
        %175 =   OpPhi %bool %false %167 %true %173
                 OpSelectionMerge %180 None
                 OpBranchConditional %175 %178 %179

        %178 =     OpLabel
        %181 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %182 =       OpLoad %v4float %181           ; RelaxedPrecision
                     OpStore %176 %182
                     OpBranch %180

        %179 =     OpLabel
        %183 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %185 =       OpLoad %v4float %183           ; RelaxedPrecision
                     OpStore %176 %185
                     OpBranch %180

        %180 = OpLabel
        %186 =   OpLoad %v4float %176               ; RelaxedPrecision
                 OpReturnValue %186
               OpFunctionEnd
