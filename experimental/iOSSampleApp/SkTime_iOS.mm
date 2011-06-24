//#if defined(SK_BUILD_FOR_IOS)
#include "SkTime.h"
#include <sys/time.h>
#include <Foundation/Foundation.h>
void SkTime::GetDateTime(DateTime* dt)
{
    if (dt)
    {
        NSCalendar *calendar = [[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar];
        NSCalendarUnit unitFlags =  NSYearCalendarUnit | NSMonthCalendarUnit | 
                                    NSWeekCalendarUnit | NSDayCalendarUnit | 
                                    NSHourCalendarUnit | NSMinuteCalendarUnit | 
                                    NSSecondCalendarUnit;
        
        NSDate *date = [NSDate date];
        NSDateComponents *dateComponents = [calendar components:unitFlags 
                                                       fromDate:date];
        dt->fYear       = SkToU16([dateComponents year]);
        dt->fMonth      = SkToU8([dateComponents month]);
        dt->fDayOfWeek  = SkToU8([dateComponents weekday]);
        dt->fDay        = SkToU8([dateComponents day]);
        dt->fHour       = SkToU8([dateComponents hour]);
        dt->fMinute     = SkToU8([dateComponents minute]);
        dt->fSecond     = SkToU8([dateComponents second]);
    }
}

SkMSec SkTime::GetMSecs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (SkMSec) (tv.tv_sec * 1000 + tv.tv_usec / 1000 ); // microseconds to milliseconds
}
//#endif