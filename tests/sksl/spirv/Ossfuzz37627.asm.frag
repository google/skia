               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %main "main"                  ; id %6
               OpName %x "x"                        ; id %10

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
       %void = OpTypeVoid
          %8 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
     %uint_1 = OpConstant %uint 1


               ; Function main
       %main = OpFunction %void None %8

          %9 = OpLabel
          %x =   OpVariable %_ptr_Function_uint Function
         %14 =   OpLoad %uint %x
         %15 =   OpIAdd %uint %14 %uint_1
                 OpStore %x %15
         %16 =   OpSNegate %uint %15
                 OpReturn
               OpFunctionEnd
