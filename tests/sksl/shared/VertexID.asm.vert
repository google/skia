               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %sk_VertexID %id

               ; Debug Information
               OpName %sk_VertexID "sk_VertexID"    ; id %3
               OpName %id "id"                      ; id %6
               OpName %main "main"                  ; id %2

               ; Annotations
               OpDecorate %sk_VertexID BuiltIn VertexIndex
               OpDecorate %id Location 1

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%sk_VertexID = OpVariable %_ptr_Input_int Input     ; BuiltIn VertexIndex
%_ptr_Output_int = OpTypePointer Output %int
         %id = OpVariable %_ptr_Output_int Output   ; Location 1
       %void = OpTypeVoid
          %9 = OpTypeFunction %void


               ; Function main
       %main = OpFunction %void None %9

         %10 = OpLabel
         %11 =   OpLoad %int %sk_VertexID
                 OpStore %id %11
                 OpReturn
               OpFunctionEnd
