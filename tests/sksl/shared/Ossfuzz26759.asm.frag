               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %main "main"                  ; id %6
               OpName %i "i"                        ; id %10

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
       %void = OpTypeVoid
          %8 = OpTypeFunction %void
%_ptr_Function_int = OpTypePointer Function %int
      %int_1 = OpConstant %int 1
      %v3int = OpTypeVector %int 3


               ; Function main
       %main = OpFunction %void None %8

          %9 = OpLabel
          %i =   OpVariable %_ptr_Function_int Function
         %12 =   OpLoad %int %i
         %14 =   OpISub %int %12 %int_1
                 OpStore %i %14
         %16 =   OpCompositeConstruct %v3int %12 %12 %12
                 OpReturn
               OpFunctionEnd
