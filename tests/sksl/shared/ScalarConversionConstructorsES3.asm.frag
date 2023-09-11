               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %f "f"
               OpName %i "i"
               OpName %u "u"
               OpName %b "b"
               OpName %f1 "f1"
               OpName %f2 "f2"
               OpName %f3 "f3"
               OpName %f4 "f4"
               OpName %i1 "i1"
               OpName %i2 "i2"
               OpName %i3 "i3"
               OpName %i4 "i4"
               OpName %u1 "u1"
               OpName %u2 "u2"
               OpName %u3 "u3"
               OpName %u4 "u4"
               OpName %b1 "b1"
               OpName %b2 "b2"
               OpName %b3 "b3"
               OpName %b4 "b4"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
       %uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
    %float_1 = OpConstant %float 1
      %int_1 = OpConstant %int 1
     %uint_1 = OpConstant %uint 1
     %uint_0 = OpConstant %uint 0
   %float_16 = OpConstant %float 16
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %12
         %13 = OpLabel
         %17 = OpVariable %_ptr_Function_v2float Function
               OpStore %17 %16
         %19 = OpFunctionCall %v4float %main %17
               OpStore %sk_FragColor %19
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpLabel
          %f = OpVariable %_ptr_Function_float Function
          %i = OpVariable %_ptr_Function_int Function
          %u = OpVariable %_ptr_Function_uint Function
          %b = OpVariable %_ptr_Function_bool Function
         %f1 = OpVariable %_ptr_Function_float Function
         %f2 = OpVariable %_ptr_Function_float Function
         %f3 = OpVariable %_ptr_Function_float Function
         %f4 = OpVariable %_ptr_Function_float Function
         %i1 = OpVariable %_ptr_Function_int Function
         %i2 = OpVariable %_ptr_Function_int Function
         %i3 = OpVariable %_ptr_Function_int Function
         %i4 = OpVariable %_ptr_Function_int Function
         %u1 = OpVariable %_ptr_Function_uint Function
         %u2 = OpVariable %_ptr_Function_uint Function
         %u3 = OpVariable %_ptr_Function_uint Function
         %u4 = OpVariable %_ptr_Function_uint Function
         %b1 = OpVariable %_ptr_Function_bool Function
         %b2 = OpVariable %_ptr_Function_bool Function
         %b3 = OpVariable %_ptr_Function_bool Function
         %b4 = OpVariable %_ptr_Function_bool Function
        %112 = OpVariable %_ptr_Function_v4float Function
         %25 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %29 = OpLoad %v4float %25
         %30 = OpCompositeExtract %float %29 1
               OpStore %f %30
         %33 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %34 = OpLoad %v4float %33
         %35 = OpCompositeExtract %float %34 1
         %36 = OpConvertFToS %int %35
               OpStore %i %36
         %40 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %41 = OpLoad %v4float %40
         %42 = OpCompositeExtract %float %41 1
         %43 = OpConvertFToU %uint %42
               OpStore %u %43
         %47 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %48 = OpLoad %v4float %47
         %49 = OpCompositeExtract %float %48 1
         %50 = OpFUnordNotEqual %bool %49 %float_0
               OpStore %b %50
               OpStore %f1 %30
         %53 = OpConvertSToF %float %36
               OpStore %f2 %53
         %55 = OpConvertUToF %float %43
               OpStore %f3 %55
         %57 = OpSelect %float %50 %float_1 %float_0
               OpStore %f4 %57
         %60 = OpConvertFToS %int %30
               OpStore %i1 %60
               OpStore %i2 %36
         %63 = OpBitcast %int %43
               OpStore %i3 %63
         %65 = OpSelect %int %50 %int_1 %int_0
               OpStore %i4 %65
         %68 = OpConvertFToU %uint %30
               OpStore %u1 %68
         %70 = OpBitcast %uint %36
               OpStore %u2 %70
               OpStore %u3 %43
         %73 = OpSelect %uint %50 %uint_1 %uint_0
               OpStore %u4 %73
         %77 = OpFUnordNotEqual %bool %30 %float_0
               OpStore %b1 %77
         %79 = OpINotEqual %bool %36 %int_0
               OpStore %b2 %79
         %81 = OpINotEqual %bool %43 %uint_0
               OpStore %b3 %81
               OpStore %b4 %50
         %83 = OpFAdd %float %30 %53
         %84 = OpFAdd %float %83 %55
         %85 = OpFAdd %float %84 %57
         %86 = OpConvertSToF %float %60
         %87 = OpFAdd %float %85 %86
         %88 = OpConvertSToF %float %36
         %89 = OpFAdd %float %87 %88
         %90 = OpConvertSToF %float %63
         %91 = OpFAdd %float %89 %90
         %92 = OpConvertSToF %float %65
         %93 = OpFAdd %float %91 %92
         %94 = OpConvertUToF %float %68
         %95 = OpFAdd %float %93 %94
         %96 = OpConvertUToF %float %70
         %97 = OpFAdd %float %95 %96
         %98 = OpConvertUToF %float %43
         %99 = OpFAdd %float %97 %98
        %100 = OpConvertUToF %float %73
        %101 = OpFAdd %float %99 %100
        %102 = OpSelect %float %77 %float_1 %float_0
        %103 = OpFAdd %float %101 %102
        %104 = OpSelect %float %79 %float_1 %float_0
        %105 = OpFAdd %float %103 %104
        %106 = OpSelect %float %81 %float_1 %float_0
        %107 = OpFAdd %float %105 %106
        %108 = OpSelect %float %50 %float_1 %float_0
        %109 = OpFAdd %float %107 %108
        %111 = OpFOrdEqual %bool %109 %float_16
               OpSelectionMerge %116 None
               OpBranchConditional %111 %114 %115
        %114 = OpLabel
        %117 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %118 = OpLoad %v4float %117
               OpStore %112 %118
               OpBranch %116
        %115 = OpLabel
        %119 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %120 = OpLoad %v4float %119
               OpStore %112 %120
               OpBranch %116
        %116 = OpLabel
        %121 = OpLoad %v4float %112
               OpReturnValue %121
               OpFunctionEnd
