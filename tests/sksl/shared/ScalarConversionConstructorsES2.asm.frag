               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %_UniformBuffer "_UniformBuffer"  ; id %8
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %10
               OpName %main "main"                      ; id %2
               OpName %f "f"                            ; id %23
               OpName %i "i"                            ; id %31
               OpName %b "b"                            ; id %37
               OpName %f1 "f1"                          ; id %44
               OpName %f2 "f2"                          ; id %45
               OpName %f3 "f3"                          ; id %47
               OpName %i1 "i1"                          ; id %50
               OpName %i2 "i2"                          ; id %52
               OpName %i3 "i3"                          ; id %53
               OpName %b1 "b1"                          ; id %56
               OpName %b2 "b2"                          ; id %58
               OpName %b3 "b3"                          ; id %60

               ; Annotations
               OpDecorate %main RelaxedPrecision
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
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
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
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
    %float_1 = OpConstant %float 1
      %int_1 = OpConstant %int 1
    %float_9 = OpConstant %float 9
%_ptr_Function_v4float = OpTypePointer Function %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %12

         %13 = OpLabel
         %17 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %17 %16
         %19 =   OpFunctionCall %v4float %main %17
                 OpStore %sk_FragColor %19
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %20         ; RelaxedPrecision
         %21 = OpFunctionParameter %_ptr_Function_v2float

         %22 = OpLabel
          %f =   OpVariable %_ptr_Function_float Function
          %i =   OpVariable %_ptr_Function_int Function
          %b =   OpVariable %_ptr_Function_bool Function
         %f1 =   OpVariable %_ptr_Function_float Function
         %f2 =   OpVariable %_ptr_Function_float Function
         %f3 =   OpVariable %_ptr_Function_float Function
         %i1 =   OpVariable %_ptr_Function_int Function
         %i2 =   OpVariable %_ptr_Function_int Function
         %i3 =   OpVariable %_ptr_Function_int Function
         %b1 =   OpVariable %_ptr_Function_bool Function
         %b2 =   OpVariable %_ptr_Function_bool Function
         %b3 =   OpVariable %_ptr_Function_bool Function
         %77 =   OpVariable %_ptr_Function_v4float Function
         %25 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %29 =   OpLoad %v4float %25                ; RelaxedPrecision
         %30 =   OpCompositeExtract %float %29 1    ; RelaxedPrecision
                 OpStore %f %30
         %33 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %34 =   OpLoad %v4float %33                ; RelaxedPrecision
         %35 =   OpCompositeExtract %float %34 1    ; RelaxedPrecision
         %36 =   OpConvertFToS %int %35
                 OpStore %i %36
         %40 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %41 =   OpLoad %v4float %40                ; RelaxedPrecision
         %42 =   OpCompositeExtract %float %41 1    ; RelaxedPrecision
         %43 =   OpFUnordNotEqual %bool %42 %float_0
                 OpStore %b %43
                 OpStore %f1 %30
         %46 =   OpConvertSToF %float %36
                 OpStore %f2 %46
         %48 =   OpSelect %float %43 %float_1 %float_0
                 OpStore %f3 %48
         %51 =   OpConvertFToS %int %30
                 OpStore %i1 %51
                 OpStore %i2 %36
         %54 =   OpSelect %int %43 %int_1 %int_0
                 OpStore %i3 %54
         %57 =   OpFUnordNotEqual %bool %30 %float_0
                 OpStore %b1 %57
         %59 =   OpINotEqual %bool %36 %int_0
                 OpStore %b2 %59
                 OpStore %b3 %43
         %61 =   OpFAdd %float %30 %46              ; RelaxedPrecision
         %62 =   OpFAdd %float %61 %48              ; RelaxedPrecision
         %63 =   OpConvertSToF %float %51           ; RelaxedPrecision
         %64 =   OpFAdd %float %62 %63              ; RelaxedPrecision
         %65 =   OpConvertSToF %float %36           ; RelaxedPrecision
         %66 =   OpFAdd %float %64 %65              ; RelaxedPrecision
         %67 =   OpConvertSToF %float %54           ; RelaxedPrecision
         %68 =   OpFAdd %float %66 %67              ; RelaxedPrecision
         %69 =   OpSelect %float %57 %float_1 %float_0  ; RelaxedPrecision
         %70 =   OpFAdd %float %68 %69                  ; RelaxedPrecision
         %71 =   OpSelect %float %59 %float_1 %float_0  ; RelaxedPrecision
         %72 =   OpFAdd %float %70 %71                  ; RelaxedPrecision
         %73 =   OpSelect %float %43 %float_1 %float_0  ; RelaxedPrecision
         %74 =   OpFAdd %float %72 %73                  ; RelaxedPrecision
         %76 =   OpFOrdEqual %bool %74 %float_9
                 OpSelectionMerge %81 None
                 OpBranchConditional %76 %79 %80

         %79 =     OpLabel
         %82 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %83 =       OpLoad %v4float %82            ; RelaxedPrecision
                     OpStore %77 %83
                     OpBranch %81

         %80 =     OpLabel
         %84 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %85 =       OpLoad %v4float %84            ; RelaxedPrecision
                     OpStore %77 %85
                     OpBranch %81

         %81 = OpLabel
         %86 =   OpLoad %v4float %77                ; RelaxedPrecision
                 OpReturnValue %86
               OpFunctionEnd
