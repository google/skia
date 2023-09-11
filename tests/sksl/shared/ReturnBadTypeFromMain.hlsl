### Compilation failed:

error: SPIR-V validation error: [VUID-StandaloneSpirv-None-04633] OpEntryPoint Entry Point <id> '2[%main]'s function return type is not void.
  OpEntryPoint Fragment %main "main"

               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
        %int = OpTypeInt 32 1
      %v3int = OpTypeVector %int 3
          %5 = OpTypeFunction %v3int
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
         %10 = OpConstantComposite %v3int %int_1 %int_2 %int_3
       %main = OpFunction %v3int None %5
          %6 = OpLabel
               OpReturnValue %10
               OpFunctionEnd

1 error
