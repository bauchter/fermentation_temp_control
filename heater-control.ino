// -----------------------------------------
// Function and Variable with Thermistor
// -----------------------------------------
// *** Previously Known as: Function and Variable with Photoresistors Example ***
//
// In this fork, of the above, I register a Particle.variable() with the cloud so that I can read 
// ADC temperature levels from the thermistor.
// I also register a Particle.function so that we can turn the heater on and off remotely.
// Note:  This is a Walgreens special thermal blanket.  One Pulse of the Output (Off-On-Off) results in Low.
//        Another pulse results in "Medium", the third pulse results in "High", and the final pulse results 
//        in "off".  If left alone, the heater stays on for about 2 hours (two hour shutoff)

#define TEMPSETPOINT 2233 //This is ~68.5F
//2236 is ~69F
//2246 is ~70F
#define NUMSAMPLES 5 //number of samples to read and average

// We're going to start by declaring which pins everything is plugged into.

int heater = D6; // This is where your Heater Relay Control is plugged in. The relay connects to a Normally Open (NO) pushbutton
//              The relay closes on "High"

int thermistor = A0; // This is where your 60 Ohm @25C thermistor is plugged in. 
//  This is a resistor divider circuit, with 73 ohm resistor between thermistor and GND, and 3.3VDC tied to the thermistor. 
//  Thus, the range for the thermistor stays about in the middle of the ADC range (0-to-4095)

int analogvalue; // Here we are declaring the variable analogvalue, which we will store the avg value of the thermistor.
int samples[NUMSAMPLES]; //array for storing all analog input samples (thermistor)
int heatersetting; // values 0 to 3, Off..Low..Med..High
int lowcount; // number of sampled cycles that the temperature was lower than setpoint

// Next we go into the setup function.

void setup(void) {

    // First, declare all of our pins. This lets our device know which ones will be used for outputting voltage, 
    //  and which ones will read incoming voltage.
    pinMode(heater,OUTPUT); // Our Heater pin is output (pulsing the Heater)
    pinMode(thermistor,INPUT);  // Our thermistor pin is input (reading the thermistor)

    // We are going to declare Particle.variable() here so that we can access the value of the photoresistor from the cloud.
    Particle.variable("analogvalue", &analogvalue, INT);
    // This is saying that when we ask the cloud for "analogvalue", this will reference the variable analogvalue in this app,
    //  which is an integer variable.

    // We are also going to declare a Particle.function so that we can turn the LED on and off from the cloud.
    Particle.function("heater",heaterPulse);
    // This is saying that when we ask the cloud for the function "heater", it will employ the function heaterPulse() 
    //  from this app.

    heatersetting = 0;
    lowcount = 0;
}


// Next is the loop function...

void loop(void) {
    uint8_t i;
    float average;

    // check to see what the value of the thermistor is and store it in the int variable analogvalue
    // take N samples in a row, with a slight delay
    for (i=0; i< NUMSAMPLES; i++) {
        samples[i] = analogRead(thermistor);
        delay(10);
    }
 
    // average all the samples out
    average = 0;
    for (i=0; i< NUMSAMPLES; i++) {
        average += samples[i];
    }
    average /= NUMSAMPLES;

    analogvalue = int(average);

    // check analogvalues.  70.0F is ~2247 on the ADC
    if (analogvalue < TEMPSETPOINT){
        if (lowcount < 3){
            //if below 70.0 and lowcount < 2 , set heater to low; increment lowcount; heatersetting=1
            //set heater to low (assume off state)
            incrementHeater();
            heatersetting = 1; // set to low
            lowcount +=1; // increment by 1
        }
        else if (lowcount < 5){
            //elseif below 70.0 and lowcount < 3 , set heater to medium; increment lowcount; heatersetting=2
            incrementHeater();
            incrementHeater(); // set to medium
            heatersetting = 2; // set to Med
            lowcount +=1; // increment by 1
        }
        else {
            //elseif below 70.0 and lowcount > 3 , set heater to high ; increment lowcount; heatersetting=3
            incrementHeater();
            incrementHeater();
            incrementHeater(); // set to high
            heatersetting = 3; // set to High
            lowcount +=1; // increment by 1
        }
    }
    else {  //else, temp is above 70.0, leave heater off ; heatersetting=0
        heatersetting = 0;
    }

    //delay loop (15 minutes in 60 second delay loops?)
    for (int x=0; x<15; x++){ //15 minutes
        for (int y=0; y<60; y++){ // 60 seconds
            delay(1000); //wait 1 second
        }
    }

    if (heatersetting > 0){ // reset to off for next cycle
        for (int z=0; z < (4-heatersetting); z++){
            incrementHeater();
        }
    } 

    heatersetting=0;

    //for good measure, set digitalWrite(heater,LOW);
    digitalWrite(heater,LOW);  // no need to keep the relay coil activated...
}


// Finally, we will write out our heaterPulse function, which is referenced by the Particle.function() called "heater"

int heaterPulse(String command) {

    if (command=="on") {
        digitalWrite(heater,HIGH);
        delay(1000); // wait 1 second
        digitalWrite(heater,LOW);
        delay(1000); // wait 1 second
        return 1;
    }
    else if (command=="off") {
        digitalWrite(heater,LOW);
        delay(1000); // wait 1 second
        return 0;
    }
    else {
        return -1;
    }

}


void incrementHeater(){
    digitalWrite(heater,HIGH);
    delay(1000); // wait 1 second
    digitalWrite(heater,LOW);
    delay(1000); // wait 1 second
}



