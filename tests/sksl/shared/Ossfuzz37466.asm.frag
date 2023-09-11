               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %foo_ff "foo_ff"
               OpName %main "main"
               OpName %y "y"
               OpDecorate %_arr_float_int_2 ArrayStride 16
      %float = OpTypeFloat 32
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
          %9 = OpTypeFunction %float %_ptr_Function__arr_float_int_2
      %int_1 = OpConstant %int 1
%_ptr_Function_float = OpTypePointer Function %float
      %int_0 = OpConstant %int 0
       %void = OpTypeVoid
         %19 = OpTypeFunction %void
     %foo_ff = OpFunction %float None %9
         %10 = OpFunctionParameter %_ptr_Function__arr_float_int_2
         %11 = OpLabel
         %13 = OpAccessChain %_ptr_Function_float %10 %int_1
         %15 = OpLoad %float %13
         %17 = OpAccessChain %_ptr_Function_float %10 %int_0
               OpStore %17 %15
               OpReturnValue %15
               OpFunctionEnd
       %main = OpFunction %void None %19
         %20 = OpLabel
          %y = OpVariable %_ptr_Function__arr_float_int_2 Function
         %23 = OpVariable %_ptr_Function__arr_float_int_2 Function
         %22 = OpLoad %_arr_float_int_2 %y
               OpStore %23 %22
         %24 = OpFunctionCall %float %foo_ff %23
               OpReturn
               OpFunctionEnd
