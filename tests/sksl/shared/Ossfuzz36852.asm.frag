               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %x "x"
               OpName %y "y"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %x RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %29 = OpConstantComposite %v2float %float_0 %float_1
         %30 = OpConstantComposite %v2float %float_2 %float_3
         %31 = OpConstantComposite %mat2v2float %29 %30
         %33 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
%_entrypoint_v = OpFunction %void None %12
         %13 = OpLabel
         %17 = OpVariable %_ptr_Function_v2float Function
               OpStore %17 %16
         %19 = OpFunctionCall %v4float %main %17
               OpStore %sk_FragColor %19
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpLabel
          %x = OpVariable %_ptr_Function_mat2v2float Function
          %y = OpVariable %_ptr_Function_v2float Function
               OpStore %x %31
         %34 = OpVectorShuffle %v2float %33 %33 0 1
               OpStore %y %34
         %35 = OpCompositeExtract %float %34 0
         %36 = OpCompositeExtract %float %34 1
         %37 = OpCompositeConstruct %v4float %35 %36 %float_0 %float_1
               OpReturnValue %37
               OpFunctionEnd
