
// inline utilities
/* Returns 0 if negative, 1 if zero, 2 if positive
*/
inline int side(double x) {
    return (x > 0) + (x >= 0);
}

/* Returns 1 if negative, 2 if zero, 4 if positive
*/
inline int sideBit(double x) {
    return 1 << side(x);
}

/* Given the set [0, 1, 2, 3], and two of the four members, compute an XOR mask
   that computes the other two. Note that:
   
   one ^ two == 3 for (0, 3), (1, 2)
   one ^ two <  3 for (0, 1), (0, 2), (1, 3), (2, 3)
   3 - (one ^ two) is either 0, 1, or 2
   1 >> 3 - (one ^ two) is either 0 or 1
thus:
   returned == 2 for (0, 3), (1, 2)
   returned == 3 for (0, 1), (0, 2), (1, 3), (2, 3)
given that:
   (0, 3) ^ 2 -> (2, 1)  (1, 2) ^ 2 -> (3, 0)
   (0, 1) ^ 3 -> (3, 2)  (0, 2) ^ 3 -> (3, 1)  (1, 3) ^ 3 -> (2, 0)  (2, 3) ^ 3 -> (1, 0)
*/
inline int other_two(int one, int two) {
    return 1 >> 3 - (one ^ two) ^ 3;
}

/* Returns -1 if negative, 0 if zero, 1 if positive
*/
inline int sign(double x) {
    return (x > 0) - (x < 0);
}

inline double interp(double A, double B, double t) {
    return A + (B - A) * t;
}
