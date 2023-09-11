               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %d_vi "d_vi"
               OpName %b "b"
               OpName %c_vi "c_vi"
               OpName %b_vi "b_vi"
               OpName %a_vi "a_vi"
               OpName %main "main"
               OpName %i "i"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
         %26 = OpTypeFunction %void %_ptr_Function_int
      %int_4 = OpConstant %int 4
         %49 = OpTypeFunction %v4float %_ptr_Function_v2float
         %56 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_entrypoint_v = OpFunction %void None %16
         %17 = OpLabel
         %21 = OpVariable %_ptr_Function_v2float Function
               OpStore %21 %20
         %23 = OpFunctionCall %v4float %main %21
               OpStore %sk_FragColor %23
               OpReturn
               OpFunctionEnd
       %d_vi = OpFunction %void None %26
         %27 = OpFunctionParameter %_ptr_Function_int
         %28 = OpLabel
          %b = OpVariable %_ptr_Function_int Function
               OpStore %b %int_4
               OpReturn
               OpFunctionEnd
       %c_vi = OpFunction %void None %26
         %31 = OpFunctionParameter %_ptr_Function_int
         %32 = OpLabel
         %34 = OpVariable %_ptr_Function_int Function
         %33 = OpLoad %int %31
               OpStore %34 %33
         %35 = OpFunctionCall %void %d_vi %34
               OpReturn
               OpFunctionEnd
       %b_vi = OpFunction %void None %26
         %36 = OpFunctionParameter %_ptr_Function_int
         %37 = OpLabel
         %39 = OpVariable %_ptr_Function_int Function
         %38 = OpLoad %int %36
               OpStore %39 %38
         %40 = OpFunctionCall %void %c_vi %39
               OpReturn
               OpFunctionEnd
       %a_vi = OpFunction %void None %26
         %41 = OpFunctionParameter %_ptr_Function_int
         %42 = OpLabel
         %44 = OpVariable %_ptr_Function_int Function
         %47 = OpVariable %_ptr_Function_int Function
         %43 = OpLoad %int %41
               OpStore %44 %43
         %45 = OpFunctionCall %void %b_vi %44
         %46 = OpLoad %int %41
               OpStore %47 %46
         %48 = OpFunctionCall %void %b_vi %47
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %49
         %50 = OpFunctionParameter %_ptr_Function_v2float
         %51 = OpLabel
          %i = OpVariable %_ptr_Function_int Function
         %54 = OpVariable %_ptr_Function_int Function
         %53 = OpLoad %int %i
               OpStore %54 %53
         %55 = OpFunctionCall %void %a_vi %54
               OpReturnValue %56
               OpFunctionEnd
