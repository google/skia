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
               OpName %h4 "h4"                          ; id %23

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
               OpDecorate %h4 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision

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
%_ptr_Uniform_float = OpTypePointer Uniform %float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1


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
         %h4 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %25 =   OpAccessChain %_ptr_Uniform_float %7 %int_0
         %29 =   OpLoad %float %25                  ; RelaxedPrecision
         %30 =   OpCompositeConstruct %v4float %29 %29 %29 %29  ; RelaxedPrecision
                 OpStore %h4 %30
         %31 =   OpAccessChain %_ptr_Uniform_float %7 %int_0
         %32 =   OpLoad %float %31                  ; RelaxedPrecision
         %33 =   OpCompositeConstruct %v2float %32 %32  ; RelaxedPrecision
         %34 =   OpCompositeExtract %float %33 0        ; RelaxedPrecision
         %35 =   OpCompositeExtract %float %33 1        ; RelaxedPrecision
         %37 =   OpCompositeConstruct %v4float %34 %35 %float_0 %float_1    ; RelaxedPrecision
                 OpStore %h4 %37
         %38 =   OpAccessChain %_ptr_Uniform_float %7 %int_0
         %39 =   OpLoad %float %38                  ; RelaxedPrecision
         %40 =   OpCompositeConstruct %v4float %float_0 %39 %float_1 %float_0   ; RelaxedPrecision
                 OpStore %h4 %40
         %41 =   OpAccessChain %_ptr_Uniform_float %7 %int_0
         %42 =   OpLoad %float %41                  ; RelaxedPrecision
         %43 =   OpAccessChain %_ptr_Uniform_float %7 %int_0
         %44 =   OpLoad %float %43                  ; RelaxedPrecision
         %45 =   OpCompositeConstruct %v4float %float_0 %42 %float_0 %44    ; RelaxedPrecision
                 OpStore %h4 %45
                 OpReturnValue %45
               OpFunctionEnd
