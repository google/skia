               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %this_function_is_prototyped_after_its_definition_h4h4 "this_function_is_prototyped_after_its_definition_h4h4"
               OpName %this_function_is_defined_before_use_h4h4 "this_function_is_defined_before_use_h4h4"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %9 Binding 0
               OpDecorate %9 DescriptorSet 0
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %9 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %18 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %23 = OpTypeFunction %v4float %_ptr_Function_v4float
         %35 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %14
         %15 = OpLabel
         %19 = OpVariable %_ptr_Function_v2float Function
               OpStore %19 %18
         %21 = OpFunctionCall %v4float %main %19
               OpStore %sk_FragColor %21
               OpReturn
               OpFunctionEnd
%this_function_is_prototyped_after_its_definition_h4h4 = OpFunction %v4float None %23
         %24 = OpFunctionParameter %_ptr_Function_v4float
         %25 = OpLabel
         %26 = OpLoad %v4float %24
         %27 = OpFNegate %v4float %26
               OpReturnValue %27
               OpFunctionEnd
%this_function_is_defined_before_use_h4h4 = OpFunction %v4float None %23
         %28 = OpFunctionParameter %_ptr_Function_v4float
         %29 = OpLabel
         %32 = OpVariable %_ptr_Function_v4float Function
         %30 = OpLoad %v4float %28
         %31 = OpFNegate %v4float %30
               OpStore %32 %31
         %33 = OpFunctionCall %v4float %this_function_is_prototyped_after_its_definition_h4h4 %32
         %34 = OpFNegate %v4float %33
               OpReturnValue %34
               OpFunctionEnd
       %main = OpFunction %v4float None %35
         %36 = OpFunctionParameter %_ptr_Function_v2float
         %37 = OpLabel
         %44 = OpVariable %_ptr_Function_v4float Function
         %38 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
         %42 = OpLoad %v4float %38
         %43 = OpFNegate %v4float %42
               OpStore %44 %43
         %45 = OpFunctionCall %v4float %this_function_is_defined_before_use_h4h4 %44
               OpReturnValue %45
               OpFunctionEnd
