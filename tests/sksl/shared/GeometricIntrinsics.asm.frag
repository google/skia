               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %_0_x "_0_x"
               OpName %_1_x "_1_x"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %50 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
         %35 = OpConstantComposite %v2float %float_1 %float_2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
         %41 = OpConstantComposite %v2float %float_3 %float_4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %15
         %16 = OpLabel
         %20 = OpVariable %_ptr_Function_v2float Function
               OpStore %20 %19
         %22 = OpFunctionCall %v4float %main %20
               OpStore %sk_FragColor %22
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %23
         %24 = OpFunctionParameter %_ptr_Function_v2float
         %25 = OpLabel
       %_0_x = OpVariable %_ptr_Function_float Function
       %_1_x = OpVariable %_ptr_Function_v2float Function
               OpStore %_0_x %float_1
         %29 = OpExtInst %float %1 Length %float_1
               OpStore %_0_x %29
         %30 = OpExtInst %float %1 Distance %29 %float_2
               OpStore %_0_x %30
         %32 = OpFMul %float %30 %float_2
               OpStore %_0_x %32
         %33 = OpExtInst %float %1 Normalize %32
               OpStore %_0_x %33
               OpStore %_1_x %35
         %36 = OpExtInst %float %1 Length %35
         %37 = OpCompositeConstruct %v2float %36 %36
               OpStore %_1_x %37
         %38 = OpExtInst %float %1 Distance %37 %41
         %42 = OpCompositeConstruct %v2float %38 %38
               OpStore %_1_x %42
         %43 = OpDot %float %42 %41
         %44 = OpCompositeConstruct %v2float %43 %43
               OpStore %_1_x %44
         %45 = OpExtInst %v2float %1 Normalize %44
               OpStore %_1_x %45
         %46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %50 = OpLoad %v4float %46
               OpReturnValue %50
               OpFunctionEnd
