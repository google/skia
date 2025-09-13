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
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %22 RelaxedPrecision
               OpDecorate %24 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
    %v2float = OpTypeVector %float 2
%_UniformBuffer = OpTypeStruct %v2float %v4float    ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
%_ptr_Output_float = OpTypePointer Output %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1


               ; Function main
       %main = OpFunction %void None %16

         %17 = OpLabel
         %19 =   OpAccessChain %_ptr_Uniform_v2float %11 %int_0
         %22 =   OpLoad %v2float %19                ; RelaxedPrecision
         %18 =   OpExtInst %uint %5 PackHalf2x16 %22
         %24 =   OpConvertUToF %float %18           ; RelaxedPrecision
         %25 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
                 OpStore %25 %24
         %28 =   OpAccessChain %_ptr_Uniform_v2float %11 %int_0
         %29 =   OpLoad %v2float %28                ; RelaxedPrecision
         %27 =   OpExtInst %uint %5 PackUnorm2x16 %29
         %30 =   OpConvertUToF %float %27           ; RelaxedPrecision
         %31 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
                 OpStore %31 %30
         %33 =   OpAccessChain %_ptr_Uniform_v2float %11 %int_0
         %34 =   OpLoad %v2float %33                ; RelaxedPrecision
         %32 =   OpExtInst %uint %5 PackSnorm2x16 %34
         %35 =   OpConvertUToF %float %32           ; RelaxedPrecision
         %36 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
                 OpStore %36 %35
         %38 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %41 =   OpLoad %v4float %38                ; RelaxedPrecision
         %37 =   OpExtInst %uint %5 PackUnorm4x8 %41
         %42 =   OpConvertUToF %float %37           ; RelaxedPrecision
         %43 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
                 OpStore %43 %42
         %45 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %46 =   OpLoad %v4float %45                ; RelaxedPrecision
         %44 =   OpExtInst %uint %5 PackSnorm4x8 %46
         %47 =   OpConvertUToF %float %44           ; RelaxedPrecision
         %48 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
                 OpStore %48 %47
                 OpReturn
               OpFunctionEnd
