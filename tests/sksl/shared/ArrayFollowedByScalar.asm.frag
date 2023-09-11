               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %rgb "rgb"
               OpName %a "a"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %rgb RelaxedPrecision
               OpDecorate %_arr_float_int_3 ArrayStride 16
               OpDecorate %a RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %13 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %17 = OpTypeFunction %v4float %_ptr_Function_v2float
        %int = OpTypeInt 32 1
      %int_3 = OpConstant %int 3
%_arr_float_int_3 = OpTypeArray %float %int_3
%_ptr_Function__arr_float_int_3 = OpTypePointer Function %_arr_float_int_3
%_ptr_Function_float = OpTypePointer Function %float
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %9
         %10 = OpLabel
         %14 = OpVariable %_ptr_Function_v2float Function
               OpStore %14 %13
         %16 = OpFunctionCall %v4float %main %14
               OpStore %sk_FragColor %16
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %17
         %18 = OpFunctionParameter %_ptr_Function_v2float
         %19 = OpLabel
        %rgb = OpVariable %_ptr_Function__arr_float_int_3 Function
          %a = OpVariable %_ptr_Function_float Function
         %28 = OpAccessChain %_ptr_Function_float %rgb %int_0
               OpStore %28 %float_0
         %31 = OpAccessChain %_ptr_Function_float %rgb %int_1
               OpStore %31 %float_1
         %33 = OpAccessChain %_ptr_Function_float %rgb %int_2
               OpStore %33 %float_0
               OpStore %a %float_1
         %34 = OpAccessChain %_ptr_Function_float %rgb %int_0
         %35 = OpLoad %float %34
         %36 = OpAccessChain %_ptr_Function_float %rgb %int_1
         %37 = OpLoad %float %36
         %38 = OpAccessChain %_ptr_Function_float %rgb %int_2
         %39 = OpLoad %float %38
         %40 = OpCompositeConstruct %v4float %35 %37 %39 %float_1
               OpReturnValue %40
               OpFunctionEnd
