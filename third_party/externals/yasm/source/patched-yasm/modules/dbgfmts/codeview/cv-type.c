/*
 * CodeView debugging format - type information
 *
 *  Copyright (C) 2006-2007  Peter Johnson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of other contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND OTHER CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR OTHER CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <util.h>

#include <libyasm.h>

#include "cv-dbgfmt.h"

enum cv_reservedtype {
    /* Bitfields representation - type */
    CV_TYPE_SPECIAL     = 0x00<<4,      /* Special */
    CV_TYPE_SIGNED      = 0x01<<4,      /* Signed integral value */
    CV_TYPE_UNSIGNED    = 0x02<<4,      /* Unsigned integral value */
    CV_TYPE_BOOLEAN     = 0x03<<4,      /* Boolean */
    CV_TYPE_REAL        = 0x04<<4,      /* Real */
    CV_TYPE_COMPLEX     = 0x05<<4,      /* Complex */
    CV_TYPE_SPECIAL2    = 0x06<<4,      /* Special2 */
    CV_TYPE_REALINT     = 0x07<<4,      /* Really int value */

    /* "size" of CV_TYPE_SPECIAL */
    CV_SPECIAL_NOTYPE   = 0x00<<0,      /* No type */
    CV_SPECIAL_ABS      = 0x01<<0,      /* Absolute symbol */
    CV_SPECIAL_SEG      = 0x02<<0,      /* Segment */
    CV_SPECIAL_VOID     = 0x03<<0,      /* Void */
    CV_SPECIAL_CURRENCY = 0x04<<0,      /* Basic 8-byte currency value */
    CV_SPECIAL_NEARBSTR = 0x05<<0,      /* Near Basic string */
    CV_SPECIAL_FARBSTR  = 0x06<<0,      /* Far Basic string */

    /* Size of CV_TYPE_SIGNED, CV_TYPE_UNSIGNED, and CV_TYPE_BOOLEAN */
    CV_INTEGER_1BYTE    = 0x00<<0,      /* 1 byte */
    CV_INTEGER_2BYTE    = 0x01<<0,      /* 2 byte */
    CV_INTEGER_4BYTE    = 0x02<<0,      /* 4 byte */
    CV_INTEGER_8BYTE    = 0x03<<0,      /* 8 byte */

    /* Size of CV_TYPE_REAL and CV_TYPE_COMPLEX */
    CV_REAL_32BIT       = 0x00<<0,      /* 32 bit */
    CV_REAL_64BIT       = 0x01<<0,      /* 64 bit */
    CV_REAL_80BIT       = 0x02<<0,      /* 80 bit */
    CV_REAL_128BIT      = 0x03<<0,      /* 128 bit */
    CV_REAL_48BIT       = 0x04<<0,      /* 48 bit */

    /* "size" of CV_TYPE_SPECIAL2 */
    CV_SPECIAL2_BIT     = 0x00<<0,      /* Bit */
    CV_SPECIAL2_PASCHAR = 0x01<<0,      /* Pascal CHAR */

    /* Size of CV_TYPE_REALINT */
    CV_REALINT_CHAR     = 0x00<<0,      /* Char */
    CV_REALINT_WCHAR    = 0x01<<0,      /* Wide character */
    CV_REALINT_S2BYTE   = 0x02<<0,      /* 2-byte signed integer */
    CV_REALINT_U2BYTE   = 0x03<<0,      /* 2-byte unsigned integer */
    CV_REALINT_S4BYTE   = 0x04<<0,      /* 4-byte signed integer */
    CV_REALINT_U4BYTE   = 0x05<<0,      /* 4-byte unsigned integer */
    CV_REALINT_S8BYTE   = 0x06<<0,      /* 8-byte signed integer */
    CV_REALINT_U8BYTE   = 0x07<<0,      /* 8-byte unsigned integer */

    /* Mode */
    CV_MODE_DIRECT      = 0x00<<8,      /* Direct; not a pointer */
    CV_MODE_NEAR        = 0x01<<8,      /* Near pointer */
    CV_MODE_FAR         = 0x02<<8,      /* Far pointer */
    CV_MODE_HUGE        = 0x03<<8,      /* Huge pointer */
    CV_MODE_NEAR32      = 0x04<<8,      /* 32-bit near pointer */
    CV_MODE_FAR32       = 0x05<<8,      /* 32-bit far pointer */
    CV_MODE_NEAR64      = 0x06<<8,      /* 64-bit near pointer */

    /* Pure primitive type listing - based on above bitfields */

    /* Special Types */
    CV_T_NOTYPE         = 0x0000, /* Uncharacterized type (no type) */
    CV_T_ABS            = 0x0001, /* Absolute symbol */
    CV_T_SEGMENT        = 0x0002, /* Segment type */
    CV_T_VOID           = 0x0003, /* Void */
    CV_T_PVOID          = 0x0103, /* Near pointer to void */
    CV_T_PFVOID         = 0x0203, /* Far pointer to void */
    CV_T_PHVOID         = 0x0303, /* Huge pointer to void */
    CV_T_32PVOID        = 0x0403, /* 32-bit near pointer to void */
    CV_T_32PFVOID       = 0x0503, /* 32-bit far pointer to void */
    CV_T_CURRENCY       = 0x0004, /* Basic 8-byte currency value */
    CV_T_NBASICSTR      = 0x0005, /* Near Basic string */
    CV_T_FBASICSTR      = 0x0006, /* Far Basic string */
    CV_T_BIT            = 0x0060, /* Bit */
    CV_T_PASCHAR        = 0x0061, /* Pascal CHAR */
    /* Character Types */
    CV_T_CHAR           = 0x0010, /* 8-bit signed */
    CV_T_UCHAR          = 0x0020, /* 8-bit unsigned */
    CV_T_PCHAR          = 0x0110, /* Near pointer to 8-bit signed */
    CV_T_PUCHAR         = 0x0120, /* Near pointer to 8-bit unsigned */
    CV_T_PFCHAR         = 0x0210, /* Far pointer to 8-bit signed */
    CV_T_PFUCHAR        = 0x0220, /* Far pointer to 8-bit unsigned */
    CV_T_PHCHAR         = 0x0310, /* Huge pointer to 8-bit signed */
    CV_T_PHUCHAR        = 0x0320, /* Huge pointer to 8-bit unsigned */
    CV_T_32PCHAR        = 0x0410, /* 16:32 near pointer to 8-bit signed */
    CV_T_32PUCHAR       = 0x0420, /* 16:32 near pointer to 8-bit unsigned */
    CV_T_32PFCHAR       = 0x0510, /* 16:32 far pointer to 8-bit signed */
    CV_T_32PFUCHAR      = 0x0520, /* 16:32 far pointer to 8-bit unsigned */
    /* Real Character Types */
    CV_T_RCHAR          = 0x0070, /* Real char */
    CV_T_PRCHAR         = 0x0170, /* Near pointer to a real char */
    CV_T_PFRCHAR        = 0x0270, /* Far pointer to a real char */
    CV_T_PHRCHAR        = 0x0370, /* Huge pointer to a real char */
    CV_T_32PRCHAR       = 0x0470, /* 16:32 near pointer to a real char */
    CV_T_32PFRCHAR      = 0x0570, /* 16:32 far pointer to a real char */
    /* Wide Character Types */
    CV_T_WCHAR          = 0x0071, /* Wide char */
    CV_T_PWCHAR         = 0x0171, /* Near pointer to a wide char */
    CV_T_PFWCHAR        = 0x0271, /* Far pointer to a wide char */
    CV_T_PHWCHAR        = 0x0371, /* Huge pointer to a wide char */
    CV_T_32PWCHAR       = 0x0471, /* 16:32 near pointer to a wide char */
    CV_T_32PFWCHAR      = 0x0571, /* 16:32 far pointer to a wide char */
    /* Real 16-bit Integer Types */
    CV_T_INT2           = 0x0072, /* Real 16-bit signed int */
    CV_T_UINT2          = 0x0073, /* Real 16-bit unsigned int */
    CV_T_PINT2          = 0x0172, /* Near pointer to 16-bit signed int */
    CV_T_PUINT2         = 0x0173, /* Near pointer to 16-bit unsigned int */
    CV_T_PFINT2         = 0x0272, /* Far pointer to 16-bit signed int */
    CV_T_PFUINT2        = 0x0273, /* Far pointer to 16-bit unsigned int */
    CV_T_PHINT2         = 0x0372, /* Huge pointer to 16-bit signed int */
    CV_T_PHUINT2        = 0x0373, /* Huge pointer to 16-bit unsigned int */
    CV_T_32PINT2        = 0x0472, /* 16:32 near pointer to 16-bit signed int */
    CV_T_32PUINT2       = 0x0473, /* 16:32 near pointer to 16-bit unsigned int */
    CV_T_32PFINT2       = 0x0572, /* 16:32 far pointer to 16-bit signed int */
    CV_T_32PFUINT2      = 0x0573, /* 16:32 far pointer to 16-bit unsigned int */
    /* 16-bit Short Types */
    CV_T_SHORT          = 0x0011, /* 16-bit signed */
    CV_T_USHORT         = 0x0021, /* 16-bit unsigned */
    CV_T_PSHORT         = 0x0111, /* Near pointer to 16-bit signed */
    CV_T_PUSHORT        = 0x0121, /* Near pointer to 16-bit unsigned */
    CV_T_PFSHORT        = 0x0211, /* Far pointer to 16-bit signed */
    CV_T_PFUSHORT       = 0x0221, /* Far pointer to 16-bit unsigned */
    CV_T_PHSHORT        = 0x0311, /* Huge pointer to 16-bit signed */
    CV_T_PHUSHORT       = 0x0321, /* Huge pointer to 16-bit unsigned */
    CV_T_32PSHORT       = 0x0411, /* 16:32 near pointer to 16-bit signed */
    CV_T_32PUSHORT      = 0x0421, /* 16:32 near pointer to 16-bit unsigned */
    CV_T_32PFSHORT      = 0x0511, /* 16:32 far pointer to 16-bit signed */
    CV_T_32PFUSHORT     = 0x0521, /* 16:32 far pointer to 16-bit unsigned */
    /* Real 32-bit Integer Types */
    CV_T_INT4           = 0x0074, /* Real 32-bit signed int */
    CV_T_UINT4          = 0x0075, /* Real 32-bit unsigned int */
    CV_T_PINT4          = 0x0174, /* Near pointer to 32-bit signed int */
    CV_T_PUINT4         = 0x0175, /* Near pointer to 32-bit unsigned int */
    CV_T_PFINT4         = 0x0274, /* Far pointer to 32-bit signed int */
    CV_T_PFUINT4        = 0x0275, /* Far pointer to 32-bit unsigned int */
    CV_T_PHINT4         = 0x0374, /* Huge pointer to 32-bit signed int */
    CV_T_PHUINT4        = 0x0375, /* Huge pointer to 32-bit unsigned int */
    CV_T_32PINT4        = 0x0474, /* 16:32 near pointer to 32-bit signed int */
    CV_T_32PUINT4       = 0x0475, /* 16:32 near pointer to 32-bit unsigned int */
    CV_T_32PFINT4       = 0x0574, /* 16:32 far pointer to 32-bit signed int */
    CV_T_32PFUINT4      = 0x0575, /* 16:32 far pointer to 32-bit unsigned int */
    /* 32-bit Long Types */
    CV_T_LONG           = 0x0012, /* 32-bit signed */
    CV_T_ULONG          = 0x0022, /* 32-bit unsigned */
    CV_T_PLONG          = 0x0112, /* Near pointer to 32-bit signed */
    CV_T_PULONG         = 0x0122, /* Near pointer to 32-bit unsigned */
    CV_T_PFLONG         = 0x0212, /* Far pointer to 32-bit signed */
    CV_T_PFULONG        = 0x0222, /* Far pointer to 32-bit unsigned */
    CV_T_PHLONG         = 0x0312, /* Huge pointer to 32-bit signed */
    CV_T_PHULONG        = 0x0322, /* Huge pointer to 32-bit unsigned */
    CV_T_32PLONG        = 0x0412, /* 16:32 near pointer to 32-bit signed */
    CV_T_32PULONG       = 0x0422, /* 16:32 near pointer to 32-bit unsigned */
    CV_T_32PFLONG       = 0x0512, /* 16:32 far pointer to 32-bit signed */
    CV_T_32PFULONG      = 0x0522, /* 16:32 far pointer to 32-bit unsigned */
    /* Real 64-bit int Types */
    CV_T_INT8           = 0x0076, /* 64-bit signed int */
    CV_T_UINT8          = 0x0077, /* 64-bit unsigned int */
    CV_T_PINT8          = 0x0176, /* Near pointer to 64-bit signed int */
    CV_T_PUINT8         = 0x0177, /* Near pointer to 64-bit unsigned int */
    CV_T_PFINT8         = 0x0276, /* Far pointer to 64-bit signed int */
    CV_T_PFUINT8        = 0x0277, /* Far pointer to 64-bit unsigned int */
    CV_T_PHINT8         = 0x0376, /* Huge pointer to 64-bit signed int */
    CV_T_PHUINT8        = 0x0377, /* Huge pointer to 64-bit unsigned int */
    CV_T_32PINT8        = 0x0476, /* 16:32 near pointer to 64-bit signed int */
    CV_T_32PUINT8       = 0x0477, /* 16:32 near pointer to 64-bit unsigned int */
    CV_T_32PFINT8       = 0x0576, /* 16:32 far pointer to 64-bit signed int */
    CV_T_32PFUINT8      = 0x0577, /* 16:32 far pointer to 64-bit unsigned int */
    /* 64-bit Integral Types */
    CV_T_QUAD           = 0x0013, /* 64-bit signed */
    CV_T_UQUAD          = 0x0023, /* 64-bit unsigned */
    CV_T_PQUAD          = 0x0113, /* Near pointer to 64-bit signed */
    CV_T_PUQUAD         = 0x0123, /* Near pointer to 64-bit unsigned */
    CV_T_PFQUAD         = 0x0213, /* Far pointer to 64-bit signed */
    CV_T_PFUQUAD        = 0x0223, /* Far pointer to 64-bit unsigned */
    CV_T_PHQUAD         = 0x0313, /* Huge pointer to 64-bit signed */
    CV_T_PHUQUAD        = 0x0323, /* Huge pointer to 64-bit unsigned */
    CV_T_32PQUAD        = 0x0413, /* 16:32 near pointer to 64-bit signed */
    CV_T_32PUQUAD       = 0x0423, /* 16:32 near pointer to 64-bit unsigned */
    CV_T_32PFQUAD       = 0x0513, /* 16:32 far pointer to 64-bit signed */
    CV_T_32PFUQUAD      = 0x0523, /* 16:32 far pointer to 64-bit unsigned */
    /* 32-bit Real Types */
    CV_T_REAL32         = 0x0040, /* 32-bit real */
    CV_T_PREAL32        = 0x0140, /* Near pointer to 32-bit real */
    CV_T_PFREAL32       = 0x0240, /* Far pointer to 32-bit real */
    CV_T_PHREAL32       = 0x0340, /* Huge pointer to 32-bit real */
    CV_T_32PREAL32      = 0x0440, /* 16:32 near pointer to 32-bit real */
    CV_T_32PFREAL32     = 0x0540, /* 16:32 far pointer to 32-bit real */
    /* 48-bit Real Types */
    CV_T_REAL48         = 0x0044, /* 48-bit real */
    CV_T_PREAL48        = 0x0144, /* Near pointer to 48-bit real */
    CV_T_PFREAL48       = 0x0244, /* Far pointer to 48-bit real */
    CV_T_PHREAL48       = 0x0344, /* Huge pointer to 48-bit real */
    CV_T_32PREAL48      = 0x0444, /* 16:32 near pointer to 48-bit real */
    CV_T_32PFREAL48     = 0x0544, /* 16:32 far pointer to 48-bit real */
    /* 64-bit Real Types */
    CV_T_REAL64         = 0x0041, /* 64-bit real */
    CV_T_PREAL64        = 0x0141, /* Near pointer to 64-bit real */
    CV_T_PFREAL64       = 0x0241, /* Far pointer to 64-bit real */
    CV_T_PHREAL64       = 0x0341, /* Huge pointer to 64-bit real */
    CV_T_32PREAL64      = 0x0441, /* 16:32 near pointer to 64-bit real */
    CV_T_32PFREAL64     = 0x0541, /* 16:32 far pointer to 64-bit real */
    /* 80-bit Real Types */
    CV_T_REAL80         = 0x0042, /* 80-bit real */
    CV_T_PREAL80        = 0x0142, /* Near pointer to 80-bit real */
    CV_T_PFREAL80       = 0x0242, /* Far pointer to 80-bit real */
    CV_T_PHREAL80       = 0x0342, /* Huge pointer to 80-bit real */
    CV_T_32PREAL80      = 0x0442, /* 16:32 near pointer to 80-bit real */
    CV_T_32PFREAL80     = 0x0542, /* 16:32 far pointer to 80-bit real */
    /* 128-bit Real Types */
    CV_T_REAL128        = 0x0043, /* 128-bit real */
    CV_T_PREAL128       = 0x0143, /* Near pointer to 128-bit real */
    CV_T_PFREAL128      = 0x0243, /* Far pointer to 128-bit real */
    CV_T_PHREAL128      = 0x0343, /* Huge pointer to 128-bit real */
    CV_T_32PREAL128     = 0x0443, /* 16:32 near pointer to 128-bit real */
    CV_T_32PFREAL128    = 0x0543, /* 16:32 far pointer to 128-bit real */
    /* 32-bit Complex Types */
    CV_T_CPLX32         = 0x0050, /* 32-bit complex */
    CV_T_PCPLX32        = 0x0150, /* Near pointer to 32-bit complex */
    CV_T_PFCPLX32       = 0x0250, /* Far pointer to 32-bit complex */
    CV_T_PHCPLX32       = 0x0350, /* Huge pointer to 32-bit complex */
    CV_T_32PCPLX32      = 0x0450, /* 16:32 near pointer to 32-bit complex */
    CV_T_32PFCPLX32     = 0x0550, /* 16:32 far pointer to 32-bit complex */
    /* 64-bit Complex Types */
    CV_T_CPLX64         = 0x0051, /* 64-bit complex */
    CV_T_PCPLX64        = 0x0151, /* Near pointer to 64-bit complex */
    CV_T_PFCPLX64       = 0x0251, /* Far pointer to 64-bit complex */
    CV_T_PHCPLX64       = 0x0351, /* Huge pointer to 64-bit complex */
    CV_T_32PCPLX64      = 0x0451, /* 16:32 near pointer to 64-bit complex */
    CV_T_32PFCPLX64     = 0x0551, /* 16:32 far pointer to 64-bit complex */
    /* 80-bit Complex Types */
    CV_T_CPLX80         = 0x0052, /* 80-bit complex */
    CV_T_PCPLX80        = 0x0152, /* Near pointer to 80-bit complex */
    CV_T_PFCPLX80       = 0x0252, /* Far pointer to 80-bit complex */
    CV_T_PHCPLX80       = 0x0352, /* Huge pointer to 80-bit complex */
    CV_T_32PCPLX80      = 0x0452, /* 16:32 near pointer to 80-bit complex */
    CV_T_32PFCPLX80     = 0x0552, /* 16:32 far pointer to 80-bit complex */
    /* 128-bit Complex Types */
    CV_T_CPLX128        = 0x0053, /* 128-bit complex */
    CV_T_PCPLX128       = 0x0153, /* Near pointer to 128-bit complex */
    CV_T_PFCPLX128      = 0x0253, /* Far pointer to 128-bit complex */
    CV_T_PHCPLX128      = 0x0353, /* Huge pointer to 128-bit real */
    CV_T_32PCPLX128     = 0x0453, /* 16:32 near pointer to 128-bit complex */
    CV_T_32PFCPLX128    = 0x0553, /* 16:32 far pointer to 128-bit complex */
    /* Boolean Types */
    CV_T_BOOL08         = 0x0030, /* 8-bit Boolean */
    CV_T_BOOL16         = 0x0031, /* 16-bit Boolean */
    CV_T_BOOL32         = 0x0032, /* 32-bit Boolean */
    CV_T_BOOL64         = 0x0033, /* 64-bit Boolean */
    CV_T_PBOOL08        = 0x0130, /* Near pointer to 8-bit Boolean */
    CV_T_PBOOL16        = 0x0131, /* Near pointer to 16-bit Boolean */
    CV_T_PBOOL32        = 0x0132, /* Near pointer to 32-bit Boolean */
    CV_T_PBOOL64        = 0x0133, /* Near pointer to 64-bit Boolean */
    CV_T_PFBOOL08       = 0x0230, /* Far pointer to 8-bit Boolean */
    CV_T_PFBOOL16       = 0x0231, /* Far pointer to 16-bit Boolean */
    CV_T_PFBOOL32       = 0x0232, /* Far pointer to 32-bit Boolean */
    CV_T_PFBOOL64       = 0x0233, /* Far pointer to 64-bit Boolean */
    CV_T_PHBOOL08       = 0x0330, /* Huge pointer to 8-bit Boolean */
    CV_T_PHBOOL16       = 0x0331, /* Huge pointer to 16-bit Boolean */
    CV_T_PHBOOL32       = 0x0332, /* Huge pointer to 32-bit Boolean */
    CV_T_PHBOOL64       = 0x0333, /* Huge pointer to 64-bit Boolean */
    CV_T_32PBOOL08      = 0x0430, /* 16:32 near pointer to 8-bit Boolean */
    CV_T_32PBOOL16      = 0x0431, /* 16:32 near pointer to 16-bit Boolean */
    CV_T_32PBOOL32      = 0x0432, /* 16:32 near pointer to 32-bit Boolean */
    CV_T_32PBOOL64      = 0x0433, /* 16:32 near pointer to 64-bit Boolean */
    CV_T_32PFBOOL08     = 0x0530, /* 16:32 far pointer to 8-bit Boolean */
    CV_T_32PFBOOL16     = 0x0531, /* 16:32 far pointer to 16-bit Boolean */
    CV_T_32PFBOOL32     = 0x0532, /* 16:32 far pointer to 32-bit Boolean */
    CV_T_32PFBOOL64     = 0x0533, /* 16:32 far pointer to 64-bit Boolean */

    /* Non-primitive types are stored in the TYPES section (generated in
     * cv-type.c) and start at this index (e.g. 0x1000 is the first type
     * in TYPES, 0x1001 the second, etc.
     */
    CV_FIRST_NONPRIM    = 0x1000
};

enum cv_leaftype {
    /* Leaf indices for type records that can be referenced from symbols */
    CV4_LF_MODIFIER     = 0x0001,       /* Type Modifier */
    CV4_LF_POINTER      = 0x0002,       /* Pointer */
    CV4_LF_ARRAY        = 0x0003,       /* Simple Array */
    CV4_LF_CLASS        = 0x0004,       /* Classes */
    CV4_LF_STRUCTURE    = 0x0005,       /* Structures */
    CV4_LF_UNION        = 0x0006,       /* Unions */
    CV4_LF_ENUM         = 0x0007,       /* Enumeration */
    CV4_LF_PROCEDURE    = 0x0008,       /* Procedure */
    CV4_LF_MFUNCTION    = 0x0009,       /* Member Function */
    CV4_LF_VTSHAPE      = 0x000a,       /* Virtual Function Table Shape */
    CV4_LF_BARRAY       = 0x000d,       /* Basic Array */
    CV4_LF_LABEL        = 0x000e,       /* Label */
    CV4_LF_NULL         = 0x000f,       /* Null */
    CV4_LF_DIMARRAY     = 0x0011,       /* Multiply Dimensioned Array */
    CV4_LF_VFTPATH      = 0x0012,       /* Path to Virtual Function Table */
    CV4_LF_PRECOMP      = 0x0013,       /* Reference Precompiled Types */
    CV4_LF_ENDPRECOMP   = 0x0014,       /* End of Precompiled Types */

    /* CodeView 5.0 version */
    CV5_LF_MODIFIER     = 0x1001,       /* Type Modifier */
    CV5_LF_POINTER      = 0x1002,       /* Pointer */
    CV5_LF_ARRAY        = 0x1003,       /* Simple Array */
    CV5_LF_CLASS        = 0x1004,       /* Classes */
    CV5_LF_STRUCTURE    = 0x1005,       /* Structures */
    CV5_LF_UNION        = 0x1006,       /* Unions */
    CV5_LF_ENUM         = 0x1007,       /* Enumeration */
    CV5_LF_PROCEDURE    = 0x1008,       /* Procedure */
    CV5_LF_MFUNCTION    = 0x1009,       /* Member Function */
    CV5_LF_VTSHAPE      = 0x000a,       /* Virtual Function Table Shape */
    CV5_LF_BARRAY       = 0x100d,       /* Basic Array */
    CV5_LF_LABEL        = 0x000e,       /* Label */
    CV5_LF_NULL         = 0x000f,       /* Null */
    CV5_LF_DIMARRAY     = 0x100c,       /* Multiply Dimensioned Array */
    CV5_LF_VFTPATH      = 0x100d,       /* Path to Virtual Function Table */
    CV5_LF_PRECOMP      = 0x100e,       /* Reference Precompiled Types */
    CV5_LF_ENDPRECOMP   = 0x0014,       /* End of Precompiled Types */
    CV5_LF_TYPESERVER   = 0x0016,       /* Reference Typeserver */

    /* Leaf indices for type records that can be referenced from other type
     * records
     */
    CV4_LF_SKIP         = 0x0200,       /* Skip */
    CV4_LF_ARGLIST      = 0x0201,       /* Argument List */
    CV4_LF_DEFARG       = 0x0202,       /* Default Argument */
    CV4_LF_LIST         = 0x0203,       /* Arbitrary List */
    CV4_LF_FIELDLIST    = 0x0204,       /* Field List */
    CV4_LF_DERIVED      = 0x0205,       /* Derived Classes */
    CV4_LF_BITFIELD     = 0x0206,       /* Bit Fields */
    CV4_LF_METHODLIST   = 0x0207,       /* Method List */
    CV4_LF_DIMCONU      = 0x0208,       /* Dimensioned Array with Constant Upper Bound */
    CV4_LF_DIMCONLU     = 0x0209,       /* Dimensioned Array with Constant Lower and Upper Bounds */
    CV4_LF_DIMVARU      = 0x020a,       /* Dimensioned Array with Variable Upper Bound */
    CV4_LF_DIMVARLU     = 0x020b,       /* Dimensioned Array with Variable Lower and Upper Bounds */
    CV4_LF_REFSYM       = 0x020c,       /* Referenced Symbol */

    /* CodeView 5.0 version */
    CV5_LF_SKIP         = 0x1200,       /* Skip */
    CV5_LF_ARGLIST      = 0x1201,       /* Argument List */
    CV5_LF_DEFARG       = 0x1202,       /* Default Argument */
    CV5_LF_FIELDLIST    = 0x1203,       /* Field List */
    CV5_LF_DERIVED      = 0x1204,       /* Derived Classes */
    CV5_LF_BITFIELD     = 0x1205,       /* Bit Fields */
    CV5_LF_METHODLIST   = 0x1206,       /* Method List */
    CV5_LF_DIMCONU      = 0x1207,       /* Dimensioned Array with Constant Upper Bound */
    CV5_LF_DIMCONLU     = 0x1208,       /* Dimensioned Array with Constant Lower and Upper Bounds */
    CV5_LF_DIMVARU      = 0x1209,       /* Dimensioned Array with Variable Upper Bound */
    CV5_LF_DIMVARLU     = 0x120a,       /* Dimensioned Array with Variable Lower and Upper Bounds */
    CV5_LF_REFSYM       = 0x020c,       /* Referenced Symbol */

    /* Leaf indices for fields of complex lists */
    CV4_LF_BCLASS       = 0x0400,       /* Real Base Class */
    CV4_LF_VBCLASS      = 0x0401,       /* Direct Virtual Base Class */
    CV4_LF_IVBCLASS     = 0x0402,       /* Indirect Virtual Base Class */
    CV4_LF_ENUMERATE    = 0x0403,       /* Enumeration Name and Value */
    CV4_LF_FRIENDFCN    = 0x0404,       /* Friend Function */
    CV4_LF_INDEX        = 0x0405,       /* Index To Another Type Record */
    CV4_LF_MEMBER       = 0x0406,       /* Data Member */
    CV4_LF_STMEMBER     = 0x0407,       /* Static Data Member */
    CV4_LF_METHOD       = 0x0408,       /* Method */
    CV4_LF_NESTTYPE     = 0x0409,       /* Nested Type Definition */
    CV4_LF_VFUNCTAB     = 0x040a,       /* Virtual Function Table Pointer */
    CV4_LF_FRIENDCLS    = 0x040b,       /* Friend Class */
    CV4_LF_ONEMETHOD    = 0x040c,       /* One Method */
    CV4_LF_VFUNCOFF     = 0x040d,       /* Virtual Function Offset */

    /* CodeView 5.0 version */
    CV5_LF_BCLASS       = 0x1400,       /* Real Base Class */
    CV5_LF_VBCLASS      = 0x1401,       /* Direct Virtual Base Class */
    CV5_LF_IVBCLASS     = 0x1402,       /* Indirect Virtual Base Class */
    CV5_LF_ENUMERATE    = 0x0403,       /* Enumeration Name and Value */
    CV5_LF_FRIENDFCN    = 0x1403,       /* Friend Function */
    CV5_LF_INDEX        = 0x1404,       /* Index To Another Type Record */
    CV5_LF_MEMBER       = 0x1405,       /* Data Member */
    CV5_LF_STMEMBER     = 0x1406,       /* Static Data Member */
    CV5_LF_METHOD       = 0x1407,       /* Method */
    CV5_LF_NESTTYPE     = 0x1408,       /* Nested Type Definition */
    CV5_LF_VFUNCTAB     = 0x1409,       /* Virtual Function Table Pointer */
    CV5_LF_FRIENDCLS    = 0x140a,       /* Friend Class */
    CV5_LF_ONEMETHOD    = 0x140b,       /* One Method */
    CV5_LF_VFUNCOFF     = 0x140c,       /* Virtual Function Offset */
    CV5_LF_NESTTYPEEX   = 0x140d,       /* Nested Type Extended Definition */
    CV5_LF_MEMBERMODIFY = 0x140e,       /* Member Modification */
    /* XXX: CodeView 5.0 spec also lists 0x040f as LF_MEMBERMODIFY? */

    /* Leaf indices for numeric fields of symbols and type records */
    CV_LF_NUMERIC       = 0x8000,
    CV_LF_CHAR          = 0x8000,       /* Signed Char (8-bit) */
    CV_LF_SHORT         = 0x8001,       /* Signed Short (16-bit) */
    CV_LF_USHORT        = 0x8002,       /* Unsigned Short (16-bit) */
    CV_LF_LONG          = 0x8003,       /* Signed Long (32-bit) */
    CV_LF_ULONG         = 0x8004,       /* Unsigned Long (32-bit) */
    CV_LF_REAL32        = 0x8005,       /* 32-bit Float */
    CV_LF_REAL64        = 0x8006,       /* 64-bit Float */
    CV_LF_REAL80        = 0x8007,       /* 80-bit Float */
    CV_LF_REAL128       = 0x8008,       /* 128-bit Float */
    CV_LF_QUADWORD      = 0x8009,       /* Signed Quad Word (64-bit) */
    CV_LF_UQUADWORD     = 0x800a,       /* Unsigned Quad Word (64-bit) */
    CV_LF_REAL48        = 0x800b,       /* 48-bit Float */
    CV_LF_COMPLEX32     = 0x800c,       /* 32-bit Complex */
    CV_LF_COMPLEX64     = 0x800d,       /* 64-bit Complex */
    CV_LF_COMPLEX80     = 0x800e,       /* 80-bit Complex */
    CV_LF_COMPLEX128    = 0x800f,       /* 128-bit Complex */
    CV_LF_VARSTRING     = 0x8010,       /* Variable-length String */

    /* Leaf padding bytes */
    CV_LF_PAD0          = 0xf0,
    CV_LF_PAD1          = 0xf1,
    CV_LF_PAD2          = 0xf2,
    CV_LF_PAD3          = 0xf3,
    CV_LF_PAD4          = 0xf4,
    CV_LF_PAD5          = 0xf5,
    CV_LF_PAD6          = 0xf6,
    CV_LF_PAD7          = 0xf7,
    CV_LF_PAD8          = 0xf8,
    CV_LF_PAD9          = 0xf9,
    CV_LF_PAD10         = 0xfa,
    CV_LF_PAD11         = 0xfb,
    CV_LF_PAD12         = 0xfc,
    CV_LF_PAD13         = 0xfc,
    CV_LF_PAD14         = 0xfe,
    CV_LF_PAD15         = 0xff
};

/* Leaves use a bit of meta-programming to encode formats: each character
 * of format represents the output generated, as follows:
 * 'b' : 1 byte value (integer)
 * 'h' : 2 byte value (integer)
 * 'w' : 4 byte value (integer)
 * 'L' : subleaf, recurses into cv_leaf (pointer)
 * 'T' : 4 byte type index, pulls cv_type.index from cv_type (pointer)
 * 'S' : length-prefixed string (pointer)
 */
typedef struct cv_leaf {
    enum cv_leaftype type;
    const char *format;     /* format of args */
    union {
        unsigned long i;
        void *p;
    } args[6];
} cv_leaf;

typedef struct cv_type {
    unsigned long indx;     /* type # (must be same as output order) */
    size_t num_leaves;
    /*@null@*/ /*@only@*/ cv_leaf **leaves;
} cv_type;

/* Bytecode callback function prototypes */
static void cv_type_bc_destroy(void *contents);
static void cv_type_bc_print(const void *contents, FILE *f, int indent_level);
static int cv_type_bc_calc_len
    (yasm_bytecode *bc, yasm_bc_add_span_func add_span, void *add_span_data);
static int cv_type_bc_tobytes
    (yasm_bytecode *bc, unsigned char **bufp, unsigned char *bufstart, void *d,
     yasm_output_value_func output_value,
     /*@null@*/ yasm_output_reloc_func output_reloc);

/* Bytecode callback structures */
static const yasm_bytecode_callback cv_type_bc_callback = {
    cv_type_bc_destroy,
    cv_type_bc_print,
    yasm_bc_finalize_common,
    NULL,
    cv_type_bc_calc_len,
    yasm_bc_expand_common,
    cv_type_bc_tobytes,
    0
};

static cv_type *cv_type_create(unsigned long indx);
static void cv_type_append_leaf(cv_type *type, /*@keep@*/ cv_leaf *leaf);


static cv_leaf *
cv_leaf_create_label(int is_far)
{
    cv_leaf *leaf = yasm_xmalloc(sizeof(cv_leaf));
    leaf->type = CV5_LF_LABEL;
    leaf->format = "h";
    leaf->args[0].i = is_far ? 4 : 0;
    return leaf;
}

yasm_section *
yasm_cv__generate_type(yasm_object *object)
{
    int new;
    unsigned long indx = CV_FIRST_NONPRIM;
    yasm_section *debug_type;
    yasm_bytecode *bc;
    cv_type *type;

    debug_type =
        yasm_object_get_general(object, ".debug$T", 1, 0, 0, &new, 0);

    /* Add label type */
    type = cv_type_create(indx++);
    cv_type_append_leaf(type, cv_leaf_create_label(0));
    bc = yasm_bc_create_common(&cv_type_bc_callback, type, 0);
    yasm_bc_finalize(bc, yasm_cv__append_bc(debug_type, bc));
    yasm_bc_calc_len(bc, NULL, NULL);

    return debug_type;
}

static void
cv_leaf_destroy(cv_leaf *leaf)
{
    const char *ch = leaf->format;
    int arg = 0;

    while (*ch) {
        switch (*ch) {
            case 'b':
            case 'h':
            case 'w':
                arg++;
                break;  /* nothing to destroy */
            case 'L':
                cv_leaf_destroy((cv_leaf *)leaf->args[arg++].p);
                break;
            case 'T':
                arg++;  /* nothing to destroy */
                break;
            case 'S':
                yasm_xfree(leaf->args[arg++].p);
                break;
            default:
                yasm_internal_error(N_("unknown leaf format character"));
        }
        ch++;
    }
}

static unsigned long
cv_leaf_size(const cv_leaf *leaf)
{
    const char *ch = leaf->format;
    unsigned long len = 2;  /* leaf type */
    unsigned long slen;
    int arg = 0;

    while (*ch) {
        switch (*ch) {
            case 'b':
                len++;
                arg++;
                break;
            case 'h':
                len += 2;
                arg++;
                break;
            case 'w':
                len += 4;
                arg++;
                break;
            case 'L':
                len += cv_leaf_size((const cv_leaf *)leaf->args[arg++].p);
                break;
            case 'T':
                len += 4;       /* XXX: will be 2 in CV4 */
                arg++;
                break;
            case 'S':
                len += 1;       /* XXX: is this 1 or 2? */
                slen = (unsigned long)strlen((const char *)leaf->args[arg++].p);
                len += slen <= 0xff ? slen : 0xff;
                break;
            default:
                yasm_internal_error(N_("unknown leaf format character"));
        }
        ch++;
    }

    return len;
}

static void
cv_leaf_tobytes(const cv_leaf *leaf, yasm_bytecode *bc, yasm_arch *arch,
                unsigned char **bufp, yasm_intnum *cval)
{
    unsigned char *buf = *bufp;
    const char *ch = leaf->format;
    size_t len;
    int arg = 0;

    /* leaf type */
    yasm_intnum_set_uint(cval, leaf->type);
    yasm_arch_intnum_tobytes(arch, cval, buf, 2, 16, 0, bc, 0);
    buf += 2;

    while (*ch) {
        switch (*ch) {
            case 'b':
                YASM_WRITE_8(buf, leaf->args[arg].i);
                arg++;
                break;
            case 'h':
                yasm_intnum_set_uint(cval, leaf->args[arg++].i);
                yasm_arch_intnum_tobytes(arch, cval, buf, 2, 16, 0, bc, 0);
                buf += 2;
                break;
            case 'w':
                yasm_intnum_set_uint(cval, leaf->args[arg++].i);
                yasm_arch_intnum_tobytes(arch, cval, buf, 4, 32, 0, bc, 0);
                buf += 4;
                break;
            case 'L':
                cv_leaf_tobytes((const cv_leaf *)leaf->args[arg++].p, bc, arch,
                                &buf, cval);
                break;
            case 'T':
                yasm_intnum_set_uint(cval,
                    ((const cv_type *)leaf->args[arg++].p)->indx);
                yasm_arch_intnum_tobytes(arch, cval, buf, 4, 32, 0, bc, 0);
                buf += 4;       /* XXX: will be 2 in CV4 */
                break;
            case 'S':
                len = strlen((const char *)leaf->args[arg].p);
                len = len <= 0xff ? len : 0xff;
                YASM_WRITE_8(buf, len);
                memcpy(buf, (const char *)leaf->args[arg].p, len);
                buf += len;
                arg++;
                break;
            default:
                yasm_internal_error(N_("unknown leaf format character"));
        }
        ch++;
    }

    *bufp = buf;
}

static cv_type *
cv_type_create(unsigned long indx)
{
    cv_type *type = yasm_xmalloc(sizeof(cv_type));

    type->indx = indx;
    type->num_leaves = 0;
    type->leaves = NULL;

    return type;
}

static void
cv_type_append_leaf(cv_type *type, /*@keep@*/ cv_leaf *leaf)
{
    type->num_leaves++;

    /* This is inefficient for large numbers of leaves, but that won't happen
     * until we add structure support.
     */
    type->leaves = yasm_xrealloc(type->leaves,
                                 type->num_leaves*sizeof(cv_leaf *));

    type->leaves[type->num_leaves-1] = leaf;
}

static void
cv_type_bc_destroy(void *contents)
{
    cv_type *type = (cv_type *)contents;
    size_t i;

    for (i=0; i<type->num_leaves; i++)
        cv_leaf_destroy(type->leaves[i]);
    if (type->leaves)
        yasm_xfree(type->leaves);
    yasm_xfree(contents);
}

static void
cv_type_bc_print(const void *contents, FILE *f, int indent_level)
{
    /* TODO */
}

static int
cv_type_bc_calc_len(yasm_bytecode *bc, yasm_bc_add_span_func add_span,
                    void *add_span_data)
{
    cv_type *type = (cv_type *)bc->contents;
    size_t i;

    if (type->indx == CV_FIRST_NONPRIM)
        bc->len = 4+2;
    else
        bc->len = 2;

    for (i=0; i<type->num_leaves; i++)
        bc->len += cv_leaf_size(type->leaves[i]);

    /* Pad to multiple of 4 */
    if (bc->len & 0x3)
        bc->len += 4-(bc->len & 0x3);

    return 0;
}

static int
cv_type_bc_tobytes(yasm_bytecode *bc, unsigned char **bufp,
                   unsigned char *bufstart, void *d,
                   yasm_output_value_func output_value,
                   yasm_output_reloc_func output_reloc)
{
    yasm_object *object = yasm_section_get_object(bc->section);
    cv_type *type = (cv_type *)bc->contents;
    unsigned char *buf = *bufp;
    yasm_intnum *cval;
    size_t i;
    unsigned long reclen = bc->len - 2;

    cval = yasm_intnum_create_uint(4);      /* version */
    if (type->indx == CV_FIRST_NONPRIM) {
        yasm_arch_intnum_tobytes(object->arch, cval, buf, 4, 32, 0, bc, 1);
        buf += 4;
        reclen -= 4;
    }

    /* Total length of record (following this field) - 2 bytes */
    yasm_intnum_set_uint(cval, reclen);
    yasm_arch_intnum_tobytes(object->arch, cval, buf, 2, 16, 0, bc, 1);
    buf += 2;

    /* Leaves */
    for (i=0; i<type->num_leaves; i++)
        cv_leaf_tobytes(type->leaves[i], bc, object->arch, &buf, cval);

    /* Pad to multiple of 4 */
    switch ((buf-(*bufp)) & 0x3) {
        case 3:
            YASM_WRITE_8(buf, CV_LF_PAD3);
        case 2:
            YASM_WRITE_8(buf, CV_LF_PAD2);
        case 1:
            YASM_WRITE_8(buf, CV_LF_PAD1);
        case 0:
            break;
    }

    *bufp = buf;

    yasm_intnum_destroy(cval);
    return 0;
}
