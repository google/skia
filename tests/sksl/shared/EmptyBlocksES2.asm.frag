               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %_UniformBuffer "_UniformBuffer"  ; id %8
               OpMemberName %_UniformBuffer 0 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %10
               OpName %main "main"                      ; id %2
               OpName %color "color"                    ; id %23

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %color RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %float                   ; Block
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
         %25 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Uniform_float = OpTypePointer Uniform %float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
%_ptr_Function_float = OpTypePointer Function %float
      %int_1 = OpConstant %int 1
    %float_2 = OpConstant %float 2
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
      %color =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
                 OpStore %color %25
         %26 =   OpAccessChain %_ptr_Uniform_float %7 %int_0
         %30 =   OpLoad %float %26                  ; RelaxedPrecision
         %32 =   OpFOrdEqual %bool %30 %float_1
                 OpSelectionMerge %35 None
                 OpBranchConditional %32 %34 %35

         %34 =     OpLabel
         %36 =       OpAccessChain %_ptr_Function_float %color %int_1
                     OpStore %36 %float_1
                     OpBranch %35

         %35 = OpLabel
         %39 =   OpAccessChain %_ptr_Uniform_float %7 %int_0
         %40 =   OpLoad %float %39                  ; RelaxedPrecision
         %42 =   OpFOrdEqual %bool %40 %float_2
                 OpSelectionMerge %45 None
                 OpBranchConditional %42 %43 %44

         %43 =     OpLabel
                     OpBranch %45

         %44 =     OpLabel
         %46 =       OpAccessChain %_ptr_Function_float %color %int_3
                     OpStore %46 %float_1
                     OpBranch %45

         %45 = OpLabel
         %48 =   OpLoad %v4float %color             ; RelaxedPrecision
                 OpReturnValue %48
               OpFunctionEnd
