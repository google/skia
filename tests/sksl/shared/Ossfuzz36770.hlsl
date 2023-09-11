### Compilation failed:

error: SPIR-V validation error: [VUID-StandaloneSpirv-Location-04919] Member index 0 is missing a location assignment
  %T = OpTypeStruct %int

               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %3
               OpExecutionMode %main OriginUpperLeft
               OpName %T "T"
               OpMemberName %T 0 "x"
               OpName %main "main"
               OpMemberDecorate %T 0 Offset 0
               OpDecorate %T Block
        %int = OpTypeInt 32 1
          %T = OpTypeStruct %int
%_ptr_Input_T = OpTypePointer Input %T
          %3 = OpVariable %_ptr_Input_T Input
       %void = OpTypeVoid
          %8 = OpTypeFunction %void
       %main = OpFunction %void None %8
          %9 = OpLabel
               OpReturn
               OpFunctionEnd

1 error
