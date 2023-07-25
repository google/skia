               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %i "i"
               OpName %i4 "i4"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %32 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_0 = OpConstant %int 0
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
      %v2int = OpTypeVector %int 2
      %int_1 = OpConstant %int 1
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
          %i = OpVariable %_ptr_Function_int Function
         %i4 = OpVariable %_ptr_Function_v4int Function
         %29 = OpAccessChain %_ptr_Uniform_float %10 %int_0
         %32 = OpLoad %float %29
         %33 = OpConvertFToS %int %32
               OpStore %i %33
         %37 = OpCompositeConstruct %v4int %33 %33 %33 %33
               OpStore %i4 %37
         %39 = OpCompositeConstruct %v2int %33 %33
         %40 = OpCompositeExtract %int %39 0
         %41 = OpCompositeExtract %int %39 1
         %43 = OpCompositeConstruct %v4int %40 %41 %int_0 %int_1
               OpStore %i4 %43
         %44 = OpCompositeConstruct %v4int %int_0 %33 %int_1 %int_0
               OpStore %i4 %44
         %45 = OpCompositeConstruct %v4int %int_0 %33 %int_0 %33
               OpStore %i4 %45
         %46 = OpConvertSToF %float %int_0
         %47 = OpCompositeExtract %int %45 1
         %48 = OpConvertSToF %float %47
         %49 = OpCompositeExtract %int %45 2
         %50 = OpConvertSToF %float %49
         %51 = OpCompositeExtract %int %45 3
         %52 = OpConvertSToF %float %51
         %53 = OpCompositeConstruct %v4float %46 %48 %50 %52
               OpReturnValue %53
               OpFunctionEnd
