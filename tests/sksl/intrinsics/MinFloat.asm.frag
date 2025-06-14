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
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %132 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %144 RelaxedPrecision
               OpDecorate %172 RelaxedPrecision
               OpDecorate %175 RelaxedPrecision
               OpDecorate %176 RelaxedPrecision

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
%float_n1_25 = OpConstant %float -1.25
  %float_0_5 = OpConstant %float 0.5
         %31 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_5 %float_0_5
    %float_1 = OpConstant %float 1
         %34 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0 %float_1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
         %50 = OpConstantComposite %v2float %float_0_5 %float_0_5
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
         %63 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
     %v3bool = OpTypeVector %bool 3
         %74 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
         %85 = OpConstantComposite %v2float %float_n1_25 %float_0
         %92 = OpConstantComposite %v3float %float_n1_25 %float_0 %float_0_5
      %int_1 = OpConstant %int 1
        %159 = OpConstantComposite %v3float %float_n1_25 %float_0 %float_0
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
  %expectedA =   OpVariable %_ptr_Function_v4float Function
  %expectedB =   OpVariable %_ptr_Function_v4float Function
        %167 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %expectedA %31
                 OpStore %expectedB %34
         %38 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %41 =   OpLoad %v4float %38
         %42 =   OpCompositeExtract %float %41 0
         %37 =   OpExtInst %float %5 FMin %42 %float_0_5
         %43 =   OpFOrdEqual %bool %37 %float_n1_25
                 OpSelectionMerge %45 None
                 OpBranchConditional %43 %44 %45

         %44 =     OpLabel
         %47 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %48 =       OpLoad %v4float %47
         %49 =       OpVectorShuffle %v2float %48 %48 0 1
         %46 =       OpExtInst %v2float %5 FMin %49 %50
         %51 =       OpVectorShuffle %v2float %31 %31 0 1
         %52 =       OpFOrdEqual %v2bool %46 %51
         %54 =       OpAll %bool %52
                     OpBranch %45

         %45 = OpLabel
         %55 =   OpPhi %bool %false %26 %54 %44
                 OpSelectionMerge %57 None
                 OpBranchConditional %55 %56 %57

         %56 =     OpLabel
         %59 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %60 =       OpLoad %v4float %59
         %61 =       OpVectorShuffle %v3float %60 %60 0 1 2
         %58 =       OpExtInst %v3float %5 FMin %61 %63
         %64 =       OpVectorShuffle %v3float %31 %31 0 1 2
         %65 =       OpFOrdEqual %v3bool %58 %64
         %67 =       OpAll %bool %65
                     OpBranch %57

         %57 = OpLabel
         %68 =   OpPhi %bool %false %45 %67 %56
                 OpSelectionMerge %70 None
                 OpBranchConditional %68 %69 %70

         %69 =     OpLabel
         %72 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %73 =       OpLoad %v4float %72
         %71 =       OpExtInst %v4float %5 FMin %73 %74
         %75 =       OpFOrdEqual %v4bool %71 %31
         %77 =       OpAll %bool %75
                     OpBranch %70

         %70 = OpLabel
         %78 =   OpPhi %bool %false %57 %77 %69
                 OpSelectionMerge %80 None
                 OpBranchConditional %78 %79 %80

         %79 =     OpLabel
                     OpBranch %80

         %80 = OpLabel
         %82 =   OpPhi %bool %false %70 %true %79
                 OpSelectionMerge %84 None
                 OpBranchConditional %82 %83 %84

         %83 =     OpLabel
         %86 =       OpVectorShuffle %v2float %31 %31 0 1
         %87 =       OpFOrdEqual %v2bool %85 %86
         %88 =       OpAll %bool %87
                     OpBranch %84

         %84 = OpLabel
         %89 =   OpPhi %bool %false %80 %88 %83
                 OpSelectionMerge %91 None
                 OpBranchConditional %89 %90 %91

         %90 =     OpLabel
         %93 =       OpVectorShuffle %v3float %31 %31 0 1 2
         %94 =       OpFOrdEqual %v3bool %92 %93
         %95 =       OpAll %bool %94
                     OpBranch %91

         %91 = OpLabel
         %96 =   OpPhi %bool %false %84 %95 %90
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
        %104 =       OpLoad %v4float %103
        %105 =       OpCompositeExtract %float %104 0
        %106 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %108 =       OpLoad %v4float %106           ; RelaxedPrecision
        %109 =       OpCompositeExtract %float %108 0   ; RelaxedPrecision
        %102 =       OpExtInst %float %5 FMin %105 %109
        %110 =       OpFOrdEqual %bool %102 %float_n1_25
                     OpBranch %101

        %101 = OpLabel
        %111 =   OpPhi %bool %false %98 %110 %100
                 OpSelectionMerge %113 None
                 OpBranchConditional %111 %112 %113

        %112 =     OpLabel
        %115 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %116 =       OpLoad %v4float %115
        %117 =       OpVectorShuffle %v2float %116 %116 0 1
        %118 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %119 =       OpLoad %v4float %118           ; RelaxedPrecision
        %120 =       OpVectorShuffle %v2float %119 %119 0 1     ; RelaxedPrecision
        %114 =       OpExtInst %v2float %5 FMin %117 %120
        %121 =       OpVectorShuffle %v2float %34 %34 0 1
        %122 =       OpFOrdEqual %v2bool %114 %121
        %123 =       OpAll %bool %122
                     OpBranch %113

        %113 = OpLabel
        %124 =   OpPhi %bool %false %101 %123 %112
                 OpSelectionMerge %126 None
                 OpBranchConditional %124 %125 %126

        %125 =     OpLabel
        %128 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %129 =       OpLoad %v4float %128
        %130 =       OpVectorShuffle %v3float %129 %129 0 1 2
        %131 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %132 =       OpLoad %v4float %131           ; RelaxedPrecision
        %133 =       OpVectorShuffle %v3float %132 %132 0 1 2   ; RelaxedPrecision
        %127 =       OpExtInst %v3float %5 FMin %130 %133
        %134 =       OpVectorShuffle %v3float %34 %34 0 1 2
        %135 =       OpFOrdEqual %v3bool %127 %134
        %136 =       OpAll %bool %135
                     OpBranch %126

        %126 = OpLabel
        %137 =   OpPhi %bool %false %113 %136 %125
                 OpSelectionMerge %139 None
                 OpBranchConditional %137 %138 %139

        %138 =     OpLabel
        %141 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %142 =       OpLoad %v4float %141
        %143 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %144 =       OpLoad %v4float %143           ; RelaxedPrecision
        %140 =       OpExtInst %v4float %5 FMin %142 %144
        %145 =       OpFOrdEqual %v4bool %140 %34
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
        %153 =       OpVectorShuffle %v2float %34 %34 0 1
        %154 =       OpFOrdEqual %v2bool %85 %153
        %155 =       OpAll %bool %154
                     OpBranch %152

        %152 = OpLabel
        %156 =   OpPhi %bool %false %149 %155 %151
                 OpSelectionMerge %158 None
                 OpBranchConditional %156 %157 %158

        %157 =     OpLabel
        %160 =       OpVectorShuffle %v3float %34 %34 0 1 2
        %161 =       OpFOrdEqual %v3bool %159 %160
        %162 =       OpAll %bool %161
                     OpBranch %158

        %158 = OpLabel
        %163 =   OpPhi %bool %false %152 %162 %157
                 OpSelectionMerge %165 None
                 OpBranchConditional %163 %164 %165

        %164 =     OpLabel
                     OpBranch %165

        %165 = OpLabel
        %166 =   OpPhi %bool %false %158 %true %164
                 OpSelectionMerge %170 None
                 OpBranchConditional %166 %168 %169

        %168 =     OpLabel
        %171 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %172 =       OpLoad %v4float %171           ; RelaxedPrecision
                     OpStore %167 %172
                     OpBranch %170

        %169 =     OpLabel
        %173 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %175 =       OpLoad %v4float %173           ; RelaxedPrecision
                     OpStore %167 %175
                     OpBranch %170

        %170 = OpLabel
        %176 =   OpLoad %v4float %167               ; RelaxedPrecision
                 OpReturnValue %176
               OpFunctionEnd
