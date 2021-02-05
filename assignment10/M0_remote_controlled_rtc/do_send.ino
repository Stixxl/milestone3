void do_send(osjob_t* j) {

#ifndef WITH_LORA
  uint32_t time=getTimeInSeconds();
  Serial.print("<<SEND:"); Serial.print(time);Serial.print(":");Serial.print(count);Serial.println(":>>");
#endif

#ifdef WITH_LORA
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
  } else {
    // Prepare upstream data transmission at the next possible time.

    float measuredvbat = analogRead(A7);
    measuredvbat *= 2;    // we divided by 2, so multiply back
    measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
    measuredvbat /= 1024; // convert to voltage

    char buffer[8];
    Serial.print("VBat: " ); Serial.print(measuredvbat); Serial.print(" Count: "); Serial.println(count);
    int16_t volt = measuredvbat * 100; // convert to signed 16 bits integer: 0x0929
    uint8_t buffer1[4];
    buffer1[0] = volt >> 8;
    buffer1[1] = volt;
    buffer1[2] = count >> 8;
    buffer1[3] = count;
    LMIC_setTxData2(1, buffer1, sizeof(buffer1), 0);
  }
#endif
}
