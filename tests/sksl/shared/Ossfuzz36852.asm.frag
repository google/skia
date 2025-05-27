               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %x "x"
               OpName %y "y"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %x RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
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
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %26 = OpConstantComposite %v2float %float_0 %float_1
         %27 = OpConstantComposite %v2float %float_2 %float_3
         %28 = OpConstantComposite %mat2v2float %26 %27
         %30 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
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
          %x = OpVariable %_ptr_Function_mat2v2float Function
          %y = OpVariable %_ptr_Function_v2float Function
               OpStore %x %28
         %31 = OpVectorShuffle %v2float %30 %30 0 1
               OpStore %y %31
         %32 = OpCompositeExtract %float %31 0
         %33 = OpCompositeExtract %float %31 1
         %34 = OpCompositeConstruct %v4float %32 %33 %float_0 %float_1
               OpReturnValue %34
               OpFunctionEnd
