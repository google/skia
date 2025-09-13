### Compilation failed:

error: SPIR-V validation error: [VUID-StandaloneSpirv-Location-04919] Member index 0 is missing a location assignment
  %s = OpTypeStruct %int

               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %7
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %s "s"                        ; id %8
               OpMemberName %s 0 "I"
               OpName %main "main"                  ; id %6

               ; Annotations
               OpMemberDecorate %s 0 Offset 0
               OpDecorate %s Block

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
          %s = OpTypeStruct %int                    ; Block
%_ptr_Input_s = OpTypePointer Input %s
          %7 = OpVariable %_ptr_Input_s Input
       %void = OpTypeVoid
         %11 = OpTypeFunction %void


               ; Function main
       %main = OpFunction %void None %11

         %12 = OpLabel
                 OpReturn
               OpFunctionEnd

1 error
