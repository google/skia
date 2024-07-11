               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %defaultVarying %linearVarying %flatVarying
               OpName %defaultVarying "defaultVarying"
               OpName %linearVarying "linearVarying"
               OpName %flatVarying "flatVarying"
               OpName %main "main"
               OpDecorate %defaultVarying Location 0
               OpDecorate %linearVarying Location 1
               OpDecorate %linearVarying NoPerspective
               OpDecorate %flatVarying Location 2
               OpDecorate %flatVarying Flat
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
%defaultVarying = OpVariable %_ptr_Output_float Output
%linearVarying = OpVariable %_ptr_Output_float Output
%flatVarying = OpVariable %_ptr_Output_float Output
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
       %main = OpFunction %void None %9
         %10 = OpLabel
               OpStore %defaultVarying %float_1
               OpStore %linearVarying %float_2
               OpStore %flatVarying %float_3
               OpReturn
               OpFunctionEnd
