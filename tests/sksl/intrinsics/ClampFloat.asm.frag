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
               OpDecorate %39 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %136 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision
               OpDecorate %155 RelaxedPrecision
               OpDecorate %162 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
   %float_n1 = OpConstant %float -1
 %float_0_75 = OpConstant %float 0.75
    %float_1 = OpConstant %float 1
         %32 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_75 %float_1
  %float_0_5 = OpConstant %float 0.5
 %float_2_25 = OpConstant %float 2.25
         %36 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_5 %float_2_25
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
         %52 = OpConstantComposite %v2float %float_n1 %float_n1
         %53 = OpConstantComposite %v2float %float_1 %float_1
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
         %66 = OpConstantComposite %v3float %float_n1 %float_n1 %float_n1
         %67 = OpConstantComposite %v3float %float_1 %float_1 %float_1
     %v3bool = OpTypeVector %bool 3
         %78 = OpConstantComposite %v4float %float_n1 %float_n1 %float_n1 %float_n1
         %79 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
     %v4bool = OpTypeVector %bool 4
   %float_n2 = OpConstant %float -2
         %99 = OpConstantComposite %v2float %float_n1 %float_n2
    %float_2 = OpConstant %float 2
        %101 = OpConstantComposite %v2float %float_1 %float_2
        %112 = OpConstantComposite %v3float %float_n1 %float_n2 %float_n2
        %113 = OpConstantComposite %v3float %float_1 %float_2 %float_0_5
        %123 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n2 %float_1
    %float_3 = OpConstant %float 3
        %125 = OpConstantComposite %v4float %float_1 %float_2 %float_0_5 %float_3
       %true = OpConstantTrue %bool
        %135 = OpConstantComposite %v2float %float_n1 %float_0
        %142 = OpConstantComposite %v3float %float_n1 %float_0 %float_0_75
        %161 = OpConstantComposite %v3float %float_n1 %float_0 %float_0_5
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
  %expectedA =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
  %expectedB =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
        %169 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %expectedA %32
                 OpStore %expectedB %36
         %40 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %43 =   OpLoad %v4float %40                ; RelaxedPrecision
         %44 =   OpCompositeExtract %float %43 0    ; RelaxedPrecision
         %39 =   OpExtInst %float %5 FClamp %44 %float_n1 %float_1  ; RelaxedPrecision
         %45 =   OpFOrdEqual %bool %39 %float_n1
                 OpSelectionMerge %47 None
                 OpBranchConditional %45 %46 %47

         %46 =     OpLabel
         %49 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %50 =       OpLoad %v4float %49            ; RelaxedPrecision
         %51 =       OpVectorShuffle %v2float %50 %50 0 1   ; RelaxedPrecision
         %48 =       OpExtInst %v2float %5 FClamp %51 %52 %53   ; RelaxedPrecision
         %54 =       OpVectorShuffle %v2float %32 %32 0 1       ; RelaxedPrecision
         %55 =       OpFOrdEqual %v2bool %48 %54
         %57 =       OpAll %bool %55
                     OpBranch %47

         %47 = OpLabel
         %58 =   OpPhi %bool %false %26 %57 %46
                 OpSelectionMerge %60 None
                 OpBranchConditional %58 %59 %60

         %59 =     OpLabel
         %62 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %63 =       OpLoad %v4float %62            ; RelaxedPrecision
         %64 =       OpVectorShuffle %v3float %63 %63 0 1 2     ; RelaxedPrecision
         %61 =       OpExtInst %v3float %5 FClamp %64 %66 %67   ; RelaxedPrecision
         %68 =       OpVectorShuffle %v3float %32 %32 0 1 2     ; RelaxedPrecision
         %69 =       OpFOrdEqual %v3bool %61 %68
         %71 =       OpAll %bool %69
                     OpBranch %60

         %60 = OpLabel
         %72 =   OpPhi %bool %false %47 %71 %59
                 OpSelectionMerge %74 None
                 OpBranchConditional %72 %73 %74

         %73 =     OpLabel
         %76 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %77 =       OpLoad %v4float %76            ; RelaxedPrecision
         %75 =       OpExtInst %v4float %5 FClamp %77 %78 %79   ; RelaxedPrecision
         %80 =       OpFOrdEqual %v4bool %75 %32
         %82 =       OpAll %bool %80
                     OpBranch %74

         %74 = OpLabel
         %83 =   OpPhi %bool %false %60 %82 %73
                 OpSelectionMerge %85 None
                 OpBranchConditional %83 %84 %85

         %84 =     OpLabel
         %87 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %88 =       OpLoad %v4float %87            ; RelaxedPrecision
         %89 =       OpCompositeExtract %float %88 0    ; RelaxedPrecision
         %86 =       OpExtInst %float %5 FClamp %89 %float_n1 %float_1  ; RelaxedPrecision
         %90 =       OpFOrdEqual %bool %86 %float_n1
                     OpBranch %85

         %85 = OpLabel
         %91 =   OpPhi %bool %false %74 %90 %84
                 OpSelectionMerge %93 None
                 OpBranchConditional %91 %92 %93

         %92 =     OpLabel
         %95 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %96 =       OpLoad %v4float %95            ; RelaxedPrecision
         %97 =       OpVectorShuffle %v2float %96 %96 0 1   ; RelaxedPrecision
         %94 =       OpExtInst %v2float %5 FClamp %97 %99 %101  ; RelaxedPrecision
        %102 =       OpVectorShuffle %v2float %36 %36 0 1       ; RelaxedPrecision
        %103 =       OpFOrdEqual %v2bool %94 %102
        %104 =       OpAll %bool %103
                     OpBranch %93

         %93 = OpLabel
        %105 =   OpPhi %bool %false %85 %104 %92
                 OpSelectionMerge %107 None
                 OpBranchConditional %105 %106 %107

        %106 =     OpLabel
        %109 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %110 =       OpLoad %v4float %109           ; RelaxedPrecision
        %111 =       OpVectorShuffle %v3float %110 %110 0 1 2   ; RelaxedPrecision
        %108 =       OpExtInst %v3float %5 FClamp %111 %112 %113    ; RelaxedPrecision
        %114 =       OpVectorShuffle %v3float %36 %36 0 1 2         ; RelaxedPrecision
        %115 =       OpFOrdEqual %v3bool %108 %114
        %116 =       OpAll %bool %115
                     OpBranch %107

        %107 = OpLabel
        %117 =   OpPhi %bool %false %93 %116 %106
                 OpSelectionMerge %119 None
                 OpBranchConditional %117 %118 %119

        %118 =     OpLabel
        %121 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %122 =       OpLoad %v4float %121           ; RelaxedPrecision
        %120 =       OpExtInst %v4float %5 FClamp %122 %123 %125    ; RelaxedPrecision
        %126 =       OpFOrdEqual %v4bool %120 %36
        %127 =       OpAll %bool %126
                     OpBranch %119

        %119 = OpLabel
        %128 =   OpPhi %bool %false %107 %127 %118
                 OpSelectionMerge %130 None
                 OpBranchConditional %128 %129 %130

        %129 =     OpLabel
                     OpBranch %130

        %130 = OpLabel
        %132 =   OpPhi %bool %false %119 %true %129
                 OpSelectionMerge %134 None
                 OpBranchConditional %132 %133 %134

        %133 =     OpLabel
        %136 =       OpVectorShuffle %v2float %32 %32 0 1   ; RelaxedPrecision
        %137 =       OpFOrdEqual %v2bool %135 %136
        %138 =       OpAll %bool %137
                     OpBranch %134

        %134 = OpLabel
        %139 =   OpPhi %bool %false %130 %138 %133
                 OpSelectionMerge %141 None
                 OpBranchConditional %139 %140 %141

        %140 =     OpLabel
        %143 =       OpVectorShuffle %v3float %32 %32 0 1 2     ; RelaxedPrecision
        %144 =       OpFOrdEqual %v3bool %142 %143
        %145 =       OpAll %bool %144
                     OpBranch %141

        %141 = OpLabel
        %146 =   OpPhi %bool %false %134 %145 %140
                 OpSelectionMerge %148 None
                 OpBranchConditional %146 %147 %148

        %147 =     OpLabel
                     OpBranch %148

        %148 = OpLabel
        %149 =   OpPhi %bool %false %141 %true %147
                 OpSelectionMerge %151 None
                 OpBranchConditional %149 %150 %151

        %150 =     OpLabel
                     OpBranch %151

        %151 = OpLabel
        %152 =   OpPhi %bool %false %148 %true %150
                 OpSelectionMerge %154 None
                 OpBranchConditional %152 %153 %154

        %153 =     OpLabel
        %155 =       OpVectorShuffle %v2float %36 %36 0 1   ; RelaxedPrecision
        %156 =       OpFOrdEqual %v2bool %135 %155
        %157 =       OpAll %bool %156
                     OpBranch %154

        %154 = OpLabel
        %158 =   OpPhi %bool %false %151 %157 %153
                 OpSelectionMerge %160 None
                 OpBranchConditional %158 %159 %160

        %159 =     OpLabel
        %162 =       OpVectorShuffle %v3float %36 %36 0 1 2     ; RelaxedPrecision
        %163 =       OpFOrdEqual %v3bool %161 %162
        %164 =       OpAll %bool %163
                     OpBranch %160

        %160 = OpLabel
        %165 =   OpPhi %bool %false %154 %164 %159
                 OpSelectionMerge %167 None
                 OpBranchConditional %165 %166 %167

        %166 =     OpLabel
                     OpBranch %167

        %167 = OpLabel
        %168 =   OpPhi %bool %false %160 %true %166
                 OpSelectionMerge %172 None
                 OpBranchConditional %168 %170 %171

        %170 =     OpLabel
        %173 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %175 =       OpLoad %v4float %173           ; RelaxedPrecision
                     OpStore %169 %175
                     OpBranch %172

        %171 =     OpLabel
        %176 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %178 =       OpLoad %v4float %176           ; RelaxedPrecision
                     OpStore %169 %178
                     OpBranch %172

        %172 = OpLabel
        %179 =   OpLoad %v4float %169               ; RelaxedPrecision
                 OpReturnValue %179
               OpFunctionEnd
