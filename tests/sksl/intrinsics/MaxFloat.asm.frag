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
               OpName %expectedB "expectedB"            ; id %33

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
               OpDecorate %38 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %134 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %154 RelaxedPrecision
               OpDecorate %161 RelaxedPrecision
               OpDecorate %173 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision
               OpDecorate %177 RelaxedPrecision

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
  %float_0_5 = OpConstant %float 0.5
 %float_0_75 = OpConstant %float 0.75
 %float_2_25 = OpConstant %float 2.25
         %32 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_75 %float_2_25
    %float_1 = OpConstant %float 1
         %35 = OpConstantComposite %v4float %float_0 %float_1 %float_0_75 %float_2_25
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
         %51 = OpConstantComposite %v2float %float_0_5 %float_0_5
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
         %64 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
     %v3bool = OpTypeVector %bool 3
         %75 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
         %92 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_75
      %int_1 = OpConstant %int 1
        %153 = OpConstantComposite %v2float %float_0 %float_1
        %160 = OpConstantComposite %v3float %float_0 %float_1 %float_0_75
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
  %expectedA =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
  %expectedB =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
        %168 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %expectedA %32
                 OpStore %expectedB %35
         %39 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %42 =   OpLoad %v4float %39                ; RelaxedPrecision
         %43 =   OpCompositeExtract %float %42 0    ; RelaxedPrecision
         %38 =   OpExtInst %float %5 FMax %43 %float_0_5    ; RelaxedPrecision
         %44 =   OpFOrdEqual %bool %38 %float_0_5
                 OpSelectionMerge %46 None
                 OpBranchConditional %44 %45 %46

         %45 =     OpLabel
         %48 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %49 =       OpLoad %v4float %48            ; RelaxedPrecision
         %50 =       OpVectorShuffle %v2float %49 %49 0 1   ; RelaxedPrecision
         %47 =       OpExtInst %v2float %5 FMax %50 %51     ; RelaxedPrecision
         %52 =       OpVectorShuffle %v2float %32 %32 0 1   ; RelaxedPrecision
         %53 =       OpFOrdEqual %v2bool %47 %52
         %55 =       OpAll %bool %53
                     OpBranch %46

         %46 = OpLabel
         %56 =   OpPhi %bool %false %26 %55 %45
                 OpSelectionMerge %58 None
                 OpBranchConditional %56 %57 %58

         %57 =     OpLabel
         %60 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %61 =       OpLoad %v4float %60            ; RelaxedPrecision
         %62 =       OpVectorShuffle %v3float %61 %61 0 1 2     ; RelaxedPrecision
         %59 =       OpExtInst %v3float %5 FMax %62 %64         ; RelaxedPrecision
         %65 =       OpVectorShuffle %v3float %32 %32 0 1 2     ; RelaxedPrecision
         %66 =       OpFOrdEqual %v3bool %59 %65
         %68 =       OpAll %bool %66
                     OpBranch %58

         %58 = OpLabel
         %69 =   OpPhi %bool %false %46 %68 %57
                 OpSelectionMerge %71 None
                 OpBranchConditional %69 %70 %71

         %70 =     OpLabel
         %73 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %74 =       OpLoad %v4float %73            ; RelaxedPrecision
         %72 =       OpExtInst %v4float %5 FMax %74 %75     ; RelaxedPrecision
         %76 =       OpFOrdEqual %v4bool %72 %32
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
         %86 =       OpVectorShuffle %v2float %32 %32 0 1   ; RelaxedPrecision
         %87 =       OpFOrdEqual %v2bool %51 %86
         %88 =       OpAll %bool %87
                     OpBranch %85

         %85 = OpLabel
         %89 =   OpPhi %bool %false %81 %88 %84
                 OpSelectionMerge %91 None
                 OpBranchConditional %89 %90 %91

         %90 =     OpLabel
         %93 =       OpVectorShuffle %v3float %32 %32 0 1 2     ; RelaxedPrecision
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
        %106 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %108 =       OpLoad %v4float %106           ; RelaxedPrecision
        %109 =       OpCompositeExtract %float %108 0   ; RelaxedPrecision
        %102 =       OpExtInst %float %5 FMax %105 %109     ; RelaxedPrecision
        %110 =       OpFOrdEqual %bool %102 %float_0
                     OpBranch %101

        %101 = OpLabel
        %111 =   OpPhi %bool %false %98 %110 %100
                 OpSelectionMerge %113 None
                 OpBranchConditional %111 %112 %113

        %112 =     OpLabel
        %115 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %116 =       OpLoad %v4float %115           ; RelaxedPrecision
        %117 =       OpVectorShuffle %v2float %116 %116 0 1     ; RelaxedPrecision
        %118 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %119 =       OpLoad %v4float %118           ; RelaxedPrecision
        %120 =       OpVectorShuffle %v2float %119 %119 0 1     ; RelaxedPrecision
        %114 =       OpExtInst %v2float %5 FMax %117 %120       ; RelaxedPrecision
        %121 =       OpVectorShuffle %v2float %35 %35 0 1       ; RelaxedPrecision
        %122 =       OpFOrdEqual %v2bool %114 %121
        %123 =       OpAll %bool %122
                     OpBranch %113

        %113 = OpLabel
        %124 =   OpPhi %bool %false %101 %123 %112
                 OpSelectionMerge %126 None
                 OpBranchConditional %124 %125 %126

        %125 =     OpLabel
        %128 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %129 =       OpLoad %v4float %128           ; RelaxedPrecision
        %130 =       OpVectorShuffle %v3float %129 %129 0 1 2   ; RelaxedPrecision
        %131 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %132 =       OpLoad %v4float %131           ; RelaxedPrecision
        %133 =       OpVectorShuffle %v3float %132 %132 0 1 2   ; RelaxedPrecision
        %127 =       OpExtInst %v3float %5 FMax %130 %133       ; RelaxedPrecision
        %134 =       OpVectorShuffle %v3float %35 %35 0 1 2     ; RelaxedPrecision
        %135 =       OpFOrdEqual %v3bool %127 %134
        %136 =       OpAll %bool %135
                     OpBranch %126

        %126 = OpLabel
        %137 =   OpPhi %bool %false %113 %136 %125
                 OpSelectionMerge %139 None
                 OpBranchConditional %137 %138 %139

        %138 =     OpLabel
        %141 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %142 =       OpLoad %v4float %141           ; RelaxedPrecision
        %143 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %144 =       OpLoad %v4float %143           ; RelaxedPrecision
        %140 =       OpExtInst %v4float %5 FMax %142 %144   ; RelaxedPrecision
        %145 =       OpFOrdEqual %v4bool %140 %35
        %146 =       OpAll %bool %145
                     OpBranch %139

        %139 = OpLabel
        %147 =   OpPhi %bool %false %126 %146 %138
                 OpSelectionMerge %149 None
                 OpBranchConditional %147 %148 %149

        %148 =     OpLabel
                     OpBranch %149

        %149 = OpLabel
        %150 =   OpPhi %bool %false %139 %true %148
                 OpSelectionMerge %152 None
                 OpBranchConditional %150 %151 %152

        %151 =     OpLabel
        %154 =       OpVectorShuffle %v2float %35 %35 0 1   ; RelaxedPrecision
        %155 =       OpFOrdEqual %v2bool %153 %154
        %156 =       OpAll %bool %155
                     OpBranch %152

        %152 = OpLabel
        %157 =   OpPhi %bool %false %149 %156 %151
                 OpSelectionMerge %159 None
                 OpBranchConditional %157 %158 %159

        %158 =     OpLabel
        %161 =       OpVectorShuffle %v3float %35 %35 0 1 2     ; RelaxedPrecision
        %162 =       OpFOrdEqual %v3bool %160 %161
        %163 =       OpAll %bool %162
                     OpBranch %159

        %159 = OpLabel
        %164 =   OpPhi %bool %false %152 %163 %158
                 OpSelectionMerge %166 None
                 OpBranchConditional %164 %165 %166

        %165 =     OpLabel
                     OpBranch %166

        %166 = OpLabel
        %167 =   OpPhi %bool %false %159 %true %165
                 OpSelectionMerge %171 None
                 OpBranchConditional %167 %169 %170

        %169 =     OpLabel
        %172 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %173 =       OpLoad %v4float %172           ; RelaxedPrecision
                     OpStore %168 %173
                     OpBranch %171

        %170 =     OpLabel
        %174 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %176 =       OpLoad %v4float %174           ; RelaxedPrecision
                     OpStore %168 %176
                     OpBranch %171

        %171 = OpLabel
        %177 =   OpLoad %v4float %168               ; RelaxedPrecision
                 OpReturnValue %177
               OpFunctionEnd
