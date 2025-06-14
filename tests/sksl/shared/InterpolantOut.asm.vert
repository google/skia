               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %defaultVarying %linearVarying %flatVarying

               ; Debug Information
               OpName %defaultVarying "defaultVarying"  ; id %7
               OpName %linearVarying "linearVarying"    ; id %10
               OpName %flatVarying "flatVarying"        ; id %11
               OpName %main "main"                      ; id %6

               ; Annotations
               OpDecorate %defaultVarying Location 0
               OpDecorate %linearVarying Location 1
               OpDecorate %linearVarying NoPerspective
               OpDecorate %flatVarying Location 2
               OpDecorate %flatVarying Flat

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
%_ptr_Output_float = OpTypePointer Output %float
%defaultVarying = OpVariable %_ptr_Output_float Output  ; Location 0
%linearVarying = OpVariable %_ptr_Output_float Output   ; Location 1, NoPerspective
%flatVarying = OpVariable %_ptr_Output_float Output     ; Location 2, Flat
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3


               ; Function main
       %main = OpFunction %void None %13

         %14 = OpLabel
                 OpStore %defaultVarying %float_1
                 OpStore %linearVarying %float_2
                 OpStore %flatVarying %float_3
                 OpReturn
               OpFunctionEnd
