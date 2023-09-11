               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %main "main"
               OpName %i "i"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
       %void = OpTypeVoid
          %7 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_1 = OpConstant %int 1
      %v3int = OpTypeVector %int 3
       %main = OpFunction %void None %7
          %8 = OpLabel
          %i = OpVariable %_ptr_Function_int Function
         %12 = OpLoad %int %i
         %14 = OpISub %int %12 %int_1
               OpStore %i %14
         %16 = OpCompositeConstruct %v3int %12 %12 %12
               OpReturn
               OpFunctionEnd
