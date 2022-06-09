/*
 * Fantasy Court - Nerf Target Practise
 *
 * TODO: Maak een beschrijving van hoe het ongeveer werkt, en hoe je het configureert
 * Controle snelheid via potmeter en settings via rotary switch
 *
 * KP: Updated motor enable pin & status LED from old run indicator code, changed pin types from bool to const int, added delay so gates close before enable turns off.
 * Changed motor pin numbers, lost game.
 * Oude gekopieërde meuk:
 *
*/

const int startButtonPin = 15; // Start button (inverted)
const int panicButtonPin = 14; // Emergency Exit Button (inverted)

const bool invertStartButton = true; // Whether the start button signal becomes LOW upon press
const bool invertPanicButton = true; // idem

// Analog inputs
const int speedModAnalog = A7;

// Define targets
const int target1Pin = 2;
const int target2Pin = 7;
const int target3Pin = 4;
const int target4Pin = 6;
const int target5Pin = 5;
const int target6Pin = 3;

const int numTargets = 4;
const int maxNumSubtargets = 2;
const int targets[numTargets][maxNumSubtargets] = {{target1Pin, target2Pin}, {target3Pin, target4Pin}, {target5Pin, -1}, {target6Pin, -1}};

// Amount of times targets open up (not used in infinite mode)
const int sequenceLength = 32;

// Hardcoded sequence useful for testing
bool sequence[sequenceLength][numTargets] = {
  { false, false, false, false },
  { true, false, false, false },
  { false, true, true, false },
  { true, false, false, false },
  { false, false, true, true },
  { true, false, false, false },
  { false, false, true, false },
  { true, false, false, false },
  { false, false, true, false },
  { false, true, false, false },
  { false, false, true, false },
  { false, true, false, false },
  { true, false, false, false },
  { false, true, true, false },
  { false, false, false, true },
  { false, false, true, false },
  { false, true, false, false },
  { true, false, false, false },
  { false, true, false, false },
  { false, false, true, true },
  { true, false, false, false },
  { false, false, true, false },
  { false, true, false, false },
  { false, false, true, false },
  { false, false, false, true },
  { false, false, true, false },
  { false, true, false, false },
  { false, false, true, false },
  { true, false, false, false },
  { false, false, true, false },
  { false, true, false, false },
  { false, false, false, false },
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
const int driverEnablePin = 11; // Motor driver enable pin
const int statusLEDPin = 12; // Status LED, currently function same as Motor driver enable

const bool allowConsecutiveActivity = false; // Whether a target can be active for two consecutive sequences
const int minActiveTargetsPerSequence = 1; // Minimum number of active targets (NOT YET IMPLEMENTED)
const int maxActiveTargetsPerSequence = 4; // Maximum number of active targets (NOT YET IMPLEMENTED)

typedef enum {
  GAMEMODE_SHORT,
  GAMEMODE_LONG,
  GAMEMODE_INFINITE,
  GAMEMODE_BULLSHIT
} GameMode;
GameMode currentGameMode = GAMEMODE_SHORT;

const int mode0Pin = 19;
const int mode1Pin = 18;
const int mode2Pin = 17;

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

  // Gamemode Selector
  pinMode(mode0Pin, INPUT_PULLUP);
  pinMode(mode1Pin, INPUT_PULLUP);
  pinMode(mode2Pin, INPUT_PULLUP);

  // Run indicator
  pinMode(driverEnablePin, OUTPUT);
  pinMode(statusLEDPin, OUTPUT);

  debugprint("Marking following pins as output: ");
  // Mark the pins of targets as output
  for (int i = 0; i < numTargets; i++) {
    debugprint("(");
    for (int j = 0; j < maxNumSubtargets; j++){
      if (targets[i][j] >= 0) { // target must be set
        pinMode(targets[i][j], OUTPUT);
        debugprint(String(targets[i][j]) + ", ");
      }
    }
    debugprint("), ");
  }
  debugprintln("");

  // Get current time
  lastDebounceTime = millis();

  reset(true);
}


// Reset all components
void reset(bool open_targets) {
  debugprintln("RESET - STOPPING EVERYTHING!!");
  isRunning = false;

  // Supply power so the targets can still close
  digitalWrite(driverEnablePin, HIGH);
  digitalWrite(statusLEDPin, HIGH);

  // Reset targets
  for (int i = 0; i < numTargets; i++) {
    setTarget(i, open_targets ? HIGH : LOW);
    currentSequence[i] = open_targets;
  }
  delay(1000); //Needs some time to open(?) before drivers are disabled.
  digitalWrite(driverEnablePin, LOW);  //Disable driver after opening(?) all targets.
  digitalWrite(statusLEDPin, LOW);
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
    reset(true);
  }

  if (!isRunning) {
    // If we're not running, then we can change the gamemode
    UpdateGameMode();
    
    // We can also use the start button to start the run
    int startButtonVal = digitalRead(startButtonPin);
    int pressVal = invertStartButton ? LOW : HIGH;
    if (startButtonVal == pressVal) {
      // Close all targets before starting      
      reset(false);

      delay(4000); // Wait a little so the players can be ready
      // Re-enable power
      isRunning = true;
      digitalWrite(driverEnablePin, HIGH);
      digitalWrite(statusLEDPin, HIGH);

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
    int endOfSequence = currentGameMode == GAMEMODE_SHORT ? sequenceLength / 2 : sequenceLength;
    if (sequenceState >= endOfSequence) {
      if (currentGameMode == GAMEMODE_INFINITE or currentGameMode == GAMEMODE_BULLSHIT) {
        // Continue infinitely; restart from beginning
        sequenceState = 0;
      } else {
        // There is no next stage; Reset; run complete
        reset(false);
        return;
      }
    }

    currentSequenceStateDuration = random(minSequenceDuration, maxSequenceDuration);

    int extra_delay = (analogRead(speedModAnalog) / 1023.0) * 10000;
    currentSequenceStateDuration += extra_delay;

    if (currentGameMode == GAMEMODE_BULLSHIT and random(0, 3) == 0) {
      // Almost no delay in bullshit mode with a 1/3 chance (syke!)
      currentSequenceStateDuration = 250;
    }

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
    setTarget(i, targetStates[i] ? HIGH : LOW);
  }
}

void setTarget(int targetIndex, int value) {
  for (int i = 0; i < maxNumSubtargets; i++) {
    if (targets[targetIndex][i] >= 0) { // target must be set
      digitalWrite(targets[targetIndex][i], value);
    }
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

GameMode UpdateGameMode() {
  if (digitalRead(mode0Pin) == LOW) {
    if (currentGameMode != GAMEMODE_SHORT) {
      debugprintln("Gamemode: SHORT");
    }
    currentGameMode = GAMEMODE_SHORT;
  } else if (digitalRead(mode1Pin) == LOW) {
    if (currentGameMode != GAMEMODE_LONG) {
      debugprintln("Gamemode: LONG");
    }
    currentGameMode = GAMEMODE_LONG;
  } else if (digitalRead(mode2Pin) == LOW) {
    if (currentGameMode != GAMEMODE_INFINITE) {
      debugprintln("Gamemode: INFINITE");
    }
    currentGameMode = GAMEMODE_INFINITE;
  } else {
    // Fallback; can be a different mode 
    if (currentGameMode != GAMEMODE_BULLSHIT) {
      debugprintln("Gamemode: BULLSHIT");
    } 
    currentGameMode = GAMEMODE_BULLSHIT;
  }
}
