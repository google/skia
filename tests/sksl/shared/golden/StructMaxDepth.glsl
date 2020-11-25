
struct S1 {
    int x;
};
struct S2 {
    S1 x;
};
struct S3 {
    S2 x;
};
struct S4 {
    S3 x;
};
struct S5 {
    S4 x;
};
struct S6 {
    S5 x;
};
struct S7 {
    S6 x;
};
struct S8 {
    S7 x;
};
in S8 s8;
struct SA1 {
    int[8][7][6][5][4][3][2][1] x;
};
struct SA2 {
    SA1[7][1][1][1][1][1][1][1] x;
};
struct SA3 {
    SA2[6][1][1][1][1][1][1][1] x;
};
struct SA4 {
    SA3[5][1][1][1][1][1][1][1] x;
};
struct SA5 {
    SA4[4][1][1][1][1][1][1][1] x;
};
struct SA6 {
    SA5[3][1][1][1][1][1][1][1] x;
};
struct SA7 {
    SA6[2][1][1][1][1][1][1][1] x;
};
struct SA8 {
    SA7[1][1][1][1][1][1][1][1] x;
};
in SA8 sa8[1][2][3][4][5][6][7][8];
