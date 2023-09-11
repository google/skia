               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %S "S"
               OpMemberName %S 0 "rgb"
               OpMemberName %S 1 "a"
               OpName %s "s"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %_arr_float_int_3 ArrayStride 16
               OpMemberDecorate %S 0 Offset 0
               OpMemberDecorate %S 0 RelaxedPrecision
               OpMemberDecorate %S 1 Offset 48
               OpMemberDecorate %S 1 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
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
          %S = OpTypeStruct %_arr_float_int_3 %float
%_ptr_Function_S = OpTypePointer Function %S
      %int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
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
          %s = OpVariable %_ptr_Function_S Function
         %27 = OpAccessChain %_ptr_Function_float %s %int_0 %int_0
               OpStore %27 %float_0
         %31 = OpAccessChain %_ptr_Function_float %s %int_0 %int_1
               OpStore %31 %float_1
         %33 = OpAccessChain %_ptr_Function_float %s %int_0 %int_2
               OpStore %33 %float_0
         %34 = OpAccessChain %_ptr_Function_float %s %int_1
               OpStore %34 %float_1
         %35 = OpAccessChain %_ptr_Function_float %s %int_0 %int_0
         %36 = OpLoad %float %35
         %37 = OpAccessChain %_ptr_Function_float %s %int_0 %int_1
         %38 = OpLoad %float %37
         %39 = OpAccessChain %_ptr_Function_float %s %int_0 %int_2
         %40 = OpLoad %float %39
         %41 = OpAccessChain %_ptr_Function_float %s %int_1
         %42 = OpLoad %float %41
         %43 = OpCompositeConstruct %v4float %36 %38 %40 %42
               OpReturnValue %43
               OpFunctionEnd
