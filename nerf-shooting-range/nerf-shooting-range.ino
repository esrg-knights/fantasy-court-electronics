/*
 * Fantasy Court - Nerf Target Practise
 * 
 * TODO: Maak een beschrijving van hoe het ongeveer werkt, en hoe je het configureert
 * 
 * 
 * Oude gekopieërde meuk:
 * 
  Input Pull-up Serial

  This example demonstrates the use of pinMode(INPUT_PULLUP). It reads a digital
  input on pin 2 and prints the results to the Serial Monitor.

  The circuit:
  - momentary switch attached from pin 2 to ground
  - built-in LED on pin 13

  Unlike pinMode(INPUT), there is no pull-down resistor necessary. An internal
  20K-ohm resistor is pulled to 5V. This configuration causes the input to read
  HIGH when the switch is open, and LOW when it is closed.

  created 14 Mar 2012
  by Scott Fitzgerald

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/InputPullupSerial
*/

const int startButtonPin = 15; // Start button (inverted)
const int panicButtonPin = 14; // Emergency Exit Button (inverted)

const bool invertStartButton = true; // Whether the start button signal becomes LOW upon press
const bool invertPanicButton = true; // idem

// Define targets
const int target1Pin = 2;
const int target2Pin = 3;
const int target3Pin = 4;
const int target4Pin = 5;

const int numTargets = 3;
const int targets[] = {target1Pin, target2Pin, target3Pin};

// Amount of times targets open up
const int sequenceLength = 4;

// Hardcoded sequence useful for testing
bool sequence[sequenceLength][numTargets] = {
  { false, false, false },
  { true, false, false },
  { false, true, true },
  { false, false, true },
};

// Current states
bool currentSequence[numTargets];

// Initial state for the sequence
int sequenceState = 0;
int currentSequenceStateDuration = 0; // The duration the current sequence state will last for before switching to the next
unsigned long lastDebounceTime = 0; // the time a sequence step was started

// Whether to use a randomized sequence (of length sequenceLength) instead of the hardcoded one
const bool useRandomizedSequence = true;

// Whether debug mode is enabled (prints to Serial)
const bool debugModeEnabled = true;

// Duration of a single sequence state
const int minSequenceDuration = 1000;
const int maxSequenceDuration = 3000;
//const int sequenceDuration = 1000; // Duration of a single sequence state

// Whether the sequence is currently playing
bool isRunning = false;
bool runIndicatorPin = 11; // Internal Light

const bool allowConsecutiveActivity = false; // Whether a target can be active for two consecutive sequences
const int minActiveTargetsPerSequence = 1; // Minimum number of active targets
const int maxActiveTargetsPerSequence = 4; // Maximum number of active targets

/*
 * Prints a string to the serial output, but only if
 * debug mode is enabled.
 */
void debugprintln(String s) {
  if (debugModeEnabled) {
    Serial.println(s);
  }
}

/*
 * Same as above, but without a newline instead
 */
void debugprint(String s) {
  if (debugModeEnabled) {
    Serial.print(s);
  }
}

void setup() {
  //start serial connection
  if (debugModeEnabled) {
    Serial.begin(9600);
    Serial.println("DEBUG MODE IS ENABLED");
    Serial.println("INITIALIZING...");
  }

  // Use a random seed
  randomSeed(42);
//  randomSeed(analogRead(0));
  
  // Configure start button and emergency stop as an input, and enable
  //  the internal pull-up resistor as they're buttons
  pinMode(startButtonPin, INPUT_PULLUP);
  pinMode(panicButtonPin, INPUT_PULLUP);

  // Run indicator
  pinMode(runIndicatorPin, OUTPUT);

  debugprint("Marking following pins as output: ");
  // Mark the pins of targets as output  
  for (int i = 0; i < numTargets; i++) {
    pinMode(targets[i], OUTPUT);
    debugprint(String(targets[i]) + ", ");
  }
  debugprintln("");

  // Get current time
  lastDebounceTime = millis();

  reset();
}


// Reset all components
void reset() {
  debugprintln("RESET - STOPPING EVERYTHING!!");
  isRunning = false;
  digitalWrite(runIndicatorPin, LOW);  
  
  // Reset targets
  for (int i = 0; i < numTargets; i++) {
    digitalWrite(targets[i], LOW);
    currentSequence[i] = false;
  }

  // Reset sequence
  sequenceState = 0;
}

/*
 * Main loop
 */
void loop() {
  // Panic button should stop execution at any point.
  int panicButtonVal = digitalRead(panicButtonPin);
  int pressVal = invertPanicButton ? LOW : HIGH;
  if (panicButtonVal == pressVal) {
    isRunning = false;
    reset();
  }

  if (!isRunning) {
    // If we're not running, then the start button can start the run
    int startButtonVal = digitalRead(startButtonPin);
    int pressVal = invertStartButton ? LOW : HIGH;
    if (startButtonVal == pressVal) {
      isRunning = true;
      digitalWrite(runIndicatorPin, HIGH);
      debugprintln("ACTIVATION COMPLETE");
    }
  } else {
    // We're running; execute main program 
    duringRun();
  }
}

/*
 * The program when it is running
 */
void duringRun() {
  if ((millis() - lastDebounceTime) > currentSequenceStateDuration) {
    // time has passed, get to the next stage
    if (sequenceState >= sequenceLength) {
      // There is no next stage; Reset; run complete
      reset();
      return;
    }

    currentSequenceStateDuration = random(minSequenceDuration, maxSequenceDuration);
    debugprintln("Advanced to stage " + String(sequenceState) + " (" + String(currentSequenceStateDuration / 1000.0) + " seconds)");

    lastDebounceTime = millis();

    if (!useRandomizedSequence) {
      // Should actually set currentSequence = sequence[sequenceState] but C doesn't allow this in
      //  a single line and it doesn't matter here anyway aardappel
      setTargets(sequence[sequenceState]);
    } else {
      // Generate a random sequence instead
      SetRandomSequence();
    }
    sequenceState += 1;
  }
}

void setTargets(bool targetStates[numTargets]) {
  for (int i = 0; i < numTargets; i++) {
    digitalWrite(targets[i], targetStates[i] ? HIGH : LOW);
  }
}

void SetRandomSequence() {
  bool randomSequence[numTargets];
  debugprint("randomSequence = {");
  for (int i = 0; i < numTargets; i++) {
    randomSequence[i] = false;

    // Randomize state if the pin was off previously, or if consecutive activity is allowed
    if (allowConsecutiveActivity or not currentSequence[i]) {
      randomSequence[i] = random(0, 2) == 1;
    }
  
    currentSequence[i] = randomSequence[i];  
    debugprint(String(randomSequence[i]) + ", ");
  }
  setTargets(randomSequence);
  debugprintln("}");
}
