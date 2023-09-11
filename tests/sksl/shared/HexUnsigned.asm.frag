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
               OpName %u1 "u1"
               OpName %u2 "u2"
               OpName %u3 "u3"
               OpName %u4 "u4"
               OpName %u5 "u5"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %u5 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
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
         %u1 = OpVariable %_ptr_Function_uint Function
         %u2 = OpVariable %_ptr_Function_uint Function
         %u3 = OpVariable %_ptr_Function_uint Function
         %u4 = OpVariable %_ptr_Function_uint Function
         %u5 = OpVariable %_ptr_Function_uint Function
               OpStore %u1 %uint_0
         %31 = OpIAdd %uint %uint_0 %uint_1
               OpStore %u1 %31
               OpStore %u2 %uint_305441741
         %34 = OpIAdd %uint %uint_305441741 %uint_1
               OpStore %u2 %34
               OpStore %u3 %uint_2147483646
         %37 = OpIAdd %uint %uint_2147483646 %uint_1
               OpStore %u3 %37
               OpStore %u4 %uint_4294967294
         %40 = OpIAdd %uint %uint_4294967294 %uint_1
               OpStore %u4 %40
               OpStore %u5 %uint_65534
         %43 = OpIAdd %uint %uint_65534 %uint_1
               OpStore %u5 %43
         %44 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %48 = OpLoad %v4float %44
               OpReturnValue %48
               OpFunctionEnd
