unsigned long time_read_button = millis();
enum States {Init, Waiting, Draining2}; // Init: initial state; Waiting: waiting mode; Draining2: automatic draining mode
States state = Init; // initialize the state
int record_pressure; 
int current_pressure; 

// input pins
const int button = 2; // read signal from the button
const int moisture = A4; // read signal from the moisture sensor
const int pressure = A5; // read signal from the water pressure sensor

// output pins
const int valve = 4; // output for the "transistor" to open or close the "valve"
const int vcc_button = 8; // power supply to the button "LED"
const int vcc_moisture = 12; // power supply to the moisture sensor
const int vcc_pressure = 13; // power supply to the water pressure sensor

void setup()
{
  // input
  pinMode(button, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(button), manualDrain, CHANGE); // when the button changes, go check manual draining mode
  // output
  pinMode(valve, OUTPUT);
  digitalWrite(valve, LOW); // close valve
  pinMode(vcc_button, OUTPUT);
  digitalWrite(vcc_button, LOW); // turn off button LED
  pinMode(vcc_moisture, OUTPUT);
  digitalWrite(vcc_moisture, HIGH); // turn on the moisture sensor
  pinMode(vcc_pressure, OUTPUT);
  digitalWrite(vcc_pressure, HIGH); // turn on the water pressure sensor
}

void loop()
{
  
  switch (state) {

    case Init:
      // Get the record pressure
      record_pressure =  averagePressureReading(); 
      state = Waiting; // go to "waiting" state
      break;

    case Waiting:
      // Get the current pressure
      current_pressure = averagePressureReading();
      
      // if the pressure is not increasing and the pressure is not low and the soild moisture is low, go to automatic draining mode
      // The -2 is there to provide "hysterisis".  I.e. the pressure can fluctuate up to 2 units higher without
      // stopping the valve from opening. But if the pressure increases more than 2 units per 10 seconds the valve will
      // not open becaue the statement will be false.  Each unit is about .024psi or .6 inH2O ((5-0V)/(1024-0units)*(5-0psi)/(4.5-0.5V)).
      if ( (current_pressure-2) <= (record_pressure) && current_pressure >= 150 &&  analogRead(moisture) < 600 ) {
        state = Draining2;
      }
      
      // Set the recorded pressure to the current pressure if not going to the automatic draining mode
      record_pressure = current_pressure;
      break;

    // automatic draining mode
    case Draining2: 
      // Open the valve until there is about 4-5in of water in the barrel. This corresponds to about 120-130 units. Or open the valve until the soil is wet enough.
      while (averagePressureReading() >= 130 && analogRead(moisture) < 600) {
        digitalWrite(valve, HIGH);
        digitalWrite(vcc_button, HIGH); // turn on the button LED simultaneously to indicate the valve is open
      }
      digitalWrite(valve, LOW);
      digitalWrite(vcc_button, LOW);
      state = Init; // go back to initial state
      break;
  }
}

int averagePressureReading () {
  //Take 50 readings .1 seconds apart and average them, then return the average
  float averageVal = 0;
  for (int k = 1; k <= 50; k++) {
    //  Update the average with a new reading
    averageVal = ((averageVal * (k - 1)) + analogRead(pressure)) / k;
    delay(100);
  }
  // return the average value.
  return  (int)averageVal;
}

// manual draining mode
void manualDrain() { 
  if (digitalRead(button) == HIGH) { 
    // prevent debounce
    if ( (millis() - time_read_button) > 100) {

      while (digitalRead(button) == HIGH) { // when the button is pressing.. open the valve and turn on the LED
        digitalWrite(valve, HIGH); 
        digitalWrite(vcc_button, HIGH);
      }
      digitalWrite(valve, LOW);
      digitalWrite(vcc_button, LOW);
      // Update the time
      time_read_button = millis();
    }
  }
  state = Init; // go back to initial state
}
