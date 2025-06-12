               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %_UniformBuffer "_UniformBuffer"  ; id %8
               OpMemberName %_UniformBuffer 0 "pos1"
               OpMemberName %_UniformBuffer 1 "pos2"
               OpMemberName %_UniformBuffer 2 "colorGreen"
               OpMemberName %_UniformBuffer 3 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %10
               OpName %main "main"                      ; id %2
               OpName %expected "expected"              ; id %23

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
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %expected RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float  ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_3 = OpConstant %float 3
    %float_5 = OpConstant %float 5
   %float_13 = OpConstant %float 13
         %28 = OpConstantComposite %v4float %float_3 %float_3 %float_5 %float_13
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
    %v3float = OpTypeVector %float 3
       %true = OpConstantTrue %bool
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %12

         %13 = OpLabel
         %17 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %17 %16
         %19 =   OpFunctionCall %v4float %main %17
                 OpStore %sk_FragColor %19
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %20         ; RelaxedPrecision
         %21 = OpFunctionParameter %_ptr_Function_v2float

         %22 = OpLabel
   %expected =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %88 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %expected %28
         %32 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %36 =   OpLoad %v4float %32                ; RelaxedPrecision
         %37 =   OpCompositeExtract %float %36 0    ; RelaxedPrecision
         %38 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %40 =   OpLoad %v4float %38                ; RelaxedPrecision
         %41 =   OpCompositeExtract %float %40 0    ; RelaxedPrecision
         %31 =   OpExtInst %float %1 Distance %37 %41   ; RelaxedPrecision
         %42 =   OpFOrdEqual %bool %31 %float_3
                 OpSelectionMerge %44 None
                 OpBranchConditional %42 %43 %44

         %43 =     OpLabel
         %46 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %47 =       OpLoad %v4float %46            ; RelaxedPrecision
         %48 =       OpVectorShuffle %v2float %47 %47 0 1   ; RelaxedPrecision
         %49 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %50 =       OpLoad %v4float %49            ; RelaxedPrecision
         %51 =       OpVectorShuffle %v2float %50 %50 0 1   ; RelaxedPrecision
         %45 =       OpExtInst %float %1 Distance %48 %51   ; RelaxedPrecision
         %52 =       OpFOrdEqual %bool %45 %float_3
                     OpBranch %44

         %44 = OpLabel
         %53 =   OpPhi %bool %false %22 %52 %43
                 OpSelectionMerge %55 None
                 OpBranchConditional %53 %54 %55

         %54 =     OpLabel
         %57 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %58 =       OpLoad %v4float %57            ; RelaxedPrecision
         %59 =       OpVectorShuffle %v3float %58 %58 0 1 2     ; RelaxedPrecision
         %61 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %62 =       OpLoad %v4float %61            ; RelaxedPrecision
         %63 =       OpVectorShuffle %v3float %62 %62 0 1 2     ; RelaxedPrecision
         %56 =       OpExtInst %float %1 Distance %59 %63       ; RelaxedPrecision
         %64 =       OpFOrdEqual %bool %56 %float_5
                     OpBranch %55

         %55 = OpLabel
         %65 =   OpPhi %bool %false %44 %64 %54
                 OpSelectionMerge %67 None
                 OpBranchConditional %65 %66 %67

         %66 =     OpLabel
         %69 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %70 =       OpLoad %v4float %69            ; RelaxedPrecision
         %71 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %72 =       OpLoad %v4float %71            ; RelaxedPrecision
         %68 =       OpExtInst %float %1 Distance %70 %72   ; RelaxedPrecision
         %73 =       OpFOrdEqual %bool %68 %float_13
                     OpBranch %67

         %67 = OpLabel
         %74 =   OpPhi %bool %false %55 %73 %66
                 OpSelectionMerge %76 None
                 OpBranchConditional %74 %75 %76

         %75 =     OpLabel
                     OpBranch %76

         %76 = OpLabel
         %78 =   OpPhi %bool %false %67 %true %75
                 OpSelectionMerge %80 None
                 OpBranchConditional %78 %79 %80

         %79 =     OpLabel
                     OpBranch %80

         %80 = OpLabel
         %81 =   OpPhi %bool %false %76 %true %79
                 OpSelectionMerge %83 None
                 OpBranchConditional %81 %82 %83

         %82 =     OpLabel
                     OpBranch %83

         %83 = OpLabel
         %84 =   OpPhi %bool %false %80 %true %82
                 OpSelectionMerge %86 None
                 OpBranchConditional %84 %85 %86

         %85 =     OpLabel
                     OpBranch %86

         %86 = OpLabel
         %87 =   OpPhi %bool %false %83 %true %85
                 OpSelectionMerge %91 None
                 OpBranchConditional %87 %89 %90

         %89 =     OpLabel
         %92 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %94 =       OpLoad %v4float %92            ; RelaxedPrecision
                     OpStore %88 %94
                     OpBranch %91

         %90 =     OpLabel
         %95 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_3
         %97 =       OpLoad %v4float %95            ; RelaxedPrecision
                     OpStore %88 %97
                     OpBranch %91

         %91 = OpLabel
         %98 =   OpLoad %v4float %88                ; RelaxedPrecision
                 OpReturnValue %98
               OpFunctionEnd
