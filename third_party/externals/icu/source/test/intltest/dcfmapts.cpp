/********************************************************************
 * COPYRIGHT:
 * Copyright (c) 1997-2014, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING

#include "dcfmapts.h"

#include "unicode/currpinf.h"
#include "unicode/dcfmtsym.h"
#include "unicode/decimfmt.h"
#include "unicode/fmtable.h"
#include "unicode/localpointer.h"
#include "unicode/parseerr.h"
#include "unicode/stringpiece.h"

#include "putilimp.h"
#include "plurrule_impl.h"
#include <stdio.h>

// This is an API test, not a unit test.  It doesn't test very many cases, and doesn't
// try to test the full functionality.  It just calls each function in the class and
// verifies that it works on a basic level.

void IntlTestDecimalFormatAPI::runIndexedTest( int32_t index, UBool exec, const char* &name, char* /*par*/ )
{
    if (exec) logln((UnicodeString)"TestSuite DecimalFormatAPI");
    switch (index) {
        case 0: name = "DecimalFormat API test";
                if (exec) {
                    logln((UnicodeString)"DecimalFormat API test---"); logln((UnicodeString)"");
                    UErrorCode status = U_ZERO_ERROR;
                    Locale saveLocale;
                    Locale::setDefault(Locale::getEnglish(), status);
                    if(U_FAILURE(status)) {
                        errln((UnicodeString)"ERROR: Could not set default locale, test may not give correct results");
                    }
                    testAPI(/*par*/);
                    Locale::setDefault(saveLocale, status);
                }
                break;
        case 1: name = "Rounding test";
            if(exec) {
               logln((UnicodeString)"DecimalFormat Rounding test---");
               testRounding(/*par*/);
            }
            break;
        case 2: name = "Test6354";
            if(exec) {
               logln((UnicodeString)"DecimalFormat Rounding Increment test---");
               testRoundingInc(/*par*/);
            }
            break;
        case 3: name = "TestCurrencyPluralInfo";
            if(exec) {
               logln((UnicodeString)"CurrencyPluralInfo API test---");
               TestCurrencyPluralInfo();
            }
            break;
        case 4: name = "TestScale";
            if(exec) {
               logln((UnicodeString)"Scale test---");
               TestScale();
            }
            break;
         case 5: name = "TestFixedDecimal";
            if(exec) {
               logln((UnicodeString)"TestFixedDecimal ---");
               TestFixedDecimal();
            }
            break;
         case 6: name = "TestBadFastpath";
            if(exec) {
               logln((UnicodeString)"TestBadFastpath ---");
               TestBadFastpath();
            }
            break;
         case 7: name = "TestRequiredDecimalPoint";
            if(exec) {
               logln((UnicodeString)"TestRequiredDecimalPoint ---");
               TestRequiredDecimalPoint();
            }
            break;
       default: name = ""; break;
    }
}

/**
 * This test checks various generic API methods in DecimalFormat to achieve 100%
 * API coverage.
 */
void IntlTestDecimalFormatAPI::testAPI(/*char *par*/)
{
    UErrorCode status = U_ZERO_ERROR;

// ======= Test constructors

    logln((UnicodeString)"Testing DecimalFormat constructors");

    DecimalFormat def(status);
    if(U_FAILURE(status)) {
        errcheckln(status, "ERROR: Could not create DecimalFormat (default) - %s", u_errorName(status));
        return;
    }

    // bug 10864
    status = U_ZERO_ERROR;
    DecimalFormat noGrouping("###0.##", status);
    if (noGrouping.getGroupingSize() != 0) {
      errln("Grouping size should be 0 for no grouping.");
    }
    // end bug 10864

    status = U_ZERO_ERROR;
    const UnicodeString pattern("#,##0.# FF");
    DecimalFormat pat(pattern, status);
    if(U_FAILURE(status)) {
        errln((UnicodeString)"ERROR: Could not create DecimalFormat (pattern)");
        return;
    }

    status = U_ZERO_ERROR;
    DecimalFormatSymbols *symbols = new DecimalFormatSymbols(Locale::getFrench(), status);
    if(U_FAILURE(status)) {
        errln((UnicodeString)"ERROR: Could not create DecimalFormatSymbols (French)");
        return;
    }

    status = U_ZERO_ERROR;
    DecimalFormat cust1(pattern, symbols, status);
    if(U_FAILURE(status)) {
        errln((UnicodeString)"ERROR: Could not create DecimalFormat (pattern, symbols*)");
    }

    status = U_ZERO_ERROR;
    DecimalFormat cust2(pattern, *symbols, status);
    if(U_FAILURE(status)) {
        errln((UnicodeString)"ERROR: Could not create DecimalFormat (pattern, symbols)");
    }

    DecimalFormat copy(pat);

// ======= Test clone(), assignment, and equality

    logln((UnicodeString)"Testing clone(), assignment and equality operators");

    if( ! (copy == pat) || copy != pat) {
        errln((UnicodeString)"ERROR: Copy constructor or == failed");
    }

    copy = cust1;
    if(copy != cust1) {
        errln((UnicodeString)"ERROR: Assignment (or !=) failed");
    }

    Format *clone = def.clone();
    if( ! (*clone == def) ) {
        errln((UnicodeString)"ERROR: Clone() failed");
    }
    delete clone;

// ======= Test various format() methods

    logln((UnicodeString)"Testing various format() methods");

    double d = -10456.0037;
    int32_t l = 100000000;
    Formattable fD(d);
    Formattable fL(l);

    UnicodeString res1, res2, res3, res4;
    FieldPosition pos1(0), pos2(0), pos3(0), pos4(0);

    res1 = def.format(d, res1, pos1);
    logln( (UnicodeString) "" + (int32_t) d + " formatted to " + res1);

    res2 = pat.format(l, res2, pos2);
    logln((UnicodeString) "" + (int32_t) l + " formatted to " + res2);

    status = U_ZERO_ERROR;
    res3 = cust1.format(fD, res3, pos3, status);
    if(U_FAILURE(status)) {
        errln((UnicodeString)"ERROR: format(Formattable [double]) failed");
    }
    logln((UnicodeString) "" + (int32_t) fD.getDouble() + " formatted to " + res3);

    status = U_ZERO_ERROR;
    res4 = cust2.format(fL, res4, pos4, status);
    if(U_FAILURE(status)) {
        errln((UnicodeString)"ERROR: format(Formattable [long]) failed");
    }
    logln((UnicodeString) "" + fL.getLong() + " formatted to " + res4);

// ======= Test parse()

    logln((UnicodeString)"Testing parse()");

    UnicodeString text("-10,456.0037");
    Formattable result1, result2;
    ParsePosition pos(0);
    UnicodeString patt("#,##0.#");
    status = U_ZERO_ERROR;
    pat.applyPattern(patt, status);
    if(U_FAILURE(status)) {
        errln((UnicodeString)"ERROR: applyPattern() failed");
    }
    pat.parse(text, result1, pos);
    if(result1.getType() != Formattable::kDouble && result1.getDouble() != d) {
        errln((UnicodeString)"ERROR: Roundtrip failed (via parse()) for " + text);
    }
    logln(text + " parsed into " + (int32_t) result1.getDouble());

    status = U_ZERO_ERROR;
    pat.parse(text, result2, status);
    if(U_FAILURE(status)) {
        errln((UnicodeString)"ERROR: parse() failed");
    }
    if(result2.getType() != Formattable::kDouble && result2.getDouble() != d) {
        errln((UnicodeString)"ERROR: Roundtrip failed (via parse()) for " + text);
    }
    logln(text + " parsed into " + (int32_t) result2.getDouble());

// ======= Test getters and setters

    logln((UnicodeString)"Testing getters and setters");

    const DecimalFormatSymbols *syms = pat.getDecimalFormatSymbols();
    DecimalFormatSymbols *newSyms = new DecimalFormatSymbols(*syms);
    def.setDecimalFormatSymbols(*newSyms);
    def.adoptDecimalFormatSymbols(newSyms); // don't use newSyms after this
    if( *(pat.getDecimalFormatSymbols()) != *(def.getDecimalFormatSymbols())) {
        errln((UnicodeString)"ERROR: adopt or set DecimalFormatSymbols() failed");
    }

    UnicodeString posPrefix;
    pat.setPositivePrefix("+");
    posPrefix = pat.getPositivePrefix(posPrefix);
    logln((UnicodeString)"Positive prefix (should be +): " + posPrefix);
    if(posPrefix != "+") {
        errln((UnicodeString)"ERROR: setPositivePrefix() failed");
    }

    UnicodeString negPrefix;
    pat.setNegativePrefix("-");
    negPrefix = pat.getNegativePrefix(negPrefix);
    logln((UnicodeString)"Negative prefix (should be -): " + negPrefix);
    if(negPrefix != "-") {
        errln((UnicodeString)"ERROR: setNegativePrefix() failed");
    }

    UnicodeString posSuffix;
    pat.setPositiveSuffix("_");
    posSuffix = pat.getPositiveSuffix(posSuffix);
    logln((UnicodeString)"Positive suffix (should be _): " + posSuffix);
    if(posSuffix != "_") {
        errln((UnicodeString)"ERROR: setPositiveSuffix() failed");
    }

    UnicodeString negSuffix;
    pat.setNegativeSuffix("~");
    negSuffix = pat.getNegativeSuffix(negSuffix);
    logln((UnicodeString)"Negative suffix (should be ~): " + negSuffix);
    if(negSuffix != "~") {
        errln((UnicodeString)"ERROR: setNegativeSuffix() failed");
    }

    int32_t multiplier = 0;
    pat.setMultiplier(8);
    multiplier = pat.getMultiplier();
    logln((UnicodeString)"Multiplier (should be 8): " + multiplier);
    if(multiplier != 8) {
        errln((UnicodeString)"ERROR: setMultiplier() failed");
    }

    int32_t groupingSize = 0;
    pat.setGroupingSize(2);
    groupingSize = pat.getGroupingSize();
    logln((UnicodeString)"Grouping size (should be 2): " + (int32_t) groupingSize);
    if(groupingSize != 2) {
        errln((UnicodeString)"ERROR: setGroupingSize() failed");
    }

    pat.setDecimalSeparatorAlwaysShown(TRUE);
    UBool tf = pat.isDecimalSeparatorAlwaysShown();
    logln((UnicodeString)"DecimalSeparatorIsAlwaysShown (should be TRUE) is " + (UnicodeString) (tf ? "TRUE" : "FALSE"));
    if(tf != TRUE) {
        errln((UnicodeString)"ERROR: setDecimalSeparatorAlwaysShown() failed");
    }
    // Added by Ken Liu testing set/isExponentSignAlwaysShown
    pat.setExponentSignAlwaysShown(TRUE);
    UBool esas = pat.isExponentSignAlwaysShown();
    logln((UnicodeString)"ExponentSignAlwaysShown (should be TRUE) is " + (UnicodeString) (esas ? "TRUE" : "FALSE"));
    if(esas != TRUE) {
        errln((UnicodeString)"ERROR: ExponentSignAlwaysShown() failed");
    }

    // Added by Ken Liu testing set/isScientificNotation
    pat.setScientificNotation(TRUE);
    UBool sn = pat.isScientificNotation();
    logln((UnicodeString)"isScientificNotation (should be TRUE) is " + (UnicodeString) (sn ? "TRUE" : "FALSE"));
    if(sn != TRUE) {
        errln((UnicodeString)"ERROR: setScientificNotation() failed");
    }

    // Added by Ken Liu testing set/getMinimumExponentDigits
    int8_t MinimumExponentDigits = 0;
    pat.setMinimumExponentDigits(2);
    MinimumExponentDigits = pat.getMinimumExponentDigits();
    logln((UnicodeString)"MinimumExponentDigits (should be 2) is " + (int8_t) MinimumExponentDigits);
    if(MinimumExponentDigits != 2) {
        errln((UnicodeString)"ERROR: setMinimumExponentDigits() failed");
    }

    // Added by Ken Liu testing set/getRoundingIncrement
    double RoundingIncrement = 0.0;
    pat.setRoundingIncrement(2.0);
    RoundingIncrement = pat.getRoundingIncrement();
    logln((UnicodeString)"RoundingIncrement (should be 2.0) is " + (double) RoundingIncrement);
    if(RoundingIncrement != 2.0) {
        errln((UnicodeString)"ERROR: setRoundingIncrement() failed");
    }
    //end of Ken's Adding

    UnicodeString funkyPat;
    funkyPat = pat.toPattern(funkyPat);
    logln((UnicodeString)"Pattern is " + funkyPat);

    UnicodeString locPat;
    locPat = pat.toLocalizedPattern(locPat);
    logln((UnicodeString)"Localized pattern is " + locPat);

// ======= Test applyPattern()

    logln((UnicodeString)"Testing applyPattern()");

    UnicodeString p1("#,##0.0#;(#,##0.0#)");
    logln((UnicodeString)"Applying pattern " + p1);
    status = U_ZERO_ERROR;
    pat.applyPattern(p1, status);
    if(U_FAILURE(status)) {
        errln((UnicodeString)"ERROR: applyPattern() failed with " + (int32_t) status);
    }
    UnicodeString s2;
    s2 = pat.toPattern(s2);
    logln((UnicodeString)"Extracted pattern is " + s2);
    if(s2 != p1) {
        errln((UnicodeString)"ERROR: toPattern() result did not match pattern applied");
    }

    if(pat.getSecondaryGroupingSize() != 0) {
        errln("FAIL: Secondary Grouping Size should be 0, not %d\n", pat.getSecondaryGroupingSize());
    }

    if(pat.getGroupingSize() != 3) {
        errln("FAIL: Primary Grouping Size should be 3, not %d\n", pat.getGroupingSize());
    }

    UnicodeString p2("#,##,##0.0# FF;(#,##,##0.0# FF)");
    logln((UnicodeString)"Applying pattern " + p2);
    status = U_ZERO_ERROR;
    pat.applyLocalizedPattern(p2, status);
    if(U_FAILURE(status)) {
        errln((UnicodeString)"ERROR: applyPattern() failed with " + (int32_t) status);
    }
    UnicodeString s3;
    s3 = pat.toLocalizedPattern(s3);
    logln((UnicodeString)"Extracted pattern is " + s3);
    if(s3 != p2) {
        errln((UnicodeString)"ERROR: toLocalizedPattern() result did not match pattern applied");
    }

    status = U_ZERO_ERROR;
    UParseError pe;
    pat.applyLocalizedPattern(p2, pe, status);
    if(U_FAILURE(status)) {
        errln((UnicodeString)"ERROR: applyPattern((with ParseError)) failed with " + (int32_t) status);
    }
    UnicodeString s4;
    s4 = pat.toLocalizedPattern(s3);
    logln((UnicodeString)"Extracted pattern is " + s4);
    if(s4 != p2) {
        errln((UnicodeString)"ERROR: toLocalizedPattern(with ParseErr) result did not match pattern applied");
    }

    if(pat.getSecondaryGroupingSize() != 2) {
        errln("FAIL: Secondary Grouping Size should be 2, not %d\n", pat.getSecondaryGroupingSize());
    }

    if(pat.getGroupingSize() != 3) {
        errln("FAIL: Primary Grouping Size should be 3, not %d\n", pat.getGroupingSize());
    }

// ======= Test getStaticClassID()

    logln((UnicodeString)"Testing getStaticClassID()");

    status = U_ZERO_ERROR;
    NumberFormat *test = new DecimalFormat(status);
    if(U_FAILURE(status)) {
        errln((UnicodeString)"ERROR: Couldn't create a DecimalFormat");
    }

    if(test->getDynamicClassID() != DecimalFormat::getStaticClassID()) {
        errln((UnicodeString)"ERROR: getDynamicClassID() didn't return the expected value");
    }

    delete test;
}

void IntlTestDecimalFormatAPI::TestCurrencyPluralInfo(){
    UErrorCode status = U_ZERO_ERROR;

    CurrencyPluralInfo *cpi = new CurrencyPluralInfo(status);
    if(U_FAILURE(status)) {
        errln((UnicodeString)"ERROR: CurrencyPluralInfo(UErrorCode) could not be created");
    }

    CurrencyPluralInfo cpi1 = *cpi;

    if(cpi->getDynamicClassID() != CurrencyPluralInfo::getStaticClassID()){
        errln((UnicodeString)"ERROR: CurrencyPluralInfo::getDynamicClassID() didn't return the expected value");
    }

    cpi->setCurrencyPluralPattern("","",status);
    if(U_FAILURE(status)) {
        errln((UnicodeString)"ERROR: CurrencyPluralInfo::setCurrencyPluralPattern");
    }

    cpi->setLocale(Locale::getCanada(), status);
    if(U_FAILURE(status)) {
        errln((UnicodeString)"ERROR: CurrencyPluralInfo::setLocale");
    }

    cpi->setPluralRules("",status);
    if(U_FAILURE(status)) {
        errln((UnicodeString)"ERROR: CurrencyPluralInfo::setPluralRules");
    }

    DecimalFormat *df = new DecimalFormat(status);
    if(U_FAILURE(status)) {
        errcheckln(status, "ERROR: Could not create DecimalFormat - %s", u_errorName(status));
    }

    df->adoptCurrencyPluralInfo(cpi);

    df->getCurrencyPluralInfo();

    df->setCurrencyPluralInfo(cpi1);

    delete df;
}

void IntlTestDecimalFormatAPI::testRounding(/*char *par*/)
{
    UErrorCode status = U_ZERO_ERROR;
    double Roundingnumber = 2.55;
    double Roundingnumber1 = -2.55;
                      //+2.55 results   -2.55 results
    double result[]={   3.0,            -2.0,    //  kRoundCeiling  0,
                        2.0,            -3.0,    //  kRoundFloor    1,
                        2.0,            -2.0,    //  kRoundDown     2,
                        3.0,            -3.0,    //  kRoundUp       3,
                        3.0,            -3.0,    //  kRoundHalfEven 4,
                        3.0,            -3.0,    //  kRoundHalfDown 5,
                        3.0,            -3.0     //  kRoundHalfUp   6
    };
    DecimalFormat pat(status);
    if(U_FAILURE(status)) {
      errcheckln(status, "ERROR: Could not create DecimalFormat (default) - %s", u_errorName(status));
      return;
    }
    uint16_t mode;
    uint16_t i=0;
    UnicodeString message;
    UnicodeString resultStr;
    for(mode=0;mode < 7;mode++){
        pat.setRoundingMode((DecimalFormat::ERoundingMode)mode);
        if(pat.getRoundingMode() != (DecimalFormat::ERoundingMode)mode){
            errln((UnicodeString)"SetRoundingMode or GetRoundingMode failed for mode=" + mode);
        }


        //for +2.55 with RoundingIncrement=1.0
        pat.setRoundingIncrement(1.0);
        pat.format(Roundingnumber, resultStr);
        message= (UnicodeString)"Round() failed:  round(" + (double)Roundingnumber + UnicodeString(",") + mode + UnicodeString(",FALSE) with RoundingIncrement=1.0==>");
        verify(message, resultStr, result[i++]);
        message.remove();
        resultStr.remove();

        //for -2.55 with RoundingIncrement=1.0
        pat.format(Roundingnumber1, resultStr);
        message= (UnicodeString)"Round() failed:  round(" + (double)Roundingnumber1 + UnicodeString(",") + mode + UnicodeString(",FALSE) with RoundingIncrement=1.0==>");
        verify(message, resultStr, result[i++]);
        message.remove();
        resultStr.remove();
    }

}
void IntlTestDecimalFormatAPI::verify(const UnicodeString& message, const UnicodeString& got, double expected){
    logln((UnicodeString)message + got + (UnicodeString)" Expected : " + expected);
    UnicodeString expectedStr("");
    expectedStr=expectedStr + expected;
    if(got != expectedStr ) {
            errln((UnicodeString)"ERROR: " + message + got + (UnicodeString)"  Expected : " + expectedStr);
        }
}

void IntlTestDecimalFormatAPI::verifyString(const UnicodeString& message, const UnicodeString& got, UnicodeString& expected){
    logln((UnicodeString)message + got + (UnicodeString)" Expected : " + expected);
    if(got != expected ) {
            errln((UnicodeString)"ERROR: " + message + got + (UnicodeString)"  Expected : " + expected);
        }
}

void IntlTestDecimalFormatAPI::testRoundingInc(/*char *par*/)
{
    UErrorCode status = U_ZERO_ERROR;
    DecimalFormat pat(UnicodeString("#,##0.00"),status);
    if(U_FAILURE(status)) {
      errcheckln(status, "ERROR: Could not create DecimalFormat (default) - %s", u_errorName(status));
      return;
    }

    // get default rounding increment
    double roundingInc = pat.getRoundingIncrement();
    if (roundingInc != 0.0) {
      errln((UnicodeString)"ERROR: Rounding increment not zero");
      return;
    }

    // With rounding now being handled by decNumber, we no longer
    // set a rounding increment to enable non-default mode rounding,
    // checking of which was the original point of this test.

    // set rounding mode with zero increment.  Rounding
    // increment should not be set by this operation
    pat.setRoundingMode((DecimalFormat::ERoundingMode)0);
    roundingInc = pat.getRoundingIncrement();
    if (roundingInc != 0.0) {
      errln((UnicodeString)"ERROR: Rounding increment not zero after setRoundingMode");
      return;
    }
}

void IntlTestDecimalFormatAPI::TestScale()
{
    typedef struct TestData {
        double inputValue;
        int inputScale;
        const char *expectedOutput;
    } TestData;

    static TestData testData[] = {
        { 100.0, 3,  "100,000" },
        { 10034.0, -2, "100.34" },
        { 0.86, -3, "0.0009" },
        { -0.000455, 1, "-0%" },
        { -0.000555, 1, "-1%" },
        { 0.000455, 1, "0%" },
        { 0.000555, 1, "1%" },
    };

    UErrorCode status = U_ZERO_ERROR;
    DecimalFormat pat(status);
    if(U_FAILURE(status)) {
      errcheckln(status, "ERROR: Could not create DecimalFormat (default) - %s", u_errorName(status));
      return;
    }

    UnicodeString message;
    UnicodeString resultStr;
    UnicodeString exp;
    UnicodeString percentPattern("#,##0%");
    pat.setMaximumFractionDigits(4);

    for(int32_t i=0; i < UPRV_LENGTHOF(testData); i++) {
        if ( i > 2 ) {
            pat.applyPattern(percentPattern,status);
        }
        pat.setAttribute(UNUM_SCALE,testData[i].inputScale,status);
        pat.format(testData[i].inputValue, resultStr);
        message = UnicodeString("Unexpected output for ") + testData[i].inputValue + UnicodeString(" and scale ") +
                  testData[i].inputScale + UnicodeString(". Got: ");
        exp = testData[i].expectedOutput;
        verifyString(message, resultStr, exp);
        message.remove();
        resultStr.remove();
        exp.remove();
    }
}


#define ASSERT_EQUAL(expect, actual) { char tmp[200]; sprintf(tmp, "(%g==%g)", (double)(expect), (double)(actual)); \
    assertTrue(tmp, ((expect)==(actual)), FALSE, TRUE, __FILE__, __LINE__); }

void IntlTestDecimalFormatAPI::TestFixedDecimal() {
    UErrorCode status = U_ZERO_ERROR;

    LocalPointer<DecimalFormat> df(new DecimalFormat("###", status));
    TEST_ASSERT_STATUS(status);
    FixedDecimal fd = df->getFixedDecimal(44, status);
    TEST_ASSERT_STATUS(status);
    ASSERT_EQUAL(44, fd.source);
    ASSERT_EQUAL(0, fd.visibleDecimalDigitCount);

    df.adoptInstead(new DecimalFormat("###.00##", status));
    TEST_ASSERT_STATUS(status);
    fd = df->getFixedDecimal(123.456, status);
    TEST_ASSERT_STATUS(status);
    ASSERT_EQUAL(3, fd.visibleDecimalDigitCount);
    ASSERT_EQUAL(456, fd.decimalDigits);
    ASSERT_EQUAL(456, fd.decimalDigitsWithoutTrailingZeros);
    ASSERT_EQUAL(123, fd.intValue);
    ASSERT_EQUAL(FALSE, fd.hasIntegerValue);
    ASSERT_EQUAL(FALSE, fd.isNegative);

    df.adoptInstead(new DecimalFormat("###", status));
    TEST_ASSERT_STATUS(status);
    fd = df->getFixedDecimal(123.456, status);
    TEST_ASSERT_STATUS(status);
    ASSERT_EQUAL(0, fd.visibleDecimalDigitCount);
    ASSERT_EQUAL(0, fd.decimalDigits);
    ASSERT_EQUAL(0, fd.decimalDigitsWithoutTrailingZeros);
    ASSERT_EQUAL(123, fd.intValue);
    ASSERT_EQUAL(TRUE, fd.hasIntegerValue);
    ASSERT_EQUAL(FALSE, fd.isNegative);

    df.adoptInstead(new DecimalFormat("###.0", status));
    TEST_ASSERT_STATUS(status);
    fd = df->getFixedDecimal(123.01, status);
    TEST_ASSERT_STATUS(status);
    ASSERT_EQUAL(1, fd.visibleDecimalDigitCount);
    ASSERT_EQUAL(0, fd.decimalDigits);
    ASSERT_EQUAL(0, fd.decimalDigitsWithoutTrailingZeros);
    ASSERT_EQUAL(123, fd.intValue);
    ASSERT_EQUAL(TRUE, fd.hasIntegerValue);
    ASSERT_EQUAL(FALSE, fd.isNegative);

    df.adoptInstead(new DecimalFormat("###.0", status));
    TEST_ASSERT_STATUS(status);
    fd = df->getFixedDecimal(123.06, status);
    TEST_ASSERT_STATUS(status);
    ASSERT_EQUAL(1, fd.visibleDecimalDigitCount);
    ASSERT_EQUAL(1, fd.decimalDigits);
    ASSERT_EQUAL(1, fd.decimalDigitsWithoutTrailingZeros);
    ASSERT_EQUAL(123, fd.intValue);
    ASSERT_EQUAL(FALSE, fd.hasIntegerValue);
    ASSERT_EQUAL(FALSE, fd.isNegative);

    df.adoptInstead(new DecimalFormat("@@@@@", status));  // Significant Digits
    TEST_ASSERT_STATUS(status);
    fd = df->getFixedDecimal(123, status);
    TEST_ASSERT_STATUS(status);
    ASSERT_EQUAL(2, fd.visibleDecimalDigitCount);
    ASSERT_EQUAL(0, fd.decimalDigits);
    ASSERT_EQUAL(0, fd.decimalDigitsWithoutTrailingZeros);
    ASSERT_EQUAL(123, fd.intValue);
    ASSERT_EQUAL(TRUE, fd.hasIntegerValue);
    ASSERT_EQUAL(FALSE, fd.isNegative);

    df.adoptInstead(new DecimalFormat("@@@@@", status));  // Significant Digits
    TEST_ASSERT_STATUS(status);
    fd = df->getFixedDecimal(1.23, status);
    TEST_ASSERT_STATUS(status);
    ASSERT_EQUAL(4, fd.visibleDecimalDigitCount);
    ASSERT_EQUAL(2300, fd.decimalDigits);
    ASSERT_EQUAL(23, fd.decimalDigitsWithoutTrailingZeros);
    ASSERT_EQUAL(1, fd.intValue);
    ASSERT_EQUAL(FALSE, fd.hasIntegerValue);
    ASSERT_EQUAL(FALSE, fd.isNegative);

    fd = df->getFixedDecimal(uprv_getInfinity(), status);
    TEST_ASSERT_STATUS(status);
    ASSERT_EQUAL(TRUE, fd.isNanOrInfinity);
    fd = df->getFixedDecimal(0.0, status);
    ASSERT_EQUAL(FALSE, fd.isNanOrInfinity);
    fd = df->getFixedDecimal(uprv_getNaN(), status);
    ASSERT_EQUAL(TRUE, fd.isNanOrInfinity);
    TEST_ASSERT_STATUS(status);

    // Test Big Decimal input.
    // 22 digits before and after decimal, will exceed the precision of a double
    //    and force DecimalFormat::getFixedDecimal() to work with a digit list.
    df.adoptInstead(new DecimalFormat("#####################0.00####################", status));
    TEST_ASSERT_STATUS(status);
    Formattable fable("12.34", status);
    TEST_ASSERT_STATUS(status);
    fd = df->getFixedDecimal(fable, status);
    TEST_ASSERT_STATUS(status);
    ASSERT_EQUAL(2, fd.visibleDecimalDigitCount);
    ASSERT_EQUAL(34, fd.decimalDigits);
    ASSERT_EQUAL(34, fd.decimalDigitsWithoutTrailingZeros);
    ASSERT_EQUAL(12, fd.intValue);
    ASSERT_EQUAL(FALSE, fd.hasIntegerValue);
    ASSERT_EQUAL(FALSE, fd.isNegative);

    fable.setDecimalNumber("12.345678901234567890123456789", status);
    TEST_ASSERT_STATUS(status);
    fd = df->getFixedDecimal(fable, status);
    TEST_ASSERT_STATUS(status);
    ASSERT_EQUAL(22, fd.visibleDecimalDigitCount);
    ASSERT_EQUAL(345678901234567890LL, fd.decimalDigits);
    ASSERT_EQUAL(34567890123456789LL, fd.decimalDigitsWithoutTrailingZeros);
    ASSERT_EQUAL(12, fd.intValue);
    ASSERT_EQUAL(FALSE, fd.hasIntegerValue);
    ASSERT_EQUAL(FALSE, fd.isNegative);

    // On field overflow, Integer part is truncated on the left, fraction part on the right.
    fable.setDecimalNumber("123456789012345678901234567890.123456789012345678901234567890", status);
    TEST_ASSERT_STATUS(status);
    fd = df->getFixedDecimal(fable, status);
    TEST_ASSERT_STATUS(status);
    ASSERT_EQUAL(22, fd.visibleDecimalDigitCount);
    ASSERT_EQUAL(123456789012345678LL, fd.decimalDigits);
    ASSERT_EQUAL(123456789012345678LL, fd.decimalDigitsWithoutTrailingZeros);
    ASSERT_EQUAL(345678901234567890LL, fd.intValue);
    ASSERT_EQUAL(FALSE, fd.hasIntegerValue);
    ASSERT_EQUAL(FALSE, fd.isNegative);

    // Digits way to the right of the decimal but within the format's precision aren't truncated
    fable.setDecimalNumber("1.0000000000000000000012", status);
    TEST_ASSERT_STATUS(status);
    fd = df->getFixedDecimal(fable, status);
    TEST_ASSERT_STATUS(status);
    ASSERT_EQUAL(22, fd.visibleDecimalDigitCount);
    ASSERT_EQUAL(12, fd.decimalDigits);
    ASSERT_EQUAL(12, fd.decimalDigitsWithoutTrailingZeros);
    ASSERT_EQUAL(1, fd.intValue);
    ASSERT_EQUAL(FALSE, fd.hasIntegerValue);
    ASSERT_EQUAL(FALSE, fd.isNegative);

    // Digits beyond the precision of the format are rounded away
    fable.setDecimalNumber("1.000000000000000000000012", status);
    TEST_ASSERT_STATUS(status);
    fd = df->getFixedDecimal(fable, status);
    TEST_ASSERT_STATUS(status);
    ASSERT_EQUAL(2, fd.visibleDecimalDigitCount);
    ASSERT_EQUAL(0, fd.decimalDigits);
    ASSERT_EQUAL(0, fd.decimalDigitsWithoutTrailingZeros);
    ASSERT_EQUAL(1, fd.intValue);
    ASSERT_EQUAL(TRUE, fd.hasIntegerValue);
    ASSERT_EQUAL(FALSE, fd.isNegative);

    // Negative numbers come through
    fable.setDecimalNumber("-1.0000000000000000000012", status);
    TEST_ASSERT_STATUS(status);
    fd = df->getFixedDecimal(fable, status);
    TEST_ASSERT_STATUS(status);
    ASSERT_EQUAL(22, fd.visibleDecimalDigitCount);
    ASSERT_EQUAL(12, fd.decimalDigits);
    ASSERT_EQUAL(12, fd.decimalDigitsWithoutTrailingZeros);
    ASSERT_EQUAL(1, fd.intValue);
    ASSERT_EQUAL(FALSE, fd.hasIntegerValue);
    ASSERT_EQUAL(TRUE, fd.isNegative);

    // MinFractionDigits from format larger than from number.
    fable.setDecimalNumber("1000000000000000000000.3", status);
    TEST_ASSERT_STATUS(status);
    fd = df->getFixedDecimal(fable, status);
    TEST_ASSERT_STATUS(status);
    ASSERT_EQUAL(2, fd.visibleDecimalDigitCount);
    ASSERT_EQUAL(30, fd.decimalDigits);
    ASSERT_EQUAL(3, fd.decimalDigitsWithoutTrailingZeros);
    ASSERT_EQUAL(100000000000000000LL, fd.intValue);
    ASSERT_EQUAL(FALSE, fd.hasIntegerValue);
    ASSERT_EQUAL(FALSE, fd.isNegative);

    // Test some int64_t values that are out of the range of a double
    fable.setInt64(4503599627370496LL);
    TEST_ASSERT_STATUS(status);
    fd = df->getFixedDecimal(fable, status);
    TEST_ASSERT_STATUS(status);
    ASSERT_EQUAL(2, fd.visibleDecimalDigitCount);
    ASSERT_EQUAL(0, fd.decimalDigits);
    ASSERT_EQUAL(0, fd.decimalDigitsWithoutTrailingZeros);
    ASSERT_EQUAL(4503599627370496LL, fd.intValue);
    ASSERT_EQUAL(TRUE, fd.hasIntegerValue);
    ASSERT_EQUAL(FALSE, fd.isNegative);

    fable.setInt64(4503599627370497LL);
    TEST_ASSERT_STATUS(status);
    fd = df->getFixedDecimal(fable, status);
    TEST_ASSERT_STATUS(status);
    ASSERT_EQUAL(2, fd.visibleDecimalDigitCount);
    ASSERT_EQUAL(0, fd.decimalDigits);
    ASSERT_EQUAL(0, fd.decimalDigitsWithoutTrailingZeros);
    ASSERT_EQUAL(4503599627370497LL, fd.intValue);
    ASSERT_EQUAL(TRUE, fd.hasIntegerValue);
    ASSERT_EQUAL(FALSE, fd.isNegative);

    fable.setInt64(9223372036854775807LL);
    TEST_ASSERT_STATUS(status);
    fd = df->getFixedDecimal(fable, status);
    TEST_ASSERT_STATUS(status);
    ASSERT_EQUAL(2, fd.visibleDecimalDigitCount);
    ASSERT_EQUAL(0, fd.decimalDigits);
    ASSERT_EQUAL(0, fd.decimalDigitsWithoutTrailingZeros);
    // note: going through DigitList path to FixedDecimal, which is trimming
    //       int64_t fields to 18 digits. See ticket Ticket #10374
    // ASSERT_EQUAL(223372036854775807LL, fd.intValue);
    if (!(fd.intValue == 223372036854775807LL || fd.intValue == 9223372036854775807LL)) {
        dataerrln("File %s, Line %d, fd.intValue = %lld", __FILE__, __LINE__, fd.intValue);
    }
    ASSERT_EQUAL(TRUE, fd.hasIntegerValue);
    ASSERT_EQUAL(FALSE, fd.isNegative);

}

void IntlTestDecimalFormatAPI::TestBadFastpath() {
    UErrorCode status = U_ZERO_ERROR;

    LocalPointer<DecimalFormat> df(new DecimalFormat("###", status));
    if (U_FAILURE(status)) {
        dataerrln("Error creating new DecimalFormat - %s", u_errorName(status));
        return;
    }

    UnicodeString fmt;
    fmt.remove();
    assertEquals("Format 1234", "1234", df->format(1234, fmt));
    df->setGroupingUsed(FALSE);
    fmt.remove();
    assertEquals("Format 1234", "1234", df->format(1234, fmt));
    df->setGroupingUsed(TRUE);
    fmt.remove();
    assertEquals("Format 1234 w/ grouping", "1,234", df->format(1234, fmt));
}

void IntlTestDecimalFormatAPI::TestRequiredDecimalPoint() {
    UErrorCode status = U_ZERO_ERROR;
    UnicodeString text("99");
    Formattable result1;
    UnicodeString pat1("##.0000");
    UnicodeString pat2("00.0");

    LocalPointer<DecimalFormat> df(new DecimalFormat(pat1, status));
    if (U_FAILURE(status)) {
        dataerrln("Error creating new DecimalFormat - %s", u_errorName(status));
        return;
    }

    status = U_ZERO_ERROR;
    df->applyPattern(pat1, status);
    if(U_FAILURE(status)) {
        errln((UnicodeString)"ERROR: applyPattern() failed");
    }
    df->parse(text, result1, status);
    if(U_FAILURE(status)) {
        errln((UnicodeString)"ERROR: parse() failed");
    }
    df->setDecimalPatternMatchRequired(TRUE);
    df->parse(text, result1, status);
    if(U_SUCCESS(status)) {
        errln((UnicodeString)"ERROR: unexpected parse()");
    }


    status = U_ZERO_ERROR;
    df->applyPattern(pat2, status);
    df->setDecimalPatternMatchRequired(FALSE);
    if(U_FAILURE(status)) {
        errln((UnicodeString)"ERROR: applyPattern(2) failed");
    }
    df->parse(text, result1, status);
    if(U_FAILURE(status)) {
        errln((UnicodeString)"ERROR: parse(2) failed - " + u_errorName(status));
    }
    df->setDecimalPatternMatchRequired(TRUE);
    df->parse(text, result1, status);
    if(U_SUCCESS(status)) {
        errln((UnicodeString)"ERROR: unexpected parse(2)");
    }
}

#endif /* #if !UCONFIG_NO_FORMATTING */
