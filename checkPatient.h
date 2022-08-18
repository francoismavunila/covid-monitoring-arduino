uint8_t getFingerID() {
  Serial.println("Driver Place Your FingerPrint");

  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return 150;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("done");

  return finger.fingerID;
}
uint8_t checkPatient() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("place finger on");
    lcd.setCursor(0, 1);
    lcd.print("scanner");
  uint8_t flag=getFingerID();
  while(flag==255){
    flag =getFingerID();
    delay(50);   
  }
 return flag;

}
