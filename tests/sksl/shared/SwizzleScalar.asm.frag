               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %h4 "h4"                          ; id %27

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
               OpDecorate %h4 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %float                   ; Block
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
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1


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
         %h4 =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %29 =   OpAccessChain %_ptr_Uniform_float %11 %int_0
         %32 =   OpLoad %float %29                  ; RelaxedPrecision
         %33 =   OpCompositeConstruct %v4float %32 %32 %32 %32  ; RelaxedPrecision
                 OpStore %h4 %33
         %34 =   OpAccessChain %_ptr_Uniform_float %11 %int_0
         %35 =   OpLoad %float %34                  ; RelaxedPrecision
         %36 =   OpCompositeConstruct %v2float %35 %35  ; RelaxedPrecision
         %37 =   OpCompositeExtract %float %36 0        ; RelaxedPrecision
         %38 =   OpCompositeExtract %float %36 1        ; RelaxedPrecision
         %40 =   OpCompositeConstruct %v4float %37 %38 %float_0 %float_1    ; RelaxedPrecision
                 OpStore %h4 %40
         %41 =   OpAccessChain %_ptr_Uniform_float %11 %int_0
         %42 =   OpLoad %float %41                  ; RelaxedPrecision
         %43 =   OpCompositeConstruct %v4float %float_0 %42 %float_1 %float_0   ; RelaxedPrecision
                 OpStore %h4 %43
         %44 =   OpAccessChain %_ptr_Uniform_float %11 %int_0
         %45 =   OpLoad %float %44                  ; RelaxedPrecision
         %46 =   OpAccessChain %_ptr_Uniform_float %11 %int_0
         %47 =   OpLoad %float %46                  ; RelaxedPrecision
         %48 =   OpCompositeConstruct %v4float %float_0 %45 %float_0 %47    ; RelaxedPrecision
                 OpStore %h4 %48
                 OpReturnValue %48
               OpFunctionEnd
