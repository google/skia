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
               OpName %expected "expected"              ; id %27

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
               OpDecorate %expected RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision

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
%float_n1_5625 = OpConstant %float -1.5625
 %float_0_75 = OpConstant %float 0.75
%float_3_375 = OpConstant %float 3.375
         %32 = OpConstantComposite %v4float %float_n1_5625 %float_0 %float_0_75 %float_3_375
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %50 = OpConstantComposite %v2float %float_2 %float_3
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
    %float_1 = OpConstant %float 1
         %64 = OpConstantComposite %v3float %float_2 %float_3 %float_1
     %v3bool = OpTypeVector %bool 3
  %float_1_5 = OpConstant %float 1.5
         %76 = OpConstantComposite %v4float %float_2 %float_3 %float_1 %float_1_5
     %v4bool = OpTypeVector %bool 4
%float_1_5625 = OpConstant %float 1.5625
         %88 = OpConstantComposite %v2float %float_1_5625 %float_0
         %95 = OpConstantComposite %v3float %float_1_5625 %float_0 %float_0_75
        %102 = OpConstantComposite %v4float %float_1_5625 %float_0 %float_0_75 %float_3_375
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
   %expected =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
        %106 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %expected %32
         %36 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %39 =   OpLoad %v4float %36                ; RelaxedPrecision
         %40 =   OpCompositeExtract %float %39 0    ; RelaxedPrecision
         %35 =   OpExtInst %float %5 Pow %40 %float_2   ; RelaxedPrecision
         %42 =   OpFOrdEqual %bool %35 %float_n1_5625
                 OpSelectionMerge %44 None
                 OpBranchConditional %42 %43 %44

         %43 =     OpLabel
         %46 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %47 =       OpLoad %v4float %46            ; RelaxedPrecision
         %48 =       OpVectorShuffle %v2float %47 %47 0 1   ; RelaxedPrecision
         %45 =       OpExtInst %v2float %5 Pow %48 %50      ; RelaxedPrecision
         %51 =       OpVectorShuffle %v2float %32 %32 0 1   ; RelaxedPrecision
         %52 =       OpFOrdEqual %v2bool %45 %51
         %54 =       OpAll %bool %52
                     OpBranch %44

         %44 = OpLabel
         %55 =   OpPhi %bool %false %26 %54 %43
                 OpSelectionMerge %57 None
                 OpBranchConditional %55 %56 %57

         %56 =     OpLabel
         %59 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %60 =       OpLoad %v4float %59            ; RelaxedPrecision
         %61 =       OpVectorShuffle %v3float %60 %60 0 1 2     ; RelaxedPrecision
         %58 =       OpExtInst %v3float %5 Pow %61 %64          ; RelaxedPrecision
         %65 =       OpVectorShuffle %v3float %32 %32 0 1 2     ; RelaxedPrecision
         %66 =       OpFOrdEqual %v3bool %58 %65
         %68 =       OpAll %bool %66
                     OpBranch %57

         %57 = OpLabel
         %69 =   OpPhi %bool %false %44 %68 %56
                 OpSelectionMerge %71 None
                 OpBranchConditional %69 %70 %71

         %70 =     OpLabel
         %73 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %74 =       OpLoad %v4float %73            ; RelaxedPrecision
         %72 =       OpExtInst %v4float %5 Pow %74 %76  ; RelaxedPrecision
         %77 =       OpFOrdEqual %v4bool %72 %32
         %79 =       OpAll %bool %77
                     OpBranch %71

         %71 = OpLabel
         %80 =   OpPhi %bool %false %57 %79 %70
                 OpSelectionMerge %82 None
                 OpBranchConditional %80 %81 %82

         %81 =     OpLabel
         %84 =       OpFOrdEqual %bool %float_1_5625 %float_n1_5625
                     OpBranch %82

         %82 = OpLabel
         %85 =   OpPhi %bool %false %71 %84 %81
                 OpSelectionMerge %87 None
                 OpBranchConditional %85 %86 %87

         %86 =     OpLabel
         %89 =       OpVectorShuffle %v2float %32 %32 0 1   ; RelaxedPrecision
         %90 =       OpFOrdEqual %v2bool %88 %89
         %91 =       OpAll %bool %90
                     OpBranch %87

         %87 = OpLabel
         %92 =   OpPhi %bool %false %82 %91 %86
                 OpSelectionMerge %94 None
                 OpBranchConditional %92 %93 %94

         %93 =     OpLabel
         %96 =       OpVectorShuffle %v3float %32 %32 0 1 2     ; RelaxedPrecision
         %97 =       OpFOrdEqual %v3bool %95 %96
         %98 =       OpAll %bool %97
                     OpBranch %94

         %94 = OpLabel
         %99 =   OpPhi %bool %false %87 %98 %93
                 OpSelectionMerge %101 None
                 OpBranchConditional %99 %100 %101

        %100 =     OpLabel
        %103 =       OpFOrdEqual %v4bool %102 %32
        %104 =       OpAll %bool %103
                     OpBranch %101

        %101 = OpLabel
        %105 =   OpPhi %bool %false %94 %104 %100
                 OpSelectionMerge %109 None
                 OpBranchConditional %105 %107 %108

        %107 =     OpLabel
        %110 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %112 =       OpLoad %v4float %110           ; RelaxedPrecision
                     OpStore %106 %112
                     OpBranch %109

        %108 =     OpLabel
        %113 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
        %115 =       OpLoad %v4float %113           ; RelaxedPrecision
                     OpStore %106 %115
                     OpBranch %109

        %109 = OpLabel
        %116 =   OpLoad %v4float %106               ; RelaxedPrecision
                 OpReturnValue %116
               OpFunctionEnd
