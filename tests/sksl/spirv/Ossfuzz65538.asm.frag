### Compilation failed:

error: SPIR-V validation error: Variables can not have a function[7] storage class outside of a function
  %4 = OpVariable %_ptr_Function_InterfaceBlockA Function

               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragCoord
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %InterfaceBlockA "InterfaceBlockA"    ; id %8
               OpMemberName %InterfaceBlockA 0 "a"
               OpMemberName %InterfaceBlockA 1 "u_skRTFlip"
               OpName %InterfaceBlockB "InterfaceBlockB"    ; id %11
               OpMemberName %InterfaceBlockB 0 "b"
               OpName %sk_FragCoord "sk_FragCoord"  ; id %13
               OpName %main "main"                  ; id %2

               ; Annotations
               OpMemberDecorate %InterfaceBlockA 0 Offset 0
               OpMemberDecorate %InterfaceBlockA 1 Offset 16384
               OpDecorate %InterfaceBlockA Block
               OpMemberDecorate %InterfaceBlockB 0 Offset 0
               OpDecorate %InterfaceBlockB Block
               OpDecorate %sk_FragCoord BuiltIn FragCoord

               ; Types, variables and constants
        %int = OpTypeInt 32 1
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%InterfaceBlockA = OpTypeStruct %int %v2float       ; Block
%_ptr_Function_InterfaceBlockA = OpTypePointer Function %InterfaceBlockA
          %4 = OpVariable %_ptr_Function_InterfaceBlockA Function
%InterfaceBlockB = OpTypeStruct %int                ; Block
%_ptr_Function_InterfaceBlockB = OpTypePointer Function %InterfaceBlockB
         %10 = OpVariable %_ptr_Function_InterfaceBlockB Function
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input    ; BuiltIn FragCoord
       %void = OpTypeVoid
         %17 = OpTypeFunction %void


               ; Function main
       %main = OpFunction %void None %17

         %18 = OpLabel
                 OpReturn
               OpFunctionEnd

1 error
