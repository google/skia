.set bar,(foo-1)
.set foo,5
.byte foo /* 0x5 */
.byte bar /* 0x4 */
.set foo,6
.byte foo /* 0x6 */
.byte bar /* 0x4 */
.set bar,10
.byte bar /* 0xa */
.set bar,foo+1
.byte bar /* 0x7 */
.set bar,bar+1
.byte bar /* 0x8 */
.set bar,5
.set bam,bar-1
.set bar,1
.byte bam /* 0x4 */
.set bar, boo + 1
.set boo, 5
.set boo, 6
.byte bar /* 0x6 */
.set a, b+c
.set b, 0x1
.set b, 0x2
.set c, 0x10
.set c, 0x20
.byte a /* 0x11 */
.set x, 5
.set y, x+z
.set x, 10
.set z, 1
.byte y /* 0x6 */
.set z, 0xfe
.byte z /* 0xfe */
