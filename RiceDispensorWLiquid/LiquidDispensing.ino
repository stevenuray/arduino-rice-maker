void openValve(){
  digitalWrite(relayPinRed, HIGH);
  digitalWrite(relayPinBlack, LOW);
}

void closeValve(){
  digitalWrite(relayPinRed, LOW);
  digitalWrite(relayPinBlack, LOW);
}

/*
Interrupt Service Routine
 */
void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}

void printLitresFlowed(){
    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");       
    Serial.print(totalMilliLitres);
    Serial.println("mL"); 
    Serial.print("\t"); 		  // Print tab space
	  Serial.print(totalMilliLitres/1000);
	  Serial.print("L");
	  Serial.print("\t");
	  Serial.print("Pulse Count: ");
	  Serial.println(pulseCount);
}

float calculateCycleMilliLitres(){
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    float flowRate = ((1000.0 / (millis() - lastFlowCalculatedMS)) * pulseCount) / calibrationFactor;
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    return (flowRate / 60) * 1000;
}

void calculateFlowMilliliters(){
    // Only process counters once per second
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(digitalPinToInterrupt(waterSensorInputPin));
        
    flowMilliLitres = calculateCycleMilliLitres();
    
    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;

    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    
    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    lastFlowCalculatedMS = millis();
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(digitalPinToInterrupt(waterSensorInputPin), pulseCounter, FALLING);
}

boolean isLiquidDispensorEnabled(){
  if(analogRead(liquidDispensorEnablePin) >= buttonPressThreshold){
    return true;
  } else {
    return false;
  }
}

void setupForValve(){
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  attachInterrupt(digitalPinToInterrupt(waterSensorInputPin), pulseCounter, FALLING);
  liquidDispenseStartTimeMS = millis();
  openValve();
}

//Special drain only mode for valve to empty tank for maintenance, must be put in setup/loop via code and arduino reprogrammed because this is anticipated to be a rare event
void drainMode(){
  openValve();
  delay(drainModeMillis);
  closeValve();
}

void loopForValve(){
  if((millis() - lastFlowCalculatedMS) > 1000) {
    calculateFlowMilliliters();
    printLitresFlowed();
  }
  
  if(isLiquidDispenseCycleCompleted()) {
    closeValve();
    Serial.println("Valve Closed");
    Serial.println("FINISHED");
  } else {
    openValve();
  }
}

boolean isLiquidDispenseCycleCompleted(){
  return (totalMilliLitres >= targetMilliLitresToDispense || millis() - liquidDispenseStartTimeMS >= maxLiquidDispenseMS);
}