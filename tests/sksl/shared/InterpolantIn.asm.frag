               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor %defaultVarying %linearVarying %flatVarying
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %defaultVarying "defaultVarying"
               OpName %linearVarying "linearVarying"
               OpName %flatVarying "flatVarying"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %defaultVarying Location 0
               OpDecorate %linearVarying Location 1
               OpDecorate %linearVarying NoPerspective
               OpDecorate %flatVarying Location 2
               OpDecorate %flatVarying Flat
               OpDecorate %18 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_ptr_Input_float = OpTypePointer Input %float
%defaultVarying = OpVariable %_ptr_Input_float Input
%linearVarying = OpVariable %_ptr_Input_float Input
%flatVarying = OpVariable %_ptr_Input_float Input
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_1 = OpConstant %float 1
       %main = OpFunction %void None %12
         %13 = OpLabel
         %14 = OpLoad %float %defaultVarying
         %15 = OpLoad %float %linearVarying
         %16 = OpLoad %float %flatVarying
         %18 = OpCompositeConstruct %v4float %14 %15 %16 %float_1
               OpStore %sk_FragColor %18
               OpReturn
               OpFunctionEnd
