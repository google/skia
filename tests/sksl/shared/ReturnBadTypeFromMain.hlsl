### Compilation failed:

error: SPIR-V validation error: [VUID-StandaloneSpirv-None-04633] OpEntryPoint Entry Point <id> '6[%main]'s function return type is not void.
  OpEntryPoint Fragment %main "main"

               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %main "main"                  ; id %6

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %v3int = OpTypeVector %int 3
          %8 = OpTypeFunction %v3int
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
         %13 = OpConstantComposite %v3int %int_1 %int_2 %int_3


               ; Function main
       %main = OpFunction %v3int None %8

          %9 = OpLabel
                 OpReturnValue %13
               OpFunctionEnd

1 error
