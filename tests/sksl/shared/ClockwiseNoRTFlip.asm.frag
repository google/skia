               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %main "main"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %13 RelaxedPrecision
               OpDecorate %18 RelaxedPrecision
               OpDecorate %19 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
         %11 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_1 = OpConstant %int 1
     %int_n1 = OpConstant %int -1
       %main = OpFunction %void None %11
         %12 = OpLabel
         %13 = OpLoad %bool %sk_Clockwise
         %14 = OpSelect %int %13 %int_1 %int_n1
         %18 = OpConvertSToF %float %14
         %19 = OpCompositeConstruct %v4float %18 %18 %18 %18
               OpStore %sk_FragColor %19
               OpReturn
               OpFunctionEnd
