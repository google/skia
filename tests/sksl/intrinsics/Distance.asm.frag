               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "pos1"
               OpMemberName %_UniformBuffer 1 "pos2"
               OpMemberName %_UniformBuffer 2 "colorGreen"
               OpMemberName %_UniformBuffer 3 "colorRed"
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
               OpMemberDecorate %_UniformBuffer 3 Offset 48
               OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %expected RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float  ; Block
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
    %float_3 = OpConstant %float 3
    %float_5 = OpConstant %float 5
   %float_13 = OpConstant %float 13
         %32 = OpConstantComposite %v4float %float_3 %float_3 %float_5 %float_13
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
    %v3float = OpTypeVector %float 3
       %true = OpConstantTrue %bool
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3


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
         %91 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %expected %32
         %36 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %39 =   OpLoad %v4float %36                ; RelaxedPrecision
         %40 =   OpCompositeExtract %float %39 0    ; RelaxedPrecision
         %41 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %43 =   OpLoad %v4float %41                ; RelaxedPrecision
         %44 =   OpCompositeExtract %float %43 0    ; RelaxedPrecision
         %35 =   OpExtInst %float %5 Distance %40 %44   ; RelaxedPrecision
         %45 =   OpFOrdEqual %bool %35 %float_3
                 OpSelectionMerge %47 None
                 OpBranchConditional %45 %46 %47

         %46 =     OpLabel
         %49 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %50 =       OpLoad %v4float %49            ; RelaxedPrecision
         %51 =       OpVectorShuffle %v2float %50 %50 0 1   ; RelaxedPrecision
         %52 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %53 =       OpLoad %v4float %52            ; RelaxedPrecision
         %54 =       OpVectorShuffle %v2float %53 %53 0 1   ; RelaxedPrecision
         %48 =       OpExtInst %float %5 Distance %51 %54   ; RelaxedPrecision
         %55 =       OpFOrdEqual %bool %48 %float_3
                     OpBranch %47

         %47 = OpLabel
         %56 =   OpPhi %bool %false %26 %55 %46
                 OpSelectionMerge %58 None
                 OpBranchConditional %56 %57 %58

         %57 =     OpLabel
         %60 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %61 =       OpLoad %v4float %60            ; RelaxedPrecision
         %62 =       OpVectorShuffle %v3float %61 %61 0 1 2     ; RelaxedPrecision
         %64 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %65 =       OpLoad %v4float %64            ; RelaxedPrecision
         %66 =       OpVectorShuffle %v3float %65 %65 0 1 2     ; RelaxedPrecision
         %59 =       OpExtInst %float %5 Distance %62 %66       ; RelaxedPrecision
         %67 =       OpFOrdEqual %bool %59 %float_5
                     OpBranch %58

         %58 = OpLabel
         %68 =   OpPhi %bool %false %47 %67 %57
                 OpSelectionMerge %70 None
                 OpBranchConditional %68 %69 %70

         %69 =     OpLabel
         %72 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %73 =       OpLoad %v4float %72            ; RelaxedPrecision
         %74 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %75 =       OpLoad %v4float %74            ; RelaxedPrecision
         %71 =       OpExtInst %float %5 Distance %73 %75   ; RelaxedPrecision
         %76 =       OpFOrdEqual %bool %71 %float_13
                     OpBranch %70

         %70 = OpLabel
         %77 =   OpPhi %bool %false %58 %76 %69
                 OpSelectionMerge %79 None
                 OpBranchConditional %77 %78 %79

         %78 =     OpLabel
                     OpBranch %79

         %79 = OpLabel
         %81 =   OpPhi %bool %false %70 %true %78
                 OpSelectionMerge %83 None
                 OpBranchConditional %81 %82 %83

         %82 =     OpLabel
                     OpBranch %83

         %83 = OpLabel
         %84 =   OpPhi %bool %false %79 %true %82
                 OpSelectionMerge %86 None
                 OpBranchConditional %84 %85 %86

         %85 =     OpLabel
                     OpBranch %86

         %86 = OpLabel
         %87 =   OpPhi %bool %false %83 %true %85
                 OpSelectionMerge %89 None
                 OpBranchConditional %87 %88 %89

         %88 =     OpLabel
                     OpBranch %89

         %89 = OpLabel
         %90 =   OpPhi %bool %false %86 %true %88
                 OpSelectionMerge %94 None
                 OpBranchConditional %90 %92 %93

         %92 =     OpLabel
         %95 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %97 =       OpLoad %v4float %95            ; RelaxedPrecision
                     OpStore %91 %97
                     OpBranch %94

         %93 =     OpLabel
         %98 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_3
        %100 =       OpLoad %v4float %98            ; RelaxedPrecision
                     OpStore %91 %100
                     OpBranch %94

         %94 = OpLabel
        %101 =   OpLoad %v4float %91                ; RelaxedPrecision
                 OpReturnValue %101
               OpFunctionEnd
