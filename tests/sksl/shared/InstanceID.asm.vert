               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %sk_InstanceID %id

               ; Debug Information
               OpName %sk_InstanceID "sk_InstanceID"    ; id %7
               OpName %id "id"                          ; id %8
               OpName %main "main"                      ; id %6

               ; Annotations
               OpDecorate %sk_InstanceID BuiltIn InstanceIndex
               OpDecorate %id Location 1

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%sk_InstanceID = OpVariable %_ptr_Input_int Input   ; BuiltIn InstanceIndex
%_ptr_Output_int = OpTypePointer Output %int
         %id = OpVariable %_ptr_Output_int Output   ; Location 1
       %void = OpTypeVoid
         %11 = OpTypeFunction %void


               ; Function main
       %main = OpFunction %void None %11

         %12 = OpLabel
         %13 =   OpLoad %int %sk_InstanceID
                 OpStore %id %13
                 OpReturn
               OpFunctionEnd
