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

/* Given: r1 = Resultant[
 *      a*(1 - t)^3 + 3*b*(1 - t)^2*t + 3*c*(1 - t)*t^2 + d*t^3 - x, 
 *      e*(1 - t)^3 + 3*f*(1 - t)^2*t + 3*g*(1 - t)*t^2 + h*t^3 - y, t]
 *        Collect[r1, {x, y}, Simplify]
 *        CForm[%]
 *      then use regex to replace Power\(([a-h]),3\) with \1*\1*\1
 *                            and Power\(([a-h]),2\) with \1*\1
 * yields:
 
d*d*d*e*e*e - 3*d*d*(3*c*e*e*f + 3*b*e*(-3*f*f + 2*e*g) + a*(9*f*f*f - 9*e*f*g + e*e*h)) - 
   h*(27*c*c*c*e*e - 27*c*c*(3*b*e*f - 3*a*f*f + 2*a*e*g) + 
      h*(-27*b*b*b*e + 27*a*b*b*f - 9*a*a*b*g + a*a*a*h) + 
      9*c*(9*b*b*e*g + a*b*(-9*f*g + 3*e*h) + a*a*(3*g*g - 2*f*h))) + 
   3*d*(9*c*c*e*e*g + 9*b*b*e*(3*g*g - 2*f*h) + 3*a*b*(-9*f*g*g + 6*f*f*h + e*g*h) + 
      a*a*(9*g*g*g - 9*f*g*h + e*h*h) + 3*c*(3*b*e*(-3*f*g + e*h) + a*(9*f*f*g - 6*e*g*g - e*f*h)))
      
- Power(e - 3*f + 3*g - h,3)*Power(x,3) 
   
+ 3*(6*b*d*d*e*e - d*d*d*e*e + 18*b*b*d*e*f - 18*b*d*d*e*f - 
      9*b*d*d*f*f - 54*b*b*d*e*g + 12*b*d*d*e*g - 27*b*b*d*g*g - 18*b*b*b*e*h + 18*b*b*d*e*h + 
      18*b*b*d*f*h + a*a*a*h*h - 9*b*b*b*h*h + 9*c*c*c*e*(e + 2*h) + 
      a*a*(-3*b*h*(2*g + h) + d*(-27*g*g + 9*g*h - h*(2*e + h) + 9*f*(g + h))) + 
      a*(9*b*b*h*(2*f + h) - 3*b*d*(6*f*f - 6*f*(3*g - 2*h) + g*(-9*g + h) + e*(g + h)) + 
         d*d*(e*e + 9*f*(3*f - g) + e*(-9*f - 9*g + 2*h))) - 
      9*c*c*(d*e*(e + 2*g) + 3*b*(f*h + e*(f + h)) + a*(-3*f*f - 6*f*h + 2*(g*h + e*(g + h)))) + 
      3*c*(d*d*e*(e + 2*f) + a*a*(3*g*g + 6*g*h - 2*h*(2*f + h)) + 9*b*b*(g*h + e*(g + h)) + 
         a*d*(-9*f*f - 18*f*g + 6*g*g + f*h + e*(f + 12*g + h)) + 
         b*(d*(-3*e*e + 9*f*g + e*(9*f + 9*g - 6*h)) + 3*a*(h*(2*e - 3*g + h) - 3*f*(g + h)))))*y 
         
- 3*(18*c*c*c*e - 18*c*c*d*e + 6*c*d*d*e - d*d*d*e + 3*c*d*d*f - 9*c*c*d*g + a*a*a*h + 9*c*c*c*h - 
      9*b*b*b*(e + 2*h) - a*a*(d*(e - 9*f + 18*g - 7*h) + 3*c*(2*f - 6*g + h)) + 
      a*(-9*c*c*(2*e - 6*f + 2*g - h) + d*d*(-7*e + 18*f - 9*g + h) + 3*c*d*(7*e - 17*f + 3*g + h)) + 
      9*b*b*(3*c*(e + g + h) + a*(f + 2*h) - d*(e - 2*(f - 3*g + h))) - 
      3*b*(-(d*d*(e - 6*f + 2*g)) - 3*c*d*(e + 3*f + 3*g - h) + 9*c*c*(e + f + h) + a*a*(g + 2*h) + 
         a*(c*(-3*e + 9*f + 9*g + 3*h) + d*(e + 3*f - 17*g + 7*h))))*Power(y,2) 
         
+ Power(a - 3*b + 3*c - d,3)*Power(y,3) 
         
+ Power(x,2)*(-3*(-9*b*e*f*f + 9*a*f*f*f + 6*b*e*e*g - 9*a*e*f*g + 27*b*e*f*g - 27*a*f*f*g + 18*a*e*g*g - 54*b*e*g*g + 
         27*a*f*g*g + 27*b*f*g*g - 18*a*g*g*g + a*e*e*h - 9*b*e*e*h + 3*a*e*f*h + 9*b*e*f*h + 9*a*f*f*h - 
         18*b*f*f*h - 21*a*e*g*h + 51*b*e*g*h - 9*a*f*g*h - 27*b*f*g*h + 18*a*g*g*h + 7*a*e*h*h - 18*b*e*h*h - 3*a*f*h*h + 
         18*b*f*h*h - 6*a*g*h*h - 3*b*g*h*h + a*h*h*h + 
         3*c*(-9*f*f*(g - 2*h) + 3*g*g*h - f*h*(9*g + 2*h) + e*e*(f - 6*g + 6*h) + 
            e*(9*f*g + 6*g*g - 17*f*h - 3*g*h + 3*h*h)) - 
         d*(e*e*e + e*e*(-6*f - 3*g + 7*h) - 9*(2*f - g)*(f*f + g*g - f*(g + h)) + 
            e*(18*f*f + 9*g*g + 3*g*h + h*h - 3*f*(3*g + 7*h)))) )
            
+ Power(x,2)*(3*(a - 3*b + 3*c - d)*Power(e - 3*f + 3*g - h,2)*y)
            
+ x*(-3*(27*b*b*e*g*g - 27*a*b*f*g*g + 9*a*a*g*g*g - 18*b*b*e*f*h + 18*a*b*f*f*h + 3*a*b*e*g*h - 
         27*b*b*e*g*h - 9*a*a*f*g*h + 27*a*b*f*g*h - 9*a*a*g*g*h + a*a*e*h*h - 9*a*b*e*h*h + 
         27*b*b*e*h*h + 6*a*a*f*h*h - 18*a*b*f*h*h - 9*b*b*f*h*h + 3*a*a*g*h*h + 
         6*a*b*g*h*h - a*a*h*h*h + 9*c*c*(e*e*(g - 3*h) - 3*f*f*h + e*(3*f + 2*g)*h) + 
         d*d*(e*e*e - 9*f*f*f + 9*e*f*(f + g) - e*e*(3*f + 6*g + h)) + 
         d*(-3*c*(-9*f*f*g + e*e*(2*f - 6*g - 3*h) + e*(9*f*g + 6*g*g + f*h)) + 
            a*(-18*f*f*f - 18*e*g*g + 18*g*g*g - 2*e*e*h + 3*e*g*h + 2*e*h*h + 9*f*f*(3*g + 2*h) + 
               3*f*(6*e*g - 9*g*g - e*h - 6*g*h)) - 3*b*(9*f*g*g + e*e*(4*g - 3*h) - 6*f*f*h - 
               e*(6*f*f + g*(18*g + h) - 3*f*(3*g + 4*h)))) + 
         3*c*(3*b*(e*e*h + 3*f*g*h - e*(3*f*g - 6*f*h + 6*g*h + h*h)) + 
            a*(9*f*f*(g - 2*h) + f*h*(-e + 9*g + 4*h) - 3*(2*g*g*h + e*(2*g*g - 4*g*h + h*h))))) )
            
+ x*3*(-2*a*d*e*e - 7*d*d*e*e + 15*a*d*e*f + 21*d*d*e*f - 9*a*d*f*f - 18*d*d*f*f - 15*a*d*e*g - 
         3*d*d*e*g - 9*a*a*f*g + 9*d*d*f*g + 18*a*a*g*g + 9*a*d*g*g + 2*a*a*e*h - 2*d*d*e*h + 
         3*a*a*f*h + 15*a*d*f*h - 21*a*a*g*h - 15*a*d*g*h + 7*a*a*h*h + 2*a*d*h*h - 
         9*c*c*(2*e*e + 3*f*f + 3*f*h - 2*g*h + e*(-3*f - 4*g + h)) + 
         9*b*b*(3*g*g - 3*g*h + 2*h*(-2*f + h) + e*(-2*f + 3*g + h)) + 
         3*b*(3*c*(e*e + 3*e*(f - 3*g) + (9*f - 3*g - h)*h) + a*(6*f*f + e*g - 9*f*g - 9*g*g - 5*e*h + 9*f*h + 14*g*h - 7*h*h) + 
            d*(-e*e + 12*f*f - 27*f*g + e*(-9*f + 20*g - 5*h) + g*(9*g + h))) + 
         3*c*(a*(-(e*f) - 9*f*f + 27*f*g - 12*g*g + 5*e*h - 20*f*h + 9*g*h + h*h) + 
            d*(7*e*e + 9*f*f + 9*f*g - 6*g*g - f*h + e*(-14*f - 9*g + 5*h))))*y 
            
- x*3*Power(a - 3*b + 3*c - d,2)*(e - 3*f + 3*g - h)*Power(y,2)
  
*/
   
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
