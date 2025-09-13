               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %value "value"                    ; id %27
               OpName %ok "ok"                          ; id %34
               OpName %whole "whole"                    ; id %40
               OpName %fraction "fraction"              ; id %41

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %128 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %137 RelaxedPrecision
               OpDecorate %138 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
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
  %float_2_5 = OpConstant %float 2.5
 %float_n2_5 = OpConstant %float -2.5
    %float_8 = OpConstant %float 8
%float_n0_125 = OpConstant %float -0.125
         %33 = OpConstantComposite %v4float %float_2_5 %float_n2_5 %float_8 %float_n0_125
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
      %false = OpConstantFalse %bool
         %39 = OpConstantComposite %v4bool %false %false %false %false
%_ptr_Function_float = OpTypePointer Function %float
      %int_0 = OpConstant %int 0
    %float_2 = OpConstant %float 2
  %float_0_5 = OpConstant %float 0.5
%_ptr_Function_bool = OpTypePointer Function %bool
   %float_n2 = OpConstant %float -2
         %73 = OpConstantComposite %v2float %float_2 %float_n2
     %v2bool = OpTypeVector %bool 2
 %float_n0_5 = OpConstant %float -0.5
         %81 = OpConstantComposite %v2float %float_0_5 %float_n0_5
      %int_1 = OpConstant %int 1
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %99 = OpConstantComposite %v3float %float_2 %float_n2 %float_8
     %v3bool = OpTypeVector %bool 3
        %106 = OpConstantComposite %v3float %float_0_5 %float_n0_5 %float_0
      %int_2 = OpConstant %int 2
        %116 = OpConstantComposite %v4float %float_2 %float_n2 %float_8 %float_0
        %121 = OpConstantComposite %v4float %float_0_5 %float_n0_5 %float_0 %float_n0_125
      %int_3 = OpConstant %int 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float


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
      %value =   OpVariable %_ptr_Function_v4float Function
         %ok =   OpVariable %_ptr_Function_v4bool Function
      %whole =   OpVariable %_ptr_Function_v4float Function
   %fraction =   OpVariable %_ptr_Function_v4float Function
         %46 =   OpVariable %_ptr_Function_float Function
         %65 =   OpVariable %_ptr_Function_v2float Function
         %91 =   OpVariable %_ptr_Function_v3float Function
        %114 =   OpVariable %_ptr_Function_v4float Function
        %129 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %value %33
                 OpStore %ok %39
         %43 =   OpAccessChain %_ptr_Function_float %whole %int_0
         %42 =   OpExtInst %float %5 Modf %float_2_5 %46
         %47 =   OpLoad %float %46
                 OpStore %43 %47
         %48 =   OpAccessChain %_ptr_Function_float %fraction %int_0
                 OpStore %48 %42
         %49 =   OpLoad %v4float %whole
         %50 =   OpCompositeExtract %float %49 0
         %52 =   OpFOrdEqual %bool %50 %float_2
                 OpSelectionMerge %54 None
                 OpBranchConditional %52 %53 %54

         %53 =     OpLabel
         %55 =       OpLoad %v4float %fraction
         %56 =       OpCompositeExtract %float %55 0
         %58 =       OpFOrdEqual %bool %56 %float_0_5
                     OpBranch %54

         %54 = OpLabel
         %59 =   OpPhi %bool %false %26 %58 %53
         %60 =   OpAccessChain %_ptr_Function_bool %ok %int_0
                 OpStore %60 %59
         %63 =   OpLoad %v4float %value
         %64 =   OpVectorShuffle %v2float %63 %63 0 1
         %62 =   OpExtInst %v2float %5 Modf %64 %65
         %66 =   OpLoad %v2float %65
         %67 =   OpLoad %v4float %whole
         %68 =   OpVectorShuffle %v4float %67 %66 4 5 2 3
                 OpStore %whole %68
         %69 =   OpLoad %v4float %fraction
         %70 =   OpVectorShuffle %v4float %69 %62 4 5 2 3
                 OpStore %fraction %70
         %71 =   OpVectorShuffle %v2float %68 %68 0 1
         %74 =   OpFOrdEqual %v2bool %71 %73
         %76 =   OpAll %bool %74
                 OpSelectionMerge %78 None
                 OpBranchConditional %76 %77 %78

         %77 =     OpLabel
         %79 =       OpVectorShuffle %v2float %70 %70 0 1
         %82 =       OpFOrdEqual %v2bool %79 %81
         %83 =       OpAll %bool %82
                     OpBranch %78

         %78 = OpLabel
         %84 =   OpPhi %bool %false %54 %83 %77
         %85 =   OpAccessChain %_ptr_Function_bool %ok %int_1
                 OpStore %85 %84
         %88 =   OpLoad %v4float %value
         %89 =   OpVectorShuffle %v3float %88 %88 0 1 2
         %87 =   OpExtInst %v3float %5 Modf %89 %91
         %93 =   OpLoad %v3float %91
         %94 =   OpLoad %v4float %whole
         %95 =   OpVectorShuffle %v4float %94 %93 4 5 6 3
                 OpStore %whole %95
         %96 =   OpLoad %v4float %fraction
         %97 =   OpVectorShuffle %v4float %96 %87 4 5 6 3
                 OpStore %fraction %97
         %98 =   OpVectorShuffle %v3float %95 %95 0 1 2
        %100 =   OpFOrdEqual %v3bool %98 %99
        %102 =   OpAll %bool %100
                 OpSelectionMerge %104 None
                 OpBranchConditional %102 %103 %104

        %103 =     OpLabel
        %105 =       OpVectorShuffle %v3float %97 %97 0 1 2
        %107 =       OpFOrdEqual %v3bool %105 %106
        %108 =       OpAll %bool %107
                     OpBranch %104

        %104 = OpLabel
        %109 =   OpPhi %bool %false %78 %108 %103
        %110 =   OpAccessChain %_ptr_Function_bool %ok %int_2
                 OpStore %110 %109
        %113 =   OpLoad %v4float %value
        %112 =   OpExtInst %v4float %5 Modf %113 %114
        %115 =   OpLoad %v4float %114
                 OpStore %whole %115
                 OpStore %fraction %112
        %117 =   OpFOrdEqual %v4bool %115 %116
        %118 =   OpAll %bool %117
                 OpSelectionMerge %120 None
                 OpBranchConditional %118 %119 %120

        %119 =     OpLabel
        %122 =       OpFOrdEqual %v4bool %112 %121
        %123 =       OpAll %bool %122
                     OpBranch %120

        %120 = OpLabel
        %124 =   OpPhi %bool %false %104 %123 %119
        %125 =   OpAccessChain %_ptr_Function_bool %ok %int_3
                 OpStore %125 %124
        %128 =   OpLoad %v4bool %ok                 ; RelaxedPrecision
        %127 =   OpAll %bool %128
                 OpSelectionMerge %132 None
                 OpBranchConditional %127 %130 %131

        %130 =     OpLabel
        %133 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %135 =       OpLoad %v4float %133           ; RelaxedPrecision
                     OpStore %129 %135
                     OpBranch %132

        %131 =     OpLabel
        %136 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %137 =       OpLoad %v4float %136           ; RelaxedPrecision
                     OpStore %129 %137
                     OpBranch %132

        %132 = OpLabel
        %138 =   OpLoad %v4float %129               ; RelaxedPrecision
                 OpReturnValue %138
               OpFunctionEnd
