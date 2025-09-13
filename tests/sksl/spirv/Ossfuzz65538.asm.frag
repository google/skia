### Compilation failed:

error: SPIR-V validation error: Variables can not have a function[7] storage class outside of a function
  %8 = OpVariable %_ptr_Function_InterfaceBlockA Function

               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragCoord
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %InterfaceBlockA "InterfaceBlockA"    ; id %11
               OpMemberName %InterfaceBlockA 0 "a"
               OpMemberName %InterfaceBlockA 1 "u_skRTFlip"
               OpName %InterfaceBlockB "InterfaceBlockB"    ; id %14
               OpMemberName %InterfaceBlockB 0 "b"
               OpName %sk_FragCoord "sk_FragCoord"  ; id %16
               OpName %main "main"                  ; id %6

               ; Annotations
               OpMemberDecorate %InterfaceBlockA 0 Offset 0
               OpMemberDecorate %InterfaceBlockA 1 Offset 16384
               OpDecorate %InterfaceBlockA Block
               OpMemberDecorate %InterfaceBlockB 0 Offset 0
               OpDecorate %InterfaceBlockB Block
               OpDecorate %sk_FragCoord BuiltIn FragCoord

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v2float = OpTypeVector %float 2
%InterfaceBlockA = OpTypeStruct %int %v2float       ; Block
%_ptr_Function_InterfaceBlockA = OpTypePointer Function %InterfaceBlockA
          %8 = OpVariable %_ptr_Function_InterfaceBlockA Function
%InterfaceBlockB = OpTypeStruct %int                ; Block
%_ptr_Function_InterfaceBlockB = OpTypePointer Function %InterfaceBlockB
         %13 = OpVariable %_ptr_Function_InterfaceBlockB Function
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input    ; BuiltIn FragCoord
       %void = OpTypeVoid
         %20 = OpTypeFunction %void


               ; Function main
       %main = OpFunction %void None %20

         %21 = OpLabel
                 OpReturn
               OpFunctionEnd

1 error
