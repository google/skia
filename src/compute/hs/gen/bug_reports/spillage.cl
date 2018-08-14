//
// Copyright (c) PIXEL I/O, LLC.  All rights reserved.
//

__kernel
void
spillage_uint(__global const uint* const __restrict__ in, __global uint* const out)
{
  uint r1;
  uint r2;
  uint r3;
  uint r4;
  uint r5;
  uint r6;
  uint r7;
  uint r8;
  uint r9;
  uint r10;
  uint r11;
  uint r12;
  uint r13;
  uint r14;
  uint r15;
  uint r16;
  uint r17;
  uint r18;
  uint r19;
  uint r20;
  uint r21;
  uint r22;
  uint r23;
  uint r24;
  uint r25;
  uint r26;
  uint r27;
  uint r28;
  uint r29;
  uint r30;
  uint r31;
  uint r32;
  uint r33;
  uint r34;
  uint r35;
  uint r36;
  uint r37;
  uint r38;
  uint r39;
  uint r40;
  uint r41;
  uint r42;
  uint r43;
  uint r44;
  uint r45;
  uint r46;
  uint r47;
  uint r48;
  uint r49;
  uint r50;
  uint r51;
  uint r52;
  uint r53;
  uint r54;
  uint r55;
  uint r56;
  uint r57;
  uint r58;
  uint r59;
  uint r60;
  uint r61;
  uint r62;
  uint r63;
  uint r64;
  uint r65;
  uint r66;
  uint r67;
  uint r68;
  uint r69;
  uint r70;
  uint r71;
  uint r72;
  uint r73;
  uint r74;
  uint r75;
  uint r76;
  uint r77;
  uint r78;
  uint r79;
  uint r80;
  uint r81;
  uint r82;
  uint r83;
  uint r84;
  uint r85;
  uint r86;
  uint r87;
  uint r88;
  uint r89;
  uint r90;
  uint r91;
  uint r92;
  uint r93;
  uint r94;
  uint r95;
  uint r96;
  uint r97;
  uint r98;
  uint r99;
  uint r100;
  {
    const uint simd_id      = (get_local_size(0) * get_group_id(0) + get_local_id(0)) / 8;
    const uint simd_lane_id = get_local_id(0) & 7;
    const uint simd_ld_idx  = simd_id * 800 + simd_lane_id;
    r1   = in[simd_ld_idx + 0   * 8];
    r2   = in[simd_ld_idx + 1   * 8];
    r3   = in[simd_ld_idx + 2   * 8];
    r4   = in[simd_ld_idx + 3   * 8];
    r5   = in[simd_ld_idx + 4   * 8];
    r6   = in[simd_ld_idx + 5   * 8];
    r7   = in[simd_ld_idx + 6   * 8];
    r8   = in[simd_ld_idx + 7   * 8];
    r9   = in[simd_ld_idx + 8   * 8];
    r10  = in[simd_ld_idx + 9   * 8];
    r11  = in[simd_ld_idx + 10  * 8];
    r12  = in[simd_ld_idx + 11  * 8];
    r13  = in[simd_ld_idx + 12  * 8];
    r14  = in[simd_ld_idx + 13  * 8];
    r15  = in[simd_ld_idx + 14  * 8];
    r16  = in[simd_ld_idx + 15  * 8];
    r17  = in[simd_ld_idx + 16  * 8];
    r18  = in[simd_ld_idx + 17  * 8];
    r19  = in[simd_ld_idx + 18  * 8];
    r20  = in[simd_ld_idx + 19  * 8];
    r21  = in[simd_ld_idx + 20  * 8];
    r22  = in[simd_ld_idx + 21  * 8];
    r23  = in[simd_ld_idx + 22  * 8];
    r24  = in[simd_ld_idx + 23  * 8];
    r25  = in[simd_ld_idx + 24  * 8];
    r26  = in[simd_ld_idx + 25  * 8];
    r27  = in[simd_ld_idx + 26  * 8];
    r28  = in[simd_ld_idx + 27  * 8];
    r29  = in[simd_ld_idx + 28  * 8];
    r30  = in[simd_ld_idx + 29  * 8];
    r31  = in[simd_ld_idx + 30  * 8];
    r32  = in[simd_ld_idx + 31  * 8];
    r33  = in[simd_ld_idx + 32  * 8];
    r34  = in[simd_ld_idx + 33  * 8];
    r35  = in[simd_ld_idx + 34  * 8];
    r36  = in[simd_ld_idx + 35  * 8];
    r37  = in[simd_ld_idx + 36  * 8];
    r38  = in[simd_ld_idx + 37  * 8];
    r39  = in[simd_ld_idx + 38  * 8];
    r40  = in[simd_ld_idx + 39  * 8];
    r41  = in[simd_ld_idx + 40  * 8];
    r42  = in[simd_ld_idx + 41  * 8];
    r43  = in[simd_ld_idx + 42  * 8];
    r44  = in[simd_ld_idx + 43  * 8];
    r45  = in[simd_ld_idx + 44  * 8];
    r46  = in[simd_ld_idx + 45  * 8];
    r47  = in[simd_ld_idx + 46  * 8];
    r48  = in[simd_ld_idx + 47  * 8];
    r49  = in[simd_ld_idx + 48  * 8];
    r50  = in[simd_ld_idx + 49  * 8];
    r51  = in[simd_ld_idx + 50  * 8];
    r52  = in[simd_ld_idx + 51  * 8];
    r53  = in[simd_ld_idx + 52  * 8];
    r54  = in[simd_ld_idx + 53  * 8];
    r55  = in[simd_ld_idx + 54  * 8];
    r56  = in[simd_ld_idx + 55  * 8];
    r57  = in[simd_ld_idx + 56  * 8];
    r58  = in[simd_ld_idx + 57  * 8];
    r59  = in[simd_ld_idx + 58  * 8];
    r60  = in[simd_ld_idx + 59  * 8];
    r61  = in[simd_ld_idx + 60  * 8];
    r62  = in[simd_ld_idx + 61  * 8];
    r63  = in[simd_ld_idx + 62  * 8];
    r64  = in[simd_ld_idx + 63  * 8];
    r65  = in[simd_ld_idx + 64  * 8];
    r66  = in[simd_ld_idx + 65  * 8];
    r67  = in[simd_ld_idx + 66  * 8];
    r68  = in[simd_ld_idx + 67  * 8];
    r69  = in[simd_ld_idx + 68  * 8];
    r70  = in[simd_ld_idx + 69  * 8];
    r71  = in[simd_ld_idx + 70  * 8];
    r72  = in[simd_ld_idx + 71  * 8];
    r73  = in[simd_ld_idx + 72  * 8];
    r74  = in[simd_ld_idx + 73  * 8];
    r75  = in[simd_ld_idx + 74  * 8];
    r76  = in[simd_ld_idx + 75  * 8];
    r77  = in[simd_ld_idx + 76  * 8];
    r78  = in[simd_ld_idx + 77  * 8];
    r79  = in[simd_ld_idx + 78  * 8];
    r80  = in[simd_ld_idx + 79  * 8];
    r81  = in[simd_ld_idx + 80  * 8];
    r82  = in[simd_ld_idx + 81  * 8];
    r83  = in[simd_ld_idx + 82  * 8];
    r84  = in[simd_ld_idx + 83  * 8];
    r85  = in[simd_ld_idx + 84  * 8];
    r86  = in[simd_ld_idx + 85  * 8];
    r87  = in[simd_ld_idx + 86  * 8];
    r88  = in[simd_ld_idx + 87  * 8];
    r89  = in[simd_ld_idx + 88  * 8];
    r90  = in[simd_ld_idx + 89  * 8];
    r91  = in[simd_ld_idx + 90  * 8];
    r92  = in[simd_ld_idx + 91  * 8];
    r93  = in[simd_ld_idx + 92  * 8];
    r94  = in[simd_ld_idx + 93  * 8];
    r95  = in[simd_ld_idx + 94  * 8];
    r96  = in[simd_ld_idx + 95  * 8];
    r97  = in[simd_ld_idx + 96  * 8];
    r98  = in[simd_ld_idx + 97  * 8];
    r99  = in[simd_ld_idx + 98  * 8];
    r100 = in[simd_ld_idx + 99  * 8];
  }
  {
    const uint  t = min(r1  ,r65 );
    r65           = max(r1  ,r65 );
    r1            = t;
  }
  {
    const uint  t = min(r2  ,r66 );
    r66           = max(r2  ,r66 );
    r2            = t;
  }
  {
    const uint  t = min(r3  ,r67 );
    r67           = max(r3  ,r67 );
    r3            = t;
  }
  {
    const uint  t = min(r4  ,r68 );
    r68           = max(r4  ,r68 );
    r4            = t;
  }
  {
    const uint  t = min(r5  ,r69 );
    r69           = max(r5  ,r69 );
    r5            = t;
  }
  {
    const uint  t = min(r6  ,r70 );
    r70           = max(r6  ,r70 );
    r6            = t;
  }
  {
    const uint  t = min(r7  ,r71 );
    r71           = max(r7  ,r71 );
    r7            = t;
  }
  {
    const uint  t = min(r8  ,r72 );
    r72           = max(r8  ,r72 );
    r8            = t;
  }
  {
    const uint  t = min(r9  ,r73 );
    r73           = max(r9  ,r73 );
    r9            = t;
  }
  {
    const uint  t = min(r10 ,r74 );
    r74           = max(r10 ,r74 );
    r10           = t;
  }
  {
    const uint  t = min(r11 ,r75 );
    r75           = max(r11 ,r75 );
    r11           = t;
  }
  {
    const uint  t = min(r12 ,r76 );
    r76           = max(r12 ,r76 );
    r12           = t;
  }
  {
    const uint  t = min(r13 ,r77 );
    r77           = max(r13 ,r77 );
    r13           = t;
  }
  {
    const uint  t = min(r14 ,r78 );
    r78           = max(r14 ,r78 );
    r14           = t;
  }
  {
    const uint  t = min(r15 ,r79 );
    r79           = max(r15 ,r79 );
    r15           = t;
  }
  {
    const uint  t = min(r16 ,r80 );
    r80           = max(r16 ,r80 );
    r16           = t;
  }
  {
    const uint  t = min(r17 ,r81 );
    r81           = max(r17 ,r81 );
    r17           = t;
  }
  {
    const uint  t = min(r18 ,r82 );
    r82           = max(r18 ,r82 );
    r18           = t;
  }
  {
    const uint  t = min(r19 ,r83 );
    r83           = max(r19 ,r83 );
    r19           = t;
  }
  {
    const uint  t = min(r20 ,r84 );
    r84           = max(r20 ,r84 );
    r20           = t;
  }
  {
    const uint  t = min(r21 ,r85 );
    r85           = max(r21 ,r85 );
    r21           = t;
  }
  {
    const uint  t = min(r22 ,r86 );
    r86           = max(r22 ,r86 );
    r22           = t;
  }
  {
    const uint  t = min(r23 ,r87 );
    r87           = max(r23 ,r87 );
    r23           = t;
  }
  {
    const uint  t = min(r24 ,r88 );
    r88           = max(r24 ,r88 );
    r24           = t;
  }
  {
    const uint  t = min(r25 ,r89 );
    r89           = max(r25 ,r89 );
    r25           = t;
  }
  {
    const uint  t = min(r26 ,r90 );
    r90           = max(r26 ,r90 );
    r26           = t;
  }
  {
    const uint  t = min(r27 ,r91 );
    r91           = max(r27 ,r91 );
    r27           = t;
  }
  {
    const uint  t = min(r28 ,r92 );
    r92           = max(r28 ,r92 );
    r28           = t;
  }
  {
    const uint  t = min(r29 ,r93 );
    r93           = max(r29 ,r93 );
    r29           = t;
  }
  {
    const uint  t = min(r30 ,r94 );
    r94           = max(r30 ,r94 );
    r30           = t;
  }
  {
    const uint  t = min(r31 ,r95 );
    r95           = max(r31 ,r95 );
    r31           = t;
  }
  {
    const uint  t = min(r32 ,r96 );
    r96           = max(r32 ,r96 );
    r32           = t;
  }
  {
    const uint  t = min(r33 ,r97 );
    r97           = max(r33 ,r97 );
    r33           = t;
  }
  {
    const uint  t = min(r34 ,r98 );
    r98           = max(r34 ,r98 );
    r34           = t;
  }
  {
    const uint  t = min(r35 ,r99 );
    r99           = max(r35 ,r99 );
    r35           = t;
  }
  {
    const uint  t = min(r36 ,r100);
    r100          = max(r36 ,r100);
    r36           = t;
  }
  {
    const uint  t = min(r1  ,r33 );
    r33           = max(r1  ,r33 );
    r1            = t;
  }
  {
    const uint  t = min(r2  ,r34 );
    r34           = max(r2  ,r34 );
    r2            = t;
  }
  {
    const uint  t = min(r3  ,r35 );
    r35           = max(r3  ,r35 );
    r3            = t;
  }
  {
    const uint  t = min(r4  ,r36 );
    r36           = max(r4  ,r36 );
    r4            = t;
  }
  {
    const uint  t = min(r5  ,r37 );
    r37           = max(r5  ,r37 );
    r5            = t;
  }
  {
    const uint  t = min(r6  ,r38 );
    r38           = max(r6  ,r38 );
    r6            = t;
  }
  {
    const uint  t = min(r7  ,r39 );
    r39           = max(r7  ,r39 );
    r7            = t;
  }
  {
    const uint  t = min(r8  ,r40 );
    r40           = max(r8  ,r40 );
    r8            = t;
  }
  {
    const uint  t = min(r9  ,r41 );
    r41           = max(r9  ,r41 );
    r9            = t;
  }
  {
    const uint  t = min(r10 ,r42 );
    r42           = max(r10 ,r42 );
    r10           = t;
  }
  {
    const uint  t = min(r11 ,r43 );
    r43           = max(r11 ,r43 );
    r11           = t;
  }
  {
    const uint  t = min(r12 ,r44 );
    r44           = max(r12 ,r44 );
    r12           = t;
  }
  {
    const uint  t = min(r13 ,r45 );
    r45           = max(r13 ,r45 );
    r13           = t;
  }
  {
    const uint  t = min(r14 ,r46 );
    r46           = max(r14 ,r46 );
    r14           = t;
  }
  {
    const uint  t = min(r15 ,r47 );
    r47           = max(r15 ,r47 );
    r15           = t;
  }
  {
    const uint  t = min(r16 ,r48 );
    r48           = max(r16 ,r48 );
    r16           = t;
  }
  {
    const uint  t = min(r17 ,r49 );
    r49           = max(r17 ,r49 );
    r17           = t;
  }
  {
    const uint  t = min(r18 ,r50 );
    r50           = max(r18 ,r50 );
    r18           = t;
  }
  {
    const uint  t = min(r19 ,r51 );
    r51           = max(r19 ,r51 );
    r19           = t;
  }
  {
    const uint  t = min(r20 ,r52 );
    r52           = max(r20 ,r52 );
    r20           = t;
  }
  {
    const uint  t = min(r21 ,r53 );
    r53           = max(r21 ,r53 );
    r21           = t;
  }
  {
    const uint  t = min(r22 ,r54 );
    r54           = max(r22 ,r54 );
    r22           = t;
  }
  {
    const uint  t = min(r23 ,r55 );
    r55           = max(r23 ,r55 );
    r23           = t;
  }
  {
    const uint  t = min(r24 ,r56 );
    r56           = max(r24 ,r56 );
    r24           = t;
  }
  {
    const uint  t = min(r25 ,r57 );
    r57           = max(r25 ,r57 );
    r25           = t;
  }
  {
    const uint  t = min(r26 ,r58 );
    r58           = max(r26 ,r58 );
    r26           = t;
  }
  {
    const uint  t = min(r27 ,r59 );
    r59           = max(r27 ,r59 );
    r27           = t;
  }
  {
    const uint  t = min(r28 ,r60 );
    r60           = max(r28 ,r60 );
    r28           = t;
  }
  {
    const uint  t = min(r29 ,r61 );
    r61           = max(r29 ,r61 );
    r29           = t;
  }
  {
    const uint  t = min(r30 ,r62 );
    r62           = max(r30 ,r62 );
    r30           = t;
  }
  {
    const uint  t = min(r31 ,r63 );
    r63           = max(r31 ,r63 );
    r31           = t;
  }
  {
    const uint  t = min(r32 ,r64 );
    r64           = max(r32 ,r64 );
    r32           = t;
  }
  {
    const uint  t = min(r65 ,r97 );
    r97           = max(r65 ,r97 );
    r65           = t;
  }
  {
    const uint  t = min(r66 ,r98 );
    r98           = max(r66 ,r98 );
    r66           = t;
  }
  {
    const uint  t = min(r67 ,r99 );
    r99           = max(r67 ,r99 );
    r67           = t;
  }
  {
    const uint  t = min(r68 ,r100);
    r100          = max(r68 ,r100);
    r68           = t;
  }
  {
    const uint  t = min(r33 ,r65 );
    r65           = max(r33 ,r65 );
    r33           = t;
  }
  {
    const uint  t = min(r34 ,r66 );
    r66           = max(r34 ,r66 );
    r34           = t;
  }
  {
    const uint  t = min(r35 ,r67 );
    r67           = max(r35 ,r67 );
    r35           = t;
  }
  {
    const uint  t = min(r36 ,r68 );
    r68           = max(r36 ,r68 );
    r36           = t;
  }
  {
    const uint  t = min(r37 ,r69 );
    r69           = max(r37 ,r69 );
    r37           = t;
  }
  {
    const uint  t = min(r38 ,r70 );
    r70           = max(r38 ,r70 );
    r38           = t;
  }
  {
    const uint  t = min(r39 ,r71 );
    r71           = max(r39 ,r71 );
    r39           = t;
  }
  {
    const uint  t = min(r40 ,r72 );
    r72           = max(r40 ,r72 );
    r40           = t;
  }
  {
    const uint  t = min(r41 ,r73 );
    r73           = max(r41 ,r73 );
    r41           = t;
  }
  {
    const uint  t = min(r42 ,r74 );
    r74           = max(r42 ,r74 );
    r42           = t;
  }
  {
    const uint  t = min(r43 ,r75 );
    r75           = max(r43 ,r75 );
    r43           = t;
  }
  {
    const uint  t = min(r44 ,r76 );
    r76           = max(r44 ,r76 );
    r44           = t;
  }
  {
    const uint  t = min(r45 ,r77 );
    r77           = max(r45 ,r77 );
    r45           = t;
  }
  {
    const uint  t = min(r46 ,r78 );
    r78           = max(r46 ,r78 );
    r46           = t;
  }
  {
    const uint  t = min(r47 ,r79 );
    r79           = max(r47 ,r79 );
    r47           = t;
  }
  {
    const uint  t = min(r48 ,r80 );
    r80           = max(r48 ,r80 );
    r48           = t;
  }
  {
    const uint  t = min(r49 ,r81 );
    r81           = max(r49 ,r81 );
    r49           = t;
  }
  {
    const uint  t = min(r50 ,r82 );
    r82           = max(r50 ,r82 );
    r50           = t;
  }
  {
    const uint  t = min(r51 ,r83 );
    r83           = max(r51 ,r83 );
    r51           = t;
  }
  {
    const uint  t = min(r52 ,r84 );
    r84           = max(r52 ,r84 );
    r52           = t;
  }
  {
    const uint  t = min(r53 ,r85 );
    r85           = max(r53 ,r85 );
    r53           = t;
  }
  {
    const uint  t = min(r54 ,r86 );
    r86           = max(r54 ,r86 );
    r54           = t;
  }
  {
    const uint  t = min(r55 ,r87 );
    r87           = max(r55 ,r87 );
    r55           = t;
  }
  {
    const uint  t = min(r56 ,r88 );
    r88           = max(r56 ,r88 );
    r56           = t;
  }
  {
    const uint  t = min(r57 ,r89 );
    r89           = max(r57 ,r89 );
    r57           = t;
  }
  {
    const uint  t = min(r58 ,r90 );
    r90           = max(r58 ,r90 );
    r58           = t;
  }
  {
    const uint  t = min(r59 ,r91 );
    r91           = max(r59 ,r91 );
    r59           = t;
  }
  {
    const uint  t = min(r60 ,r92 );
    r92           = max(r60 ,r92 );
    r60           = t;
  }
  {
    const uint  t = min(r61 ,r93 );
    r93           = max(r61 ,r93 );
    r61           = t;
  }
  {
    const uint  t = min(r62 ,r94 );
    r94           = max(r62 ,r94 );
    r62           = t;
  }
  {
    const uint  t = min(r63 ,r95 );
    r95           = max(r63 ,r95 );
    r63           = t;
  }
  {
    const uint  t = min(r64 ,r96 );
    r96           = max(r64 ,r96 );
    r64           = t;
  }
  {
    const uint  t = min(r1  ,r17 );
    r17           = max(r1  ,r17 );
    r1            = t;
  }
  {
    const uint  t = min(r2  ,r18 );
    r18           = max(r2  ,r18 );
    r2            = t;
  }
  {
    const uint  t = min(r3  ,r19 );
    r19           = max(r3  ,r19 );
    r3            = t;
  }
  {
    const uint  t = min(r4  ,r20 );
    r20           = max(r4  ,r20 );
    r4            = t;
  }
  {
    const uint  t = min(r5  ,r21 );
    r21           = max(r5  ,r21 );
    r5            = t;
  }
  {
    const uint  t = min(r6  ,r22 );
    r22           = max(r6  ,r22 );
    r6            = t;
  }
  {
    const uint  t = min(r7  ,r23 );
    r23           = max(r7  ,r23 );
    r7            = t;
  }
  {
    const uint  t = min(r8  ,r24 );
    r24           = max(r8  ,r24 );
    r8            = t;
  }
  {
    const uint  t = min(r9  ,r25 );
    r25           = max(r9  ,r25 );
    r9            = t;
  }
  {
    const uint  t = min(r10 ,r26 );
    r26           = max(r10 ,r26 );
    r10           = t;
  }
  {
    const uint  t = min(r11 ,r27 );
    r27           = max(r11 ,r27 );
    r11           = t;
  }
  {
    const uint  t = min(r12 ,r28 );
    r28           = max(r12 ,r28 );
    r12           = t;
  }
  {
    const uint  t = min(r13 ,r29 );
    r29           = max(r13 ,r29 );
    r13           = t;
  }
  {
    const uint  t = min(r14 ,r30 );
    r30           = max(r14 ,r30 );
    r14           = t;
  }
  {
    const uint  t = min(r15 ,r31 );
    r31           = max(r15 ,r31 );
    r15           = t;
  }
  {
    const uint  t = min(r16 ,r32 );
    r32           = max(r16 ,r32 );
    r16           = t;
  }
  {
    const uint  t = min(r33 ,r49 );
    r49           = max(r33 ,r49 );
    r33           = t;
  }
  {
    const uint  t = min(r34 ,r50 );
    r50           = max(r34 ,r50 );
    r34           = t;
  }
  {
    const uint  t = min(r35 ,r51 );
    r51           = max(r35 ,r51 );
    r35           = t;
  }
  {
    const uint  t = min(r36 ,r52 );
    r52           = max(r36 ,r52 );
    r36           = t;
  }
  {
    const uint  t = min(r37 ,r53 );
    r53           = max(r37 ,r53 );
    r37           = t;
  }
  {
    const uint  t = min(r38 ,r54 );
    r54           = max(r38 ,r54 );
    r38           = t;
  }
  {
    const uint  t = min(r39 ,r55 );
    r55           = max(r39 ,r55 );
    r39           = t;
  }
  {
    const uint  t = min(r40 ,r56 );
    r56           = max(r40 ,r56 );
    r40           = t;
  }
  {
    const uint  t = min(r41 ,r57 );
    r57           = max(r41 ,r57 );
    r41           = t;
  }
  {
    const uint  t = min(r42 ,r58 );
    r58           = max(r42 ,r58 );
    r42           = t;
  }
  {
    const uint  t = min(r43 ,r59 );
    r59           = max(r43 ,r59 );
    r43           = t;
  }
  {
    const uint  t = min(r44 ,r60 );
    r60           = max(r44 ,r60 );
    r44           = t;
  }
  {
    const uint  t = min(r45 ,r61 );
    r61           = max(r45 ,r61 );
    r45           = t;
  }
  {
    const uint  t = min(r46 ,r62 );
    r62           = max(r46 ,r62 );
    r46           = t;
  }
  {
    const uint  t = min(r47 ,r63 );
    r63           = max(r47 ,r63 );
    r47           = t;
  }
  {
    const uint  t = min(r48 ,r64 );
    r64           = max(r48 ,r64 );
    r48           = t;
  }
  {
    const uint  t = min(r65 ,r81 );
    r81           = max(r65 ,r81 );
    r65           = t;
  }
  {
    const uint  t = min(r66 ,r82 );
    r82           = max(r66 ,r82 );
    r66           = t;
  }
  {
    const uint  t = min(r67 ,r83 );
    r83           = max(r67 ,r83 );
    r67           = t;
  }
  {
    const uint  t = min(r68 ,r84 );
    r84           = max(r68 ,r84 );
    r68           = t;
  }
  {
    const uint  t = min(r69 ,r85 );
    r85           = max(r69 ,r85 );
    r69           = t;
  }
  {
    const uint  t = min(r70 ,r86 );
    r86           = max(r70 ,r86 );
    r70           = t;
  }
  {
    const uint  t = min(r71 ,r87 );
    r87           = max(r71 ,r87 );
    r71           = t;
  }
  {
    const uint  t = min(r72 ,r88 );
    r88           = max(r72 ,r88 );
    r72           = t;
  }
  {
    const uint  t = min(r73 ,r89 );
    r89           = max(r73 ,r89 );
    r73           = t;
  }
  {
    const uint  t = min(r74 ,r90 );
    r90           = max(r74 ,r90 );
    r74           = t;
  }
  {
    const uint  t = min(r75 ,r91 );
    r91           = max(r75 ,r91 );
    r75           = t;
  }
  {
    const uint  t = min(r76 ,r92 );
    r92           = max(r76 ,r92 );
    r76           = t;
  }
  {
    const uint  t = min(r77 ,r93 );
    r93           = max(r77 ,r93 );
    r77           = t;
  }
  {
    const uint  t = min(r78 ,r94 );
    r94           = max(r78 ,r94 );
    r78           = t;
  }
  {
    const uint  t = min(r79 ,r95 );
    r95           = max(r79 ,r95 );
    r79           = t;
  }
  {
    const uint  t = min(r80 ,r96 );
    r96           = max(r80 ,r96 );
    r80           = t;
  }
  {
    const uint  t = min(r17 ,r65 );
    r65           = max(r17 ,r65 );
    r17           = t;
  }
  {
    const uint  t = min(r18 ,r66 );
    r66           = max(r18 ,r66 );
    r18           = t;
  }
  {
    const uint  t = min(r19 ,r67 );
    r67           = max(r19 ,r67 );
    r19           = t;
  }
  {
    const uint  t = min(r20 ,r68 );
    r68           = max(r20 ,r68 );
    r20           = t;
  }
  {
    const uint  t = min(r21 ,r69 );
    r69           = max(r21 ,r69 );
    r21           = t;
  }
  {
    const uint  t = min(r22 ,r70 );
    r70           = max(r22 ,r70 );
    r22           = t;
  }
  {
    const uint  t = min(r23 ,r71 );
    r71           = max(r23 ,r71 );
    r23           = t;
  }
  {
    const uint  t = min(r24 ,r72 );
    r72           = max(r24 ,r72 );
    r24           = t;
  }
  {
    const uint  t = min(r25 ,r73 );
    r73           = max(r25 ,r73 );
    r25           = t;
  }
  {
    const uint  t = min(r26 ,r74 );
    r74           = max(r26 ,r74 );
    r26           = t;
  }
  {
    const uint  t = min(r27 ,r75 );
    r75           = max(r27 ,r75 );
    r27           = t;
  }
  {
    const uint  t = min(r28 ,r76 );
    r76           = max(r28 ,r76 );
    r28           = t;
  }
  {
    const uint  t = min(r29 ,r77 );
    r77           = max(r29 ,r77 );
    r29           = t;
  }
  {
    const uint  t = min(r30 ,r78 );
    r78           = max(r30 ,r78 );
    r30           = t;
  }
  {
    const uint  t = min(r31 ,r79 );
    r79           = max(r31 ,r79 );
    r31           = t;
  }
  {
    const uint  t = min(r32 ,r80 );
    r80           = max(r32 ,r80 );
    r32           = t;
  }
  {
    const uint  t = min(r49 ,r97 );
    r97           = max(r49 ,r97 );
    r49           = t;
  }
  {
    const uint  t = min(r50 ,r98 );
    r98           = max(r50 ,r98 );
    r50           = t;
  }
  {
    const uint  t = min(r51 ,r99 );
    r99           = max(r51 ,r99 );
    r51           = t;
  }
  {
    const uint  t = min(r52 ,r100);
    r100          = max(r52 ,r100);
    r52           = t;
  }
  {
    const uint  t = min(r17 ,r33 );
    r33           = max(r17 ,r33 );
    r17           = t;
  }
  {
    const uint  t = min(r18 ,r34 );
    r34           = max(r18 ,r34 );
    r18           = t;
  }
  {
    const uint  t = min(r19 ,r35 );
    r35           = max(r19 ,r35 );
    r19           = t;
  }
  {
    const uint  t = min(r20 ,r36 );
    r36           = max(r20 ,r36 );
    r20           = t;
  }
  {
    const uint  t = min(r21 ,r37 );
    r37           = max(r21 ,r37 );
    r21           = t;
  }
  {
    const uint  t = min(r22 ,r38 );
    r38           = max(r22 ,r38 );
    r22           = t;
  }
  {
    const uint  t = min(r23 ,r39 );
    r39           = max(r23 ,r39 );
    r23           = t;
  }
  {
    const uint  t = min(r24 ,r40 );
    r40           = max(r24 ,r40 );
    r24           = t;
  }
  {
    const uint  t = min(r25 ,r41 );
    r41           = max(r25 ,r41 );
    r25           = t;
  }
  {
    const uint  t = min(r26 ,r42 );
    r42           = max(r26 ,r42 );
    r26           = t;
  }
  {
    const uint  t = min(r27 ,r43 );
    r43           = max(r27 ,r43 );
    r27           = t;
  }
  {
    const uint  t = min(r28 ,r44 );
    r44           = max(r28 ,r44 );
    r28           = t;
  }
  {
    const uint  t = min(r29 ,r45 );
    r45           = max(r29 ,r45 );
    r29           = t;
  }
  {
    const uint  t = min(r30 ,r46 );
    r46           = max(r30 ,r46 );
    r30           = t;
  }
  {
    const uint  t = min(r31 ,r47 );
    r47           = max(r31 ,r47 );
    r31           = t;
  }
  {
    const uint  t = min(r32 ,r48 );
    r48           = max(r32 ,r48 );
    r32           = t;
  }
  {
    const uint  t = min(r49 ,r65 );
    r65           = max(r49 ,r65 );
    r49           = t;
  }
  {
    const uint  t = min(r50 ,r66 );
    r66           = max(r50 ,r66 );
    r50           = t;
  }
  {
    const uint  t = min(r51 ,r67 );
    r67           = max(r51 ,r67 );
    r51           = t;
  }
  {
    const uint  t = min(r52 ,r68 );
    r68           = max(r52 ,r68 );
    r52           = t;
  }
  {
    const uint  t = min(r53 ,r69 );
    r69           = max(r53 ,r69 );
    r53           = t;
  }
  {
    const uint  t = min(r54 ,r70 );
    r70           = max(r54 ,r70 );
    r54           = t;
  }
  {
    const uint  t = min(r55 ,r71 );
    r71           = max(r55 ,r71 );
    r55           = t;
  }
  {
    const uint  t = min(r56 ,r72 );
    r72           = max(r56 ,r72 );
    r56           = t;
  }
  {
    const uint  t = min(r57 ,r73 );
    r73           = max(r57 ,r73 );
    r57           = t;
  }
  {
    const uint  t = min(r58 ,r74 );
    r74           = max(r58 ,r74 );
    r58           = t;
  }
  {
    const uint  t = min(r59 ,r75 );
    r75           = max(r59 ,r75 );
    r59           = t;
  }
  {
    const uint  t = min(r60 ,r76 );
    r76           = max(r60 ,r76 );
    r60           = t;
  }
  {
    const uint  t = min(r61 ,r77 );
    r77           = max(r61 ,r77 );
    r61           = t;
  }
  {
    const uint  t = min(r62 ,r78 );
    r78           = max(r62 ,r78 );
    r62           = t;
  }
  {
    const uint  t = min(r63 ,r79 );
    r79           = max(r63 ,r79 );
    r63           = t;
  }
  {
    const uint  t = min(r64 ,r80 );
    r80           = max(r64 ,r80 );
    r64           = t;
  }
  {
    const uint  t = min(r81 ,r97 );
    r97           = max(r81 ,r97 );
    r81           = t;
  }
  {
    const uint  t = min(r82 ,r98 );
    r98           = max(r82 ,r98 );
    r82           = t;
  }
  {
    const uint  t = min(r83 ,r99 );
    r99           = max(r83 ,r99 );
    r83           = t;
  }
  {
    const uint  t = min(r84 ,r100);
    r100          = max(r84 ,r100);
    r84           = t;
  }
  {
    const uint  t = min(r1  ,r9  );
    r9            = max(r1  ,r9  );
    r1            = t;
  }
  {
    const uint  t = min(r2  ,r10 );
    r10           = max(r2  ,r10 );
    r2            = t;
  }
  {
    const uint  t = min(r3  ,r11 );
    r11           = max(r3  ,r11 );
    r3            = t;
  }
  {
    const uint  t = min(r4  ,r12 );
    r12           = max(r4  ,r12 );
    r4            = t;
  }
  {
    const uint  t = min(r5  ,r13 );
    r13           = max(r5  ,r13 );
    r5            = t;
  }
  {
    const uint  t = min(r6  ,r14 );
    r14           = max(r6  ,r14 );
    r6            = t;
  }
  {
    const uint  t = min(r7  ,r15 );
    r15           = max(r7  ,r15 );
    r7            = t;
  }
  {
    const uint  t = min(r8  ,r16 );
    r16           = max(r8  ,r16 );
    r8            = t;
  }
  {
    const uint  t = min(r17 ,r25 );
    r25           = max(r17 ,r25 );
    r17           = t;
  }
  {
    const uint  t = min(r18 ,r26 );
    r26           = max(r18 ,r26 );
    r18           = t;
  }
  {
    const uint  t = min(r19 ,r27 );
    r27           = max(r19 ,r27 );
    r19           = t;
  }
  {
    const uint  t = min(r20 ,r28 );
    r28           = max(r20 ,r28 );
    r20           = t;
  }
  {
    const uint  t = min(r21 ,r29 );
    r29           = max(r21 ,r29 );
    r21           = t;
  }
  {
    const uint  t = min(r22 ,r30 );
    r30           = max(r22 ,r30 );
    r22           = t;
  }
  {
    const uint  t = min(r23 ,r31 );
    r31           = max(r23 ,r31 );
    r23           = t;
  }
  {
    const uint  t = min(r24 ,r32 );
    r32           = max(r24 ,r32 );
    r24           = t;
  }
  {
    const uint  t = min(r33 ,r41 );
    r41           = max(r33 ,r41 );
    r33           = t;
  }
  {
    const uint  t = min(r34 ,r42 );
    r42           = max(r34 ,r42 );
    r34           = t;
  }
  {
    const uint  t = min(r35 ,r43 );
    r43           = max(r35 ,r43 );
    r35           = t;
  }
  {
    const uint  t = min(r36 ,r44 );
    r44           = max(r36 ,r44 );
    r36           = t;
  }
  {
    const uint  t = min(r37 ,r45 );
    r45           = max(r37 ,r45 );
    r37           = t;
  }
  {
    const uint  t = min(r38 ,r46 );
    r46           = max(r38 ,r46 );
    r38           = t;
  }
  {
    const uint  t = min(r39 ,r47 );
    r47           = max(r39 ,r47 );
    r39           = t;
  }
  {
    const uint  t = min(r40 ,r48 );
    r48           = max(r40 ,r48 );
    r40           = t;
  }
  {
    const uint  t = min(r49 ,r57 );
    r57           = max(r49 ,r57 );
    r49           = t;
  }
  {
    const uint  t = min(r50 ,r58 );
    r58           = max(r50 ,r58 );
    r50           = t;
  }
  {
    const uint  t = min(r51 ,r59 );
    r59           = max(r51 ,r59 );
    r51           = t;
  }
  {
    const uint  t = min(r52 ,r60 );
    r60           = max(r52 ,r60 );
    r52           = t;
  }
  {
    const uint  t = min(r53 ,r61 );
    r61           = max(r53 ,r61 );
    r53           = t;
  }
  {
    const uint  t = min(r54 ,r62 );
    r62           = max(r54 ,r62 );
    r54           = t;
  }
  {
    const uint  t = min(r55 ,r63 );
    r63           = max(r55 ,r63 );
    r55           = t;
  }
  {
    const uint  t = min(r56 ,r64 );
    r64           = max(r56 ,r64 );
    r56           = t;
  }
  {
    const uint  t = min(r65 ,r73 );
    r73           = max(r65 ,r73 );
    r65           = t;
  }
  {
    const uint  t = min(r66 ,r74 );
    r74           = max(r66 ,r74 );
    r66           = t;
  }
  {
    const uint  t = min(r67 ,r75 );
    r75           = max(r67 ,r75 );
    r67           = t;
  }
  {
    const uint  t = min(r68 ,r76 );
    r76           = max(r68 ,r76 );
    r68           = t;
  }
  {
    const uint  t = min(r69 ,r77 );
    r77           = max(r69 ,r77 );
    r69           = t;
  }
  {
    const uint  t = min(r70 ,r78 );
    r78           = max(r70 ,r78 );
    r70           = t;
  }
  {
    const uint  t = min(r71 ,r79 );
    r79           = max(r71 ,r79 );
    r71           = t;
  }
  {
    const uint  t = min(r72 ,r80 );
    r80           = max(r72 ,r80 );
    r72           = t;
  }
  {
    const uint  t = min(r81 ,r89 );
    r89           = max(r81 ,r89 );
    r81           = t;
  }
  {
    const uint  t = min(r82 ,r90 );
    r90           = max(r82 ,r90 );
    r82           = t;
  }
  {
    const uint  t = min(r83 ,r91 );
    r91           = max(r83 ,r91 );
    r83           = t;
  }
  {
    const uint  t = min(r84 ,r92 );
    r92           = max(r84 ,r92 );
    r84           = t;
  }
  {
    const uint  t = min(r85 ,r93 );
    r93           = max(r85 ,r93 );
    r85           = t;
  }
  {
    const uint  t = min(r86 ,r94 );
    r94           = max(r86 ,r94 );
    r86           = t;
  }
  {
    const uint  t = min(r87 ,r95 );
    r95           = max(r87 ,r95 );
    r87           = t;
  }
  {
    const uint  t = min(r88 ,r96 );
    r96           = max(r88 ,r96 );
    r88           = t;
  }
  {
    const uint  t = min(r9  ,r65 );
    r65           = max(r9  ,r65 );
    r9            = t;
  }
  {
    const uint  t = min(r10 ,r66 );
    r66           = max(r10 ,r66 );
    r10           = t;
  }
  {
    const uint  t = min(r11 ,r67 );
    r67           = max(r11 ,r67 );
    r11           = t;
  }
  {
    const uint  t = min(r12 ,r68 );
    r68           = max(r12 ,r68 );
    r12           = t;
  }
  {
    const uint  t = min(r13 ,r69 );
    r69           = max(r13 ,r69 );
    r13           = t;
  }
  {
    const uint  t = min(r14 ,r70 );
    r70           = max(r14 ,r70 );
    r14           = t;
  }
  {
    const uint  t = min(r15 ,r71 );
    r71           = max(r15 ,r71 );
    r15           = t;
  }
  {
    const uint  t = min(r16 ,r72 );
    r72           = max(r16 ,r72 );
    r16           = t;
  }
  {
    const uint  t = min(r25 ,r81 );
    r81           = max(r25 ,r81 );
    r25           = t;
  }
  {
    const uint  t = min(r26 ,r82 );
    r82           = max(r26 ,r82 );
    r26           = t;
  }
  {
    const uint  t = min(r27 ,r83 );
    r83           = max(r27 ,r83 );
    r27           = t;
  }
  {
    const uint  t = min(r28 ,r84 );
    r84           = max(r28 ,r84 );
    r28           = t;
  }
  {
    const uint  t = min(r29 ,r85 );
    r85           = max(r29 ,r85 );
    r29           = t;
  }
  {
    const uint  t = min(r30 ,r86 );
    r86           = max(r30 ,r86 );
    r30           = t;
  }
  {
    const uint  t = min(r31 ,r87 );
    r87           = max(r31 ,r87 );
    r31           = t;
  }
  {
    const uint  t = min(r32 ,r88 );
    r88           = max(r32 ,r88 );
    r32           = t;
  }
  {
    const uint  t = min(r41 ,r97 );
    r97           = max(r41 ,r97 );
    r41           = t;
  }
  {
    const uint  t = min(r42 ,r98 );
    r98           = max(r42 ,r98 );
    r42           = t;
  }
  {
    const uint  t = min(r43 ,r99 );
    r99           = max(r43 ,r99 );
    r43           = t;
  }
  {
    const uint  t = min(r44 ,r100);
    r100          = max(r44 ,r100);
    r44           = t;
  }
  {
    const uint  t = min(r9  ,r33 );
    r33           = max(r9  ,r33 );
    r9            = t;
  }
  {
    const uint  t = min(r10 ,r34 );
    r34           = max(r10 ,r34 );
    r10           = t;
  }
  {
    const uint  t = min(r11 ,r35 );
    r35           = max(r11 ,r35 );
    r11           = t;
  }
  {
    const uint  t = min(r12 ,r36 );
    r36           = max(r12 ,r36 );
    r12           = t;
  }
  {
    const uint  t = min(r13 ,r37 );
    r37           = max(r13 ,r37 );
    r13           = t;
  }
  {
    const uint  t = min(r14 ,r38 );
    r38           = max(r14 ,r38 );
    r14           = t;
  }
  {
    const uint  t = min(r15 ,r39 );
    r39           = max(r15 ,r39 );
    r15           = t;
  }
  {
    const uint  t = min(r16 ,r40 );
    r40           = max(r16 ,r40 );
    r16           = t;
  }
  {
    const uint  t = min(r25 ,r49 );
    r49           = max(r25 ,r49 );
    r25           = t;
  }
  {
    const uint  t = min(r26 ,r50 );
    r50           = max(r26 ,r50 );
    r26           = t;
  }
  {
    const uint  t = min(r27 ,r51 );
    r51           = max(r27 ,r51 );
    r27           = t;
  }
  {
    const uint  t = min(r28 ,r52 );
    r52           = max(r28 ,r52 );
    r28           = t;
  }
  {
    const uint  t = min(r29 ,r53 );
    r53           = max(r29 ,r53 );
    r29           = t;
  }
  {
    const uint  t = min(r30 ,r54 );
    r54           = max(r30 ,r54 );
    r30           = t;
  }
  {
    const uint  t = min(r31 ,r55 );
    r55           = max(r31 ,r55 );
    r31           = t;
  }
  {
    const uint  t = min(r32 ,r56 );
    r56           = max(r32 ,r56 );
    r32           = t;
  }
  {
    const uint  t = min(r41 ,r65 );
    r65           = max(r41 ,r65 );
    r41           = t;
  }
  {
    const uint  t = min(r42 ,r66 );
    r66           = max(r42 ,r66 );
    r42           = t;
  }
  {
    const uint  t = min(r43 ,r67 );
    r67           = max(r43 ,r67 );
    r43           = t;
  }
  {
    const uint  t = min(r44 ,r68 );
    r68           = max(r44 ,r68 );
    r44           = t;
  }
  {
    const uint  t = min(r45 ,r69 );
    r69           = max(r45 ,r69 );
    r45           = t;
  }
  {
    const uint  t = min(r46 ,r70 );
    r70           = max(r46 ,r70 );
    r46           = t;
  }
  {
    const uint  t = min(r47 ,r71 );
    r71           = max(r47 ,r71 );
    r47           = t;
  }
  {
    const uint  t = min(r48 ,r72 );
    r72           = max(r48 ,r72 );
    r48           = t;
  }
  {
    const uint  t = min(r57 ,r81 );
    r81           = max(r57 ,r81 );
    r57           = t;
  }
  {
    const uint  t = min(r58 ,r82 );
    r82           = max(r58 ,r82 );
    r58           = t;
  }
  {
    const uint  t = min(r59 ,r83 );
    r83           = max(r59 ,r83 );
    r59           = t;
  }
  {
    const uint  t = min(r60 ,r84 );
    r84           = max(r60 ,r84 );
    r60           = t;
  }
  {
    const uint  t = min(r61 ,r85 );
    r85           = max(r61 ,r85 );
    r61           = t;
  }
  {
    const uint  t = min(r62 ,r86 );
    r86           = max(r62 ,r86 );
    r62           = t;
  }
  {
    const uint  t = min(r63 ,r87 );
    r87           = max(r63 ,r87 );
    r63           = t;
  }
  {
    const uint  t = min(r64 ,r88 );
    r88           = max(r64 ,r88 );
    r64           = t;
  }
  {
    const uint  t = min(r73 ,r97 );
    r97           = max(r73 ,r97 );
    r73           = t;
  }
  {
    const uint  t = min(r74 ,r98 );
    r98           = max(r74 ,r98 );
    r74           = t;
  }
  {
    const uint  t = min(r75 ,r99 );
    r99           = max(r75 ,r99 );
    r75           = t;
  }
  {
    const uint  t = min(r76 ,r100);
    r100          = max(r76 ,r100);
    r76           = t;
  }
  {
    const uint  t = min(r9  ,r17 );
    r17           = max(r9  ,r17 );
    r9            = t;
  }
  {
    const uint  t = min(r10 ,r18 );
    r18           = max(r10 ,r18 );
    r10           = t;
  }
  {
    const uint  t = min(r11 ,r19 );
    r19           = max(r11 ,r19 );
    r11           = t;
  }
  {
    const uint  t = min(r12 ,r20 );
    r20           = max(r12 ,r20 );
    r12           = t;
  }
  {
    const uint  t = min(r13 ,r21 );
    r21           = max(r13 ,r21 );
    r13           = t;
  }
  {
    const uint  t = min(r14 ,r22 );
    r22           = max(r14 ,r22 );
    r14           = t;
  }
  {
    const uint  t = min(r15 ,r23 );
    r23           = max(r15 ,r23 );
    r15           = t;
  }
  {
    const uint  t = min(r16 ,r24 );
    r24           = max(r16 ,r24 );
    r16           = t;
  }
  {
    const uint  t = min(r25 ,r33 );
    r33           = max(r25 ,r33 );
    r25           = t;
  }
  {
    const uint  t = min(r26 ,r34 );
    r34           = max(r26 ,r34 );
    r26           = t;
  }
  {
    const uint  t = min(r27 ,r35 );
    r35           = max(r27 ,r35 );
    r27           = t;
  }
  {
    const uint  t = min(r28 ,r36 );
    r36           = max(r28 ,r36 );
    r28           = t;
  }
  {
    const uint  t = min(r29 ,r37 );
    r37           = max(r29 ,r37 );
    r29           = t;
  }
  {
    const uint  t = min(r30 ,r38 );
    r38           = max(r30 ,r38 );
    r30           = t;
  }
  {
    const uint  t = min(r31 ,r39 );
    r39           = max(r31 ,r39 );
    r31           = t;
  }
  {
    const uint  t = min(r32 ,r40 );
    r40           = max(r32 ,r40 );
    r32           = t;
  }
  {
    const uint  t = min(r41 ,r49 );
    r49           = max(r41 ,r49 );
    r41           = t;
  }
  {
    const uint  t = min(r42 ,r50 );
    r50           = max(r42 ,r50 );
    r42           = t;
  }
  {
    const uint  t = min(r43 ,r51 );
    r51           = max(r43 ,r51 );
    r43           = t;
  }
  {
    const uint  t = min(r44 ,r52 );
    r52           = max(r44 ,r52 );
    r44           = t;
  }
  {
    const uint  t = min(r45 ,r53 );
    r53           = max(r45 ,r53 );
    r45           = t;
  }
  {
    const uint  t = min(r46 ,r54 );
    r54           = max(r46 ,r54 );
    r46           = t;
  }
  {
    const uint  t = min(r47 ,r55 );
    r55           = max(r47 ,r55 );
    r47           = t;
  }
  {
    const uint  t = min(r48 ,r56 );
    r56           = max(r48 ,r56 );
    r48           = t;
  }
  {
    const uint  t = min(r57 ,r65 );
    r65           = max(r57 ,r65 );
    r57           = t;
  }
  {
    const uint  t = min(r58 ,r66 );
    r66           = max(r58 ,r66 );
    r58           = t;
  }
  {
    const uint  t = min(r59 ,r67 );
    r67           = max(r59 ,r67 );
    r59           = t;
  }
  {
    const uint  t = min(r60 ,r68 );
    r68           = max(r60 ,r68 );
    r60           = t;
  }
  {
    const uint  t = min(r61 ,r69 );
    r69           = max(r61 ,r69 );
    r61           = t;
  }
  {
    const uint  t = min(r62 ,r70 );
    r70           = max(r62 ,r70 );
    r62           = t;
  }
  {
    const uint  t = min(r63 ,r71 );
    r71           = max(r63 ,r71 );
    r63           = t;
  }
  {
    const uint  t = min(r64 ,r72 );
    r72           = max(r64 ,r72 );
    r64           = t;
  }
  {
    const uint  t = min(r73 ,r81 );
    r81           = max(r73 ,r81 );
    r73           = t;
  }
  {
    const uint  t = min(r74 ,r82 );
    r82           = max(r74 ,r82 );
    r74           = t;
  }
  {
    const uint  t = min(r75 ,r83 );
    r83           = max(r75 ,r83 );
    r75           = t;
  }
  {
    const uint  t = min(r76 ,r84 );
    r84           = max(r76 ,r84 );
    r76           = t;
  }
  {
    const uint  t = min(r77 ,r85 );
    r85           = max(r77 ,r85 );
    r77           = t;
  }
  {
    const uint  t = min(r78 ,r86 );
    r86           = max(r78 ,r86 );
    r78           = t;
  }
  {
    const uint  t = min(r79 ,r87 );
    r87           = max(r79 ,r87 );
    r79           = t;
  }
  {
    const uint  t = min(r80 ,r88 );
    r88           = max(r80 ,r88 );
    r80           = t;
  }
  {
    const uint  t = min(r89 ,r97 );
    r97           = max(r89 ,r97 );
    r89           = t;
  }
  {
    const uint  t = min(r90 ,r98 );
    r98           = max(r90 ,r98 );
    r90           = t;
  }
  {
    const uint  t = min(r91 ,r99 );
    r99           = max(r91 ,r99 );
    r91           = t;
  }
  {
    const uint  t = min(r92 ,r100);
    r100          = max(r92 ,r100);
    r92           = t;
  }
  {
    const uint  t = min(r1  ,r5  );
    r5            = max(r1  ,r5  );
    r1            = t;
  }
  {
    const uint  t = min(r2  ,r6  );
    r6            = max(r2  ,r6  );
    r2            = t;
  }
  {
    const uint  t = min(r3  ,r7  );
    r7            = max(r3  ,r7  );
    r3            = t;
  }
  {
    const uint  t = min(r4  ,r8  );
    r8            = max(r4  ,r8  );
    r4            = t;
  }
  {
    const uint  t = min(r9  ,r13 );
    r13           = max(r9  ,r13 );
    r9            = t;
  }
  {
    const uint  t = min(r10 ,r14 );
    r14           = max(r10 ,r14 );
    r10           = t;
  }
  {
    const uint  t = min(r11 ,r15 );
    r15           = max(r11 ,r15 );
    r11           = t;
  }
  {
    const uint  t = min(r12 ,r16 );
    r16           = max(r12 ,r16 );
    r12           = t;
  }
  {
    const uint  t = min(r17 ,r21 );
    r21           = max(r17 ,r21 );
    r17           = t;
  }
  {
    const uint  t = min(r18 ,r22 );
    r22           = max(r18 ,r22 );
    r18           = t;
  }
  {
    const uint  t = min(r19 ,r23 );
    r23           = max(r19 ,r23 );
    r19           = t;
  }
  {
    const uint  t = min(r20 ,r24 );
    r24           = max(r20 ,r24 );
    r20           = t;
  }
  {
    const uint  t = min(r25 ,r29 );
    r29           = max(r25 ,r29 );
    r25           = t;
  }
  {
    const uint  t = min(r26 ,r30 );
    r30           = max(r26 ,r30 );
    r26           = t;
  }
  {
    const uint  t = min(r27 ,r31 );
    r31           = max(r27 ,r31 );
    r27           = t;
  }
  {
    const uint  t = min(r28 ,r32 );
    r32           = max(r28 ,r32 );
    r28           = t;
  }
  {
    const uint  t = min(r33 ,r37 );
    r37           = max(r33 ,r37 );
    r33           = t;
  }
  {
    const uint  t = min(r34 ,r38 );
    r38           = max(r34 ,r38 );
    r34           = t;
  }
  {
    const uint  t = min(r35 ,r39 );
    r39           = max(r35 ,r39 );
    r35           = t;
  }
  {
    const uint  t = min(r36 ,r40 );
    r40           = max(r36 ,r40 );
    r36           = t;
  }
  {
    const uint  t = min(r41 ,r45 );
    r45           = max(r41 ,r45 );
    r41           = t;
  }
  {
    const uint  t = min(r42 ,r46 );
    r46           = max(r42 ,r46 );
    r42           = t;
  }
  {
    const uint  t = min(r43 ,r47 );
    r47           = max(r43 ,r47 );
    r43           = t;
  }
  {
    const uint  t = min(r44 ,r48 );
    r48           = max(r44 ,r48 );
    r44           = t;
  }
  {
    const uint  t = min(r49 ,r53 );
    r53           = max(r49 ,r53 );
    r49           = t;
  }
  {
    const uint  t = min(r50 ,r54 );
    r54           = max(r50 ,r54 );
    r50           = t;
  }
  {
    const uint  t = min(r51 ,r55 );
    r55           = max(r51 ,r55 );
    r51           = t;
  }
  {
    const uint  t = min(r52 ,r56 );
    r56           = max(r52 ,r56 );
    r52           = t;
  }
  {
    const uint  t = min(r57 ,r61 );
    r61           = max(r57 ,r61 );
    r57           = t;
  }
  {
    const uint  t = min(r58 ,r62 );
    r62           = max(r58 ,r62 );
    r58           = t;
  }
  {
    const uint  t = min(r59 ,r63 );
    r63           = max(r59 ,r63 );
    r59           = t;
  }
  {
    const uint  t = min(r60 ,r64 );
    r64           = max(r60 ,r64 );
    r60           = t;
  }
  {
    const uint  t = min(r65 ,r69 );
    r69           = max(r65 ,r69 );
    r65           = t;
  }
  {
    const uint  t = min(r66 ,r70 );
    r70           = max(r66 ,r70 );
    r66           = t;
  }
  {
    const uint  t = min(r67 ,r71 );
    r71           = max(r67 ,r71 );
    r67           = t;
  }
  {
    const uint  t = min(r68 ,r72 );
    r72           = max(r68 ,r72 );
    r68           = t;
  }
  {
    const uint  t = min(r73 ,r77 );
    r77           = max(r73 ,r77 );
    r73           = t;
  }
  {
    const uint  t = min(r74 ,r78 );
    r78           = max(r74 ,r78 );
    r74           = t;
  }
  {
    const uint  t = min(r75 ,r79 );
    r79           = max(r75 ,r79 );
    r75           = t;
  }
  {
    const uint  t = min(r76 ,r80 );
    r80           = max(r76 ,r80 );
    r76           = t;
  }
  {
    const uint  t = min(r81 ,r85 );
    r85           = max(r81 ,r85 );
    r81           = t;
  }
  {
    const uint  t = min(r82 ,r86 );
    r86           = max(r82 ,r86 );
    r82           = t;
  }
  {
    const uint  t = min(r83 ,r87 );
    r87           = max(r83 ,r87 );
    r83           = t;
  }
  {
    const uint  t = min(r84 ,r88 );
    r88           = max(r84 ,r88 );
    r84           = t;
  }
  {
    const uint  t = min(r89 ,r93 );
    r93           = max(r89 ,r93 );
    r89           = t;
  }
  {
    const uint  t = min(r90 ,r94 );
    r94           = max(r90 ,r94 );
    r90           = t;
  }
  {
    const uint  t = min(r91 ,r95 );
    r95           = max(r91 ,r95 );
    r91           = t;
  }
  {
    const uint  t = min(r92 ,r96 );
    r96           = max(r92 ,r96 );
    r92           = t;
  }
  {
    const uint  t = min(r5  ,r65 );
    r65           = max(r5  ,r65 );
    r5            = t;
  }
  {
    const uint  t = min(r6  ,r66 );
    r66           = max(r6  ,r66 );
    r6            = t;
  }
  {
    const uint  t = min(r7  ,r67 );
    r67           = max(r7  ,r67 );
    r7            = t;
  }
  {
    const uint  t = min(r8  ,r68 );
    r68           = max(r8  ,r68 );
    r8            = t;
  }
  {
    const uint  t = min(r13 ,r73 );
    r73           = max(r13 ,r73 );
    r13           = t;
  }
  {
    const uint  t = min(r14 ,r74 );
    r74           = max(r14 ,r74 );
    r14           = t;
  }
  {
    const uint  t = min(r15 ,r75 );
    r75           = max(r15 ,r75 );
    r15           = t;
  }
  {
    const uint  t = min(r16 ,r76 );
    r76           = max(r16 ,r76 );
    r16           = t;
  }
  {
    const uint  t = min(r21 ,r81 );
    r81           = max(r21 ,r81 );
    r21           = t;
  }
  {
    const uint  t = min(r22 ,r82 );
    r82           = max(r22 ,r82 );
    r22           = t;
  }
  {
    const uint  t = min(r23 ,r83 );
    r83           = max(r23 ,r83 );
    r23           = t;
  }
  {
    const uint  t = min(r24 ,r84 );
    r84           = max(r24 ,r84 );
    r24           = t;
  }
  {
    const uint  t = min(r29 ,r89 );
    r89           = max(r29 ,r89 );
    r29           = t;
  }
  {
    const uint  t = min(r30 ,r90 );
    r90           = max(r30 ,r90 );
    r30           = t;
  }
  {
    const uint  t = min(r31 ,r91 );
    r91           = max(r31 ,r91 );
    r31           = t;
  }
  {
    const uint  t = min(r32 ,r92 );
    r92           = max(r32 ,r92 );
    r32           = t;
  }
  {
    const uint  t = min(r37 ,r97 );
    r97           = max(r37 ,r97 );
    r37           = t;
  }
  {
    const uint  t = min(r38 ,r98 );
    r98           = max(r38 ,r98 );
    r38           = t;
  }
  {
    const uint  t = min(r39 ,r99 );
    r99           = max(r39 ,r99 );
    r39           = t;
  }
  {
    const uint  t = min(r40 ,r100);
    r100          = max(r40 ,r100);
    r40           = t;
  }
  {
    const uint  t = min(r5  ,r33 );
    r33           = max(r5  ,r33 );
    r5            = t;
  }
  {
    const uint  t = min(r6  ,r34 );
    r34           = max(r6  ,r34 );
    r6            = t;
  }
  {
    const uint  t = min(r7  ,r35 );
    r35           = max(r7  ,r35 );
    r7            = t;
  }
  {
    const uint  t = min(r8  ,r36 );
    r36           = max(r8  ,r36 );
    r8            = t;
  }
  {
    const uint  t = min(r13 ,r41 );
    r41           = max(r13 ,r41 );
    r13           = t;
  }
  {
    const uint  t = min(r14 ,r42 );
    r42           = max(r14 ,r42 );
    r14           = t;
  }
  {
    const uint  t = min(r15 ,r43 );
    r43           = max(r15 ,r43 );
    r15           = t;
  }
  {
    const uint  t = min(r16 ,r44 );
    r44           = max(r16 ,r44 );
    r16           = t;
  }
  {
    const uint  t = min(r21 ,r49 );
    r49           = max(r21 ,r49 );
    r21           = t;
  }
  {
    const uint  t = min(r22 ,r50 );
    r50           = max(r22 ,r50 );
    r22           = t;
  }
  {
    const uint  t = min(r23 ,r51 );
    r51           = max(r23 ,r51 );
    r23           = t;
  }
  {
    const uint  t = min(r24 ,r52 );
    r52           = max(r24 ,r52 );
    r24           = t;
  }
  {
    const uint  t = min(r29 ,r57 );
    r57           = max(r29 ,r57 );
    r29           = t;
  }
  {
    const uint  t = min(r30 ,r58 );
    r58           = max(r30 ,r58 );
    r30           = t;
  }
  {
    const uint  t = min(r31 ,r59 );
    r59           = max(r31 ,r59 );
    r31           = t;
  }
  {
    const uint  t = min(r32 ,r60 );
    r60           = max(r32 ,r60 );
    r32           = t;
  }
  {
    const uint  t = min(r37 ,r65 );
    r65           = max(r37 ,r65 );
    r37           = t;
  }
  {
    const uint  t = min(r38 ,r66 );
    r66           = max(r38 ,r66 );
    r38           = t;
  }
  {
    const uint  t = min(r39 ,r67 );
    r67           = max(r39 ,r67 );
    r39           = t;
  }
  {
    const uint  t = min(r40 ,r68 );
    r68           = max(r40 ,r68 );
    r40           = t;
  }
  {
    const uint  t = min(r45 ,r73 );
    r73           = max(r45 ,r73 );
    r45           = t;
  }
  {
    const uint  t = min(r46 ,r74 );
    r74           = max(r46 ,r74 );
    r46           = t;
  }
  {
    const uint  t = min(r47 ,r75 );
    r75           = max(r47 ,r75 );
    r47           = t;
  }
  {
    const uint  t = min(r48 ,r76 );
    r76           = max(r48 ,r76 );
    r48           = t;
  }
  {
    const uint  t = min(r53 ,r81 );
    r81           = max(r53 ,r81 );
    r53           = t;
  }
  {
    const uint  t = min(r54 ,r82 );
    r82           = max(r54 ,r82 );
    r54           = t;
  }
  {
    const uint  t = min(r55 ,r83 );
    r83           = max(r55 ,r83 );
    r55           = t;
  }
  {
    const uint  t = min(r56 ,r84 );
    r84           = max(r56 ,r84 );
    r56           = t;
  }
  {
    const uint  t = min(r61 ,r89 );
    r89           = max(r61 ,r89 );
    r61           = t;
  }
  {
    const uint  t = min(r62 ,r90 );
    r90           = max(r62 ,r90 );
    r62           = t;
  }
  {
    const uint  t = min(r63 ,r91 );
    r91           = max(r63 ,r91 );
    r63           = t;
  }
  {
    const uint  t = min(r64 ,r92 );
    r92           = max(r64 ,r92 );
    r64           = t;
  }
  {
    const uint  t = min(r69 ,r97 );
    r97           = max(r69 ,r97 );
    r69           = t;
  }
  {
    const uint  t = min(r70 ,r98 );
    r98           = max(r70 ,r98 );
    r70           = t;
  }
  {
    const uint  t = min(r71 ,r99 );
    r99           = max(r71 ,r99 );
    r71           = t;
  }
  {
    const uint  t = min(r72 ,r100);
    r100          = max(r72 ,r100);
    r72           = t;
  }
  {
    const uint  t = min(r5  ,r17 );
    r17           = max(r5  ,r17 );
    r5            = t;
  }
  {
    const uint  t = min(r6  ,r18 );
    r18           = max(r6  ,r18 );
    r6            = t;
  }
  {
    const uint  t = min(r7  ,r19 );
    r19           = max(r7  ,r19 );
    r7            = t;
  }
  {
    const uint  t = min(r8  ,r20 );
    r20           = max(r8  ,r20 );
    r8            = t;
  }
  {
    const uint  t = min(r13 ,r25 );
    r25           = max(r13 ,r25 );
    r13           = t;
  }
  {
    const uint  t = min(r14 ,r26 );
    r26           = max(r14 ,r26 );
    r14           = t;
  }
  {
    const uint  t = min(r15 ,r27 );
    r27           = max(r15 ,r27 );
    r15           = t;
  }
  {
    const uint  t = min(r16 ,r28 );
    r28           = max(r16 ,r28 );
    r16           = t;
  }
  {
    const uint  t = min(r21 ,r33 );
    r33           = max(r21 ,r33 );
    r21           = t;
  }
  {
    const uint  t = min(r22 ,r34 );
    r34           = max(r22 ,r34 );
    r22           = t;
  }
  {
    const uint  t = min(r23 ,r35 );
    r35           = max(r23 ,r35 );
    r23           = t;
  }
  {
    const uint  t = min(r24 ,r36 );
    r36           = max(r24 ,r36 );
    r24           = t;
  }
  {
    const uint  t = min(r29 ,r41 );
    r41           = max(r29 ,r41 );
    r29           = t;
  }
  {
    const uint  t = min(r30 ,r42 );
    r42           = max(r30 ,r42 );
    r30           = t;
  }
  {
    const uint  t = min(r31 ,r43 );
    r43           = max(r31 ,r43 );
    r31           = t;
  }
  {
    const uint  t = min(r32 ,r44 );
    r44           = max(r32 ,r44 );
    r32           = t;
  }
  {
    const uint  t = min(r37 ,r49 );
    r49           = max(r37 ,r49 );
    r37           = t;
  }
  {
    const uint  t = min(r38 ,r50 );
    r50           = max(r38 ,r50 );
    r38           = t;
  }
  {
    const uint  t = min(r39 ,r51 );
    r51           = max(r39 ,r51 );
    r39           = t;
  }
  {
    const uint  t = min(r40 ,r52 );
    r52           = max(r40 ,r52 );
    r40           = t;
  }
  {
    const uint  t = min(r45 ,r57 );
    r57           = max(r45 ,r57 );
    r45           = t;
  }
  {
    const uint  t = min(r46 ,r58 );
    r58           = max(r46 ,r58 );
    r46           = t;
  }
  {
    const uint  t = min(r47 ,r59 );
    r59           = max(r47 ,r59 );
    r47           = t;
  }
  {
    const uint  t = min(r48 ,r60 );
    r60           = max(r48 ,r60 );
    r48           = t;
  }
  {
    const uint  t = min(r53 ,r65 );
    r65           = max(r53 ,r65 );
    r53           = t;
  }
  {
    const uint  t = min(r54 ,r66 );
    r66           = max(r54 ,r66 );
    r54           = t;
  }
  {
    const uint  t = min(r55 ,r67 );
    r67           = max(r55 ,r67 );
    r55           = t;
  }
  {
    const uint  t = min(r56 ,r68 );
    r68           = max(r56 ,r68 );
    r56           = t;
  }
  {
    const uint  t = min(r61 ,r73 );
    r73           = max(r61 ,r73 );
    r61           = t;
  }
  {
    const uint  t = min(r62 ,r74 );
    r74           = max(r62 ,r74 );
    r62           = t;
  }
  {
    const uint  t = min(r63 ,r75 );
    r75           = max(r63 ,r75 );
    r63           = t;
  }
  {
    const uint  t = min(r64 ,r76 );
    r76           = max(r64 ,r76 );
    r64           = t;
  }
  {
    const uint  t = min(r69 ,r81 );
    r81           = max(r69 ,r81 );
    r69           = t;
  }
  {
    const uint  t = min(r70 ,r82 );
    r82           = max(r70 ,r82 );
    r70           = t;
  }
  {
    const uint  t = min(r71 ,r83 );
    r83           = max(r71 ,r83 );
    r71           = t;
  }
  {
    const uint  t = min(r72 ,r84 );
    r84           = max(r72 ,r84 );
    r72           = t;
  }
  {
    const uint  t = min(r77 ,r89 );
    r89           = max(r77 ,r89 );
    r77           = t;
  }
  {
    const uint  t = min(r78 ,r90 );
    r90           = max(r78 ,r90 );
    r78           = t;
  }
  {
    const uint  t = min(r79 ,r91 );
    r91           = max(r79 ,r91 );
    r79           = t;
  }
  {
    const uint  t = min(r80 ,r92 );
    r92           = max(r80 ,r92 );
    r80           = t;
  }
  {
    const uint  t = min(r85 ,r97 );
    r97           = max(r85 ,r97 );
    r85           = t;
  }
  {
    const uint  t = min(r86 ,r98 );
    r98           = max(r86 ,r98 );
    r86           = t;
  }
  {
    const uint  t = min(r87 ,r99 );
    r99           = max(r87 ,r99 );
    r87           = t;
  }
  {
    const uint  t = min(r88 ,r100);
    r100          = max(r88 ,r100);
    r88           = t;
  }
  {
    const uint  t = min(r5  ,r9  );
    r9            = max(r5  ,r9  );
    r5            = t;
  }
  {
    const uint  t = min(r6  ,r10 );
    r10           = max(r6  ,r10 );
    r6            = t;
  }
  {
    const uint  t = min(r7  ,r11 );
    r11           = max(r7  ,r11 );
    r7            = t;
  }
  {
    const uint  t = min(r8  ,r12 );
    r12           = max(r8  ,r12 );
    r8            = t;
  }
  {
    const uint  t = min(r13 ,r17 );
    r17           = max(r13 ,r17 );
    r13           = t;
  }
  {
    const uint  t = min(r14 ,r18 );
    r18           = max(r14 ,r18 );
    r14           = t;
  }
  {
    const uint  t = min(r15 ,r19 );
    r19           = max(r15 ,r19 );
    r15           = t;
  }
  {
    const uint  t = min(r16 ,r20 );
    r20           = max(r16 ,r20 );
    r16           = t;
  }
  {
    const uint  t = min(r21 ,r25 );
    r25           = max(r21 ,r25 );
    r21           = t;
  }
  {
    const uint  t = min(r22 ,r26 );
    r26           = max(r22 ,r26 );
    r22           = t;
  }
  {
    const uint  t = min(r23 ,r27 );
    r27           = max(r23 ,r27 );
    r23           = t;
  }
  {
    const uint  t = min(r24 ,r28 );
    r28           = max(r24 ,r28 );
    r24           = t;
  }
  {
    const uint  t = min(r29 ,r33 );
    r33           = max(r29 ,r33 );
    r29           = t;
  }
  {
    const uint  t = min(r30 ,r34 );
    r34           = max(r30 ,r34 );
    r30           = t;
  }
  {
    const uint  t = min(r31 ,r35 );
    r35           = max(r31 ,r35 );
    r31           = t;
  }
  {
    const uint  t = min(r32 ,r36 );
    r36           = max(r32 ,r36 );
    r32           = t;
  }
  {
    const uint  t = min(r37 ,r41 );
    r41           = max(r37 ,r41 );
    r37           = t;
  }
  {
    const uint  t = min(r38 ,r42 );
    r42           = max(r38 ,r42 );
    r38           = t;
  }
  {
    const uint  t = min(r39 ,r43 );
    r43           = max(r39 ,r43 );
    r39           = t;
  }
  {
    const uint  t = min(r40 ,r44 );
    r44           = max(r40 ,r44 );
    r40           = t;
  }
  {
    const uint  t = min(r45 ,r49 );
    r49           = max(r45 ,r49 );
    r45           = t;
  }
  {
    const uint  t = min(r46 ,r50 );
    r50           = max(r46 ,r50 );
    r46           = t;
  }
  {
    const uint  t = min(r47 ,r51 );
    r51           = max(r47 ,r51 );
    r47           = t;
  }
  {
    const uint  t = min(r48 ,r52 );
    r52           = max(r48 ,r52 );
    r48           = t;
  }
  {
    const uint  t = min(r53 ,r57 );
    r57           = max(r53 ,r57 );
    r53           = t;
  }
  {
    const uint  t = min(r54 ,r58 );
    r58           = max(r54 ,r58 );
    r54           = t;
  }
  {
    const uint  t = min(r55 ,r59 );
    r59           = max(r55 ,r59 );
    r55           = t;
  }
  {
    const uint  t = min(r56 ,r60 );
    r60           = max(r56 ,r60 );
    r56           = t;
  }
  {
    const uint  t = min(r61 ,r65 );
    r65           = max(r61 ,r65 );
    r61           = t;
  }
  {
    const uint  t = min(r62 ,r66 );
    r66           = max(r62 ,r66 );
    r62           = t;
  }
  {
    const uint  t = min(r63 ,r67 );
    r67           = max(r63 ,r67 );
    r63           = t;
  }
  {
    const uint  t = min(r64 ,r68 );
    r68           = max(r64 ,r68 );
    r64           = t;
  }
  {
    const uint  t = min(r69 ,r73 );
    r73           = max(r69 ,r73 );
    r69           = t;
  }
  {
    const uint  t = min(r70 ,r74 );
    r74           = max(r70 ,r74 );
    r70           = t;
  }
  {
    const uint  t = min(r71 ,r75 );
    r75           = max(r71 ,r75 );
    r71           = t;
  }
  {
    const uint  t = min(r72 ,r76 );
    r76           = max(r72 ,r76 );
    r72           = t;
  }
  {
    const uint  t = min(r77 ,r81 );
    r81           = max(r77 ,r81 );
    r77           = t;
  }
  {
    const uint  t = min(r78 ,r82 );
    r82           = max(r78 ,r82 );
    r78           = t;
  }
  {
    const uint  t = min(r79 ,r83 );
    r83           = max(r79 ,r83 );
    r79           = t;
  }
  {
    const uint  t = min(r80 ,r84 );
    r84           = max(r80 ,r84 );
    r80           = t;
  }
  {
    const uint  t = min(r85 ,r89 );
    r89           = max(r85 ,r89 );
    r85           = t;
  }
  {
    const uint  t = min(r86 ,r90 );
    r90           = max(r86 ,r90 );
    r86           = t;
  }
  {
    const uint  t = min(r87 ,r91 );
    r91           = max(r87 ,r91 );
    r87           = t;
  }
  {
    const uint  t = min(r88 ,r92 );
    r92           = max(r88 ,r92 );
    r88           = t;
  }
  {
    const uint  t = min(r93 ,r97 );
    r97           = max(r93 ,r97 );
    r93           = t;
  }
  {
    const uint  t = min(r94 ,r98 );
    r98           = max(r94 ,r98 );
    r94           = t;
  }
  {
    const uint  t = min(r95 ,r99 );
    r99           = max(r95 ,r99 );
    r95           = t;
  }
  {
    const uint  t = min(r96 ,r100);
    r100          = max(r96 ,r100);
    r96           = t;
  }
  {
    const uint  t = min(r1  ,r3  );
    r3            = max(r1  ,r3  );
    r1            = t;
  }
  {
    const uint  t = min(r2  ,r4  );
    r4            = max(r2  ,r4  );
    r2            = t;
  }
  {
    const uint  t = min(r5  ,r7  );
    r7            = max(r5  ,r7  );
    r5            = t;
  }
  {
    const uint  t = min(r6  ,r8  );
    r8            = max(r6  ,r8  );
    r6            = t;
  }
  {
    const uint  t = min(r9  ,r11 );
    r11           = max(r9  ,r11 );
    r9            = t;
  }
  {
    const uint  t = min(r10 ,r12 );
    r12           = max(r10 ,r12 );
    r10           = t;
  }
  {
    const uint  t = min(r13 ,r15 );
    r15           = max(r13 ,r15 );
    r13           = t;
  }
  {
    const uint  t = min(r14 ,r16 );
    r16           = max(r14 ,r16 );
    r14           = t;
  }
  {
    const uint  t = min(r17 ,r19 );
    r19           = max(r17 ,r19 );
    r17           = t;
  }
  {
    const uint  t = min(r18 ,r20 );
    r20           = max(r18 ,r20 );
    r18           = t;
  }
  {
    const uint  t = min(r21 ,r23 );
    r23           = max(r21 ,r23 );
    r21           = t;
  }
  {
    const uint  t = min(r22 ,r24 );
    r24           = max(r22 ,r24 );
    r22           = t;
  }
  {
    const uint  t = min(r25 ,r27 );
    r27           = max(r25 ,r27 );
    r25           = t;
  }
  {
    const uint  t = min(r26 ,r28 );
    r28           = max(r26 ,r28 );
    r26           = t;
  }
  {
    const uint  t = min(r29 ,r31 );
    r31           = max(r29 ,r31 );
    r29           = t;
  }
  {
    const uint  t = min(r30 ,r32 );
    r32           = max(r30 ,r32 );
    r30           = t;
  }
  {
    const uint  t = min(r33 ,r35 );
    r35           = max(r33 ,r35 );
    r33           = t;
  }
  {
    const uint  t = min(r34 ,r36 );
    r36           = max(r34 ,r36 );
    r34           = t;
  }
  {
    const uint  t = min(r37 ,r39 );
    r39           = max(r37 ,r39 );
    r37           = t;
  }
  {
    const uint  t = min(r38 ,r40 );
    r40           = max(r38 ,r40 );
    r38           = t;
  }
  {
    const uint  t = min(r41 ,r43 );
    r43           = max(r41 ,r43 );
    r41           = t;
  }
  {
    const uint  t = min(r42 ,r44 );
    r44           = max(r42 ,r44 );
    r42           = t;
  }
  {
    const uint  t = min(r45 ,r47 );
    r47           = max(r45 ,r47 );
    r45           = t;
  }
  {
    const uint  t = min(r46 ,r48 );
    r48           = max(r46 ,r48 );
    r46           = t;
  }
  {
    const uint  t = min(r49 ,r51 );
    r51           = max(r49 ,r51 );
    r49           = t;
  }
  {
    const uint  t = min(r50 ,r52 );
    r52           = max(r50 ,r52 );
    r50           = t;
  }
  {
    const uint  t = min(r53 ,r55 );
    r55           = max(r53 ,r55 );
    r53           = t;
  }
  {
    const uint  t = min(r54 ,r56 );
    r56           = max(r54 ,r56 );
    r54           = t;
  }
  {
    const uint  t = min(r57 ,r59 );
    r59           = max(r57 ,r59 );
    r57           = t;
  }
  {
    const uint  t = min(r58 ,r60 );
    r60           = max(r58 ,r60 );
    r58           = t;
  }
  {
    const uint  t = min(r61 ,r63 );
    r63           = max(r61 ,r63 );
    r61           = t;
  }
  {
    const uint  t = min(r62 ,r64 );
    r64           = max(r62 ,r64 );
    r62           = t;
  }
  {
    const uint  t = min(r65 ,r67 );
    r67           = max(r65 ,r67 );
    r65           = t;
  }
  {
    const uint  t = min(r66 ,r68 );
    r68           = max(r66 ,r68 );
    r66           = t;
  }
  {
    const uint  t = min(r69 ,r71 );
    r71           = max(r69 ,r71 );
    r69           = t;
  }
  {
    const uint  t = min(r70 ,r72 );
    r72           = max(r70 ,r72 );
    r70           = t;
  }
  {
    const uint  t = min(r73 ,r75 );
    r75           = max(r73 ,r75 );
    r73           = t;
  }
  {
    const uint  t = min(r74 ,r76 );
    r76           = max(r74 ,r76 );
    r74           = t;
  }
  {
    const uint  t = min(r77 ,r79 );
    r79           = max(r77 ,r79 );
    r77           = t;
  }
  {
    const uint  t = min(r78 ,r80 );
    r80           = max(r78 ,r80 );
    r78           = t;
  }
  {
    const uint  t = min(r81 ,r83 );
    r83           = max(r81 ,r83 );
    r81           = t;
  }
  {
    const uint  t = min(r82 ,r84 );
    r84           = max(r82 ,r84 );
    r82           = t;
  }
  {
    const uint  t = min(r85 ,r87 );
    r87           = max(r85 ,r87 );
    r85           = t;
  }
  {
    const uint  t = min(r86 ,r88 );
    r88           = max(r86 ,r88 );
    r86           = t;
  }
  {
    const uint  t = min(r89 ,r91 );
    r91           = max(r89 ,r91 );
    r89           = t;
  }
  {
    const uint  t = min(r90 ,r92 );
    r92           = max(r90 ,r92 );
    r90           = t;
  }
  {
    const uint  t = min(r93 ,r95 );
    r95           = max(r93 ,r95 );
    r93           = t;
  }
  {
    const uint  t = min(r94 ,r96 );
    r96           = max(r94 ,r96 );
    r94           = t;
  }
  {
    const uint  t = min(r97 ,r99 );
    r99           = max(r97 ,r99 );
    r97           = t;
  }
  {
    const uint  t = min(r98 ,r100);
    r100          = max(r98 ,r100);
    r98           = t;
  }
  {
    const uint  t = min(r3  ,r65 );
    r65           = max(r3  ,r65 );
    r3            = t;
  }
  {
    const uint  t = min(r4  ,r66 );
    r66           = max(r4  ,r66 );
    r4            = t;
  }
  {
    const uint  t = min(r7  ,r69 );
    r69           = max(r7  ,r69 );
    r7            = t;
  }
  {
    const uint  t = min(r8  ,r70 );
    r70           = max(r8  ,r70 );
    r8            = t;
  }
  {
    const uint  t = min(r11 ,r73 );
    r73           = max(r11 ,r73 );
    r11           = t;
  }
  {
    const uint  t = min(r12 ,r74 );
    r74           = max(r12 ,r74 );
    r12           = t;
  }
  {
    const uint  t = min(r15 ,r77 );
    r77           = max(r15 ,r77 );
    r15           = t;
  }
  {
    const uint  t = min(r16 ,r78 );
    r78           = max(r16 ,r78 );
    r16           = t;
  }
  {
    const uint  t = min(r19 ,r81 );
    r81           = max(r19 ,r81 );
    r19           = t;
  }
  {
    const uint  t = min(r20 ,r82 );
    r82           = max(r20 ,r82 );
    r20           = t;
  }
  {
    const uint  t = min(r23 ,r85 );
    r85           = max(r23 ,r85 );
    r23           = t;
  }
  {
    const uint  t = min(r24 ,r86 );
    r86           = max(r24 ,r86 );
    r24           = t;
  }
  {
    const uint  t = min(r27 ,r89 );
    r89           = max(r27 ,r89 );
    r27           = t;
  }
  {
    const uint  t = min(r28 ,r90 );
    r90           = max(r28 ,r90 );
    r28           = t;
  }
  {
    const uint  t = min(r31 ,r93 );
    r93           = max(r31 ,r93 );
    r31           = t;
  }
  {
    const uint  t = min(r32 ,r94 );
    r94           = max(r32 ,r94 );
    r32           = t;
  }
  {
    const uint  t = min(r35 ,r97 );
    r97           = max(r35 ,r97 );
    r35           = t;
  }
  {
    const uint  t = min(r36 ,r98 );
    r98           = max(r36 ,r98 );
    r36           = t;
  }
  {
    const uint  t = min(r3  ,r33 );
    r33           = max(r3  ,r33 );
    r3            = t;
  }
  {
    const uint  t = min(r4  ,r34 );
    r34           = max(r4  ,r34 );
    r4            = t;
  }
  {
    const uint  t = min(r7  ,r37 );
    r37           = max(r7  ,r37 );
    r7            = t;
  }
  {
    const uint  t = min(r8  ,r38 );
    r38           = max(r8  ,r38 );
    r8            = t;
  }
  {
    const uint  t = min(r11 ,r41 );
    r41           = max(r11 ,r41 );
    r11           = t;
  }
  {
    const uint  t = min(r12 ,r42 );
    r42           = max(r12 ,r42 );
    r12           = t;
  }
  {
    const uint  t = min(r15 ,r45 );
    r45           = max(r15 ,r45 );
    r15           = t;
  }
  {
    const uint  t = min(r16 ,r46 );
    r46           = max(r16 ,r46 );
    r16           = t;
  }
  {
    const uint  t = min(r19 ,r49 );
    r49           = max(r19 ,r49 );
    r19           = t;
  }
  {
    const uint  t = min(r20 ,r50 );
    r50           = max(r20 ,r50 );
    r20           = t;
  }
  {
    const uint  t = min(r23 ,r53 );
    r53           = max(r23 ,r53 );
    r23           = t;
  }
  {
    const uint  t = min(r24 ,r54 );
    r54           = max(r24 ,r54 );
    r24           = t;
  }
  {
    const uint  t = min(r27 ,r57 );
    r57           = max(r27 ,r57 );
    r27           = t;
  }
  {
    const uint  t = min(r28 ,r58 );
    r58           = max(r28 ,r58 );
    r28           = t;
  }
  {
    const uint  t = min(r31 ,r61 );
    r61           = max(r31 ,r61 );
    r31           = t;
  }
  {
    const uint  t = min(r32 ,r62 );
    r62           = max(r32 ,r62 );
    r32           = t;
  }
  {
    const uint  t = min(r35 ,r65 );
    r65           = max(r35 ,r65 );
    r35           = t;
  }
  {
    const uint  t = min(r36 ,r66 );
    r66           = max(r36 ,r66 );
    r36           = t;
  }
  {
    const uint  t = min(r39 ,r69 );
    r69           = max(r39 ,r69 );
    r39           = t;
  }
  {
    const uint  t = min(r40 ,r70 );
    r70           = max(r40 ,r70 );
    r40           = t;
  }
  {
    const uint  t = min(r43 ,r73 );
    r73           = max(r43 ,r73 );
    r43           = t;
  }
  {
    const uint  t = min(r44 ,r74 );
    r74           = max(r44 ,r74 );
    r44           = t;
  }
  {
    const uint  t = min(r47 ,r77 );
    r77           = max(r47 ,r77 );
    r47           = t;
  }
  {
    const uint  t = min(r48 ,r78 );
    r78           = max(r48 ,r78 );
    r48           = t;
  }
  {
    const uint  t = min(r51 ,r81 );
    r81           = max(r51 ,r81 );
    r51           = t;
  }
  {
    const uint  t = min(r52 ,r82 );
    r82           = max(r52 ,r82 );
    r52           = t;
  }
  {
    const uint  t = min(r55 ,r85 );
    r85           = max(r55 ,r85 );
    r55           = t;
  }
  {
    const uint  t = min(r56 ,r86 );
    r86           = max(r56 ,r86 );
    r56           = t;
  }
  {
    const uint  t = min(r59 ,r89 );
    r89           = max(r59 ,r89 );
    r59           = t;
  }
  {
    const uint  t = min(r60 ,r90 );
    r90           = max(r60 ,r90 );
    r60           = t;
  }
  {
    const uint  t = min(r63 ,r93 );
    r93           = max(r63 ,r93 );
    r63           = t;
  }
  {
    const uint  t = min(r64 ,r94 );
    r94           = max(r64 ,r94 );
    r64           = t;
  }
  {
    const uint  t = min(r67 ,r97 );
    r97           = max(r67 ,r97 );
    r67           = t;
  }
  {
    const uint  t = min(r68 ,r98 );
    r98           = max(r68 ,r98 );
    r68           = t;
  }
  {
    const uint  t = min(r3  ,r17 );
    r17           = max(r3  ,r17 );
    r3            = t;
  }
  {
    const uint  t = min(r4  ,r18 );
    r18           = max(r4  ,r18 );
    r4            = t;
  }
  {
    const uint  t = min(r7  ,r21 );
    r21           = max(r7  ,r21 );
    r7            = t;
  }
  {
    const uint  t = min(r8  ,r22 );
    r22           = max(r8  ,r22 );
    r8            = t;
  }
  {
    const uint  t = min(r11 ,r25 );
    r25           = max(r11 ,r25 );
    r11           = t;
  }
  {
    const uint  t = min(r12 ,r26 );
    r26           = max(r12 ,r26 );
    r12           = t;
  }
  {
    const uint  t = min(r15 ,r29 );
    r29           = max(r15 ,r29 );
    r15           = t;
  }
  {
    const uint  t = min(r16 ,r30 );
    r30           = max(r16 ,r30 );
    r16           = t;
  }
  {
    const uint  t = min(r19 ,r33 );
    r33           = max(r19 ,r33 );
    r19           = t;
  }
  {
    const uint  t = min(r20 ,r34 );
    r34           = max(r20 ,r34 );
    r20           = t;
  }
  {
    const uint  t = min(r23 ,r37 );
    r37           = max(r23 ,r37 );
    r23           = t;
  }
  {
    const uint  t = min(r24 ,r38 );
    r38           = max(r24 ,r38 );
    r24           = t;
  }
  {
    const uint  t = min(r27 ,r41 );
    r41           = max(r27 ,r41 );
    r27           = t;
  }
  {
    const uint  t = min(r28 ,r42 );
    r42           = max(r28 ,r42 );
    r28           = t;
  }
  {
    const uint  t = min(r31 ,r45 );
    r45           = max(r31 ,r45 );
    r31           = t;
  }
  {
    const uint  t = min(r32 ,r46 );
    r46           = max(r32 ,r46 );
    r32           = t;
  }
  {
    const uint  t = min(r35 ,r49 );
    r49           = max(r35 ,r49 );
    r35           = t;
  }
  {
    const uint  t = min(r36 ,r50 );
    r50           = max(r36 ,r50 );
    r36           = t;
  }
  {
    const uint  t = min(r39 ,r53 );
    r53           = max(r39 ,r53 );
    r39           = t;
  }
  {
    const uint  t = min(r40 ,r54 );
    r54           = max(r40 ,r54 );
    r40           = t;
  }
  {
    const uint  t = min(r43 ,r57 );
    r57           = max(r43 ,r57 );
    r43           = t;
  }
  {
    const uint  t = min(r44 ,r58 );
    r58           = max(r44 ,r58 );
    r44           = t;
  }
  {
    const uint  t = min(r47 ,r61 );
    r61           = max(r47 ,r61 );
    r47           = t;
  }
  {
    const uint  t = min(r48 ,r62 );
    r62           = max(r48 ,r62 );
    r48           = t;
  }
  {
    const uint  t = min(r51 ,r65 );
    r65           = max(r51 ,r65 );
    r51           = t;
  }
  {
    const uint  t = min(r52 ,r66 );
    r66           = max(r52 ,r66 );
    r52           = t;
  }
  {
    const uint  t = min(r55 ,r69 );
    r69           = max(r55 ,r69 );
    r55           = t;
  }
  {
    const uint  t = min(r56 ,r70 );
    r70           = max(r56 ,r70 );
    r56           = t;
  }
  {
    const uint  t = min(r59 ,r73 );
    r73           = max(r59 ,r73 );
    r59           = t;
  }
  {
    const uint  t = min(r60 ,r74 );
    r74           = max(r60 ,r74 );
    r60           = t;
  }
  {
    const uint  t = min(r63 ,r77 );
    r77           = max(r63 ,r77 );
    r63           = t;
  }
  {
    const uint  t = min(r64 ,r78 );
    r78           = max(r64 ,r78 );
    r64           = t;
  }
  {
    const uint  t = min(r67 ,r81 );
    r81           = max(r67 ,r81 );
    r67           = t;
  }
  {
    const uint  t = min(r68 ,r82 );
    r82           = max(r68 ,r82 );
    r68           = t;
  }
  {
    const uint  t = min(r71 ,r85 );
    r85           = max(r71 ,r85 );
    r71           = t;
  }
  {
    const uint  t = min(r72 ,r86 );
    r86           = max(r72 ,r86 );
    r72           = t;
  }
  {
    const uint  t = min(r75 ,r89 );
    r89           = max(r75 ,r89 );
    r75           = t;
  }
  {
    const uint  t = min(r76 ,r90 );
    r90           = max(r76 ,r90 );
    r76           = t;
  }
  {
    const uint  t = min(r79 ,r93 );
    r93           = max(r79 ,r93 );
    r79           = t;
  }
  {
    const uint  t = min(r80 ,r94 );
    r94           = max(r80 ,r94 );
    r80           = t;
  }
  {
    const uint  t = min(r83 ,r97 );
    r97           = max(r83 ,r97 );
    r83           = t;
  }
  {
    const uint  t = min(r84 ,r98 );
    r98           = max(r84 ,r98 );
    r84           = t;
  }
  {
    const uint  t = min(r3  ,r9  );
    r9            = max(r3  ,r9  );
    r3            = t;
  }
  {
    const uint  t = min(r4  ,r10 );
    r10           = max(r4  ,r10 );
    r4            = t;
  }
  {
    const uint  t = min(r7  ,r13 );
    r13           = max(r7  ,r13 );
    r7            = t;
  }
  {
    const uint  t = min(r8  ,r14 );
    r14           = max(r8  ,r14 );
    r8            = t;
  }
  {
    const uint  t = min(r11 ,r17 );
    r17           = max(r11 ,r17 );
    r11           = t;
  }
  {
    const uint  t = min(r12 ,r18 );
    r18           = max(r12 ,r18 );
    r12           = t;
  }
  {
    const uint  t = min(r15 ,r21 );
    r21           = max(r15 ,r21 );
    r15           = t;
  }
  {
    const uint  t = min(r16 ,r22 );
    r22           = max(r16 ,r22 );
    r16           = t;
  }
  {
    const uint  t = min(r19 ,r25 );
    r25           = max(r19 ,r25 );
    r19           = t;
  }
  {
    const uint  t = min(r20 ,r26 );
    r26           = max(r20 ,r26 );
    r20           = t;
  }
  {
    const uint  t = min(r23 ,r29 );
    r29           = max(r23 ,r29 );
    r23           = t;
  }
  {
    const uint  t = min(r24 ,r30 );
    r30           = max(r24 ,r30 );
    r24           = t;
  }
  {
    const uint  t = min(r27 ,r33 );
    r33           = max(r27 ,r33 );
    r27           = t;
  }
  {
    const uint  t = min(r28 ,r34 );
    r34           = max(r28 ,r34 );
    r28           = t;
  }
  {
    const uint  t = min(r31 ,r37 );
    r37           = max(r31 ,r37 );
    r31           = t;
  }
  {
    const uint  t = min(r32 ,r38 );
    r38           = max(r32 ,r38 );
    r32           = t;
  }
  {
    const uint  t = min(r35 ,r41 );
    r41           = max(r35 ,r41 );
    r35           = t;
  }
  {
    const uint  t = min(r36 ,r42 );
    r42           = max(r36 ,r42 );
    r36           = t;
  }
  {
    const uint  t = min(r39 ,r45 );
    r45           = max(r39 ,r45 );
    r39           = t;
  }
  {
    const uint  t = min(r40 ,r46 );
    r46           = max(r40 ,r46 );
    r40           = t;
  }
  {
    const uint  t = min(r43 ,r49 );
    r49           = max(r43 ,r49 );
    r43           = t;
  }
  {
    const uint  t = min(r44 ,r50 );
    r50           = max(r44 ,r50 );
    r44           = t;
  }
  {
    const uint  t = min(r47 ,r53 );
    r53           = max(r47 ,r53 );
    r47           = t;
  }
  {
    const uint  t = min(r48 ,r54 );
    r54           = max(r48 ,r54 );
    r48           = t;
  }
  {
    const uint  t = min(r51 ,r57 );
    r57           = max(r51 ,r57 );
    r51           = t;
  }
  {
    const uint  t = min(r52 ,r58 );
    r58           = max(r52 ,r58 );
    r52           = t;
  }
  {
    const uint  t = min(r55 ,r61 );
    r61           = max(r55 ,r61 );
    r55           = t;
  }
  {
    const uint  t = min(r56 ,r62 );
    r62           = max(r56 ,r62 );
    r56           = t;
  }
  {
    const uint  t = min(r59 ,r65 );
    r65           = max(r59 ,r65 );
    r59           = t;
  }
  {
    const uint  t = min(r60 ,r66 );
    r66           = max(r60 ,r66 );
    r60           = t;
  }
  {
    const uint  t = min(r63 ,r69 );
    r69           = max(r63 ,r69 );
    r63           = t;
  }
  {
    const uint  t = min(r64 ,r70 );
    r70           = max(r64 ,r70 );
    r64           = t;
  }
  {
    const uint  t = min(r67 ,r73 );
    r73           = max(r67 ,r73 );
    r67           = t;
  }
  {
    const uint  t = min(r68 ,r74 );
    r74           = max(r68 ,r74 );
    r68           = t;
  }
  {
    const uint  t = min(r71 ,r77 );
    r77           = max(r71 ,r77 );
    r71           = t;
  }
  {
    const uint  t = min(r72 ,r78 );
    r78           = max(r72 ,r78 );
    r72           = t;
  }
  {
    const uint  t = min(r75 ,r81 );
    r81           = max(r75 ,r81 );
    r75           = t;
  }
  {
    const uint  t = min(r76 ,r82 );
    r82           = max(r76 ,r82 );
    r76           = t;
  }
  {
    const uint  t = min(r79 ,r85 );
    r85           = max(r79 ,r85 );
    r79           = t;
  }
  {
    const uint  t = min(r80 ,r86 );
    r86           = max(r80 ,r86 );
    r80           = t;
  }
  {
    const uint  t = min(r83 ,r89 );
    r89           = max(r83 ,r89 );
    r83           = t;
  }
  {
    const uint  t = min(r84 ,r90 );
    r90           = max(r84 ,r90 );
    r84           = t;
  }
  {
    const uint  t = min(r87 ,r93 );
    r93           = max(r87 ,r93 );
    r87           = t;
  }
  {
    const uint  t = min(r88 ,r94 );
    r94           = max(r88 ,r94 );
    r88           = t;
  }
  {
    const uint  t = min(r91 ,r97 );
    r97           = max(r91 ,r97 );
    r91           = t;
  }
  {
    const uint  t = min(r92 ,r98 );
    r98           = max(r92 ,r98 );
    r92           = t;
  }
  {
    const uint  t = min(r3  ,r5  );
    r5            = max(r3  ,r5  );
    r3            = t;
  }
  {
    const uint  t = min(r4  ,r6  );
    r6            = max(r4  ,r6  );
    r4            = t;
  }
  {
    const uint  t = min(r7  ,r9  );
    r9            = max(r7  ,r9  );
    r7            = t;
  }
  {
    const uint  t = min(r8  ,r10 );
    r10           = max(r8  ,r10 );
    r8            = t;
  }
  {
    const uint  t = min(r11 ,r13 );
    r13           = max(r11 ,r13 );
    r11           = t;
  }
  {
    const uint  t = min(r12 ,r14 );
    r14           = max(r12 ,r14 );
    r12           = t;
  }
  {
    const uint  t = min(r15 ,r17 );
    r17           = max(r15 ,r17 );
    r15           = t;
  }
  {
    const uint  t = min(r16 ,r18 );
    r18           = max(r16 ,r18 );
    r16           = t;
  }
  {
    const uint  t = min(r19 ,r21 );
    r21           = max(r19 ,r21 );
    r19           = t;
  }
  {
    const uint  t = min(r20 ,r22 );
    r22           = max(r20 ,r22 );
    r20           = t;
  }
  {
    const uint  t = min(r23 ,r25 );
    r25           = max(r23 ,r25 );
    r23           = t;
  }
  {
    const uint  t = min(r24 ,r26 );
    r26           = max(r24 ,r26 );
    r24           = t;
  }
  {
    const uint  t = min(r27 ,r29 );
    r29           = max(r27 ,r29 );
    r27           = t;
  }
  {
    const uint  t = min(r28 ,r30 );
    r30           = max(r28 ,r30 );
    r28           = t;
  }
  {
    const uint  t = min(r31 ,r33 );
    r33           = max(r31 ,r33 );
    r31           = t;
  }
  {
    const uint  t = min(r32 ,r34 );
    r34           = max(r32 ,r34 );
    r32           = t;
  }
  {
    const uint  t = min(r35 ,r37 );
    r37           = max(r35 ,r37 );
    r35           = t;
  }
  {
    const uint  t = min(r36 ,r38 );
    r38           = max(r36 ,r38 );
    r36           = t;
  }
  {
    const uint  t = min(r39 ,r41 );
    r41           = max(r39 ,r41 );
    r39           = t;
  }
  {
    const uint  t = min(r40 ,r42 );
    r42           = max(r40 ,r42 );
    r40           = t;
  }
  {
    const uint  t = min(r43 ,r45 );
    r45           = max(r43 ,r45 );
    r43           = t;
  }
  {
    const uint  t = min(r44 ,r46 );
    r46           = max(r44 ,r46 );
    r44           = t;
  }
  {
    const uint  t = min(r47 ,r49 );
    r49           = max(r47 ,r49 );
    r47           = t;
  }
  {
    const uint  t = min(r48 ,r50 );
    r50           = max(r48 ,r50 );
    r48           = t;
  }
  {
    const uint  t = min(r51 ,r53 );
    r53           = max(r51 ,r53 );
    r51           = t;
  }
  {
    const uint  t = min(r52 ,r54 );
    r54           = max(r52 ,r54 );
    r52           = t;
  }
  {
    const uint  t = min(r55 ,r57 );
    r57           = max(r55 ,r57 );
    r55           = t;
  }
  {
    const uint  t = min(r56 ,r58 );
    r58           = max(r56 ,r58 );
    r56           = t;
  }
  {
    const uint  t = min(r59 ,r61 );
    r61           = max(r59 ,r61 );
    r59           = t;
  }
  {
    const uint  t = min(r60 ,r62 );
    r62           = max(r60 ,r62 );
    r60           = t;
  }
  {
    const uint  t = min(r63 ,r65 );
    r65           = max(r63 ,r65 );
    r63           = t;
  }
  {
    const uint  t = min(r64 ,r66 );
    r66           = max(r64 ,r66 );
    r64           = t;
  }
  {
    const uint  t = min(r67 ,r69 );
    r69           = max(r67 ,r69 );
    r67           = t;
  }
  {
    const uint  t = min(r68 ,r70 );
    r70           = max(r68 ,r70 );
    r68           = t;
  }
  {
    const uint  t = min(r71 ,r73 );
    r73           = max(r71 ,r73 );
    r71           = t;
  }
  {
    const uint  t = min(r72 ,r74 );
    r74           = max(r72 ,r74 );
    r72           = t;
  }
  {
    const uint  t = min(r75 ,r77 );
    r77           = max(r75 ,r77 );
    r75           = t;
  }
  {
    const uint  t = min(r76 ,r78 );
    r78           = max(r76 ,r78 );
    r76           = t;
  }
  {
    const uint  t = min(r79 ,r81 );
    r81           = max(r79 ,r81 );
    r79           = t;
  }
  {
    const uint  t = min(r80 ,r82 );
    r82           = max(r80 ,r82 );
    r80           = t;
  }
  {
    const uint  t = min(r83 ,r85 );
    r85           = max(r83 ,r85 );
    r83           = t;
  }
  {
    const uint  t = min(r84 ,r86 );
    r86           = max(r84 ,r86 );
    r84           = t;
  }
  {
    const uint  t = min(r87 ,r89 );
    r89           = max(r87 ,r89 );
    r87           = t;
  }
  {
    const uint  t = min(r88 ,r90 );
    r90           = max(r88 ,r90 );
    r88           = t;
  }
  {
    const uint  t = min(r91 ,r93 );
    r93           = max(r91 ,r93 );
    r91           = t;
  }
  {
    const uint  t = min(r92 ,r94 );
    r94           = max(r92 ,r94 );
    r92           = t;
  }
  {
    const uint  t = min(r95 ,r97 );
    r97           = max(r95 ,r97 );
    r95           = t;
  }
  {
    const uint  t = min(r96 ,r98 );
    r98           = max(r96 ,r98 );
    r96           = t;
  }
  {
    const uint  t = min(r1  ,r2  );
    r2            = max(r1  ,r2  );
    r1            = t;
  }
  {
    const uint  t = min(r3  ,r4  );
    r4            = max(r3  ,r4  );
    r3            = t;
  }
  {
    const uint  t = min(r5  ,r6  );
    r6            = max(r5  ,r6  );
    r5            = t;
  }
  {
    const uint  t = min(r7  ,r8  );
    r8            = max(r7  ,r8  );
    r7            = t;
  }
  {
    const uint  t = min(r9  ,r10 );
    r10           = max(r9  ,r10 );
    r9            = t;
  }
  {
    const uint  t = min(r11 ,r12 );
    r12           = max(r11 ,r12 );
    r11           = t;
  }
  {
    const uint  t = min(r13 ,r14 );
    r14           = max(r13 ,r14 );
    r13           = t;
  }
  {
    const uint  t = min(r15 ,r16 );
    r16           = max(r15 ,r16 );
    r15           = t;
  }
  {
    const uint  t = min(r17 ,r18 );
    r18           = max(r17 ,r18 );
    r17           = t;
  }
  {
    const uint  t = min(r19 ,r20 );
    r20           = max(r19 ,r20 );
    r19           = t;
  }
  {
    const uint  t = min(r21 ,r22 );
    r22           = max(r21 ,r22 );
    r21           = t;
  }
  {
    const uint  t = min(r23 ,r24 );
    r24           = max(r23 ,r24 );
    r23           = t;
  }
  {
    const uint  t = min(r25 ,r26 );
    r26           = max(r25 ,r26 );
    r25           = t;
  }
  {
    const uint  t = min(r27 ,r28 );
    r28           = max(r27 ,r28 );
    r27           = t;
  }
  {
    const uint  t = min(r29 ,r30 );
    r30           = max(r29 ,r30 );
    r29           = t;
  }
  {
    const uint  t = min(r31 ,r32 );
    r32           = max(r31 ,r32 );
    r31           = t;
  }
  {
    const uint  t = min(r33 ,r34 );
    r34           = max(r33 ,r34 );
    r33           = t;
  }
  {
    const uint  t = min(r35 ,r36 );
    r36           = max(r35 ,r36 );
    r35           = t;
  }
  {
    const uint  t = min(r37 ,r38 );
    r38           = max(r37 ,r38 );
    r37           = t;
  }
  {
    const uint  t = min(r39 ,r40 );
    r40           = max(r39 ,r40 );
    r39           = t;
  }
  {
    const uint  t = min(r41 ,r42 );
    r42           = max(r41 ,r42 );
    r41           = t;
  }
  {
    const uint  t = min(r43 ,r44 );
    r44           = max(r43 ,r44 );
    r43           = t;
  }
  {
    const uint  t = min(r45 ,r46 );
    r46           = max(r45 ,r46 );
    r45           = t;
  }
  {
    const uint  t = min(r47 ,r48 );
    r48           = max(r47 ,r48 );
    r47           = t;
  }
  {
    const uint  t = min(r49 ,r50 );
    r50           = max(r49 ,r50 );
    r49           = t;
  }
  {
    const uint  t = min(r51 ,r52 );
    r52           = max(r51 ,r52 );
    r51           = t;
  }
  {
    const uint  t = min(r53 ,r54 );
    r54           = max(r53 ,r54 );
    r53           = t;
  }
  {
    const uint  t = min(r55 ,r56 );
    r56           = max(r55 ,r56 );
    r55           = t;
  }
  {
    const uint  t = min(r57 ,r58 );
    r58           = max(r57 ,r58 );
    r57           = t;
  }
  {
    const uint  t = min(r59 ,r60 );
    r60           = max(r59 ,r60 );
    r59           = t;
  }
  {
    const uint  t = min(r61 ,r62 );
    r62           = max(r61 ,r62 );
    r61           = t;
  }
  {
    const uint  t = min(r63 ,r64 );
    r64           = max(r63 ,r64 );
    r63           = t;
  }
  {
    const uint  t = min(r65 ,r66 );
    r66           = max(r65 ,r66 );
    r65           = t;
  }
  {
    const uint  t = min(r67 ,r68 );
    r68           = max(r67 ,r68 );
    r67           = t;
  }
  {
    const uint  t = min(r69 ,r70 );
    r70           = max(r69 ,r70 );
    r69           = t;
  }
  {
    const uint  t = min(r71 ,r72 );
    r72           = max(r71 ,r72 );
    r71           = t;
  }
  {
    const uint  t = min(r73 ,r74 );
    r74           = max(r73 ,r74 );
    r73           = t;
  }
  {
    const uint  t = min(r75 ,r76 );
    r76           = max(r75 ,r76 );
    r75           = t;
  }
  {
    const uint  t = min(r77 ,r78 );
    r78           = max(r77 ,r78 );
    r77           = t;
  }
  {
    const uint  t = min(r79 ,r80 );
    r80           = max(r79 ,r80 );
    r79           = t;
  }
  {
    const uint  t = min(r81 ,r82 );
    r82           = max(r81 ,r82 );
    r81           = t;
  }
  {
    const uint  t = min(r83 ,r84 );
    r84           = max(r83 ,r84 );
    r83           = t;
  }
  {
    const uint  t = min(r85 ,r86 );
    r86           = max(r85 ,r86 );
    r85           = t;
  }
  {
    const uint  t = min(r87 ,r88 );
    r88           = max(r87 ,r88 );
    r87           = t;
  }
  {
    const uint  t = min(r89 ,r90 );
    r90           = max(r89 ,r90 );
    r89           = t;
  }
  {
    const uint  t = min(r91 ,r92 );
    r92           = max(r91 ,r92 );
    r91           = t;
  }
  {
    const uint  t = min(r93 ,r94 );
    r94           = max(r93 ,r94 );
    r93           = t;
  }
  {
    const uint  t = min(r95 ,r96 );
    r96           = max(r95 ,r96 );
    r95           = t;
  }
  {
    const uint  t = min(r97 ,r98 );
    r98           = max(r97 ,r98 );
    r97           = t;
  }
  {
    const uint  t = min(r99 ,r100);
    r100          = max(r99 ,r100);
    r99           = t;
  }
  {
    const uint  t = min(r2  ,r65 );
    r65           = max(r2  ,r65 );
    r2            = t;
  }
  {
    const uint  t = min(r4  ,r67 );
    r67           = max(r4  ,r67 );
    r4            = t;
  }
  {
    const uint  t = min(r6  ,r69 );
    r69           = max(r6  ,r69 );
    r6            = t;
  }
  {
    const uint  t = min(r8  ,r71 );
    r71           = max(r8  ,r71 );
    r8            = t;
  }
  {
    const uint  t = min(r10 ,r73 );
    r73           = max(r10 ,r73 );
    r10           = t;
  }
  {
    const uint  t = min(r12 ,r75 );
    r75           = max(r12 ,r75 );
    r12           = t;
  }
  {
    const uint  t = min(r14 ,r77 );
    r77           = max(r14 ,r77 );
    r14           = t;
  }
  {
    const uint  t = min(r16 ,r79 );
    r79           = max(r16 ,r79 );
    r16           = t;
  }
  {
    const uint  t = min(r18 ,r81 );
    r81           = max(r18 ,r81 );
    r18           = t;
  }
  {
    const uint  t = min(r20 ,r83 );
    r83           = max(r20 ,r83 );
    r20           = t;
  }
  {
    const uint  t = min(r22 ,r85 );
    r85           = max(r22 ,r85 );
    r22           = t;
  }
  {
    const uint  t = min(r24 ,r87 );
    r87           = max(r24 ,r87 );
    r24           = t;
  }
  {
    const uint  t = min(r26 ,r89 );
    r89           = max(r26 ,r89 );
    r26           = t;
  }
  {
    const uint  t = min(r28 ,r91 );
    r91           = max(r28 ,r91 );
    r28           = t;
  }
  {
    const uint  t = min(r30 ,r93 );
    r93           = max(r30 ,r93 );
    r30           = t;
  }
  {
    const uint  t = min(r32 ,r95 );
    r95           = max(r32 ,r95 );
    r32           = t;
  }
  {
    const uint  t = min(r34 ,r97 );
    r97           = max(r34 ,r97 );
    r34           = t;
  }
  {
    const uint  t = min(r36 ,r99 );
    r99           = max(r36 ,r99 );
    r36           = t;
  }
  {
    const uint  t = min(r2  ,r33 );
    r33           = max(r2  ,r33 );
    r2            = t;
  }
  {
    const uint  t = min(r4  ,r35 );
    r35           = max(r4  ,r35 );
    r4            = t;
  }
  {
    const uint  t = min(r6  ,r37 );
    r37           = max(r6  ,r37 );
    r6            = t;
  }
  {
    const uint  t = min(r8  ,r39 );
    r39           = max(r8  ,r39 );
    r8            = t;
  }
  {
    const uint  t = min(r10 ,r41 );
    r41           = max(r10 ,r41 );
    r10           = t;
  }
  {
    const uint  t = min(r12 ,r43 );
    r43           = max(r12 ,r43 );
    r12           = t;
  }
  {
    const uint  t = min(r14 ,r45 );
    r45           = max(r14 ,r45 );
    r14           = t;
  }
  {
    const uint  t = min(r16 ,r47 );
    r47           = max(r16 ,r47 );
    r16           = t;
  }
  {
    const uint  t = min(r18 ,r49 );
    r49           = max(r18 ,r49 );
    r18           = t;
  }
  {
    const uint  t = min(r20 ,r51 );
    r51           = max(r20 ,r51 );
    r20           = t;
  }
  {
    const uint  t = min(r22 ,r53 );
    r53           = max(r22 ,r53 );
    r22           = t;
  }
  {
    const uint  t = min(r24 ,r55 );
    r55           = max(r24 ,r55 );
    r24           = t;
  }
  {
    const uint  t = min(r26 ,r57 );
    r57           = max(r26 ,r57 );
    r26           = t;
  }
  {
    const uint  t = min(r28 ,r59 );
    r59           = max(r28 ,r59 );
    r28           = t;
  }
  {
    const uint  t = min(r30 ,r61 );
    r61           = max(r30 ,r61 );
    r30           = t;
  }
  {
    const uint  t = min(r32 ,r63 );
    r63           = max(r32 ,r63 );
    r32           = t;
  }
  {
    const uint  t = min(r34 ,r65 );
    r65           = max(r34 ,r65 );
    r34           = t;
  }
  {
    const uint  t = min(r36 ,r67 );
    r67           = max(r36 ,r67 );
    r36           = t;
  }
  {
    const uint  t = min(r38 ,r69 );
    r69           = max(r38 ,r69 );
    r38           = t;
  }
  {
    const uint  t = min(r40 ,r71 );
    r71           = max(r40 ,r71 );
    r40           = t;
  }
  {
    const uint  t = min(r42 ,r73 );
    r73           = max(r42 ,r73 );
    r42           = t;
  }
  {
    const uint  t = min(r44 ,r75 );
    r75           = max(r44 ,r75 );
    r44           = t;
  }
  {
    const uint  t = min(r46 ,r77 );
    r77           = max(r46 ,r77 );
    r46           = t;
  }
  {
    const uint  t = min(r48 ,r79 );
    r79           = max(r48 ,r79 );
    r48           = t;
  }
  {
    const uint  t = min(r50 ,r81 );
    r81           = max(r50 ,r81 );
    r50           = t;
  }
  {
    const uint  t = min(r52 ,r83 );
    r83           = max(r52 ,r83 );
    r52           = t;
  }
  {
    const uint  t = min(r54 ,r85 );
    r85           = max(r54 ,r85 );
    r54           = t;
  }
  {
    const uint  t = min(r56 ,r87 );
    r87           = max(r56 ,r87 );
    r56           = t;
  }
  {
    const uint  t = min(r58 ,r89 );
    r89           = max(r58 ,r89 );
    r58           = t;
  }
  {
    const uint  t = min(r60 ,r91 );
    r91           = max(r60 ,r91 );
    r60           = t;
  }
  {
    const uint  t = min(r62 ,r93 );
    r93           = max(r62 ,r93 );
    r62           = t;
  }
  {
    const uint  t = min(r64 ,r95 );
    r95           = max(r64 ,r95 );
    r64           = t;
  }
  {
    const uint  t = min(r66 ,r97 );
    r97           = max(r66 ,r97 );
    r66           = t;
  }
  {
    const uint  t = min(r68 ,r99 );
    r99           = max(r68 ,r99 );
    r68           = t;
  }
  {
    const uint  t = min(r2  ,r17 );
    r17           = max(r2  ,r17 );
    r2            = t;
  }
  {
    const uint  t = min(r4  ,r19 );
    r19           = max(r4  ,r19 );
    r4            = t;
  }
  {
    const uint  t = min(r6  ,r21 );
    r21           = max(r6  ,r21 );
    r6            = t;
  }
  {
    const uint  t = min(r8  ,r23 );
    r23           = max(r8  ,r23 );
    r8            = t;
  }
  {
    const uint  t = min(r10 ,r25 );
    r25           = max(r10 ,r25 );
    r10           = t;
  }
  {
    const uint  t = min(r12 ,r27 );
    r27           = max(r12 ,r27 );
    r12           = t;
  }
  {
    const uint  t = min(r14 ,r29 );
    r29           = max(r14 ,r29 );
    r14           = t;
  }
  {
    const uint  t = min(r16 ,r31 );
    r31           = max(r16 ,r31 );
    r16           = t;
  }
  {
    const uint  t = min(r18 ,r33 );
    r33           = max(r18 ,r33 );
    r18           = t;
  }
  {
    const uint  t = min(r20 ,r35 );
    r35           = max(r20 ,r35 );
    r20           = t;
  }
  {
    const uint  t = min(r22 ,r37 );
    r37           = max(r22 ,r37 );
    r22           = t;
  }
  {
    const uint  t = min(r24 ,r39 );
    r39           = max(r24 ,r39 );
    r24           = t;
  }
  {
    const uint  t = min(r26 ,r41 );
    r41           = max(r26 ,r41 );
    r26           = t;
  }
  {
    const uint  t = min(r28 ,r43 );
    r43           = max(r28 ,r43 );
    r28           = t;
  }
  {
    const uint  t = min(r30 ,r45 );
    r45           = max(r30 ,r45 );
    r30           = t;
  }
  {
    const uint  t = min(r32 ,r47 );
    r47           = max(r32 ,r47 );
    r32           = t;
  }
  {
    const uint  t = min(r34 ,r49 );
    r49           = max(r34 ,r49 );
    r34           = t;
  }
  {
    const uint  t = min(r36 ,r51 );
    r51           = max(r36 ,r51 );
    r36           = t;
  }
  {
    const uint  t = min(r38 ,r53 );
    r53           = max(r38 ,r53 );
    r38           = t;
  }
  {
    const uint  t = min(r40 ,r55 );
    r55           = max(r40 ,r55 );
    r40           = t;
  }
  {
    const uint  t = min(r42 ,r57 );
    r57           = max(r42 ,r57 );
    r42           = t;
  }
  {
    const uint  t = min(r44 ,r59 );
    r59           = max(r44 ,r59 );
    r44           = t;
  }
  {
    const uint  t = min(r46 ,r61 );
    r61           = max(r46 ,r61 );
    r46           = t;
  }
  {
    const uint  t = min(r48 ,r63 );
    r63           = max(r48 ,r63 );
    r48           = t;
  }
  {
    const uint  t = min(r50 ,r65 );
    r65           = max(r50 ,r65 );
    r50           = t;
  }
  {
    const uint  t = min(r52 ,r67 );
    r67           = max(r52 ,r67 );
    r52           = t;
  }
  {
    const uint  t = min(r54 ,r69 );
    r69           = max(r54 ,r69 );
    r54           = t;
  }
  {
    const uint  t = min(r56 ,r71 );
    r71           = max(r56 ,r71 );
    r56           = t;
  }
  {
    const uint  t = min(r58 ,r73 );
    r73           = max(r58 ,r73 );
    r58           = t;
  }
  {
    const uint  t = min(r60 ,r75 );
    r75           = max(r60 ,r75 );
    r60           = t;
  }
  {
    const uint  t = min(r62 ,r77 );
    r77           = max(r62 ,r77 );
    r62           = t;
  }
  {
    const uint  t = min(r64 ,r79 );
    r79           = max(r64 ,r79 );
    r64           = t;
  }
  {
    const uint  t = min(r66 ,r81 );
    r81           = max(r66 ,r81 );
    r66           = t;
  }
  {
    const uint  t = min(r68 ,r83 );
    r83           = max(r68 ,r83 );
    r68           = t;
  }
  {
    const uint  t = min(r70 ,r85 );
    r85           = max(r70 ,r85 );
    r70           = t;
  }
  {
    const uint  t = min(r72 ,r87 );
    r87           = max(r72 ,r87 );
    r72           = t;
  }
  {
    const uint  t = min(r74 ,r89 );
    r89           = max(r74 ,r89 );
    r74           = t;
  }
  {
    const uint  t = min(r76 ,r91 );
    r91           = max(r76 ,r91 );
    r76           = t;
  }
  {
    const uint  t = min(r78 ,r93 );
    r93           = max(r78 ,r93 );
    r78           = t;
  }
  {
    const uint  t = min(r80 ,r95 );
    r95           = max(r80 ,r95 );
    r80           = t;
  }
  {
    const uint  t = min(r82 ,r97 );
    r97           = max(r82 ,r97 );
    r82           = t;
  }
  {
    const uint  t = min(r84 ,r99 );
    r99           = max(r84 ,r99 );
    r84           = t;
  }
  {
    const uint  t = min(r2  ,r9  );
    r9            = max(r2  ,r9  );
    r2            = t;
  }
  {
    const uint  t = min(r4  ,r11 );
    r11           = max(r4  ,r11 );
    r4            = t;
  }
  {
    const uint  t = min(r6  ,r13 );
    r13           = max(r6  ,r13 );
    r6            = t;
  }
  {
    const uint  t = min(r8  ,r15 );
    r15           = max(r8  ,r15 );
    r8            = t;
  }
  {
    const uint  t = min(r10 ,r17 );
    r17           = max(r10 ,r17 );
    r10           = t;
  }
  {
    const uint  t = min(r12 ,r19 );
    r19           = max(r12 ,r19 );
    r12           = t;
  }
  {
    const uint  t = min(r14 ,r21 );
    r21           = max(r14 ,r21 );
    r14           = t;
  }
  {
    const uint  t = min(r16 ,r23 );
    r23           = max(r16 ,r23 );
    r16           = t;
  }
  {
    const uint  t = min(r18 ,r25 );
    r25           = max(r18 ,r25 );
    r18           = t;
  }
  {
    const uint  t = min(r20 ,r27 );
    r27           = max(r20 ,r27 );
    r20           = t;
  }
  {
    const uint  t = min(r22 ,r29 );
    r29           = max(r22 ,r29 );
    r22           = t;
  }
  {
    const uint  t = min(r24 ,r31 );
    r31           = max(r24 ,r31 );
    r24           = t;
  }
  {
    const uint  t = min(r26 ,r33 );
    r33           = max(r26 ,r33 );
    r26           = t;
  }
  {
    const uint  t = min(r28 ,r35 );
    r35           = max(r28 ,r35 );
    r28           = t;
  }
  {
    const uint  t = min(r30 ,r37 );
    r37           = max(r30 ,r37 );
    r30           = t;
  }
  {
    const uint  t = min(r32 ,r39 );
    r39           = max(r32 ,r39 );
    r32           = t;
  }
  {
    const uint  t = min(r34 ,r41 );
    r41           = max(r34 ,r41 );
    r34           = t;
  }
  {
    const uint  t = min(r36 ,r43 );
    r43           = max(r36 ,r43 );
    r36           = t;
  }
  {
    const uint  t = min(r38 ,r45 );
    r45           = max(r38 ,r45 );
    r38           = t;
  }
  {
    const uint  t = min(r40 ,r47 );
    r47           = max(r40 ,r47 );
    r40           = t;
  }
  {
    const uint  t = min(r42 ,r49 );
    r49           = max(r42 ,r49 );
    r42           = t;
  }
  {
    const uint  t = min(r44 ,r51 );
    r51           = max(r44 ,r51 );
    r44           = t;
  }
  {
    const uint  t = min(r46 ,r53 );
    r53           = max(r46 ,r53 );
    r46           = t;
  }
  {
    const uint  t = min(r48 ,r55 );
    r55           = max(r48 ,r55 );
    r48           = t;
  }
  {
    const uint  t = min(r50 ,r57 );
    r57           = max(r50 ,r57 );
    r50           = t;
  }
  {
    const uint  t = min(r52 ,r59 );
    r59           = max(r52 ,r59 );
    r52           = t;
  }
  {
    const uint  t = min(r54 ,r61 );
    r61           = max(r54 ,r61 );
    r54           = t;
  }
  {
    const uint  t = min(r56 ,r63 );
    r63           = max(r56 ,r63 );
    r56           = t;
  }
  {
    const uint  t = min(r58 ,r65 );
    r65           = max(r58 ,r65 );
    r58           = t;
  }
  {
    const uint  t = min(r60 ,r67 );
    r67           = max(r60 ,r67 );
    r60           = t;
  }
  {
    const uint  t = min(r62 ,r69 );
    r69           = max(r62 ,r69 );
    r62           = t;
  }
  {
    const uint  t = min(r64 ,r71 );
    r71           = max(r64 ,r71 );
    r64           = t;
  }
  {
    const uint  t = min(r66 ,r73 );
    r73           = max(r66 ,r73 );
    r66           = t;
  }
  {
    const uint  t = min(r68 ,r75 );
    r75           = max(r68 ,r75 );
    r68           = t;
  }
  {
    const uint  t = min(r70 ,r77 );
    r77           = max(r70 ,r77 );
    r70           = t;
  }
  {
    const uint  t = min(r72 ,r79 );
    r79           = max(r72 ,r79 );
    r72           = t;
  }
  {
    const uint  t = min(r74 ,r81 );
    r81           = max(r74 ,r81 );
    r74           = t;
  }
  {
    const uint  t = min(r76 ,r83 );
    r83           = max(r76 ,r83 );
    r76           = t;
  }
  {
    const uint  t = min(r78 ,r85 );
    r85           = max(r78 ,r85 );
    r78           = t;
  }
  {
    const uint  t = min(r80 ,r87 );
    r87           = max(r80 ,r87 );
    r80           = t;
  }
  {
    const uint  t = min(r82 ,r89 );
    r89           = max(r82 ,r89 );
    r82           = t;
  }
  {
    const uint  t = min(r84 ,r91 );
    r91           = max(r84 ,r91 );
    r84           = t;
  }
  {
    const uint  t = min(r86 ,r93 );
    r93           = max(r86 ,r93 );
    r86           = t;
  }
  {
    const uint  t = min(r88 ,r95 );
    r95           = max(r88 ,r95 );
    r88           = t;
  }
  {
    const uint  t = min(r90 ,r97 );
    r97           = max(r90 ,r97 );
    r90           = t;
  }
  {
    const uint  t = min(r92 ,r99 );
    r99           = max(r92 ,r99 );
    r92           = t;
  }
  {
    const uint  t = min(r2  ,r5  );
    r5            = max(r2  ,r5  );
    r2            = t;
  }
  {
    const uint  t = min(r4  ,r7  );
    r7            = max(r4  ,r7  );
    r4            = t;
  }
  {
    const uint  t = min(r6  ,r9  );
    r9            = max(r6  ,r9  );
    r6            = t;
  }
  {
    const uint  t = min(r8  ,r11 );
    r11           = max(r8  ,r11 );
    r8            = t;
  }
  {
    const uint  t = min(r10 ,r13 );
    r13           = max(r10 ,r13 );
    r10           = t;
  }
  {
    const uint  t = min(r12 ,r15 );
    r15           = max(r12 ,r15 );
    r12           = t;
  }
  {
    const uint  t = min(r14 ,r17 );
    r17           = max(r14 ,r17 );
    r14           = t;
  }
  {
    const uint  t = min(r16 ,r19 );
    r19           = max(r16 ,r19 );
    r16           = t;
  }
  {
    const uint  t = min(r18 ,r21 );
    r21           = max(r18 ,r21 );
    r18           = t;
  }
  {
    const uint  t = min(r20 ,r23 );
    r23           = max(r20 ,r23 );
    r20           = t;
  }
  {
    const uint  t = min(r22 ,r25 );
    r25           = max(r22 ,r25 );
    r22           = t;
  }
  {
    const uint  t = min(r24 ,r27 );
    r27           = max(r24 ,r27 );
    r24           = t;
  }
  {
    const uint  t = min(r26 ,r29 );
    r29           = max(r26 ,r29 );
    r26           = t;
  }
  {
    const uint  t = min(r28 ,r31 );
    r31           = max(r28 ,r31 );
    r28           = t;
  }
  {
    const uint  t = min(r30 ,r33 );
    r33           = max(r30 ,r33 );
    r30           = t;
  }
  {
    const uint  t = min(r32 ,r35 );
    r35           = max(r32 ,r35 );
    r32           = t;
  }
  {
    const uint  t = min(r34 ,r37 );
    r37           = max(r34 ,r37 );
    r34           = t;
  }
  {
    const uint  t = min(r36 ,r39 );
    r39           = max(r36 ,r39 );
    r36           = t;
  }
  {
    const uint  t = min(r38 ,r41 );
    r41           = max(r38 ,r41 );
    r38           = t;
  }
  {
    const uint  t = min(r40 ,r43 );
    r43           = max(r40 ,r43 );
    r40           = t;
  }
  {
    const uint  t = min(r42 ,r45 );
    r45           = max(r42 ,r45 );
    r42           = t;
  }
  {
    const uint  t = min(r44 ,r47 );
    r47           = max(r44 ,r47 );
    r44           = t;
  }
  {
    const uint  t = min(r46 ,r49 );
    r49           = max(r46 ,r49 );
    r46           = t;
  }
  {
    const uint  t = min(r48 ,r51 );
    r51           = max(r48 ,r51 );
    r48           = t;
  }
  {
    const uint  t = min(r50 ,r53 );
    r53           = max(r50 ,r53 );
    r50           = t;
  }
  {
    const uint  t = min(r52 ,r55 );
    r55           = max(r52 ,r55 );
    r52           = t;
  }
  {
    const uint  t = min(r54 ,r57 );
    r57           = max(r54 ,r57 );
    r54           = t;
  }
  {
    const uint  t = min(r56 ,r59 );
    r59           = max(r56 ,r59 );
    r56           = t;
  }
  {
    const uint  t = min(r58 ,r61 );
    r61           = max(r58 ,r61 );
    r58           = t;
  }
  {
    const uint  t = min(r60 ,r63 );
    r63           = max(r60 ,r63 );
    r60           = t;
  }
  {
    const uint  t = min(r62 ,r65 );
    r65           = max(r62 ,r65 );
    r62           = t;
  }
  {
    const uint  t = min(r64 ,r67 );
    r67           = max(r64 ,r67 );
    r64           = t;
  }
  {
    const uint  t = min(r66 ,r69 );
    r69           = max(r66 ,r69 );
    r66           = t;
  }
  {
    const uint  t = min(r68 ,r71 );
    r71           = max(r68 ,r71 );
    r68           = t;
  }
  {
    const uint  t = min(r70 ,r73 );
    r73           = max(r70 ,r73 );
    r70           = t;
  }
  {
    const uint  t = min(r72 ,r75 );
    r75           = max(r72 ,r75 );
    r72           = t;
  }
  {
    const uint  t = min(r74 ,r77 );
    r77           = max(r74 ,r77 );
    r74           = t;
  }
  {
    const uint  t = min(r76 ,r79 );
    r79           = max(r76 ,r79 );
    r76           = t;
  }
  {
    const uint  t = min(r78 ,r81 );
    r81           = max(r78 ,r81 );
    r78           = t;
  }
  {
    const uint  t = min(r80 ,r83 );
    r83           = max(r80 ,r83 );
    r80           = t;
  }
  {
    const uint  t = min(r82 ,r85 );
    r85           = max(r82 ,r85 );
    r82           = t;
  }
  {
    const uint  t = min(r84 ,r87 );
    r87           = max(r84 ,r87 );
    r84           = t;
  }
  {
    const uint  t = min(r86 ,r89 );
    r89           = max(r86 ,r89 );
    r86           = t;
  }
  {
    const uint  t = min(r88 ,r91 );
    r91           = max(r88 ,r91 );
    r88           = t;
  }
  {
    const uint  t = min(r90 ,r93 );
    r93           = max(r90 ,r93 );
    r90           = t;
  }
  {
    const uint  t = min(r92 ,r95 );
    r95           = max(r92 ,r95 );
    r92           = t;
  }
  {
    const uint  t = min(r94 ,r97 );
    r97           = max(r94 ,r97 );
    r94           = t;
  }
  {
    const uint  t = min(r96 ,r99 );
    r99           = max(r96 ,r99 );
    r96           = t;
  }
  {
    const uint  t = min(r2  ,r3  );
    r3            = max(r2  ,r3  );
    r2            = t;
  }
  {
    const uint  t = min(r4  ,r5  );
    r5            = max(r4  ,r5  );
    r4            = t;
  }
  {
    const uint  t = min(r6  ,r7  );
    r7            = max(r6  ,r7  );
    r6            = t;
  }
  {
    const uint  t = min(r8  ,r9  );
    r9            = max(r8  ,r9  );
    r8            = t;
  }
  {
    const uint  t = min(r10 ,r11 );
    r11           = max(r10 ,r11 );
    r10           = t;
  }
  {
    const uint  t = min(r12 ,r13 );
    r13           = max(r12 ,r13 );
    r12           = t;
  }
  {
    const uint  t = min(r14 ,r15 );
    r15           = max(r14 ,r15 );
    r14           = t;
  }
  {
    const uint  t = min(r16 ,r17 );
    r17           = max(r16 ,r17 );
    r16           = t;
  }
  {
    const uint  t = min(r18 ,r19 );
    r19           = max(r18 ,r19 );
    r18           = t;
  }
  {
    const uint  t = min(r20 ,r21 );
    r21           = max(r20 ,r21 );
    r20           = t;
  }
  {
    const uint  t = min(r22 ,r23 );
    r23           = max(r22 ,r23 );
    r22           = t;
  }
  {
    const uint  t = min(r24 ,r25 );
    r25           = max(r24 ,r25 );
    r24           = t;
  }
  {
    const uint  t = min(r26 ,r27 );
    r27           = max(r26 ,r27 );
    r26           = t;
  }
  {
    const uint  t = min(r28 ,r29 );
    r29           = max(r28 ,r29 );
    r28           = t;
  }
  {
    const uint  t = min(r30 ,r31 );
    r31           = max(r30 ,r31 );
    r30           = t;
  }
  {
    const uint  t = min(r32 ,r33 );
    r33           = max(r32 ,r33 );
    r32           = t;
  }
  {
    const uint  t = min(r34 ,r35 );
    r35           = max(r34 ,r35 );
    r34           = t;
  }
  {
    const uint  t = min(r36 ,r37 );
    r37           = max(r36 ,r37 );
    r36           = t;
  }
  {
    const uint  t = min(r38 ,r39 );
    r39           = max(r38 ,r39 );
    r38           = t;
  }
  {
    const uint  t = min(r40 ,r41 );
    r41           = max(r40 ,r41 );
    r40           = t;
  }
  {
    const uint  t = min(r42 ,r43 );
    r43           = max(r42 ,r43 );
    r42           = t;
  }
  {
    const uint  t = min(r44 ,r45 );
    r45           = max(r44 ,r45 );
    r44           = t;
  }
  {
    const uint  t = min(r46 ,r47 );
    r47           = max(r46 ,r47 );
    r46           = t;
  }
  {
    const uint  t = min(r48 ,r49 );
    r49           = max(r48 ,r49 );
    r48           = t;
  }
  {
    const uint  t = min(r50 ,r51 );
    r51           = max(r50 ,r51 );
    r50           = t;
  }
  {
    const uint  t = min(r52 ,r53 );
    r53           = max(r52 ,r53 );
    r52           = t;
  }
  {
    const uint  t = min(r54 ,r55 );
    r55           = max(r54 ,r55 );
    r54           = t;
  }
  {
    const uint  t = min(r56 ,r57 );
    r57           = max(r56 ,r57 );
    r56           = t;
  }
  {
    const uint  t = min(r58 ,r59 );
    r59           = max(r58 ,r59 );
    r58           = t;
  }
  {
    const uint  t = min(r60 ,r61 );
    r61           = max(r60 ,r61 );
    r60           = t;
  }
  {
    const uint  t = min(r62 ,r63 );
    r63           = max(r62 ,r63 );
    r62           = t;
  }
  {
    const uint  t = min(r64 ,r65 );
    r65           = max(r64 ,r65 );
    r64           = t;
  }
  {
    const uint  t = min(r66 ,r67 );
    r67           = max(r66 ,r67 );
    r66           = t;
  }
  {
    const uint  t = min(r68 ,r69 );
    r69           = max(r68 ,r69 );
    r68           = t;
  }
  {
    const uint  t = min(r70 ,r71 );
    r71           = max(r70 ,r71 );
    r70           = t;
  }
  {
    const uint  t = min(r72 ,r73 );
    r73           = max(r72 ,r73 );
    r72           = t;
  }
  {
    const uint  t = min(r74 ,r75 );
    r75           = max(r74 ,r75 );
    r74           = t;
  }
  {
    const uint  t = min(r76 ,r77 );
    r77           = max(r76 ,r77 );
    r76           = t;
  }
  {
    const uint  t = min(r78 ,r79 );
    r79           = max(r78 ,r79 );
    r78           = t;
  }
  {
    const uint  t = min(r80 ,r81 );
    r81           = max(r80 ,r81 );
    r80           = t;
  }
  {
    const uint  t = min(r82 ,r83 );
    r83           = max(r82 ,r83 );
    r82           = t;
  }
  {
    const uint  t = min(r84 ,r85 );
    r85           = max(r84 ,r85 );
    r84           = t;
  }
  {
    const uint  t = min(r86 ,r87 );
    r87           = max(r86 ,r87 );
    r86           = t;
  }
  {
    const uint  t = min(r88 ,r89 );
    r89           = max(r88 ,r89 );
    r88           = t;
  }
  {
    const uint  t = min(r90 ,r91 );
    r91           = max(r90 ,r91 );
    r90           = t;
  }
  {
    const uint  t = min(r92 ,r93 );
    r93           = max(r92 ,r93 );
    r92           = t;
  }
  {
    const uint  t = min(r94 ,r95 );
    r95           = max(r94 ,r95 );
    r94           = t;
  }
  {
    const uint  t = min(r96 ,r97 );
    r97           = max(r96 ,r97 );
    r96           = t;
  }
  {
    const uint  t = min(r98 ,r99 );
    r99           = max(r98 ,r99 );
    r98           = t;
  }
  {
    const uint simd_id      = (get_local_size(0) * get_group_id(0) + get_local_id(0)) / 8;
    const uint simd_lane_id = get_local_id(0) & 7;
    const uint simd_st_idx  = simd_id * 800 + simd_lane_id;
    out[simd_st_idx + 0   * 8] = r1;
    out[simd_st_idx + 1   * 8] = r2;
    out[simd_st_idx + 2   * 8] = r3;
    out[simd_st_idx + 3   * 8] = r4;
    out[simd_st_idx + 4   * 8] = r5;
    out[simd_st_idx + 5   * 8] = r6;
    out[simd_st_idx + 6   * 8] = r7;
    out[simd_st_idx + 7   * 8] = r8;
    out[simd_st_idx + 8   * 8] = r9;
    out[simd_st_idx + 9   * 8] = r10;
    out[simd_st_idx + 10  * 8] = r11;
    out[simd_st_idx + 11  * 8] = r12;
    out[simd_st_idx + 12  * 8] = r13;
    out[simd_st_idx + 13  * 8] = r14;
    out[simd_st_idx + 14  * 8] = r15;
    out[simd_st_idx + 15  * 8] = r16;
    out[simd_st_idx + 16  * 8] = r17;
    out[simd_st_idx + 17  * 8] = r18;
    out[simd_st_idx + 18  * 8] = r19;
    out[simd_st_idx + 19  * 8] = r20;
    out[simd_st_idx + 20  * 8] = r21;
    out[simd_st_idx + 21  * 8] = r22;
    out[simd_st_idx + 22  * 8] = r23;
    out[simd_st_idx + 23  * 8] = r24;
    out[simd_st_idx + 24  * 8] = r25;
    out[simd_st_idx + 25  * 8] = r26;
    out[simd_st_idx + 26  * 8] = r27;
    out[simd_st_idx + 27  * 8] = r28;
    out[simd_st_idx + 28  * 8] = r29;
    out[simd_st_idx + 29  * 8] = r30;
    out[simd_st_idx + 30  * 8] = r31;
    out[simd_st_idx + 31  * 8] = r32;
    out[simd_st_idx + 32  * 8] = r33;
    out[simd_st_idx + 33  * 8] = r34;
    out[simd_st_idx + 34  * 8] = r35;
    out[simd_st_idx + 35  * 8] = r36;
    out[simd_st_idx + 36  * 8] = r37;
    out[simd_st_idx + 37  * 8] = r38;
    out[simd_st_idx + 38  * 8] = r39;
    out[simd_st_idx + 39  * 8] = r40;
    out[simd_st_idx + 40  * 8] = r41;
    out[simd_st_idx + 41  * 8] = r42;
    out[simd_st_idx + 42  * 8] = r43;
    out[simd_st_idx + 43  * 8] = r44;
    out[simd_st_idx + 44  * 8] = r45;
    out[simd_st_idx + 45  * 8] = r46;
    out[simd_st_idx + 46  * 8] = r47;
    out[simd_st_idx + 47  * 8] = r48;
    out[simd_st_idx + 48  * 8] = r49;
    out[simd_st_idx + 49  * 8] = r50;
    out[simd_st_idx + 50  * 8] = r51;
    out[simd_st_idx + 51  * 8] = r52;
    out[simd_st_idx + 52  * 8] = r53;
    out[simd_st_idx + 53  * 8] = r54;
    out[simd_st_idx + 54  * 8] = r55;
    out[simd_st_idx + 55  * 8] = r56;
    out[simd_st_idx + 56  * 8] = r57;
    out[simd_st_idx + 57  * 8] = r58;
    out[simd_st_idx + 58  * 8] = r59;
    out[simd_st_idx + 59  * 8] = r60;
    out[simd_st_idx + 60  * 8] = r61;
    out[simd_st_idx + 61  * 8] = r62;
    out[simd_st_idx + 62  * 8] = r63;
    out[simd_st_idx + 63  * 8] = r64;
    out[simd_st_idx + 64  * 8] = r65;
    out[simd_st_idx + 65  * 8] = r66;
    out[simd_st_idx + 66  * 8] = r67;
    out[simd_st_idx + 67  * 8] = r68;
    out[simd_st_idx + 68  * 8] = r69;
    out[simd_st_idx + 69  * 8] = r70;
    out[simd_st_idx + 70  * 8] = r71;
    out[simd_st_idx + 71  * 8] = r72;
    out[simd_st_idx + 72  * 8] = r73;
    out[simd_st_idx + 73  * 8] = r74;
    out[simd_st_idx + 74  * 8] = r75;
    out[simd_st_idx + 75  * 8] = r76;
    out[simd_st_idx + 76  * 8] = r77;
    out[simd_st_idx + 77  * 8] = r78;
    out[simd_st_idx + 78  * 8] = r79;
    out[simd_st_idx + 79  * 8] = r80;
    out[simd_st_idx + 80  * 8] = r81;
    out[simd_st_idx + 81  * 8] = r82;
    out[simd_st_idx + 82  * 8] = r83;
    out[simd_st_idx + 83  * 8] = r84;
    out[simd_st_idx + 84  * 8] = r85;
    out[simd_st_idx + 85  * 8] = r86;
    out[simd_st_idx + 86  * 8] = r87;
    out[simd_st_idx + 87  * 8] = r88;
    out[simd_st_idx + 88  * 8] = r89;
    out[simd_st_idx + 89  * 8] = r90;
    out[simd_st_idx + 90  * 8] = r91;
    out[simd_st_idx + 91  * 8] = r92;
    out[simd_st_idx + 92  * 8] = r93;
    out[simd_st_idx + 93  * 8] = r94;
    out[simd_st_idx + 94  * 8] = r95;
    out[simd_st_idx + 95  * 8] = r96;
    out[simd_st_idx + 96  * 8] = r97;
    out[simd_st_idx + 97  * 8] = r98;
    out[simd_st_idx + 98  * 8] = r99;
    out[simd_st_idx + 99  * 8] = r100;
  }
}

// Copyright 2015 Allan MacKinnon. All rights reserved.

__kernel
void
spillage_ulong_v1(__global const ulong* const __restrict__ in, __global ulong* const out)
{
  ulong r1;
  ulong r2;
  ulong r3;
  ulong r4;
  ulong r5;
  ulong r6;
  ulong r7;
  ulong r8;
  ulong r9;
  ulong r10;
  ulong r11;
  ulong r12;
  ulong r13;
  ulong r14;
  ulong r15;
  ulong r16;
  ulong r17;
  ulong r18;
  ulong r19;
  ulong r20;
  ulong r21;
  ulong r22;
  ulong r23;
  ulong r24;
  ulong r25;
  ulong r26;
  ulong r27;
  ulong r28;
  ulong r29;
  ulong r30;
  ulong r31;
  ulong r32;
  ulong r33;
  ulong r34;
  ulong r35;
  ulong r36;
  ulong r37;
  ulong r38;
  ulong r39;
  ulong r40;
  ulong r41;
  ulong r42;
  ulong r43;
  ulong r44;
  ulong r45;
  ulong r46;
  ulong r47;
  ulong r48;
  ulong r49;
  ulong r50;
  {
    const uint simd_id      = (get_local_size(0) * get_group_id(0) + get_local_id(0)) / 8;
    const uint simd_lane_id = get_local_id(0) & 7;
    const uint simd_ld_idx  = simd_id * 400 + simd_lane_id;
    r1   = in[simd_ld_idx + 0   * 8];
    r2   = in[simd_ld_idx + 1   * 8];
    r3   = in[simd_ld_idx + 2   * 8];
    r4   = in[simd_ld_idx + 3   * 8];
    r5   = in[simd_ld_idx + 4   * 8];
    r6   = in[simd_ld_idx + 5   * 8];
    r7   = in[simd_ld_idx + 6   * 8];
    r8   = in[simd_ld_idx + 7   * 8];
    r9   = in[simd_ld_idx + 8   * 8];
    r10  = in[simd_ld_idx + 9   * 8];
    r11  = in[simd_ld_idx + 10  * 8];
    r12  = in[simd_ld_idx + 11  * 8];
    r13  = in[simd_ld_idx + 12  * 8];
    r14  = in[simd_ld_idx + 13  * 8];
    r15  = in[simd_ld_idx + 14  * 8];
    r16  = in[simd_ld_idx + 15  * 8];
    r17  = in[simd_ld_idx + 16  * 8];
    r18  = in[simd_ld_idx + 17  * 8];
    r19  = in[simd_ld_idx + 18  * 8];
    r20  = in[simd_ld_idx + 19  * 8];
    r21  = in[simd_ld_idx + 20  * 8];
    r22  = in[simd_ld_idx + 21  * 8];
    r23  = in[simd_ld_idx + 22  * 8];
    r24  = in[simd_ld_idx + 23  * 8];
    r25  = in[simd_ld_idx + 24  * 8];
    r26  = in[simd_ld_idx + 25  * 8];
    r27  = in[simd_ld_idx + 26  * 8];
    r28  = in[simd_ld_idx + 27  * 8];
    r29  = in[simd_ld_idx + 28  * 8];
    r30  = in[simd_ld_idx + 29  * 8];
    r31  = in[simd_ld_idx + 30  * 8];
    r32  = in[simd_ld_idx + 31  * 8];
    r33  = in[simd_ld_idx + 32  * 8];
    r34  = in[simd_ld_idx + 33  * 8];
    r35  = in[simd_ld_idx + 34  * 8];
    r36  = in[simd_ld_idx + 35  * 8];
    r37  = in[simd_ld_idx + 36  * 8];
    r38  = in[simd_ld_idx + 37  * 8];
    r39  = in[simd_ld_idx + 38  * 8];
    r40  = in[simd_ld_idx + 39  * 8];
    r41  = in[simd_ld_idx + 40  * 8];
    r42  = in[simd_ld_idx + 41  * 8];
    r43  = in[simd_ld_idx + 42  * 8];
    r44  = in[simd_ld_idx + 43  * 8];
    r45  = in[simd_ld_idx + 44  * 8];
    r46  = in[simd_ld_idx + 45  * 8];
    r47  = in[simd_ld_idx + 46  * 8];
    r48  = in[simd_ld_idx + 47  * 8];
    r49  = in[simd_ld_idx + 48  * 8];
    r50  = in[simd_ld_idx + 49  * 8];
  }
  if (r1   > r33) {
    const ulong t = r33;
    r33           = r1;
    r1            = t;
  }
  if (r2   > r34) {
    const ulong t = r34;
    r34           = r2;
    r2            = t;
  }
  if (r3   > r35) {
    const ulong t = r35;
    r35           = r3;
    r3            = t;
  }
  if (r4   > r36) {
    const ulong t = r36;
    r36           = r4;
    r4            = t;
  }
  if (r5   > r37) {
    const ulong t = r37;
    r37           = r5;
    r5            = t;
  }
  if (r6   > r38) {
    const ulong t = r38;
    r38           = r6;
    r6            = t;
  }
  if (r7   > r39) {
    const ulong t = r39;
    r39           = r7;
    r7            = t;
  }
  if (r8   > r40) {
    const ulong t = r40;
    r40           = r8;
    r8            = t;
  }
  if (r9   > r41) {
    const ulong t = r41;
    r41           = r9;
    r9            = t;
  }
  if (r10  > r42) {
    const ulong t = r42;
    r42           = r10;
    r10           = t;
  }
  if (r11  > r43) {
    const ulong t = r43;
    r43           = r11;
    r11           = t;
  }
  if (r12  > r44) {
    const ulong t = r44;
    r44           = r12;
    r12           = t;
  }
  if (r13  > r45) {
    const ulong t = r45;
    r45           = r13;
    r13           = t;
  }
  if (r14  > r46) {
    const ulong t = r46;
    r46           = r14;
    r14           = t;
  }
  if (r15  > r47) {
    const ulong t = r47;
    r47           = r15;
    r15           = t;
  }
  if (r16  > r48) {
    const ulong t = r48;
    r48           = r16;
    r16           = t;
  }
  if (r17  > r49) {
    const ulong t = r49;
    r49           = r17;
    r17           = t;
  }
  if (r18  > r50) {
    const ulong t = r50;
    r50           = r18;
    r18           = t;
  }
  if (r1   > r17) {
    const ulong t = r17;
    r17           = r1;
    r1            = t;
  }
  if (r2   > r18) {
    const ulong t = r18;
    r18           = r2;
    r2            = t;
  }
  if (r3   > r19) {
    const ulong t = r19;
    r19           = r3;
    r3            = t;
  }
  if (r4   > r20) {
    const ulong t = r20;
    r20           = r4;
    r4            = t;
  }
  if (r5   > r21) {
    const ulong t = r21;
    r21           = r5;
    r5            = t;
  }
  if (r6   > r22) {
    const ulong t = r22;
    r22           = r6;
    r6            = t;
  }
  if (r7   > r23) {
    const ulong t = r23;
    r23           = r7;
    r7            = t;
  }
  if (r8   > r24) {
    const ulong t = r24;
    r24           = r8;
    r8            = t;
  }
  if (r9   > r25) {
    const ulong t = r25;
    r25           = r9;
    r9            = t;
  }
  if (r10  > r26) {
    const ulong t = r26;
    r26           = r10;
    r10           = t;
  }
  if (r11  > r27) {
    const ulong t = r27;
    r27           = r11;
    r11           = t;
  }
  if (r12  > r28) {
    const ulong t = r28;
    r28           = r12;
    r12           = t;
  }
  if (r13  > r29) {
    const ulong t = r29;
    r29           = r13;
    r13           = t;
  }
  if (r14  > r30) {
    const ulong t = r30;
    r30           = r14;
    r14           = t;
  }
  if (r15  > r31) {
    const ulong t = r31;
    r31           = r15;
    r15           = t;
  }
  if (r16  > r32) {
    const ulong t = r32;
    r32           = r16;
    r16           = t;
  }
  if (r33  > r49) {
    const ulong t = r49;
    r49           = r33;
    r33           = t;
  }
  if (r34  > r50) {
    const ulong t = r50;
    r50           = r34;
    r34           = t;
  }
  if (r17  > r33) {
    const ulong t = r33;
    r33           = r17;
    r17           = t;
  }
  if (r18  > r34) {
    const ulong t = r34;
    r34           = r18;
    r18           = t;
  }
  if (r19  > r35) {
    const ulong t = r35;
    r35           = r19;
    r19           = t;
  }
  if (r20  > r36) {
    const ulong t = r36;
    r36           = r20;
    r20           = t;
  }
  if (r21  > r37) {
    const ulong t = r37;
    r37           = r21;
    r21           = t;
  }
  if (r22  > r38) {
    const ulong t = r38;
    r38           = r22;
    r22           = t;
  }
  if (r23  > r39) {
    const ulong t = r39;
    r39           = r23;
    r23           = t;
  }
  if (r24  > r40) {
    const ulong t = r40;
    r40           = r24;
    r24           = t;
  }
  if (r25  > r41) {
    const ulong t = r41;
    r41           = r25;
    r25           = t;
  }
  if (r26  > r42) {
    const ulong t = r42;
    r42           = r26;
    r26           = t;
  }
  if (r27  > r43) {
    const ulong t = r43;
    r43           = r27;
    r27           = t;
  }
  if (r28  > r44) {
    const ulong t = r44;
    r44           = r28;
    r28           = t;
  }
  if (r29  > r45) {
    const ulong t = r45;
    r45           = r29;
    r29           = t;
  }
  if (r30  > r46) {
    const ulong t = r46;
    r46           = r30;
    r30           = t;
  }
  if (r31  > r47) {
    const ulong t = r47;
    r47           = r31;
    r31           = t;
  }
  if (r32  > r48) {
    const ulong t = r48;
    r48           = r32;
    r32           = t;
  }
  if (r1   > r9) {
    const ulong t = r9;
    r9            = r1;
    r1            = t;
  }
  if (r2   > r10) {
    const ulong t = r10;
    r10           = r2;
    r2            = t;
  }
  if (r3   > r11) {
    const ulong t = r11;
    r11           = r3;
    r3            = t;
  }
  if (r4   > r12) {
    const ulong t = r12;
    r12           = r4;
    r4            = t;
  }
  if (r5   > r13) {
    const ulong t = r13;
    r13           = r5;
    r5            = t;
  }
  if (r6   > r14) {
    const ulong t = r14;
    r14           = r6;
    r6            = t;
  }
  if (r7   > r15) {
    const ulong t = r15;
    r15           = r7;
    r7            = t;
  }
  if (r8   > r16) {
    const ulong t = r16;
    r16           = r8;
    r8            = t;
  }
  if (r17  > r25) {
    const ulong t = r25;
    r25           = r17;
    r17           = t;
  }
  if (r18  > r26) {
    const ulong t = r26;
    r26           = r18;
    r18           = t;
  }
  if (r19  > r27) {
    const ulong t = r27;
    r27           = r19;
    r19           = t;
  }
  if (r20  > r28) {
    const ulong t = r28;
    r28           = r20;
    r20           = t;
  }
  if (r21  > r29) {
    const ulong t = r29;
    r29           = r21;
    r21           = t;
  }
  if (r22  > r30) {
    const ulong t = r30;
    r30           = r22;
    r22           = t;
  }
  if (r23  > r31) {
    const ulong t = r31;
    r31           = r23;
    r23           = t;
  }
  if (r24  > r32) {
    const ulong t = r32;
    r32           = r24;
    r24           = t;
  }
  if (r33  > r41) {
    const ulong t = r41;
    r41           = r33;
    r33           = t;
  }
  if (r34  > r42) {
    const ulong t = r42;
    r42           = r34;
    r34           = t;
  }
  if (r35  > r43) {
    const ulong t = r43;
    r43           = r35;
    r35           = t;
  }
  if (r36  > r44) {
    const ulong t = r44;
    r44           = r36;
    r36           = t;
  }
  if (r37  > r45) {
    const ulong t = r45;
    r45           = r37;
    r37           = t;
  }
  if (r38  > r46) {
    const ulong t = r46;
    r46           = r38;
    r38           = t;
  }
  if (r39  > r47) {
    const ulong t = r47;
    r47           = r39;
    r39           = t;
  }
  if (r40  > r48) {
    const ulong t = r48;
    r48           = r40;
    r40           = t;
  }
  if (r9   > r33) {
    const ulong t = r33;
    r33           = r9;
    r9            = t;
  }
  if (r10  > r34) {
    const ulong t = r34;
    r34           = r10;
    r10           = t;
  }
  if (r11  > r35) {
    const ulong t = r35;
    r35           = r11;
    r11           = t;
  }
  if (r12  > r36) {
    const ulong t = r36;
    r36           = r12;
    r12           = t;
  }
  if (r13  > r37) {
    const ulong t = r37;
    r37           = r13;
    r13           = t;
  }
  if (r14  > r38) {
    const ulong t = r38;
    r38           = r14;
    r14           = t;
  }
  if (r15  > r39) {
    const ulong t = r39;
    r39           = r15;
    r15           = t;
  }
  if (r16  > r40) {
    const ulong t = r40;
    r40           = r16;
    r16           = t;
  }
  if (r25  > r49) {
    const ulong t = r49;
    r49           = r25;
    r25           = t;
  }
  if (r26  > r50) {
    const ulong t = r50;
    r50           = r26;
    r26           = t;
  }
  if (r9   > r17) {
    const ulong t = r17;
    r17           = r9;
    r9            = t;
  }
  if (r10  > r18) {
    const ulong t = r18;
    r18           = r10;
    r10           = t;
  }
  if (r11  > r19) {
    const ulong t = r19;
    r19           = r11;
    r11           = t;
  }
  if (r12  > r20) {
    const ulong t = r20;
    r20           = r12;
    r12           = t;
  }
  if (r13  > r21) {
    const ulong t = r21;
    r21           = r13;
    r13           = t;
  }
  if (r14  > r22) {
    const ulong t = r22;
    r22           = r14;
    r14           = t;
  }
  if (r15  > r23) {
    const ulong t = r23;
    r23           = r15;
    r15           = t;
  }
  if (r16  > r24) {
    const ulong t = r24;
    r24           = r16;
    r16           = t;
  }
  if (r25  > r33) {
    const ulong t = r33;
    r33           = r25;
    r25           = t;
  }
  if (r26  > r34) {
    const ulong t = r34;
    r34           = r26;
    r26           = t;
  }
  if (r27  > r35) {
    const ulong t = r35;
    r35           = r27;
    r27           = t;
  }
  if (r28  > r36) {
    const ulong t = r36;
    r36           = r28;
    r28           = t;
  }
  if (r29  > r37) {
    const ulong t = r37;
    r37           = r29;
    r29           = t;
  }
  if (r30  > r38) {
    const ulong t = r38;
    r38           = r30;
    r30           = t;
  }
  if (r31  > r39) {
    const ulong t = r39;
    r39           = r31;
    r31           = t;
  }
  if (r32  > r40) {
    const ulong t = r40;
    r40           = r32;
    r32           = t;
  }
  if (r41  > r49) {
    const ulong t = r49;
    r49           = r41;
    r41           = t;
  }
  if (r42  > r50) {
    const ulong t = r50;
    r50           = r42;
    r42           = t;
  }
  if (r1   > r5) {
    const ulong t = r5;
    r5            = r1;
    r1            = t;
  }
  if (r2   > r6) {
    const ulong t = r6;
    r6            = r2;
    r2            = t;
  }
  if (r3   > r7) {
    const ulong t = r7;
    r7            = r3;
    r3            = t;
  }
  if (r4   > r8) {
    const ulong t = r8;
    r8            = r4;
    r4            = t;
  }
  if (r9   > r13) {
    const ulong t = r13;
    r13           = r9;
    r9            = t;
  }
  if (r10  > r14) {
    const ulong t = r14;
    r14           = r10;
    r10           = t;
  }
  if (r11  > r15) {
    const ulong t = r15;
    r15           = r11;
    r11           = t;
  }
  if (r12  > r16) {
    const ulong t = r16;
    r16           = r12;
    r12           = t;
  }
  if (r17  > r21) {
    const ulong t = r21;
    r21           = r17;
    r17           = t;
  }
  if (r18  > r22) {
    const ulong t = r22;
    r22           = r18;
    r18           = t;
  }
  if (r19  > r23) {
    const ulong t = r23;
    r23           = r19;
    r19           = t;
  }
  if (r20  > r24) {
    const ulong t = r24;
    r24           = r20;
    r20           = t;
  }
  if (r25  > r29) {
    const ulong t = r29;
    r29           = r25;
    r25           = t;
  }
  if (r26  > r30) {
    const ulong t = r30;
    r30           = r26;
    r26           = t;
  }
  if (r27  > r31) {
    const ulong t = r31;
    r31           = r27;
    r27           = t;
  }
  if (r28  > r32) {
    const ulong t = r32;
    r32           = r28;
    r28           = t;
  }
  if (r33  > r37) {
    const ulong t = r37;
    r37           = r33;
    r33           = t;
  }
  if (r34  > r38) {
    const ulong t = r38;
    r38           = r34;
    r34           = t;
  }
  if (r35  > r39) {
    const ulong t = r39;
    r39           = r35;
    r35           = t;
  }
  if (r36  > r40) {
    const ulong t = r40;
    r40           = r36;
    r36           = t;
  }
  if (r41  > r45) {
    const ulong t = r45;
    r45           = r41;
    r41           = t;
  }
  if (r42  > r46) {
    const ulong t = r46;
    r46           = r42;
    r42           = t;
  }
  if (r43  > r47) {
    const ulong t = r47;
    r47           = r43;
    r43           = t;
  }
  if (r44  > r48) {
    const ulong t = r48;
    r48           = r44;
    r44           = t;
  }
  if (r5   > r33) {
    const ulong t = r33;
    r33           = r5;
    r5            = t;
  }
  if (r6   > r34) {
    const ulong t = r34;
    r34           = r6;
    r6            = t;
  }
  if (r7   > r35) {
    const ulong t = r35;
    r35           = r7;
    r7            = t;
  }
  if (r8   > r36) {
    const ulong t = r36;
    r36           = r8;
    r8            = t;
  }
  if (r13  > r41) {
    const ulong t = r41;
    r41           = r13;
    r13           = t;
  }
  if (r14  > r42) {
    const ulong t = r42;
    r42           = r14;
    r14           = t;
  }
  if (r15  > r43) {
    const ulong t = r43;
    r43           = r15;
    r15           = t;
  }
  if (r16  > r44) {
    const ulong t = r44;
    r44           = r16;
    r16           = t;
  }
  if (r21  > r49) {
    const ulong t = r49;
    r49           = r21;
    r21           = t;
  }
  if (r22  > r50) {
    const ulong t = r50;
    r50           = r22;
    r22           = t;
  }
  if (r5   > r17) {
    const ulong t = r17;
    r17           = r5;
    r5            = t;
  }
  if (r6   > r18) {
    const ulong t = r18;
    r18           = r6;
    r6            = t;
  }
  if (r7   > r19) {
    const ulong t = r19;
    r19           = r7;
    r7            = t;
  }
  if (r8   > r20) {
    const ulong t = r20;
    r20           = r8;
    r8            = t;
  }
  if (r13  > r25) {
    const ulong t = r25;
    r25           = r13;
    r13           = t;
  }
  if (r14  > r26) {
    const ulong t = r26;
    r26           = r14;
    r14           = t;
  }
  if (r15  > r27) {
    const ulong t = r27;
    r27           = r15;
    r15           = t;
  }
  if (r16  > r28) {
    const ulong t = r28;
    r28           = r16;
    r16           = t;
  }
  if (r21  > r33) {
    const ulong t = r33;
    r33           = r21;
    r21           = t;
  }
  if (r22  > r34) {
    const ulong t = r34;
    r34           = r22;
    r22           = t;
  }
  if (r23  > r35) {
    const ulong t = r35;
    r35           = r23;
    r23           = t;
  }
  if (r24  > r36) {
    const ulong t = r36;
    r36           = r24;
    r24           = t;
  }
  if (r29  > r41) {
    const ulong t = r41;
    r41           = r29;
    r29           = t;
  }
  if (r30  > r42) {
    const ulong t = r42;
    r42           = r30;
    r30           = t;
  }
  if (r31  > r43) {
    const ulong t = r43;
    r43           = r31;
    r31           = t;
  }
  if (r32  > r44) {
    const ulong t = r44;
    r44           = r32;
    r32           = t;
  }
  if (r37  > r49) {
    const ulong t = r49;
    r49           = r37;
    r37           = t;
  }
  if (r38  > r50) {
    const ulong t = r50;
    r50           = r38;
    r38           = t;
  }
  if (r5   > r9) {
    const ulong t = r9;
    r9            = r5;
    r5            = t;
  }
  if (r6   > r10) {
    const ulong t = r10;
    r10           = r6;
    r6            = t;
  }
  if (r7   > r11) {
    const ulong t = r11;
    r11           = r7;
    r7            = t;
  }
  if (r8   > r12) {
    const ulong t = r12;
    r12           = r8;
    r8            = t;
  }
  if (r13  > r17) {
    const ulong t = r17;
    r17           = r13;
    r13           = t;
  }
  if (r14  > r18) {
    const ulong t = r18;
    r18           = r14;
    r14           = t;
  }
  if (r15  > r19) {
    const ulong t = r19;
    r19           = r15;
    r15           = t;
  }
  if (r16  > r20) {
    const ulong t = r20;
    r20           = r16;
    r16           = t;
  }
  if (r21  > r25) {
    const ulong t = r25;
    r25           = r21;
    r21           = t;
  }
  if (r22  > r26) {
    const ulong t = r26;
    r26           = r22;
    r22           = t;
  }
  if (r23  > r27) {
    const ulong t = r27;
    r27           = r23;
    r23           = t;
  }
  if (r24  > r28) {
    const ulong t = r28;
    r28           = r24;
    r24           = t;
  }
  if (r29  > r33) {
    const ulong t = r33;
    r33           = r29;
    r29           = t;
  }
  if (r30  > r34) {
    const ulong t = r34;
    r34           = r30;
    r30           = t;
  }
  if (r31  > r35) {
    const ulong t = r35;
    r35           = r31;
    r31           = t;
  }
  if (r32  > r36) {
    const ulong t = r36;
    r36           = r32;
    r32           = t;
  }
  if (r37  > r41) {
    const ulong t = r41;
    r41           = r37;
    r37           = t;
  }
  if (r38  > r42) {
    const ulong t = r42;
    r42           = r38;
    r38           = t;
  }
  if (r39  > r43) {
    const ulong t = r43;
    r43           = r39;
    r39           = t;
  }
  if (r40  > r44) {
    const ulong t = r44;
    r44           = r40;
    r40           = t;
  }
  if (r45  > r49) {
    const ulong t = r49;
    r49           = r45;
    r45           = t;
  }
  if (r46  > r50) {
    const ulong t = r50;
    r50           = r46;
    r46           = t;
  }
  if (r1   > r3) {
    const ulong t = r3;
    r3            = r1;
    r1            = t;
  }
  if (r2   > r4) {
    const ulong t = r4;
    r4            = r2;
    r2            = t;
  }
  if (r5   > r7) {
    const ulong t = r7;
    r7            = r5;
    r5            = t;
  }
  if (r6   > r8) {
    const ulong t = r8;
    r8            = r6;
    r6            = t;
  }
  if (r9   > r11) {
    const ulong t = r11;
    r11           = r9;
    r9            = t;
  }
  if (r10  > r12) {
    const ulong t = r12;
    r12           = r10;
    r10           = t;
  }
  if (r13  > r15) {
    const ulong t = r15;
    r15           = r13;
    r13           = t;
  }
  if (r14  > r16) {
    const ulong t = r16;
    r16           = r14;
    r14           = t;
  }
  if (r17  > r19) {
    const ulong t = r19;
    r19           = r17;
    r17           = t;
  }
  if (r18  > r20) {
    const ulong t = r20;
    r20           = r18;
    r18           = t;
  }
  if (r21  > r23) {
    const ulong t = r23;
    r23           = r21;
    r21           = t;
  }
  if (r22  > r24) {
    const ulong t = r24;
    r24           = r22;
    r22           = t;
  }
  if (r25  > r27) {
    const ulong t = r27;
    r27           = r25;
    r25           = t;
  }
  if (r26  > r28) {
    const ulong t = r28;
    r28           = r26;
    r26           = t;
  }
  if (r29  > r31) {
    const ulong t = r31;
    r31           = r29;
    r29           = t;
  }
  if (r30  > r32) {
    const ulong t = r32;
    r32           = r30;
    r30           = t;
  }
  if (r33  > r35) {
    const ulong t = r35;
    r35           = r33;
    r33           = t;
  }
  if (r34  > r36) {
    const ulong t = r36;
    r36           = r34;
    r34           = t;
  }
  if (r37  > r39) {
    const ulong t = r39;
    r39           = r37;
    r37           = t;
  }
  if (r38  > r40) {
    const ulong t = r40;
    r40           = r38;
    r38           = t;
  }
  if (r41  > r43) {
    const ulong t = r43;
    r43           = r41;
    r41           = t;
  }
  if (r42  > r44) {
    const ulong t = r44;
    r44           = r42;
    r42           = t;
  }
  if (r45  > r47) {
    const ulong t = r47;
    r47           = r45;
    r45           = t;
  }
  if (r46  > r48) {
    const ulong t = r48;
    r48           = r46;
    r46           = t;
  }
  if (r3   > r33) {
    const ulong t = r33;
    r33           = r3;
    r3            = t;
  }
  if (r4   > r34) {
    const ulong t = r34;
    r34           = r4;
    r4            = t;
  }
  if (r7   > r37) {
    const ulong t = r37;
    r37           = r7;
    r7            = t;
  }
  if (r8   > r38) {
    const ulong t = r38;
    r38           = r8;
    r8            = t;
  }
  if (r11  > r41) {
    const ulong t = r41;
    r41           = r11;
    r11           = t;
  }
  if (r12  > r42) {
    const ulong t = r42;
    r42           = r12;
    r12           = t;
  }
  if (r15  > r45) {
    const ulong t = r45;
    r45           = r15;
    r15           = t;
  }
  if (r16  > r46) {
    const ulong t = r46;
    r46           = r16;
    r16           = t;
  }
  if (r19  > r49) {
    const ulong t = r49;
    r49           = r19;
    r19           = t;
  }
  if (r20  > r50) {
    const ulong t = r50;
    r50           = r20;
    r20           = t;
  }
  if (r3   > r17) {
    const ulong t = r17;
    r17           = r3;
    r3            = t;
  }
  if (r4   > r18) {
    const ulong t = r18;
    r18           = r4;
    r4            = t;
  }
  if (r7   > r21) {
    const ulong t = r21;
    r21           = r7;
    r7            = t;
  }
  if (r8   > r22) {
    const ulong t = r22;
    r22           = r8;
    r8            = t;
  }
  if (r11  > r25) {
    const ulong t = r25;
    r25           = r11;
    r11           = t;
  }
  if (r12  > r26) {
    const ulong t = r26;
    r26           = r12;
    r12           = t;
  }
  if (r15  > r29) {
    const ulong t = r29;
    r29           = r15;
    r15           = t;
  }
  if (r16  > r30) {
    const ulong t = r30;
    r30           = r16;
    r16           = t;
  }
  if (r19  > r33) {
    const ulong t = r33;
    r33           = r19;
    r19           = t;
  }
  if (r20  > r34) {
    const ulong t = r34;
    r34           = r20;
    r20           = t;
  }
  if (r23  > r37) {
    const ulong t = r37;
    r37           = r23;
    r23           = t;
  }
  if (r24  > r38) {
    const ulong t = r38;
    r38           = r24;
    r24           = t;
  }
  if (r27  > r41) {
    const ulong t = r41;
    r41           = r27;
    r27           = t;
  }
  if (r28  > r42) {
    const ulong t = r42;
    r42           = r28;
    r28           = t;
  }
  if (r31  > r45) {
    const ulong t = r45;
    r45           = r31;
    r31           = t;
  }
  if (r32  > r46) {
    const ulong t = r46;
    r46           = r32;
    r32           = t;
  }
  if (r35  > r49) {
    const ulong t = r49;
    r49           = r35;
    r35           = t;
  }
  if (r36  > r50) {
    const ulong t = r50;
    r50           = r36;
    r36           = t;
  }
  if (r3   > r9) {
    const ulong t = r9;
    r9            = r3;
    r3            = t;
  }
  if (r4   > r10) {
    const ulong t = r10;
    r10           = r4;
    r4            = t;
  }
  if (r7   > r13) {
    const ulong t = r13;
    r13           = r7;
    r7            = t;
  }
  if (r8   > r14) {
    const ulong t = r14;
    r14           = r8;
    r8            = t;
  }
  if (r11  > r17) {
    const ulong t = r17;
    r17           = r11;
    r11           = t;
  }
  if (r12  > r18) {
    const ulong t = r18;
    r18           = r12;
    r12           = t;
  }
  if (r15  > r21) {
    const ulong t = r21;
    r21           = r15;
    r15           = t;
  }
  if (r16  > r22) {
    const ulong t = r22;
    r22           = r16;
    r16           = t;
  }
  if (r19  > r25) {
    const ulong t = r25;
    r25           = r19;
    r19           = t;
  }
  if (r20  > r26) {
    const ulong t = r26;
    r26           = r20;
    r20           = t;
  }
  if (r23  > r29) {
    const ulong t = r29;
    r29           = r23;
    r23           = t;
  }
  if (r24  > r30) {
    const ulong t = r30;
    r30           = r24;
    r24           = t;
  }
  if (r27  > r33) {
    const ulong t = r33;
    r33           = r27;
    r27           = t;
  }
  if (r28  > r34) {
    const ulong t = r34;
    r34           = r28;
    r28           = t;
  }
  if (r31  > r37) {
    const ulong t = r37;
    r37           = r31;
    r31           = t;
  }
  if (r32  > r38) {
    const ulong t = r38;
    r38           = r32;
    r32           = t;
  }
  if (r35  > r41) {
    const ulong t = r41;
    r41           = r35;
    r35           = t;
  }
  if (r36  > r42) {
    const ulong t = r42;
    r42           = r36;
    r36           = t;
  }
  if (r39  > r45) {
    const ulong t = r45;
    r45           = r39;
    r39           = t;
  }
  if (r40  > r46) {
    const ulong t = r46;
    r46           = r40;
    r40           = t;
  }
  if (r43  > r49) {
    const ulong t = r49;
    r49           = r43;
    r43           = t;
  }
  if (r44  > r50) {
    const ulong t = r50;
    r50           = r44;
    r44           = t;
  }
  if (r3   > r5) {
    const ulong t = r5;
    r5            = r3;
    r3            = t;
  }
  if (r4   > r6) {
    const ulong t = r6;
    r6            = r4;
    r4            = t;
  }
  if (r7   > r9) {
    const ulong t = r9;
    r9            = r7;
    r7            = t;
  }
  if (r8   > r10) {
    const ulong t = r10;
    r10           = r8;
    r8            = t;
  }
  if (r11  > r13) {
    const ulong t = r13;
    r13           = r11;
    r11           = t;
  }
  if (r12  > r14) {
    const ulong t = r14;
    r14           = r12;
    r12           = t;
  }
  if (r15  > r17) {
    const ulong t = r17;
    r17           = r15;
    r15           = t;
  }
  if (r16  > r18) {
    const ulong t = r18;
    r18           = r16;
    r16           = t;
  }
  if (r19  > r21) {
    const ulong t = r21;
    r21           = r19;
    r19           = t;
  }
  if (r20  > r22) {
    const ulong t = r22;
    r22           = r20;
    r20           = t;
  }
  if (r23  > r25) {
    const ulong t = r25;
    r25           = r23;
    r23           = t;
  }
  if (r24  > r26) {
    const ulong t = r26;
    r26           = r24;
    r24           = t;
  }
  if (r27  > r29) {
    const ulong t = r29;
    r29           = r27;
    r27           = t;
  }
  if (r28  > r30) {
    const ulong t = r30;
    r30           = r28;
    r28           = t;
  }
  if (r31  > r33) {
    const ulong t = r33;
    r33           = r31;
    r31           = t;
  }
  if (r32  > r34) {
    const ulong t = r34;
    r34           = r32;
    r32           = t;
  }
  if (r35  > r37) {
    const ulong t = r37;
    r37           = r35;
    r35           = t;
  }
  if (r36  > r38) {
    const ulong t = r38;
    r38           = r36;
    r36           = t;
  }
  if (r39  > r41) {
    const ulong t = r41;
    r41           = r39;
    r39           = t;
  }
  if (r40  > r42) {
    const ulong t = r42;
    r42           = r40;
    r40           = t;
  }
  if (r43  > r45) {
    const ulong t = r45;
    r45           = r43;
    r43           = t;
  }
  if (r44  > r46) {
    const ulong t = r46;
    r46           = r44;
    r44           = t;
  }
  if (r47  > r49) {
    const ulong t = r49;
    r49           = r47;
    r47           = t;
  }
  if (r48  > r50) {
    const ulong t = r50;
    r50           = r48;
    r48           = t;
  }
  if (r1   > r2) {
    const ulong t = r2;
    r2            = r1;
    r1            = t;
  }
  if (r3   > r4) {
    const ulong t = r4;
    r4            = r3;
    r3            = t;
  }
  if (r5   > r6) {
    const ulong t = r6;
    r6            = r5;
    r5            = t;
  }
  if (r7   > r8) {
    const ulong t = r8;
    r8            = r7;
    r7            = t;
  }
  if (r9   > r10) {
    const ulong t = r10;
    r10           = r9;
    r9            = t;
  }
  if (r11  > r12) {
    const ulong t = r12;
    r12           = r11;
    r11           = t;
  }
  if (r13  > r14) {
    const ulong t = r14;
    r14           = r13;
    r13           = t;
  }
  if (r15  > r16) {
    const ulong t = r16;
    r16           = r15;
    r15           = t;
  }
  if (r17  > r18) {
    const ulong t = r18;
    r18           = r17;
    r17           = t;
  }
  if (r19  > r20) {
    const ulong t = r20;
    r20           = r19;
    r19           = t;
  }
  if (r21  > r22) {
    const ulong t = r22;
    r22           = r21;
    r21           = t;
  }
  if (r23  > r24) {
    const ulong t = r24;
    r24           = r23;
    r23           = t;
  }
  if (r25  > r26) {
    const ulong t = r26;
    r26           = r25;
    r25           = t;
  }
  if (r27  > r28) {
    const ulong t = r28;
    r28           = r27;
    r27           = t;
  }
  if (r29  > r30) {
    const ulong t = r30;
    r30           = r29;
    r29           = t;
  }
  if (r31  > r32) {
    const ulong t = r32;
    r32           = r31;
    r31           = t;
  }
  if (r33  > r34) {
    const ulong t = r34;
    r34           = r33;
    r33           = t;
  }
  if (r35  > r36) {
    const ulong t = r36;
    r36           = r35;
    r35           = t;
  }
  if (r37  > r38) {
    const ulong t = r38;
    r38           = r37;
    r37           = t;
  }
  if (r39  > r40) {
    const ulong t = r40;
    r40           = r39;
    r39           = t;
  }
  if (r41  > r42) {
    const ulong t = r42;
    r42           = r41;
    r41           = t;
  }
  if (r43  > r44) {
    const ulong t = r44;
    r44           = r43;
    r43           = t;
  }
  if (r45  > r46) {
    const ulong t = r46;
    r46           = r45;
    r45           = t;
  }
  if (r47  > r48) {
    const ulong t = r48;
    r48           = r47;
    r47           = t;
  }
  if (r49  > r50) {
    const ulong t = r50;
    r50           = r49;
    r49           = t;
  }
  if (r2   > r33) {
    const ulong t = r33;
    r33           = r2;
    r2            = t;
  }
  if (r4   > r35) {
    const ulong t = r35;
    r35           = r4;
    r4            = t;
  }
  if (r6   > r37) {
    const ulong t = r37;
    r37           = r6;
    r6            = t;
  }
  if (r8   > r39) {
    const ulong t = r39;
    r39           = r8;
    r8            = t;
  }
  if (r10  > r41) {
    const ulong t = r41;
    r41           = r10;
    r10           = t;
  }
  if (r12  > r43) {
    const ulong t = r43;
    r43           = r12;
    r12           = t;
  }
  if (r14  > r45) {
    const ulong t = r45;
    r45           = r14;
    r14           = t;
  }
  if (r16  > r47) {
    const ulong t = r47;
    r47           = r16;
    r16           = t;
  }
  if (r18  > r49) {
    const ulong t = r49;
    r49           = r18;
    r18           = t;
  }
  if (r2   > r17) {
    const ulong t = r17;
    r17           = r2;
    r2            = t;
  }
  if (r4   > r19) {
    const ulong t = r19;
    r19           = r4;
    r4            = t;
  }
  if (r6   > r21) {
    const ulong t = r21;
    r21           = r6;
    r6            = t;
  }
  if (r8   > r23) {
    const ulong t = r23;
    r23           = r8;
    r8            = t;
  }
  if (r10  > r25) {
    const ulong t = r25;
    r25           = r10;
    r10           = t;
  }
  if (r12  > r27) {
    const ulong t = r27;
    r27           = r12;
    r12           = t;
  }
  if (r14  > r29) {
    const ulong t = r29;
    r29           = r14;
    r14           = t;
  }
  if (r16  > r31) {
    const ulong t = r31;
    r31           = r16;
    r16           = t;
  }
  if (r18  > r33) {
    const ulong t = r33;
    r33           = r18;
    r18           = t;
  }
  if (r20  > r35) {
    const ulong t = r35;
    r35           = r20;
    r20           = t;
  }
  if (r22  > r37) {
    const ulong t = r37;
    r37           = r22;
    r22           = t;
  }
  if (r24  > r39) {
    const ulong t = r39;
    r39           = r24;
    r24           = t;
  }
  if (r26  > r41) {
    const ulong t = r41;
    r41           = r26;
    r26           = t;
  }
  if (r28  > r43) {
    const ulong t = r43;
    r43           = r28;
    r28           = t;
  }
  if (r30  > r45) {
    const ulong t = r45;
    r45           = r30;
    r30           = t;
  }
  if (r32  > r47) {
    const ulong t = r47;
    r47           = r32;
    r32           = t;
  }
  if (r34  > r49) {
    const ulong t = r49;
    r49           = r34;
    r34           = t;
  }
  if (r2   > r9) {
    const ulong t = r9;
    r9            = r2;
    r2            = t;
  }
  if (r4   > r11) {
    const ulong t = r11;
    r11           = r4;
    r4            = t;
  }
  if (r6   > r13) {
    const ulong t = r13;
    r13           = r6;
    r6            = t;
  }
  if (r8   > r15) {
    const ulong t = r15;
    r15           = r8;
    r8            = t;
  }
  if (r10  > r17) {
    const ulong t = r17;
    r17           = r10;
    r10           = t;
  }
  if (r12  > r19) {
    const ulong t = r19;
    r19           = r12;
    r12           = t;
  }
  if (r14  > r21) {
    const ulong t = r21;
    r21           = r14;
    r14           = t;
  }
  if (r16  > r23) {
    const ulong t = r23;
    r23           = r16;
    r16           = t;
  }
  if (r18  > r25) {
    const ulong t = r25;
    r25           = r18;
    r18           = t;
  }
  if (r20  > r27) {
    const ulong t = r27;
    r27           = r20;
    r20           = t;
  }
  if (r22  > r29) {
    const ulong t = r29;
    r29           = r22;
    r22           = t;
  }
  if (r24  > r31) {
    const ulong t = r31;
    r31           = r24;
    r24           = t;
  }
  if (r26  > r33) {
    const ulong t = r33;
    r33           = r26;
    r26           = t;
  }
  if (r28  > r35) {
    const ulong t = r35;
    r35           = r28;
    r28           = t;
  }
  if (r30  > r37) {
    const ulong t = r37;
    r37           = r30;
    r30           = t;
  }
  if (r32  > r39) {
    const ulong t = r39;
    r39           = r32;
    r32           = t;
  }
  if (r34  > r41) {
    const ulong t = r41;
    r41           = r34;
    r34           = t;
  }
  if (r36  > r43) {
    const ulong t = r43;
    r43           = r36;
    r36           = t;
  }
  if (r38  > r45) {
    const ulong t = r45;
    r45           = r38;
    r38           = t;
  }
  if (r40  > r47) {
    const ulong t = r47;
    r47           = r40;
    r40           = t;
  }
  if (r42  > r49) {
    const ulong t = r49;
    r49           = r42;
    r42           = t;
  }
  if (r2   > r5) {
    const ulong t = r5;
    r5            = r2;
    r2            = t;
  }
  if (r4   > r7) {
    const ulong t = r7;
    r7            = r4;
    r4            = t;
  }
  if (r6   > r9) {
    const ulong t = r9;
    r9            = r6;
    r6            = t;
  }
  if (r8   > r11) {
    const ulong t = r11;
    r11           = r8;
    r8            = t;
  }
  if (r10  > r13) {
    const ulong t = r13;
    r13           = r10;
    r10           = t;
  }
  if (r12  > r15) {
    const ulong t = r15;
    r15           = r12;
    r12           = t;
  }
  if (r14  > r17) {
    const ulong t = r17;
    r17           = r14;
    r14           = t;
  }
  if (r16  > r19) {
    const ulong t = r19;
    r19           = r16;
    r16           = t;
  }
  if (r18  > r21) {
    const ulong t = r21;
    r21           = r18;
    r18           = t;
  }
  if (r20  > r23) {
    const ulong t = r23;
    r23           = r20;
    r20           = t;
  }
  if (r22  > r25) {
    const ulong t = r25;
    r25           = r22;
    r22           = t;
  }
  if (r24  > r27) {
    const ulong t = r27;
    r27           = r24;
    r24           = t;
  }
  if (r26  > r29) {
    const ulong t = r29;
    r29           = r26;
    r26           = t;
  }
  if (r28  > r31) {
    const ulong t = r31;
    r31           = r28;
    r28           = t;
  }
  if (r30  > r33) {
    const ulong t = r33;
    r33           = r30;
    r30           = t;
  }
  if (r32  > r35) {
    const ulong t = r35;
    r35           = r32;
    r32           = t;
  }
  if (r34  > r37) {
    const ulong t = r37;
    r37           = r34;
    r34           = t;
  }
  if (r36  > r39) {
    const ulong t = r39;
    r39           = r36;
    r36           = t;
  }
  if (r38  > r41) {
    const ulong t = r41;
    r41           = r38;
    r38           = t;
  }
  if (r40  > r43) {
    const ulong t = r43;
    r43           = r40;
    r40           = t;
  }
  if (r42  > r45) {
    const ulong t = r45;
    r45           = r42;
    r42           = t;
  }
  if (r44  > r47) {
    const ulong t = r47;
    r47           = r44;
    r44           = t;
  }
  if (r46  > r49) {
    const ulong t = r49;
    r49           = r46;
    r46           = t;
  }
  if (r2   > r3) {
    const ulong t = r3;
    r3            = r2;
    r2            = t;
  }
  if (r4   > r5) {
    const ulong t = r5;
    r5            = r4;
    r4            = t;
  }
  if (r6   > r7) {
    const ulong t = r7;
    r7            = r6;
    r6            = t;
  }
  if (r8   > r9) {
    const ulong t = r9;
    r9            = r8;
    r8            = t;
  }
  if (r10  > r11) {
    const ulong t = r11;
    r11           = r10;
    r10           = t;
  }
  if (r12  > r13) {
    const ulong t = r13;
    r13           = r12;
    r12           = t;
  }
  if (r14  > r15) {
    const ulong t = r15;
    r15           = r14;
    r14           = t;
  }
  if (r16  > r17) {
    const ulong t = r17;
    r17           = r16;
    r16           = t;
  }
  if (r18  > r19) {
    const ulong t = r19;
    r19           = r18;
    r18           = t;
  }
  if (r20  > r21) {
    const ulong t = r21;
    r21           = r20;
    r20           = t;
  }
  if (r22  > r23) {
    const ulong t = r23;
    r23           = r22;
    r22           = t;
  }
  if (r24  > r25) {
    const ulong t = r25;
    r25           = r24;
    r24           = t;
  }
  if (r26  > r27) {
    const ulong t = r27;
    r27           = r26;
    r26           = t;
  }
  if (r28  > r29) {
    const ulong t = r29;
    r29           = r28;
    r28           = t;
  }
  if (r30  > r31) {
    const ulong t = r31;
    r31           = r30;
    r30           = t;
  }
  if (r32  > r33) {
    const ulong t = r33;
    r33           = r32;
    r32           = t;
  }
  if (r34  > r35) {
    const ulong t = r35;
    r35           = r34;
    r34           = t;
  }
  if (r36  > r37) {
    const ulong t = r37;
    r37           = r36;
    r36           = t;
  }
  if (r38  > r39) {
    const ulong t = r39;
    r39           = r38;
    r38           = t;
  }
  if (r40  > r41) {
    const ulong t = r41;
    r41           = r40;
    r40           = t;
  }
  if (r42  > r43) {
    const ulong t = r43;
    r43           = r42;
    r42           = t;
  }
  if (r44  > r45) {
    const ulong t = r45;
    r45           = r44;
    r44           = t;
  }
  if (r46  > r47) {
    const ulong t = r47;
    r47           = r46;
    r46           = t;
  }
  if (r48  > r49) {
    const ulong t = r49;
    r49           = r48;
    r48           = t;
  }
  {
    const uint simd_id      = (get_local_size(0) * get_group_id(0) + get_local_id(0)) / 8;
    const uint simd_lane_id = get_local_id(0) & 7;
    const uint simd_st_idx  = simd_id * 400 + simd_lane_id;
    out[simd_st_idx + 0   * 8] = r1;
    out[simd_st_idx + 1   * 8] = r2;
    out[simd_st_idx + 2   * 8] = r3;
    out[simd_st_idx + 3   * 8] = r4;
    out[simd_st_idx + 4   * 8] = r5;
    out[simd_st_idx + 5   * 8] = r6;
    out[simd_st_idx + 6   * 8] = r7;
    out[simd_st_idx + 7   * 8] = r8;
    out[simd_st_idx + 8   * 8] = r9;
    out[simd_st_idx + 9   * 8] = r10;
    out[simd_st_idx + 10  * 8] = r11;
    out[simd_st_idx + 11  * 8] = r12;
    out[simd_st_idx + 12  * 8] = r13;
    out[simd_st_idx + 13  * 8] = r14;
    out[simd_st_idx + 14  * 8] = r15;
    out[simd_st_idx + 15  * 8] = r16;
    out[simd_st_idx + 16  * 8] = r17;
    out[simd_st_idx + 17  * 8] = r18;
    out[simd_st_idx + 18  * 8] = r19;
    out[simd_st_idx + 19  * 8] = r20;
    out[simd_st_idx + 20  * 8] = r21;
    out[simd_st_idx + 21  * 8] = r22;
    out[simd_st_idx + 22  * 8] = r23;
    out[simd_st_idx + 23  * 8] = r24;
    out[simd_st_idx + 24  * 8] = r25;
    out[simd_st_idx + 25  * 8] = r26;
    out[simd_st_idx + 26  * 8] = r27;
    out[simd_st_idx + 27  * 8] = r28;
    out[simd_st_idx + 28  * 8] = r29;
    out[simd_st_idx + 29  * 8] = r30;
    out[simd_st_idx + 30  * 8] = r31;
    out[simd_st_idx + 31  * 8] = r32;
    out[simd_st_idx + 32  * 8] = r33;
    out[simd_st_idx + 33  * 8] = r34;
    out[simd_st_idx + 34  * 8] = r35;
    out[simd_st_idx + 35  * 8] = r36;
    out[simd_st_idx + 36  * 8] = r37;
    out[simd_st_idx + 37  * 8] = r38;
    out[simd_st_idx + 38  * 8] = r39;
    out[simd_st_idx + 39  * 8] = r40;
    out[simd_st_idx + 40  * 8] = r41;
    out[simd_st_idx + 41  * 8] = r42;
    out[simd_st_idx + 42  * 8] = r43;
    out[simd_st_idx + 43  * 8] = r44;
    out[simd_st_idx + 44  * 8] = r45;
    out[simd_st_idx + 45  * 8] = r46;
    out[simd_st_idx + 46  * 8] = r47;
    out[simd_st_idx + 47  * 8] = r48;
    out[simd_st_idx + 48  * 8] = r49;
    out[simd_st_idx + 49  * 8] = r50;
  }
}

// Copyright 2015 Allan MacKinnon. All rights reserved.

__kernel
void
spillage_ulong_v2(__global const ulong* const __restrict__ in, __global ulong* const out)
{
  ulong r1;
  ulong r2;
  ulong r3;
  ulong r4;
  ulong r5;
  ulong r6;
  ulong r7;
  ulong r8;
  ulong r9;
  ulong r10;
  ulong r11;
  ulong r12;
  ulong r13;
  ulong r14;
  ulong r15;
  ulong r16;
  ulong r17;
  ulong r18;
  ulong r19;
  ulong r20;
  ulong r21;
  ulong r22;
  ulong r23;
  ulong r24;
  ulong r25;
  ulong r26;
  ulong r27;
  ulong r28;
  ulong r29;
  ulong r30;
  ulong r31;
  ulong r32;
  ulong r33;
  ulong r34;
  ulong r35;
  ulong r36;
  ulong r37;
  ulong r38;
  ulong r39;
  ulong r40;
  ulong r41;
  ulong r42;
  ulong r43;
  ulong r44;
  ulong r45;
  ulong r46;
  ulong r47;
  ulong r48;
  ulong r49;
  ulong r50;
  {
    const uint simd_id      = (get_local_size(0) * get_group_id(0) + get_local_id(0)) / 8;
    const uint simd_lane_id = get_local_id(0) & 7;
    const uint simd_ld_idx  = simd_id * 400 + simd_lane_id;
    r1   = in[simd_ld_idx + 0   * 8];
    r2   = in[simd_ld_idx + 1   * 8];
    r3   = in[simd_ld_idx + 2   * 8];
    r4   = in[simd_ld_idx + 3   * 8];
    r5   = in[simd_ld_idx + 4   * 8];
    r6   = in[simd_ld_idx + 5   * 8];
    r7   = in[simd_ld_idx + 6   * 8];
    r8   = in[simd_ld_idx + 7   * 8];
    r9   = in[simd_ld_idx + 8   * 8];
    r10  = in[simd_ld_idx + 9   * 8];
    r11  = in[simd_ld_idx + 10  * 8];
    r12  = in[simd_ld_idx + 11  * 8];
    r13  = in[simd_ld_idx + 12  * 8];
    r14  = in[simd_ld_idx + 13  * 8];
    r15  = in[simd_ld_idx + 14  * 8];
    r16  = in[simd_ld_idx + 15  * 8];
    r17  = in[simd_ld_idx + 16  * 8];
    r18  = in[simd_ld_idx + 17  * 8];
    r19  = in[simd_ld_idx + 18  * 8];
    r20  = in[simd_ld_idx + 19  * 8];
    r21  = in[simd_ld_idx + 20  * 8];
    r22  = in[simd_ld_idx + 21  * 8];
    r23  = in[simd_ld_idx + 22  * 8];
    r24  = in[simd_ld_idx + 23  * 8];
    r25  = in[simd_ld_idx + 24  * 8];
    r26  = in[simd_ld_idx + 25  * 8];
    r27  = in[simd_ld_idx + 26  * 8];
    r28  = in[simd_ld_idx + 27  * 8];
    r29  = in[simd_ld_idx + 28  * 8];
    r30  = in[simd_ld_idx + 29  * 8];
    r31  = in[simd_ld_idx + 30  * 8];
    r32  = in[simd_ld_idx + 31  * 8];
    r33  = in[simd_ld_idx + 32  * 8];
    r34  = in[simd_ld_idx + 33  * 8];
    r35  = in[simd_ld_idx + 34  * 8];
    r36  = in[simd_ld_idx + 35  * 8];
    r37  = in[simd_ld_idx + 36  * 8];
    r38  = in[simd_ld_idx + 37  * 8];
    r39  = in[simd_ld_idx + 38  * 8];
    r40  = in[simd_ld_idx + 39  * 8];
    r41  = in[simd_ld_idx + 40  * 8];
    r42  = in[simd_ld_idx + 41  * 8];
    r43  = in[simd_ld_idx + 42  * 8];
    r44  = in[simd_ld_idx + 43  * 8];
    r45  = in[simd_ld_idx + 44  * 8];
    r46  = in[simd_ld_idx + 45  * 8];
    r47  = in[simd_ld_idx + 46  * 8];
    r48  = in[simd_ld_idx + 47  * 8];
    r49  = in[simd_ld_idx + 48  * 8];
    r50  = in[simd_ld_idx + 49  * 8];
  }
  {
    const ulong t = min(r1  ,r33 );
    r33           = max(r1  ,r33 );
    r1            = t;
  }
  {
    const ulong t = min(r2  ,r34 );
    r34           = max(r2  ,r34 );
    r2            = t;
  }
  {
    const ulong t = min(r3  ,r35 );
    r35           = max(r3  ,r35 );
    r3            = t;
  }
  {
    const ulong t = min(r4  ,r36 );
    r36           = max(r4  ,r36 );
    r4            = t;
  }
  {
    const ulong t = min(r5  ,r37 );
    r37           = max(r5  ,r37 );
    r5            = t;
  }
  {
    const ulong t = min(r6  ,r38 );
    r38           = max(r6  ,r38 );
    r6            = t;
  }
  {
    const ulong t = min(r7  ,r39 );
    r39           = max(r7  ,r39 );
    r7            = t;
  }
  {
    const ulong t = min(r8  ,r40 );
    r40           = max(r8  ,r40 );
    r8            = t;
  }
  {
    const ulong t = min(r9  ,r41 );
    r41           = max(r9  ,r41 );
    r9            = t;
  }
  {
    const ulong t = min(r10 ,r42 );
    r42           = max(r10 ,r42 );
    r10           = t;
  }
  {
    const ulong t = min(r11 ,r43 );
    r43           = max(r11 ,r43 );
    r11           = t;
  }
  {
    const ulong t = min(r12 ,r44 );
    r44           = max(r12 ,r44 );
    r12           = t;
  }
  {
    const ulong t = min(r13 ,r45 );
    r45           = max(r13 ,r45 );
    r13           = t;
  }
  {
    const ulong t = min(r14 ,r46 );
    r46           = max(r14 ,r46 );
    r14           = t;
  }
  {
    const ulong t = min(r15 ,r47 );
    r47           = max(r15 ,r47 );
    r15           = t;
  }
  {
    const ulong t = min(r16 ,r48 );
    r48           = max(r16 ,r48 );
    r16           = t;
  }
  {
    const ulong t = min(r17 ,r49 );
    r49           = max(r17 ,r49 );
    r17           = t;
  }
  {
    const ulong t = min(r18 ,r50 );
    r50           = max(r18 ,r50 );
    r18           = t;
  }
  {
    const ulong t = min(r1  ,r17 );
    r17           = max(r1  ,r17 );
    r1            = t;
  }
  {
    const ulong t = min(r2  ,r18 );
    r18           = max(r2  ,r18 );
    r2            = t;
  }
  {
    const ulong t = min(r3  ,r19 );
    r19           = max(r3  ,r19 );
    r3            = t;
  }
  {
    const ulong t = min(r4  ,r20 );
    r20           = max(r4  ,r20 );
    r4            = t;
  }
  {
    const ulong t = min(r5  ,r21 );
    r21           = max(r5  ,r21 );
    r5            = t;
  }
  {
    const ulong t = min(r6  ,r22 );
    r22           = max(r6  ,r22 );
    r6            = t;
  }
  {
    const ulong t = min(r7  ,r23 );
    r23           = max(r7  ,r23 );
    r7            = t;
  }
  {
    const ulong t = min(r8  ,r24 );
    r24           = max(r8  ,r24 );
    r8            = t;
  }
  {
    const ulong t = min(r9  ,r25 );
    r25           = max(r9  ,r25 );
    r9            = t;
  }
  {
    const ulong t = min(r10 ,r26 );
    r26           = max(r10 ,r26 );
    r10           = t;
  }
  {
    const ulong t = min(r11 ,r27 );
    r27           = max(r11 ,r27 );
    r11           = t;
  }
  {
    const ulong t = min(r12 ,r28 );
    r28           = max(r12 ,r28 );
    r12           = t;
  }
  {
    const ulong t = min(r13 ,r29 );
    r29           = max(r13 ,r29 );
    r13           = t;
  }
  {
    const ulong t = min(r14 ,r30 );
    r30           = max(r14 ,r30 );
    r14           = t;
  }
  {
    const ulong t = min(r15 ,r31 );
    r31           = max(r15 ,r31 );
    r15           = t;
  }
  {
    const ulong t = min(r16 ,r32 );
    r32           = max(r16 ,r32 );
    r16           = t;
  }
  {
    const ulong t = min(r33 ,r49 );
    r49           = max(r33 ,r49 );
    r33           = t;
  }
  {
    const ulong t = min(r34 ,r50 );
    r50           = max(r34 ,r50 );
    r34           = t;
  }
  {
    const ulong t = min(r17 ,r33 );
    r33           = max(r17 ,r33 );
    r17           = t;
  }
  {
    const ulong t = min(r18 ,r34 );
    r34           = max(r18 ,r34 );
    r18           = t;
  }
  {
    const ulong t = min(r19 ,r35 );
    r35           = max(r19 ,r35 );
    r19           = t;
  }
  {
    const ulong t = min(r20 ,r36 );
    r36           = max(r20 ,r36 );
    r20           = t;
  }
  {
    const ulong t = min(r21 ,r37 );
    r37           = max(r21 ,r37 );
    r21           = t;
  }
  {
    const ulong t = min(r22 ,r38 );
    r38           = max(r22 ,r38 );
    r22           = t;
  }
  {
    const ulong t = min(r23 ,r39 );
    r39           = max(r23 ,r39 );
    r23           = t;
  }
  {
    const ulong t = min(r24 ,r40 );
    r40           = max(r24 ,r40 );
    r24           = t;
  }
  {
    const ulong t = min(r25 ,r41 );
    r41           = max(r25 ,r41 );
    r25           = t;
  }
  {
    const ulong t = min(r26 ,r42 );
    r42           = max(r26 ,r42 );
    r26           = t;
  }
  {
    const ulong t = min(r27 ,r43 );
    r43           = max(r27 ,r43 );
    r27           = t;
  }
  {
    const ulong t = min(r28 ,r44 );
    r44           = max(r28 ,r44 );
    r28           = t;
  }
  {
    const ulong t = min(r29 ,r45 );
    r45           = max(r29 ,r45 );
    r29           = t;
  }
  {
    const ulong t = min(r30 ,r46 );
    r46           = max(r30 ,r46 );
    r30           = t;
  }
  {
    const ulong t = min(r31 ,r47 );
    r47           = max(r31 ,r47 );
    r31           = t;
  }
  {
    const ulong t = min(r32 ,r48 );
    r48           = max(r32 ,r48 );
    r32           = t;
  }
  {
    const ulong t = min(r1  ,r9  );
    r9            = max(r1  ,r9  );
    r1            = t;
  }
  {
    const ulong t = min(r2  ,r10 );
    r10           = max(r2  ,r10 );
    r2            = t;
  }
  {
    const ulong t = min(r3  ,r11 );
    r11           = max(r3  ,r11 );
    r3            = t;
  }
  {
    const ulong t = min(r4  ,r12 );
    r12           = max(r4  ,r12 );
    r4            = t;
  }
  {
    const ulong t = min(r5  ,r13 );
    r13           = max(r5  ,r13 );
    r5            = t;
  }
  {
    const ulong t = min(r6  ,r14 );
    r14           = max(r6  ,r14 );
    r6            = t;
  }
  {
    const ulong t = min(r7  ,r15 );
    r15           = max(r7  ,r15 );
    r7            = t;
  }
  {
    const ulong t = min(r8  ,r16 );
    r16           = max(r8  ,r16 );
    r8            = t;
  }
  {
    const ulong t = min(r17 ,r25 );
    r25           = max(r17 ,r25 );
    r17           = t;
  }
  {
    const ulong t = min(r18 ,r26 );
    r26           = max(r18 ,r26 );
    r18           = t;
  }
  {
    const ulong t = min(r19 ,r27 );
    r27           = max(r19 ,r27 );
    r19           = t;
  }
  {
    const ulong t = min(r20 ,r28 );
    r28           = max(r20 ,r28 );
    r20           = t;
  }
  {
    const ulong t = min(r21 ,r29 );
    r29           = max(r21 ,r29 );
    r21           = t;
  }
  {
    const ulong t = min(r22 ,r30 );
    r30           = max(r22 ,r30 );
    r22           = t;
  }
  {
    const ulong t = min(r23 ,r31 );
    r31           = max(r23 ,r31 );
    r23           = t;
  }
  {
    const ulong t = min(r24 ,r32 );
    r32           = max(r24 ,r32 );
    r24           = t;
  }
  {
    const ulong t = min(r33 ,r41 );
    r41           = max(r33 ,r41 );
    r33           = t;
  }
  {
    const ulong t = min(r34 ,r42 );
    r42           = max(r34 ,r42 );
    r34           = t;
  }
  {
    const ulong t = min(r35 ,r43 );
    r43           = max(r35 ,r43 );
    r35           = t;
  }
  {
    const ulong t = min(r36 ,r44 );
    r44           = max(r36 ,r44 );
    r36           = t;
  }
  {
    const ulong t = min(r37 ,r45 );
    r45           = max(r37 ,r45 );
    r37           = t;
  }
  {
    const ulong t = min(r38 ,r46 );
    r46           = max(r38 ,r46 );
    r38           = t;
  }
  {
    const ulong t = min(r39 ,r47 );
    r47           = max(r39 ,r47 );
    r39           = t;
  }
  {
    const ulong t = min(r40 ,r48 );
    r48           = max(r40 ,r48 );
    r40           = t;
  }
  {
    const ulong t = min(r9  ,r33 );
    r33           = max(r9  ,r33 );
    r9            = t;
  }
  {
    const ulong t = min(r10 ,r34 );
    r34           = max(r10 ,r34 );
    r10           = t;
  }
  {
    const ulong t = min(r11 ,r35 );
    r35           = max(r11 ,r35 );
    r11           = t;
  }
  {
    const ulong t = min(r12 ,r36 );
    r36           = max(r12 ,r36 );
    r12           = t;
  }
  {
    const ulong t = min(r13 ,r37 );
    r37           = max(r13 ,r37 );
    r13           = t;
  }
  {
    const ulong t = min(r14 ,r38 );
    r38           = max(r14 ,r38 );
    r14           = t;
  }
  {
    const ulong t = min(r15 ,r39 );
    r39           = max(r15 ,r39 );
    r15           = t;
  }
  {
    const ulong t = min(r16 ,r40 );
    r40           = max(r16 ,r40 );
    r16           = t;
  }
  {
    const ulong t = min(r25 ,r49 );
    r49           = max(r25 ,r49 );
    r25           = t;
  }
  {
    const ulong t = min(r26 ,r50 );
    r50           = max(r26 ,r50 );
    r26           = t;
  }
  {
    const ulong t = min(r9  ,r17 );
    r17           = max(r9  ,r17 );
    r9            = t;
  }
  {
    const ulong t = min(r10 ,r18 );
    r18           = max(r10 ,r18 );
    r10           = t;
  }
  {
    const ulong t = min(r11 ,r19 );
    r19           = max(r11 ,r19 );
    r11           = t;
  }
  {
    const ulong t = min(r12 ,r20 );
    r20           = max(r12 ,r20 );
    r12           = t;
  }
  {
    const ulong t = min(r13 ,r21 );
    r21           = max(r13 ,r21 );
    r13           = t;
  }
  {
    const ulong t = min(r14 ,r22 );
    r22           = max(r14 ,r22 );
    r14           = t;
  }
  {
    const ulong t = min(r15 ,r23 );
    r23           = max(r15 ,r23 );
    r15           = t;
  }
  {
    const ulong t = min(r16 ,r24 );
    r24           = max(r16 ,r24 );
    r16           = t;
  }
  {
    const ulong t = min(r25 ,r33 );
    r33           = max(r25 ,r33 );
    r25           = t;
  }
  {
    const ulong t = min(r26 ,r34 );
    r34           = max(r26 ,r34 );
    r26           = t;
  }
  {
    const ulong t = min(r27 ,r35 );
    r35           = max(r27 ,r35 );
    r27           = t;
  }
  {
    const ulong t = min(r28 ,r36 );
    r36           = max(r28 ,r36 );
    r28           = t;
  }
  {
    const ulong t = min(r29 ,r37 );
    r37           = max(r29 ,r37 );
    r29           = t;
  }
  {
    const ulong t = min(r30 ,r38 );
    r38           = max(r30 ,r38 );
    r30           = t;
  }
  {
    const ulong t = min(r31 ,r39 );
    r39           = max(r31 ,r39 );
    r31           = t;
  }
  {
    const ulong t = min(r32 ,r40 );
    r40           = max(r32 ,r40 );
    r32           = t;
  }
  {
    const ulong t = min(r41 ,r49 );
    r49           = max(r41 ,r49 );
    r41           = t;
  }
  {
    const ulong t = min(r42 ,r50 );
    r50           = max(r42 ,r50 );
    r42           = t;
  }
  {
    const ulong t = min(r1  ,r5  );
    r5            = max(r1  ,r5  );
    r1            = t;
  }
  {
    const ulong t = min(r2  ,r6  );
    r6            = max(r2  ,r6  );
    r2            = t;
  }
  {
    const ulong t = min(r3  ,r7  );
    r7            = max(r3  ,r7  );
    r3            = t;
  }
  {
    const ulong t = min(r4  ,r8  );
    r8            = max(r4  ,r8  );
    r4            = t;
  }
  {
    const ulong t = min(r9  ,r13 );
    r13           = max(r9  ,r13 );
    r9            = t;
  }
  {
    const ulong t = min(r10 ,r14 );
    r14           = max(r10 ,r14 );
    r10           = t;
  }
  {
    const ulong t = min(r11 ,r15 );
    r15           = max(r11 ,r15 );
    r11           = t;
  }
  {
    const ulong t = min(r12 ,r16 );
    r16           = max(r12 ,r16 );
    r12           = t;
  }
  {
    const ulong t = min(r17 ,r21 );
    r21           = max(r17 ,r21 );
    r17           = t;
  }
  {
    const ulong t = min(r18 ,r22 );
    r22           = max(r18 ,r22 );
    r18           = t;
  }
  {
    const ulong t = min(r19 ,r23 );
    r23           = max(r19 ,r23 );
    r19           = t;
  }
  {
    const ulong t = min(r20 ,r24 );
    r24           = max(r20 ,r24 );
    r20           = t;
  }
  {
    const ulong t = min(r25 ,r29 );
    r29           = max(r25 ,r29 );
    r25           = t;
  }
  {
    const ulong t = min(r26 ,r30 );
    r30           = max(r26 ,r30 );
    r26           = t;
  }
  {
    const ulong t = min(r27 ,r31 );
    r31           = max(r27 ,r31 );
    r27           = t;
  }
  {
    const ulong t = min(r28 ,r32 );
    r32           = max(r28 ,r32 );
    r28           = t;
  }
  {
    const ulong t = min(r33 ,r37 );
    r37           = max(r33 ,r37 );
    r33           = t;
  }
  {
    const ulong t = min(r34 ,r38 );
    r38           = max(r34 ,r38 );
    r34           = t;
  }
  {
    const ulong t = min(r35 ,r39 );
    r39           = max(r35 ,r39 );
    r35           = t;
  }
  {
    const ulong t = min(r36 ,r40 );
    r40           = max(r36 ,r40 );
    r36           = t;
  }
  {
    const ulong t = min(r41 ,r45 );
    r45           = max(r41 ,r45 );
    r41           = t;
  }
  {
    const ulong t = min(r42 ,r46 );
    r46           = max(r42 ,r46 );
    r42           = t;
  }
  {
    const ulong t = min(r43 ,r47 );
    r47           = max(r43 ,r47 );
    r43           = t;
  }
  {
    const ulong t = min(r44 ,r48 );
    r48           = max(r44 ,r48 );
    r44           = t;
  }
  {
    const ulong t = min(r5  ,r33 );
    r33           = max(r5  ,r33 );
    r5            = t;
  }
  {
    const ulong t = min(r6  ,r34 );
    r34           = max(r6  ,r34 );
    r6            = t;
  }
  {
    const ulong t = min(r7  ,r35 );
    r35           = max(r7  ,r35 );
    r7            = t;
  }
  {
    const ulong t = min(r8  ,r36 );
    r36           = max(r8  ,r36 );
    r8            = t;
  }
  {
    const ulong t = min(r13 ,r41 );
    r41           = max(r13 ,r41 );
    r13           = t;
  }
  {
    const ulong t = min(r14 ,r42 );
    r42           = max(r14 ,r42 );
    r14           = t;
  }
  {
    const ulong t = min(r15 ,r43 );
    r43           = max(r15 ,r43 );
    r15           = t;
  }
  {
    const ulong t = min(r16 ,r44 );
    r44           = max(r16 ,r44 );
    r16           = t;
  }
  {
    const ulong t = min(r21 ,r49 );
    r49           = max(r21 ,r49 );
    r21           = t;
  }
  {
    const ulong t = min(r22 ,r50 );
    r50           = max(r22 ,r50 );
    r22           = t;
  }
  {
    const ulong t = min(r5  ,r17 );
    r17           = max(r5  ,r17 );
    r5            = t;
  }
  {
    const ulong t = min(r6  ,r18 );
    r18           = max(r6  ,r18 );
    r6            = t;
  }
  {
    const ulong t = min(r7  ,r19 );
    r19           = max(r7  ,r19 );
    r7            = t;
  }
  {
    const ulong t = min(r8  ,r20 );
    r20           = max(r8  ,r20 );
    r8            = t;
  }
  {
    const ulong t = min(r13 ,r25 );
    r25           = max(r13 ,r25 );
    r13           = t;
  }
  {
    const ulong t = min(r14 ,r26 );
    r26           = max(r14 ,r26 );
    r14           = t;
  }
  {
    const ulong t = min(r15 ,r27 );
    r27           = max(r15 ,r27 );
    r15           = t;
  }
  {
    const ulong t = min(r16 ,r28 );
    r28           = max(r16 ,r28 );
    r16           = t;
  }
  {
    const ulong t = min(r21 ,r33 );
    r33           = max(r21 ,r33 );
    r21           = t;
  }
  {
    const ulong t = min(r22 ,r34 );
    r34           = max(r22 ,r34 );
    r22           = t;
  }
  {
    const ulong t = min(r23 ,r35 );
    r35           = max(r23 ,r35 );
    r23           = t;
  }
  {
    const ulong t = min(r24 ,r36 );
    r36           = max(r24 ,r36 );
    r24           = t;
  }
  {
    const ulong t = min(r29 ,r41 );
    r41           = max(r29 ,r41 );
    r29           = t;
  }
  {
    const ulong t = min(r30 ,r42 );
    r42           = max(r30 ,r42 );
    r30           = t;
  }
  {
    const ulong t = min(r31 ,r43 );
    r43           = max(r31 ,r43 );
    r31           = t;
  }
  {
    const ulong t = min(r32 ,r44 );
    r44           = max(r32 ,r44 );
    r32           = t;
  }
  {
    const ulong t = min(r37 ,r49 );
    r49           = max(r37 ,r49 );
    r37           = t;
  }
  {
    const ulong t = min(r38 ,r50 );
    r50           = max(r38 ,r50 );
    r38           = t;
  }
  {
    const ulong t = min(r5  ,r9  );
    r9            = max(r5  ,r9  );
    r5            = t;
  }
  {
    const ulong t = min(r6  ,r10 );
    r10           = max(r6  ,r10 );
    r6            = t;
  }
  {
    const ulong t = min(r7  ,r11 );
    r11           = max(r7  ,r11 );
    r7            = t;
  }
  {
    const ulong t = min(r8  ,r12 );
    r12           = max(r8  ,r12 );
    r8            = t;
  }
  {
    const ulong t = min(r13 ,r17 );
    r17           = max(r13 ,r17 );
    r13           = t;
  }
  {
    const ulong t = min(r14 ,r18 );
    r18           = max(r14 ,r18 );
    r14           = t;
  }
  {
    const ulong t = min(r15 ,r19 );
    r19           = max(r15 ,r19 );
    r15           = t;
  }
  {
    const ulong t = min(r16 ,r20 );
    r20           = max(r16 ,r20 );
    r16           = t;
  }
  {
    const ulong t = min(r21 ,r25 );
    r25           = max(r21 ,r25 );
    r21           = t;
  }
  {
    const ulong t = min(r22 ,r26 );
    r26           = max(r22 ,r26 );
    r22           = t;
  }
  {
    const ulong t = min(r23 ,r27 );
    r27           = max(r23 ,r27 );
    r23           = t;
  }
  {
    const ulong t = min(r24 ,r28 );
    r28           = max(r24 ,r28 );
    r24           = t;
  }
  {
    const ulong t = min(r29 ,r33 );
    r33           = max(r29 ,r33 );
    r29           = t;
  }
  {
    const ulong t = min(r30 ,r34 );
    r34           = max(r30 ,r34 );
    r30           = t;
  }
  {
    const ulong t = min(r31 ,r35 );
    r35           = max(r31 ,r35 );
    r31           = t;
  }
  {
    const ulong t = min(r32 ,r36 );
    r36           = max(r32 ,r36 );
    r32           = t;
  }
  {
    const ulong t = min(r37 ,r41 );
    r41           = max(r37 ,r41 );
    r37           = t;
  }
  {
    const ulong t = min(r38 ,r42 );
    r42           = max(r38 ,r42 );
    r38           = t;
  }
  {
    const ulong t = min(r39 ,r43 );
    r43           = max(r39 ,r43 );
    r39           = t;
  }
  {
    const ulong t = min(r40 ,r44 );
    r44           = max(r40 ,r44 );
    r40           = t;
  }
  {
    const ulong t = min(r45 ,r49 );
    r49           = max(r45 ,r49 );
    r45           = t;
  }
  {
    const ulong t = min(r46 ,r50 );
    r50           = max(r46 ,r50 );
    r46           = t;
  }
  {
    const ulong t = min(r1  ,r3  );
    r3            = max(r1  ,r3  );
    r1            = t;
  }
  {
    const ulong t = min(r2  ,r4  );
    r4            = max(r2  ,r4  );
    r2            = t;
  }
  {
    const ulong t = min(r5  ,r7  );
    r7            = max(r5  ,r7  );
    r5            = t;
  }
  {
    const ulong t = min(r6  ,r8  );
    r8            = max(r6  ,r8  );
    r6            = t;
  }
  {
    const ulong t = min(r9  ,r11 );
    r11           = max(r9  ,r11 );
    r9            = t;
  }
  {
    const ulong t = min(r10 ,r12 );
    r12           = max(r10 ,r12 );
    r10           = t;
  }
  {
    const ulong t = min(r13 ,r15 );
    r15           = max(r13 ,r15 );
    r13           = t;
  }
  {
    const ulong t = min(r14 ,r16 );
    r16           = max(r14 ,r16 );
    r14           = t;
  }
  {
    const ulong t = min(r17 ,r19 );
    r19           = max(r17 ,r19 );
    r17           = t;
  }
  {
    const ulong t = min(r18 ,r20 );
    r20           = max(r18 ,r20 );
    r18           = t;
  }
  {
    const ulong t = min(r21 ,r23 );
    r23           = max(r21 ,r23 );
    r21           = t;
  }
  {
    const ulong t = min(r22 ,r24 );
    r24           = max(r22 ,r24 );
    r22           = t;
  }
  {
    const ulong t = min(r25 ,r27 );
    r27           = max(r25 ,r27 );
    r25           = t;
  }
  {
    const ulong t = min(r26 ,r28 );
    r28           = max(r26 ,r28 );
    r26           = t;
  }
  {
    const ulong t = min(r29 ,r31 );
    r31           = max(r29 ,r31 );
    r29           = t;
  }
  {
    const ulong t = min(r30 ,r32 );
    r32           = max(r30 ,r32 );
    r30           = t;
  }
  {
    const ulong t = min(r33 ,r35 );
    r35           = max(r33 ,r35 );
    r33           = t;
  }
  {
    const ulong t = min(r34 ,r36 );
    r36           = max(r34 ,r36 );
    r34           = t;
  }
  {
    const ulong t = min(r37 ,r39 );
    r39           = max(r37 ,r39 );
    r37           = t;
  }
  {
    const ulong t = min(r38 ,r40 );
    r40           = max(r38 ,r40 );
    r38           = t;
  }
  {
    const ulong t = min(r41 ,r43 );
    r43           = max(r41 ,r43 );
    r41           = t;
  }
  {
    const ulong t = min(r42 ,r44 );
    r44           = max(r42 ,r44 );
    r42           = t;
  }
  {
    const ulong t = min(r45 ,r47 );
    r47           = max(r45 ,r47 );
    r45           = t;
  }
  {
    const ulong t = min(r46 ,r48 );
    r48           = max(r46 ,r48 );
    r46           = t;
  }
  {
    const ulong t = min(r3  ,r33 );
    r33           = max(r3  ,r33 );
    r3            = t;
  }
  {
    const ulong t = min(r4  ,r34 );
    r34           = max(r4  ,r34 );
    r4            = t;
  }
  {
    const ulong t = min(r7  ,r37 );
    r37           = max(r7  ,r37 );
    r7            = t;
  }
  {
    const ulong t = min(r8  ,r38 );
    r38           = max(r8  ,r38 );
    r8            = t;
  }
  {
    const ulong t = min(r11 ,r41 );
    r41           = max(r11 ,r41 );
    r11           = t;
  }
  {
    const ulong t = min(r12 ,r42 );
    r42           = max(r12 ,r42 );
    r12           = t;
  }
  {
    const ulong t = min(r15 ,r45 );
    r45           = max(r15 ,r45 );
    r15           = t;
  }
  {
    const ulong t = min(r16 ,r46 );
    r46           = max(r16 ,r46 );
    r16           = t;
  }
  {
    const ulong t = min(r19 ,r49 );
    r49           = max(r19 ,r49 );
    r19           = t;
  }
  {
    const ulong t = min(r20 ,r50 );
    r50           = max(r20 ,r50 );
    r20           = t;
  }
  {
    const ulong t = min(r3  ,r17 );
    r17           = max(r3  ,r17 );
    r3            = t;
  }
  {
    const ulong t = min(r4  ,r18 );
    r18           = max(r4  ,r18 );
    r4            = t;
  }
  {
    const ulong t = min(r7  ,r21 );
    r21           = max(r7  ,r21 );
    r7            = t;
  }
  {
    const ulong t = min(r8  ,r22 );
    r22           = max(r8  ,r22 );
    r8            = t;
  }
  {
    const ulong t = min(r11 ,r25 );
    r25           = max(r11 ,r25 );
    r11           = t;
  }
  {
    const ulong t = min(r12 ,r26 );
    r26           = max(r12 ,r26 );
    r12           = t;
  }
  {
    const ulong t = min(r15 ,r29 );
    r29           = max(r15 ,r29 );
    r15           = t;
  }
  {
    const ulong t = min(r16 ,r30 );
    r30           = max(r16 ,r30 );
    r16           = t;
  }
  {
    const ulong t = min(r19 ,r33 );
    r33           = max(r19 ,r33 );
    r19           = t;
  }
  {
    const ulong t = min(r20 ,r34 );
    r34           = max(r20 ,r34 );
    r20           = t;
  }
  {
    const ulong t = min(r23 ,r37 );
    r37           = max(r23 ,r37 );
    r23           = t;
  }
  {
    const ulong t = min(r24 ,r38 );
    r38           = max(r24 ,r38 );
    r24           = t;
  }
  {
    const ulong t = min(r27 ,r41 );
    r41           = max(r27 ,r41 );
    r27           = t;
  }
  {
    const ulong t = min(r28 ,r42 );
    r42           = max(r28 ,r42 );
    r28           = t;
  }
  {
    const ulong t = min(r31 ,r45 );
    r45           = max(r31 ,r45 );
    r31           = t;
  }
  {
    const ulong t = min(r32 ,r46 );
    r46           = max(r32 ,r46 );
    r32           = t;
  }
  {
    const ulong t = min(r35 ,r49 );
    r49           = max(r35 ,r49 );
    r35           = t;
  }
  {
    const ulong t = min(r36 ,r50 );
    r50           = max(r36 ,r50 );
    r36           = t;
  }
  {
    const ulong t = min(r3  ,r9  );
    r9            = max(r3  ,r9  );
    r3            = t;
  }
  {
    const ulong t = min(r4  ,r10 );
    r10           = max(r4  ,r10 );
    r4            = t;
  }
  {
    const ulong t = min(r7  ,r13 );
    r13           = max(r7  ,r13 );
    r7            = t;
  }
  {
    const ulong t = min(r8  ,r14 );
    r14           = max(r8  ,r14 );
    r8            = t;
  }
  {
    const ulong t = min(r11 ,r17 );
    r17           = max(r11 ,r17 );
    r11           = t;
  }
  {
    const ulong t = min(r12 ,r18 );
    r18           = max(r12 ,r18 );
    r12           = t;
  }
  {
    const ulong t = min(r15 ,r21 );
    r21           = max(r15 ,r21 );
    r15           = t;
  }
  {
    const ulong t = min(r16 ,r22 );
    r22           = max(r16 ,r22 );
    r16           = t;
  }
  {
    const ulong t = min(r19 ,r25 );
    r25           = max(r19 ,r25 );
    r19           = t;
  }
  {
    const ulong t = min(r20 ,r26 );
    r26           = max(r20 ,r26 );
    r20           = t;
  }
  {
    const ulong t = min(r23 ,r29 );
    r29           = max(r23 ,r29 );
    r23           = t;
  }
  {
    const ulong t = min(r24 ,r30 );
    r30           = max(r24 ,r30 );
    r24           = t;
  }
  {
    const ulong t = min(r27 ,r33 );
    r33           = max(r27 ,r33 );
    r27           = t;
  }
  {
    const ulong t = min(r28 ,r34 );
    r34           = max(r28 ,r34 );
    r28           = t;
  }
  {
    const ulong t = min(r31 ,r37 );
    r37           = max(r31 ,r37 );
    r31           = t;
  }
  {
    const ulong t = min(r32 ,r38 );
    r38           = max(r32 ,r38 );
    r32           = t;
  }
  {
    const ulong t = min(r35 ,r41 );
    r41           = max(r35 ,r41 );
    r35           = t;
  }
  {
    const ulong t = min(r36 ,r42 );
    r42           = max(r36 ,r42 );
    r36           = t;
  }
  {
    const ulong t = min(r39 ,r45 );
    r45           = max(r39 ,r45 );
    r39           = t;
  }
  {
    const ulong t = min(r40 ,r46 );
    r46           = max(r40 ,r46 );
    r40           = t;
  }
  {
    const ulong t = min(r43 ,r49 );
    r49           = max(r43 ,r49 );
    r43           = t;
  }
  {
    const ulong t = min(r44 ,r50 );
    r50           = max(r44 ,r50 );
    r44           = t;
  }
  {
    const ulong t = min(r3  ,r5  );
    r5            = max(r3  ,r5  );
    r3            = t;
  }
  {
    const ulong t = min(r4  ,r6  );
    r6            = max(r4  ,r6  );
    r4            = t;
  }
  {
    const ulong t = min(r7  ,r9  );
    r9            = max(r7  ,r9  );
    r7            = t;
  }
  {
    const ulong t = min(r8  ,r10 );
    r10           = max(r8  ,r10 );
    r8            = t;
  }
  {
    const ulong t = min(r11 ,r13 );
    r13           = max(r11 ,r13 );
    r11           = t;
  }
  {
    const ulong t = min(r12 ,r14 );
    r14           = max(r12 ,r14 );
    r12           = t;
  }
  {
    const ulong t = min(r15 ,r17 );
    r17           = max(r15 ,r17 );
    r15           = t;
  }
  {
    const ulong t = min(r16 ,r18 );
    r18           = max(r16 ,r18 );
    r16           = t;
  }
  {
    const ulong t = min(r19 ,r21 );
    r21           = max(r19 ,r21 );
    r19           = t;
  }
  {
    const ulong t = min(r20 ,r22 );
    r22           = max(r20 ,r22 );
    r20           = t;
  }
  {
    const ulong t = min(r23 ,r25 );
    r25           = max(r23 ,r25 );
    r23           = t;
  }
  {
    const ulong t = min(r24 ,r26 );
    r26           = max(r24 ,r26 );
    r24           = t;
  }
  {
    const ulong t = min(r27 ,r29 );
    r29           = max(r27 ,r29 );
    r27           = t;
  }
  {
    const ulong t = min(r28 ,r30 );
    r30           = max(r28 ,r30 );
    r28           = t;
  }
  {
    const ulong t = min(r31 ,r33 );
    r33           = max(r31 ,r33 );
    r31           = t;
  }
  {
    const ulong t = min(r32 ,r34 );
    r34           = max(r32 ,r34 );
    r32           = t;
  }
  {
    const ulong t = min(r35 ,r37 );
    r37           = max(r35 ,r37 );
    r35           = t;
  }
  {
    const ulong t = min(r36 ,r38 );
    r38           = max(r36 ,r38 );
    r36           = t;
  }
  {
    const ulong t = min(r39 ,r41 );
    r41           = max(r39 ,r41 );
    r39           = t;
  }
  {
    const ulong t = min(r40 ,r42 );
    r42           = max(r40 ,r42 );
    r40           = t;
  }
  {
    const ulong t = min(r43 ,r45 );
    r45           = max(r43 ,r45 );
    r43           = t;
  }
  {
    const ulong t = min(r44 ,r46 );
    r46           = max(r44 ,r46 );
    r44           = t;
  }
  {
    const ulong t = min(r47 ,r49 );
    r49           = max(r47 ,r49 );
    r47           = t;
  }
  {
    const ulong t = min(r48 ,r50 );
    r50           = max(r48 ,r50 );
    r48           = t;
  }
  {
    const ulong t = min(r1  ,r2  );
    r2            = max(r1  ,r2  );
    r1            = t;
  }
  {
    const ulong t = min(r3  ,r4  );
    r4            = max(r3  ,r4  );
    r3            = t;
  }
  {
    const ulong t = min(r5  ,r6  );
    r6            = max(r5  ,r6  );
    r5            = t;
  }
  {
    const ulong t = min(r7  ,r8  );
    r8            = max(r7  ,r8  );
    r7            = t;
  }
  {
    const ulong t = min(r9  ,r10 );
    r10           = max(r9  ,r10 );
    r9            = t;
  }
  {
    const ulong t = min(r11 ,r12 );
    r12           = max(r11 ,r12 );
    r11           = t;
  }
  {
    const ulong t = min(r13 ,r14 );
    r14           = max(r13 ,r14 );
    r13           = t;
  }
  {
    const ulong t = min(r15 ,r16 );
    r16           = max(r15 ,r16 );
    r15           = t;
  }
  {
    const ulong t = min(r17 ,r18 );
    r18           = max(r17 ,r18 );
    r17           = t;
  }
  {
    const ulong t = min(r19 ,r20 );
    r20           = max(r19 ,r20 );
    r19           = t;
  }
  {
    const ulong t = min(r21 ,r22 );
    r22           = max(r21 ,r22 );
    r21           = t;
  }
  {
    const ulong t = min(r23 ,r24 );
    r24           = max(r23 ,r24 );
    r23           = t;
  }
  {
    const ulong t = min(r25 ,r26 );
    r26           = max(r25 ,r26 );
    r25           = t;
  }
  {
    const ulong t = min(r27 ,r28 );
    r28           = max(r27 ,r28 );
    r27           = t;
  }
  {
    const ulong t = min(r29 ,r30 );
    r30           = max(r29 ,r30 );
    r29           = t;
  }
  {
    const ulong t = min(r31 ,r32 );
    r32           = max(r31 ,r32 );
    r31           = t;
  }
  {
    const ulong t = min(r33 ,r34 );
    r34           = max(r33 ,r34 );
    r33           = t;
  }
  {
    const ulong t = min(r35 ,r36 );
    r36           = max(r35 ,r36 );
    r35           = t;
  }
  {
    const ulong t = min(r37 ,r38 );
    r38           = max(r37 ,r38 );
    r37           = t;
  }
  {
    const ulong t = min(r39 ,r40 );
    r40           = max(r39 ,r40 );
    r39           = t;
  }
  {
    const ulong t = min(r41 ,r42 );
    r42           = max(r41 ,r42 );
    r41           = t;
  }
  {
    const ulong t = min(r43 ,r44 );
    r44           = max(r43 ,r44 );
    r43           = t;
  }
  {
    const ulong t = min(r45 ,r46 );
    r46           = max(r45 ,r46 );
    r45           = t;
  }
  {
    const ulong t = min(r47 ,r48 );
    r48           = max(r47 ,r48 );
    r47           = t;
  }
  {
    const ulong t = min(r49 ,r50 );
    r50           = max(r49 ,r50 );
    r49           = t;
  }
  {
    const ulong t = min(r2  ,r33 );
    r33           = max(r2  ,r33 );
    r2            = t;
  }
  {
    const ulong t = min(r4  ,r35 );
    r35           = max(r4  ,r35 );
    r4            = t;
  }
  {
    const ulong t = min(r6  ,r37 );
    r37           = max(r6  ,r37 );
    r6            = t;
  }
  {
    const ulong t = min(r8  ,r39 );
    r39           = max(r8  ,r39 );
    r8            = t;
  }
  {
    const ulong t = min(r10 ,r41 );
    r41           = max(r10 ,r41 );
    r10           = t;
  }
  {
    const ulong t = min(r12 ,r43 );
    r43           = max(r12 ,r43 );
    r12           = t;
  }
  {
    const ulong t = min(r14 ,r45 );
    r45           = max(r14 ,r45 );
    r14           = t;
  }
  {
    const ulong t = min(r16 ,r47 );
    r47           = max(r16 ,r47 );
    r16           = t;
  }
  {
    const ulong t = min(r18 ,r49 );
    r49           = max(r18 ,r49 );
    r18           = t;
  }
  {
    const ulong t = min(r2  ,r17 );
    r17           = max(r2  ,r17 );
    r2            = t;
  }
  {
    const ulong t = min(r4  ,r19 );
    r19           = max(r4  ,r19 );
    r4            = t;
  }
  {
    const ulong t = min(r6  ,r21 );
    r21           = max(r6  ,r21 );
    r6            = t;
  }
  {
    const ulong t = min(r8  ,r23 );
    r23           = max(r8  ,r23 );
    r8            = t;
  }
  {
    const ulong t = min(r10 ,r25 );
    r25           = max(r10 ,r25 );
    r10           = t;
  }
  {
    const ulong t = min(r12 ,r27 );
    r27           = max(r12 ,r27 );
    r12           = t;
  }
  {
    const ulong t = min(r14 ,r29 );
    r29           = max(r14 ,r29 );
    r14           = t;
  }
  {
    const ulong t = min(r16 ,r31 );
    r31           = max(r16 ,r31 );
    r16           = t;
  }
  {
    const ulong t = min(r18 ,r33 );
    r33           = max(r18 ,r33 );
    r18           = t;
  }
  {
    const ulong t = min(r20 ,r35 );
    r35           = max(r20 ,r35 );
    r20           = t;
  }
  {
    const ulong t = min(r22 ,r37 );
    r37           = max(r22 ,r37 );
    r22           = t;
  }
  {
    const ulong t = min(r24 ,r39 );
    r39           = max(r24 ,r39 );
    r24           = t;
  }
  {
    const ulong t = min(r26 ,r41 );
    r41           = max(r26 ,r41 );
    r26           = t;
  }
  {
    const ulong t = min(r28 ,r43 );
    r43           = max(r28 ,r43 );
    r28           = t;
  }
  {
    const ulong t = min(r30 ,r45 );
    r45           = max(r30 ,r45 );
    r30           = t;
  }
  {
    const ulong t = min(r32 ,r47 );
    r47           = max(r32 ,r47 );
    r32           = t;
  }
  {
    const ulong t = min(r34 ,r49 );
    r49           = max(r34 ,r49 );
    r34           = t;
  }
  {
    const ulong t = min(r2  ,r9  );
    r9            = max(r2  ,r9  );
    r2            = t;
  }
  {
    const ulong t = min(r4  ,r11 );
    r11           = max(r4  ,r11 );
    r4            = t;
  }
  {
    const ulong t = min(r6  ,r13 );
    r13           = max(r6  ,r13 );
    r6            = t;
  }
  {
    const ulong t = min(r8  ,r15 );
    r15           = max(r8  ,r15 );
    r8            = t;
  }
  {
    const ulong t = min(r10 ,r17 );
    r17           = max(r10 ,r17 );
    r10           = t;
  }
  {
    const ulong t = min(r12 ,r19 );
    r19           = max(r12 ,r19 );
    r12           = t;
  }
  {
    const ulong t = min(r14 ,r21 );
    r21           = max(r14 ,r21 );
    r14           = t;
  }
  {
    const ulong t = min(r16 ,r23 );
    r23           = max(r16 ,r23 );
    r16           = t;
  }
  {
    const ulong t = min(r18 ,r25 );
    r25           = max(r18 ,r25 );
    r18           = t;
  }
  {
    const ulong t = min(r20 ,r27 );
    r27           = max(r20 ,r27 );
    r20           = t;
  }
  {
    const ulong t = min(r22 ,r29 );
    r29           = max(r22 ,r29 );
    r22           = t;
  }
  {
    const ulong t = min(r24 ,r31 );
    r31           = max(r24 ,r31 );
    r24           = t;
  }
  {
    const ulong t = min(r26 ,r33 );
    r33           = max(r26 ,r33 );
    r26           = t;
  }
  {
    const ulong t = min(r28 ,r35 );
    r35           = max(r28 ,r35 );
    r28           = t;
  }
  {
    const ulong t = min(r30 ,r37 );
    r37           = max(r30 ,r37 );
    r30           = t;
  }
  {
    const ulong t = min(r32 ,r39 );
    r39           = max(r32 ,r39 );
    r32           = t;
  }
  {
    const ulong t = min(r34 ,r41 );
    r41           = max(r34 ,r41 );
    r34           = t;
  }
  {
    const ulong t = min(r36 ,r43 );
    r43           = max(r36 ,r43 );
    r36           = t;
  }
  {
    const ulong t = min(r38 ,r45 );
    r45           = max(r38 ,r45 );
    r38           = t;
  }
  {
    const ulong t = min(r40 ,r47 );
    r47           = max(r40 ,r47 );
    r40           = t;
  }
  {
    const ulong t = min(r42 ,r49 );
    r49           = max(r42 ,r49 );
    r42           = t;
  }
  {
    const ulong t = min(r2  ,r5  );
    r5            = max(r2  ,r5  );
    r2            = t;
  }
  {
    const ulong t = min(r4  ,r7  );
    r7            = max(r4  ,r7  );
    r4            = t;
  }
  {
    const ulong t = min(r6  ,r9  );
    r9            = max(r6  ,r9  );
    r6            = t;
  }
  {
    const ulong t = min(r8  ,r11 );
    r11           = max(r8  ,r11 );
    r8            = t;
  }
  {
    const ulong t = min(r10 ,r13 );
    r13           = max(r10 ,r13 );
    r10           = t;
  }
  {
    const ulong t = min(r12 ,r15 );
    r15           = max(r12 ,r15 );
    r12           = t;
  }
  {
    const ulong t = min(r14 ,r17 );
    r17           = max(r14 ,r17 );
    r14           = t;
  }
  {
    const ulong t = min(r16 ,r19 );
    r19           = max(r16 ,r19 );
    r16           = t;
  }
  {
    const ulong t = min(r18 ,r21 );
    r21           = max(r18 ,r21 );
    r18           = t;
  }
  {
    const ulong t = min(r20 ,r23 );
    r23           = max(r20 ,r23 );
    r20           = t;
  }
  {
    const ulong t = min(r22 ,r25 );
    r25           = max(r22 ,r25 );
    r22           = t;
  }
  {
    const ulong t = min(r24 ,r27 );
    r27           = max(r24 ,r27 );
    r24           = t;
  }
  {
    const ulong t = min(r26 ,r29 );
    r29           = max(r26 ,r29 );
    r26           = t;
  }
  {
    const ulong t = min(r28 ,r31 );
    r31           = max(r28 ,r31 );
    r28           = t;
  }
  {
    const ulong t = min(r30 ,r33 );
    r33           = max(r30 ,r33 );
    r30           = t;
  }
  {
    const ulong t = min(r32 ,r35 );
    r35           = max(r32 ,r35 );
    r32           = t;
  }
  {
    const ulong t = min(r34 ,r37 );
    r37           = max(r34 ,r37 );
    r34           = t;
  }
  {
    const ulong t = min(r36 ,r39 );
    r39           = max(r36 ,r39 );
    r36           = t;
  }
  {
    const ulong t = min(r38 ,r41 );
    r41           = max(r38 ,r41 );
    r38           = t;
  }
  {
    const ulong t = min(r40 ,r43 );
    r43           = max(r40 ,r43 );
    r40           = t;
  }
  {
    const ulong t = min(r42 ,r45 );
    r45           = max(r42 ,r45 );
    r42           = t;
  }
  {
    const ulong t = min(r44 ,r47 );
    r47           = max(r44 ,r47 );
    r44           = t;
  }
  {
    const ulong t = min(r46 ,r49 );
    r49           = max(r46 ,r49 );
    r46           = t;
  }
  {
    const ulong t = min(r2  ,r3  );
    r3            = max(r2  ,r3  );
    r2            = t;
  }
  {
    const ulong t = min(r4  ,r5  );
    r5            = max(r4  ,r5  );
    r4            = t;
  }
  {
    const ulong t = min(r6  ,r7  );
    r7            = max(r6  ,r7  );
    r6            = t;
  }
  {
    const ulong t = min(r8  ,r9  );
    r9            = max(r8  ,r9  );
    r8            = t;
  }
  {
    const ulong t = min(r10 ,r11 );
    r11           = max(r10 ,r11 );
    r10           = t;
  }
  {
    const ulong t = min(r12 ,r13 );
    r13           = max(r12 ,r13 );
    r12           = t;
  }
  {
    const ulong t = min(r14 ,r15 );
    r15           = max(r14 ,r15 );
    r14           = t;
  }
  {
    const ulong t = min(r16 ,r17 );
    r17           = max(r16 ,r17 );
    r16           = t;
  }
  {
    const ulong t = min(r18 ,r19 );
    r19           = max(r18 ,r19 );
    r18           = t;
  }
  {
    const ulong t = min(r20 ,r21 );
    r21           = max(r20 ,r21 );
    r20           = t;
  }
  {
    const ulong t = min(r22 ,r23 );
    r23           = max(r22 ,r23 );
    r22           = t;
  }
  {
    const ulong t = min(r24 ,r25 );
    r25           = max(r24 ,r25 );
    r24           = t;
  }
  {
    const ulong t = min(r26 ,r27 );
    r27           = max(r26 ,r27 );
    r26           = t;
  }
  {
    const ulong t = min(r28 ,r29 );
    r29           = max(r28 ,r29 );
    r28           = t;
  }
  {
    const ulong t = min(r30 ,r31 );
    r31           = max(r30 ,r31 );
    r30           = t;
  }
  {
    const ulong t = min(r32 ,r33 );
    r33           = max(r32 ,r33 );
    r32           = t;
  }
  {
    const ulong t = min(r34 ,r35 );
    r35           = max(r34 ,r35 );
    r34           = t;
  }
  {
    const ulong t = min(r36 ,r37 );
    r37           = max(r36 ,r37 );
    r36           = t;
  }
  {
    const ulong t = min(r38 ,r39 );
    r39           = max(r38 ,r39 );
    r38           = t;
  }
  {
    const ulong t = min(r40 ,r41 );
    r41           = max(r40 ,r41 );
    r40           = t;
  }
  {
    const ulong t = min(r42 ,r43 );
    r43           = max(r42 ,r43 );
    r42           = t;
  }
  {
    const ulong t = min(r44 ,r45 );
    r45           = max(r44 ,r45 );
    r44           = t;
  }
  {
    const ulong t = min(r46 ,r47 );
    r47           = max(r46 ,r47 );
    r46           = t;
  }
  {
    const ulong t = min(r48 ,r49 );
    r49           = max(r48 ,r49 );
    r48           = t;
  }
  {
    const uint simd_id      = (get_local_size(0) * get_group_id(0) + get_local_id(0)) / 8;
    const uint simd_lane_id = get_local_id(0) & 7;
    const uint simd_st_idx  = simd_id * 400 + simd_lane_id;
    out[simd_st_idx + 0   * 8] = r1;
    out[simd_st_idx + 1   * 8] = r2;
    out[simd_st_idx + 2   * 8] = r3;
    out[simd_st_idx + 3   * 8] = r4;
    out[simd_st_idx + 4   * 8] = r5;
    out[simd_st_idx + 5   * 8] = r6;
    out[simd_st_idx + 6   * 8] = r7;
    out[simd_st_idx + 7   * 8] = r8;
    out[simd_st_idx + 8   * 8] = r9;
    out[simd_st_idx + 9   * 8] = r10;
    out[simd_st_idx + 10  * 8] = r11;
    out[simd_st_idx + 11  * 8] = r12;
    out[simd_st_idx + 12  * 8] = r13;
    out[simd_st_idx + 13  * 8] = r14;
    out[simd_st_idx + 14  * 8] = r15;
    out[simd_st_idx + 15  * 8] = r16;
    out[simd_st_idx + 16  * 8] = r17;
    out[simd_st_idx + 17  * 8] = r18;
    out[simd_st_idx + 18  * 8] = r19;
    out[simd_st_idx + 19  * 8] = r20;
    out[simd_st_idx + 20  * 8] = r21;
    out[simd_st_idx + 21  * 8] = r22;
    out[simd_st_idx + 22  * 8] = r23;
    out[simd_st_idx + 23  * 8] = r24;
    out[simd_st_idx + 24  * 8] = r25;
    out[simd_st_idx + 25  * 8] = r26;
    out[simd_st_idx + 26  * 8] = r27;
    out[simd_st_idx + 27  * 8] = r28;
    out[simd_st_idx + 28  * 8] = r29;
    out[simd_st_idx + 29  * 8] = r30;
    out[simd_st_idx + 30  * 8] = r31;
    out[simd_st_idx + 31  * 8] = r32;
    out[simd_st_idx + 32  * 8] = r33;
    out[simd_st_idx + 33  * 8] = r34;
    out[simd_st_idx + 34  * 8] = r35;
    out[simd_st_idx + 35  * 8] = r36;
    out[simd_st_idx + 36  * 8] = r37;
    out[simd_st_idx + 37  * 8] = r38;
    out[simd_st_idx + 38  * 8] = r39;
    out[simd_st_idx + 39  * 8] = r40;
    out[simd_st_idx + 40  * 8] = r41;
    out[simd_st_idx + 41  * 8] = r42;
    out[simd_st_idx + 42  * 8] = r43;
    out[simd_st_idx + 43  * 8] = r44;
    out[simd_st_idx + 44  * 8] = r45;
    out[simd_st_idx + 45  * 8] = r46;
    out[simd_st_idx + 46  * 8] = r47;
    out[simd_st_idx + 47  * 8] = r48;
    out[simd_st_idx + 48  * 8] = r49;
    out[simd_st_idx + 49  * 8] = r50;
  }
}

