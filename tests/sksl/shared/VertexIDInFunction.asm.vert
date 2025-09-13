               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %sk_VertexID %id

               ; Debug Information
               OpName %sk_VertexID "sk_VertexID"    ; id %8
               OpName %id "id"                      ; id %9
               OpName %fn_i "fn_i"                  ; id %6
               OpName %main "main"                  ; id %7

               ; Annotations
               OpDecorate %sk_VertexID BuiltIn VertexIndex
               OpDecorate %id Location 1

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%sk_VertexID = OpVariable %_ptr_Input_int Input     ; BuiltIn VertexIndex
%_ptr_Output_int = OpTypePointer Output %int
         %id = OpVariable %_ptr_Output_int Output   ; Location 1
         %11 = OpTypeFunction %int
       %void = OpTypeVoid
         %15 = OpTypeFunction %void


               ; Function fn_i
       %fn_i = OpFunction %int None %11

         %12 = OpLabel
         %13 =   OpLoad %int %sk_VertexID
                 OpReturnValue %13
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %15

         %16 = OpLabel
         %17 =   OpFunctionCall %int %fn_i
                 OpStore %id %17
                 OpReturn
               OpFunctionEnd
