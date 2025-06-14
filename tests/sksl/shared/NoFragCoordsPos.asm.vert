               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %7 %pos

               ; Debug Information
               OpName %sk_PerVertex "sk_PerVertex"  ; id %10
               OpMemberName %sk_PerVertex 0 "sk_Position"
               OpMemberName %sk_PerVertex 1 "sk_PointSize"
               OpName %pos "pos"                    ; id %12
               OpName %main "main"                  ; id %6

               ; Annotations
               OpMemberDecorate %sk_PerVertex 0 BuiltIn Position
               OpMemberDecorate %sk_PerVertex 1 BuiltIn PointSize
               OpDecorate %sk_PerVertex Block
               OpDecorate %pos Location 0

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%sk_PerVertex = OpTypeStruct %v4float %float        ; Block
%_ptr_Output_sk_PerVertex = OpTypePointer Output %sk_PerVertex
          %7 = OpVariable %_ptr_Output_sk_PerVertex Output
%_ptr_Input_v4float = OpTypePointer Input %v4float
        %pos = OpVariable %_ptr_Input_v4float Input     ; Location 0
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
      %int_0 = OpConstant %int 0
%_ptr_Output_v4float = OpTypePointer Output %v4float


               ; Function main
       %main = OpFunction %void None %15

         %16 = OpLabel
         %17 =   OpLoad %v4float %pos
         %19 =   OpAccessChain %_ptr_Output_v4float %7 %int_0
                 OpStore %19 %17
                 OpReturn
               OpFunctionEnd
