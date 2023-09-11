               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %foo_ff "foo_ff"
               OpName %main "main"
               OpName %y "y"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %_arr_float_int_2 ArrayStride 16
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
         %12 = OpTypeFunction %float %_ptr_Function__arr_float_int_2
      %int_1 = OpConstant %int 1
%_ptr_Function_float = OpTypePointer Function %float
      %int_0 = OpConstant %int 0
       %void = OpTypeVoid
         %22 = OpTypeFunction %void
     %foo_ff = OpFunction %float None %12
         %13 = OpFunctionParameter %_ptr_Function__arr_float_int_2
         %14 = OpLabel
         %16 = OpAccessChain %_ptr_Function_float %13 %int_1
         %18 = OpLoad %float %16
         %20 = OpAccessChain %_ptr_Function_float %13 %int_0
               OpStore %20 %18
               OpReturnValue %18
               OpFunctionEnd
       %main = OpFunction %void None %22
         %23 = OpLabel
          %y = OpVariable %_ptr_Function__arr_float_int_2 Function
         %26 = OpVariable %_ptr_Function__arr_float_int_2 Function
         %25 = OpLoad %_arr_float_int_2 %y
               OpStore %26 %25
         %27 = OpFunctionCall %float %foo_ff %26
               OpReturn
               OpFunctionEnd
