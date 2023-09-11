               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor %sk_SampleMask %sk_SampleMaskIn
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %sk_SampleMask "sk_SampleMask"
               OpName %sk_SampleMaskIn "sk_SampleMaskIn"
               OpName %samplemaskin_as_color_h4 "samplemaskin_as_color_h4"
               OpName %clear_samplemask_v "clear_samplemask_v"
               OpName %reset_samplemask_v "reset_samplemask_v"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %_arr_uint_int_1 ArrayStride 16
               OpDecorate %sk_SampleMask BuiltIn SampleMask
               OpDecorate %sk_SampleMaskIn BuiltIn SampleMask
               OpDecorate %24 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
%_arr_uint_int_1 = OpTypeArray %uint %int_1
%_ptr_Output__arr_uint_int_1 = OpTypePointer Output %_arr_uint_int_1
%sk_SampleMask = OpVariable %_ptr_Output__arr_uint_int_1 Output
%_ptr_Input__arr_uint_int_1 = OpTypePointer Input %_arr_uint_int_1
%sk_SampleMaskIn = OpVariable %_ptr_Input__arr_uint_int_1 Input
         %18 = OpTypeFunction %v4float
%_ptr_Input_uint = OpTypePointer Input %uint
      %int_0 = OpConstant %int 0
       %void = OpTypeVoid
         %27 = OpTypeFunction %void
     %uint_0 = OpConstant %uint 0
%_ptr_Output_uint = OpTypePointer Output %uint
%uint_4294967295 = OpConstant %uint 4294967295
%float_0_00390625 = OpConstant %float 0.00390625
%samplemaskin_as_color_h4 = OpFunction %v4float None %18
         %19 = OpLabel
         %20 = OpAccessChain %_ptr_Input_uint %sk_SampleMaskIn %int_0
         %23 = OpLoad %uint %20
         %24 = OpConvertUToF %float %23
         %25 = OpCompositeConstruct %v4float %24 %24 %24 %24
               OpReturnValue %25
               OpFunctionEnd
%clear_samplemask_v = OpFunction %void None %27
         %28 = OpLabel
         %30 = OpAccessChain %_ptr_Output_uint %sk_SampleMask %int_0
               OpStore %30 %uint_0
               OpReturn
               OpFunctionEnd
%reset_samplemask_v = OpFunction %void None %27
         %32 = OpLabel
         %33 = OpAccessChain %_ptr_Input_uint %sk_SampleMaskIn %int_0
         %34 = OpLoad %uint %33
         %35 = OpAccessChain %_ptr_Output_uint %sk_SampleMask %int_0
               OpStore %35 %34
               OpReturn
               OpFunctionEnd
       %main = OpFunction %void None %27
         %36 = OpLabel
         %37 = OpFunctionCall %void %clear_samplemask_v
         %38 = OpFunctionCall %void %reset_samplemask_v
         %40 = OpAccessChain %_ptr_Output_uint %sk_SampleMask %int_0
               OpStore %40 %uint_4294967295
         %41 = OpFunctionCall %v4float %samplemaskin_as_color_h4
         %43 = OpVectorTimesScalar %v4float %41 %float_0_00390625
               OpStore %sk_FragColor %43
               OpReturn
               OpFunctionEnd
