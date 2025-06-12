               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %main "main"                  ; id %2
               OpName %x "x"                        ; id %6

               ; Types, variables and constants
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
     %uint_1 = OpConstant %uint 1


               ; Function main
       %main = OpFunction %void None %4

          %5 = OpLabel
          %x =   OpVariable %_ptr_Function_uint Function
         %10 =   OpLoad %uint %x
         %11 =   OpIAdd %uint %10 %uint_1
                 OpStore %x %11
         %12 =   OpSNegate %uint %11
                 OpReturn
               OpFunctionEnd
