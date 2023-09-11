               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor %sk_SampleMask %sk_SampleMaskIn
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %sk_SampleMask "sk_SampleMask"
               OpName %sk_SampleMaskIn "sk_SampleMaskIn"
               OpName %samplemaskin_as_color_h4 "samplemaskin_as_color_h4"
               OpName %clear_samplemask_v "clear_samplemask_v"
               OpName %reset_samplemask_v "reset_samplemask_v"
               OpName %main "main"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %_arr_uint_int_1 ArrayStride 16
               OpDecorate %sk_SampleMask BuiltIn SampleMask
               OpDecorate %sk_SampleMaskIn BuiltIn SampleMask
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
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
         %21 = OpTypeFunction %v4float
%_ptr_Input_uint = OpTypePointer Input %uint
      %int_0 = OpConstant %int 0
       %void = OpTypeVoid
         %30 = OpTypeFunction %void
     %uint_0 = OpConstant %uint 0
%_ptr_Output_uint = OpTypePointer Output %uint
%uint_4294967295 = OpConstant %uint 4294967295
%float_0_00390625 = OpConstant %float 0.00390625
%samplemaskin_as_color_h4 = OpFunction %v4float None %21
         %22 = OpLabel
         %23 = OpAccessChain %_ptr_Input_uint %sk_SampleMaskIn %int_0
         %26 = OpLoad %uint %23
         %27 = OpConvertUToF %float %26
         %28 = OpCompositeConstruct %v4float %27 %27 %27 %27
               OpReturnValue %28
               OpFunctionEnd
%clear_samplemask_v = OpFunction %void None %30
         %31 = OpLabel
         %33 = OpAccessChain %_ptr_Output_uint %sk_SampleMask %int_0
               OpStore %33 %uint_0
               OpReturn
               OpFunctionEnd
%reset_samplemask_v = OpFunction %void None %30
         %35 = OpLabel
         %36 = OpAccessChain %_ptr_Input_uint %sk_SampleMaskIn %int_0
         %37 = OpLoad %uint %36
         %38 = OpAccessChain %_ptr_Output_uint %sk_SampleMask %int_0
               OpStore %38 %37
               OpReturn
               OpFunctionEnd
       %main = OpFunction %void None %30
         %39 = OpLabel
         %40 = OpFunctionCall %void %clear_samplemask_v
         %41 = OpFunctionCall %void %reset_samplemask_v
         %43 = OpAccessChain %_ptr_Output_uint %sk_SampleMask %int_0
               OpStore %43 %uint_4294967295
         %44 = OpFunctionCall %v4float %samplemaskin_as_color_h4
         %46 = OpVectorTimesScalar %v4float %44 %float_0_00390625
               OpStore %sk_FragColor %46
               OpReturn
               OpFunctionEnd
