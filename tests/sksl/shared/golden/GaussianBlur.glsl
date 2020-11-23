
out vec4 sk_FragColor;
layout (binding = 0) uniform sampler2D uTextureSampler_0_Stage1;
layout (binding = 0) uniform uniformBuffer {
    layout (offset = 0) vec4 sk_RTAdjust;
    layout (offset = 16) vec2 uIncrement_Stage1_c0;
    layout (offset = 32) vec4[7] uKernel_Stage1_c0;
    layout (offset = 144) mat3 umatrix_Stage1_c0_c0;
    layout (offset = 192) vec4 uborder_Stage1_c0_c0_c0;
    layout (offset = 208) vec4 usubset_Stage1_c0_c0_c0;
    layout (offset = 224) vec4 unorm_Stage1_c0_c0_c0;
};
layout (location = 0) in vec2 vLocalCoord_Stage0;
void main() {
    vec4 output_Stage1;
    vec4 _207_GaussianConvolution_Stage1_c0;
    {
        vec4 _208_output;
        _208_output = vec4(0.0, 0.0, 0.0, 0.0);
        vec2 _209_coord = vLocalCoord_Stage0 - 12.0 * uIncrement_Stage1_c0;
        vec2 _210_coordSampled = vec2(0.0, 0.0);
        _210_coordSampled = _209_coord;
        vec4 _211_7_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _212_8_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _213_9_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _214_10_2_inCoord = _213_9_1_coords;
                _214_10_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _215_11_3_subsetCoord;
                _215_11_3_subsetCoord.x = _214_10_2_inCoord.x;
                _215_11_3_subsetCoord.y = _214_10_2_inCoord.y;
                vec2 _216_12_4_clampedCoord;
                _216_12_4_clampedCoord = _215_11_3_subsetCoord;
                vec4 _217_13_5_textureColor = texture(uTextureSampler_0_Stage1, _216_12_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _218_14_6_snappedX = floor(_214_10_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_218_14_6_snappedX < usubset_Stage1_c0_c0_c0.x || _218_14_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _217_13_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _212_8_0_TextureEffect_Stage1_c0_c0_c0 = _217_13_5_textureColor;
            }
            _211_7_MatrixEffect_Stage1_c0_c0 = _212_8_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _211_7_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[0].x;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _219_15_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _220_16_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _221_17_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _222_18_2_inCoord = _221_17_1_coords;
                _222_18_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _223_19_3_subsetCoord;
                _223_19_3_subsetCoord.x = _222_18_2_inCoord.x;
                _223_19_3_subsetCoord.y = _222_18_2_inCoord.y;
                vec2 _224_20_4_clampedCoord;
                _224_20_4_clampedCoord = _223_19_3_subsetCoord;
                vec4 _225_21_5_textureColor = texture(uTextureSampler_0_Stage1, _224_20_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _226_22_6_snappedX = floor(_222_18_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_226_22_6_snappedX < usubset_Stage1_c0_c0_c0.x || _226_22_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _225_21_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _220_16_0_TextureEffect_Stage1_c0_c0_c0 = _225_21_5_textureColor;
            }
            _219_15_MatrixEffect_Stage1_c0_c0 = _220_16_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _219_15_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[0].y;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _227_23_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _228_24_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _229_25_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _230_26_2_inCoord = _229_25_1_coords;
                _230_26_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _231_27_3_subsetCoord;
                _231_27_3_subsetCoord.x = _230_26_2_inCoord.x;
                _231_27_3_subsetCoord.y = _230_26_2_inCoord.y;
                vec2 _232_28_4_clampedCoord;
                _232_28_4_clampedCoord = _231_27_3_subsetCoord;
                vec4 _233_29_5_textureColor = texture(uTextureSampler_0_Stage1, _232_28_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _234_30_6_snappedX = floor(_230_26_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_234_30_6_snappedX < usubset_Stage1_c0_c0_c0.x || _234_30_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _233_29_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _228_24_0_TextureEffect_Stage1_c0_c0_c0 = _233_29_5_textureColor;
            }
            _227_23_MatrixEffect_Stage1_c0_c0 = _228_24_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _227_23_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[0].z;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _235_31_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _236_32_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _237_33_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _238_34_2_inCoord = _237_33_1_coords;
                _238_34_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _239_35_3_subsetCoord;
                _239_35_3_subsetCoord.x = _238_34_2_inCoord.x;
                _239_35_3_subsetCoord.y = _238_34_2_inCoord.y;
                vec2 _240_36_4_clampedCoord;
                _240_36_4_clampedCoord = _239_35_3_subsetCoord;
                vec4 _241_37_5_textureColor = texture(uTextureSampler_0_Stage1, _240_36_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _242_38_6_snappedX = floor(_238_34_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_242_38_6_snappedX < usubset_Stage1_c0_c0_c0.x || _242_38_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _241_37_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _236_32_0_TextureEffect_Stage1_c0_c0_c0 = _241_37_5_textureColor;
            }
            _235_31_MatrixEffect_Stage1_c0_c0 = _236_32_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _235_31_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[0].w;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _243_39_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _244_40_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _245_41_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _246_42_2_inCoord = _245_41_1_coords;
                _246_42_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _247_43_3_subsetCoord;
                _247_43_3_subsetCoord.x = _246_42_2_inCoord.x;
                _247_43_3_subsetCoord.y = _246_42_2_inCoord.y;
                vec2 _248_44_4_clampedCoord;
                _248_44_4_clampedCoord = _247_43_3_subsetCoord;
                vec4 _249_45_5_textureColor = texture(uTextureSampler_0_Stage1, _248_44_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _250_46_6_snappedX = floor(_246_42_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_250_46_6_snappedX < usubset_Stage1_c0_c0_c0.x || _250_46_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _249_45_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _244_40_0_TextureEffect_Stage1_c0_c0_c0 = _249_45_5_textureColor;
            }
            _243_39_MatrixEffect_Stage1_c0_c0 = _244_40_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _243_39_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[1].x;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _251_47_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _252_48_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _253_49_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _254_50_2_inCoord = _253_49_1_coords;
                _254_50_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _255_51_3_subsetCoord;
                _255_51_3_subsetCoord.x = _254_50_2_inCoord.x;
                _255_51_3_subsetCoord.y = _254_50_2_inCoord.y;
                vec2 _256_52_4_clampedCoord;
                _256_52_4_clampedCoord = _255_51_3_subsetCoord;
                vec4 _257_53_5_textureColor = texture(uTextureSampler_0_Stage1, _256_52_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _258_54_6_snappedX = floor(_254_50_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_258_54_6_snappedX < usubset_Stage1_c0_c0_c0.x || _258_54_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _257_53_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _252_48_0_TextureEffect_Stage1_c0_c0_c0 = _257_53_5_textureColor;
            }
            _251_47_MatrixEffect_Stage1_c0_c0 = _252_48_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _251_47_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[1].y;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _259_55_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _260_56_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _261_57_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _262_58_2_inCoord = _261_57_1_coords;
                _262_58_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _263_59_3_subsetCoord;
                _263_59_3_subsetCoord.x = _262_58_2_inCoord.x;
                _263_59_3_subsetCoord.y = _262_58_2_inCoord.y;
                vec2 _264_60_4_clampedCoord;
                _264_60_4_clampedCoord = _263_59_3_subsetCoord;
                vec4 _265_61_5_textureColor = texture(uTextureSampler_0_Stage1, _264_60_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _266_62_6_snappedX = floor(_262_58_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_266_62_6_snappedX < usubset_Stage1_c0_c0_c0.x || _266_62_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _265_61_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _260_56_0_TextureEffect_Stage1_c0_c0_c0 = _265_61_5_textureColor;
            }
            _259_55_MatrixEffect_Stage1_c0_c0 = _260_56_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _259_55_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[1].z;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _267_63_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _268_64_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _269_65_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _270_66_2_inCoord = _269_65_1_coords;
                _270_66_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _271_67_3_subsetCoord;
                _271_67_3_subsetCoord.x = _270_66_2_inCoord.x;
                _271_67_3_subsetCoord.y = _270_66_2_inCoord.y;
                vec2 _272_68_4_clampedCoord;
                _272_68_4_clampedCoord = _271_67_3_subsetCoord;
                vec4 _273_69_5_textureColor = texture(uTextureSampler_0_Stage1, _272_68_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _274_70_6_snappedX = floor(_270_66_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_274_70_6_snappedX < usubset_Stage1_c0_c0_c0.x || _274_70_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _273_69_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _268_64_0_TextureEffect_Stage1_c0_c0_c0 = _273_69_5_textureColor;
            }
            _267_63_MatrixEffect_Stage1_c0_c0 = _268_64_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _267_63_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[1].w;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _275_71_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _276_72_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _277_73_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _278_74_2_inCoord = _277_73_1_coords;
                _278_74_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _279_75_3_subsetCoord;
                _279_75_3_subsetCoord.x = _278_74_2_inCoord.x;
                _279_75_3_subsetCoord.y = _278_74_2_inCoord.y;
                vec2 _280_76_4_clampedCoord;
                _280_76_4_clampedCoord = _279_75_3_subsetCoord;
                vec4 _281_77_5_textureColor = texture(uTextureSampler_0_Stage1, _280_76_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _282_78_6_snappedX = floor(_278_74_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_282_78_6_snappedX < usubset_Stage1_c0_c0_c0.x || _282_78_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _281_77_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _276_72_0_TextureEffect_Stage1_c0_c0_c0 = _281_77_5_textureColor;
            }
            _275_71_MatrixEffect_Stage1_c0_c0 = _276_72_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _275_71_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[2].x;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _283_79_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _284_80_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _285_81_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _286_82_2_inCoord = _285_81_1_coords;
                _286_82_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _287_83_3_subsetCoord;
                _287_83_3_subsetCoord.x = _286_82_2_inCoord.x;
                _287_83_3_subsetCoord.y = _286_82_2_inCoord.y;
                vec2 _288_84_4_clampedCoord;
                _288_84_4_clampedCoord = _287_83_3_subsetCoord;
                vec4 _289_85_5_textureColor = texture(uTextureSampler_0_Stage1, _288_84_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _290_86_6_snappedX = floor(_286_82_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_290_86_6_snappedX < usubset_Stage1_c0_c0_c0.x || _290_86_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _289_85_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _284_80_0_TextureEffect_Stage1_c0_c0_c0 = _289_85_5_textureColor;
            }
            _283_79_MatrixEffect_Stage1_c0_c0 = _284_80_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _283_79_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[2].y;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _291_87_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _292_88_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _293_89_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _294_90_2_inCoord = _293_89_1_coords;
                _294_90_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _295_91_3_subsetCoord;
                _295_91_3_subsetCoord.x = _294_90_2_inCoord.x;
                _295_91_3_subsetCoord.y = _294_90_2_inCoord.y;
                vec2 _296_92_4_clampedCoord;
                _296_92_4_clampedCoord = _295_91_3_subsetCoord;
                vec4 _297_93_5_textureColor = texture(uTextureSampler_0_Stage1, _296_92_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _298_94_6_snappedX = floor(_294_90_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_298_94_6_snappedX < usubset_Stage1_c0_c0_c0.x || _298_94_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _297_93_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _292_88_0_TextureEffect_Stage1_c0_c0_c0 = _297_93_5_textureColor;
            }
            _291_87_MatrixEffect_Stage1_c0_c0 = _292_88_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _291_87_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[2].z;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _299_95_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _300_96_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _301_97_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _302_98_2_inCoord = _301_97_1_coords;
                _302_98_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _303_99_3_subsetCoord;
                _303_99_3_subsetCoord.x = _302_98_2_inCoord.x;
                _303_99_3_subsetCoord.y = _302_98_2_inCoord.y;
                vec2 _304_100_4_clampedCoord;
                _304_100_4_clampedCoord = _303_99_3_subsetCoord;
                vec4 _305_101_5_textureColor = texture(uTextureSampler_0_Stage1, _304_100_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _306_102_6_snappedX = floor(_302_98_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_306_102_6_snappedX < usubset_Stage1_c0_c0_c0.x || _306_102_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _305_101_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _300_96_0_TextureEffect_Stage1_c0_c0_c0 = _305_101_5_textureColor;
            }
            _299_95_MatrixEffect_Stage1_c0_c0 = _300_96_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _299_95_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[2].w;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _307_103_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _308_104_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _309_105_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _310_106_2_inCoord = _309_105_1_coords;
                _310_106_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _311_107_3_subsetCoord;
                _311_107_3_subsetCoord.x = _310_106_2_inCoord.x;
                _311_107_3_subsetCoord.y = _310_106_2_inCoord.y;
                vec2 _312_108_4_clampedCoord;
                _312_108_4_clampedCoord = _311_107_3_subsetCoord;
                vec4 _313_109_5_textureColor = texture(uTextureSampler_0_Stage1, _312_108_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _314_110_6_snappedX = floor(_310_106_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_314_110_6_snappedX < usubset_Stage1_c0_c0_c0.x || _314_110_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _313_109_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _308_104_0_TextureEffect_Stage1_c0_c0_c0 = _313_109_5_textureColor;
            }
            _307_103_MatrixEffect_Stage1_c0_c0 = _308_104_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _307_103_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[3].x;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _315_111_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _316_112_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _317_113_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _318_114_2_inCoord = _317_113_1_coords;
                _318_114_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _319_115_3_subsetCoord;
                _319_115_3_subsetCoord.x = _318_114_2_inCoord.x;
                _319_115_3_subsetCoord.y = _318_114_2_inCoord.y;
                vec2 _320_116_4_clampedCoord;
                _320_116_4_clampedCoord = _319_115_3_subsetCoord;
                vec4 _321_117_5_textureColor = texture(uTextureSampler_0_Stage1, _320_116_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _322_118_6_snappedX = floor(_318_114_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_322_118_6_snappedX < usubset_Stage1_c0_c0_c0.x || _322_118_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _321_117_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _316_112_0_TextureEffect_Stage1_c0_c0_c0 = _321_117_5_textureColor;
            }
            _315_111_MatrixEffect_Stage1_c0_c0 = _316_112_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _315_111_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[3].y;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _323_119_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _324_120_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _325_121_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _326_122_2_inCoord = _325_121_1_coords;
                _326_122_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _327_123_3_subsetCoord;
                _327_123_3_subsetCoord.x = _326_122_2_inCoord.x;
                _327_123_3_subsetCoord.y = _326_122_2_inCoord.y;
                vec2 _328_124_4_clampedCoord;
                _328_124_4_clampedCoord = _327_123_3_subsetCoord;
                vec4 _329_125_5_textureColor = texture(uTextureSampler_0_Stage1, _328_124_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _330_126_6_snappedX = floor(_326_122_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_330_126_6_snappedX < usubset_Stage1_c0_c0_c0.x || _330_126_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _329_125_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _324_120_0_TextureEffect_Stage1_c0_c0_c0 = _329_125_5_textureColor;
            }
            _323_119_MatrixEffect_Stage1_c0_c0 = _324_120_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _323_119_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[3].z;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _331_127_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _332_128_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _333_129_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _334_130_2_inCoord = _333_129_1_coords;
                _334_130_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _335_131_3_subsetCoord;
                _335_131_3_subsetCoord.x = _334_130_2_inCoord.x;
                _335_131_3_subsetCoord.y = _334_130_2_inCoord.y;
                vec2 _336_132_4_clampedCoord;
                _336_132_4_clampedCoord = _335_131_3_subsetCoord;
                vec4 _337_133_5_textureColor = texture(uTextureSampler_0_Stage1, _336_132_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _338_134_6_snappedX = floor(_334_130_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_338_134_6_snappedX < usubset_Stage1_c0_c0_c0.x || _338_134_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _337_133_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _332_128_0_TextureEffect_Stage1_c0_c0_c0 = _337_133_5_textureColor;
            }
            _331_127_MatrixEffect_Stage1_c0_c0 = _332_128_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _331_127_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[3].w;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _339_135_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _340_136_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _341_137_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _342_138_2_inCoord = _341_137_1_coords;
                _342_138_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _343_139_3_subsetCoord;
                _343_139_3_subsetCoord.x = _342_138_2_inCoord.x;
                _343_139_3_subsetCoord.y = _342_138_2_inCoord.y;
                vec2 _344_140_4_clampedCoord;
                _344_140_4_clampedCoord = _343_139_3_subsetCoord;
                vec4 _345_141_5_textureColor = texture(uTextureSampler_0_Stage1, _344_140_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _346_142_6_snappedX = floor(_342_138_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_346_142_6_snappedX < usubset_Stage1_c0_c0_c0.x || _346_142_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _345_141_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _340_136_0_TextureEffect_Stage1_c0_c0_c0 = _345_141_5_textureColor;
            }
            _339_135_MatrixEffect_Stage1_c0_c0 = _340_136_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _339_135_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[4].x;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _347_143_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _348_144_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _349_145_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _350_146_2_inCoord = _349_145_1_coords;
                _350_146_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _351_147_3_subsetCoord;
                _351_147_3_subsetCoord.x = _350_146_2_inCoord.x;
                _351_147_3_subsetCoord.y = _350_146_2_inCoord.y;
                vec2 _352_148_4_clampedCoord;
                _352_148_4_clampedCoord = _351_147_3_subsetCoord;
                vec4 _353_149_5_textureColor = texture(uTextureSampler_0_Stage1, _352_148_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _354_150_6_snappedX = floor(_350_146_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_354_150_6_snappedX < usubset_Stage1_c0_c0_c0.x || _354_150_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _353_149_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _348_144_0_TextureEffect_Stage1_c0_c0_c0 = _353_149_5_textureColor;
            }
            _347_143_MatrixEffect_Stage1_c0_c0 = _348_144_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _347_143_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[4].y;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _355_151_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _356_152_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _357_153_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _358_154_2_inCoord = _357_153_1_coords;
                _358_154_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _359_155_3_subsetCoord;
                _359_155_3_subsetCoord.x = _358_154_2_inCoord.x;
                _359_155_3_subsetCoord.y = _358_154_2_inCoord.y;
                vec2 _360_156_4_clampedCoord;
                _360_156_4_clampedCoord = _359_155_3_subsetCoord;
                vec4 _361_157_5_textureColor = texture(uTextureSampler_0_Stage1, _360_156_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _362_158_6_snappedX = floor(_358_154_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_362_158_6_snappedX < usubset_Stage1_c0_c0_c0.x || _362_158_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _361_157_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _356_152_0_TextureEffect_Stage1_c0_c0_c0 = _361_157_5_textureColor;
            }
            _355_151_MatrixEffect_Stage1_c0_c0 = _356_152_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _355_151_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[4].z;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _363_159_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _364_160_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _365_161_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _366_162_2_inCoord = _365_161_1_coords;
                _366_162_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _367_163_3_subsetCoord;
                _367_163_3_subsetCoord.x = _366_162_2_inCoord.x;
                _367_163_3_subsetCoord.y = _366_162_2_inCoord.y;
                vec2 _368_164_4_clampedCoord;
                _368_164_4_clampedCoord = _367_163_3_subsetCoord;
                vec4 _369_165_5_textureColor = texture(uTextureSampler_0_Stage1, _368_164_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _370_166_6_snappedX = floor(_366_162_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_370_166_6_snappedX < usubset_Stage1_c0_c0_c0.x || _370_166_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _369_165_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _364_160_0_TextureEffect_Stage1_c0_c0_c0 = _369_165_5_textureColor;
            }
            _363_159_MatrixEffect_Stage1_c0_c0 = _364_160_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _363_159_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[4].w;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _371_167_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _372_168_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _373_169_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _374_170_2_inCoord = _373_169_1_coords;
                _374_170_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _375_171_3_subsetCoord;
                _375_171_3_subsetCoord.x = _374_170_2_inCoord.x;
                _375_171_3_subsetCoord.y = _374_170_2_inCoord.y;
                vec2 _376_172_4_clampedCoord;
                _376_172_4_clampedCoord = _375_171_3_subsetCoord;
                vec4 _377_173_5_textureColor = texture(uTextureSampler_0_Stage1, _376_172_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _378_174_6_snappedX = floor(_374_170_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_378_174_6_snappedX < usubset_Stage1_c0_c0_c0.x || _378_174_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _377_173_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _372_168_0_TextureEffect_Stage1_c0_c0_c0 = _377_173_5_textureColor;
            }
            _371_167_MatrixEffect_Stage1_c0_c0 = _372_168_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _371_167_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[5].x;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _379_175_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _380_176_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _381_177_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _382_178_2_inCoord = _381_177_1_coords;
                _382_178_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _383_179_3_subsetCoord;
                _383_179_3_subsetCoord.x = _382_178_2_inCoord.x;
                _383_179_3_subsetCoord.y = _382_178_2_inCoord.y;
                vec2 _384_180_4_clampedCoord;
                _384_180_4_clampedCoord = _383_179_3_subsetCoord;
                vec4 _385_181_5_textureColor = texture(uTextureSampler_0_Stage1, _384_180_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _386_182_6_snappedX = floor(_382_178_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_386_182_6_snappedX < usubset_Stage1_c0_c0_c0.x || _386_182_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _385_181_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _380_176_0_TextureEffect_Stage1_c0_c0_c0 = _385_181_5_textureColor;
            }
            _379_175_MatrixEffect_Stage1_c0_c0 = _380_176_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _379_175_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[5].y;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _387_183_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _388_184_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _389_185_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _390_186_2_inCoord = _389_185_1_coords;
                _390_186_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _391_187_3_subsetCoord;
                _391_187_3_subsetCoord.x = _390_186_2_inCoord.x;
                _391_187_3_subsetCoord.y = _390_186_2_inCoord.y;
                vec2 _392_188_4_clampedCoord;
                _392_188_4_clampedCoord = _391_187_3_subsetCoord;
                vec4 _393_189_5_textureColor = texture(uTextureSampler_0_Stage1, _392_188_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _394_190_6_snappedX = floor(_390_186_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_394_190_6_snappedX < usubset_Stage1_c0_c0_c0.x || _394_190_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _393_189_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _388_184_0_TextureEffect_Stage1_c0_c0_c0 = _393_189_5_textureColor;
            }
            _387_183_MatrixEffect_Stage1_c0_c0 = _388_184_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _387_183_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[5].z;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _395_191_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _396_192_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _397_193_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _398_194_2_inCoord = _397_193_1_coords;
                _398_194_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _399_195_3_subsetCoord;
                _399_195_3_subsetCoord.x = _398_194_2_inCoord.x;
                _399_195_3_subsetCoord.y = _398_194_2_inCoord.y;
                vec2 _400_196_4_clampedCoord;
                _400_196_4_clampedCoord = _399_195_3_subsetCoord;
                vec4 _401_197_5_textureColor = texture(uTextureSampler_0_Stage1, _400_196_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _402_198_6_snappedX = floor(_398_194_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_402_198_6_snappedX < usubset_Stage1_c0_c0_c0.x || _402_198_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _401_197_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _396_192_0_TextureEffect_Stage1_c0_c0_c0 = _401_197_5_textureColor;
            }
            _395_191_MatrixEffect_Stage1_c0_c0 = _396_192_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _395_191_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[5].w;

        _209_coord += uIncrement_Stage1_c0;
        _210_coordSampled = _209_coord;
        vec4 _403_199_MatrixEffect_Stage1_c0_c0;
        {
            vec4 _404_200_0_TextureEffect_Stage1_c0_c0_c0;
            vec2 _405_201_1_coords = (umatrix_Stage1_c0_c0 * vec3(_210_coordSampled, 1.0)).xy;
            {
                vec2 _406_202_2_inCoord = _405_201_1_coords;
                _406_202_2_inCoord *= unorm_Stage1_c0_c0_c0.xy;
                vec2 _407_203_3_subsetCoord;
                _407_203_3_subsetCoord.x = _406_202_2_inCoord.x;
                _407_203_3_subsetCoord.y = _406_202_2_inCoord.y;
                vec2 _408_204_4_clampedCoord;
                _408_204_4_clampedCoord = _407_203_3_subsetCoord;
                vec4 _409_205_5_textureColor = texture(uTextureSampler_0_Stage1, _408_204_4_clampedCoord * unorm_Stage1_c0_c0_c0.zw);
                float _410_206_6_snappedX = floor(_406_202_2_inCoord.x + 0.0010000000474974513) + 0.5;
                if (_410_206_6_snappedX < usubset_Stage1_c0_c0_c0.x || _410_206_6_snappedX > usubset_Stage1_c0_c0_c0.z) {
                    _409_205_5_textureColor = uborder_Stage1_c0_c0_c0;
                }
                _404_200_0_TextureEffect_Stage1_c0_c0_c0 = _409_205_5_textureColor;
            }
            _403_199_MatrixEffect_Stage1_c0_c0 = _404_200_0_TextureEffect_Stage1_c0_c0_c0;

        }
        _208_output += _403_199_MatrixEffect_Stage1_c0_c0 * uKernel_Stage1_c0[6].x;

        _209_coord += uIncrement_Stage1_c0;
        _207_GaussianConvolution_Stage1_c0 = _208_output;
    }
    output_Stage1 = _207_GaussianConvolution_Stage1_c0;

    {
        sk_FragColor = output_Stage1;
    }
}
