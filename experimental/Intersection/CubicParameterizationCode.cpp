#include <vector>

/* Given:
 * Resultant[a*t^3 + b*t^2 + c*t + d - x, e*t^3 + f*t^2 + g*t + h - y, t]
 */

const char result1[] =
"-d^3 e^3 + c d^2 e^2 f - b d^2 e f^2 + a d^2 f^3 - c^2 d e^2 g + "
" 2 b d^2 e^2 g + b c d e f g - 3 a d^2 e f g - a c d f^2 g - "
" b^2 d e g^2 + 2 a c d e g^2 + a b d f g^2 - a^2 d g^3 + c^3 e^2 h - "
" 3 b c d e^2 h + 3 a d^2 e^2 h - b c^2 e f h + 2 b^2 d e f h + "
" a c d e f h + a c^2 f^2 h - 2 a b d f^2 h + b^2 c e g h - "
" 2 a c^2 e g h - a b d e g h - a b c f g h + 3 a^2 d f g h + "
" a^2 c g^2 h - b^3 e h^2 + 3 a b c e h^2 - 3 a^2 d e h^2 + "
" a b^2 f h^2 - 2 a^2 c f h^2 - a^2 b g h^2 + a^3 h^3 + 3 d^2 e^3 x - "
" 2 c d e^2 f x + 2 b d e f^2 x - 2 a d f^3 x + c^2 e^2 g x - "
" 4 b d e^2 g x - b c e f g x + 6 a d e f g x + a c f^2 g x + "
" b^2 e g^2 x - 2 a c e g^2 x - a b f g^2 x + a^2 g^3 x + "
" 3 b c e^2 h x - 6 a d e^2 h x - 2 b^2 e f h x - a c e f h x + "
" 2 a b f^2 h x + a b e g h x - 3 a^2 f g h x + 3 a^2 e h^2 x - "
" 3 d e^3 x^2 + c e^2 f x^2 - b e f^2 x^2 + a f^3 x^2 + "
" 2 b e^2 g x^2 - 3 a e f g x^2 + 3 a e^2 h x^2 + e^3 x^3 - "
" c^3 e^2 y + 3 b c d e^2 y - 3 a d^2 e^2 y + b c^2 e f y - "
" 2 b^2 d e f y - a c d e f y - a c^2 f^2 y + 2 a b d f^2 y - "
" b^2 c e g y + 2 a c^2 e g y + a b d e g y + a b c f g y - "
" 3 a^2 d f g y - a^2 c g^2 y + 2 b^3 e h y - 6 a b c e h y + "
" 6 a^2 d e h y - 2 a b^2 f h y + 4 a^2 c f h y + 2 a^2 b g h y - "
" 3 a^3 h^2 y - 3 b c e^2 x y + 6 a d e^2 x y + 2 b^2 e f x y + "
" a c e f x y - 2 a b f^2 x y - a b e g x y + 3 a^2 f g x y - "
" 6 a^2 e h x y - 3 a e^2 x^2 y - b^3 e y^2 + 3 a b c e y^2 - "
" 3 a^2 d e y^2 + a b^2 f y^2 - 2 a^2 c f y^2 - a^2 b g y^2 + "
" 3 a^3 h y^2 + 3 a^2 e x y^2 - a^3 y^3";

const size_t len1 = sizeof(result1) - 1;

/* Given:
 * Expand[
 * Det[{{a, b, c, (d - x),      0,      0},
 *      {0, a, b,      c,  (d - x),     0},
 *      {0, 0, a,      b,       c, (d - x)},
 *      {e, f, g, (h - y),      0,      0},
 *      {0, e, f,      g,  (h - y),     0},
 *      {0, 0, e,      f,       g, (h - y)}}]]
 */
 // result1 and result2 are the same. 102 factors:
const char result2[] =
"-d^3 e^3       + c d^2 e^2 f   - b d^2 e f^2   + a d^2 f^3     - c^2 d e^2 g + "
" 2 b d^2 e^2 g + b c d e f g   - 3 a d^2 e f g - a c d f^2 g   - "
" b^2 d e g^2   + 2 a c d e g^2 + a b d f g^2   - a^2 d g^3     + c^3 e^2 h - "
" 3 b c d e^2 h + 3 a d^2 e^2 h - b c^2 e f h   + 2 b^2 d e f h + "
" a c d e f h   + a c^2 f^2 h   - 2 a b d f^2 h + b^2 c e g h   - "
" 2 a c^2 e g h - a b d e g h   - a b c f g h   + 3 a^2 d f g h + "
" a^2 c g^2 h   - b^3 e h^2     + 3 a b c e h^2 - 3 a^2 d e h^2 + "
" a b^2 f h^2   - 2 a^2 c f h^2 - a^2 b g h^2   + a^3 h^3       + 3 d^2 e^3 x - "
" 2 c d e^2 f x + 2 b d e f^2 x - 2 a d f^3 x   + c^2 e^2 g x   - "
" 4 b d e^2 g x - b c e f g x   + 6 a d e f g x + a c f^2 g x   + "
" b^2 e g^2 x   - 2 a c e g^2 x - a b f g^2 x   + a^2 g^3 x     + "
" 3 b c e^2 h x - 6 a d e^2 h x - 2 b^2 e f h x - a c e f h x   + "
" 2 a b f^2 h x + a b e g h x   - 3 a^2 f g h x + 3 a^2 e h^2 x - "
" 3 d e^3 x^2   + c e^2 f x^2   - b e f^2 x^2   + a f^3 x^2     + "
" 2 b e^2 g x^2 - 3 a e f g x^2 + 3 a e^2 h x^2 + e^3 x^3       - "
" c^3 e^2 y     + 3 b c d e^2 y - 3 a d^2 e^2 y + b c^2 e f y   - "
" 2 b^2 d e f y - a c d e f y   - a c^2 f^2 y   + 2 a b d f^2 y - "
" b^2 c e g y   + 2 a c^2 e g y + a b d e g y   + a b c f g y   - "
" 3 a^2 d f g y - a^2 c g^2 y   + 2 b^3 e h y   - 6 a b c e h y + "
" 6 a^2 d e h y - 2 a b^2 f h y + 4 a^2 c f h y + 2 a^2 b g h y - "
" 3 a^3 h^2 y   - 3 b c e^2 x y + 6 a d e^2 x y + 2 b^2 e f x y + "
" a c e f x y   - 2 a b f^2 x y - a b e g x y   + 3 a^2 f g x y - "
" 6 a^2 e h x y - 3 a e^2 x^2 y - b^3 e y^2     + 3 a b c e y^2 - "
" 3 a^2 d e y^2 + a b^2 f y^2   - 2 a^2 c f y^2 - a^2 b g y^2   + "
" 3 a^3 h y^2   + 3 a^2 e x y^2 - a^3 y^3";


const size_t len2 = sizeof(result2) - 1;

const int factors = 8;

struct coeff {
    int s; // constant and coefficient sign
    int n[factors]; // 0 or power of a (1, 2, or 3) for a through h
};

enum {
    xxx_coeff,
    xxy_coeff,
    xyy_coeff,
    yyy_coeff,
    xx_coeff,
    xy_coeff,
    yy_coeff,
    x_coeff,
    y_coeff,
    c_coeff,
    coeff_count
};

typedef std::vector<coeff> coeffs;
typedef std::vector<coeffs> n_coeffs;

static char skipSpace(const char* str, size_t& index) {
    do {
        ++index;
    } while (str[index] == ' ');
    return str[index];
}

static char backSkipSpace(const char* str, size_t& end) {
    while (str[end - 1] == ' ') {
        --end;
    }
    return str[end - 1];
}

static void match(const char* str, size_t len, coeffs& co, const char pattern[]) {
    size_t patternLen = strlen(pattern);
    size_t index = 0;
    while (index < len) {
        char ch = str[index];
        if (ch != '-' && ch != '+') {
            printf("missing sign\n");
        }
        size_t end = index + 1;
        while (str[end] != '+' && str[end] != '-' && ++end < len) {
            ;
        }
        backSkipSpace(str, end);
        size_t idx = index;
        index = end;
        skipSpace(str, index);
        if (!strncmp(&str[end - patternLen], pattern, patternLen) == 0) {
            continue;
        }
        size_t endCoeff = end - patternLen;
        char last = backSkipSpace(str, endCoeff);
        if (last == '2' || last == '3') {
            last = str[endCoeff - 3]; // skip ^2
        }
        if (last == 'x' || last == 'y') {
            continue;
        }
        coeff c;
        c.s = str[idx] == '-' ? -1 : 1;
        bzero(c.n, sizeof(c.n));
        ch = skipSpace(str, idx);
        if (ch >= '2' && ch <= '6') {
            c.s *= ch - '0';
            ch = skipSpace(str, idx);
        }
        while (idx < endCoeff) {
            char x = str[idx];
            if (x < 'a' || x > 'a' + factors) {
                printf("expected factor\n");
            }
            idx++;
            int pow = 1;
            if (str[idx] == '^') {
                idx++;
                char exp = str[idx];
                if (exp < '2' || exp > '3') {
                    printf("expected exponent\n");
                }
                pow = exp - '0';
            }
            skipSpace(str, idx);
            c.n[x - 'a'] = pow;
        }
        co.push_back(c);
    }
}

void cubecode_test(int test);

void cubecode_test(int test) {
    const char* str = test ? result2 : result1;
    size_t len = strlen(str);
    n_coeffs c(coeff_count);
    match(str, len, c[xxx_coeff], "x^3");   // 1 factor
    match(str, len, c[xxy_coeff], "x^2 y"); // 1 factor
    match(str, len, c[xyy_coeff], "x y^2"); // 1 factor
    match(str, len, c[yyy_coeff], "y^3");   // 1 factor
    match(str, len, c[xx_coeff], "x^2");    // 7 factors
    match(str, len, c[xy_coeff], "x y");    // 8 factors
    match(str, len, c[yy_coeff], "y^2");    // 7 factors
    match(str, len, c[x_coeff], "x");       // 21 factors
    match(str, len, c[y_coeff], "y");       // 21 factors
    match(str, len, c[c_coeff], "");        // 34 factors : total 102
#define COMPUTE_MOST_FREQUENT_EXPRESSION_TRIPLETS 0
#define WRITE_AS_NONOPTIMIZED_C_CODE 0
#if COMPUTE_MOST_FREQUENT_EXPRESSION_TRIPLETS
    int count[factors][factors][factors];
    bzero(count, sizeof(count));
#endif
#if WRITE_AS_NONOPTIMIZED_C_CODE
    printf("// start of generated code");
#endif
    for (n_coeffs::iterator it = c.begin(); it < c.end(); ++it) {
        coeffs& co = *it;
#if WRITE_AS_NONOPTIMIZED_C_CODE
        printf("\nstatic double calc_%c(double a, double b, double c, double d,"
               "\n                     double e, double f, double g, double h) {"
               "\n    return"
               "\n ", 'A' + (it - c.begin()));
        if (co[0].s > 0) {
            printf(" ");
        } 
        if (abs(co[0].s) == 1) {
            printf("    ");
        }
#endif
        for (coeffs::iterator ct = co.begin(); ct < co.end(); ++ct) {
            const coeff& cf = *ct;
#if WRITE_AS_NONOPTIMIZED_C_CODE
            printf("        ");
            bool firstFactor = false;
            if (ct - co.begin() > 0 || cf.s < 0) {
                printf("%c", cf.s < 0 ? '-' : '+');
            }
            if (ct - co.begin() > 0) {
                printf(" ");
            }
            if (abs(cf.s) > 1) {
                printf("%d * ", abs(cf.s));
            } else {
                if (ct - co.begin() > 0) {
                    printf("    ");
                }
            }
#endif
            for (int x = 0; x < factors; ++x) {
                if (cf.n[x] == 0) {
                    continue;
                }
#if WRITE_AS_NONOPTIMIZED_C_CODE
                for (int y = 0 ; y < cf.n[x]; ++y) {
                    if (y > 0 || firstFactor) {
                        printf(" * ");
                    }
                    printf("%c", 'a' + x);
                }
                firstFactor = true;
#endif
#if COMPUTE_MOST_FREQUENT_EXPRESSION_TRIPLETS
                for (int y = x; y < factors; ++y) {
                    if (cf.n[y] == 0) {
                        continue;
                    }
                    if (x == y && cf.n[y] == 1) {
                        continue;
                    }
                    for (int z = y; z < factors; ++z) {
                        if (cf.n[z] == 0) {
                            continue;
                        }
                        if ((x == z || y == z) && cf.n[z] == 1) {
                            continue;
                        }
                        if (x == y && y == z && cf.n[z] == 2) {
                            continue;
                        }
                        count[x][y][z]++;
                    }
                }
#endif
            }
#if WRITE_AS_NONOPTIMIZED_C_CODE
            if (ct + 1 < co.end()) {
                printf("\n");
            }
#endif
        }
#if WRITE_AS_NONOPTIMIZED_C_CODE
            printf(";\n}\n");
#endif
    }
#if WRITE_AS_NONOPTIMIZED_C_CODE
    printf("// end of generated code\n");
#endif
#if COMPUTE_MOST_FREQUENT_EXPRESSION_TRIPLETS
    const int bestCount = 20;
    int best[bestCount][4];
    bzero(best, sizeof(best));
    for (int x = 0; x < factors; ++x) {
        for (int y = x; y < factors; ++y) {
            for (int z = y; z < factors; ++z) {
                if (!count[x][y][z]) {
                    continue;
                }
                for (int w = 0; w < bestCount; ++w) {
                    if (best[w][0] < count[x][y][z]) {
                        best[w][0] = count[x][y][z];
                        best[w][1] = x;
                        best[w][2] = y;
                        best[w][3] = z;
                        break;
                    }
                }
            }
        }
    }
    for (int w = 0; w < bestCount; ++w) {
        printf("%c%c%c=%d\n", 'a' + best[w][1], 'a'  + best[w][2],
            'a' + best[w][3], best[w][0]);
    }
#endif
#if WRITE_AS_NONOPTIMIZED_C_CODE
    printf("\n");
#endif
}

/* results: variable triplets used 10 or more times:
aah=14
ade=14
aeh=14
dee=14
bce=13
beg=13
beh=12
bbe=11
bef=11
cee=11
cef=11
def=11
ceh=10
deg=10
*/
