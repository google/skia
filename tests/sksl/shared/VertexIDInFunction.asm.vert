               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %sk_VertexID %id

               ; Debug Information
               OpName %sk_VertexID "sk_VertexID"    ; id %4
               OpName %id "id"                      ; id %7
               OpName %fn_i "fn_i"                  ; id %2
               OpName %main "main"                  ; id %3

               ; Annotations
               OpDecorate %sk_VertexID BuiltIn VertexIndex
               OpDecorate %id Location 1

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%sk_VertexID = OpVariable %_ptr_Input_int Input     ; BuiltIn VertexIndex
%_ptr_Output_int = OpTypePointer Output %int
         %id = OpVariable %_ptr_Output_int Output   ; Location 1
          %9 = OpTypeFunction %int
       %void = OpTypeVoid
         %13 = OpTypeFunction %void


               ; Function fn_i
       %fn_i = OpFunction %int None %9

         %10 = OpLabel
         %11 =   OpLoad %int %sk_VertexID
                 OpReturnValue %11
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %13

         %14 = OpLabel
         %15 =   OpFunctionCall %int %fn_i
                 OpStore %id %15
                 OpReturn
               OpFunctionEnd
