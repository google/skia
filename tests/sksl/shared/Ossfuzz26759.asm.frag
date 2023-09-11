               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpName %main "main"
               OpName %i "i"
       %void = OpTypeVoid
          %4 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_1 = OpConstant %int 1
      %v3int = OpTypeVector %int 3
       %main = OpFunction %void None %4
          %5 = OpLabel
          %i = OpVariable %_ptr_Function_int Function
          %9 = OpLoad %int %i
         %11 = OpISub %int %9 %int_1
               OpStore %i %11
         %13 = OpCompositeConstruct %v3int %9 %9 %9
               OpReturn
               OpFunctionEnd
