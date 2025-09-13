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
               OpDecorate %29 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision

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
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
   %float_n1 = OpConstant %float -1
         %43 = OpConstantComposite %v2float %float_n1 %float_0
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
    %float_1 = OpConstant %float 1
         %56 = OpConstantComposite %v3float %float_n1 %float_0 %float_1
     %v3bool = OpTypeVector %bool 3
    %float_2 = OpConstant %float 2
         %67 = OpConstantComposite %v4float %float_n1 %float_0 %float_1 %float_2
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
         %72 =   OpVariable %_ptr_Function_v4float Function
         %30 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %33 =   OpLoad %v4float %30                ; RelaxedPrecision
         %34 =   OpCompositeExtract %float %33 0    ; RelaxedPrecision
         %29 =   OpExtInst %float %5 Round %34      ; RelaxedPrecision
         %36 =   OpFOrdEqual %bool %29 %float_n1
                 OpSelectionMerge %38 None
                 OpBranchConditional %36 %37 %38

         %37 =     OpLabel
         %40 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %41 =       OpLoad %v4float %40            ; RelaxedPrecision
         %42 =       OpVectorShuffle %v2float %41 %41 0 1   ; RelaxedPrecision
         %39 =       OpExtInst %v2float %5 Round %42        ; RelaxedPrecision
         %44 =       OpFOrdEqual %v2bool %39 %43
         %46 =       OpAll %bool %44
                     OpBranch %38

         %38 = OpLabel
         %47 =   OpPhi %bool %false %26 %46 %37
                 OpSelectionMerge %49 None
                 OpBranchConditional %47 %48 %49

         %48 =     OpLabel
         %51 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %52 =       OpLoad %v4float %51            ; RelaxedPrecision
         %53 =       OpVectorShuffle %v3float %52 %52 0 1 2     ; RelaxedPrecision
         %50 =       OpExtInst %v3float %5 Round %53            ; RelaxedPrecision
         %57 =       OpFOrdEqual %v3bool %50 %56
         %59 =       OpAll %bool %57
                     OpBranch %49

         %49 = OpLabel
         %60 =   OpPhi %bool %false %38 %59 %48
                 OpSelectionMerge %62 None
                 OpBranchConditional %60 %61 %62

         %61 =     OpLabel
         %64 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %65 =       OpLoad %v4float %64            ; RelaxedPrecision
         %63 =       OpExtInst %v4float %5 Round %65    ; RelaxedPrecision
         %68 =       OpFOrdEqual %v4bool %63 %67
         %70 =       OpAll %bool %68
                     OpBranch %62

         %62 = OpLabel
         %71 =   OpPhi %bool %false %49 %70 %61
                 OpSelectionMerge %76 None
                 OpBranchConditional %71 %74 %75

         %74 =     OpLabel
         %77 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %79 =       OpLoad %v4float %77            ; RelaxedPrecision
                     OpStore %72 %79
                     OpBranch %76

         %75 =     OpLabel
         %80 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %82 =       OpLoad %v4float %80            ; RelaxedPrecision
                     OpStore %72 %82
                     OpBranch %76

         %76 = OpLabel
         %83 =   OpLoad %v4float %72                ; RelaxedPrecision
                 OpReturnValue %83
               OpFunctionEnd
