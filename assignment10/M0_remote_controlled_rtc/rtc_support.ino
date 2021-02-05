void printTimeOnDisplay(uint32_t time,bool withSeconds){
    uint8_t day,month,year,hours,minutes,seconds;
    convertFromSeconds(time,day,month,year,hours,minutes,seconds);
    
    if (hours<10) display.write("0");
    display.print(hours);
    display.write(":");
    if (minutes<10) display.write("0");
    display.print(minutes);
    if (withSeconds){
      display.write(":");
      if (seconds<10) display.write("0");
      display.print(seconds);
    }
}

void printTime(uint32_t time) {
  uint8_t year, month, day, hours, minutes, seconds;
  convertFromSeconds(time,day,month,year,hours,minutes,seconds);

  Serial.print("Time: ");
  // Print date...
  print2digits(day);
  Serial.print("/");
  print2digits(month);
  Serial.print("/");
  print2digits(year);
  Serial.print(" ");

  // ...and time
  print2digits(hours);
  Serial.print(":");
  print2digits(minutes);
  Serial.print(":");
  print2digits(seconds);
  Serial.print("  ");

}

void printCurrentTime() {
  uint32_t now=getTimeInSeconds();
  now=applyTimeWarping(now);
  printTime(now);

}

void print2digits(int number) {
  if (number < 10) {
    Serial.print("0"); // print a 0 before if the number is < than 10
  }
  Serial.print(number);
}


uint32_t getTimeInSeconds() {
  uint32_t time;
  uint8_t year, month, day, hours, minutes, seconds;

  
  year=rtc.getYear();
  month = rtc.getMonth();
  day = rtc.getDay();
  hours = rtc.getHours();
  minutes = rtc.getMinutes();
  seconds = rtc.getSeconds();

  time=convertToSeconds(day,month,year,hours,minutes,seconds);
  time=applyTimeWarping(time);
  return time;
}

void getTimeInCalender(uint8_t &day, uint8_t &month, uint8_t &year, uint8_t &hours, uint8_t  &minutes, uint8_t &seconds) {
  uint32_t time=getTimeInSeconds();
  convertFromSeconds(time,day,month,year,hours,minutes,seconds);

}

void setAlarmFromSeconds(uint32_t time) {
  uint8_t year, month, day, hours, minutes, seconds;
  convertFromSeconds(time,day,month,year,hours,minutes,seconds);

  rtc.setAlarmSeconds(seconds);
  rtc.setAlarmMinutes(minutes);
  rtc.setAlarmHours(hours);
  rtc.setAlarmDay(day);
  rtc.setAlarmMonth(month);
  rtc.setAlarmYear(year);
  
  nextAlarm=time;
}

void setTime(uint32_t time){
  uint8_t year, month, day, hours, minutes, seconds;
  initialTime=time;

  convertFromSeconds(time,day,month,year,hours,minutes,seconds);
  
  // Set the time
  rtc.setHours(hours);
  rtc.setMinutes(minutes);
  rtc.setSeconds(seconds);

  // Set the date
  rtc.setDay(day);
  rtc.setMonth(month);
  rtc.setYear(year);

}

uint32_t convertToSeconds(uint8_t day, uint8_t month, uint8_t year, uint8_t hours, uint8_t  minutes, uint8_t seconds){
  uint32_t tmp;
  time_t time;
  struct tm calTime;
  calTime.tm_year=year+100;
  calTime.tm_mon=month-1;
  calTime.tm_mday=day;
  calTime.tm_hour=hours;
  calTime.tm_min=minutes; 
  calTime.tm_sec=seconds; 

  time=mktime(&calTime);
  tmp=time;
  return tmp; 
}

uint32_t applyTimeWarping(uint32_t time){
  time = initialTime+timewarpFactor*(time-initialTime);
  return(time);
}

void convertFromSeconds(uint32_t time, uint8_t &day, uint8_t &month, uint8_t &year, uint8_t &hours, uint8_t  &minutes, uint8_t &seconds){

  time_t utcTime = time; 
  struct tm *calTime=localtime(&utcTime);
  
  year=calTime->tm_year-100;
  month=calTime->tm_mon+1;
  day=calTime->tm_mday;
  hours=calTime->tm_hour;
  minutes=calTime->tm_min;
  seconds=calTime->tm_sec;

}




uint32_t getInitialTime(){
  uint32_t time;
  
  /* Change these values to set the current initial time */
  const byte seconds = 0;
  const byte minutes = 0;
  const byte hours = 12;
  
  /* Change these values to set the current initial date */
  const byte day = 03;
  const byte month = 05;
  const byte year = 20;

  time=convertToSeconds(day,month,year,hours,minutes,seconds);
  return time;

}
