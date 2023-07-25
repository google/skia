               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor %sk_FragCoord
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %sk_FragCoord "sk_FragCoord"
               OpName %main "main"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %sk_FragCoord BuiltIn FragCoord
               OpDecorate %18 RelaxedPrecision
               OpDecorate %19 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
    %v2float = OpTypeVector %float 2
       %main = OpFunction %void None %13
         %14 = OpLabel
         %15 = OpLoad %v4float %sk_FragCoord
         %16 = OpVectorShuffle %v2float %15 %15 0 1
         %18 = OpLoad %v4float %sk_FragColor
         %19 = OpVectorShuffle %v4float %18 %16 4 5 2 3
               OpStore %sk_FragColor %19
               OpReturn
               OpFunctionEnd
