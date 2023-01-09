/*
  time.h - low level time and date functions
*/

/*
  July 3 2011 - fixed elapsedSecsThisWeek macro (thanks Vincent Valdy for this)
              - fixed  daysToTime_t macro (thanks maniacbug)
*/     

/**
 ******************************************************************************
 * @file    spark_wiring_time.h
 * @author  Satish Nair
 * @version V1.0.0
 * @date    3-March-2014
 * @brief   Header for spark_wiring_time.cpp module
 ******************************************************************************
  Copyright (c) 2013-2015 Particle Industries, Inc.  All rights reserved.
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */

#ifndef _Time_h
#ifdef __cplusplus
#define _Time_h

#include <inttypes.h>
#ifndef __AVR__
#include <sys/types.h> // for __time_t_defined, but avr libc lacks sys/types.h
#include <sys/time.h>
#include <ctime>
#endif


#if !defined(__time_t_defined) // avoid conflict with newlib or other posix libc
typedef unsigned long time_t;
#endif


// This ugly hack allows us to define C++ overloaded functions, when included
// from within an extern "C", as newlib's sys/stat.h does.  Actually it is
// intended to include "time.h" from the C library (on ARM, but AVR does not
// have that file at all).  On Mac and Windows, the compiler will find this
// "Time.h" instead of the C library "time.h", so we may cause other weird
// and unpredictable effects by conflicting with the C library header "time.h",
// but at least this hack lets us define C++ functions as intended.  Hopefully
// nothing too terrible will result from overriding the C library header?!
extern "C++" {
typedef enum {timeNotSet, timeNeedsSync, timeSet
}  timeStatus_t ;


extern const char* TIME_FORMAT_DEFAULT;
extern const char* TIME_FORMAT_ISO8601_FULL;

//convenience macros to convert to and from tm years 
#define  tmYearToCalendar(Y) ((Y) + 1970)  // full four digit year 
#define  CalendarYrToTm(Y)   ((Y) - 1970)
#define  tmYearToY2k(Y)      ((Y) - 30)    // offset is from 2000
#define  y2kYearToTm(Y)      ((Y) + 30)   

typedef time_t(*getExternalTime)();
//typedef void  (*setExternalTime)(const time_t); // not used in this version


/*==============================================================================*/
/* Useful Constants */
#define SECS_PER_MIN  ((time_t)(60UL))
#define SECS_PER_HOUR ((time_t)(3600UL))
#define SECS_PER_DAY  ((time_t)(SECS_PER_HOUR * 24UL))
#define DAYS_PER_WEEK ((time_t)(7UL))
#define SECS_PER_WEEK ((time_t)(SECS_PER_DAY * DAYS_PER_WEEK))
#define SECS_PER_YEAR ((time_t)(SECS_PER_DAY * 365UL)) // TODO: ought to handle leap years
#define SECS_YR_2000  ((time_t)(946684800UL)) // the time at the start of y2k
 
/* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) ((_time_) % SECS_PER_MIN)  
#define numberOfMinutes(_time_) (((_time_) / SECS_PER_MIN) % SECS_PER_MIN) 
#define numberOfHours(_time_) (((_time_) % SECS_PER_DAY) / SECS_PER_HOUR)
#define dayOfWeek(_time_) ((((_time_) / SECS_PER_DAY + 4)  % DAYS_PER_WEEK)+1) // 1 = Sunday
#define elapsedDays(_time_) ((_time_) / SECS_PER_DAY)  // this is number of days since Jan 1 1970
#define elapsedSecsToday(_time_) ((_time_) % SECS_PER_DAY)   // the number of seconds since last midnight 
// The following macros are used in calculating alarms and assume the clock is set to a date later than Jan 1 1971
// Always set the correct time before setting alarms
#define previousMidnight(_time_) (((_time_) / SECS_PER_DAY) * SECS_PER_DAY)  // time at the start of the given day
#define nextMidnight(_time_) (previousMidnight(_time_)  + SECS_PER_DAY)   // time at the end of the given day 
#define elapsedSecsThisWeek(_time_) (elapsedSecsToday(_time_) +  ((dayOfWeek(_time_)-1) * SECS_PER_DAY))   // note that week starts on day 1
#define previousSunday(_time_) ((_time_) - elapsedSecsThisWeek(_time_))      // time at the start of the week for the given time
#define nextSunday(_time_) (previousSunday(_time_)+SECS_PER_WEEK)          // time at the end of the week for the given time


/* Useful Macros for converting elapsed time to a time_t */
#define minutesToTime_t ((M)) ( (M) * SECS_PER_MIN)  
#define hoursToTime_t   ((H)) ( (H) * SECS_PER_HOUR)  
#define daysToTime_t    ((D)) ( (D) * SECS_PER_DAY) // fixed on Jul 22 2011
#define weeksToTime_t   ((W)) ( (W) * SECS_PER_WEEK)   

#define dt_MAX_STRING_LEN 9 // length of longest date string (excluding terminating null)

class TimeClass {
public:
    TimeClass() {
    }

    /*============================================================================*/
    /*  time and date functions   */
    static int     hour();            // the hour now 
    static int     hour(time_t t);    // the hour for the given time
    static int     hourFormat12();    // the hour now in 12 hour format
    static int     hourFormat12(time_t t); // the hour for the given time in 12 hour format
    static uint8_t isAM();            // returns true if time now is AM
    static uint8_t isAM(time_t t);    // returns true the given time is AM
    static uint8_t isPM();            // returns true if time now is PM
    static uint8_t isPM(time_t t);    // returns true the given time is PM
    static int     minute();          // the minute now 
    static int     minute(time_t t);  // the minute for the given time
    static int     second();          // the second now 
    static int     second(time_t t);  // the second for the given time
    static int     day();             // the day now 
    static int     day(time_t t);     // the day for the given time
    static int     weekday();         // the weekday now (Sunday is day 1) 
    static int     weekday(time_t t); // the weekday for the given time 
    static int     month();           // the month now  (Jan is month 1)
    static int     month(time_t t);   // the month for the given time
    static int     year();            // the full four digit year: (2009, 2010 etc) 
    static int     year(time_t t);    // the year for the given time

    static time_t now();              // return the current time as seconds since Jan 1 1970 
    static void    setTime(time_t t);
    static void    setTime(int hr,int min,int sec,int day, int month, int yr);
    static void    adjustTime(long adjustment);

    /* date strings */ 
    static char* monthStr(uint8_t month);
    static char* dayStr(uint8_t day);
    static char* monthShortStr(uint8_t month);
    static char* dayShortStr(uint8_t day);
      
    /* time sync functions	*/
    static timeStatus_t timeStatus(); // indicates if time has been set and recently synchronized
    static void    setSyncProvider( getExternalTime getTimeFunction); // identify the external time provider
    static void    setSyncInterval(time_t interval); // set the number of seconds between re-sync

    static void refreshCache(time_t t);

    /* low level functions to convert to and from system time                     */
    static void breakTime(time_t time, tm &tme);  // break time_t into elements
    static time_t makeTime(const tm &tme);  // convert time elements into time_t

    /* return string representation of the current time */
    inline const char* timeStr()
    {
      return timeStr(now());
    }

    /* return string representation for the given time */
    static const char* timeStr(time_t t);

    /**
     * Return a string representation of the given time using strftime().
     * This function takes several kilobytes of flash memory so it's kept separate
     * from `timeStr()` to reduce memory footprint for applications that don't use
     * alternative time formats.
     *
     * @param t
     * @param format_spec
     * @return
     */
    const char* format(time_t t, const char* format_spec=NULL);

    inline const char* format(const char* format_spec=NULL)
    {
        return format(now(), format_spec);
    }

    void setFormat(const char* format)
    {
        this->format_spec = format;
    }

    const char* getFormat() const { return format_spec; }

private:
    static struct tm _tm;
    static const char* format_spec;
    static const char* timeFormatImpl(tm* calendar_time, const char* format, int time_zone);
};

extern TimeClass Time;
} // extern "C++"
#endif // __cplusplus
#endif /* _Time_h */

