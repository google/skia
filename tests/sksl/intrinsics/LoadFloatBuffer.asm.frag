               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %FloatBuffer "FloatBuffer"
               OpMemberName %FloatBuffer 0 "floatData"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %avoidInline_vf "avoidInline_vf"
               OpName %main "main"
               OpName %f "f"
               OpDecorate %_runtimearr_float ArrayStride 16
               OpMemberDecorate %FloatBuffer 0 Offset 0
               OpDecorate %FloatBuffer BufferBlock
               OpDecorate %4 Binding 0
               OpDecorate %4 DescriptorSet 0
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %34 RelaxedPrecision
      %float = OpTypeFloat 32
%_runtimearr_float = OpTypeRuntimeArray %float
%FloatBuffer = OpTypeStruct %_runtimearr_float
%_ptr_Uniform_FloatBuffer = OpTypePointer Uniform %FloatBuffer
          %4 = OpVariable %_ptr_Uniform_FloatBuffer Uniform
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
         %18 = OpTypeFunction %void %_ptr_Function_float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float
         %27 = OpTypeFunction %v4float
    %float_0 = OpConstant %float 0
%_entrypoint_v = OpFunction %void None %14
         %15 = OpLabel
         %16 = OpFunctionCall %v4float %main
               OpStore %sk_FragColor %16
               OpReturn
               OpFunctionEnd
%avoidInline_vf = OpFunction %void None %18
         %19 = OpFunctionParameter %_ptr_Function_float
         %20 = OpLabel
         %24 = OpAccessChain %_ptr_Uniform_float %4 %int_0 %int_0
         %26 = OpLoad %float %24
               OpStore %19 %26
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %27
         %28 = OpLabel
          %f = OpVariable %_ptr_Function_float Function
         %31 = OpVariable %_ptr_Function_float Function
               OpStore %f %float_0
         %32 = OpFunctionCall %void %avoidInline_vf %31
         %33 = OpLoad %float %31
               OpStore %f %33
         %34 = OpCompositeConstruct %v4float %33 %33 %33 %33
               OpReturnValue %34
               OpFunctionEnd
