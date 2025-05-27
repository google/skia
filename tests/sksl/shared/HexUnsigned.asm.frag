               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %u1 "u1"
               OpName %u2 "u2"
               OpName %u3 "u3"
               OpName %u4 "u4"
               OpName %u5 "u5"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %u5 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
%uint_305441741 = OpConstant %uint 305441741
%uint_2147483646 = OpConstant %uint 2147483646
%uint_4294967294 = OpConstant %uint 4294967294
 %uint_65534 = OpConstant %uint 65534
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
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
         %u1 = OpVariable %_ptr_Function_uint Function
         %u2 = OpVariable %_ptr_Function_uint Function
         %u3 = OpVariable %_ptr_Function_uint Function
         %u4 = OpVariable %_ptr_Function_uint Function
         %u5 = OpVariable %_ptr_Function_uint Function
               OpStore %u1 %uint_0
         %28 = OpIAdd %uint %uint_0 %uint_1
               OpStore %u1 %28
               OpStore %u2 %uint_305441741
         %31 = OpIAdd %uint %uint_305441741 %uint_1
               OpStore %u2 %31
               OpStore %u3 %uint_2147483646
         %34 = OpIAdd %uint %uint_2147483646 %uint_1
               OpStore %u3 %34
               OpStore %u4 %uint_4294967294
         %37 = OpIAdd %uint %uint_4294967294 %uint_1
               OpStore %u4 %37
               OpStore %u5 %uint_65534
         %40 = OpIAdd %uint %uint_65534 %uint_1
               OpStore %u5 %40
         %41 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %45 = OpLoad %v4float %41
               OpReturnValue %45
               OpFunctionEnd
