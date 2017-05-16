/********************************************************************
 * COPYRIGHT:
 * Copyright (c) 1999-2014, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/
/************************************************************************
*   Date        Name        Description
*   12/15/99    Madhu        Creation.
*   01/12/2000  Madhu        Updated for changed API and added new tests
************************************************************************/

#include "utypeinfo.h"  // for 'typeid' to work

#include "unicode/utypes.h"

#if !UCONFIG_NO_BREAK_ITERATION

#include "unicode/utypes.h"
#include "unicode/brkiter.h"
#include "unicode/rbbi.h"
#include "unicode/uchar.h"
#include "unicode/utf16.h"
#include "unicode/ucnv.h"
#include "unicode/schriter.h"
#include "unicode/uniset.h"
#if !UCONFIG_NO_REGULAR_EXPRESSIONS
#include "unicode/regex.h"
#endif
#include "unicode/ustring.h"
#include "unicode/utext.h"
#include "intltest.h"
#include "rbbitst.h"
#include <string.h>
#include "charstr.h"
#include "uvector.h"
#include "uvectr32.h"
#include <stdio.h>
#include <stdlib.h>
#include "unicode/numfmt.h"
#include "unicode/uscript.h"

#define TEST_ASSERT(x) {if (!(x)) { \
    errln("Failure in file %s, line %d", __FILE__, __LINE__);}}

#define TEST_ASSERT_SUCCESS(errcode) { if (U_FAILURE(errcode)) { \
    errcheckln(errcode, "Failure in file %s, line %d, status = \"%s\"", __FILE__, __LINE__, u_errorName(errcode));}}


//---------------------------------------------
// runIndexedTest
//---------------------------------------------


//  Note:  Before adding new tests to this file, check whether the desired test data can 
//         simply be added to the file testdata/rbbitest.txt.  In most cases it can,
//         it's much less work than writing a new test, diagnostic output in the event of failures
//         is good, and the test data file will is shared with ICU4J, so eventually the test
//         will run there as well, without additional effort.

void RBBITest::runIndexedTest( int32_t index, UBool exec, const char* &name, char* params )
{
    if (exec) logln("TestSuite RuleBasedBreakIterator: ");

    switch (index) {
#if !UCONFIG_NO_FILE_IO
        case 0: name = "TestBug4153072";
            if(exec) TestBug4153072();                         break;
#else
        case 0: name = "skip";
            break;
#endif

        case 1: name = "skip";
            break;
        case 2: name = "TestStatusReturn";
            if(exec) TestStatusReturn();                       break;
 
#if !UCONFIG_NO_FILE_IO
        case 3: name = "TestUnicodeFiles";
            if(exec) TestUnicodeFiles();                       break;
        case 4: name = "TestEmptyString";
            if(exec) TestEmptyString();                        break;
#else
        case 3: case 4: name = "skip";
            break;
#endif

        case 5: name = "TestGetAvailableLocales";
            if(exec) TestGetAvailableLocales();                break;

        case 6: name = "TestGetDisplayName";
            if(exec) TestGetDisplayName();                     break;

#if !UCONFIG_NO_FILE_IO
        case 7: name = "TestEndBehaviour";
            if(exec) TestEndBehaviour();                       break;
        case 8: case 9: case 10: name = "skip";
             break;
        case 11: name = "TestWordBreaks";
             if(exec) TestWordBreaks();                        break;
        case 12: name = "TestWordBoundary";
             if(exec) TestWordBoundary();                      break;
        case 13: name = "TestLineBreaks";
             if(exec) TestLineBreaks();                        break;
        case 14: name = "TestSentBreaks";
             if(exec) TestSentBreaks();                        break;
        case 15: name = "TestExtended";
             if(exec) TestExtended();                          break;
#else
        case 7: case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15: name = "skip";
             break;
#endif

#if !UCONFIG_NO_REGULAR_EXPRESSIONS && !UCONFIG_NO_FILE_IO
        case 16:  
            name = "TestMonkey"; if(exec)  TestMonkey(params); break;
#else
        case 16:
             name = "skip";                                    break;
#endif

#if !UCONFIG_NO_FILE_IO
        case 17: name = "TestBug3818";
            if(exec) TestBug3818();                            break;
#else
        case 17: name = "skip";
            break;
#endif

        case 18: name = "skip";
            break;
        case 19: name = "TestDebug";
            if(exec) TestDebug();                              break;
        case 20: name = "skip";
            break;

#if !UCONFIG_NO_FILE_IO
        case 21: name = "TestBug5775";
            if (exec) TestBug5775();                           break;
#else
        case 21: name = "skip";
            break;
#endif

        case 22: name = "TestBug9983";
            if (exec) TestBug9983();                           break;
        case 23: name = "TestDictRules";
            if (exec) TestDictRules();                         break;
        case 24: name = "TestBug5532";
            if (exec) TestBug5532();                           break;
        default: name = ""; break; //needed to end loop
    }
}


//---------------------------------------------------------------------------
//
//   class BITestData   Holds a set of Break iterator test data and results
//                      Includes
//                         - the string data to be broken
//                         - a vector of the expected break positions.
//                         - a vector of source line numbers for the data,
//                               (to help see where errors occured.)
//                         - The expected break tag values.
//                         - Vectors of actual break positions and tag values.
//                         - Functions for comparing actual with expected and
//                            reporting errors.
//
//----------------------------------------------------------------------------
class BITestData {
public:
    UnicodeString    fDataToBreak;
    UVector          fExpectedBreakPositions;
    UVector          fExpectedTags;
    UVector          fLineNum;
    UVector          fActualBreakPositions;   // Test Results.
    UVector          fActualTags;

    BITestData(UErrorCode &status);
    void             addDataChunk(const char *data, int32_t tag, int32_t lineNum, UErrorCode status);
    void             checkResults(const char *heading, RBBITest *test);
    void             err(const char *heading, RBBITest *test, int32_t expectedIdx, int32_t actualIdx);
    void             clearResults();
};

//
// Constructor.
//
BITestData::BITestData(UErrorCode &status)
: fExpectedBreakPositions(status), fExpectedTags(status),  fLineNum(status), fActualBreakPositions(status),
  fActualTags(status)
{
}

//
// addDataChunk.   Add a section (non-breaking) piece if data to the test data.
//                 The macro form collects the line number, which is helpful
//                 when tracking down failures.
//
//                 A null data item is inserted at the start of each test's data
//                  to put the starting zero into the data list.  The position saved for
//                  each non-null item is its ending position.
//
#define ADD_DATACHUNK(td, data, tag, status)   td.addDataChunk(data, tag, __LINE__, status);
void BITestData::addDataChunk(const char *data, int32_t tag, int32_t lineNum, UErrorCode status) {
    if (U_FAILURE(status)) {return;}
    if (data != NULL) {
        fDataToBreak.append(CharsToUnicodeString(data));
    }
    fExpectedBreakPositions.addElement(fDataToBreak.length(), status);
    fExpectedTags.addElement(tag, status);
    fLineNum.addElement(lineNum, status);
}


//
//  checkResults.   Compare the actual and expected break positions, report any differences.
//
void BITestData::checkResults(const char *heading, RBBITest *test) {
    int32_t   expectedIndex = 0;
    int32_t   actualIndex = 0;

    for (;;) {
        // If we've run through both the expected and actual results vectors, we're done.
        //   break out of the loop.
        if (expectedIndex >= fExpectedBreakPositions.size() &&
            actualIndex   >= fActualBreakPositions.size()) {
            break;
        }


        if (expectedIndex >= fExpectedBreakPositions.size()) {
            err(heading, test, expectedIndex-1, actualIndex);
            actualIndex++;
            continue;
        }

        if (actualIndex >= fActualBreakPositions.size()) {
            err(heading, test, expectedIndex, actualIndex-1);
            expectedIndex++;
            continue;
        }

        if (fActualBreakPositions.elementAti(actualIndex) != fExpectedBreakPositions.elementAti(expectedIndex)) {
            err(heading, test, expectedIndex, actualIndex);
            // Try to resync the positions of the indices, to avoid a rash of spurious erros.
            if (fActualBreakPositions.elementAti(actualIndex) < fExpectedBreakPositions.elementAti(expectedIndex)) {
                actualIndex++;
            } else {
                expectedIndex++;
            }
            continue;
        }

        if (fActualTags.elementAti(actualIndex) != fExpectedTags.elementAti(expectedIndex)) {
            test->errln("%s, tag mismatch.  Test Line = %d, expected tag=%d, got %d",
                heading, fLineNum.elementAt(expectedIndex),
                fExpectedTags.elementAti(expectedIndex), fActualTags.elementAti(actualIndex));
        }

        actualIndex++;
        expectedIndex++;
    }
}

//
//  err   -  An error was found.  Report it, along with information about where the
//                                incorrectly broken test data appeared in the source file.
//
void    BITestData::err(const char *heading, RBBITest *test, int32_t expectedIdx, int32_t actualIdx)
{
    int32_t   expected = fExpectedBreakPositions.elementAti(expectedIdx);
    int32_t   actual   = fActualBreakPositions.elementAti(actualIdx);
    int32_t   o        = 0;
    int32_t   line     = fLineNum.elementAti(expectedIdx);
    if (expectedIdx > 0) {
        // The line numbers are off by one because a premature break occurs somewhere
        //    within the previous item, rather than at the start of the current (expected) item.
        //    We want to report the offset of the unexpected break from the start of
        //      this previous item.
        o    = actual - fExpectedBreakPositions.elementAti(expectedIdx-1);
    }
    if (actual < expected) {
        test->errln("%s unexpected break at offset %d in test item from line %d. actual break: %d  expected break: %d", heading, o, line, actual, expected);
    } else {
        test->errln("%s Failed to find break at end of item from line %d. actual break: %d  expected break: %d", heading, line, actual, expected);
    }
}


void BITestData::clearResults() {
    fActualBreakPositions.removeAllElements();
    fActualTags.removeAllElements();
}


//--------------------------------------------------------------------------------------
//
//    RBBITest    constructor and destructor
//
//--------------------------------------------------------------------------------------

RBBITest::RBBITest() {
}


RBBITest::~RBBITest() {
}

//-----------------------------------------------------------------------------------
//
//   Test for status {tag} return value from break rules.
//        TODO:  a more thorough test.
//
//-----------------------------------------------------------------------------------
void RBBITest::TestStatusReturn() {
     UnicodeString rulesString1("$Letters = [:L:];\n"
                                  "$Numbers = [:N:];\n"
                                  "$Letters+{1};\n"
                                  "$Numbers+{2};\n"
                                  "Help\\ {4}/me\\!;\n"
                                  "[^$Letters $Numbers];\n"
                                  "!.*;\n", -1, US_INV);
     UnicodeString testString1  = "abc123..abc Help me Help me!";
                                // 01234567890123456789012345678
     int32_t bounds1[]   = {0, 3, 6, 7, 8, 11, 12, 16, 17, 19, 20, 25, 27, 28, -1};
     int32_t brkStatus[] = {0, 1, 2, 0, 0,  1,  0,  1,  0,  1,  0,  4,  1,  0, -1};

     UErrorCode status=U_ZERO_ERROR;
     UParseError    parseError;

     BreakIterator *bi = new RuleBasedBreakIterator(rulesString1, parseError, status);
     if(U_FAILURE(status)) {
         dataerrln("FAIL : in construction - %s", u_errorName(status));
     } else {
         int32_t  pos;
         int32_t  i = 0;
         bi->setText(testString1);
         for (pos=bi->first(); pos!= BreakIterator::DONE; pos=bi->next()) {
             if (pos != bounds1[i]) {
                 errln("FAIL:  expected break at %d, got %d\n", bounds1[i], pos);
                 break;
             }

             int tag = bi->getRuleStatus();
             if (tag != brkStatus[i]) {
                 errln("FAIL:  break at %d, expected tag %d, got tag %d\n", pos, brkStatus[i], tag);
                 break;
             }
             i++;
         }
     }
     delete bi;
}


static void printStringBreaks(UText *tstr, int expected[], int expectedCount) {
    UErrorCode status = U_ZERO_ERROR;
    char name[100];
    printf("code    alpha extend alphanum type word sent line name\n");
    int nextExpectedIndex = 0;
    utext_setNativeIndex(tstr, 0);
    for (int j = 0; j < utext_nativeLength(tstr); j=utext_getNativeIndex(tstr)) {
        if (nextExpectedIndex < expectedCount && j >= expected[nextExpectedIndex] ) {
            printf("------------------------------------------------ %d\n", j);
            ++nextExpectedIndex;
        }

        UChar32 c = utext_next32(tstr);
        u_charName(c, U_UNICODE_CHAR_NAME, name, 100, &status);
        printf("%7x %5d %6d %8d %4s %4s %4s %4s %s\n", (int)c,
                           u_isUAlphabetic(c),
                           u_hasBinaryProperty(c, UCHAR_GRAPHEME_EXTEND),
                           u_isalnum(c),
                           u_getPropertyValueName(UCHAR_GENERAL_CATEGORY,
                                                  u_charType(c),
                                                  U_SHORT_PROPERTY_NAME),
                           u_getPropertyValueName(UCHAR_WORD_BREAK,
                                                  u_getIntPropertyValue(c,
                                                          UCHAR_WORD_BREAK),
                                                  U_SHORT_PROPERTY_NAME),
                           u_getPropertyValueName(UCHAR_SENTENCE_BREAK,
                                   u_getIntPropertyValue(c,
                                           UCHAR_SENTENCE_BREAK),
                                   U_SHORT_PROPERTY_NAME),
                           u_getPropertyValueName(UCHAR_LINE_BREAK,
                                   u_getIntPropertyValue(c,
                                           UCHAR_LINE_BREAK),
                                   U_SHORT_PROPERTY_NAME),
                           name);
    }
}


static void printStringBreaks(const UnicodeString &ustr, int expected[], int expectedCount) {
   UErrorCode status = U_ZERO_ERROR;
   UText *tstr = NULL;
   tstr = utext_openConstUnicodeString(NULL, &ustr, &status);
   if (U_FAILURE(status)) {
       printf("printStringBreaks, utext_openConstUnicodeString() returns %s\n", u_errorName(status));
       return;
    }
   printStringBreaks(tstr, expected, expectedCount);
   utext_close(tstr);
}


void RBBITest::TestBug3818() {
    UErrorCode  status = U_ZERO_ERROR;

    // Four Thai words...
    static const UChar thaiWordData[] = {  0x0E43,0x0E2B,0x0E0D,0x0E48, 0x0E43,0x0E2B,0x0E0D,0x0E48,
                                           0x0E43,0x0E2B,0x0E0D,0x0E48, 0x0E43,0x0E2B,0x0E0D,0x0E48, 0 };
    UnicodeString  thaiStr(thaiWordData);

    BreakIterator* bi = BreakIterator::createWordInstance(Locale("th"), status);
    if (U_FAILURE(status) || bi == NULL) {
        errcheckln(status, "Fail at file %s, line %d, status = %s", __FILE__, __LINE__, u_errorName(status));
        return;
    }
    bi->setText(thaiStr);

    int32_t  startOfSecondWord = bi->following(1);
    if (startOfSecondWord != 4) {
        errln("Fail at file %s, line %d expected start of word at 4, got %d",
            __FILE__, __LINE__, startOfSecondWord);
    }
    startOfSecondWord = bi->following(0);
    if (startOfSecondWord != 4) {
        errln("Fail at file %s, line %d expected start of word at 4, got %d",
            __FILE__, __LINE__, startOfSecondWord);
    }
    delete bi;
}

//----------------------------------------------------------------------------
//
// generalIteratorTest      Given a break iterator and a set of test data,
//                          Run the tests and report the results.
//
//----------------------------------------------------------------------------
void RBBITest::generalIteratorTest(RuleBasedBreakIterator& bi, BITestData &td)
{

    bi.setText(td.fDataToBreak);

    testFirstAndNext(bi, td);

    testLastAndPrevious(bi, td);

    testFollowing(bi, td);
    testPreceding(bi, td);
    testIsBoundary(bi, td);
    doMultipleSelectionTest(bi, td);
}


//
//   testFirstAndNext.   Run the iterator forwards in the obvious first(), next()
//                       kind of loop.
//
void RBBITest::testFirstAndNext(RuleBasedBreakIterator& bi, BITestData &td)
{
    UErrorCode  status = U_ZERO_ERROR;
    int32_t     p;
    int32_t     lastP = -1;
    int32_t     tag;

    logln("Test first and next");
    bi.setText(td.fDataToBreak);
    td.clearResults();

    for (p=bi.first(); p!=RuleBasedBreakIterator::DONE; p=bi.next()) {
        td.fActualBreakPositions.addElement(p, status);  // Save result.
        tag = bi.getRuleStatus();
        td.fActualTags.addElement(tag, status);
        if (p <= lastP) {
            // If the iterator is not making forward progress, stop.
            //  No need to raise an error here, it'll be detected in the normal check of results.
            break;
        }
        lastP = p;
    }
    td.checkResults("testFirstAndNext", this);
}


//
//  TestLastAndPrevious.   Run the iterator backwards, starting with last().
//
void  RBBITest::testLastAndPrevious(RuleBasedBreakIterator& bi,  BITestData &td)
{
    UErrorCode  status = U_ZERO_ERROR;
    int32_t     p;
    int32_t     lastP  = 0x7ffffffe;
    int32_t     tag;

    logln("Test last and previous");
    bi.setText(td.fDataToBreak);
    td.clearResults();

    for (p=bi.last(); p!=RuleBasedBreakIterator::DONE; p=bi.previous()) {
        // Save break position.  Insert it at start of vector of results, shoving
        //    already-saved results further towards the end.
        td.fActualBreakPositions.insertElementAt(p, 0, status);
        // bi.previous();   // TODO:  Why does this fix things up????
        // bi.next();
        tag = bi.getRuleStatus();
        td.fActualTags.insertElementAt(tag, 0, status);
        if (p >= lastP) {
            // If the iterator is not making progress, stop.
            //  No need to raise an error here, it'll be detected in the normal check of results.
            break;
        }
        lastP = p;
    }
    td.checkResults("testLastAndPrevious", this);
}


void RBBITest::testFollowing(RuleBasedBreakIterator& bi, BITestData &td)
{
    UErrorCode  status = U_ZERO_ERROR;
    int32_t     p;
    int32_t     tag;
    int32_t     lastP  = -2;     // A value that will never be returned as a break position.
                                 //   cannot be -1; that is returned for DONE.
    int         i;

    logln("testFollowing():");
    bi.setText(td.fDataToBreak);
    td.clearResults();

    // Save the starting point, since we won't get that out of following.
    p = bi.first();
    td.fActualBreakPositions.addElement(p, status);  // Save result.
    tag = bi.getRuleStatus();
    td.fActualTags.addElement(tag, status);

    for (i = 0; i <= td.fDataToBreak.length()+1; i++) {
        p = bi.following(i);
        if (p != lastP) {
            if (p == RuleBasedBreakIterator::DONE) {
                break;
            }
            // We've reached a new break position.  Save it.
            td.fActualBreakPositions.addElement(p, status);  // Save result.
            tag = bi.getRuleStatus();
            td.fActualTags.addElement(tag, status);
            lastP = p;
        }
    }
    // The loop normally exits by means of the break in the middle.
    // Make sure that the index was at the correct position for the break iterator to have
    //   returned DONE.
    if (i != td.fDataToBreak.length()) {
        errln("testFollowing():  iterator returned DONE prematurely.");
    }

    // Full check of all results.
    td.checkResults("testFollowing", this);
}



void RBBITest::testPreceding(RuleBasedBreakIterator& bi,  BITestData &td) {
    UErrorCode  status = U_ZERO_ERROR;
    int32_t     p;
    int32_t     tag;
    int32_t     lastP  = 0x7ffffffe;
    int         i;

    logln("testPreceding():");
    bi.setText(td.fDataToBreak);
    td.clearResults();

    p = bi.last();
    td.fActualBreakPositions.addElement(p, status);
    tag = bi.getRuleStatus();
    td.fActualTags.addElement(tag, status);

    for (i = td.fDataToBreak.length(); i>=-1; i--) {
        p = bi.preceding(i);
        if (p != lastP) {
            if (p == RuleBasedBreakIterator::DONE) {
                break;
            }
            // We've reached a new break position.  Save it.
            td.fActualBreakPositions.insertElementAt(p, 0, status);
            lastP = p;
            tag = bi.getRuleStatus();
            td.fActualTags.insertElementAt(tag, 0, status);
        }
    }
    // The loop normally exits by means of the break in the middle.
    // Make sure that the index was at the correct position for the break iterator to have
    //   returned DONE.
    if (i != 0) {
        errln("testPreceding():  iterator returned DONE prematurely.");
    }

    // Full check of all results.
    td.checkResults("testPreceding", this);
}



void RBBITest::testIsBoundary(RuleBasedBreakIterator& bi,  BITestData &td) {
    UErrorCode  status = U_ZERO_ERROR;
    int         i;
    int32_t     tag;

    logln("testIsBoundary():");
    bi.setText(td.fDataToBreak);
    td.clearResults();

    for (i = 0; i <= td.fDataToBreak.length(); i++) {
        if (bi.isBoundary(i)) {
            td.fActualBreakPositions.addElement(i, status);  // Save result.
            tag = bi.getRuleStatus();
            td.fActualTags.addElement(tag, status);
        }
    }
    td.checkResults("testIsBoundary: ", this);
}



void RBBITest::doMultipleSelectionTest(RuleBasedBreakIterator& iterator, BITestData &td)
{
    iterator.setText(td.fDataToBreak);

    RuleBasedBreakIterator* testIterator =(RuleBasedBreakIterator*)iterator.clone();
    int32_t offset = iterator.first();
    int32_t testOffset;
    int32_t count = 0;

    logln("doMultipleSelectionTest text of length: %d", td.fDataToBreak.length());

    if (*testIterator != iterator)
        errln("clone() or operator!= failed: two clones compared unequal");

    do {
        testOffset = testIterator->first();
        testOffset = testIterator->next(count);
        if (offset != testOffset)
            errln(UnicodeString("next(n) and next() not returning consistent results: for step ") + count + ", next(n) returned " + testOffset + " and next() had " + offset);

        if (offset != RuleBasedBreakIterator::DONE) {
            count++;
            offset = iterator.next();

            if (offset != RuleBasedBreakIterator::DONE && *testIterator == iterator) {
                errln("operator== failed: Two unequal iterators compared equal. count=%d offset=%d", count, offset);
                if (count > 10000 || offset == -1) {
                    errln("operator== failed too many times. Stopping test.");
                    if (offset == -1) {
                        errln("Does (RuleBasedBreakIterator::DONE == -1)?");
                    }
                    return;
                }
            }
        }
    } while (offset != RuleBasedBreakIterator::DONE);

    // now do it backwards...
    offset = iterator.last();
    count = 0;

    do {
        testOffset = testIterator->last();
        testOffset = testIterator->next(count);   // next() with a negative arg is same as previous
        if (offset != testOffset)
            errln(UnicodeString("next(n) and next() not returning consistent results: for step ") + count + ", next(n) returned " + testOffset + " and next() had " + offset);

        if (offset != RuleBasedBreakIterator::DONE) {
            count--;
            offset = iterator.previous();
        }
    } while (offset != RuleBasedBreakIterator::DONE);

    delete testIterator;
}


//---------------------------------------------
//
//     other tests
//
//---------------------------------------------
void RBBITest::TestEmptyString()
{
    UnicodeString text = "";
    UErrorCode status = U_ZERO_ERROR;

    BITestData x(status);
    ADD_DATACHUNK(x, "", 0, status);           // Break at start of data
    RuleBasedBreakIterator* bi = (RuleBasedBreakIterator *)BreakIterator::createLineInstance(Locale::getDefault(), status);
    if (U_FAILURE(status))
    {
        errcheckln(status, "Failed to create the BreakIterator for default locale in TestEmptyString. - %s", u_errorName(status));
        return;
    }
    generalIteratorTest(*bi, x);
    delete bi;
}

void RBBITest::TestGetAvailableLocales()
{
    int32_t locCount = 0;
    const Locale* locList = BreakIterator::getAvailableLocales(locCount);

    if (locCount == 0)
        dataerrln("getAvailableLocales() returned an empty list!");
    // Just make sure that it's returning good memory.
    int32_t i;
    for (i = 0; i < locCount; ++i) {
        logln(locList[i].getName());
    }
}

//Testing the BreakIterator::getDisplayName() function
void RBBITest::TestGetDisplayName()
{
    UnicodeString   result;

    BreakIterator::getDisplayName(Locale::getUS(), result);
    if (Locale::getDefault() == Locale::getUS() && result != "English (United States)")
        dataerrln("BreakIterator::getDisplayName() failed: expected \"English (United States)\", got \""
                + result);

    BreakIterator::getDisplayName(Locale::getFrance(), Locale::getUS(), result);
    if (result != "French (France)")
        dataerrln("BreakIterator::getDisplayName() failed: expected \"French (France)\", got \""
                + result);
}
/**
 * Test End Behaviour
 * @bug 4068137
 */
void RBBITest::TestEndBehaviour()
{
    UErrorCode status = U_ZERO_ERROR;
    UnicodeString testString("boo.");
    BreakIterator *wb = BreakIterator::createWordInstance(Locale::getDefault(), status);
    if (U_FAILURE(status))
    {
        errcheckln(status, "Failed to create the BreakIterator for default locale in TestEndBehaviour. - %s", u_errorName(status));
        return;
    }
    wb->setText(testString);

    if (wb->first() != 0)
        errln("Didn't get break at beginning of string.");
    if (wb->next() != 3)
        errln("Didn't get break before period in \"boo.\"");
    if (wb->current() != 4 && wb->next() != 4)
        errln("Didn't get break at end of string.");
    delete wb;
}
/*
 * @bug 4153072
 */
void RBBITest::TestBug4153072() {
    UErrorCode status = U_ZERO_ERROR;
    BreakIterator *iter = BreakIterator::createWordInstance(Locale::getDefault(), status);
    if (U_FAILURE(status))
    {
        errcheckln(status, "Failed to create the BreakIterator for default locale in TestBug4153072 - %s", u_errorName(status));
        return;
    }
    UnicodeString str("...Hello, World!...");
    int32_t begin = 3;
    int32_t end = str.length() - 3;
    UBool onBoundary;

    StringCharacterIterator* textIterator = new StringCharacterIterator(str, begin, end, begin);
    iter->adoptText(textIterator);
    int index;
    // Note: with the switch to UText, there is no way to restrict the
    //       iteration range to begin at an index other than zero.
    //       String character iterators created with a non-zero bound are
    //         treated by RBBI as being empty.
    for (index = -1; index < begin + 1; ++index) {
        onBoundary = iter->isBoundary(index);
        if (index == 0?  !onBoundary : onBoundary) {
            errln((UnicodeString)"Didn't handle isBoundary correctly with offset = " + index +
                            " and begin index = " + begin);
        }
    }
    delete iter;
}


//
// Test for problem reported by Ashok Matoria on 9 July 2007
//    One.<kSoftHyphen><kSpace>Two.
//
//    Sentence break at start (0) and then on calling next() it breaks at
//   'T' of "Two". Now, at this point if I do next() and
//    then previous(), it breaks at <kSOftHyphen> instead of 'T' of "Two".
//
void RBBITest::TestBug5775() {
    UErrorCode status = U_ZERO_ERROR;
    BreakIterator *bi = BreakIterator::createSentenceInstance(Locale::getEnglish(), status);
    TEST_ASSERT_SUCCESS(status);
    if (U_FAILURE(status)) {
        return;
    }
// Check for status first for better handling of no data errors.
    TEST_ASSERT(bi != NULL);
    if (bi == NULL) {
        return;
    }
    
    UnicodeString s("One.\\u00ad Two.", -1, US_INV);
    //               01234      56789
    s = s.unescape();
    bi->setText(s);
    int pos = bi->next();
    TEST_ASSERT(pos == 6);
    pos = bi->next();
    TEST_ASSERT(pos == 10);
    pos = bi->previous();
    TEST_ASSERT(pos == 6);
    delete bi;
}



//------------------------------------------------------------------------------
//
//   RBBITest::Extended    Run  RBBI Tests from an external test data file
//
//------------------------------------------------------------------------------

struct TestParams {
    BreakIterator   *bi;                   // Break iterator is set while parsing test source.
                                           //   Changed out whenever test data changes break type.

    UnicodeString    dataToBreak;          // Data that is built up while parsing the test.
    UVector32       *expectedBreaks;       // Expected break positions, matches dataToBreak UnicodeString.
    UVector32       *srcLine;              // Positions in source file, indexed same as dataToBreak.
    UVector32       *srcCol;

    UText           *textToBreak;          // UText, could be UTF8 or UTF16.
    UVector32       *textMap;              // Map from UTF-16 dataToBreak offsets to UText offsets.
    CharString       utf8String;           // UTF-8 form of text to break.

    TestParams(UErrorCode &status) : dataToBreak() {
        bi               = NULL;
        expectedBreaks   = new UVector32(status);
        srcLine          = new UVector32(status);
        srcCol           = new UVector32(status);
        textToBreak      = NULL;
        textMap          = new UVector32(status);
    }

    ~TestParams() {
        delete bi;
        delete expectedBreaks;
        delete srcLine;
        delete srcCol;
        utext_close(textToBreak);
        delete textMap;
    }
    
    int32_t getSrcLine(int32_t bp);
    int32_t getExpectedBreak(int32_t bp);
    int32_t getSrcCol(int32_t bp);

    void setUTF16(UErrorCode &status);
    void setUTF8(UErrorCode &status);
};

// Append a UnicodeString to a CharString with UTF-8 encoding.
// Substitute any invalid chars.
//   Note: this is used with test data that includes a few unpaired surrogates in the UTF-16 that will be substituted.
static void CharStringAppend(CharString &dest, const UnicodeString &src, UErrorCode &status) {
    if (U_FAILURE(status)) {
        return;
    }
    int32_t utf8Length;
    u_strToUTF8WithSub(NULL, 0, &utf8Length,            // Output Buffer, NULL for preflight.
                       src.getBuffer(), src.length(),   // UTF-16 data
                       0xfffd, NULL,                    // Substitution char, number of subs.
                       &status);
    if (U_FAILURE(status) && status != U_BUFFER_OVERFLOW_ERROR) {
        return;
    }
    status = U_ZERO_ERROR;
    int32_t capacity;
    char *buffer = dest.getAppendBuffer(utf8Length, utf8Length, capacity, status);
    u_strToUTF8WithSub(buffer, utf8Length, NULL,
                       src.getBuffer(), src.length(),
                       0xfffd, NULL, &status);
    dest.append(buffer, utf8Length, status);
}
  

void TestParams::setUTF16(UErrorCode &status) {
    textToBreak = utext_openUnicodeString(textToBreak, &dataToBreak, &status);
    textMap->removeAllElements();
    for (int32_t i=0; i<dataToBreak.length(); i++) {
        if (i == dataToBreak.getChar32Start(i)) {
            textMap->addElement(i, status);
        } else {
            textMap->addElement(-1, status);
        }
    }
    textMap->addElement(dataToBreak.length(), status);
    U_ASSERT(dataToBreak.length() + 1 == textMap->size());
}


void TestParams::setUTF8(UErrorCode &status) {
    if (U_FAILURE(status)) {
        return;
    }
    utf8String.clear();
    CharStringAppend(utf8String, dataToBreak, status);
    textToBreak = utext_openUTF8(textToBreak, utf8String.data(), utf8String.length(), &status);
    if (U_FAILURE(status)) {
        return;
    }

    textMap->removeAllElements();
    int32_t utf16Index = 0;
    for (;;) {
        textMap->addElement(utf16Index, status);
        UChar32 c32 = utext_current32(textToBreak);
        if (c32 < 0) {
            break;
        }
        utf16Index += U16_LENGTH(c32);
        utext_next32(textToBreak);
        while (textMap->size() < utext_getNativeIndex(textToBreak)) {
            textMap->addElement(-1, status);
        }
    }
    U_ASSERT(utext_nativeLength(textToBreak) + 1 == textMap->size());
}


int32_t TestParams::getSrcLine(int bp) {
    if (bp >= textMap->size()) {
        bp = textMap->size() - 1;
    }
    int32_t i = 0;
    for(; bp >= 0 ; --bp) {
        // Move to a character boundary if we are not on one already.
        i = textMap->elementAti(bp);
        if (i >= 0) {
            break;
        }
    }
    return srcLine->elementAti(i);
}


int32_t TestParams::getExpectedBreak(int bp) {
    if (bp >= textMap->size()) {
        return 0;
    }
    int32_t i = textMap->elementAti(bp);
    int32_t retVal = 0;
    if (i >= 0) {
        retVal = expectedBreaks->elementAti(i);
    }
    return retVal;
}


int32_t TestParams::getSrcCol(int bp) {
    if (bp >= textMap->size()) {
        bp = textMap->size() - 1;
    }
    int32_t i = 0;
    for(; bp >= 0; --bp) {
        // Move bp to a character boundary if we are not on one already.
        i = textMap->elementAti(bp);
        if (i >= 0) {
            break;
        }
    }
    return srcCol->elementAti(i);
}


void RBBITest::executeTest(TestParams *t, UErrorCode &status) {
    int32_t    bp;
    int32_t    prevBP;
    int32_t    i;

    TEST_ASSERT_SUCCESS(status);
    if (U_FAILURE(status)) {
        return;
    }

    if (t->bi == NULL) {
        return;
    }

    t->bi->setText(t->textToBreak, status);
    //
    //  Run the iterator forward
    //
    prevBP = -1;
    for (bp = t->bi->first(); bp != BreakIterator::DONE; bp = t->bi->next()) {
        if (prevBP ==  bp) {
            // Fail for lack of forward progress.
            errln("Forward Iteration, no forward progress.  Break Pos=%4d  File line,col=%4d,%4d",
                bp, t->getSrcLine(bp), t->getSrcCol(bp));
            break;
        }

        // Check that there we didn't miss an expected break between the last one
        //  and this one.
        for (i=prevBP+1; i<bp; i++) {
            if (t->getExpectedBreak(i) != 0) {
                int expected[] = {0, i};
                printStringBreaks(t->dataToBreak, expected, 2);
                errln("Forward Iteration, break expected, but not found.  Pos=%4d  File line,col= %4d,%4d",
                      i, t->getSrcLine(i), t->getSrcCol(i));
            }
        }

        // Check that the break we did find was expected
        if (t->getExpectedBreak(bp) == 0) {
            int expected[] = {0, bp};
            printStringBreaks(t->textToBreak, expected, 2);
            errln("Forward Iteration, break found, but not expected.  Pos=%4d  File line,col= %4d,%4d",
                bp, t->getSrcLine(bp), t->getSrcCol(bp));
        } else {
            // The break was expected.
            //   Check that the {nnn} tag value is correct.
            int32_t expectedTagVal = t->getExpectedBreak(bp);
            if (expectedTagVal == -1) {
                expectedTagVal = 0;
            }
            int32_t line = t->getSrcLine(bp);
            int32_t rs = ((RuleBasedBreakIterator *)t->bi)->getRuleStatus();
            if (rs != expectedTagVal) {
                errln("Incorrect status for forward break.  Pos=%4d  File line,col= %4d,%4d.\n"
                      "          Actual, Expected status = %4d, %4d",
                    bp, line, t->getSrcCol(bp), rs, expectedTagVal);
            }
        }

        prevBP = bp;
    }

    // Verify that there were no missed expected breaks after the last one found
    for (i=prevBP+1; i<utext_nativeLength(t->textToBreak); i++) {
        if (t->getExpectedBreak(i) != 0) {
            errln("Forward Iteration, break expected, but not found.  Pos=%4d  File line,col= %4d,%4d",
                      i, t->getSrcLine(i), t->getSrcCol(i));
        }
    }

    //
    //  Run the iterator backwards, verify that the same breaks are found.
    //
    prevBP = utext_nativeLength(t->textToBreak)+2;  // start with a phony value for the last break pos seen.
    for (bp = t->bi->last(); bp != BreakIterator::DONE; bp = t->bi->previous()) {
        if (prevBP ==  bp) {
            // Fail for lack of progress.
            errln("Reverse Iteration, no progress.  Break Pos=%4d  File line,col=%4d,%4d",
                bp, t->getSrcLine(bp), t->getSrcCol(bp));
            break;
        }

        // Check that we didn't miss an expected break between the last one
        //  and this one.  (UVector returns zeros for index out of bounds.)
        for (i=prevBP-1; i>bp; i--) {
            if (t->getExpectedBreak(i) != 0) {
                errln("Reverse Iteration, break expected, but not found.  Pos=%4d  File line,col= %4d,%4d",
                      i, t->getSrcLine(i), t->getSrcCol(i));
            }
        }

        // Check that the break we did find was expected
        if (t->getExpectedBreak(bp) == 0) {
            errln("Reverse Itertion, break found, but not expected.  Pos=%4d  File line,col= %4d,%4d",
                   bp, t->getSrcLine(bp), t->getSrcCol(bp));
        } else {
            // The break was expected.
            //   Check that the {nnn} tag value is correct.
            int32_t expectedTagVal = t->getExpectedBreak(bp);
            if (expectedTagVal == -1) {
                expectedTagVal = 0;
            }
            int line = t->getSrcLine(bp);
            int32_t rs = t->bi->getRuleStatus();
            if (rs != expectedTagVal) {
                errln("Incorrect status for reverse break.  Pos=%4d  File line,col= %4d,%4d.\n"
                      "          Actual, Expected status = %4d, %4d",
                    bp, line, t->getSrcCol(bp), rs, expectedTagVal);
            }
        }

        prevBP = bp;
    }

    // Verify that there were no missed breaks prior to the last one found
    for (i=prevBP-1; i>=0; i--) {
        if (t->getExpectedBreak(i) != 0) {
            errln("Forward Itertion, break expected, but not found.  Pos=%4d  File line,col= %4d,%4d",
                      i, t->getSrcLine(i), t->getSrcCol(i));
        }
    }

    // Check isBoundary()
    for (i=0; i < utext_nativeLength(t->textToBreak); i++) {
        UBool boundaryExpected = (t->getExpectedBreak(i) != 0);
        UBool boundaryFound    = t->bi->isBoundary(i);
        if (boundaryExpected != boundaryFound) {
            errln("isBoundary(%d) incorrect. File line,col= %4d,%4d\n"
                  "        Expected, Actual= %s, %s",
                  i, t->getSrcLine(i), t->getSrcCol(i),
                  boundaryExpected ? "true":"false", boundaryFound? "true" : "false");
        }
    }

    // Check following()
    for (i=0; i < utext_nativeLength(t->textToBreak); i++) {
        int32_t actualBreak = t->bi->following(i);
        int32_t expectedBreak = BreakIterator::DONE;
        for (int32_t j=i+1; j <= utext_nativeLength(t->textToBreak); j++) {
            if (t->getExpectedBreak(j) != 0) {
                expectedBreak = j;
                break;
            }
        }
        if (expectedBreak != actualBreak) {
            errln("following(%d) incorrect. File line,col= %4d,%4d\n"
                  "        Expected, Actual= %d, %d",
                  i, t->getSrcLine(i), t->getSrcCol(i), expectedBreak, actualBreak);
        }
    }

    // Check preceding()
    for (i=utext_nativeLength(t->textToBreak); i>=0; i--) {
        int32_t actualBreak = t->bi->preceding(i);
        int32_t expectedBreak = BreakIterator::DONE;

        // For UTF-8 & UTF-16 supplementals, all code units of a character are equivalent.
        // preceding(trailing byte) will return the index of some preceding code point,
        // not the lead byte of the current code point, even though that has a smaller index.
        // Therefore, start looking at the expected break data not at i-1, but at
        // the start of code point index - 1.
        utext_setNativeIndex(t->textToBreak, i);
        int32_t j = utext_getNativeIndex(t->textToBreak) - 1;
        for (; j >= 0; j--) {
            if (t->getExpectedBreak(j) != 0) {
                expectedBreak = j;
                break;
            }
        }
        if (expectedBreak != actualBreak) {
            errln("preceding(%d) incorrect. File line,col= %4d,%4d\n"
                  "        Expected, Actual= %d, %d",
                  i, t->getSrcLine(i), t->getSrcCol(i), expectedBreak, actualBreak);
        }
    }
}


void RBBITest::TestExtended() {
#if !UCONFIG_NO_REGULAR_EXPRESSIONS
    UErrorCode      status  = U_ZERO_ERROR;
    Locale          locale("");

    UnicodeString       rules;
    TestParams          tp(status);

    RegexMatcher      localeMatcher(UNICODE_STRING_SIMPLE("<locale *([\\p{L}\\p{Nd}_]*) *>"), 0, status);
    if (U_FAILURE(status)) {
        dataerrln("Failure in file %s, line %d, status = \"%s\"", __FILE__, __LINE__, u_errorName(status));
    }


    //
    //  Open and read the test data file.
    //
    const char *testDataDirectory = IntlTest::getSourceTestData(status);
    char testFileName[1000];
    if (testDataDirectory == NULL || strlen(testDataDirectory) >= sizeof(testFileName)) {
        errln("Can't open test data.  Path too long.");
        return;
    }
    strcpy(testFileName, testDataDirectory);
    strcat(testFileName, "rbbitst.txt");

    int    len;
    UChar *testFile = ReadAndConvertFile(testFileName, len, "UTF-8", status);
    if (U_FAILURE(status)) {
        return; /* something went wrong, error already output */
    }




    //
    //  Put the test data into a UnicodeString
    //
    UnicodeString testString(FALSE, testFile, len);

    enum EParseState{
        PARSE_COMMENT,
        PARSE_TAG,
        PARSE_DATA,
        PARSE_NUM
    }
    parseState = PARSE_TAG;

    EParseState savedState = PARSE_TAG;

    static const UChar CH_LF        = 0x0a;
    static const UChar CH_CR        = 0x0d;
    static const UChar CH_HASH      = 0x23;
    /*static const UChar CH_PERIOD    = 0x2e;*/
    static const UChar CH_LT        = 0x3c;
    static const UChar CH_GT        = 0x3e;
    static const UChar CH_BACKSLASH = 0x5c;
    static const UChar CH_BULLET    = 0x2022;

    int32_t    lineNum  = 1;
    int32_t    colStart = 0;
    int32_t    column   = 0;
    int32_t    charIdx  = 0;

    int32_t    tagValue = 0;       // The numeric value of a <nnn> tag.

    for (charIdx = 0; charIdx < len; ) {
        status = U_ZERO_ERROR;
        UChar  c = testString.charAt(charIdx);
        charIdx++;
        if (c == CH_CR && charIdx<len && testString.charAt(charIdx) == CH_LF) {
            // treat CRLF as a unit
            c = CH_LF;
            charIdx++;
        }
        if (c == CH_LF || c == CH_CR) {
            lineNum++;
            colStart = charIdx;
        }
        column = charIdx - colStart + 1;

        switch (parseState) {
        case PARSE_COMMENT:
            if (c == 0x0a || c == 0x0d) {
                parseState = savedState;
            }
            break;

        case PARSE_TAG:
            {
            if (c == CH_HASH) {
                parseState = PARSE_COMMENT;
                savedState = PARSE_TAG;
                break;
            }
            if (u_isUWhiteSpace(c)) {
                break;
            }
            if (testString.compare(charIdx-1, 6, "<word>") == 0) {
                delete tp.bi;
                tp.bi = BreakIterator::createWordInstance(locale,  status);
                charIdx += 5;
                break;
            }
            if (testString.compare(charIdx-1, 6, "<char>") == 0) {
                delete tp.bi;
                tp.bi = BreakIterator::createCharacterInstance(locale,  status);
                charIdx += 5;
                break;
            }
            if (testString.compare(charIdx-1, 6, "<line>") == 0) {
                delete tp.bi;
                tp.bi = BreakIterator::createLineInstance(locale,  status);
                charIdx += 5;
                break;
            }
            if (testString.compare(charIdx-1, 6, "<sent>") == 0) {
                delete tp.bi;
                tp.bi = NULL;
                tp.bi = BreakIterator::createSentenceInstance(locale,  status);
                charIdx += 5;
                break;
            }
            if (testString.compare(charIdx-1, 7, "<title>") == 0) {
                delete tp.bi;
                tp.bi = BreakIterator::createTitleInstance(locale,  status);
                charIdx += 6;
                break;
            }

            // <locale  loc_name>
            localeMatcher.reset(testString);
            if (localeMatcher.lookingAt(charIdx-1, status)) {
                UnicodeString localeName = localeMatcher.group(1, status);
                char localeName8[100];
                localeName.extract(0, localeName.length(), localeName8, sizeof(localeName8), 0);
                locale = Locale::createFromName(localeName8);
                charIdx += localeMatcher.group(0, status).length() - 1;
                TEST_ASSERT_SUCCESS(status);
                break;
            }
            if (testString.compare(charIdx-1, 6, "<data>") == 0) {
                parseState = PARSE_DATA;
                charIdx += 5;
                tp.dataToBreak = "";
                tp.expectedBreaks->removeAllElements();
                tp.srcCol ->removeAllElements();
                tp.srcLine->removeAllElements();
                break;
            }

            errln("line %d: Tag expected in test file.", lineNum);
            parseState = PARSE_COMMENT;
            savedState = PARSE_DATA;
            goto end_test; // Stop the test.
            }
            break;

        case PARSE_DATA:
            if (c == CH_BULLET) {
                int32_t  breakIdx = tp.dataToBreak.length();
                tp.expectedBreaks->setSize(breakIdx+1);
                tp.expectedBreaks->setElementAt(-1, breakIdx);
                tp.srcLine->setSize(breakIdx+1);
                tp.srcLine->setElementAt(lineNum, breakIdx);
                tp.srcCol ->setSize(breakIdx+1);
                tp.srcCol ->setElementAt(column, breakIdx);
                break;
            }

            if (testString.compare(charIdx-1, 7, "</data>") == 0) {
                // Add final entry to mappings from break location to source file position.
                //  Need one extra because last break position returned is after the
                //    last char in the data, not at the last char.
                tp.srcLine->addElement(lineNum, status);
                tp.srcCol ->addElement(column, status);

                parseState = PARSE_TAG;
                charIdx += 6;

                // RUN THE TEST!
                status = U_ZERO_ERROR;
                tp.setUTF16(status);
                executeTest(&tp, status);
                TEST_ASSERT_SUCCESS(status);

                // Run again, this time with UTF-8 text wrapped in a UText.
                status = U_ZERO_ERROR;
                tp.setUTF8(status);
                TEST_ASSERT_SUCCESS(status);
                executeTest(&tp, status);
                break;
            }

            if (testString.compare(charIdx-1, 3, UNICODE_STRING_SIMPLE("\\N{")) == 0) {
                // Named character, e.g. \N{COMBINING GRAVE ACCENT}
                // Get the code point from the name and insert it into the test data.
                //   (Damn, no API takes names in Unicode  !!!
                //    we've got to take it back to char *)
                int32_t nameEndIdx = testString.indexOf((UChar)0x7d/*'}'*/, charIdx);
                int32_t nameLength = nameEndIdx - (charIdx+2);
                char charNameBuf[200];
                UChar32 theChar = -1;
                if (nameEndIdx != -1) {
                    UErrorCode status = U_ZERO_ERROR;
                    testString.extract(charIdx+2, nameLength, charNameBuf, sizeof(charNameBuf));
                    charNameBuf[sizeof(charNameBuf)-1] = 0;
                    theChar = u_charFromName(U_UNICODE_CHAR_NAME, charNameBuf, &status);
                    if (U_FAILURE(status)) {
                        theChar = -1;
                    }
                }
                if (theChar == -1) {
                    errln("Error in named character in test file at line %d, col %d",
                        lineNum, column);
                } else {
                    // Named code point was recognized.  Insert it
                    //   into the test data.
                    tp.dataToBreak.append(theChar);
                    while (tp.dataToBreak.length() > tp.srcLine->size()) {
                        tp.srcLine->addElement(lineNum, status);
                        tp.srcCol ->addElement(column, status);
                    }
                }
                if (nameEndIdx > charIdx) {
                    charIdx = nameEndIdx+1;

                }
                break;
            }




            if (testString.compare(charIdx-1, 2, "<>") == 0) {
                charIdx++;
                int32_t  breakIdx = tp.dataToBreak.length();
                tp.expectedBreaks->setSize(breakIdx+1);
                tp.expectedBreaks->setElementAt(-1, breakIdx);
                tp.srcLine->setSize(breakIdx+1);
                tp.srcLine->setElementAt(lineNum, breakIdx);
                tp.srcCol ->setSize(breakIdx+1);
                tp.srcCol ->setElementAt(column, breakIdx);
                break;
            }

            if (c == CH_LT) {
                tagValue   = 0;
                parseState = PARSE_NUM;
                break;
            }

            if (c == CH_HASH && column==3) {   // TODO:  why is column off so far?
                parseState = PARSE_COMMENT;
                savedState = PARSE_DATA;
                break;
            }

            if (c == CH_BACKSLASH) {
                // Check for \ at end of line, a line continuation.
                //     Advance over (discard) the newline
                UChar32 cp = testString.char32At(charIdx);
                if (cp == CH_CR && charIdx<len && testString.charAt(charIdx+1) == CH_LF) {
                    // We have a CR LF
                    //  Need an extra increment of the input ptr to move over both of them
                    charIdx++;
                }
                if (cp == CH_LF || cp == CH_CR) {
                    lineNum++;
                    colStart = charIdx;
                    charIdx++;
                    break;
                }

                // Let unescape handle the back slash.
                cp = testString.unescapeAt(charIdx);
                if (cp != -1) {
                    // Escape sequence was recognized.  Insert the char
                    //   into the test data.
                    tp.dataToBreak.append(cp);
                    while (tp.dataToBreak.length() > tp.srcLine->size()) {
                        tp.srcLine->addElement(lineNum, status);
                        tp.srcCol ->addElement(column, status);
                    }
                    break;
                }


                // Not a recognized backslash escape sequence.
                // Take the next char as a literal.
                //  TODO:  Should this be an error?
                c = testString.charAt(charIdx);
                charIdx = testString.moveIndex32(charIdx, 1);
            }

            // Normal, non-escaped data char.
            tp.dataToBreak.append(c);

            // Save the mapping from offset in the data to line/column numbers in
            //   the original input file.  Will be used for better error messages only.
            //   If there's an expected break before this char, the slot in the mapping
            //     vector will already be set for this char; don't overwrite it.
            if (tp.dataToBreak.length() > tp.srcLine->size()) {
                tp.srcLine->addElement(lineNum, status);
                tp.srcCol ->addElement(column, status);
            }
            break;


        case PARSE_NUM:
            // We are parsing an expected numeric tag value, like <1234>,
            //   within a chunk of data.
            if (u_isUWhiteSpace(c)) {
                break;
            }

            if (c == CH_GT) {
                // Finished the number.  Add the info to the expected break data,
                //   and switch parse state back to doing plain data.
                parseState = PARSE_DATA;
                if (tagValue == 0) {
                    tagValue = -1;
                }
                int32_t  breakIdx = tp.dataToBreak.length();
                tp.expectedBreaks->setSize(breakIdx+1);
                tp.expectedBreaks->setElementAt(tagValue, breakIdx);
                tp.srcLine->setSize(breakIdx+1);
                tp.srcLine->setElementAt(lineNum, breakIdx);
                tp.srcCol ->setSize(breakIdx+1);
                tp.srcCol ->setElementAt(column, breakIdx);
                break;
            }

            if (u_isdigit(c)) {
                tagValue = tagValue*10 + u_charDigitValue(c);
                break;
            }

            errln("Syntax Error in test file at line %d, col %d",
                lineNum, column);
            parseState = PARSE_COMMENT;
            goto end_test; // Stop the test
            break;
        }


        if (U_FAILURE(status)) {
            dataerrln("ICU Error %s while parsing test file at line %d.",
                u_errorName(status), lineNum);
            status = U_ZERO_ERROR;
            goto end_test; // Stop the test
        }

    }

end_test:
    delete [] testFile;
#endif
}


//-------------------------------------------------------------------------------
//
//  TestDictRules   create a break iterator from source rules that includes a
//                  dictionary range.   Regression for bug #7130.  Source rules
//                  do not declare a break iterator type (word, line, sentence, etc.
//                  but the dictionary code, without a type, would loop.
//
//-------------------------------------------------------------------------------
void RBBITest::TestDictRules() {
    const char *rules =  "$dictionary = [a-z]; \n"
                         "!!forward; \n"
                         "$dictionary $dictionary; \n"
                         "!!reverse; \n"
                         "$dictionary $dictionary; \n";
    const char *text = "aa";
    UErrorCode status = U_ZERO_ERROR;
    UParseError parseError;

    RuleBasedBreakIterator bi(rules, parseError, status);
    if (U_SUCCESS(status)) {
        UnicodeString utext = text;
        bi.setText(utext);
        int32_t position;
        int32_t loops;
        for (loops = 0; loops<10; loops++) {
            position = bi.next();
            if (position == RuleBasedBreakIterator::DONE) {
                break;
            }
        }
        TEST_ASSERT(loops == 1);
    } else {
        dataerrln("Error creating RuleBasedBreakIterator: %s", u_errorName(status));
    }
}



//-------------------------------------------------------------------------------
//
//    ReadAndConvertFile   Read a text data file, convert it to UChars, and
//    return the datain one big UChar * buffer, which the caller must delete.
//
//    parameters:
//          fileName:   the name of the file, with no directory part.  The test data directory
//                      is assumed.
//          ulen        an out parameter, receives the actual length (in UChars) of the file data.
//          encoding    The file encoding.  If the file contains a BOM, that will override the encoding
//                      specified here.  The BOM, if it exists, will be stripped from the returned data.
//                      Pass NULL for the system default encoding.
//          status
//    returns:
//                      The file data, converted to UChar.
//                      The caller must delete this when done with
//                           delete [] theBuffer;
//
//    TODO:  This is a clone of RegexTest::ReadAndConvertFile.
//           Move this function to some common place.
//
//--------------------------------------------------------------------------------
UChar *RBBITest::ReadAndConvertFile(const char *fileName, int &ulen, const char *encoding, UErrorCode &status) {
    UChar       *retPtr  = NULL;
    char        *fileBuf = NULL;
    UConverter* conv     = NULL;
    FILE        *f       = NULL;

    ulen = 0;
    if (U_FAILURE(status)) {
        return retPtr;
    }

    //
    //  Open the file.
    //
    f = fopen(fileName, "rb");
    if (f == 0) {
        dataerrln("Error opening test data file %s\n", fileName);
        status = U_FILE_ACCESS_ERROR;
        return NULL;
    }
    //
    //  Read it in
    //
    int   fileSize;
    int   amt_read;

    fseek( f, 0, SEEK_END);
    fileSize = ftell(f);
    fileBuf = new char[fileSize];
    fseek(f, 0, SEEK_SET);
    amt_read = fread(fileBuf, 1, fileSize, f);
    if (amt_read != fileSize || fileSize <= 0) {
        errln("Error reading test data file.");
        goto cleanUpAndReturn;
    }

    //
    // Look for a Unicode Signature (BOM) on the data just read
    //
    int32_t        signatureLength;
    const char *   fileBufC;
    const char*    bomEncoding;

    fileBufC = fileBuf;
    bomEncoding = ucnv_detectUnicodeSignature(
        fileBuf, fileSize, &signatureLength, &status);
    if(bomEncoding!=NULL ){
        fileBufC  += signatureLength;
        fileSize  -= signatureLength;
        encoding = bomEncoding;
    }

    //
    // Open a converter to take the rule file to UTF-16
    //
    conv = ucnv_open(encoding, &status);
    if (U_FAILURE(status)) {
        goto cleanUpAndReturn;
    }

    //
    // Convert the rules to UChar.
    //  Preflight first to determine required buffer size.
    //
    ulen = ucnv_toUChars(conv,
        NULL,           //  dest,
        0,              //  destCapacity,
        fileBufC,
        fileSize,
        &status);
    if (status == U_BUFFER_OVERFLOW_ERROR) {
        // Buffer Overflow is expected from the preflight operation.
        status = U_ZERO_ERROR;

        retPtr = new UChar[ulen+1];
        ucnv_toUChars(conv,
            retPtr,       //  dest,
            ulen+1,
            fileBufC,
            fileSize,
            &status);
    }

cleanUpAndReturn:
    fclose(f);
    delete []fileBuf;
    ucnv_close(conv);
    if (U_FAILURE(status)) {
        errln("ucnv_toUChars: ICU Error \"%s\"\n", u_errorName(status));
        delete []retPtr;
        retPtr = 0;
        ulen   = 0;
    };
    return retPtr;
}



//--------------------------------------------------------------------------------------------
//
//   Run tests from each of the boundary test data files distributed by the Unicode Consortium
//
//-------------------------------------------------------------------------------------------
void RBBITest::TestUnicodeFiles() {
    RuleBasedBreakIterator  *bi;
    UErrorCode               status = U_ZERO_ERROR;

    bi =  (RuleBasedBreakIterator *)BreakIterator::createCharacterInstance(Locale::getEnglish(), status);
    TEST_ASSERT_SUCCESS(status);
    if (U_SUCCESS(status)) {
        runUnicodeTestData("GraphemeBreakTest.txt", bi);
    }
    delete bi;

    bi =  (RuleBasedBreakIterator *)BreakIterator::createWordInstance(Locale::getEnglish(), status);
    TEST_ASSERT_SUCCESS(status);
    if (U_SUCCESS(status)) {
        runUnicodeTestData("WordBreakTest.txt", bi);
    }
    delete bi;

    bi =  (RuleBasedBreakIterator *)BreakIterator::createSentenceInstance(Locale::getEnglish(), status);
    TEST_ASSERT_SUCCESS(status);
    if (U_SUCCESS(status)) {
        runUnicodeTestData("SentenceBreakTest.txt", bi);
    }
    delete bi;

    bi =  (RuleBasedBreakIterator *)BreakIterator::createLineInstance(Locale::getEnglish(), status);
    TEST_ASSERT_SUCCESS(status);
    if (U_SUCCESS(status)) {
        runUnicodeTestData("LineBreakTest.txt", bi);
    }
    delete bi;
}


//--------------------------------------------------------------------------------------------
//
//   Run tests from one of the boundary test data files distributed by the Unicode Consortium
//
//-------------------------------------------------------------------------------------------
void RBBITest::runUnicodeTestData(const char *fileName, RuleBasedBreakIterator *bi) {
#if !UCONFIG_NO_REGULAR_EXPRESSIONS
    // TODO(andy): Match line break behavior to Unicode 6.0 and remove this time bomb. Ticket #7270
    UBool isTicket7270Fixed = !logKnownIssue("7270");
    UBool isLineBreak = 0 == strcmp(fileName, "LineBreakTest.txt");
    UErrorCode  status = U_ZERO_ERROR;

    //
    //  Open and read the test data file, put it into a UnicodeString.
    //
    const char *testDataDirectory = IntlTest::getSourceTestData(status);
    char testFileName[1000];
    if (testDataDirectory == NULL || strlen(testDataDirectory) >= sizeof(testFileName)) {
        dataerrln("Can't open test data.  Path too long.");
        return;
    }
    strcpy(testFileName, testDataDirectory);
    strcat(testFileName, fileName);
    
    logln("Opening data file %s\n", fileName);

    int    len;
    UChar *testFile = ReadAndConvertFile(testFileName, len, "UTF-8", status);
    if (status != U_FILE_ACCESS_ERROR) {
        TEST_ASSERT_SUCCESS(status);
        TEST_ASSERT(testFile != NULL);
    }
    if (U_FAILURE(status) || testFile == NULL) {
        return; /* something went wrong, error already output */
    }
    UnicodeString testFileAsString(TRUE, testFile, len);

    //
    //  Parse the test data file using a regular expression.
    //  Each kind of token is recognized in its own capture group; what type of item was scanned
    //     is identified by which group had a match.
    //
    //    Caputure Group #                  1          2            3            4           5
    //    Parses this item:               divide       x      hex digits   comment \n  unrecognized \n
    //
    UnicodeString tokenExpr("[ \t]*(?:(\\u00F7)|(\\u00D7)|([0-9a-fA-F]+)|((?:#.*?)?$.)|(.*?$.))", -1, US_INV);
    RegexMatcher    tokenMatcher(tokenExpr, testFileAsString, UREGEX_MULTILINE | UREGEX_DOTALL, status);
    UnicodeString   testString;
    UVector32       breakPositions(status);
    int             lineNumber = 1;
    TEST_ASSERT_SUCCESS(status);
    if (U_FAILURE(status)) {
        return;
    }

    //
    //  Scan through each test case, building up the string to be broken in testString,
    //   and the positions that should be boundaries in the breakPositions vector.
    //
    int spin = 0;
    while (tokenMatcher.find()) {
      	if(tokenMatcher.hitEnd()) {
          /* Shouldnt Happen(TM).  This means we didn't find the symbols we were looking for.
             This occurred when the text file was corrupt (wasn't marked as UTF-8)
             and caused an infinite loop here on EBCDIC systems!
          */
          fprintf(stderr,"FAIL: hit end of file %s for the %8dth time- corrupt data file?\r", fileName, ++spin);
          //	   return;
      	}
        if (tokenMatcher.start(1, status) >= 0) {
            // Scanned a divide sign, indicating a break position in the test data.
            if (testString.length()>0) {
                breakPositions.addElement(testString.length(), status);
            }
        }
        else if (tokenMatcher.start(2, status) >= 0) {
            // Scanned an 'x', meaning no break at this position in the test data
            //   Nothing to be done here.
            }
        else if (tokenMatcher.start(3, status) >= 0) {
            // Scanned Hex digits.  Convert them to binary, append to the character data string.
            const UnicodeString &hexNumber = tokenMatcher.group(3, status);
            int length = hexNumber.length();
            if (length<=8) {
                char buf[10];
                hexNumber.extract (0, length, buf, sizeof(buf), US_INV);
                UChar32 c = (UChar32)strtol(buf, NULL, 16);
                if (c<=0x10ffff) {
                    testString.append(c);
                } else {
                    errln("Error: Unicode Character value out of range. \'%s\', line %d.\n",
                       fileName, lineNumber);
                }
            } else {
                errln("Syntax Error: Hex Unicode Character value must have no more than 8 digits at \'%s\', line %d.\n",
                       fileName, lineNumber);
             }
        }
        else if (tokenMatcher.start(4, status) >= 0) {
            // Scanned to end of a line, possibly skipping over a comment in the process.
            //   If the line from the file contained test data, run the test now.
            //
            if (testString.length() > 0) {
// TODO(andy): Remove this time bomb code. Note: Failing line numbers may change when updating to new Unicode data.
//             Rule 8 
//                ZW SP* <break>
//             is not yet implemented.
if (!(isLineBreak && !isTicket7270Fixed && (5198 == lineNumber || 
                                            5202 == lineNumber ||
                                            5214 == lineNumber ||
                                            5246 == lineNumber ||
                                            5298 == lineNumber ||
                                            5302 == lineNumber ))) {
                checkUnicodeTestCase(fileName, lineNumber, testString, &breakPositions, bi);
}
            }

            // Clear out this test case.
            //    The string and breakPositions vector will be refilled as the next
            //       test case is parsed.
            testString.remove();
            breakPositions.removeAllElements();
            lineNumber++;
        } else {
            // Scanner catchall.  Something unrecognized appeared on the line.
            char token[16];
            UnicodeString uToken = tokenMatcher.group(0, status);
            uToken.extract(0, uToken.length(), token, (uint32_t)sizeof(token));
            token[sizeof(token)-1] = 0;
            errln("Syntax error in test data file \'%s\', line %d.  Scanning \"%s\"\n", fileName, lineNumber, token);

            // Clean up, in preparation for continuing with the next line.
            testString.remove();
            breakPositions.removeAllElements();
            lineNumber++;
        }
        TEST_ASSERT_SUCCESS(status);
        if (U_FAILURE(status)) {
            break;
        }
    }

    delete [] testFile;
 #endif   // !UCONFIG_NO_REGULAR_EXPRESSIONS
}

//--------------------------------------------------------------------------------------------
//
//   checkUnicodeTestCase()   Run one test case from one of the Unicode Consortium
//                            test data files.  Do only a simple, forward-only check -
//                            this test is mostly to check that ICU and the Unicode
//                            data agree with each other.
//
//--------------------------------------------------------------------------------------------
void RBBITest::checkUnicodeTestCase(const char *testFileName, int lineNumber,
                         const UnicodeString &testString,   // Text data to be broken
                         UVector32 *breakPositions,         // Positions where breaks should be found.
                         RuleBasedBreakIterator *bi) {
    int32_t pos;                 // Break Position in the test string
    int32_t expectedI = 0;       // Index of expected break position in the vector of expected results.
    int32_t expectedPos;         // Expected break position (index into test string)

    bi->setText(testString);
    pos = bi->first();
    pos = bi->next();

    while (pos != BreakIterator::DONE) {
        if (expectedI >= breakPositions->size()) {
            errln("Test file \"%s\", line %d, unexpected break found at position %d",
                testFileName, lineNumber, pos);
            break;
        }
        expectedPos = breakPositions->elementAti(expectedI);
        if (pos < expectedPos) {
            errln("Test file \"%s\", line %d, unexpected break found at position %d",
                testFileName, lineNumber, pos);
            break;
        }
        if (pos > expectedPos) {
            errln("Test file \"%s\", line %d, failed to find expected break at position %d",
                testFileName, lineNumber, expectedPos);
            break;
        }
        pos = bi->next();
        expectedI++;
    }

    if (pos==BreakIterator::DONE && expectedI<breakPositions->size()) {
        errln("Test file \"%s\", line %d, failed to find expected break at position %d",
            testFileName, lineNumber, breakPositions->elementAti(expectedI));
    }
}



#if !UCONFIG_NO_REGULAR_EXPRESSIONS
//---------------------------------------------------------------------------------------
//
//   classs RBBIMonkeyKind
//
//      Monkey Test for Break Iteration
//      Abstract interface class.   Concrete derived classes independently
//      implement the break rules for different iterator types.
//
//      The Monkey Test itself uses doesn't know which type of break iterator it is
//      testing, but works purely in terms of the interface defined here.
//
//---------------------------------------------------------------------------------------
class RBBIMonkeyKind {
public:
    // Return a UVector of UnicodeSets, representing the character classes used
    //   for this type of iterator.
    virtual  UVector  *charClasses() = 0;

    // Set the test text on which subsequent calls to next() will operate
    virtual  void      setText(const UnicodeString &s) = 0;

    // Find the next break postion, starting from the prev break position, or from zero.
    // Return -1 after reaching end of string.
    virtual  int32_t   next(int32_t i) = 0;

    virtual ~RBBIMonkeyKind();
    UErrorCode       deferredStatus;


protected:
    RBBIMonkeyKind();

private:
};

RBBIMonkeyKind::RBBIMonkeyKind() {
    deferredStatus = U_ZERO_ERROR;
}

RBBIMonkeyKind::~RBBIMonkeyKind() {
}


//----------------------------------------------------------------------------------------
//
//   Random Numbers.  Similar to standard lib rand() and srand()
//                    Not using library to
//                      1.  Get same results on all platforms.
//                      2.  Get access to current seed, to more easily reproduce failures.
//
//---------------------------------------------------------------------------------------
static uint32_t m_seed = 1;

static uint32_t m_rand()
{
    m_seed = m_seed * 1103515245 + 12345;
    return (uint32_t)(m_seed/65536) % 32768;
}


//------------------------------------------------------------------------------------------
//
//   class RBBICharMonkey      Character (Grapheme Cluster) specific implementation
//                             of RBBIMonkeyKind.
//
//------------------------------------------------------------------------------------------
class RBBICharMonkey: public RBBIMonkeyKind {
public:
    RBBICharMonkey();
    virtual          ~RBBICharMonkey();
    virtual  UVector *charClasses();
    virtual  void     setText(const UnicodeString &s);
    virtual  int32_t  next(int32_t i);
private:
    UVector   *fSets;

    UnicodeSet  *fCRLFSet;
    UnicodeSet  *fControlSet;
    UnicodeSet  *fExtendSet;
    UnicodeSet  *fRegionalIndicatorSet;
    UnicodeSet  *fPrependSet;
    UnicodeSet  *fSpacingSet;
    UnicodeSet  *fLSet;
    UnicodeSet  *fVSet;
    UnicodeSet  *fTSet;
    UnicodeSet  *fLVSet;
    UnicodeSet  *fLVTSet;
    UnicodeSet  *fHangulSet;
    UnicodeSet  *fAnySet;

    const UnicodeString *fText;
};


RBBICharMonkey::RBBICharMonkey() {
    UErrorCode  status = U_ZERO_ERROR;

    fText = NULL;

    fCRLFSet    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\r\\n]"), status);
    fControlSet = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Grapheme_Cluster_Break = Control}]"), status);
    fExtendSet  = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Grapheme_Cluster_Break = Extend}]"), status);
    fRegionalIndicatorSet = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Grapheme_Cluster_Break = Regional_Indicator}]"), status);
    fPrependSet = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Grapheme_Cluster_Break = Prepend}]"), status);
    fSpacingSet = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Grapheme_Cluster_Break = SpacingMark}]"), status);
    fLSet       = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Grapheme_Cluster_Break = L}]"), status);
    fVSet       = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Grapheme_Cluster_Break = V}]"), status);
    fTSet       = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Grapheme_Cluster_Break = T}]"), status);
    fLVSet      = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Grapheme_Cluster_Break = LV}]"), status);
    fLVTSet     = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Grapheme_Cluster_Break = LVT}]"), status);
    fHangulSet  = new UnicodeSet();
    fHangulSet->addAll(*fLSet);
    fHangulSet->addAll(*fVSet);
    fHangulSet->addAll(*fTSet);
    fHangulSet->addAll(*fLVSet);
    fHangulSet->addAll(*fLVTSet);
    fAnySet     = new UnicodeSet(0, 0x10ffff);

    fSets       = new UVector(status);
    fSets->addElement(fCRLFSet,    status);
    fSets->addElement(fControlSet, status);
    fSets->addElement(fExtendSet,  status);
    fSets->addElement(fRegionalIndicatorSet, status);
    if (!fPrependSet->isEmpty()) {
        fSets->addElement(fPrependSet, status);
    }
    fSets->addElement(fSpacingSet, status);
    fSets->addElement(fHangulSet,  status);
    fSets->addElement(fAnySet,     status);
    if (U_FAILURE(status)) {
        deferredStatus = status;
    }
}


void RBBICharMonkey::setText(const UnicodeString &s) {
    fText = &s;
}



int32_t RBBICharMonkey::next(int32_t prevPos) {
    int    p0, p1, p2, p3;    // Indices of the significant code points around the
                              //   break position being tested.  The candidate break
                              //   location is before p2.

    int     breakPos = -1;

    UChar32 c0, c1, c2, c3;   // The code points at p0, p1, p2 & p3.
    
    if (U_FAILURE(deferredStatus)) {
        return -1;
    }

    // Previous break at end of string.  return DONE.
    if (prevPos >= fText->length()) {
        return -1;
    }
    p0 = p1 = p2 = p3 = prevPos;
    c3 =  fText->char32At(prevPos);
    c0 = c1 = c2 = 0;
    (void)p0;   // suppress set but not used warning.
    (void)c0;

    // Loop runs once per "significant" character position in the input text.
    for (;;) {
        // Move all of the positions forward in the input string.
        p0 = p1;  c0 = c1;
        p1 = p2;  c1 = c2;
        p2 = p3;  c2 = c3;

        // Advancd p3 by one codepoint
        p3 = fText->moveIndex32(p3, 1);
        c3 = fText->char32At(p3);

        if (p1 == p2) {
            // Still warming up the loop.  (won't work with zero length strings, but we don't care)
            continue;
        }
        if (p2 == fText->length()) {
            // Reached end of string.  Always a break position.
            break;
        }

        // Rule  GB3   CR x LF
        //     No Extend or Format characters may appear between the CR and LF,
        //     which requires the additional check for p2 immediately following p1.
        //
        if (c1==0x0D && c2==0x0A && p1==(p2-1)) {
            continue;
        }

        // Rule (GB4).   ( Control | CR | LF ) <break>
        if (fControlSet->contains(c1) ||
            c1 == 0x0D ||
            c1 == 0x0A)  {
            break;
        }

        // Rule (GB5)    <break>  ( Control | CR | LF )
        //
        if (fControlSet->contains(c2) ||
            c2 == 0x0D ||
            c2 == 0x0A)  {
            break;
        }


        // Rule (GB6)  L x ( L | V | LV | LVT )
        if (fLSet->contains(c1) &&
               (fLSet->contains(c2)  ||
                fVSet->contains(c2)  ||
                fLVSet->contains(c2) ||
                fLVTSet->contains(c2))) {
            continue;
        }

        // Rule (GB7)    ( LV | V )  x  ( V | T )
        if ((fLVSet->contains(c1) || fVSet->contains(c1)) &&
            (fVSet->contains(c2) || fTSet->contains(c2)))  {
            continue;
        }

        // Rule (GB8)    ( LVT | T)  x T
        if ((fLVTSet->contains(c1) || fTSet->contains(c1)) &&
            fTSet->contains(c2))  {
            continue;
        }

        // Rule (GB8a)    Regional_Indicator x Regional_Indicator
        if (fRegionalIndicatorSet->contains(c1) && fRegionalIndicatorSet->contains(c2)) {
            continue;
        }

        // Rule (GB9)    Numeric x ALetter
        if (fExtendSet->contains(c2))  {
            continue;
        }

        // Rule (GB9a)   x  SpacingMark
        if (fSpacingSet->contains(c2)) {
            continue;
        }

        // Rule (GB9b)   Prepend x
        if (fPrependSet->contains(c1)) {
            continue;
        }

        // Rule (GB10)  Any  <break>  Any
        break;
    }

    breakPos = p2;
    return breakPos;
}



UVector  *RBBICharMonkey::charClasses() {
    return fSets;
}


RBBICharMonkey::~RBBICharMonkey() {
    delete fSets;
    delete fCRLFSet;
    delete fControlSet;
    delete fExtendSet;
    delete fRegionalIndicatorSet;
    delete fPrependSet;
    delete fSpacingSet;
    delete fLSet;
    delete fVSet;
    delete fTSet;
    delete fLVSet;
    delete fLVTSet;
    delete fHangulSet;
    delete fAnySet;
}

//------------------------------------------------------------------------------------------
//
//   class RBBIWordMonkey      Word Break specific implementation
//                             of RBBIMonkeyKind.
//
//------------------------------------------------------------------------------------------
class RBBIWordMonkey: public RBBIMonkeyKind {
public:
    RBBIWordMonkey();
    virtual          ~RBBIWordMonkey();
    virtual  UVector *charClasses();
    virtual  void     setText(const UnicodeString &s);
    virtual int32_t   next(int32_t i);
private:
    UVector      *fSets;

    UnicodeSet  *fCRSet;
    UnicodeSet  *fLFSet;
    UnicodeSet  *fNewlineSet;
    UnicodeSet  *fRegionalIndicatorSet;
    UnicodeSet  *fKatakanaSet;
    UnicodeSet  *fHebrew_LetterSet;
    UnicodeSet  *fALetterSet;
    // TODO(jungshik): Do we still need this change? 
    // UnicodeSet  *fALetterSet; // matches ALetterPlus in word.txt
    UnicodeSet  *fSingle_QuoteSet;
    UnicodeSet  *fDouble_QuoteSet;
    UnicodeSet  *fMidNumLetSet;
    UnicodeSet  *fMidLetterSet;
    UnicodeSet  *fMidNumSet;
    UnicodeSet  *fNumericSet;
    UnicodeSet  *fFormatSet;
    UnicodeSet  *fOtherSet;
    UnicodeSet  *fExtendSet;
    UnicodeSet  *fExtendNumLetSet;
    UnicodeSet  *fDictionaryCjkSet;

    const UnicodeString  *fText;
};


RBBIWordMonkey::RBBIWordMonkey()
{
    UErrorCode  status = U_ZERO_ERROR;

    fSets            = new UVector(status);

    fCRSet           = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Word_Break = CR}]"),           status);
    fLFSet           = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Word_Break = LF}]"),           status);
    fNewlineSet      = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Word_Break = Newline}]"),      status);
    fDictionaryCjkSet= new UnicodeSet("[[\\uac00-\\ud7a3][:Han:][:Hiragana:][:Katakana:]]", status);
    // Exclude Hangul syllables from ALetterSet during testing.
    // Leave CJK dictionary characters out from the monkey tests!
#if 0 
    fALetterSet      = new UnicodeSet("[\\p{Word_Break = ALetter}"
                                      "[\\p{Line_Break = Complex_Context}"
                                      "-\\p{Grapheme_Cluster_Break = Extend}"
                                      "-\\p{Grapheme_Cluster_Break = Control}"
                                      "]]",
                                      status);
#endif
    fRegionalIndicatorSet =  new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Word_Break = Regional_Indicator}]"), status);
    fKatakanaSet      = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Word_Break = Katakana}]"),     status);
    fHebrew_LetterSet = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Word_Break = Hebrew_Letter}]"), status);
    fALetterSet       = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Word_Break = ALetter}]"), status);
    fALetterSet->removeAll(*fDictionaryCjkSet);
    fSingle_QuoteSet  = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Word_Break = Single_Quote}]"),    status);
    fDouble_QuoteSet  = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Word_Break = Double_Quote}]"),    status);
    fMidNumLetSet     = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Word_Break = MidNumLet}]"),    status);
    fMidLetterSet     = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Word_Break = MidLetter}]"),    status);
    fMidNumSet        = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Word_Break = MidNum}]"),       status);
    // TODO: this set used to contain [\\uff10-\\uff19] (fullwidth digits), but this breaks the test
    // we should figure out why
    fNumericSet       = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Word_Break = Numeric}]"),      status);
    fFormatSet        = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Word_Break = Format}]"),       status);
    fExtendNumLetSet  = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Word_Break = ExtendNumLet}]"), status);
    fExtendSet        = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Word_Break = Extend}]"),       status);

    fOtherSet        = new UnicodeSet();
    if(U_FAILURE(status)) {
      deferredStatus = status;
      return;
    }

    fOtherSet->complement();
    fOtherSet->removeAll(*fCRSet);
    fOtherSet->removeAll(*fLFSet);
    fOtherSet->removeAll(*fNewlineSet);
    fOtherSet->removeAll(*fKatakanaSet);
    fOtherSet->removeAll(*fHebrew_LetterSet);
    fOtherSet->removeAll(*fALetterSet);
    fOtherSet->removeAll(*fSingle_QuoteSet);
    fOtherSet->removeAll(*fDouble_QuoteSet);
    fOtherSet->removeAll(*fMidLetterSet);
    fOtherSet->removeAll(*fMidNumSet);
    fOtherSet->removeAll(*fNumericSet);
    fOtherSet->removeAll(*fExtendNumLetSet);
    fOtherSet->removeAll(*fFormatSet);
    fOtherSet->removeAll(*fExtendSet);
    fOtherSet->removeAll(*fRegionalIndicatorSet);
    // Inhibit dictionary characters from being tested at all.
    fOtherSet->removeAll(*fDictionaryCjkSet);
    fOtherSet->removeAll(UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{LineBreak = Complex_Context}]"), status));

    fSets->addElement(fCRSet,                status);
    fSets->addElement(fLFSet,                status);
    fSets->addElement(fNewlineSet,           status);
    fSets->addElement(fRegionalIndicatorSet, status);
    fSets->addElement(fHebrew_LetterSet,     status);
    fSets->addElement(fALetterSet,           status);
    fSets->addElement(fSingle_QuoteSet,      status);
    fSets->addElement(fDouble_QuoteSet,      status);
    //fSets->addElement(fKatakanaSet,          status); //TODO: work out how to test katakana
    fSets->addElement(fMidLetterSet,         status);
    fSets->addElement(fMidNumLetSet,         status);
    fSets->addElement(fMidNumSet,            status);
    fSets->addElement(fNumericSet,           status);
    fSets->addElement(fFormatSet,            status);
    fSets->addElement(fExtendSet,            status);
    fSets->addElement(fOtherSet,             status);
    fSets->addElement(fExtendNumLetSet,      status);

    if (U_FAILURE(status)) {
        deferredStatus = status;
    }
}

void RBBIWordMonkey::setText(const UnicodeString &s) {
    fText       = &s;
}


int32_t RBBIWordMonkey::next(int32_t prevPos) {
    int    p0, p1, p2, p3;    // Indices of the significant code points around the
                              //   break position being tested.  The candidate break
                              //   location is before p2.

    int     breakPos = -1;

    UChar32 c0, c1, c2, c3;   // The code points at p0, p1, p2 & p3.
    
    if (U_FAILURE(deferredStatus)) {
        return -1;
    }

    // Prev break at end of string.  return DONE.
    if (prevPos >= fText->length()) {
        return -1;
    }
    p0 = p1 = p2 = p3 = prevPos;
    c3 =  fText->char32At(prevPos);
    c0 = c1 = c2 = 0;
    (void)p0;       // Suppress set but not used warning.

    // Loop runs once per "significant" character position in the input text.
    for (;;) {
        // Move all of the positions forward in the input string.
        p0 = p1;  c0 = c1;
        p1 = p2;  c1 = c2;
        p2 = p3;  c2 = c3;

        // Advancd p3 by    X(Extend | Format)*   Rule 4
        //    But do not advance over Extend & Format following a new line. (Unicode 5.1 change)
        do {
            p3 = fText->moveIndex32(p3, 1);
            c3 = fText->char32At(p3);
            if (fCRSet->contains(c2) || fLFSet->contains(c2) || fNewlineSet->contains(c2)) {
               break;
            };
        }
        while (fFormatSet->contains(c3) || fExtendSet->contains(c3));


        if (p1 == p2) {
            // Still warming up the loop.  (won't work with zero length strings, but we don't care)
            continue;
        }
        if (p2 == fText->length()) {
            // Reached end of string.  Always a break position.
            break;
        }

        // Rule  (3)   CR x LF
        //     No Extend or Format characters may appear between the CR and LF,
        //     which requires the additional check for p2 immediately following p1.
        //
        if (c1==0x0D && c2==0x0A) {
            continue;
        }
        
        // Rule (3a)  Break before and after newlines (including CR and LF)
        //
        if (fCRSet->contains(c1) || fLFSet->contains(c1) || fNewlineSet->contains(c1)) {
            break;
        };
        if (fCRSet->contains(c2) || fLFSet->contains(c2) || fNewlineSet->contains(c2)) {
            break;
        };

        // Rule (5).   (ALetter | Hebrew_Letter) x (ALetter | Hebrew_Letter)
        if ((fALetterSet->contains(c1) || fHebrew_LetterSet->contains(c1)) &&
            (fALetterSet->contains(c2) || fHebrew_LetterSet->contains(c2)))  {
            continue;
        }

        // Rule (6)  (ALetter | Hebrew_Letter)  x  (MidLetter | MidNumLet | Single_Quote) (ALetter | Hebrew_Letter)
        //
        if ( (fALetterSet->contains(c1) || fHebrew_LetterSet->contains(c1))   &&
             (fMidLetterSet->contains(c2) || fMidNumLetSet->contains(c2) || fSingle_QuoteSet->contains(c2)) &&
             (fALetterSet->contains(c3) || fHebrew_LetterSet->contains(c3))) {
            continue;
        }

        // Rule (7)  (ALetter | Hebrew_Letter) (MidLetter | MidNumLet | Single_Quote)  x  (ALetter | Hebrew_Letter)
        if ((fALetterSet->contains(c0) || fHebrew_LetterSet->contains(c0)) &&
            (fMidLetterSet->contains(c1) || fMidNumLetSet->contains(c1) || fSingle_QuoteSet->contains(c1)) &&
            (fALetterSet->contains(c2) || fHebrew_LetterSet->contains(c2))) {
            continue;
        }

        // Rule (7a)     Hebrew_Letter x Single_Quote
        if (fHebrew_LetterSet->contains(c1) && fSingle_QuoteSet->contains(c2)) {
            continue;
        }

        // Rule (7b)    Hebrew_Letter x Double_Quote Hebrew_Letter
        if (fHebrew_LetterSet->contains(c1) && fDouble_QuoteSet->contains(c2) && fHebrew_LetterSet->contains(c3)) {
            continue;
        }

        // Rule (7c)    Hebrew_Letter Double_Quote x Hebrew_Letter
        if (fHebrew_LetterSet->contains(c0) && fDouble_QuoteSet->contains(c1) && fHebrew_LetterSet->contains(c2)) {
            continue;
        }

        // Rule (8)    Numeric x Numeric
        if (fNumericSet->contains(c1) &&
            fNumericSet->contains(c2))  {
            continue;
        }

        // Rule (9)    (ALetter | Hebrew_Letter) x Numeric
        if ((fALetterSet->contains(c1) || fHebrew_LetterSet->contains(c1)) &&
            fNumericSet->contains(c2))  {
            continue;
        }

        // Rule (10)    Numeric x (ALetter | Hebrew_Letter)
        if (fNumericSet->contains(c1) &&
            (fALetterSet->contains(c2) || fHebrew_LetterSet->contains(c2)))  {
            continue;
        }

        // Rule (11)   Numeric (MidNum | MidNumLet | Single_Quote)  x  Numeric
        if (fNumericSet->contains(c0) &&
            (fMidNumSet->contains(c1) || fMidNumLetSet->contains(c1) || fSingle_QuoteSet->contains(c1))  &&
            fNumericSet->contains(c2)) {
            continue;
        }

        // Rule (12)  Numeric x (MidNum | MidNumLet | SingleQuote) Numeric
        if (fNumericSet->contains(c1) &&
            (fMidNumSet->contains(c2) || fMidNumLetSet->contains(c2) || fSingle_QuoteSet->contains(c2))  &&
            fNumericSet->contains(c3)) {
            continue;
        }

        // Rule (13)  Katakana x Katakana
        if (fKatakanaSet->contains(c1) &&
            fKatakanaSet->contains(c2))  {
            continue;
        }

        // Rule 13a    (ALetter | Hebrew_Letter | Numeric | KataKana | ExtendNumLet) x ExtendNumLet
        if ((fALetterSet->contains(c1) || fHebrew_LetterSet->contains(c1) ||fNumericSet->contains(c1) ||
             fKatakanaSet->contains(c1) || fExtendNumLetSet->contains(c1)) &&
             fExtendNumLetSet->contains(c2)) {
                continue;
        }

        // Rule 13b   ExtendNumLet x (ALetter | Hebrew_Letter | Numeric | Katakana)
        if (fExtendNumLetSet->contains(c1) &&
                (fALetterSet->contains(c2) || fHebrew_LetterSet->contains(c2) ||
                 fNumericSet->contains(c2) || fKatakanaSet->contains(c2)))  {
            continue;
        }

        // Rule 13c
        if (fRegionalIndicatorSet->contains(c1) && fRegionalIndicatorSet->contains(c2)) {
            continue;
        }

        // Rule 14.  Break found here.
        break;
    }

    breakPos = p2;
    return breakPos;
}


UVector  *RBBIWordMonkey::charClasses() {
    return fSets;
}


RBBIWordMonkey::~RBBIWordMonkey() {
    delete fSets;
    delete fCRSet;
    delete fLFSet;
    delete fNewlineSet;
    delete fKatakanaSet;
    delete fHebrew_LetterSet;
    delete fALetterSet;
    delete fSingle_QuoteSet;
    delete fDouble_QuoteSet;
    delete fMidNumLetSet;
    delete fMidLetterSet;
    delete fMidNumSet;
    delete fNumericSet;
    delete fFormatSet;
    delete fExtendSet;
    delete fExtendNumLetSet;
    delete fRegionalIndicatorSet;
    delete fDictionaryCjkSet;
    delete fOtherSet;
}




//------------------------------------------------------------------------------------------
//
//   class RBBISentMonkey      Sentence Break specific implementation
//                             of RBBIMonkeyKind.
//
//------------------------------------------------------------------------------------------
class RBBISentMonkey: public RBBIMonkeyKind {
public:
    RBBISentMonkey();
    virtual          ~RBBISentMonkey();
    virtual  UVector *charClasses();
    virtual  void     setText(const UnicodeString &s);
    virtual int32_t   next(int32_t i);
private:
    int               moveBack(int posFrom);
    int               moveForward(int posFrom);
    UChar32           cAt(int pos);

    UVector      *fSets;

    UnicodeSet  *fSepSet;
    UnicodeSet  *fFormatSet;
    UnicodeSet  *fSpSet;
    UnicodeSet  *fLowerSet;
    UnicodeSet  *fUpperSet;
    UnicodeSet  *fOLetterSet;
    UnicodeSet  *fNumericSet;
    UnicodeSet  *fATermSet;
    UnicodeSet  *fSContinueSet;
    UnicodeSet  *fSTermSet;
    UnicodeSet  *fCloseSet;
    UnicodeSet  *fOtherSet;
    UnicodeSet  *fExtendSet;

    const UnicodeString  *fText;

};

RBBISentMonkey::RBBISentMonkey()
{
    UErrorCode  status = U_ZERO_ERROR;

    fSets            = new UVector(status);

    //  Separator Set Note:  Beginning with Unicode 5.1, CR and LF were removed from the separator
    //                       set and made into character classes of their own.  For the monkey impl,
    //                       they remain in SEP, since Sep always appears with CR and LF in the rules.
    fSepSet          = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Sentence_Break = Sep} \\u000a \\u000d]"),     status);
    fFormatSet       = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Sentence_Break = Format}]"),    status);
    fSpSet           = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Sentence_Break = Sp}]"),        status);
    fLowerSet        = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Sentence_Break = Lower}]"),     status);
    fUpperSet        = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Sentence_Break = Upper}]"),     status);
    fOLetterSet      = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Sentence_Break = OLetter}]"),   status);
    fNumericSet      = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Sentence_Break = Numeric}]"),   status);
    fATermSet        = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Sentence_Break = ATerm}]"),     status);
    fSContinueSet    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Sentence_Break = SContinue}]"), status);
    fSTermSet        = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Sentence_Break = STerm}]"),     status);
    fCloseSet        = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Sentence_Break = Close}]"),     status);
    fExtendSet       = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Sentence_Break = Extend}]"),    status);
    fOtherSet        = new UnicodeSet();

    if(U_FAILURE(status)) {
      deferredStatus = status;
      return;
    }

    fOtherSet->complement();
    fOtherSet->removeAll(*fSepSet);
    fOtherSet->removeAll(*fFormatSet);
    fOtherSet->removeAll(*fSpSet);
    fOtherSet->removeAll(*fLowerSet);
    fOtherSet->removeAll(*fUpperSet);
    fOtherSet->removeAll(*fOLetterSet);
    fOtherSet->removeAll(*fNumericSet);
    fOtherSet->removeAll(*fATermSet);
    fOtherSet->removeAll(*fSContinueSet);
    fOtherSet->removeAll(*fSTermSet);
    fOtherSet->removeAll(*fCloseSet);
    fOtherSet->removeAll(*fExtendSet);

    fSets->addElement(fSepSet,       status);
    fSets->addElement(fFormatSet,    status);
    fSets->addElement(fSpSet,        status);
    fSets->addElement(fLowerSet,     status);
    fSets->addElement(fUpperSet,     status);
    fSets->addElement(fOLetterSet,   status);
    fSets->addElement(fNumericSet,   status);
    fSets->addElement(fATermSet,     status);
    fSets->addElement(fSContinueSet, status);
    fSets->addElement(fSTermSet,     status);
    fSets->addElement(fCloseSet,     status);
    fSets->addElement(fOtherSet,     status);
    fSets->addElement(fExtendSet,    status);

    if (U_FAILURE(status)) {
        deferredStatus = status;
    }
}



void RBBISentMonkey::setText(const UnicodeString &s) {
    fText       = &s;
}

UVector  *RBBISentMonkey::charClasses() {
    return fSets;
}


//  moveBack()   Find the "significant" code point preceding the index i.
//               Skips over ($Extend | $Format)* .
//
int RBBISentMonkey::moveBack(int i) {
    if (i <= 0) {
        return -1;
    }
    UChar32   c;
    int32_t   j = i;
    do {
        j = fText->moveIndex32(j, -1);
        c = fText->char32At(j);
    }
    while (j>0 &&(fFormatSet->contains(c) || fExtendSet->contains(c)));
    return j;

 }


int RBBISentMonkey::moveForward(int i) {
    if (i>=fText->length()) {
        return fText->length();
    }
    UChar32   c;
    int32_t   j = i;
    do {
        j = fText->moveIndex32(j, 1);
        c = cAt(j);
    }
    while (fFormatSet->contains(c) || fExtendSet->contains(c));
    return j;
}

UChar32 RBBISentMonkey::cAt(int pos) {
    if (pos<0 || pos>=fText->length()) {
        return -1;
    } else {
        return fText->char32At(pos);
    }
}

int32_t RBBISentMonkey::next(int32_t prevPos) {
    int    p0, p1, p2, p3;    // Indices of the significant code points around the
                              //   break position being tested.  The candidate break
                              //   location is before p2.

    int     breakPos = -1;

    UChar32 c0, c1, c2, c3;   // The code points at p0, p1, p2 & p3.
    UChar32 c;

    if (U_FAILURE(deferredStatus)) {
        return -1;
    }

    // Prev break at end of string.  return DONE.
    if (prevPos >= fText->length()) {
        return -1;
    }
    p0 = p1 = p2 = p3 = prevPos;
    c3 =  fText->char32At(prevPos);
    c0 = c1 = c2 = 0;
    (void)p0;     // Suppress set but not used warning.

    // Loop runs once per "significant" character position in the input text.
    for (;;) {
        // Move all of the positions forward in the input string.
        p0 = p1;  c0 = c1;
        p1 = p2;  c1 = c2;
        p2 = p3;  c2 = c3;

        // Advancd p3 by    X(Extend | Format)*   Rule 4
        p3 = moveForward(p3);
        c3 = cAt(p3);

        // Rule (3)  CR x LF
        if (c1==0x0d && c2==0x0a && p2==(p1+1)) {
            continue;
        }

        // Rule (4).   Sep  <break>
        if (fSepSet->contains(c1)) {
            p2 = p1+1;   // Separators don't combine with Extend or Format.
            break;
        }

        if (p2 >= fText->length()) {
            // Reached end of string.  Always a break position.
            break;
        }

        if (p2 == prevPos) {
            // Still warming up the loop.  (won't work with zero length strings, but we don't care)
            continue;
        }

        // Rule (6).   ATerm x Numeric
        if (fATermSet->contains(c1) &&  fNumericSet->contains(c2))  {
            continue;
        }

        // Rule (7).  Upper ATerm  x  Uppper
        if (fUpperSet->contains(c0) && fATermSet->contains(c1) && fUpperSet->contains(c2)) {
            continue;
        }

        // Rule (8)  ATerm Close* Sp*  x  (not (OLettter | Upper | Lower | Sep | STerm | ATerm))* Lower
        //           Note:  STerm | ATerm are added to the negated part of the expression by a
        //                  note to the Unicode 5.0 documents.
        int p8 = p1;
        while (fSpSet->contains(cAt(p8))) {
            p8 = moveBack(p8);
        }
        while (fCloseSet->contains(cAt(p8))) {
            p8 = moveBack(p8);
        }
        if (fATermSet->contains(cAt(p8))) {
            p8=p2;
            for (;;) {
                c = cAt(p8);
                if (c==-1 || fOLetterSet->contains(c) || fUpperSet->contains(c) ||
                    fLowerSet->contains(c) || fSepSet->contains(c) ||
                    fATermSet->contains(c) || fSTermSet->contains(c))  {
                    break;
                }
                p8 = moveForward(p8);
            }
            if (fLowerSet->contains(cAt(p8))) {
                continue;
            }
        }

        // Rule 8a   (STerm | ATerm) Close* Sp* x (SContinue | STerm | ATerm);
        if (fSContinueSet->contains(c2) || fSTermSet->contains(c2) || fATermSet->contains(c2)) {
            p8 = p1;
            while (fSpSet->contains(cAt(p8))) {
                p8 = moveBack(p8);
            }
            while (fCloseSet->contains(cAt(p8))) {
                p8 = moveBack(p8);
            }
            c = cAt(p8);
            if (fSTermSet->contains(c) || fATermSet->contains(c)) {
                continue;
            }
        }

        // Rule (9)  (STerm | ATerm) Close*  x  (Close | Sp | Sep | CR | LF)
        int p9 = p1;
        while (fCloseSet->contains(cAt(p9))) {
            p9 = moveBack(p9);
        }
        c = cAt(p9);
        if ((fSTermSet->contains(c) || fATermSet->contains(c))) {
            if (fCloseSet->contains(c2) || fSpSet->contains(c2) || fSepSet->contains(c2)) {
                continue;
            }
        }

        // Rule (10)  (Sterm | ATerm) Close* Sp*  x  (Sp | Sep | CR | LF)
        int p10 = p1;
        while (fSpSet->contains(cAt(p10))) {
            p10 = moveBack(p10);
        }
        while (fCloseSet->contains(cAt(p10))) {
            p10 = moveBack(p10);
        }
        if (fSTermSet->contains(cAt(p10)) || fATermSet->contains(cAt(p10))) {
            if (fSpSet->contains(c2) || fSepSet->contains(c2)) {
                continue;
            }
        }

        // Rule (11)  (STerm | ATerm) Close* Sp* (Sep | CR | LF)?  <break>
        int p11 = p1;
        if (fSepSet->contains(cAt(p11))) {
            p11 = moveBack(p11);
        }
        while (fSpSet->contains(cAt(p11))) {
            p11 = moveBack(p11);
        }
        while (fCloseSet->contains(cAt(p11))) {
            p11 = moveBack(p11);
        }
        if (fSTermSet->contains(cAt(p11)) || fATermSet->contains(cAt(p11))) {
            break;
        }

        //  Rule (12)  Any x Any
        continue;
    }
    breakPos = p2;
    return breakPos;
}

RBBISentMonkey::~RBBISentMonkey() {
    delete fSets;
    delete fSepSet;
    delete fFormatSet;
    delete fSpSet;
    delete fLowerSet;
    delete fUpperSet;
    delete fOLetterSet;
    delete fNumericSet;
    delete fATermSet;
    delete fSContinueSet;
    delete fSTermSet;
    delete fCloseSet;
    delete fOtherSet;
    delete fExtendSet;
}



//-------------------------------------------------------------------------------------------
//
//  RBBILineMonkey
//
//-------------------------------------------------------------------------------------------

class RBBILineMonkey: public RBBIMonkeyKind {
public:
    RBBILineMonkey();
    virtual          ~RBBILineMonkey();
    virtual  UVector *charClasses();
    virtual  void     setText(const UnicodeString &s);
    virtual  int32_t  next(int32_t i);
    virtual  void     rule9Adjust(int32_t pos, UChar32 *posChar, int32_t *nextPos, UChar32 *nextChar);
private:
    UVector      *fSets;

    UnicodeSet  *fBK;
    UnicodeSet  *fCR;
    UnicodeSet  *fLF;
    UnicodeSet  *fCM;
    UnicodeSet  *fNL;
    UnicodeSet  *fSG;
    UnicodeSet  *fWJ;
    UnicodeSet  *fZW;
    UnicodeSet  *fGL;
    UnicodeSet  *fCB;
    UnicodeSet  *fSP;
    UnicodeSet  *fB2;
    UnicodeSet  *fBA;
    UnicodeSet  *fBB;
    UnicodeSet  *fHY;
    UnicodeSet  *fH2;
    UnicodeSet  *fH3;
    UnicodeSet  *fCL;
    UnicodeSet  *fCP;
    UnicodeSet  *fEX;
    UnicodeSet  *fIN;
    UnicodeSet  *fJL;
    UnicodeSet  *fJV;
    UnicodeSet  *fJT;
    UnicodeSet  *fNS;
    UnicodeSet  *fOP;
    UnicodeSet  *fQU;
    UnicodeSet  *fIS;
    UnicodeSet  *fNU;
    UnicodeSet  *fPO;
    UnicodeSet  *fPR;
    UnicodeSet  *fSY;
    UnicodeSet  *fAI;
    UnicodeSet  *fAL;
    UnicodeSet  *fCJ;
    UnicodeSet  *fHL;
    UnicodeSet  *fID;
    UnicodeSet  *fRI;
    UnicodeSet  *fSA;
    UnicodeSet  *fXX;

    BreakIterator        *fCharBI;
    const UnicodeString  *fText;
    RegexMatcher         *fNumberMatcher;
};


RBBILineMonkey::RBBILineMonkey()
{
    UErrorCode  status = U_ZERO_ERROR;

    fSets  = new UVector(status);

    fBK    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_Break=BK}]"), status);
    fCR    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=CR}]"), status);
    fLF    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=LF}]"), status);
    fCM    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=CM}]"), status);
    fNL    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=NL}]"), status);
    fWJ    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=WJ}]"), status);
    fZW    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=ZW}]"), status);
    fGL    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=GL}]"), status);
    fCB    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=CB}]"), status);
    fSP    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=SP}]"), status);
    fB2    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=B2}]"), status);
    fBA    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=BA}]"), status);
    fBB    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=BB}]"), status);
    fHY    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=HY}]"), status);
    fH2    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=H2}]"), status);
    fH3    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=H3}]"), status);
    fCL    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=CL}]"), status);
    fCP    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=CP}]"), status);
    fEX    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=EX}]"), status);
    fIN    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=IN}]"), status);
    fJL    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=JL}]"), status);
    fJV    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=JV}]"), status);
    fJT    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=JT}]"), status);
    fNS    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=NS}]"), status);
    fOP    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=OP}]"), status);
    fQU    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=QU}]"), status);
    fIS    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=IS}]"), status);
    fNU    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=NU}]"), status);
    fPO    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=PO}]"), status);
    fPR    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=PR}]"), status);
    fSY    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=SY}]"), status);
    fAI    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=AI}]"), status);
    fAL    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=AL}]"), status);
    fCJ    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=CJ}]"), status);
    fHL    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=HL}]"), status);
    fID    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=ID}]"), status);
    fRI    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=RI}]"), status);
    fSA    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=SA}]"), status);
    fSG    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\ud800-\\udfff]"), status);
    fXX    = new UnicodeSet(UNICODE_STRING_SIMPLE("[\\p{Line_break=XX}]"), status);

    if (U_FAILURE(status)) {
        deferredStatus = status;
        fCharBI = NULL;
        fNumberMatcher = NULL;
        return;
    }

    fAL->addAll(*fXX);     // Default behavior for XX is identical to AL
    fAL->addAll(*fAI);     // Default behavior for AI is identical to AL
    fAL->addAll(*fSA);     // Default behavior for SA is XX, which defaults to AL
    fAL->addAll(*fSG);     // Default behavior for SG is identical to AL.

    fNS->addAll(*fCJ);     // Default behavior for CJ is identical to NS.

    fSets->addElement(fBK, status);
    fSets->addElement(fCR, status);
    fSets->addElement(fLF, status);
    fSets->addElement(fCM, status);
    fSets->addElement(fNL, status);
    fSets->addElement(fWJ, status);
    fSets->addElement(fZW, status);
    fSets->addElement(fGL, status);
    fSets->addElement(fCB, status);
    fSets->addElement(fSP, status);
    fSets->addElement(fB2, status);
    fSets->addElement(fBA, status);
    fSets->addElement(fBB, status);
    fSets->addElement(fHY, status);
    fSets->addElement(fH2, status);
    fSets->addElement(fH3, status);
    fSets->addElement(fCL, status);
    fSets->addElement(fCP, status);
    fSets->addElement(fEX, status);
    fSets->addElement(fIN, status);
    fSets->addElement(fJL, status);
    fSets->addElement(fJT, status);
    fSets->addElement(fJV, status);
    fSets->addElement(fNS, status);
    fSets->addElement(fOP, status);
    fSets->addElement(fQU, status);
    fSets->addElement(fIS, status);
    fSets->addElement(fNU, status);
    fSets->addElement(fPO, status);
    fSets->addElement(fPR, status);
    fSets->addElement(fSY, status);
    fSets->addElement(fAI, status);
    fSets->addElement(fAL, status);
    fSets->addElement(fHL, status);
    fSets->addElement(fID, status);
    fSets->addElement(fWJ, status);
    fSets->addElement(fRI, status);
    fSets->addElement(fSA, status);
    fSets->addElement(fSG, status);

    const char *rules = 
            "((\\p{Line_Break=PR}|\\p{Line_Break=PO})\\p{Line_Break=CM}*)?"
            "((\\p{Line_Break=OP}|\\p{Line_Break=HY})\\p{Line_Break=CM}*)?"
            "\\p{Line_Break=NU}\\p{Line_Break=CM}*"
            "((\\p{Line_Break=NU}|\\p{Line_Break=IS}|\\p{Line_Break=SY})\\p{Line_Break=CM}*)*"
            "((\\p{Line_Break=CL}|\\p{Line_Break=CP})\\p{Line_Break=CM}*)?"
            "((\\p{Line_Break=PR}|\\p{Line_Break=PO})\\p{Line_Break=CM}*)?";

    fNumberMatcher = new RegexMatcher(
        UnicodeString(rules, -1, US_INV), 0, status);

    fCharBI = BreakIterator::createCharacterInstance(Locale::getEnglish(), status);

    if (U_FAILURE(status)) {
        deferredStatus = status;
    }
}


void RBBILineMonkey::setText(const UnicodeString &s) {
    fText       = &s;
    fCharBI->setText(s);
    fNumberMatcher->reset(s);
}

//
//  rule9Adjust
//     Line Break TR rules 9 and 10 implementation.
//     This deals with combining marks and other sequences that
//     that must be treated as if they were something other than what they actually are.
//
//     This is factored out into a separate function because it must be applied twice for
//     each potential break, once to the chars before the position being checked, then
//     again to the text following the possible break.
//
void RBBILineMonkey::rule9Adjust(int32_t pos, UChar32 *posChar, int32_t *nextPos, UChar32 *nextChar) {
    if (pos == -1) {
        // Invalid initial position.  Happens during the warmup iteration of the
        //   main loop in next().
        return;
    }

    int32_t  nPos = *nextPos;

    // LB 9  Keep combining sequences together.
    //  advance over any CM class chars.  Note that Line Break CM is different
    //  from the normal Grapheme Extend property.
    if (!(fSP->contains(*posChar) || fBK->contains(*posChar) || *posChar==0x0d ||
          *posChar==0x0a ||fNL->contains(*posChar) || fZW->contains(*posChar))) {
        for (;;) {
            *nextChar = fText->char32At(nPos);
            if (!fCM->contains(*nextChar)) {
                break;
            }
            nPos = fText->moveIndex32(nPos, 1);
        }
    }


    // LB 9 Treat X CM* as if it were x.
    //       No explicit action required.

    // LB 10  Treat any remaining combining mark as AL
    if (fCM->contains(*posChar)) {
        *posChar = 0x41;   // thisChar = 'A';
    }

    // Push the updated nextPos and nextChar back to our caller.
    // This only makes a difference if posChar got bigger by consuming a
    // combining sequence.
    *nextPos  = nPos;
    *nextChar = fText->char32At(nPos);
}



int32_t RBBILineMonkey::next(int32_t startPos) {
    UErrorCode status = U_ZERO_ERROR;
    int32_t    pos;       //  Index of the char following a potential break position
    UChar32    thisChar;  //  Character at above position "pos"

    int32_t    prevPos;   //  Index of the char preceding a potential break position
    UChar32    prevChar;  //  Character at above position.  Note that prevChar
                          //   and thisChar may not be adjacent because combining
                          //   characters between them will be ignored.

    int32_t    prevPosX2; //  Second previous character.  Wider context for LB21a.
    UChar32    prevCharX2;

    int32_t    nextPos;   //  Index of the next character following pos.
                          //     Usually skips over combining marks.
    int32_t    nextCPPos; //  Index of the code point following "pos."
                          //     May point to a combining mark.
    int32_t    tPos;      //  temp value.
    UChar32    c;

    if (U_FAILURE(deferredStatus)) {
        return -1;
    }

    if (startPos >= fText->length()) {
        return -1;
    }


    // Initial values for loop.  Loop will run the first time without finding breaks,
    //                           while the invalid values shift out and the "this" and
    //                           "prev" positions are filled in with good values.
    pos      = prevPos   = prevPosX2  = -1;    // Invalid value, serves as flag for initial loop iteration.
    thisChar = prevChar  = prevCharX2 = 0;
    nextPos  = nextCPPos = startPos;


    // Loop runs once per position in the test text, until a break position
    //  is found.
    for (;;) {
        prevPosX2 = prevPos;
        prevCharX2 = prevChar;

        prevPos   = pos;
        prevChar  = thisChar;

        pos       = nextPos;
        thisChar  = fText->char32At(pos);

        nextCPPos = fText->moveIndex32(pos, 1);
        nextPos   = nextCPPos;

        // Rule LB2 - Break at end of text.
        if (pos >= fText->length()) {
            break;
        }

        // Rule LB 9 - adjust for combining sequences.
        //             We do this one out-of-order because the adjustment does not change anything
        //             that would match rules LB 3 - LB 6, but after the adjustment, LB 3-6 do need to
        //             be applied.
        rule9Adjust(prevPos, &prevChar, &pos,     &thisChar);
        nextCPPos = nextPos = fText->moveIndex32(pos, 1);
        c = fText->char32At(nextPos);
        rule9Adjust(pos,     &thisChar, &nextPos, &c);

        // If the loop is still warming up - if we haven't shifted the initial
        //   -1 positions out of prevPos yet - loop back to advance the
        //    position in the input without any further looking for breaks.
        if (prevPos == -1) {
            continue;
        }

        // LB 4  Always break after hard line breaks,
        if (fBK->contains(prevChar)) {
            break;
        }

        // LB 5  Break after CR, LF, NL, but not inside CR LF
        if (prevChar == 0x0d && thisChar == 0x0a) {
            continue;
        }
        if (prevChar == 0x0d ||
            prevChar == 0x0a ||
            prevChar == 0x85)  {
            break;
        }

        // LB 6  Don't break before hard line breaks
        if (thisChar == 0x0d || thisChar == 0x0a || thisChar == 0x85 ||
            fBK->contains(thisChar)) {
                continue;
        }


        // LB 7  Don't break before spaces or zero-width space.
        if (fSP->contains(thisChar)) {
            continue;
        }

        if (fZW->contains(thisChar)) {
            continue;
        }

        // LB 8  Break after zero width space
        if (fZW->contains(prevChar)) {
            break;
        }

        // LB 9, 10  Already done, at top of loop.
        //


        // LB 11  Do not break before or after WORD JOINER and related characters.
        //    x  WJ
        //    WJ  x
        //
        if (fWJ->contains(thisChar) || fWJ->contains(prevChar)) {
            continue;
        }

        // LB 12
        //    GL  x
        if (fGL->contains(prevChar)) {
            continue;
        }
        
        // LB 12a
        //    [^SP BA HY] x GL
        if (!(fSP->contains(prevChar) ||
              fBA->contains(prevChar) ||
              fHY->contains(prevChar)     ) && fGL->contains(thisChar)) {
            continue;
        }



        // LB 13  Don't break before closings.
        //        NU x CL,  NU x CP  and NU x IS are not matched here so that they will
        //        fall into LB 17 and the more general number regular expression.
        //
        if ((!fNU->contains(prevChar) && fCL->contains(thisChar)) ||
            (!fNU->contains(prevChar) && fCP->contains(thisChar)) ||
                                         fEX->contains(thisChar)  ||
            (!fNU->contains(prevChar) && fIS->contains(thisChar)) ||
            (!fNU->contains(prevChar) && fSY->contains(thisChar)))    {
            continue;
        }

        // LB 14 Don't break after OP SP*
        //       Scan backwards, checking for this sequence.
        //       The OP char could include combining marks, so we actually check for
        //           OP CM* SP*
        //       Another Twist: The Rule 67 fixes may have changed a SP CM
        //       sequence into a ID char, so before scanning back through spaces,
        //       verify that prevChar is indeed a space.  The prevChar variable
        //       may differ from fText[prevPos]
        tPos = prevPos;
        if (fSP->contains(prevChar)) {
            while (tPos > 0 && fSP->contains(fText->char32At(tPos))) {
                tPos=fText->moveIndex32(tPos, -1);
            }
        }
        while (tPos > 0 && fCM->contains(fText->char32At(tPos))) {
            tPos=fText->moveIndex32(tPos, -1);
        }
        if (fOP->contains(fText->char32At(tPos))) {
            continue;
        }


        // LB 15    QU SP* x OP
        if (fOP->contains(thisChar)) {
            // Scan backwards from prevChar to see if it is preceded by QU CM* SP*
            int tPos = prevPos;
            while (tPos>0 && fSP->contains(fText->char32At(tPos))) {
                tPos = fText->moveIndex32(tPos, -1);
            }
            while (tPos>0 && fCM->contains(fText->char32At(tPos))) {
                tPos = fText->moveIndex32(tPos, -1);
            }
            if (fQU->contains(fText->char32At(tPos))) {
                continue;
            }
        }



        // LB 16   (CL | CP) SP* x NS
        //    Scan backwards for SP* CM* (CL | CP)
        if (fNS->contains(thisChar)) {
            int tPos = prevPos;
            while (tPos>0 && fSP->contains(fText->char32At(tPos))) {
                tPos = fText->moveIndex32(tPos, -1);
            }
            while (tPos>0 && fCM->contains(fText->char32At(tPos))) {
                tPos = fText->moveIndex32(tPos, -1);
            }
            if (fCL->contains(fText->char32At(tPos)) || fCP->contains(fText->char32At(tPos))) {
                continue;
            }
        }


        // LB 17        B2 SP* x B2
        if (fB2->contains(thisChar)) {
            //  Scan backwards, checking for the B2 CM* SP* sequence.
            tPos = prevPos;
            if (fSP->contains(prevChar)) {
                while (tPos > 0 && fSP->contains(fText->char32At(tPos))) {
                    tPos=fText->moveIndex32(tPos, -1);
                }
            }
            while (tPos > 0 && fCM->contains(fText->char32At(tPos))) {
                tPos=fText->moveIndex32(tPos, -1);
            }
            if (fB2->contains(fText->char32At(tPos))) {
                continue;
            }
        }


        // LB 18    break after space
        if (fSP->contains(prevChar)) {
            break;
        }

        // LB 19
        //    x   QU
        //    QU  x
        if (fQU->contains(thisChar) || fQU->contains(prevChar)) {
            continue;
        }

        // LB 20  Break around a CB
        if (fCB->contains(thisChar) || fCB->contains(prevChar)) {
            break;
        }

        // LB 21
        if (fBA->contains(thisChar) ||
            fHY->contains(thisChar) ||
            fNS->contains(thisChar) ||
            fBB->contains(prevChar) )   {
            continue;
        }

        // LB 21a
        //   HL (HY | BA) x
        if (fHL->contains(prevCharX2) && 
                (fHY->contains(prevChar) || fBA->contains(prevChar))) {
            continue;
        }

        // LB 21b
        //   SY x HL
        if (fSY->contains(prevChar) && fHL->contains(thisChar)) {
            continue;
        }

        // LB 22
        if ((fAL->contains(prevChar) && fIN->contains(thisChar)) ||
            (fHL->contains(prevChar) && fIN->contains(thisChar)) ||
            (fID->contains(prevChar) && fIN->contains(thisChar)) ||
            (fIN->contains(prevChar) && fIN->contains(thisChar)) ||
            (fNU->contains(prevChar) && fIN->contains(thisChar)) )   {
            continue;
        }


        // LB 23    ID x PO
        //          AL x NU
        //          HL x NU
        //          NU x AL
        if ((fID->contains(prevChar) && fPO->contains(thisChar)) ||
            (fAL->contains(prevChar) && fNU->contains(thisChar)) ||
            (fHL->contains(prevChar) && fNU->contains(thisChar)) ||
            (fNU->contains(prevChar) && fAL->contains(thisChar)) ||
            (fNU->contains(prevChar) && fHL->contains(thisChar)) )   {
            continue;
        }

        // LB 24  Do not break between prefix and letters or ideographs.
        //        PR x ID
        //        PR x (AL | HL)
        //        PO x (AL | HL)
        if ((fPR->contains(prevChar) && fID->contains(thisChar)) ||
            (fPR->contains(prevChar) && (fAL->contains(thisChar) || fHL->contains(thisChar))) ||
            (fPO->contains(prevChar) && (fAL->contains(thisChar) || fHL->contains(thisChar))))  {
            continue;
        }



        // LB 25    Numbers
        if (fNumberMatcher->lookingAt(prevPos, status)) {
            if (U_FAILURE(status)) {
                break;
            }
            // Matched a number.  But could have been just a single digit, which would
            //    not represent a "no break here" between prevChar and thisChar
            int32_t numEndIdx = fNumberMatcher->end(status);  // idx of first char following num
            if (numEndIdx > pos) {
                // Number match includes at least our two chars being checked
                if (numEndIdx > nextPos) {
                    // Number match includes additional chars.  Update pos and nextPos
                    //   so that next loop iteration will continue at the end of the number,
                    //   checking for breaks between last char in number & whatever follows.
                    pos = nextPos = numEndIdx;
                    do {
                        pos = fText->moveIndex32(pos, -1);
                        thisChar = fText->char32At(pos);
                    } while (fCM->contains(thisChar));
                }
                continue;
            }
        }


        // LB 26 Do not break a Korean syllable.
        if (fJL->contains(prevChar) && (fJL->contains(thisChar) ||
                                        fJV->contains(thisChar) ||
                                        fH2->contains(thisChar) ||
                                        fH3->contains(thisChar))) {
                                            continue;
                                        }

        if ((fJV->contains(prevChar) || fH2->contains(prevChar))  &&
            (fJV->contains(thisChar) || fJT->contains(thisChar))) {
                continue;
        }

        if ((fJT->contains(prevChar) || fH3->contains(prevChar)) &&
            fJT->contains(thisChar)) {
                continue;
        }

        // LB 27 Treat a Korean Syllable Block the same as ID.
        if ((fJL->contains(prevChar) || fJV->contains(prevChar) ||
            fJT->contains(prevChar) || fH2->contains(prevChar) || fH3->contains(prevChar)) &&
            fIN->contains(thisChar)) {
                continue;
            }
        if ((fJL->contains(prevChar) || fJV->contains(prevChar) ||
            fJT->contains(prevChar) || fH2->contains(prevChar) || fH3->contains(prevChar)) &&
            fPO->contains(thisChar)) {
                continue;
            }
        if (fPR->contains(prevChar) && (fJL->contains(thisChar) || fJV->contains(thisChar) ||
            fJT->contains(thisChar) || fH2->contains(thisChar) || fH3->contains(thisChar))) {
                continue;
            }



        // LB 28  Do not break between alphabetics ("at").
        if ((fAL->contains(prevChar) || fHL->contains(prevChar)) && (fAL->contains(thisChar) || fHL->contains(thisChar))) {
            continue;
        }

        // LB 29  Do not break between numeric punctuation and alphabetics ("e.g.").
        if (fIS->contains(prevChar) && (fAL->contains(thisChar) || fHL->contains(thisChar))) {
            continue;
        }

        // LB 30    Do not break between letters, numbers, or ordinary symbols and opening or closing punctuation.
        //          (AL | NU) x OP
        //          CP x (AL | NU)
        if ((fAL->contains(prevChar) || fHL->contains(prevChar) || fNU->contains(prevChar)) && fOP->contains(thisChar)) {
            continue;
        }
        if (fCP->contains(prevChar) && (fAL->contains(thisChar) || fHL->contains(thisChar) || fNU->contains(thisChar))) {
            continue;
        }

        // LB30a  Do not break between regional indicators.
        //        RI x RI
        if (fRI->contains(prevChar) && fRI->contains(thisChar)) {
            continue;
        }

        // LB 31    Break everywhere else
        break;

    }

    return pos;
}


UVector  *RBBILineMonkey::charClasses() {
    return fSets;
}


RBBILineMonkey::~RBBILineMonkey() {
    delete fSets;

    delete fBK;
    delete fCR;
    delete fLF;
    delete fCM;
    delete fNL;
    delete fWJ;
    delete fZW;
    delete fGL;
    delete fCB;
    delete fSP;
    delete fB2;
    delete fBA;
    delete fBB;
    delete fHY;
    delete fH2;
    delete fH3;
    delete fCL;
    delete fCP;
    delete fEX;
    delete fIN;
    delete fJL;
    delete fJV;
    delete fJT;
    delete fNS;
    delete fOP;
    delete fQU;
    delete fIS;
    delete fNU;
    delete fPO;
    delete fPR;
    delete fSY;
    delete fAI;
    delete fAL;
    delete fCJ;
    delete fHL;
    delete fID;
    delete fRI;
    delete fSA;
    delete fSG;
    delete fXX;

    delete fCharBI;
    delete fNumberMatcher;
}


//-------------------------------------------------------------------------------------------
//
//   TestMonkey
//
//     params
//       seed=nnnnn        Random number starting seed.
//                         Setting the seed allows errors to be reproduced.
//       loop=nnn          Looping count.  Controls running time.
//                         -1:  run forever.
//                          0 or greater:  run length.
//
//       type = char | word | line | sent | title
//
//-------------------------------------------------------------------------------------------

static int32_t  getIntParam(UnicodeString name, UnicodeString &params, int32_t defaultVal) {
    int32_t val = defaultVal;
    name.append(" *= *(-?\\d+)");
    UErrorCode status = U_ZERO_ERROR;
    RegexMatcher m(name, params, 0, status);
    if (m.find()) {
        // The param exists.  Convert the string to an int.
        char valString[100];
        int32_t paramLength = m.end(1, status) - m.start(1, status);
        if (paramLength >= (int32_t)(sizeof(valString)-1)) {
            paramLength = (int32_t)(sizeof(valString)-2);
        }
        params.extract(m.start(1, status), paramLength, valString, sizeof(valString));
        val = strtol(valString,  NULL, 10);

        // Delete this parameter from the params string.
        m.reset();
        params = m.replaceFirst("", status);
    }
    U_ASSERT(U_SUCCESS(status));
    return val;
}
#endif

#if !UCONFIG_NO_REGULAR_EXPRESSIONS
static void testBreakBoundPreceding(RBBITest *test, UnicodeString ustr,
                                    BreakIterator *bi,
                                    int expected[],
                                    int expectedcount)
{
    int count = 0;
    int i = 0;
    int forward[50];
    bi->setText(ustr);
    for (i = bi->first(); i != BreakIterator::DONE; i = bi->next()) {
        forward[count] = i;
        if (count < expectedcount && expected[count] != i) {
            test->errln("break forward test failed: expected %d but got %d",
                        expected[count], i);
            break;
        }
        count ++;
    }
    if (count != expectedcount) {
        printStringBreaks(ustr, expected, expectedcount);
        test->errln("break forward test failed: missed %d match",
                    expectedcount - count);
        return;
    }
    // testing boundaries
    for (i = 1; i < expectedcount; i ++) {
        int j = expected[i - 1];
        if (!bi->isBoundary(j)) {
            printStringBreaks(ustr, expected, expectedcount);
            test->errln("isBoundary() failed.  Expected boundary at position %d", j);
            return;
        }
        for (j = expected[i - 1] + 1; j < expected[i]; j ++) {
            if (bi->isBoundary(j)) {
                printStringBreaks(ustr, expected, expectedcount);
                test->errln("isBoundary() failed.  Not expecting boundary at position %d", j);
                return;
            }
        }
    }

    for (i = bi->last(); i != BreakIterator::DONE; i = bi->previous()) {
        count --;
        if (forward[count] != i) {
            printStringBreaks(ustr, expected, expectedcount);
            test->errln("happy break test previous() failed: expected %d but got %d",
                        forward[count], i);
            break;
        }
    }
    if (count != 0) {
        printStringBreaks(ustr, expected, expectedcount);
        test->errln("break test previous() failed: missed a match");
        return;
    }

    // testing preceding
    for (i = 0; i < expectedcount - 1; i ++) {
        // int j = expected[i] + 1;
        int j = ustr.moveIndex32(expected[i], 1);
        for (; j <= expected[i + 1]; j ++) {
            if (bi->preceding(j) != expected[i]) {
                printStringBreaks(ustr, expected, expectedcount);
                test->errln("preceding(): Not expecting boundary at position %d", j);
                return;
            }
        }
    }
}
#endif

void RBBITest::TestWordBreaks(void)
{
#if !UCONFIG_NO_REGULAR_EXPRESSIONS

    Locale        locale("en");
    UErrorCode    status = U_ZERO_ERROR;
    // BreakIterator  *bi = BreakIterator::createCharacterInstance(locale, status);
    BreakIterator *bi = BreakIterator::createWordInstance(locale, status);
    // Replaced any C+J characters in a row with a random sequence of characters
    // of the same length to make our C+J segmentation not get in the way.
    static const char *strlist[] =
    {
    "\\U000e0032\\u0097\\u0f94\\uc2d8\\u05f4\\U000e0031\\u060d",
    "\\U000e0037\\u2666\\u1202\\u003a\\U000e0031\\u064d\\u0bea\\u091c\\U000e0040\\u003b",
    "\\u0589\\u3e99\\U0001d7f3\\U000e0074\\u1810\\u200e\\U000e004b\\u0027\\U000e0061\\u003a",
    "\\u398c\\U000104a5\\U0001d173\\u102d\\u002e\\uca3b\\u002e\\u002c\\u5622",
    "\\uac00\\u3588\\u009c\\u0953\\u194b",
    "\\u200e\\U000e0072\\u0a4b\\U000e003f\\ufd2b\\u2027\\u002e\\u002e",
    "\\u0602\\u2019\\ua191\\U000e0063\\u0a4c\\u003a\\ub4b5\\u003a\\u827f\\u002e",
    "\\u2f1f\\u1634\\u05f8\\u0944\\u04f2\\u0cdf\\u1f9c\\u05f4\\u002e",
    "\\U000e0042\\u002e\\u0fb8\\u09ef\\u0ed1\\u2044",
    "\\u003b\\u024a\\u102e\\U000e0071\\u0600",
    "\\u2027\\U000e0067\\u0a47\\u00b7",
    "\\u1fcd\\u002c\\u07aa\\u0027\\u11b0",
    "\\u002c\\U000e003c\\U0001d7f4\\u003a\\u0c6f\\u0027",
    "\\u0589\\U000e006e\\u0a42\\U000104a5",
    "\\u0f66\\u2523\\u003a\\u0cae\\U000e0047\\u003a",
    "\\u003a\\u0f21\\u0668\\u0dab\\u003a\\u0655\\u00b7",
    "\\u0027\\u11af\\U000e0057\\u0602",
    "\\U0001d7f2\\U000e007\\u0004\\u0589",
    "\\U000e0022\\u003a\\u10b3\\u003a\\ua21b\\u002e\\U000e0058\\u1732\\U000e002b",
    "\\U0001d7f2\\U000e007d\\u0004\\u0589",
    "\\u82ab\\u17e8\\u0736\\u2019\\U0001d64d",
    "\\ub55c\\u0a68\\U000e0037\\u0cd6\\u002c\\ub959",
    "\\U000e0065\\u302c\\uc986\\u09ee\\U000e0068",
    "\\u0be8\\u002e\\u0c68\\u066e\\u136d\\ufc99\\u59e7",
    "\\u0233\\U000e0020\\u0a69\\u0d6a",
    "\\u206f\\u0741\\ub3ab\\u2019\\ubcac\\u2019",
    "\\u18f4\\U000e0049\\u20e7\\u2027",
    "\\ub315\\U0001d7e5\\U000e0073\\u0c47\\u06f2\\u0c6a\\u0037\\u10fe",
    "\\ua183\\u102d\\u0bec\\u003a",
    "\\u17e8\\u06e7\\u002e\\u096d\\u003b",
    "\\u003a\\u0e57\\u0fad\\u002e",
    "\\u002e\\U000e004c\\U0001d7ea\\u05bb\\ud0fd\\u02de",
    "\\u32e6\\U0001d7f6\\u0fa1\\u206a\\U000e003c\\u0cec\\u003a",
    "\\U000e005d\\u2044\\u0731\\u0650\\u0061",
    "\\u003a\\u0664\\u00b7\\u1fba",
    "\\u003b\\u0027\\u00b7\\u47a3",
    "\\u2027\\U000e0067\\u0a42\\u00b7\\u4edf\\uc26c\\u003a\\u4186\\u041b",
    "\\u0027\\u003a\\U0001d70f\\U0001d7df\\ubf4a\\U0001d7f5\\U0001d177\\u003a\\u0e51\\u1058\\U000e0058\\u00b7\\u0673",
    "\\uc30d\\u002e\\U000e002c\\u0c48\\u003a\\ub5a1\\u0661\\u002c",
    };
    int loop;
    if (U_FAILURE(status)) {
        errcheckln(status, "Creation of break iterator failed %s", u_errorName(status));
        return;
    }
    for (loop = 0; loop < (int)(sizeof(strlist) / sizeof(char *)); loop ++) {
        // printf("looping %d\n", loop);
        UnicodeString ustr = CharsToUnicodeString(strlist[loop]);
        // RBBICharMonkey monkey;
        RBBIWordMonkey monkey;

        int expected[50];
        int expectedcount = 0;

        monkey.setText(ustr);
        int i;
        for (i = 0; i != BreakIterator::DONE; i = monkey.next(i)) {
            expected[expectedcount ++] = i;
        }

        testBreakBoundPreceding(this, ustr, bi, expected, expectedcount);
    }
    delete bi;
#endif
}

void RBBITest::TestWordBoundary(void)
{
    // <data><>\u1d4a\u206e<?>\u0603\U0001d7ff<>\u2019<></data>
    Locale        locale("en");
    UErrorCode    status = U_ZERO_ERROR;
    // BreakIterator  *bi = BreakIterator::createCharacterInstance(locale, status);
    BreakIterator *bi = BreakIterator::createWordInstance(locale, status);
    UChar         str[50];
    static const char *strlist[] =
    {
    "\\u200e\\U000e0072\\u0a4b\\U000e003f\\ufd2b\\u2027\\u002e\\u002e",
    "\\U000e0042\\u002e\\u0fb8\\u09ef\\u0ed1\\u2044",
    "\\u003b\\u024a\\u102e\\U000e0071\\u0600",
    "\\u2027\\U000e0067\\u0a47\\u00b7",
    "\\u1fcd\\u002c\\u07aa\\u0027\\u11b0",
    "\\u002c\\U000e003c\\U0001d7f4\\u003a\\u0c6f\\u0027",
    "\\u0589\\U000e006e\\u0a42\\U000104a5",
    "\\u4f66\\ub523\\u003a\\uacae\\U000e0047\\u003a",
    "\\u003a\\u0f21\\u0668\\u0dab\\u003a\\u0655\\u00b7",
    "\\u0027\\u11af\\U000e0057\\u0602",
    "\\U0001d7f2\\U000e007\\u0004\\u0589",
    "\\U000e0022\\u003a\\u10b3\\u003a\\ua21b\\u002e\\U000e0058\\u1732\\U000e002b",
    "\\U0001d7f2\\U000e007d\\u0004\\u0589",
    "\\u82ab\\u17e8\\u0736\\u2019\\U0001d64d",
    "\\u0e01\\ub55c\\u0a68\\U000e0037\\u0cd6\\u002c\\ub959",
    "\\U000e0065\\u302c\\u09ee\\U000e0068",
    "\\u0be8\\u002e\\u0c68\\u066e\\u136d\\ufc99\\u59e7",
    "\\u0233\\U000e0020\\u0a69\\u0d6a",
    "\\u206f\\u0741\\ub3ab\\u2019\\ubcac\\u2019",
    "\\u58f4\\U000e0049\\u20e7\\u2027",
    "\\U0001d7e5\\U000e0073\\u0c47\\u06f2\\u0c6a\\u0037\\u10fe",
    "\\ua183\\u102d\\u0bec\\u003a",
    "\\u17e8\\u06e7\\u002e\\u096d\\u003b",
    "\\u003a\\u0e57\\u0fad\\u002e",
    "\\u002e\\U000e004c\\U0001d7ea\\u05bb\\ud0fd\\u02de",
    "\\u32e6\\U0001d7f6\\u0fa1\\u206a\\U000e003c\\u0cec\\u003a",
    "\\ua2a5\\u0038\\u2044\\u002e\\u0c67\\U000e003c\\u05f4\\u2027\\u05f4\\u2019",
    "\\u003a\\u0664\\u00b7\\u1fba",
    "\\u003b\\u0027\\u00b7\\u47a3",
    };
    int loop;
    if (U_FAILURE(status)) {
        errcheckln(status, "Creation of break iterator failed %s", u_errorName(status));
        return;
    }
    for (loop = 0; loop < (int)(sizeof(strlist) / sizeof(char *)); loop ++) {
        // printf("looping %d\n", loop);
        u_unescape(strlist[loop], str, 20);
        UnicodeString ustr(str);
        int forward[50];
        int count = 0;

        bi->setText(ustr);
        int prev = 0;
        int i;
        for (i = bi->first(); i != BreakIterator::DONE; i = bi->next()) {
            forward[count ++] = i;
            if (i > prev) {
                int j;
                for (j = prev + 1; j < i; j ++) {
                    if (bi->isBoundary(j)) {
                        printStringBreaks(ustr, forward, count);
                        errln("happy boundary test failed: expected %d not a boundary",
                               j);
                        return;
                    }
                }
            }
            if (!bi->isBoundary(i)) {
                printStringBreaks(ustr, forward, count);
                errln("happy boundary test failed: expected %d a boundary",
                       i);
                return;
            }
            prev = i;
        }
    }
    delete bi;
}

void RBBITest::TestLineBreaks(void)
{
#if !UCONFIG_NO_REGULAR_EXPRESSIONS
    Locale        locale("en");
    UErrorCode    status = U_ZERO_ERROR;
    BreakIterator *bi = BreakIterator::createLineInstance(locale, status);
    const int32_t  STRSIZE = 50;
    UChar         str[STRSIZE];
    static const char *strlist[] =
    {
     "\\u300f\\ufdfc\\ub798\\u2011\\u2011\\u0020\\u0b43\\u002d\\ubeec\\ufffc",
     "\\u24ba\\u2060\\u3405\\ub290\\u000d\\U000e0032\\ufe35\\u00a0\\u0361\\"
             "U000112ed\\u0f0c\\u000a\\u308e\\ua875\\u0085\\u114d",
     "\\ufffc\\u3063\\u2e08\\u30e3\\u000d\\u002d\\u0ed8\\u002f\\U00011a57\\"
             "u2014\\U000e0105\\u118c\\u000a\\u07f8",
     "\\u0668\\u192b\\u002f\\u2034\\ufe39\\u00b4\\u0cc8\\u2571\\u200b\\u003f",
     "\\ufeff\\ufffc\\u3289\\u0085\\u2772\\u0020\\U000e010a\\u0020\\u2025\\u000a\\U000e0123",
     "\\ufe3c\\u201c\\u000d\\u2025\\u2007\\u201c\\u002d\\u20a0\\u002d\\u30a7\\u17a4",
     "\\u2772\\u0020\\U000e010a\\u0020\\u2025\\u000a\\U000e0123",
     "\\u002d\\uff1b\\u02c8\\u2029\\ufeff\\u0f22\\u2044\\ufe09\\u003a\\u096d\\u2009\\u000a\\u06f7\\u02cc\\u1019\\u2060",
     "\\u1781\\u0b68\\u0f0c\\u3010\\u0085\\U00011f7a\\u0020\\u0dd6\\u200b\\U000e007a\\u000a\\u2060\\u2026\\u002f\\u2026\\u24dc\\u101e\\u2014\\u2007\\u30a5",
     "\\u2770\\u0020\\U000e010f\\u0020\\u2060\\u000a\\u02cc\\u0bcc\\u060d\\u30e7\\u0f3b\\u002f",
     "\\ufeff\\u0028\\u003b\\U00012fec\\u2010\\u0020\\u0004\\u200b\\u0020\\u275c\\u002f\\u17b1",
     "\\u20a9\\u2014\\u00a2\\u31f1\\u002f\\u0020\\u05b8\\u200b\\u0cc2\\u003b\\u060d\\u02c8\\ua4e8\\u002f\\u17d5",
     "\\u002d\\u136f\\uff63\\u0084\\ua933\\u2028\\u002d\\u431b\\u200b\\u20b0",
     "\\uade3\\u11d6\\u000a\\U0001107d\\u203a\\u201d\\ub070\\u000d\\u2024\\ufffc",
     "\\uff5b\\u101c\\u1806\\u002f\\u2213\\uff5f",
     "\\u2014\\u0a83\\ufdfc\\u003f\\u00a0\\u0020\\u000a\\u2991\\U0001d179\\u0020\\u201d\\U000125f6\\u0a67\\u20a7\\ufeff\\u043f",
     "\\u169b\\U000e0130\\u002d\\u1041\\u0f3d\\u0abf\\u00b0\\u31fb\\u00a0\\u002d\\u02c8\\u003b",
     "\\u2762\\u1680\\u002d\\u2028\\u0027\\u01dc\\ufe56\\u003a\\u000a\\uffe6\\u29fd\\u0020\\u30ee\\u007c\\U0001d178\\u0af1\\u0085",
     "\\u3010\\u200b\\u2029\\ufeff\\ufe6a\\u275b\\U000e013b\\ufe37\\u24d4\\u002d\\u1806\\u256a\\u1806\\u247c\\u0085\\u17ac",
     "\\u99ab\\u0027\\u003b\\u2026\\ueaf0\\u0020\\u0020\\u0313\\u0020\\u3099\\uff09\\u208e\\u2011\\u2007\\u2060\\u000a\\u0020\\u0020\\u300b\\u0bf9",
     "\\u1806\\u060d\\u30f5\\u00b4\\u17e9\\u2544\\u2028\\u2024\\u2011\\u20a3\\u002d\\u09cc\\u1782\\u000d\\uff6f\\u0025",
     "\\u002f\\uf22e\\u1944\\ufe3d\\u0020\\u206f\\u31b3\\u2014\\u002d\\u2025\\u0f0c\\u0085\\u2763",
     "\\u002f\\u2563\\u202f\\u0085\\u17d5\\u200b\\u0020\\U000e0043\\u2014\\u058a\\u3d0a\\ufe57\\u2035\\u2028\\u2029",
     "\\u20ae\\U0001d169\\u9293\\uff1f\\uff1f\\u0021\\u2012\\u2039\\u0085\\u02cc\\u00a2\\u0020\\U000e01ab\\u3085\\u0f3a\\u1806\\u0f0c\\u1945\\u000a\\U0001d7e7",
     "\\uffe6\\u00a0\\u200b\\u0085\\u2116\\u255b\\U0001d7f7\\u178c\\ufffc",
     "\\u02cc\\ufe6a\\u00a0\\u0021\\u002d\\u7490\\uec2e\\u200b\\u000a",
     "\\uec2e\\u200b\\u000a\\u0020\\u2028\\u2014\\u8945",
     "\\u7490\\uec2e\\u200b\\u000a\\u0020\\u2028\\u2014",
     "\\u0020\\u2028\\u2014\\u8945\\u002c\\u005b",
     "\\u000a\\ufe3c\\u201c\\u000d\\u2025\\u2007\\u201c\\u002d\\u20a0",
     "\\u2473\\u0e9d\\u0020\\u0085\\u000a\\ufe3c\\u201c\\u000d\\u2025",
     "\\U0001d16e\\ufffc\\u2025\\u0021\\u002d",
     "\\ufffc\\u301b\\u0fa5\\U000e0103\\u2060\\u208e\\u17d5\\u034f\\u1009\\u003a\\u180e\\u2009\\u3111",
     "\\u2014\\u0020\\u000a\\u17c5\\u24fc",
     "\\ufffc\\u0020\\u2116\\uff6c\\u200b\\u0ac3\\U0001028f",
     "\\uaeb0\\u0344\\u0085\\ufffc\\u073b\\u2010",
     "\\ufeff\\u0589\\u0085\\u0eb8\\u30fd\\u002f\\u003a\\u2014\\ufe43",
     "\\u09cc\\u256a\\u276d\\u002d\\u3085\\u000d\\u0e05\\u2028\\u0fbb",
     "\\u2034\\u00bb\\u0ae6\\u300c\\u0020\\u31f8\\ufffc",
     "\\u2116\\u0ed2\\uff64\\u02cd\\u2001\\u2060",
         "\\u809d\\u2e02\\u0f0a\\uc48f\\u2540\\u000d\\u0cef\\u003a\\u0e4d"
         "\\U000e0172\\U000e005c\\u17cf\\U00010ca6\\ufeff\\uf621\\u06f3\\uffe5"
         "\\u0ea2\\ufeff\\udcea\\u3085\\ua874\\u000a\\u0020\\u000b\\u200b",
     "\\ufe10\\u2060\\u1a5a\\u2060\\u17e4\\ufffc\\ubbe1\\ufe15\\u0020\\u00a0",
         "\\u2060\\u2213\\u200b\\u2019\\uc2dc\\uff6a\\u1736\\u0085\\udb07",
    };
    int loop;
    TEST_ASSERT_SUCCESS(status);
    if (U_FAILURE(status)) {
        return;
    }
    for (loop = 0; loop < (int)(sizeof(strlist) / sizeof(char *)); loop ++) {
        // printf("looping %d\n", loop);
        int32_t t = u_unescape(strlist[loop], str, STRSIZE);
        if (t >= STRSIZE) {
            TEST_ASSERT(FALSE);
            continue;
        }


        UnicodeString ustr(str);
        RBBILineMonkey monkey;
        if (U_FAILURE(monkey.deferredStatus)) {
            continue;
        }

        const int EXPECTEDSIZE = 50;
        int expected[EXPECTEDSIZE];
        int expectedcount = 0;

        monkey.setText(ustr);
        int i;
        for (i = 0; i != BreakIterator::DONE; i = monkey.next(i)) {
            if (expectedcount >= EXPECTEDSIZE) {
                TEST_ASSERT(expectedcount < EXPECTEDSIZE);
                return;
            }
            expected[expectedcount ++] = i;
        }

        testBreakBoundPreceding(this, ustr, bi, expected, expectedcount);
    }
    delete bi;
#endif
}

void RBBITest::TestSentBreaks(void)
{
#if !UCONFIG_NO_REGULAR_EXPRESSIONS
    Locale        locale("en");
    UErrorCode    status = U_ZERO_ERROR;
    BreakIterator *bi = BreakIterator::createSentenceInstance(locale, status);
    UChar         str[200];
    static const char *strlist[] =
    {
     "Now\ris\nthe\r\ntime\n\rfor\r\r",
     "This\n",
     "Hello! how are you? I'am fine. Thankyou. How are you doing? This\n costs $20,00,000.",
     "\"Sentence ending with a quote.\" Bye.",
     "  (This is it).  Testing the sentence iterator. \"This isn't it.\"",
     "Hi! This is a simple sample sentence. (This is it.) This is a simple sample sentence. \"This isn't it.\"",
     "Hi! This is a simple sample sentence. It does not have to make any sense as you can see. ",
     "Nel mezzo del cammin di nostra vita, mi ritrovai in una selva oscura. ",
     "Che la dritta via aveo smarrita. He said, that I said, that you said!! ",
     "Don't rock the boat.\\u2029Because I am the daddy, that is why. Not on my time (el timo.)!",
     "\\U0001040a\\u203a\\u1217\\u2b23\\u000d\\uff3b\\u03dd\\uff57\\u0a69\\u104a\\ufe56\\ufe52"
             "\\u3016\\U000e002f\\U000e0077\\u0662\\u1680\\u2984\\U000e006a\\u002e\\ua6ab\\u104a"
             "\\u002e\\u019b\\u2005\\u002e\\u0477\\u0438\\u0085\\u0441\\u002e\\u5f61\\u202f"
             "\\U0001019f\\uff08\\u27e8\\u055c\\u0352",
     "\\u1f3e\\u004d\\u000a\\ua3e4\\U000e0023\\uff63\\u0c52\\u276d\\U0001d5de\\U0001d171"
             "\\u0e38\\u17e5\\U00012fe6\\u0fa9\\u267f\\u1da3\\u0046\\u03ed\\udc72\\u0030"
             "\\U0001d688\\u0b6d\\u0085\\u0c67\\u1f94\\u0c6c\\u9cb2\\u202a\\u180e\\u000b"
             "\\u002e\\U000e005e\\u035b\\u061f\\u02c1\\U000e0025\\u0357\\u0969\\u202b"
             "\\U000130c5\\u0486\\U000e0123\\u2019\\u01bc\\u2006\\u11ad\\u180e\\u2e05"
             "\\u10b7\\u013e\\u000a\\u002e\\U00013ea4"
    };
    int loop;
    if (U_FAILURE(status)) {
        errcheckln(status, "Creation of break iterator failed %s", u_errorName(status));
        return;
    }
    for (loop = 0; loop < (int)(sizeof(strlist) / sizeof(char *)); loop ++) {
        u_unescape(strlist[loop], str, (int32_t)(sizeof(str) / sizeof(str[0])));
        UnicodeString ustr(str);

        RBBISentMonkey monkey;
        if (U_FAILURE(monkey.deferredStatus)) {
            continue;
        }

        const int EXPECTEDSIZE = 50;
        int expected[EXPECTEDSIZE];
        int expectedcount = 0;

        monkey.setText(ustr);
        int i;
        for (i = 0; i != BreakIterator::DONE; i = monkey.next(i)) {
            if (expectedcount >= EXPECTEDSIZE) {
                TEST_ASSERT(expectedcount < EXPECTEDSIZE);
                return;
            }
            expected[expectedcount ++] = i;
        }

        testBreakBoundPreceding(this, ustr, bi, expected, expectedcount);
    }
    delete bi;
#endif
}

void RBBITest::TestMonkey(char *params) {
#if !UCONFIG_NO_REGULAR_EXPRESSIONS

    UErrorCode     status    = U_ZERO_ERROR;
    int32_t        loopCount = 500;
    int32_t        seed      = 1;
    UnicodeString  breakType = "all";
    Locale         locale("en");
    UBool          useUText  = FALSE;

    if (quick == FALSE) {
        loopCount = 10000;
    }

    if (params) {
        UnicodeString p(params);
        loopCount = getIntParam("loop", p, loopCount);
        seed      = getIntParam("seed", p, seed);

        RegexMatcher m(" *type *= *(char|word|line|sent|title) *", p, 0, status);
        if (m.find()) {
            breakType = m.group(1, status);
            m.reset();
            p = m.replaceFirst("", status);
        }

        RegexMatcher u(" *utext", p, 0, status);
        if (u.find()) {
            useUText = TRUE;
            u.reset();
            p = u.replaceFirst("", status);
        }


        // m.reset(p);
        if (RegexMatcher(UNICODE_STRING_SIMPLE("\\S"), p, 0, status).find()) {
            // Each option is stripped out of the option string as it is processed.
            // All options have been checked.  The option string should have been completely emptied..
            char buf[100];
            p.extract(buf, sizeof(buf), NULL, status);
            buf[sizeof(buf)-1] = 0;
            errln("Unrecognized or extra parameter:  %s\n", buf);
            return;
        }

    }

    if (breakType == "char" || breakType == "all") {
        RBBICharMonkey  m;
        BreakIterator  *bi = BreakIterator::createCharacterInstance(locale, status);
        if (U_SUCCESS(status)) {
            RunMonkey(bi, m, "char", seed, loopCount, useUText);
            if (breakType == "all" && useUText==FALSE) {
                // Also run a quick test with UText when "all" is specified
                RunMonkey(bi, m, "char", seed, loopCount, TRUE);
            }
        }
        else {
            errcheckln(status, "Creation of character break iterator failed %s", u_errorName(status));
        }
        delete bi;
    }

    if (breakType == "word" || breakType == "all") {
        logln("Word Break Monkey Test");
        RBBIWordMonkey  m;
        BreakIterator  *bi = BreakIterator::createWordInstance(locale, status);
        if (U_SUCCESS(status)) {
            RunMonkey(bi, m, "word", seed, loopCount, useUText);
        }
        else {
            errcheckln(status, "Creation of word break iterator failed %s", u_errorName(status));
        }
        delete bi;
    }

    if (breakType == "line" || breakType == "all") {
        logln("Line Break Monkey Test");
        RBBILineMonkey  m;
        BreakIterator  *bi = BreakIterator::createLineInstance(locale, status);
        if (loopCount >= 10) {
            loopCount = loopCount / 5;   // Line break runs slower than the others.
        }
        if (U_SUCCESS(status)) {
            RunMonkey(bi, m, "line", seed, loopCount, useUText);
        }
        else {
            errcheckln(status, "Creation of line break iterator failed %s", u_errorName(status));
        }
        delete bi;
    }

    if (breakType == "sent" || breakType == "all"  ) {
        logln("Sentence Break Monkey Test");
        RBBISentMonkey  m;
        BreakIterator  *bi = BreakIterator::createSentenceInstance(locale, status);
        if (loopCount >= 10) {
            loopCount = loopCount / 10;   // Sentence runs slower than the other break types
        }
        if (U_SUCCESS(status)) {
            RunMonkey(bi, m, "sentence", seed, loopCount, useUText);
        }
        else {
            errcheckln(status, "Creation of line break iterator failed %s", u_errorName(status));
        }
        delete bi;
    }

#endif
}

//
//  Run a RBBI monkey test.  Common routine, for all break iterator types.
//    Parameters:
//       bi      - the break iterator to use
//       mk      - MonkeyKind, abstraction for obtaining expected results
//       name    - Name of test (char, word, etc.) for use in error messages
//       seed    - Seed for starting random number generator (parameter from user)
//       numIterations
//
void RBBITest::RunMonkey(BreakIterator *bi, RBBIMonkeyKind &mk, const char *name, uint32_t  seed,
                         int32_t numIterations, UBool useUText) {

#if !UCONFIG_NO_REGULAR_EXPRESSIONS

    const int32_t    TESTSTRINGLEN = 500;
    UnicodeString    testText;
    int32_t          numCharClasses;
    UVector          *chClasses;
    int              expected[TESTSTRINGLEN*2 + 1];
    int              expectedCount = 0;
    char             expectedBreaks[TESTSTRINGLEN*2 + 1];
    char             forwardBreaks[TESTSTRINGLEN*2 + 1];
    char             reverseBreaks[TESTSTRINGLEN*2+1];
    char             isBoundaryBreaks[TESTSTRINGLEN*2+1];
    char             followingBreaks[TESTSTRINGLEN*2+1];
    char             precedingBreaks[TESTSTRINGLEN*2+1];
    int              i;
    int              loopCount = 0;

    m_seed = seed;

    numCharClasses = mk.charClasses()->size();
    chClasses      = mk.charClasses();

    // Check for errors that occured during the construction of the MonkeyKind object.
    //  Can't report them where they occured because errln() is a method coming from intlTest,
    //  and is not visible outside of RBBITest :-(
    if (U_FAILURE(mk.deferredStatus)) {
        errln("status of \"%s\" in creation of RBBIMonkeyKind.", u_errorName(mk.deferredStatus));
        return;
    }

    // Verify that the character classes all have at least one member.
    for (i=0; i<numCharClasses; i++) {
        UnicodeSet *s = (UnicodeSet *)chClasses->elementAt(i);
        if (s == NULL || s->size() == 0) {
            errln("Character Class #%d is null or of zero size.", i);
            return;
        }
    }

    while (loopCount < numIterations || numIterations == -1) {
        if (numIterations == -1 && loopCount % 10 == 0) {
            // If test is running in an infinite loop, display a periodic tic so
            //   we can tell that it is making progress.
            fprintf(stderr, ".");
        }
        // Save current random number seed, so that we can recreate the random numbers
        //   for this loop iteration in event of an error.
        seed = m_seed;

        // Populate a test string with data.
        testText.truncate(0);
        for (i=0; i<TESTSTRINGLEN; i++) {
            int32_t  aClassNum = m_rand() % numCharClasses;
            UnicodeSet *classSet = (UnicodeSet *)chClasses->elementAt(aClassNum);
            int32_t   charIdx = m_rand() % classSet->size();
            UChar32   c = classSet->charAt(charIdx);
            if (c < 0) {   // TODO:  deal with sets containing strings.
                errln("c < 0");
                break;
            }
            testText.append(c);
        }

        // Calculate the expected results for this test string.
        mk.setText(testText);
        memset(expectedBreaks, 0, sizeof(expectedBreaks));
        expectedBreaks[0] = 1;
        int32_t breakPos = 0;
        expectedCount = 0;
        for (;;) {
            breakPos = mk.next(breakPos);
            if (breakPos == -1) {
                break;
            }
            if (breakPos > testText.length()) {
                errln("breakPos > testText.length()");
            }
            expectedBreaks[breakPos] = 1;
            U_ASSERT(expectedCount<testText.length());
            expected[expectedCount ++] = breakPos;
            (void)expected;   // Set but not used warning.
                              // TODO (andy): check it out.
        }

        // Find the break positions using forward iteration
        memset(forwardBreaks, 0, sizeof(forwardBreaks));
        if (useUText) {
            UErrorCode status = U_ZERO_ERROR;
            UText *testUText = utext_openReplaceable(NULL, &testText, &status);
            // testUText = utext_openUnicodeString(testUText, &testText, &status);
            bi->setText(testUText, status);
            TEST_ASSERT_SUCCESS(status);
            utext_close(testUText);   // The break iterator does a shallow clone of the UText
                                      //  This UText can be closed immediately, so long as the
                                      //  testText string continues to exist.
        } else {
            bi->setText(testText);
        }

        for (i=bi->first(); i != BreakIterator::DONE; i=bi->next()) {
            if (i < 0 || i > testText.length()) {
                errln("%s break monkey test: Out of range value returned by breakIterator::next()", name);
                break;
            }
            forwardBreaks[i] = 1;
        }

        // Find the break positions using reverse iteration
        memset(reverseBreaks, 0, sizeof(reverseBreaks));
        for (i=bi->last(); i != BreakIterator::DONE; i=bi->previous()) {
            if (i < 0 || i > testText.length()) {
                errln("%s break monkey test: Out of range value returned by breakIterator::next()", name);
                break;
            }
            reverseBreaks[i] = 1;
        }

        // Find the break positions using isBoundary() tests.
        memset(isBoundaryBreaks, 0, sizeof(isBoundaryBreaks));
        U_ASSERT((int32_t)sizeof(isBoundaryBreaks) > testText.length());
        for (i=0; i<=testText.length(); i++) {
            isBoundaryBreaks[i] = bi->isBoundary(i);
        }


        // Find the break positions using the following() function.
        // printf(".");
        memset(followingBreaks, 0, sizeof(followingBreaks));
        int32_t   lastBreakPos = 0;
        followingBreaks[0] = 1;
        for (i=0; i<testText.length(); i++) {
            breakPos = bi->following(i);
            if (breakPos <= i ||
                breakPos < lastBreakPos ||
                breakPos > testText.length() ||
                (breakPos > lastBreakPos && lastBreakPos > i)) {
                errln("%s break monkey test: "
                    "Out of range value returned by BreakIterator::following().\n"
                        "Random seed=%d  index=%d; following returned %d;  lastbreak=%d",
                         name, seed, i, breakPos, lastBreakPos);
                break;
            }
            followingBreaks[breakPos] = 1;
            lastBreakPos = breakPos;
        }

        // Find the break positions using the preceding() function.
        memset(precedingBreaks, 0, sizeof(precedingBreaks));
        lastBreakPos = testText.length();
        precedingBreaks[testText.length()] = 1;
        for (i=testText.length(); i>0; i--) {
            breakPos = bi->preceding(i);
            if (breakPos >= i ||
                breakPos > lastBreakPos ||
                (breakPos < 0 && testText.getChar32Start(i)>0) ||
                (breakPos < lastBreakPos && lastBreakPos < testText.getChar32Start(i)) ) {
                errln("%s break monkey test: "
                    "Out of range value returned by BreakIterator::preceding().\n"
                    "index=%d;  prev returned %d; lastBreak=%d" ,
                    name,  i, breakPos, lastBreakPos);
                if (breakPos >= 0 && breakPos < (int32_t)sizeof(precedingBreaks)) {
                    precedingBreaks[i] = 2;   // Forces an error.
                }
            } else {
                if (breakPos >= 0) {
                    precedingBreaks[breakPos] = 1;
                } 
                lastBreakPos = breakPos;
            }
        }

        // Compare the expected and actual results.
        for (i=0; i<=testText.length(); i++) {
            const char *errorType = NULL;
            if  (forwardBreaks[i] != expectedBreaks[i]) {
                errorType = "next()";
            } else if (reverseBreaks[i] != forwardBreaks[i]) {
                errorType = "previous()";
            } else if (isBoundaryBreaks[i] != expectedBreaks[i]) {
                errorType = "isBoundary()";
            } else if (followingBreaks[i] != expectedBreaks[i]) {
                errorType = "following()";
            } else if (precedingBreaks[i] != expectedBreaks[i]) {
                errorType = "preceding()";
            }


            if (errorType != NULL) {
                // Format a range of the test text that includes the failure as
                //  a data item that can be included in the rbbi test data file.

                // Start of the range is the last point where expected and actual results
                //   both agreed that there was a break position.
                int startContext = i;
                int32_t count = 0;
                for (;;) {
                    if (startContext==0) { break; }
                    startContext --;
                    if (expectedBreaks[startContext] != 0) {
                        if (count == 2) break;
                        count ++;
                    }
                }

                // End of range is two expected breaks past the start position.
                int endContext = i + 1;
                int ci;
                for (ci=0; ci<2; ci++) {  // Number of items to include in error text.
                    for (;;) {
                        if (endContext >= testText.length()) {break;}
                        if (expectedBreaks[endContext-1] != 0) {
                            if (count == 0) break;
                            count --;
                        }
                        endContext ++;
                    }
                }

                // Format looks like   "<data>\\\uabcd\uabcd\\\U0001abcd...</data>"
                UnicodeString errorText = "<data>";
                /***if (strcmp(errorType, "next()") == 0) {
                    startContext = 0;
                    endContext = testText.length();

                    printStringBreaks(testText, expected, expectedCount);
                }***/

                for (ci=startContext; ci<endContext;) {
                    UnicodeString hexChars("0123456789abcdef");
                    UChar32  c;
                    int      bn;
                    c = testText.char32At(ci);
                    if (ci == i) {
                        // This is the location of the error.
                        errorText.append("<?>");
                    } else if (expectedBreaks[ci] != 0) {
                        // This a non-error expected break position.
                        errorText.append("\\");
                    }
                    if (c < 0x10000) {
                        errorText.append("\\u");
                        for (bn=12; bn>=0; bn-=4) {
                            errorText.append(hexChars.charAt((c>>bn)&0xf));
                        }
                    } else {
                        errorText.append("\\U");
                        for (bn=28; bn>=0; bn-=4) {
                            errorText.append(hexChars.charAt((c>>bn)&0xf));
                        }
                    }
                    ci = testText.moveIndex32(ci, 1);
                }
                errorText.append("\\");
                errorText.append("</data>\n");

                // Output the error
                char  charErrorTxt[500];
                UErrorCode status = U_ZERO_ERROR;
                errorText.extract(charErrorTxt, sizeof(charErrorTxt), NULL, status);
                charErrorTxt[sizeof(charErrorTxt)-1] = 0;
                const char *badLocale = bi->getLocaleID(ULOC_ACTUAL_LOCALE, status);
                
                errln("%s break monkey test error [%s].  %s. Operation = %s; Random seed = %d;  buf Idx = %d\n%s",
                    name, badLocale, (expectedBreaks[i]? "break expected but not found" : "break found but not expected"),
                    errorType, seed, i, charErrorTxt);
                break;
            }
        }

        loopCount++;
    }
#endif
}


//  Bug 5532.  UTF-8 based UText fails in dictionary code.
//             This test checks the initial patch,
//             which is to just keep it from crashing.  Correct word boundaries
//             await a proper fix to the dictionary code.
//
void RBBITest::TestBug5532(void)  {
   // Text includes a mixture of Thai and Latin.
   const unsigned char utf8Data[] = {
           0xE0u, 0xB8u, 0x82u, 0xE0u, 0xB8u, 0xB2u, 0xE0u, 0xB8u, 0xA2u, 0xE0u,
           0xB9u, 0x80u, 0xE0u, 0xB8u, 0x84u, 0xE0u, 0xB8u, 0xA3u, 0xE0u, 0xB8u, 
           0xB7u, 0xE0u, 0xB9u, 0x88u, 0xE0u, 0xB8u, 0xADu, 0xE0u, 0xB8u, 0x87u,
           0xE0u, 0xB9u, 0x80u, 0xE0u, 0xB8u, 0xA5u, 0xE0u, 0xB9u, 0x88u, 0xE0u,
           0xB8u, 0x99u, 0xE0u, 0xB8u, 0x8Bu, 0xE0u, 0xB8u, 0xB5u, 0xE0u, 0xB8u,
           0x94u, 0xE0u, 0xB8u, 0xB5u, 0x20u, 0x73u, 0x69u, 0x6Du, 0x20u, 0x61u, 
           0x75u, 0x64u, 0x69u, 0x6Fu, 0x2Fu, 0x20u, 0x4Du, 0x4Fu, 0x4Fu, 0x4Eu, 
           0x20u, 0x65u, 0x63u, 0x6Cu, 0x69u, 0x70u, 0x73u, 0x65u, 0x20u, 0xE0u, 
           0xB8u, 0xA3u, 0xE0u, 0xB8u, 0xB2u, 0xE0u, 0xB8u, 0x84u, 0xE0u, 0xB8u, 
           0xB2u, 0x20u, 0x34u, 0x37u, 0x30u, 0x30u, 0x20u, 0xE0u, 0xB8u, 0xA2u, 
           0xE0u, 0xB8u, 0xB9u, 0xE0u, 0xB9u, 0x82u, 0xE0u, 0xB8u, 0xA3u, 0x00};

    UErrorCode status = U_ZERO_ERROR;
    UText utext=UTEXT_INITIALIZER;
    utext_openUTF8(&utext, (const char *)utf8Data, -1, &status);
    TEST_ASSERT_SUCCESS(status);

    BreakIterator *bi = BreakIterator::createWordInstance(Locale("th"), status);
    TEST_ASSERT_SUCCESS(status);
    if (U_SUCCESS(status)) {
        bi->setText(&utext, status);
        TEST_ASSERT_SUCCESS(status);

        int32_t breakCount = 0;
        int32_t previousBreak = -1;
        for (bi->first(); bi->next() != BreakIterator::DONE; breakCount++) {
            // For now, just make sure that the break iterator doesn't hang.
            TEST_ASSERT(previousBreak < bi->current());
            previousBreak = bi->current();
        }
        TEST_ASSERT(breakCount > 0);
    }
    delete bi;
    utext_close(&utext);
}


void RBBITest::TestBug9983(void)  {
    UnicodeString text = UnicodeString("\\u002A"  // * Other
                                       "\\uFF65"  //   Other
                                       "\\u309C"  //   Katakana
                                       "\\uFF9F"  //   Extend
                                       "\\uFF65"  //   Other
                                       "\\u0020"  //   Other
                                       "\\u0000").unescape();

    UErrorCode status = U_ZERO_ERROR;
    LocalPointer<RuleBasedBreakIterator> brkiter(static_cast<RuleBasedBreakIterator *>(
        BreakIterator::createWordInstance(Locale::getRoot(), status)));
    TEST_ASSERT_SUCCESS(status);
    LocalPointer<RuleBasedBreakIterator> brkiterPOSIX(static_cast<RuleBasedBreakIterator *>(
        BreakIterator::createWordInstance(Locale::createFromName("en_US_POSIX"), status)));
    TEST_ASSERT_SUCCESS(status);
    if (U_FAILURE(status)) {
        return;
    }
    int32_t offset, rstatus, iterationCount;

    brkiter->setText(text);
    brkiter->last();
    iterationCount = 0;
    while ( (offset = brkiter->previous()) != UBRK_DONE ) {
        iterationCount++;
        rstatus = brkiter->getRuleStatus();
        (void)rstatus;     // Suppress set but not used warning.
        if (iterationCount >= 10) {
           break; 
        }
    }
    TEST_ASSERT(iterationCount == 6);

    brkiterPOSIX->setText(text);
    brkiterPOSIX->last();
    iterationCount = 0;
    while ( (offset = brkiterPOSIX->previous()) != UBRK_DONE ) {
        iterationCount++;
        rstatus = brkiterPOSIX->getRuleStatus();
        (void)rstatus;     // Suppress set but not used warning.
        if (iterationCount >= 10) {
           break; 
        }
    }
    TEST_ASSERT(iterationCount == 6);
}


//
//  TestDebug    -  A place-holder test for debugging purposes.
//                  For putting in fragments of other tests that can be invoked
//                  for tracing  without a lot of unwanted extra stuff happening.
//
void RBBITest::TestDebug(void) {
#if 0
    UErrorCode   status = U_ZERO_ERROR;
    int pos = 0;
    int ruleStatus = 0;

    RuleBasedBreakIterator* bi =
       // (RuleBasedBreakIterator *)BreakIterator::createLineInstance(Locale::getDefault(), status);
       // (RuleBasedBreakIterator *)BreakIterator::createWordInstance(Locale::Locale("th"), status);
       (RuleBasedBreakIterator *)BreakIterator::createSentenceInstance(Locale::getDefault(), status);
    UnicodeString s("\\u2008\\u002e\\udc6a\\u37cd\\u71d0\\u2048\\U000e006a\\u002e\\u0046\\ufd3f\\u000a\\u002e");
    // UnicodeString s("Aaa.  Bcd");
    s = s.unescape();
    bi->setText(s);
    UBool r = bi->isBoundary(8);
    printf("%s", r?"true":"false");
    return;
    pos = bi->last();
    do {
        // ruleStatus = bi->getRuleStatus();
        printf("%d\t%d\n", pos, ruleStatus);
        pos = bi->previous();
    } while (pos != BreakIterator::DONE);
#endif
}

void RBBITest::TestProperties() {
    UErrorCode errorCode = U_ZERO_ERROR;
    UnicodeSet prependSet(UNICODE_STRING_SIMPLE("[:GCB=Prepend:]"), errorCode);
    if (!prependSet.isEmpty()) {
        errln(
            "[:GCB=Prepend:] is not empty any more. "
            "Uncomment relevant lines in source/data/brkitr/char.txt and "
            "change this test to the opposite condition.");
    }
}

#endif /* #if !UCONFIG_NO_BREAK_ITERATION */
