               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %main "main" %sk_InstanceID %id
               OpName %sk_InstanceID "sk_InstanceID"
               OpName %id "id"
               OpName %fn_i "fn_i"
               OpName %main "main"
               OpDecorate %sk_InstanceID BuiltIn InstanceIndex
               OpDecorate %id Location 1
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%sk_InstanceID = OpVariable %_ptr_Input_int Input
%_ptr_Output_int = OpTypePointer Output %int
         %id = OpVariable %_ptr_Output_int Output
          %9 = OpTypeFunction %int
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
       %fn_i = OpFunction %int None %9
         %10 = OpLabel
         %11 = OpLoad %int %sk_InstanceID
               OpReturnValue %11
               OpFunctionEnd
       %main = OpFunction %void None %13
         %14 = OpLabel
         %15 = OpFunctionCall %int %fn_i
               OpStore %id %15
               OpReturn
               OpFunctionEnd
