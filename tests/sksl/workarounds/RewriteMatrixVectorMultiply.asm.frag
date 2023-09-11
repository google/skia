               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %m44 "m44"
               OpName %v4 "v4"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %m44 RelaxedPrecision
               OpDecorate %v4 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
         %15 = OpTypeFunction %v4float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
  %float_123 = OpConstant %float 123
    %float_0 = OpConstant %float 0
         %22 = OpConstantComposite %v4float %float_123 %float_0 %float_0 %float_0
         %23 = OpConstantComposite %v4float %float_0 %float_123 %float_0 %float_0
         %24 = OpConstantComposite %v4float %float_0 %float_0 %float_123 %float_0
         %25 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_123
         %26 = OpConstantComposite %mat4v4float %22 %23 %24 %25
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %32 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
%_entrypoint_v = OpFunction %void None %12
         %13 = OpLabel
         %14 = OpFunctionCall %v4float %main
               OpStore %sk_FragColor %14
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %15
         %16 = OpLabel
        %m44 = OpVariable %_ptr_Function_mat4v4float Function
         %v4 = OpVariable %_ptr_Function_v4float Function
               OpStore %m44 %26
               OpStore %v4 %32
         %35 = OpAccessChain %_ptr_Function_v4float %m44 %int_0
         %36 = OpLoad %v4float %35
         %37 = OpVectorTimesScalar %v4float %36 %float_0
         %39 = OpAccessChain %_ptr_Function_v4float %m44 %int_1
         %40 = OpLoad %v4float %39
         %41 = OpVectorTimesScalar %v4float %40 %float_1
         %42 = OpFAdd %v4float %37 %41
         %44 = OpAccessChain %_ptr_Function_v4float %m44 %int_2
         %45 = OpLoad %v4float %44
         %46 = OpVectorTimesScalar %v4float %45 %float_2
         %47 = OpFAdd %v4float %42 %46
         %49 = OpAccessChain %_ptr_Function_v4float %m44 %int_3
         %50 = OpLoad %v4float %49
         %51 = OpVectorTimesScalar %v4float %50 %float_3
         %52 = OpFAdd %v4float %47 %51
               OpReturnValue %52
               OpFunctionEnd
