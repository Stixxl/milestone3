volatile bool inFlag=false, outFlag=false;
void out(){
  if (inFlag) {
    count--;
    inFlag=false;
  } else {
    outFlag=true;
  }
}

void in(){
  if (outFlag) {
    count++;
    outFlag=false;
  } else {
    inFlag=true;
  }
}
