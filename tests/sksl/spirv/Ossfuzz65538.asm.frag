### Compilation failed:

error: SPIR-V validation error: Variables can not have a function[7] storage class outside of a function
  %4 = OpVariable %_ptr_Function_InterfaceBlockA Function

               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragCoord
               OpExecutionMode %main OriginUpperLeft
               OpName %InterfaceBlockA "InterfaceBlockA"
               OpMemberName %InterfaceBlockA 0 "a"
               OpMemberName %InterfaceBlockA 1 "u_skRTFlip"
               OpName %InterfaceBlockB "InterfaceBlockB"
               OpMemberName %InterfaceBlockB 0 "b"
               OpName %sk_FragCoord "sk_FragCoord"
               OpName %main "main"
               OpMemberDecorate %InterfaceBlockA 0 Offset 0
               OpMemberDecorate %InterfaceBlockA 1 Offset 16384
               OpDecorate %InterfaceBlockA Block
               OpMemberDecorate %InterfaceBlockB 0 Offset 0
               OpDecorate %InterfaceBlockB Block
               OpDecorate %sk_FragCoord BuiltIn FragCoord
        %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%InterfaceBlockA = OpTypeStruct %int %v2float
%_ptr_Function_InterfaceBlockA = OpTypePointer Function %InterfaceBlockA
          %4 = OpVariable %_ptr_Function_InterfaceBlockA Function
%InterfaceBlockB = OpTypeStruct %int
%_ptr_Function_InterfaceBlockB = OpTypePointer Function %InterfaceBlockB
         %10 = OpVariable %_ptr_Function_InterfaceBlockB Function
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
       %main = OpFunction %void None %17
         %18 = OpLabel
               OpReturn
               OpFunctionEnd

1 error
