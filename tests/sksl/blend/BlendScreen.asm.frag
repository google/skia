               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "src"
               OpMemberName %_UniformBuffer 1 "dst"
               OpName %main "main"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %20 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %24 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
      %int_1 = OpConstant %int 1
       %main = OpFunction %void None %14
         %15 = OpLabel
         %16 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %20 = OpLoad %v4float %16
         %22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %23 = OpLoad %v4float %22
         %25 = OpFSub %v4float %24 %23
         %26 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %28 = OpLoad %v4float %26
         %29 = OpFMul %v4float %25 %28
         %30 = OpFAdd %v4float %20 %29
               OpStore %sk_FragColor %30
               OpReturn
               OpFunctionEnd
