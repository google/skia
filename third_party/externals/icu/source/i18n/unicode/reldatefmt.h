/*
*****************************************************************************
* Copyright (C) 2014, International Business Machines Corporation and
* others.
* All Rights Reserved.
*****************************************************************************
*
* File RELDATEFMT.H
*****************************************************************************
*/

#ifndef __RELDATEFMT_H
#define __RELDATEFMT_H

#include "unicode/utypes.h"
#include "unicode/uobject.h"
#include "unicode/udisplaycontext.h"
#include "unicode/locid.h"

/**
 * \file
 * \brief C++ API: Formats relative dates such as "1 day ago" or "tomorrow"
 */

#if !UCONFIG_NO_FORMATTING && !UCONFIG_NO_BREAK_ITERATION

#ifndef U_HIDE_DRAFT_API

/**
 * The formatting style
 * @draft ICU 54
 */
typedef enum UDateRelativeDateTimeFormatterStyle {

  /**
   * Everything spelled out.
   * @draft ICU 54
   */
  UDAT_STYLE_LONG,

  /**
   * Abbreviations used when possible.
   * @draft ICU 54
   */
  UDAT_STYLE_SHORT,

  /**
   * Use the shortest possible form.
   * @draft ICU 54
   */
  UDAT_STYLE_NARROW,

  /**
   * The number of styles.
   * @draft ICU 54
   */
  UDAT_STYLE_COUNT
} UDateRelativeDateTimeFormatterStyle; 

/**
 * Represents the unit for formatting a relative date. e.g "in 5 days"
 * or "in 3 months"
 * @draft ICU 53
 */
typedef enum UDateRelativeUnit {

    /**
     * Seconds
     * @draft ICU 53
     */
    UDAT_RELATIVE_SECONDS,

    /**
     * Minutes
     * @draft ICU 53
     */
    UDAT_RELATIVE_MINUTES,

    /**
     * Hours
     * @draft ICU 53
     */
    UDAT_RELATIVE_HOURS,

    /**
     * Days
     * @draft ICU 53
     */
    UDAT_RELATIVE_DAYS,

    /**
     * Weeks
     * @draft ICU 53
     */
    UDAT_RELATIVE_WEEKS,

    /**
     * Months
     * @draft ICU 53
     */
    UDAT_RELATIVE_MONTHS,

    /**
     * Years
     * @draft ICU 53
     */
    UDAT_RELATIVE_YEARS,

    /**
     * Count of items in this enum.
     * @draft ICU 53
     */
    UDAT_RELATIVE_UNIT_COUNT
} UDateRelativeUnit;

/**
 * Represents an absolute unit.
 * @draft ICU 53
 */
typedef enum UDateAbsoluteUnit {

    // Days of week have to remain together and in order from Sunday to
    // Saturday.
    /**
     * Sunday
     * @draft ICU 53
     */
    UDAT_ABSOLUTE_SUNDAY,

    /**
     * Monday
     * @draft ICU 53
     */
    UDAT_ABSOLUTE_MONDAY,

    /**
     * Tuesday
     * @draft ICU 53
     */
    UDAT_ABSOLUTE_TUESDAY,

    /**
     * Wednesday
     * @draft ICU 53
     */
    UDAT_ABSOLUTE_WEDNESDAY,

    /**
     * Thursday
     * @draft ICU 53
     */
    UDAT_ABSOLUTE_THURSDAY,

    /**
     * Friday
     * @draft ICU 53
     */
    UDAT_ABSOLUTE_FRIDAY,

    /**
     * Saturday
     * @draft ICU 53
     */
    UDAT_ABSOLUTE_SATURDAY,

    /**
     * Day
     * @draft ICU 53
     */
    UDAT_ABSOLUTE_DAY,

    /**
     * Week
     * @draft ICU 53
     */
    UDAT_ABSOLUTE_WEEK,

    /**
     * Month
     * @draft ICU 53
     */
    UDAT_ABSOLUTE_MONTH,

    /**
     * Year
     * @draft ICU 53
     */
    UDAT_ABSOLUTE_YEAR,

    /**
     * Now
     * @draft ICU 53
     */
    UDAT_ABSOLUTE_NOW,

    /**
     * Count of items in this enum.
     * @draft ICU 53
     */
    UDAT_ABSOLUTE_UNIT_COUNT
} UDateAbsoluteUnit;

/**
 * Represents a direction for an absolute unit e.g "Next Tuesday"
 * or "Last Tuesday"
 * @draft ICU 53
 */
typedef enum UDateDirection {

    /**
     * Two before. Not fully supported in every locale.
     * @draft ICU 53
     */
    UDAT_DIRECTION_LAST_2,

    /**
     * Last
     * @draft ICU 53
     */
    UDAT_DIRECTION_LAST,

    /**
     * This
     * @draft ICU 53
     */
    UDAT_DIRECTION_THIS,

    /**
     * Next
     * @draft ICU 53
     */
    UDAT_DIRECTION_NEXT,

    /**
     * Two after. Not fully supported in every locale.
     * @draft ICU 53
     */
    UDAT_DIRECTION_NEXT_2,

    /**
     * Plain, which means the absence of a qualifier.
     * @draft ICU 53
     */
    UDAT_DIRECTION_PLAIN,

    /**
     * Count of items in this enum.
     * @draft ICU 53
     */
    UDAT_DIRECTION_COUNT
} UDateDirection;


U_NAMESPACE_BEGIN

class RelativeDateTimeCacheData;
class SharedNumberFormat;
class SharedPluralRules;
class SharedBreakIterator;
class NumberFormat;
class UnicodeString;

/**
 * Formats simple relative dates. There are two types of relative dates that
 * it handles:
 * <ul>
 *   <li>relative dates with a quantity e.g "in 5 days"</li>
 *   <li>relative dates without a quantity e.g "next Tuesday"</li>
 * </ul>
 * <p>
 * This API is very basic and is intended to be a building block for more
 * fancy APIs. The caller tells it exactly what to display in a locale
 * independent way. While this class automatically provides the correct plural
 * forms, the grammatical form is otherwise as neutral as possible. It is the
 * caller's responsibility to handle cut-off logic such as deciding between
 * displaying "in 7 days" or "in 1 week." This API supports relative dates
 * involving one single unit. This API does not support relative dates
 * involving compound units,
 * e.g "in 5 days and 4 hours" nor does it support parsing.
 * <p>
 * This class is mostly thread safe and immutable with the following caveats:
 * 1. The assignment operator violates Immutability. It must not be used
 *    concurrently with other operations.
 * 2. Caller must not hold onto adopted pointers.
 * <p>
 * This class is not intended for public subclassing.
 * <p>
 * Here are some examples of use:
 * <blockquote>
 * <pre>
 * UErrorCode status = U_ZERO_ERROR;
 * UnicodeString appendTo;
 * RelativeDateTimeFormatter fmt(status);
 * // Appends "in 1 day"
 * fmt.format(
 *     1, UDAT_DIRECTION_NEXT, UDAT_RELATIVE_DAYS, appendTo, status);
 * // Appends "in 3 days"
 * fmt.format(
 *     3, UDAT_DIRECTION_NEXT, UDAT_RELATIVE_DAYS, appendTo, status);
 * // Appends "3.2 years ago"
 * fmt.format(
 *     3.2, UDAT_DIRECTION_LAST, UDAT_RELATIVE_YEARS, appendTo, status);
 * // Appends "last Sunday"
 * fmt.format(UDAT_DIRECTION_LAST, UDAT_ABSOLUTE_SUNDAY, appendTo, status);
 * // Appends "this Sunday"
 * fmt.format(UDAT_DIRECTION_THIS, UDAT_ABSOLUTE_SUNDAY, appendTo, status);
 * // Appends "next Sunday"
 * fmt.format(UDAT_DIRECTION_NEXT, UDAT_ABSOLUTE_SUNDAY, appendTo, status);
 * // Appends "Sunday"
 * fmt.format(UDAT_DIRECTION_PLAIN, UDAT_ABSOLUTE_SUNDAY, appendTo, status);
 *
 * // Appends "yesterday"
 * fmt.format(UDAT_DIRECTION_LAST, UDAT_ABSOLUTE_DAY, appendTo, status);
 * // Appends "today"
 * fmt.format(UDAT_DIRECTION_THIS, UDAT_ABSOLUTE_DAY, appendTo, status);
 * // Appends "tomorrow"
 * fmt.format(UDAT_DIRECTION_NEXT, UDAT_ABSOLUTE_DAY, appendTo, status);
 * // Appends "now"
 * fmt.format(UDAT_DIRECTION_PLAIN, UDAT_ABSOLUTE_NOW, appendTo, status);
 *
 * </pre>
 * </blockquote>
 * <p>
 * In the future, we may add more forms, such as abbreviated/short forms
 * (3 secs ago), and relative day periods ("yesterday afternoon"), etc.
 *
 * The RelativeDateTimeFormatter class is not intended for public subclassing.
 *
 * @draft ICU 53
 */
class U_I18N_API RelativeDateTimeFormatter : public UObject {
public:

    /**
     * Create RelativeDateTimeFormatter with default locale.
     * @draft ICU 53
     */
    RelativeDateTimeFormatter(UErrorCode& status);

    /**
     * Create RelativeDateTimeFormatter with given locale.
     * @draft ICU 53
     */
    RelativeDateTimeFormatter(const Locale& locale, UErrorCode& status);

    /**
     * Create RelativeDateTimeFormatter with given locale and NumberFormat.
     *
     * @param locale the locale
     * @param nfToAdopt Constructed object takes ownership of this pointer.
     *   It is an error for caller to delete this pointer or change its
     *   contents after calling this constructor.
     * @status Any error is returned here.
     * @draft ICU 53
     */
    RelativeDateTimeFormatter(
        const Locale& locale, NumberFormat *nfToAdopt, UErrorCode& status);

    /**
     * Create RelativeDateTimeFormatter with given locale, NumberFormat,
     * and capitalization context.
     *
     * @param locale the locale
     * @param nfToAdopt Constructed object takes ownership of this pointer.
     *   It is an error for caller to delete this pointer or change its
     *   contents after calling this constructor. Caller may pass NULL for
     *   this argument if they want default number format behavior.
     * @param style the format style. The UDAT_RELATIVE bit field has no effect.
     * @param capitalizationContext A value from UDisplayContext that pertains to
     * capitalization.
     * @status Any error is returned here. 
     * @draft ICU 54
     */
    RelativeDateTimeFormatter(
            const Locale& locale,
            NumberFormat *nfToAdopt,
            UDateRelativeDateTimeFormatterStyle style,
            UDisplayContext capitalizationContext,
            UErrorCode& status);

    /**
     * Copy constructor.
     * @draft ICU 53
     */
    RelativeDateTimeFormatter(const RelativeDateTimeFormatter& other);

    /**
     * Assignment operator.
     * @draft ICU 53
     */
    RelativeDateTimeFormatter& operator=(
            const RelativeDateTimeFormatter& other);

    /**
     * Destructor.
     * @draft ICU 53
     */
    virtual ~RelativeDateTimeFormatter();

    /**
     * Formats a relative date with a quantity such as "in 5 days" or
     * "3 months ago"
     * @param quantity The numerical amount e.g 5. This value is formatted
     * according to this object's NumberFormat object.
     * @param direction NEXT means a future relative date; LAST means a past
     * relative date. If direction is anything else, this method sets
     * status to U_ILLEGAL_ARGUMENT_ERROR.
     * @param unit the unit e.g day? month? year?
     * @param appendTo The string to which the formatted result will be
     *  appended
     * @param status ICU error code returned here.
     * @return appendTo
     * @draft ICU 53
     */
    UnicodeString& format(
            double quantity,
            UDateDirection direction,
            UDateRelativeUnit unit,
            UnicodeString& appendTo,
            UErrorCode& status) const;

    /**
     * Formats a relative date without a quantity.
     * @param direction NEXT, LAST, THIS, etc.
     * @param unit e.g SATURDAY, DAY, MONTH
     * @param appendTo The string to which the formatted result will be
     *  appended. If the value of direction is documented as not being fully
     *  supported in all locales then this method leaves appendTo unchanged if
     *  no format string is available.
     * @param status ICU error code returned here.
     * @return appendTo
     * @draft ICU 53
     */
    UnicodeString& format(
            UDateDirection direction,
            UDateAbsoluteUnit unit,
            UnicodeString& appendTo,
            UErrorCode& status) const;

    /**
     * Combines a relative date string and a time string in this object's
     * locale. This is done with the same date-time separator used for the
     * default calendar in this locale.
     *
     * @param relativeDateString the relative date, e.g 'yesterday'
     * @param timeString the time e.g '3:45'
     * @param appendTo concatenated date and time appended here
     * @param status ICU error code returned here.
     * @return appendTo
     * @draft ICU 53
     */
    UnicodeString& combineDateAndTime(
            const UnicodeString& relativeDateString,
            const UnicodeString& timeString,
            UnicodeString& appendTo,
            UErrorCode& status) const;

    /**
     * Returns the NumberFormat this object is using.
     *
     * @draft ICU 53
     */
    const NumberFormat& getNumberFormat() const;

    /**
     * Returns the capitalization context.
     *
     * @draft ICU 54
     */
    UDisplayContext getCapitalizationContext() const;

    /**
     * Returns the format style.
     *
     * @draft ICU 54
     */
    UDateRelativeDateTimeFormatterStyle getFormatStyle() const;
private:
    const RelativeDateTimeCacheData* fCache;
    const SharedNumberFormat *fNumberFormat;
    const SharedPluralRules *fPluralRules;
    UDateRelativeDateTimeFormatterStyle fStyle;
    UDisplayContext fContext;
    const SharedBreakIterator *fOptBreakIterator;
    Locale fLocale;
    void init(
            NumberFormat *nfToAdopt,
            BreakIterator *brkIter,
            UErrorCode &status);
    void adjustForContext(UnicodeString &) const;
};

U_NAMESPACE_END

#endif /* U_HIDE_DRAFT_API */

#endif /* !UCONFIG_NO_FORMATTING && !UCONFIG_NO_BREAK_ITERATION*/
#endif
