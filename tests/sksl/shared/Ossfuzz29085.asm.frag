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
       %void = OpTypeVoid
          %8 = OpTypeFunction %void


               ; Function main
       %main = OpFunction %void None %8

          %9 = OpLabel
                 OpReturn
               OpFunctionEnd
