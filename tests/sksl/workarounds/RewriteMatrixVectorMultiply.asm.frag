               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %m44 "m44"
               OpName %v4 "v4"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %m44 RelaxedPrecision
               OpDecorate %v4 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
         %12 = OpTypeFunction %v4float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
  %float_123 = OpConstant %float 123
    %float_0 = OpConstant %float 0
         %19 = OpConstantComposite %v4float %float_123 %float_0 %float_0 %float_0
         %20 = OpConstantComposite %v4float %float_0 %float_123 %float_0 %float_0
         %21 = OpConstantComposite %v4float %float_0 %float_0 %float_123 %float_0
         %22 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_123
         %23 = OpConstantComposite %mat4v4float %19 %20 %21 %22
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %29 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
%_entrypoint_v = OpFunction %void None %9
         %10 = OpLabel
         %11 = OpFunctionCall %v4float %main
               OpStore %sk_FragColor %11
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %12
         %13 = OpLabel
        %m44 = OpVariable %_ptr_Function_mat4v4float Function
         %v4 = OpVariable %_ptr_Function_v4float Function
               OpStore %m44 %23
               OpStore %v4 %29
         %32 = OpAccessChain %_ptr_Function_v4float %m44 %int_0
         %33 = OpLoad %v4float %32
         %34 = OpVectorTimesScalar %v4float %33 %float_0
         %36 = OpAccessChain %_ptr_Function_v4float %m44 %int_1
         %37 = OpLoad %v4float %36
         %38 = OpVectorTimesScalar %v4float %37 %float_1
         %39 = OpFAdd %v4float %34 %38
         %41 = OpAccessChain %_ptr_Function_v4float %m44 %int_2
         %42 = OpLoad %v4float %41
         %43 = OpVectorTimesScalar %v4float %42 %float_2
         %44 = OpFAdd %v4float %39 %43
         %46 = OpAccessChain %_ptr_Function_v4float %m44 %int_3
         %47 = OpLoad %v4float %46
         %48 = OpVectorTimesScalar %v4float %47 %float_3
         %49 = OpFAdd %v4float %44 %48
               OpReturnValue %49
               OpFunctionEnd
