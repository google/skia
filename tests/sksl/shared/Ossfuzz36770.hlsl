### Compilation failed:

error: SPIR-V validation error: [VUID-StandaloneSpirv-Location-04919] Member index 0 is missing a location assignment
  %T = OpTypeStruct %int

               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %7
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %T "T"                        ; id %8
               OpMemberName %T 0 "x"
               OpName %main "main"                  ; id %6

               ; Annotations
               OpMemberDecorate %T 0 Offset 0
               OpDecorate %T Block

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
          %T = OpTypeStruct %int                    ; Block
%_ptr_Input_T = OpTypePointer Input %T
          %7 = OpVariable %_ptr_Input_T Input
       %void = OpTypeVoid
         %11 = OpTypeFunction %void


               ; Function main
       %main = OpFunction %void None %11

         %12 = OpLabel
                 OpReturn
               OpFunctionEnd

1 error
