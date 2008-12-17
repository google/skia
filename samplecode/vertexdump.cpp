#include "SkPoint.h"

void setup_vertexbug(SkPoint verts[], SkPoint texs[], uint16_t index[]);

void setup_vertexbug(SkPoint verts[], SkPoint texs[], uint16_t index[]) {
    verts[0].set(SkFloatToScalar(107), SkFloatToScalar(189));
    texs[0].set(SkFloatToScalar(0), SkFloatToScalar(0));
    verts[1].set(SkFloatToScalar(116), SkFloatToScalar(189));
    texs[1].set(SkFloatToScalar(9), SkFloatToScalar(0));
    verts[2].set(SkFloatToScalar(203), SkFloatToScalar(189));
    texs[2].set(SkFloatToScalar(35), SkFloatToScalar(0));
    verts[3].set(SkFloatToScalar(212), SkFloatToScalar(189));
    texs[3].set(SkFloatToScalar(44), SkFloatToScalar(0));
    verts[4].set(SkFloatToScalar(107), SkFloatToScalar(198));
    texs[4].set(SkFloatToScalar(0), SkFloatToScalar(9));
    verts[5].set(SkFloatToScalar(116), SkFloatToScalar(198));
    texs[5].set(SkFloatToScalar(9), SkFloatToScalar(9));
    verts[6].set(SkFloatToScalar(203), SkFloatToScalar(198));
    texs[6].set(SkFloatToScalar(35), SkFloatToScalar(9));
    verts[7].set(SkFloatToScalar(212), SkFloatToScalar(198));
    texs[7].set(SkFloatToScalar(44), SkFloatToScalar(9));
    verts[8].set(SkFloatToScalar(107), SkFloatToScalar(224));
    texs[8].set(SkFloatToScalar(0), SkFloatToScalar(39));
    verts[9].set(SkFloatToScalar(116), SkFloatToScalar(224));
    texs[9].set(SkFloatToScalar(9), SkFloatToScalar(39));
    verts[10].set(SkFloatToScalar(203), SkFloatToScalar(224));
    texs[10].set(SkFloatToScalar(35), SkFloatToScalar(39));
    verts[11].set(SkFloatToScalar(212), SkFloatToScalar(224));
    texs[11].set(SkFloatToScalar(44), SkFloatToScalar(39));
    verts[12].set(SkFloatToScalar(107), SkFloatToScalar(233));
    texs[12].set(SkFloatToScalar(0), SkFloatToScalar(48));
    verts[13].set(SkFloatToScalar(116), SkFloatToScalar(233));
    texs[13].set(SkFloatToScalar(9), SkFloatToScalar(48));
    verts[14].set(SkFloatToScalar(203), SkFloatToScalar(233));
    texs[14].set(SkFloatToScalar(35), SkFloatToScalar(48));
    verts[15].set(SkFloatToScalar(212), SkFloatToScalar(233));
    texs[15].set(SkFloatToScalar(44), SkFloatToScalar(48));
    index[0] = 0; index[1] = 5; index[2] = 1;
    index[3] = 0; index[4] = 4; index[5] = 5;
#if 0
    index[6] = 1; index[7] = 6; index[8] = 2;
#else
    index[6] = 6; index[7] = 2; index[8] = 1;
#endif
    index[9] = 1; index[10] = 5; index[11] = 6;
    index[12] = 2;
    index[13] = 7;
    index[14] = 3;
    index[15] = 2;
    index[16] = 6;
    index[17] = 7;
    index[18] = 4;
    index[19] = 9;
    index[20] = 5;
    index[21] = 4;
    index[22] = 8;
    index[23] = 9;
    index[24] = 5;
    index[25] = 10;
    index[26] = 6;
    index[27] = 5;
    index[28] = 9;
    index[29] = 10;
    index[30] = 6;
    index[31] = 11;
    index[32] = 7;
    index[33] = 6;
    index[34] = 10;
    index[35] = 11;
    index[36] = 8;
    index[37] = 13;
    index[38] = 9;
    index[39] = 8;
    index[40] = 12;
    index[41] = 13;
    index[42] = 9;
    index[43] = 14;
    index[44] = 10;
    index[45] = 9;
    index[46] = 13;
    index[47] = 14;
    index[48] = 10;
    index[49] = 15;
    index[50] = 11;
    index[51] = 10;
    index[52] = 14;
    index[53] = 15;
}
