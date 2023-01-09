/*
  time.c - low level time and date functions
  Copyright (c) Michael Margolis 2009-2014

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  1.0  6  Jan 2010 - initial release
  1.1  12 Feb 2010 - fixed leap year calculation error
  1.2  1  Nov 2010 - fixed setTime bug (thanks to Korman for this)
  1.3  24 Mar 2012 - many edits by Paul Stoffregen: fixed timeStatus() to update
                     status, updated examples for Arduino 1.0, fixed ARM
                     compatibility issues, added TimeArduinoDue and TimeTeensy3
                     examples, add error checking and messages to RTC examples,
                     add examples to DS1307RTC library.
  1.4  5  Sep 2014 - compatibility with Arduino 1.5.7
*/

/**
 ******************************************************************************
 * @file    spark_wiring_time.cpp
 * @author  Satish Nair
 * @version V1.0.0
 * @date    3-March-2014
 * @brief   Time utility functions to set and get Date/Time using RTC
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

#if ARDUINO >= 100
#include <Arduino.h> 
#else
#include <WProgram.h> 
#endif

#include "TimeLib.h"

const char* TIME_FORMAT_DEFAULT = "asctime";
const char* TIME_FORMAT_ISO8601_FULL = "%Y-%m-%dT%H:%M:%S%z";

TimeClass Time;
static time_t cacheTime;   // the time the cache was updated
static uint32_t syncInterval = 300;  // time sync will be attempted after this many seconds

void TimeClass::refreshCache(time_t t) {
  if (t != cacheTime) {
    TimeClass::breakTime(t, _tm); 
    cacheTime = t; 
  }
}

int TimeClass::hour() { // the hour now 
  return hour(now()); 
}

int TimeClass::hour(time_t t) { // the hour for the given time
  refreshCache(t);
  return _tm.tm_hour;  
}

int TimeClass::hourFormat12() { // the hour now in 12 hour format
  return hourFormat12(now()); 
}

int TimeClass::hourFormat12(time_t t) { // the hour for the given time in 12 hour format
  refreshCache(t);
  if(_tm.tm_hour == 0 )
    return 12; // 12 midnight
  else if(_tm.tm_hour  > 12)
    return _tm.tm_hour - 12 ;
  else
    return _tm.tm_hour;
}

uint8_t TimeClass::isAM() { // returns true if time now is AM
  return !isPM(now()); 
}

uint8_t TimeClass::isAM(time_t t) { // returns true if given time is AM
  return !isPM(t);  
}

uint8_t TimeClass::isPM() { // returns true if PM
  return isPM(now()); 
}

uint8_t TimeClass::isPM(time_t t) { // returns true if PM
  return (hour(t) >= 12); 
}

int TimeClass::minute() {
  return minute(now()); 
}

int TimeClass::minute(time_t t) { // the minute for the given time
  refreshCache(t);
  return _tm.tm_min;  
}

int TimeClass::second() {
  return second(now()); 
}

int TimeClass::second(time_t t) {  // the second for the given time
  refreshCache(t);
  return _tm.tm_sec;
}

int TimeClass::day(){
  return(day(now())); 
}

int TimeClass::day(time_t t) { // the day for the given time (0-6)
  refreshCache(t);
  return _tm.tm_mday;
}

int TimeClass::weekday() {   // Sunday is day 1
  return  weekday(now()); 
}

int TimeClass::weekday(time_t t) {
  refreshCache(t);
  return _tm.tm_wday;
}
   
int TimeClass::month(){
  return month(now()); 
}

int TimeClass::month(time_t t) {  // the month for the given time
  refreshCache(t);
  return _tm.tm_mon;
}

int TimeClass::year() {  // as in Processing, the full four digit year: (2009, 2010 etc) 
  return year(now()); 
}

int TimeClass::year(time_t t) { // the year for the given time
  refreshCache(t);
  return tmYearToCalendar(_tm.tm_year);
}

/*============================================================================*/	
/* functions to convert to and from system time */
/* These are for interfacing with time services and are not normally needed in a sketch */

// leap year calculator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)     ( ((1970+(Y))>0) && !((1970+(Y))%4) && ( ((1970+(Y))%100) || !((1970+(Y))%400) ) )

static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0
 
void TimeClass::breakTime(time_t timeInput, tm &tme){
// break the given time_t into time components
// this is a more compact version of the C library localtime function
// note that year is offset from 1970 !!!

  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time;
  unsigned long days;

  time = (uint32_t)timeInput;
  tme.tm_sec = time % 60;
  time /= 60; // now it is minutes
  tme.tm_min = time % 60;
  time /= 60; // now it is hours
  tme.tm_hour = time % 24;
  time /= 24; // now it is days
  tme.tm_wday = ((time + 4) % 7) + 1;  // Sunday is day 1 
  
  year = 0;  
  days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  tme.tm_year = year; // year is offset from 1970 
  
  days -= LEAP_YEAR(year) ? 366 : 365;
  time -= days; // now it is days in this year, starting at 0
  
  days=0;
  month=0;
  monthLength=0;
  for (month=0; month<12; month++) {
    if (month==1) { // february
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = monthDays[month];
    }
    
    if (time >= monthLength) {
      time -= monthLength;
    } else {
        break;
    }
  }
  tme.tm_mon = month + 1;  // jan is month 1  
  tme.tm_mday = time + 1;     // day of month
}

time_t TimeClass::makeTime(const tm &tme){   
// assemble time elements into time_t 
// note year argument is offset from 1970 (see macros in time.h to convert to other formats)
// previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9
  
  int i;
  uint32_t seconds;

  // seconds from 1970 till 1 jan 00:00:00 of the given year
  seconds= tme.tm_year*(SECS_PER_DAY * 365);
  for (i = 0; i < tme.tm_year; i++) {
    if (LEAP_YEAR(i)) {
      seconds += SECS_PER_DAY;   // add extra days for leap years
    }
  }
  
  // add days for this year, months start from 1
  for (i = 1; i < tme.tm_mon; i++) {
    if ( (i == 2) && LEAP_YEAR(tme.tm_year)) { 
      seconds += SECS_PER_DAY * 29;
    } else {
      seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
    }
  }
  seconds+= (tme.tm_mday-1) * SECS_PER_DAY;
  seconds+= tme.tm_hour * SECS_PER_HOUR;
  seconds+= tme.tm_min * SECS_PER_MIN;
  seconds+= tme.tm_sec;
  return (time_t)seconds; 
}
/*=====================================================*/	
/* Low level system time functions  */

static uint32_t sysTime = 0;
static uint32_t prevMillis = 0;
static uint32_t nextSyncTime = 0;
static timeStatus_t Status = timeNotSet;

getExternalTime getTimePtr;  // pointer to external sync function
//setExternalTime setTimePtr; // not used in this version

#ifdef TIME_DRIFT_INFO   // define this to get drift data
time_t sysUnsyncedTime = 0; // the time sysTime unadjusted by sync  
#endif


time_t TimeClass::now() {
	// calculate number of seconds passed since last call to now()
  while (millis() - prevMillis >= 1000) {
		// millis() and prevMillis are both unsigned ints thus the subtraction will always be the absolute value of the difference
    sysTime++;
    prevMillis += 1000;	
#ifdef TIME_DRIFT_INFO
    sysUnsyncedTime++; // this can be compared to the synced time to measure long term drift     
#endif
  }
  if (nextSyncTime <= sysTime) {
    if (getTimePtr != 0) {
      time_t t = getTimePtr();
      if (t != 0) {
        setTime(t);
      } else {
        nextSyncTime = sysTime + syncInterval;
        Status = (Status == timeNotSet) ?  timeNotSet : timeNeedsSync;
      }
    }
  }  
  return (time_t)sysTime;
}

void TimeClass::setTime(time_t t) { 
#ifdef TIME_DRIFT_INFO
 if(sysUnsyncedTime == 0) 
   sysUnsyncedTime = t;   // store the time of the first call to set a valid Time   
#endif

  sysTime = (uint32_t)t;  
  nextSyncTime = (uint32_t)t + syncInterval;
  Status = timeSet;
  prevMillis = millis();  // restart counting from now (thanks to Korman for this fix)
} 

void TimeClass::setTime(int hr,int min,int sec,int dy, int mnth, int yr){
 // year can be given as full four digit year or two digts (2010 or 10 for 2010);  
 //it is converted to years since 1970
  if( yr > 99)
      yr = yr - 1970;
  else
      yr += 30;

  _tm.tm_year = yr;
  _tm.tm_mon = mnth;
  _tm.tm_mday = dy;
  _tm.tm_hour = hr;
  _tm.tm_min = min;
  _tm.tm_sec = sec;
  setTime(makeTime(_tm));
}

void TimeClass::adjustTime(long adjustment) {
  sysTime += adjustment;
}

// indicates if time has been set and recently synchronized
timeStatus_t TimeClass::timeStatus() {
  now(); // required to actually update the status
  return Status;
}

void TimeClass::setSyncProvider( getExternalTime getTimeFunction){
  getTimePtr = getTimeFunction;  
  nextSyncTime = sysTime;
  now(); // this will sync the clock
}

void TimeClass::setSyncInterval(time_t interval){ // set the number of seconds between re-sync
  syncInterval = (uint32_t)interval;
  nextSyncTime = sysTime + syncInterval;
}

struct tm calendar_time_cache;	// a cache of calendar time structure elements
time_t unix_time_cache;  		// a cache of unix_time that was updated
time_t time_zone_cache;			// a cache of the time zone that was set
time_t dst_cache = 3600;        // a cache of the DST offset that was set (default 1hr)
time_t dst_current_cache = 0;   // a cache of the DST offset currently being applied

/* return string representation for the given time */
const char* TimeClass::timeStr(time_t t)
{
    t += time_zone_cache;
    t += dst_current_cache;
    struct tm calendar_time = {};
    localtime_r(&t, &calendar_time);
    char ascstr[26] = {};
    asctime_r(&calendar_time, ascstr);
    int len = strlen(ascstr);
    ascstr[len-1] = 0; // remove final newline
    return String(ascstr).c_str();
}

const char* TimeClass::format_spec = TIME_FORMAT_DEFAULT;
struct tm TimeClass::_tm;

const char* TimeClass::format(time_t t, const char* format_spec)
{
    if (format_spec == nullptr)
        format_spec = this->format_spec;

    if (!format_spec || !strcmp(format_spec, TIME_FORMAT_DEFAULT)) {
        return timeStr(t);
    }
    t += time_zone_cache;
    t += dst_current_cache;
    struct tm calendar_time = {};
    localtime_r(&t, &calendar_time);
    return timeFormatImpl(&calendar_time, format_spec, time_zone_cache + dst_current_cache);
}

#ifndef t_abs
#define t_abs(x) ((x)>0?(x):-(x))
#endif

const char* TimeClass::timeFormatImpl(tm* calendar_time, const char* format, int time_zone)
{
    char format_str[64];
    // only copy up to n-1 to dest if no null terminator found
    strncpy(format_str, format, sizeof(format_str) - 1); // Flawfinder: ignore (ch42318)
    format_str[sizeof(format_str) - 1] = '\0'; // ensure null termination
    size_t len = strlen(format_str); // Flawfinder: ignore (ch42318)

    char time_zone_str[16];
    // while we are not using stdlib for managing the timezone, we have to do this manually
    if (!time_zone) {
        strcpy(time_zone_str, "Z");
    }
    else {
        snprintf(time_zone_str, sizeof(time_zone_str), "%+03d:%02u", time_zone/3600, t_abs(time_zone/60)%60);
    }

    // replace %z with the timezone
    for (size_t i=0; i<len-1; i++)
    {
        if (format_str[i]=='%' && format_str[i+1]=='z')
        {
            size_t tzlen = strlen(time_zone_str);
            memcpy(format_str+i+tzlen, format_str+i+2, len-i-1);    // +1 include the 0 char
            memcpy(format_str+i, time_zone_str, tzlen);
            len = strlen(format_str);
        }
    }

    char buf[50] = {};
    strftime(buf, sizeof(buf), format_str, calendar_time);
    return String(buf).c_str();
}

