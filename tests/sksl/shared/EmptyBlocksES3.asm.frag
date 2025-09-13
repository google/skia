               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "colorWhite"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %color "color"                    ; id %27

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %color RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float                 ; Block
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
         %29 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
%_ptr_Function_float = OpTypePointer Function %float
      %int_1 = OpConstant %int 1
    %float_2 = OpConstant %float 2
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
      %color =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
                 OpStore %color %29
         %30 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %33 =   OpLoad %v4float %30                ; RelaxedPrecision
         %34 =   OpCompositeExtract %float %33 0    ; RelaxedPrecision
         %36 =   OpFOrdEqual %bool %34 %float_1
                 OpSelectionMerge %39 None
                 OpBranchConditional %36 %38 %39

         %38 =     OpLabel
         %40 =       OpAccessChain %_ptr_Function_float %color %int_1
                     OpStore %40 %float_1
                     OpBranch %39

         %39 = OpLabel
         %43 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %44 =   OpLoad %v4float %43                ; RelaxedPrecision
         %45 =   OpCompositeExtract %float %44 0    ; RelaxedPrecision
         %47 =   OpFOrdEqual %bool %45 %float_2
                 OpSelectionMerge %50 None
                 OpBranchConditional %47 %48 %49

         %48 =     OpLabel
                     OpBranch %50

         %49 =     OpLabel
         %51 =       OpAccessChain %_ptr_Function_float %color %int_3
                     OpStore %51 %float_1
                     OpBranch %50

         %50 = OpLabel
                 OpBranch %53

         %53 = OpLabel
                 OpLoopMerge %57 %56 None
                 OpBranch %54

         %54 =     OpLabel
         %58 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %59 =       OpLoad %v4float %58            ; RelaxedPrecision
         %60 =       OpCompositeExtract %float %59 0    ; RelaxedPrecision
         %61 =       OpFOrdEqual %bool %60 %float_2
                     OpBranchConditional %61 %55 %57

         %55 =         OpLabel
                         OpBranch %56

         %56 =   OpLabel
                   OpBranch %53

         %57 = OpLabel
                 OpBranch %62

         %62 = OpLabel
                 OpLoopMerge %66 %65 None
                 OpBranch %63

         %63 =     OpLabel
                     OpBranch %64

         %64 =     OpLabel
                     OpBranch %65

         %65 =   OpLabel
         %67 =     OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %68 =     OpLoad %v4float %67              ; RelaxedPrecision
         %69 =     OpCompositeExtract %float %68 0  ; RelaxedPrecision
         %70 =     OpFOrdEqual %bool %69 %float_2
                   OpBranchConditional %70 %62 %66

         %66 = OpLabel
         %71 =   OpLoad %v4float %color             ; RelaxedPrecision
                 OpReturnValue %71
               OpFunctionEnd
