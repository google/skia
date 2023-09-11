### Compilation failed:

error: SPIR-V validation error: [VUID-StandaloneSpirv-Location-04919] Member index 0 is missing a location assignment
  %s = OpTypeStruct %int

               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %3
               OpExecutionMode %main OriginUpperLeft
               OpName %s "s"
               OpMemberName %s 0 "I"
               OpName %main "main"
               OpMemberDecorate %s 0 Offset 0
               OpDecorate %s Block
        %int = OpTypeInt 32 1
          %s = OpTypeStruct %int
%_ptr_Input_s = OpTypePointer Input %s
          %3 = OpVariable %_ptr_Input_s Input
       %void = OpTypeVoid
          %8 = OpTypeFunction %void
       %main = OpFunction %void None %8
          %9 = OpLabel
               OpReturn
               OpFunctionEnd

1 error
