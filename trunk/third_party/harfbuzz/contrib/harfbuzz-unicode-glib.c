#include "harfbuzz-external.h"

#include <glib.h>

static int
hb_category_for_char(HB_UChar32 ch) {
  switch (g_unichar_type(ch)) {
    case G_UNICODE_CONTROL:
      return HB_Other_Control;
    case G_UNICODE_FORMAT:
      return HB_Other_Format;
    case G_UNICODE_UNASSIGNED:
      return HB_Other_NotAssigned;
    case G_UNICODE_PRIVATE_USE:
      return HB_Other_PrivateUse;
    case G_UNICODE_SURROGATE:
      return HB_Other_Surrogate;
    case G_UNICODE_LOWERCASE_LETTER:
      return HB_Letter_Lowercase;
    case G_UNICODE_MODIFIER_LETTER:
      return HB_Letter_Modifier;
    case G_UNICODE_OTHER_LETTER:
      return HB_Letter_Other;
    case G_UNICODE_TITLECASE_LETTER:
      return HB_Letter_Titlecase;
    case G_UNICODE_UPPERCASE_LETTER:
      return HB_Letter_Uppercase;
    case G_UNICODE_COMBINING_MARK:
      return HB_Mark_SpacingCombining;
    case G_UNICODE_ENCLOSING_MARK:
      return HB_Mark_Enclosing;
    case G_UNICODE_NON_SPACING_MARK:
      return HB_Mark_NonSpacing;
    case G_UNICODE_DECIMAL_NUMBER:
      return HB_Number_DecimalDigit;
    case G_UNICODE_LETTER_NUMBER:
      return HB_Number_Letter;
    case G_UNICODE_OTHER_NUMBER:
      return HB_Number_Other;
    case G_UNICODE_CONNECT_PUNCTUATION:
      return HB_Punctuation_Connector;
    case G_UNICODE_DASH_PUNCTUATION:
      return HB_Punctuation_Dash;
    case G_UNICODE_CLOSE_PUNCTUATION:
      return HB_Punctuation_Close;
    case G_UNICODE_FINAL_PUNCTUATION:
      return HB_Punctuation_FinalQuote;
    case G_UNICODE_INITIAL_PUNCTUATION:
      return HB_Punctuation_InitialQuote;
    case G_UNICODE_OTHER_PUNCTUATION:
      return HB_Punctuation_Other;
    case G_UNICODE_OPEN_PUNCTUATION:
      return HB_Punctuation_Open;
    case G_UNICODE_CURRENCY_SYMBOL:
      return HB_Symbol_Currency;
    case G_UNICODE_MODIFIER_SYMBOL:
      return HB_Symbol_Modifier;
    case G_UNICODE_MATH_SYMBOL:
      return HB_Symbol_Math;
    case G_UNICODE_OTHER_SYMBOL:
      return HB_Symbol_Other;
    case G_UNICODE_LINE_SEPARATOR:
      return HB_Separator_Line;
    case G_UNICODE_PARAGRAPH_SEPARATOR:
      return HB_Separator_Paragraph;
    case G_UNICODE_SPACE_SEPARATOR:
      return HB_Separator_Space;
    default:
      return HB_Symbol_Other;
  }
}

HB_LineBreakClass
HB_GetLineBreakClass(HB_UChar32 ch) {
  switch (g_unichar_break_type(ch)) {
    case G_UNICODE_BREAK_MANDATORY:
      return HB_LineBreak_BK;
    case G_UNICODE_BREAK_CARRIAGE_RETURN:
      return HB_LineBreak_CR;
    case G_UNICODE_BREAK_LINE_FEED:
      return HB_LineBreak_LF;
    case G_UNICODE_BREAK_COMBINING_MARK:
      return HB_LineBreak_CM;
    case G_UNICODE_BREAK_SURROGATE:
      return HB_LineBreak_SG;
    case G_UNICODE_BREAK_ZERO_WIDTH_SPACE:
      return HB_LineBreak_ZW;
    case G_UNICODE_BREAK_INSEPARABLE:
      return HB_LineBreak_IN;
    case G_UNICODE_BREAK_NON_BREAKING_GLUE:
      return HB_LineBreak_GL;
    case G_UNICODE_BREAK_CONTINGENT:
      return HB_LineBreak_AL;
    case G_UNICODE_BREAK_SPACE:
      return HB_LineBreak_SP;
    case G_UNICODE_BREAK_AFTER:
      return HB_LineBreak_BA;
    case G_UNICODE_BREAK_BEFORE:
      return HB_LineBreak_BB;
    case G_UNICODE_BREAK_BEFORE_AND_AFTER:
      return HB_LineBreak_B2;
    case G_UNICODE_BREAK_HYPHEN:
      return HB_LineBreak_HY;
    case G_UNICODE_BREAK_NON_STARTER:
      return HB_LineBreak_NS;
    case G_UNICODE_BREAK_OPEN_PUNCTUATION:
      return HB_LineBreak_OP;
    case G_UNICODE_BREAK_CLOSE_PUNCTUATION:
      return HB_LineBreak_CL;
    case G_UNICODE_BREAK_QUOTATION:
      return HB_LineBreak_QU;
    case G_UNICODE_BREAK_EXCLAMATION:
      return HB_LineBreak_EX;
    case G_UNICODE_BREAK_IDEOGRAPHIC:
      return HB_LineBreak_ID;
    case G_UNICODE_BREAK_NUMERIC:
      return HB_LineBreak_NU;
    case G_UNICODE_BREAK_INFIX_SEPARATOR:
      return HB_LineBreak_IS;
    case G_UNICODE_BREAK_SYMBOL:
      return HB_LineBreak_SY;
    case G_UNICODE_BREAK_ALPHABETIC:
      return HB_LineBreak_AL;
    case G_UNICODE_BREAK_PREFIX:
      return HB_LineBreak_PR;
    case G_UNICODE_BREAK_POSTFIX:
      return HB_LineBreak_PO;
    case G_UNICODE_BREAK_COMPLEX_CONTEXT:
      return HB_LineBreak_SA;
    case G_UNICODE_BREAK_AMBIGUOUS:
      return HB_LineBreak_AL;
    case G_UNICODE_BREAK_UNKNOWN:
      return HB_LineBreak_AL;
    case G_UNICODE_BREAK_NEXT_LINE:
      return HB_LineBreak_AL;
    case G_UNICODE_BREAK_WORD_JOINER:
      return HB_LineBreak_WJ;
    case G_UNICODE_BREAK_HANGUL_L_JAMO:
      return HB_LineBreak_JL;
    case G_UNICODE_BREAK_HANGUL_V_JAMO:
      return HB_LineBreak_JV;
    case G_UNICODE_BREAK_HANGUL_T_JAMO:
      return HB_LineBreak_JT;
    case G_UNICODE_BREAK_HANGUL_LV_SYLLABLE:
      return HB_LineBreak_H2;
    case G_UNICODE_BREAK_HANGUL_LVT_SYLLABLE:
      return HB_LineBreak_H3;
    default:
      return HB_LineBreak_AL;
  }
}

int
HB_GetUnicodeCharCombiningClass(HB_UChar32 ch) {
  return g_unichar_combining_class(ch);
}

void
HB_GetUnicodeCharProperties(HB_UChar32 ch,
                            HB_CharCategory *category,
                            int *combiningClass) {
  *category = hb_category_for_char(ch);
  *combiningClass = g_unichar_combining_class(ch);
}

HB_CharCategory
HB_GetUnicodeCharCategory(HB_UChar32 ch) {
  return hb_category_for_char(ch);
}
