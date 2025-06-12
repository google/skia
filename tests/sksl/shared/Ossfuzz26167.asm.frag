               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %main "main"                  ; id %2

               ; Types, variables and constants
       %void = OpTypeVoid
          %4 = OpTypeFunction %void


               ; Function main
       %main = OpFunction %void None %4

          %5 = OpLabel
                 OpReturn
               OpFunctionEnd
