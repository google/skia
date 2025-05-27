               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %i "i"
               OpName %i4 "i4"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %29 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_0 = OpConstant %int 0
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
      %v2int = OpTypeVector %int 2
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %12
         %13 = OpLabel
         %17 = OpVariable %_ptr_Function_v2float Function
               OpStore %17 %16
         %19 = OpFunctionCall %v4float %main %17
               OpStore %sk_FragColor %19
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpLabel
          %i = OpVariable %_ptr_Function_int Function
         %i4 = OpVariable %_ptr_Function_v4int Function
         %26 = OpAccessChain %_ptr_Uniform_float %7 %int_0
         %29 = OpLoad %float %26
         %30 = OpConvertFToS %int %29
               OpStore %i %30
         %34 = OpCompositeConstruct %v4int %30 %30 %30 %30
               OpStore %i4 %34
         %36 = OpCompositeConstruct %v2int %30 %30
         %37 = OpCompositeExtract %int %36 0
         %38 = OpCompositeExtract %int %36 1
         %40 = OpCompositeConstruct %v4int %37 %38 %int_0 %int_1
               OpStore %i4 %40
         %41 = OpCompositeConstruct %v4int %int_0 %30 %int_1 %int_0
               OpStore %i4 %41
         %42 = OpCompositeConstruct %v4int %int_0 %30 %int_0 %30
               OpStore %i4 %42
         %43 = OpConvertSToF %float %int_0
         %44 = OpCompositeExtract %int %42 1
         %45 = OpConvertSToF %float %44
         %46 = OpCompositeExtract %int %42 2
         %47 = OpConvertSToF %float %46
         %48 = OpCompositeExtract %int %42 3
         %49 = OpConvertSToF %float %48
         %50 = OpCompositeConstruct %v4float %43 %45 %47 %49
               OpReturnValue %50
               OpFunctionEnd
