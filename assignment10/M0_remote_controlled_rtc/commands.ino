void leaveRoom(uint8_t param){
    Serial.print("Leave ");Serial.println(param);
    while (param!=0){
      digitalWrite(triggerIn,HIGH);
      mydelay(50);
      digitalWrite(triggerIn,LOW);
      mydelay(50);
      digitalWrite(triggerOut,HIGH);
      mydelay(50);
      digitalWrite(triggerOut,LOW);
      mydelay(500);
      param--;
    }
}

void enterRoom(uint8_t param){
    Serial.print("Enter ");Serial.println(param);
    
    while (param!=0){
      digitalWrite(triggerOut,HIGH);
      mydelay(50);
      digitalWrite(triggerOut,LOW);
      mydelay(50);
      digitalWrite(triggerIn,HIGH);
      mydelay(50);
      digitalWrite(triggerIn,LOW);
      mydelay(500);
      param--;
    }
}

void peekIntoRoom(uint8_t param){
    Serial.print("Peek Into ");Serial.println(param);
    
    while (param!=0){
      digitalWrite(triggerOut,HIGH);
      mydelay(50);
      digitalWrite(triggerOut,LOW);
      mydelay(50);
      param--;
    }
}


void mydelay(int ms) {
  long var;
  volatile float temp;
  temp = 1000000.0;

  for (var = 1; var < ms * 300; var++) temp = temp / 2;
}

void showRoomState() {

  display.clearDisplay();

  display.setTextSize(2);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font


  display.clearDisplay();
  display.setCursor(0, 0);     // Start at top-left corner
  display.setTextSize(2);      // Normal 1:1 pixel scale
  display.write("G10  ");
  uint8_t day,month,year,hours,minutes,seconds;
  getTimeInCalender(day,month,year,hours,minutes,seconds);
  printTimeOnDisplay(getTimeInSeconds(),false);
  display.write("\n");
 
  display.setCursor(0, 30);     // Start at top-left corner
  display.setTextSize(4);      // Normal 1:1 pixel scale
  
  if (count<10) display.write(" ");
  display.print(count);
  display.print(" ");


  if (prediction<10) display.write(" ");
  display.print(prediction);
  display.display();

}

void resetCount(){
  count=0;
}

void setShowRoomPeriod(uint8_t param){
  showPeriod=param;
}

void setMsgPeriod(uint8_t param){
  msgPeriod=param;
}

void setTimewarpFactor(uint8_t param){
  setTime(getTimeInSeconds());
  timewarpFactor=param;
}

void setPrediction(uint8_t param){
  prediction=param;
}

void runTaskControlled(uint8_t cmd, uint8_t param, boolean repeat){
  //Serial.println(cmd);
  if (cmd==1) enterRoom(param);
  if (cmd==2) leaveRoom(param);
  if (cmd==3) peekIntoRoom(param);

  if (cmd==11) {
    showRoomState();
    if(param) insertTask(getTimeInSeconds()+showPeriod, cmd, 1);
  }
  if (cmd==12) {
    do_send(&sendjob);
    if(param) insertTask(getTimeInSeconds()+msgPeriod,cmd,1);
  }
  if (cmd==13) {
    resetCount();
    if(param) insertTask(getTimeInSeconds()+resetPeriod, cmd, 1);
  }
  if (cmd==14) setPrediction(param);

  
  if (cmd==21) setShowRoomPeriod(param);
  if (cmd==22) setMsgPeriod(param);
  if (cmd==23) setTimewarpFactor(param);

  if (cmd==31) printTasklist();
  if (cmd==32) printCurrentTime();

  if (cmd==100) peekOutOfRoom(param);
  if (cmd==101) halfEnter(param);
  if (cmd==102) halfLeave(param);
  if (cmd==103) almostEnter(param); 
  if (cmd==104) almostLeave(param); 
  if (cmd==105) unsureEnter(param);
  if (cmd==106) unsureExit(param); 
  if (cmd==107) manipulationEnter(param);
  if (cmd==108) manipulationLeave(param);
  if (cmd==109) obstructionInside(param);
  if (cmd==110) obstructionOutside(param);
  if (cmd==111) pickUpFriend(param);

}

void runTaskOnce(uint8_t cmd, uint8_t param){
  runTaskControlled(cmd,param,0);
}

void runTaskRepeated(uint8_t cmd, uint8_t param){
  runTaskControlled(cmd,param,1);
}

//CORNER CASES
// Someone briefly looks out of the room and turns back
void peekOutOfRoom(uint8_t param) {
    Serial.print("Peak out ");Serial.println(param);

    for (; param > 0; param--) {
        digitalWrite(triggerIn, HIGH);
        mydelay(50);
        digitalWrite(triggerIn, LOW);
        mydelay(500);
    }
}

// Someone walks up to the barriers from outside and turns around
void halfEnter(uint8_t param) {
    Serial.print("Half Enter ");Serial.println(param);
    for (; param > 0; param--) {
        digitalWrite(triggerOut, HIGH);
        mydelay(50);
        digitalWrite(triggerIn, HIGH);
        mydelay(50);
        digitalWrite(triggerIn, LOW);
        mydelay(500);
        digitalWrite(triggerOut, LOW);
    }
}

// Someone walks up to the barriers from inside and turns around
void halfLeave(uint8_t param) {
    Serial.print("Half Leave ");Serial.println(param);
    for (; param > 0; param--) {
        digitalWrite(triggerIn, HIGH);
        mydelay(50);
        digitalWrite(triggerOut, HIGH);
        mydelay(50);
        digitalWrite(triggerOut, LOW);
        mydelay(50);
        digitalWrite(triggerIn, LOW);
        mydelay(500);
    }
}

// Someone is almost inside the room but turns back
void almostEnter(uint8_t param) {
    Serial.print("Almost Enter ");Serial.println(param);
    for (; param > 0; param--) {
        digitalWrite(triggerOut, HIGH);
        mydelay(50);
        digitalWrite(triggerIn, HIGH);
        mydelay(50);
        digitalWrite(triggerOut, LOW);
        mydelay(50);
        digitalWrite(triggerOut, HIGH);
        mydelay(50);
        digitalWrite(triggerIn, LOW);
        mydelay(50);
        digitalWrite(triggerOut, LOW);
        mydelay(500);
    }
}

// Someone almost leaves the room completely but turns back
void almostLeave(uint8_t param) {
     Serial.print("Almost Leave ");Serial.println(param);

    for (; param > 0; param--) {
        digitalWrite(triggerIn, HIGH);
        mydelay(50);
        digitalWrite(triggerOut, HIGH);
        mydelay(50);
        digitalWrite(triggerIn, LOW);
        mydelay(50);
        digitalWrite(triggerIn, HIGH);
        mydelay(50);
        digitalWrite(triggerOut, LOW);
        mydelay(50);
        digitalWrite(triggerIn, LOW);
        mydelay(500);
    }
}

// Someone almost enters, steps back, but then enters
void unsureEnter(uint8_t param) {
    Serial.print("Unsure Enter ");Serial.println(param);

    for (; param > 0; param--) {
        digitalWrite(triggerOut, HIGH);
        mydelay(50);
        digitalWrite(triggerIn, HIGH);
        mydelay(50);
        digitalWrite(triggerOut, LOW);
        mydelay(50);
        digitalWrite(triggerOut, HIGH);
        mydelay(50);
        digitalWrite(triggerIn, LOW);
        mydelay(50);
        digitalWrite(triggerIn, HIGH);
        mydelay(50);
        digitalWrite(triggerOut, LOW);
        mydelay(50);
        digitalWrite(triggerIn, LOW);
        mydelay(50);
    }
}

// Someone almost leaves the room completely but turns back
void unsureExit(uint8_t param) {
    Serial.print("Unsure Exit ");Serial.println(param);

    for (; param > 0; param--) {
        digitalWrite(triggerIn, HIGH);
        mydelay(50);
        digitalWrite(triggerOut, HIGH);
        mydelay(50);
        digitalWrite(triggerIn, LOW);
        mydelay(50);
        digitalWrite(triggerIn, HIGH);
        mydelay(50);
        digitalWrite(triggerOut, LOW);
        mydelay(50);
        digitalWrite(triggerOut, HIGH);
        mydelay(50);
        digitalWrite(triggerIn, LOW);
        mydelay(50);
        digitalWrite(triggerOut, LOW);
        mydelay(50);
    }
}


//Someone is trying to manipulate the count by waving their arm through the barrier towards the inside
//Sequence is not possible if a person enters
void manipulationEnter(uint8_t param){ 
    Serial.print("Manipulation Enter ");Serial.println(param);
    while (param!=0){
      digitalWrite(triggerOut,HIGH);
      mydelay(15);
      digitalWrite(triggerOut,LOW);
      mydelay(15);
      digitalWrite(triggerIn,HIGH);
      mydelay(15);
      digitalWrite(triggerIn,LOW);
      mydelay(500);
      param--;
    }
}

//Someone is trying to manipulate the count by waving their arm through the barrier towards the outside
void manipulationLeave(uint8_t param){   
    Serial.print("Manipulation Leave ");Serial.println(param);
    while (param!=0){
      digitalWrite(triggerIn,HIGH);
      mydelay(15);
      digitalWrite(triggerIn,LOW);
      mydelay(15);
      digitalWrite(triggerOut,HIGH);
      mydelay(15);
      digitalWrite(triggerOut,LOW);
      mydelay(500);
      param--;
    }
}

//Someone is standing in the inside barrier, making counting impossible
void obstructionInside(uint8_t param){ 
  Serial.println("Obstruction Inside");
  digitalWrite(triggerIn,HIGH);
  mydelay(6000);
  digitalWrite(triggerOut,HIGH);
  mydelay(3000);
  // Resolve obstruction
  digitalWrite(triggerIn,LOW);
  digitalWrite(triggerOut,LOW);
  Serial.println("Obstruction inside resolved");
  mydelay(500);
}

//Someone is standing in the outside barrier, making counting impossible
void obstructionOutside(uint8_t param){
  Serial.println("Obstruction Outside");
  digitalWrite(triggerOut,HIGH);
  mydelay(6000);
  digitalWrite(triggerIn,HIGH);
  mydelay(3000);
  // Resolve obstruction
  digitalWrite(triggerOut,LOW);
  digitalWrite(triggerIn,LOW);
  Serial.println("Obstruction outside resolved");
  mydelay(500);
}

//Someone is peeking in and then leaving with another person coming from inside
void pickUpFriend(uint8_t param){
    Serial.print("Pick up Friend ");Serial.println(param);
    while (param!=0){
      digitalWrite(triggerOut,HIGH);
      mydelay(3000);
      digitalWrite(triggerIn,HIGH);
      mydelay(50);
      digitalWrite(triggerOut,LOW);
      mydelay(50);
      digitalWrite(triggerIn,LOW);
      mydelay(50);
      digitalWrite(triggerOut,HIGH);
      mydelay(50);
      digitalWrite(triggerOut,LOW);
      mydelay(500);
      param--;
    }
}
