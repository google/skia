               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %13
               OpMemberName %_UniformBuffer 0 "a"
               OpMemberName %_UniformBuffer 1 "b"
               OpName %main "main"                  ; id %6

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 1 Offset 4
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %23 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %uint = OpTypeInt 32 0
%_UniformBuffer = OpTypeStruct %int %uint           ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
%_ptr_Uniform_int = OpTypePointer Uniform %int
      %int_0 = OpConstant %int 0
%_ptr_Output_float = OpTypePointer Output %float
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
      %int_1 = OpConstant %int 1


               ; Function main
       %main = OpFunction %void None %16

         %17 = OpLabel
         %19 =   OpAccessChain %_ptr_Uniform_int %11 %int_0
         %22 =   OpLoad %int %19
         %18 =   OpExtInst %int %5 FindSMsb %22
         %23 =   OpConvertSToF %float %18           ; RelaxedPrecision
         %24 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
                 OpStore %24 %23
         %27 =   OpAccessChain %_ptr_Uniform_uint %11 %int_1
         %30 =   OpLoad %uint %27
         %26 =   OpExtInst %int %5 FindUMsb %30
         %31 =   OpConvertSToF %float %26           ; RelaxedPrecision
         %32 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
                 OpStore %32 %31
                 OpReturn
               OpFunctionEnd
