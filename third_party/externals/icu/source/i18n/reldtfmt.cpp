/*
*******************************************************************************
* Copyright (C) 2007-2014, International Business Machines Corporation and
* others. All Rights Reserved.
*******************************************************************************
*/

#include "unicode/utypes.h"

#if !UCONFIG_NO_FORMATTING

#include <stdlib.h>

#include "reldtfmt.h"
#include "unicode/datefmt.h"
#include "unicode/smpdtfmt.h"
#include "unicode/msgfmt.h"
#include "unicode/udisplaycontext.h"
#include "unicode/uchar.h"
#include "unicode/brkiter.h"

#include "gregoimp.h" // for CalendarData
#include "cmemory.h"
#include "uresimp.h"

U_NAMESPACE_BEGIN


/**
 * An array of URelativeString structs is used to store the resource data loaded out of the bundle.
 */
struct URelativeString {
    int32_t offset;         /** offset of this item, such as, the relative date **/
    int32_t len;            /** length of the string **/
    const UChar* string;    /** string, or NULL if not set **/
};

static const char DT_DateTimePatternsTag[]="DateTimePatterns";


UOBJECT_DEFINE_RTTI_IMPLEMENTATION(RelativeDateFormat)

RelativeDateFormat::RelativeDateFormat(const RelativeDateFormat& other) :
 DateFormat(other), fDateTimeFormatter(NULL), fDatePattern(other.fDatePattern),
 fTimePattern(other.fTimePattern), fCombinedFormat(NULL),
 fDateStyle(other.fDateStyle), fLocale(other.fLocale),
 fDayMin(other.fDayMin), fDayMax(other.fDayMax),
 fDatesLen(other.fDatesLen), fDates(NULL),
 fCombinedHasDateAtStart(other.fCombinedHasDateAtStart),
 fCapitalizationInfoSet(other.fCapitalizationInfoSet),
 fCapitalizationOfRelativeUnitsForUIListMenu(other.fCapitalizationOfRelativeUnitsForUIListMenu),
 fCapitalizationOfRelativeUnitsForStandAlone(other.fCapitalizationOfRelativeUnitsForStandAlone),
 fCapitalizationBrkIter(NULL)
{
    if(other.fDateTimeFormatter != NULL) {
        fDateTimeFormatter = (SimpleDateFormat*)other.fDateTimeFormatter->clone();
    }
    if(other.fCombinedFormat != NULL) {
        fCombinedFormat = (MessageFormat*)other.fCombinedFormat->clone();
    }
    if (fDatesLen > 0) {
        fDates = (URelativeString*) uprv_malloc(sizeof(fDates[0])*fDatesLen);
        uprv_memcpy(fDates, other.fDates, sizeof(fDates[0])*fDatesLen);
    }
#if !UCONFIG_NO_BREAK_ITERATION
    if (other.fCapitalizationBrkIter != NULL) {
        fCapitalizationBrkIter = (other.fCapitalizationBrkIter)->clone();
    }
#endif
}

RelativeDateFormat::RelativeDateFormat( UDateFormatStyle timeStyle, UDateFormatStyle dateStyle,
                                        const Locale& locale, UErrorCode& status) :
 DateFormat(), fDateTimeFormatter(NULL), fDatePattern(), fTimePattern(), fCombinedFormat(NULL),
 fDateStyle(dateStyle), fLocale(locale), fDayMin(0), fDayMax(0), fDatesLen(0), fDates(NULL),
 fCombinedHasDateAtStart(FALSE), fCapitalizationInfoSet(FALSE),
 fCapitalizationOfRelativeUnitsForUIListMenu(FALSE), fCapitalizationOfRelativeUnitsForStandAlone(FALSE),
 fCapitalizationBrkIter(NULL)
{
    if(U_FAILURE(status) ) {
        return;
    }
    
    if (timeStyle < UDAT_NONE || timeStyle > UDAT_SHORT) {
        // don't support other time styles (e.g. relative styles), for now
        status = U_ILLEGAL_ARGUMENT_ERROR;
        return;
    }
    UDateFormatStyle baseDateStyle = (dateStyle > UDAT_SHORT)? (UDateFormatStyle)(dateStyle & ~UDAT_RELATIVE): dateStyle;
    DateFormat * df;
    // Get fDateTimeFormatter from either date or time style (does not matter, we will override the pattern).
    // We do need to get separate patterns for the date & time styles.
    if (baseDateStyle != UDAT_NONE) {
        df = createDateInstance((EStyle)baseDateStyle, locale);
        fDateTimeFormatter=dynamic_cast<SimpleDateFormat *>(df);
        if (fDateTimeFormatter == NULL) {
            status = U_UNSUPPORTED_ERROR;
             return;
        }
        fDateTimeFormatter->toPattern(fDatePattern);
        if (timeStyle != UDAT_NONE) {
            df = createTimeInstance((EStyle)timeStyle, locale);
            SimpleDateFormat *sdf = dynamic_cast<SimpleDateFormat *>(df);
            if (sdf != NULL) {
                sdf->toPattern(fTimePattern);
                delete sdf;
            }
        }
    } else {
        // does not matter whether timeStyle is UDAT_NONE, we need something for fDateTimeFormatter
        df = createTimeInstance((EStyle)timeStyle, locale);
        fDateTimeFormatter=dynamic_cast<SimpleDateFormat *>(df);
        if (fDateTimeFormatter == NULL) {
            status = U_UNSUPPORTED_ERROR;
            return;
        }
        fDateTimeFormatter->toPattern(fTimePattern);
    }
    
    // Initialize the parent fCalendar, so that parse() works correctly.
    initializeCalendar(NULL, locale, status);
    loadDates(status);
}

RelativeDateFormat::~RelativeDateFormat() {
    delete fDateTimeFormatter;
    delete fCombinedFormat;
    uprv_free(fDates);
#if !UCONFIG_NO_BREAK_ITERATION
    delete fCapitalizationBrkIter;
#endif
}


Format* RelativeDateFormat::clone(void) const {
    return new RelativeDateFormat(*this);
}

UBool RelativeDateFormat::operator==(const Format& other) const {
    if(DateFormat::operator==(other)) {
        // The DateFormat::operator== check for fCapitalizationContext equality above
        //   is sufficient to check equality of all derived context-related data.
        // DateFormat::operator== guarantees following cast is safe
        RelativeDateFormat* that = (RelativeDateFormat*)&other;
        return (fDateStyle==that->fDateStyle   &&
                fDatePattern==that->fDatePattern   &&
                fTimePattern==that->fTimePattern   &&
                fLocale==that->fLocale );
    }
    return FALSE;
}

static const UChar APOSTROPHE = (UChar)0x0027;

UnicodeString& RelativeDateFormat::format(  Calendar& cal,
                                UnicodeString& appendTo,
                                FieldPosition& pos) const {
                                
    UErrorCode status = U_ZERO_ERROR;
    UnicodeString relativeDayString;
    UDisplayContext capitalizationContext = getContext(UDISPCTX_TYPE_CAPITALIZATION, status);
    
    // calculate the difference, in days, between 'cal' and now.
    int dayDiff = dayDifference(cal, status);

    // look up string
    int32_t len = 0;
    const UChar *theString = getStringForDay(dayDiff, len, status);
    if(U_SUCCESS(status) && (theString!=NULL)) {
        // found a relative string
        relativeDayString.setTo(theString, len);
    }

    if ( relativeDayString.length() > 0 && !fDatePattern.isEmpty() && 
         (fTimePattern.isEmpty() || fCombinedFormat == NULL || fCombinedHasDateAtStart)) {
#if !UCONFIG_NO_BREAK_ITERATION
        // capitalize relativeDayString according to context for relative, set formatter no context
        if ( u_islower(relativeDayString.char32At(0)) && fCapitalizationBrkIter!= NULL &&
             ( capitalizationContext==UDISPCTX_CAPITALIZATION_FOR_BEGINNING_OF_SENTENCE ||
               (capitalizationContext==UDISPCTX_CAPITALIZATION_FOR_UI_LIST_OR_MENU && fCapitalizationOfRelativeUnitsForUIListMenu) ||
               (capitalizationContext==UDISPCTX_CAPITALIZATION_FOR_STANDALONE && fCapitalizationOfRelativeUnitsForStandAlone) ) ) {
            // titlecase first word of relativeDayString
            relativeDayString.toTitle(fCapitalizationBrkIter, fLocale, U_TITLECASE_NO_LOWERCASE | U_TITLECASE_NO_BREAK_ADJUSTMENT);
        }
#endif
        fDateTimeFormatter->setContext(UDISPCTX_CAPITALIZATION_NONE, status);
    } else {
        // set our context for the formatter
        fDateTimeFormatter->setContext(capitalizationContext, status);
    }

    if (fDatePattern.isEmpty()) {
        fDateTimeFormatter->applyPattern(fTimePattern);
        fDateTimeFormatter->format(cal,appendTo,pos);
    } else if (fTimePattern.isEmpty() || fCombinedFormat == NULL) {
        if (relativeDayString.length() > 0) {
            appendTo.append(relativeDayString);
        } else {
            fDateTimeFormatter->applyPattern(fDatePattern);
            fDateTimeFormatter->format(cal,appendTo,pos);
        }
    } else {
        UnicodeString datePattern;
        if (relativeDayString.length() > 0) {
            // Need to quote the relativeDayString to make it a legal date pattern
            relativeDayString.findAndReplace(UNICODE_STRING("'", 1), UNICODE_STRING("''", 2)); // double any existing APOSTROPHE
            relativeDayString.insert(0, APOSTROPHE); // add APOSTROPHE at beginning...
            relativeDayString.append(APOSTROPHE); // and at end
            datePattern.setTo(relativeDayString);
        } else {
            datePattern.setTo(fDatePattern);
        }
        UnicodeString combinedPattern;
        Formattable timeDatePatterns[] = { fTimePattern, datePattern };
        fCombinedFormat->format(timeDatePatterns, 2, combinedPattern, pos, status); // pos is ignored by this
        fDateTimeFormatter->applyPattern(combinedPattern);
        fDateTimeFormatter->format(cal,appendTo,pos);
    }
    
    return appendTo;
}



UnicodeString&
RelativeDateFormat::format(const Formattable& obj, 
                         UnicodeString& appendTo, 
                         FieldPosition& pos,
                         UErrorCode& status) const
{
    // this is just here to get around the hiding problem
    // (the previous format() override would hide the version of
    // format() on DateFormat that this function correspond to, so we
    // have to redefine it here)
    return DateFormat::format(obj, appendTo, pos, status);
}


void RelativeDateFormat::parse( const UnicodeString& text,
                    Calendar& cal,
                    ParsePosition& pos) const {

    int32_t startIndex = pos.getIndex();
    if (fDatePattern.isEmpty()) {
        // no date pattern, try parsing as time
        fDateTimeFormatter->applyPattern(fTimePattern);
        fDateTimeFormatter->parse(text,cal,pos);
    } else if (fTimePattern.isEmpty() || fCombinedFormat == NULL) {
        // no time pattern or way to combine, try parsing as date
        // first check whether text matches a relativeDayString
        UBool matchedRelative = FALSE;
        for (int n=0; n < fDatesLen && !matchedRelative; n++) {
            if (fDates[n].string != NULL &&
                    text.compare(startIndex, fDates[n].len, fDates[n].string) == 0) {
                // it matched, handle the relative day string
                UErrorCode status = U_ZERO_ERROR;
                matchedRelative = TRUE;

                // Set the calendar to now+offset
                cal.setTime(Calendar::getNow(),status);
                cal.add(UCAL_DATE,fDates[n].offset, status);

                if(U_FAILURE(status)) { 
                    // failure in setting calendar field, set offset to beginning of rel day string
                    pos.setErrorIndex(startIndex);
                } else {
                    pos.setIndex(startIndex + fDates[n].len);
                }
            }
        }
        if (!matchedRelative) {
            // just parse as normal date
            fDateTimeFormatter->applyPattern(fDatePattern);
            fDateTimeFormatter->parse(text,cal,pos);
        }
    } else {
        // Here we replace any relativeDayString in text with the equivalent date
        // formatted per fDatePattern, then parse text normally using the combined pattern.
        UnicodeString modifiedText(text);
        FieldPosition fPos;
        int32_t dateStart = 0, origDateLen = 0, modDateLen = 0;
        UErrorCode status = U_ZERO_ERROR;
        for (int n=0; n < fDatesLen; n++) {
            int32_t relativeStringOffset;
            if (fDates[n].string != NULL &&
                    (relativeStringOffset = modifiedText.indexOf(fDates[n].string, fDates[n].len, startIndex)) >= startIndex) {
                // it matched, replace the relative date with a real one for parsing
                UnicodeString dateString;
                Calendar * tempCal = cal.clone();

                // Set the calendar to now+offset
                tempCal->setTime(Calendar::getNow(),status);
                tempCal->add(UCAL_DATE,fDates[n].offset, status);
                if(U_FAILURE(status)) { 
                    pos.setErrorIndex(startIndex);
                    delete tempCal;
                    return;
                }

                fDateTimeFormatter->applyPattern(fDatePattern);
                fDateTimeFormatter->format(*tempCal, dateString, fPos);
                dateStart = relativeStringOffset;
                origDateLen = fDates[n].len;
                modDateLen = dateString.length();
                modifiedText.replace(dateStart, origDateLen, dateString);
                delete tempCal;
                break;
            }
        }
        UnicodeString combinedPattern;
        Formattable timeDatePatterns[] = { fTimePattern, fDatePattern };
        fCombinedFormat->format(timeDatePatterns, 2, combinedPattern, fPos, status); // pos is ignored by this
        fDateTimeFormatter->applyPattern(combinedPattern);
        fDateTimeFormatter->parse(modifiedText,cal,pos);

        // Adjust offsets
        UBool noError = (pos.getErrorIndex() < 0);
        int32_t offset = (noError)? pos.getIndex(): pos.getErrorIndex();
        if (offset >= dateStart + modDateLen) {
            // offset at or after the end of the replaced text,
            // correct by the difference between original and replacement
            offset -= (modDateLen - origDateLen);
        } else if (offset >= dateStart) {
            // offset in the replaced text, set it to the beginning of that text
            // (i.e. the beginning of the relative day string)
            offset = dateStart;
        }
        if (noError) {
            pos.setIndex(offset);
        } else {
            pos.setErrorIndex(offset);
        }
    }
}

UDate
RelativeDateFormat::parse( const UnicodeString& text,
                         ParsePosition& pos) const {
    // redefined here because the other parse() function hides this function's
    // cunterpart on DateFormat
    return DateFormat::parse(text, pos);
}

UDate
RelativeDateFormat::parse(const UnicodeString& text, UErrorCode& status) const
{
    // redefined here because the other parse() function hides this function's
    // counterpart on DateFormat
    return DateFormat::parse(text, status);
}


const UChar *RelativeDateFormat::getStringForDay(int32_t day, int32_t &len, UErrorCode &status) const {
    if(U_FAILURE(status)) {
        return NULL;
    }
    
    // Is it outside the resource bundle's range?
    if(day < fDayMin || day > fDayMax) {
        return NULL; // don't have it.
    }
    
    // Linear search the held strings
    for(int n=0;n<fDatesLen;n++) {
        if(fDates[n].offset == day) {
            len = fDates[n].len;
            return fDates[n].string;
        }
    }
    
    return NULL;  // not found.
}

UnicodeString&
RelativeDateFormat::toPattern(UnicodeString& result, UErrorCode& status) const
{
    if (!U_FAILURE(status)) {
        result.remove();
        if (fDatePattern.isEmpty()) {
            result.setTo(fTimePattern);
        } else if (fTimePattern.isEmpty() || fCombinedFormat == NULL) {
            result.setTo(fDatePattern);
        } else {
            Formattable timeDatePatterns[] = { fTimePattern, fDatePattern };
            FieldPosition pos;
            fCombinedFormat->format(timeDatePatterns, 2, result, pos, status);
        }
    }
    return result;
}

UnicodeString&
RelativeDateFormat::toPatternDate(UnicodeString& result, UErrorCode& status) const
{
    if (!U_FAILURE(status)) {
        result.remove();
        result.setTo(fDatePattern);
    }
    return result;
}

UnicodeString&
RelativeDateFormat::toPatternTime(UnicodeString& result, UErrorCode& status) const
{
    if (!U_FAILURE(status)) {
        result.remove();
        result.setTo(fTimePattern);
    }
    return result;
}

void
RelativeDateFormat::applyPatterns(const UnicodeString& datePattern, const UnicodeString& timePattern, UErrorCode &status)
{
    if (!U_FAILURE(status)) {
        fDatePattern.setTo(datePattern);
        fTimePattern.setTo(timePattern);
    }
}

const DateFormatSymbols*
RelativeDateFormat::getDateFormatSymbols() const
{
    return fDateTimeFormatter->getDateFormatSymbols();
}

// override the DateFormat implementation in order to
// lazily initialize relevant items
void
RelativeDateFormat::setContext(UDisplayContext value, UErrorCode& status)
{
    DateFormat::setContext(value, status);
    if (U_SUCCESS(status)) {
        if (!fCapitalizationInfoSet &&
                (value==UDISPCTX_CAPITALIZATION_FOR_UI_LIST_OR_MENU || value==UDISPCTX_CAPITALIZATION_FOR_STANDALONE)) {
            initCapitalizationContextInfo(fLocale);
            fCapitalizationInfoSet = TRUE;
        }
#if !UCONFIG_NO_BREAK_ITERATION
        if ( fCapitalizationBrkIter == NULL && (value==UDISPCTX_CAPITALIZATION_FOR_BEGINNING_OF_SENTENCE ||
                (value==UDISPCTX_CAPITALIZATION_FOR_UI_LIST_OR_MENU && fCapitalizationOfRelativeUnitsForUIListMenu) ||
                (value==UDISPCTX_CAPITALIZATION_FOR_STANDALONE && fCapitalizationOfRelativeUnitsForStandAlone)) ) {
            UErrorCode status = U_ZERO_ERROR;
            fCapitalizationBrkIter = BreakIterator::createSentenceInstance(fLocale, status);
            if (U_FAILURE(status)) {
                delete fCapitalizationBrkIter;
                fCapitalizationBrkIter = NULL;
            }
        }
#endif
    }
}

void
RelativeDateFormat::initCapitalizationContextInfo(const Locale& thelocale)
{
#if !UCONFIG_NO_BREAK_ITERATION
    const char * localeID = (thelocale != NULL)? thelocale.getBaseName(): NULL;
    UErrorCode status = U_ZERO_ERROR;
    UResourceBundle *rb = ures_open(NULL, localeID, &status);
    rb = ures_getByKeyWithFallback(rb, "contextTransforms", rb, &status);
    rb = ures_getByKeyWithFallback(rb, "relative", rb, &status);
    if (U_SUCCESS(status) && rb != NULL) {
        int32_t len = 0;
        const int32_t * intVector = ures_getIntVector(rb, &len, &status);
        if (U_SUCCESS(status) && intVector != NULL && len >= 2) {
            fCapitalizationOfRelativeUnitsForUIListMenu = intVector[0];
            fCapitalizationOfRelativeUnitsForStandAlone = intVector[1];
        }
    }
    ures_close(rb);
#endif
}

static const UChar patItem1[] = {0x7B,0x31,0x7D}; // "{1}"
static const int32_t patItem1Len = 3;

void RelativeDateFormat::loadDates(UErrorCode &status) {
    CalendarData calData(fLocale, "gregorian", status);
    
    UErrorCode tempStatus = status;
    UResourceBundle *dateTimePatterns = calData.getByKey(DT_DateTimePatternsTag, tempStatus);
    if(U_SUCCESS(tempStatus)) {
        int32_t patternsSize = ures_getSize(dateTimePatterns);
        if (patternsSize > kDateTime) {
            int32_t resStrLen = 0;

            int32_t glueIndex = kDateTime;
            if (patternsSize >= (DateFormat::kDateTimeOffset + DateFormat::kShort + 1)) {
                // Get proper date time format
                switch (fDateStyle) { 
                case kFullRelative: 
                case kFull: 
                    glueIndex = kDateTimeOffset + kFull; 
                    break; 
                case kLongRelative: 
                case kLong: 
                    glueIndex = kDateTimeOffset + kLong; 
                    break; 
                case kMediumRelative: 
                case kMedium: 
                    glueIndex = kDateTimeOffset + kMedium; 
                    break;         
                case kShortRelative: 
                case kShort: 
                    glueIndex = kDateTimeOffset + kShort; 
                    break; 
                default: 
                    break; 
                } 
            }

            const UChar *resStr = ures_getStringByIndex(dateTimePatterns, glueIndex, &resStrLen, &tempStatus);
            if (U_SUCCESS(tempStatus) && resStrLen >= patItem1Len && u_strncmp(resStr,patItem1,patItem1Len)==0) {
                fCombinedHasDateAtStart = TRUE;
            }
            fCombinedFormat = new MessageFormat(UnicodeString(TRUE, resStr, resStrLen), fLocale, tempStatus);
        }
    }

    UResourceBundle *rb = ures_open(NULL, fLocale.getBaseName(), &status);
    rb = ures_getByKeyWithFallback(rb, "fields", rb, &status);
    rb = ures_getByKeyWithFallback(rb, "day", rb, &status);
    rb = ures_getByKeyWithFallback(rb, "relative", rb, &status);
    // set up min/max 
    fDayMin=-1;
    fDayMax=1;

    if(U_FAILURE(status)) {
        fDatesLen=0;
        ures_close(rb);
        return;
    }

    fDatesLen = ures_getSize(rb);
    fDates = (URelativeString*) uprv_malloc(sizeof(fDates[0])*fDatesLen);

    // Load in each item into the array...
    int n = 0;

    UResourceBundle *subString = NULL;
    
    while(ures_hasNext(rb) && U_SUCCESS(status)) {  // iterate over items
        subString = ures_getNextResource(rb, subString, &status);
        
        if(U_FAILURE(status) || (subString==NULL)) break;
        
        // key = offset #
        const char *key = ures_getKey(subString);
        
        // load the string and length
        int32_t aLen;
        const UChar* aString = ures_getString(subString, &aLen, &status);
        
        if(U_FAILURE(status) || aString == NULL) break;

        // calculate the offset
        int32_t offset = atoi(key);
        
        // set min/max
        if(offset < fDayMin) {
            fDayMin = offset;
        }
        if(offset > fDayMax) {
            fDayMax = offset;
        }
        
        // copy the string pointer
        fDates[n].offset = offset;
        fDates[n].string = aString;
        fDates[n].len = aLen; 

        n++;
    }
    ures_close(subString);
    ures_close(rb);
    
    // the fDates[] array could be sorted here, for direct access.
}

//----------------------------------------------------------------------

// this should to be in DateFormat, instead it was copied from SimpleDateFormat.

Calendar*
RelativeDateFormat::initializeCalendar(TimeZone* adoptZone, const Locale& locale, UErrorCode& status)
{
    if(!U_FAILURE(status)) {
        fCalendar = Calendar::createInstance(adoptZone?adoptZone:TimeZone::createDefault(), locale, status);
    }
    if (U_SUCCESS(status) && fCalendar == NULL) {
        status = U_MEMORY_ALLOCATION_ERROR;
    }
    return fCalendar;
}

int32_t RelativeDateFormat::dayDifference(Calendar &cal, UErrorCode &status) {
    if(U_FAILURE(status)) {
        return 0;
    }
    // TODO: Cache the nowCal to avoid heap allocs? Would be difficult, don't know the calendar type
    Calendar *nowCal = cal.clone();
    nowCal->setTime(Calendar::getNow(), status);

    // For the day difference, we are interested in the difference in the (modified) julian day number
    // which is midnight to midnight.  Using fieldDifference() is NOT correct here, because 
    // 6pm Jan 4th  to 10am Jan 5th should be considered "tomorrow".
    int32_t dayDiff = cal.get(UCAL_JULIAN_DAY, status) - nowCal->get(UCAL_JULIAN_DAY, status);

    delete nowCal;
    return dayDiff;
}

U_NAMESPACE_END

#endif

