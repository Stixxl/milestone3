void insertTask(uint32_t time, uint8_t cmd, uint8_t param){
  if (taskCounter == capacity){
    Serial.print("Sorry, the Tasklist reached it's maxmimum limit of ");
    Serial.print(capacity, DEC);
    Serial.println(" items. The task will not be processed");
    }
   else{
      if (taskCounter == 0){
              taskList[0].TimeStamp = time;
              taskList[0].Command = cmd;
              taskList[0].Parameter = param;
        }
       else{
        for (unsigned i = taskCounter ; i-- > 0 ; ) // start mit index i = 0
        {
          if (taskList[i].TimeStamp < time){
            taskList[i+1].TimeStamp = time;
            taskList[i+1].Command = cmd;
            taskList[i+1].Parameter = param;
            }
          else{
            taskList[i+1] = taskList[i];
            if (i == 0){
              taskList[i].TimeStamp = time;
              taskList[i].Command = cmd;
              taskList[i].Parameter = param;
            }
          }
        }
      }
    
      
      taskCounter++;
   }   
}

void printTasklist(){
  Serial.print("Task list contains ");
  Serial.print(taskCounter, DEC);
  Serial.println(" items");
  Serial.println(" ");

  if (taskCounter!=0){
      for (int i = 0; i < taskCounter; i++) {
        Serial.print("Task ");
        Serial.print(i, DEC);
        Serial.print(" TS: ");
        Serial.print(taskList[i].TimeStamp, DEC);
        Serial.print(" COM: ");
        Serial.print(taskList[i].Command, DEC);
        Serial.print(" ARG: ");
        Serial.println(taskList[i].Parameter, DEC);
      }
    }
}

void runAvailTasks(){
  int executedTasks = 0;
  uint32_t currentTime = rtc.getEpoch(); //GetCurrentTime
  int ctm = currentTime % 100;

  if (ctm == 0){
    Serial.print("UNIX TIME ");
    Serial.println(currentTime, DEC);
  }

  for (int i = 0; i < taskCounter; i++) {
    if (taskList[i].TimeStamp <= currentTime){
      Serial.print("Running Task ");
      Serial.print(taskList[i].Command, DEC);
      Serial.print(" With Param ");
      Serial.println(taskList[i].Parameter, DEC);
      runTaskControlled(taskList[i].Command, taskList[i].Parameter, false);
      executedTasks++;
    }
    else{
      break;
    }
  }

  if (executedTasks > 0){
    //Serial.println("None of that");
    for (int i = 0; i <= taskCounter-executedTasks; i++) {
      taskList[i] = taskList[i+executedTasks];
      }
    taskCounter= taskCounter - executedTasks;
  }
  else
  {
    if (ctm == 0){
    //Serial.println("There are no Tasks to process right now.");
    }
  }
}
