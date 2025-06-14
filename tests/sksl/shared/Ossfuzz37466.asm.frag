               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %foo_ff "foo_ff"              ; id %6
               OpName %main "main"                  ; id %7
               OpName %y "y"                        ; id %24

               ; Annotations
               OpDecorate %_arr_float_int_2 ArrayStride 16

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2       ; ArrayStride 16
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
         %12 = OpTypeFunction %float %_ptr_Function__arr_float_int_2
      %int_1 = OpConstant %int 1
%_ptr_Function_float = OpTypePointer Function %float
      %int_0 = OpConstant %int 0
       %void = OpTypeVoid
         %22 = OpTypeFunction %void


               ; Function foo_ff
     %foo_ff = OpFunction %float None %12
         %13 = OpFunctionParameter %_ptr_Function__arr_float_int_2

         %14 = OpLabel
         %16 =   OpAccessChain %_ptr_Function_float %13 %int_1
         %18 =   OpLoad %float %16
         %20 =   OpAccessChain %_ptr_Function_float %13 %int_0
                 OpStore %20 %18
                 OpReturnValue %18
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %22

         %23 = OpLabel
          %y =   OpVariable %_ptr_Function__arr_float_int_2 Function
         %26 =   OpVariable %_ptr_Function__arr_float_int_2 Function
         %25 =   OpLoad %_arr_float_int_2 %y
                 OpStore %26 %25
         %27 =   OpFunctionCall %float %foo_ff %26
                 OpReturn
               OpFunctionEnd
