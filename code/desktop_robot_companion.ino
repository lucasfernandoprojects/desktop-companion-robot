#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <math.h>
#include <esp_sleep.h>

// =====================================================
// HARDWARE CONFIGURATION
// =====================================================

#define OLED_SDA       26
#define OLED_SCL       27
#define OLED_ADDRESS   0x3C

#define TOUCH_PIN      12

#define BUZZER_PIN     25
#define BUZZER_VOLUME  30
#define BUZZER_RESOLUTION 8

Adafruit_SH1107 display(128, 128, &Wire);

// =====================================================
// TIMING CONFIGURATION
// =====================================================

// Idle behavior (testing values)
const unsigned long IDLE_SLEEPY_TIME = 180000;  // 3 minutes to enter SLEEPY mode
const unsigned long IDLE_SLEEP_TIME = 120000;   // 2 minutes to enter SLEEPING mode (after SLEEPY has started)
const unsigned long LIGHT_SLEEP_TIME = 300000;  // 5 minutes to enter LIGHT SLEEP (after SLEEPING has started)

// Interaction expressions
const unsigned long EXPRESSION_TIME = 10000;
const unsigned long SLEEP_WAKE_SURPRISE_TIME = 3000;
const unsigned long LIGHT_WAKE_SURPRISE_TIME = 5000;

// Goodbye message
const unsigned long GOODBYE_TIME = 5000;

// Facial animations
const unsigned long BLINK_MIN_INTERVAL = 2500;
const unsigned long BLINK_MAX_INTERVAL = 6000;

// =====================================================
// EXPRESSIONS
// =====================================================

enum Expression {
  NEUTRAL,
  HAPPY,
  ENJOYING,
  EXCITED,
  SLEEPY,
  SLEEPING,
  SURPRISED,
  EXPRESSION_COUNT
};

const char* expressionNames[] = {
  "Neutral",
  "Happy",
  "Enjoying",
  "Excited",
  "Sleepy",
  "Sleeping",
  "Surprised"
};

Expression currentExpression = EXPRESSION_COUNT;

// =====================================================
// INTERACTION COUNTER
// =====================================================

int petCount = 0;

// =====================================================
// IDLE AND SLEEP MANAGEMENT
// =====================================================

unsigned long lastInteraction = 0;

bool isSleepy = false;
bool isSleeping = false;

bool goodbyeActive = false;
unsigned long goodbyeStarted = 0;

bool screenIsOff = false;
bool readyForLightSleep = false;

// =====================================================
// TIMED EXPRESSIONS
// =====================================================

enum TimedAction {
  RETURN_TO_NEUTRAL,
  SHOW_COUNTED_RESPONSE,
  SHOW_HAPPY_AFTER_WAKE
};

bool timedExpressionActive = false;

unsigned long timedExpressionUntil = 0;

TimedAction timedAction = RETURN_TO_NEUTRAL;

// =====================================================
// TOUCH SYSTEM
// =====================================================

bool touchWasActive = false;
bool ignoreTouchUntilRelease = false;

// =====================================================
// FACIAL ANIMATION
// =====================================================

int eyeOffsetX = 0;
int targetEyeOffsetX = 0;

unsigned long lastEyeMovement = 0;
unsigned long nextEyeMovement = 0;
unsigned long eyeReturnTime = 0;

bool eyeReturning = false;

bool isBlinking = false;
unsigned long blinkStart = 0;
unsigned long nextBlink = 0;

// Forward declaration
void scheduleNextBlink();

// =====================================================
// BUZZER AUDIO SYSTEM
// =====================================================

struct Note {
  int frequency;
  unsigned int duration;
};

// Happy melody
const Note happyMelody[] = {
  {784, 130},
  {988, 130},
  {1175, 220}
};

// Enjoying melody
const Note enjoyingMelody[] = {
  {523, 160},
  {659, 160},
  {784, 160},
  {1047, 300}
};

// Excited melody
const Note excitedMelody[] = {
  {784, 100},
  {988, 100},
  {1175, 100},
  {1568, 220},
  {1318, 120},
  {1568, 300}
};

// =====================================================
// SLEEPING MELODY
// =====================================================

// Original lullaby-style melody.
// It uses a slower rhythm, several melodic phrases,
// and short pauses between some notes.

const Note sleepMelody[] = {

  // Phrase 1
  {392, 400},   // G4
  {440, 350},   // A4
  {523, 550},   // C5
  {440, 350},   // A4
  {392, 600},   // G4
  {0, 180},

  // Phrase 2
  {330, 400},   // E4
  {392, 350},   // G4
  {440, 550},   // A4
  {392, 350},   // G4
  {330, 650},   // E4
  {0, 180},

  // Phrase 3
  {294, 400},   // D4
  {330, 350},   // E4
  {392, 550},   // G4
  {440, 350},   // A4
  {523, 700},   // C5
  {0, 250},

  // Phrase 4
  {523, 400},   // C5
  {440, 350},   // A4
  {392, 550},   // G4
  {330, 400},   // E4
  {294, 700},   // D4
  {0, 300},

  // Final phrase
  {330, 400},   // E4
  {392, 400},   // G4
  {440, 600},   // A4
  {392, 400},   // G4
  {330, 550},   // E4
  {294, 800},   // D4
  {0, 500}
};

// Face transition sounds
const Note neutralBeep[] = {
  {523, 100}
};

const Note sleepyBeep[] = {
  {392, 180}
};

const Note surprisedBeep[] = {
  {1047, 180}
};

const Note goodbyeBeep[] = {
  {392, 150},
  {330, 200}
};

const Note* activeMelody = nullptr;

int melodyLength = 0;
int melodyIndex = 0;

bool audioPlaying = false;
bool noteStarted = false;
bool buzzerReady = false;

bool loopMelody = false;

unsigned long noteStartTime = 0;

// =====================================================
// BUZZER FUNCTIONS
// =====================================================

void stopAudio() {

  audioPlaying = false;
  noteStarted = false;
  activeMelody = nullptr;
  loopMelody = false;

  if (buzzerReady) {
    ledcWrite(BUZZER_PIN, 0);
  }
}

void playMelody(
  const Note* melody,
  int length,
  bool repeat = false
) {

  if (!buzzerReady) return;

  stopAudio();

  activeMelody = melody;
  melodyLength = length;
  melodyIndex = 0;

  loopMelody = repeat;
  noteStarted = false;
  audioPlaying = true;
}

void updateAudio() {

  if (!audioPlaying || activeMelody == nullptr) {
    return;
  }

  unsigned long now = millis();

  if (!noteStarted) {

    if (melodyIndex >= melodyLength) {

      if (loopMelody) {
        melodyIndex = 0;
      } else {
        stopAudio();
        return;
      }
    }

    int frequency = activeMelody[melodyIndex].frequency;

    if (frequency > 0) {
      ledcWriteTone(BUZZER_PIN, frequency);
      ledcWrite(BUZZER_PIN, BUZZER_VOLUME);
    } else {
      ledcWrite(BUZZER_PIN, 0);
    }

    noteStartTime = now;
    noteStarted = true;

    return;
  }

  if (now - noteStartTime >=
      activeMelody[melodyIndex].duration) {

    ledcWrite(BUZZER_PIN, 0);

    melodyIndex++;
    noteStarted = false;

    if (melodyIndex >= melodyLength) {

      if (loopMelody) {
        melodyIndex = 0;
      } else {
        stopAudio();
      }
    }
  }
}

// =====================================================
// DRAWING UTILITIES
// =====================================================

int eyeX(int x) {
  return x + eyeOffsetX;
}

void drawThickLine(
  int x1, int y1,
  int x2, int y2,
  int thickness
) {

  float dx = x2 - x1;
  float dy = y2 - y1;

  float length = sqrt(dx * dx + dy * dy);

  if (length == 0) {
    display.fillCircle(
      x1, y1, thickness / 2, SH110X_WHITE
    );
    return;
  }

  float steps = length * 2.0;
  int radius = (thickness + 1) / 2;

  for (int i = 0; i <= (int)steps; i++) {

    float t = (float)i / steps;

    int x = round(x1 + dx * t);
    int y = round(y1 + dy * t);

    display.fillCircle(
      x, y, radius, SH110X_WHITE
    );
  }
}

void drawEyeLine(
  int x1, int y1,
  int x2, int y2,
  int thickness
) {

  drawThickLine(
    eyeX(x1), y1,
    eyeX(x2), y2,
    thickness
  );
}

void drawHeart(int centerX, int topY) {

  display.fillCircle(
    eyeX(centerX - 7), topY + 8, 9, SH110X_WHITE
  );

  display.fillCircle(
    eyeX(centerX + 7), topY + 8, 9, SH110X_WHITE
  );

  display.fillTriangle(
    eyeX(centerX - 16), topY + 9,
    eyeX(centerX + 16), topY + 9,
    eyeX(centerX), topY + 29,
    SH110X_WHITE
  );
}

// =====================================================
// NEUTRAL FACE
// =====================================================

void drawNeutralEyes() {

  display.fillRoundRect(
    eyeX(20), 30, 34, 52, 16, SH110X_WHITE
  );

  display.fillRoundRect(
    eyeX(74), 30, 34, 52, 16, SH110X_WHITE
  );
}

void drawNeutralMouth() {

  drawThickLine(55, 84, 61, 88, 3);
  drawThickLine(61, 88, 67, 89, 3);
  drawThickLine(67, 89, 73, 86, 3);
}

// =====================================================
// HAPPY FACE
// =====================================================

void drawHappyEyes() {

  drawEyeLine(19, 39, 48, 51, 6);
  drawEyeLine(48, 51, 20, 64, 6);

  drawEyeLine(109, 39, 80, 51, 6);
  drawEyeLine(80, 51, 108, 64, 6);
}

void drawHappyMouth() {

  display.fillRoundRect(
    45, 68, 38, 34, 12, SH110X_WHITE
  );

  display.fillRoundRect(
    52, 68, 24, 14, 5, SH110X_BLACK
  );
}

// =====================================================
// ENJOYING FACE
// =====================================================

void drawEnjoyingEyes() {

  drawEyeLine(18, 29, 19, 47, 5);
  drawEyeLine(19, 47, 25, 62, 5);
  drawEyeLine(25, 62, 38, 68, 5);
  drawEyeLine(38, 68, 51, 62, 5);
  drawEyeLine(51, 62, 57, 47, 5);
  drawEyeLine(57, 47, 58, 29, 5);

  drawEyeLine(70, 29, 71, 47, 5);
  drawEyeLine(71, 47, 77, 62, 5);
  drawEyeLine(77, 62, 90, 68, 5);
  drawEyeLine(90, 68, 103, 62, 5);
  drawEyeLine(103, 62, 109, 47, 5);
  drawEyeLine(109, 47, 110, 29, 5);
}

void drawEnjoyingMouth() {

  drawThickLine(52, 76, 58, 84, 4);
  drawThickLine(58, 84, 64, 76, 4);
  drawThickLine(64, 76, 70, 84, 4);
  drawThickLine(70, 84, 76, 76, 4);
}

// =====================================================
// EXCITED FACE
// =====================================================

void drawExcitedEyes() {

  drawHeart(35, 24);
  drawHeart(93, 24);
}

void drawExcitedMouth() {

  display.fillCircle(
    64, 77, 19, SH110X_WHITE
  );

  display.fillRect(
    44, 57, 40, 20, SH110X_BLACK
  );
}

// =====================================================
// SLEEPY FACE
// =====================================================

void drawSleepyEyes() {

  display.fillRoundRect(
    eyeX(16), 38, 43, 34, 8, SH110X_WHITE
  );

  display.fillRoundRect(
    eyeX(69), 38, 43, 34, 8, SH110X_WHITE
  );

  display.fillRect(
    eyeX(16), 38, 43, 19, SH110X_BLACK
  );

  display.fillRect(
    eyeX(69), 38, 43, 19, SH110X_BLACK
  );
}

void drawSleepyMouth() {

  display.fillRoundRect(
    51, 89, 26, 4, 2, SH110X_WHITE
  );
}

// =====================================================
// SLEEPING FACE
// =====================================================

void drawSleepingEyes() {

  // Closed, gently curved eyes positioned lower to match
  // the vertical placement of the other expressions.
  drawEyeLine(17, 55, 25, 62, 5);
  drawEyeLine(25, 62, 32, 64, 5);
  drawEyeLine(32, 64, 40, 62, 5);
  drawEyeLine(40, 62, 47, 55, 5);

  drawEyeLine(81, 55, 88, 62, 5);
  drawEyeLine(88, 62, 96, 64, 5);
  drawEyeLine(96, 64, 103, 62, 5);
  drawEyeLine(103, 62, 111, 55, 5);
}

void drawSleepingMouth() {

  // Small vertical oval, positioned with the other mouths.
  display.fillRoundRect(
    58, 74, 12, 21, 6, SH110X_WHITE
  );
}

// =====================================================
// SURPRISED FACE
// =====================================================

void drawSurprisedEyes() {

  display.fillRoundRect(
    eyeX(20), 30, 34, 52, 16, SH110X_WHITE
  );

  display.fillRoundRect(
    eyeX(74), 30, 34, 52, 16, SH110X_WHITE
  );
}

void drawSurprisedMouth() {

  display.drawCircle(64, 103, 9, SH110X_WHITE);
  display.drawCircle(64, 103, 8, SH110X_WHITE);
}

// =====================================================
// FACE RENDERER
// =====================================================

void drawFace(Expression expression) {

  display.clearDisplay();

  switch (expression) {

    case NEUTRAL:
      drawNeutralEyes();
      drawNeutralMouth();
      break;

    case HAPPY:
      drawHappyEyes();
      drawHappyMouth();
      break;

    case ENJOYING:
      drawEnjoyingEyes();
      drawEnjoyingMouth();
      break;

    case EXCITED:
      drawExcitedEyes();
      drawExcitedMouth();
      break;

    case SLEEPY:
      drawSleepyEyes();
      drawSleepyMouth();
      break;

    case SLEEPING:
      drawSleepingEyes();
      drawSleepingMouth();
      break;

    case SURPRISED:
      drawSurprisedEyes();
      drawSurprisedMouth();
      break;

    default:
      break;
  }

  display.display();
}

// =====================================================
// EXPRESSION SOUND
// =====================================================

void playFaceSound(Expression expression) {

  switch (expression) {

    case NEUTRAL:
      playMelody(
        neutralBeep,
        sizeof(neutralBeep) / sizeof(Note)
      );
      break;

    case SLEEPY:
      playMelody(
        sleepyBeep,
        sizeof(sleepyBeep) / sizeof(Note)
      );
      break;

    case SURPRISED:
      playMelody(
        surprisedBeep,
        sizeof(surprisedBeep) / sizeof(Note)
      );
      break;

    default:
      break;
  }
}

// =====================================================
// EXPRESSION MANAGEMENT
// =====================================================

void setExpression(
  Expression expression,
  bool playSound = true
) {

  if (currentExpression == expression) return;

  currentExpression = expression;

  Serial.print("Expression: ");
  Serial.println(expressionNames[expression]);

  isBlinking = false;

  // Schedule a fresh blink after every expression change.
  scheduleNextBlink();

  drawFace(expression);

  if (playSound) {
    playFaceSound(expression);
  }
}

// =====================================================
// PET RESPONSE
// =====================================================

Expression getPetExpression() {

  if (petCount >= 10) {
    return EXCITED;
  }

  if (petCount >= 5) {
    return ENJOYING;
  }

  return HAPPY;
}

void showPetResponse() {

  Expression response = getPetExpression();

  stopAudio();

  setExpression(response, false);

  switch (response) {

    case HAPPY:
      playMelody(
        happyMelody,
        sizeof(happyMelody) / sizeof(Note)
      );
      break;

    case ENJOYING:
      playMelody(
        enjoyingMelody,
        sizeof(enjoyingMelody) / sizeof(Note)
      );
      break;

    case EXCITED:
      playMelody(
        excitedMelody,
        sizeof(excitedMelody) / sizeof(Note)
      );
      break;

    default:
      break;
  }

  timedExpressionActive = true;
  timedExpressionUntil = millis() + EXPRESSION_TIME;
  timedAction = RETURN_TO_NEUTRAL;

  Serial.print("Pet counter: ");
  Serial.println(petCount);
}

// =====================================================
// TIMED EXPRESSIONS
// =====================================================

void showTimedExpression(
  Expression expression,
  unsigned long duration,
  TimedAction action
) {

  timedExpressionActive = true;
  timedExpressionUntil = millis() + duration;
  timedAction = action;

  setExpression(expression);
}

void updateTimedExpression() {

  if (!timedExpressionActive) return;

  if (millis() < timedExpressionUntil) {
    return;
  }

  timedExpressionActive = false;

  switch (timedAction) {

    case RETURN_TO_NEUTRAL:

      if (currentExpression == EXCITED) {
        petCount = 0;
        Serial.println("Pet counter reset.");
      }

      setExpression(NEUTRAL);
      break;

    case SHOW_COUNTED_RESPONSE:

      showPetResponse();
      break;

    case SHOW_HAPPY_AFTER_WAKE:

      setExpression(HAPPY, false);

      playMelody(
        happyMelody,
        sizeof(happyMelody) / sizeof(Note)
      );

      timedExpressionActive = true;
      timedExpressionUntil = millis() + EXPRESSION_TIME;
      timedAction = RETURN_TO_NEUTRAL;
      break;
  }
}

// =====================================================
// GOODBYE MESSAGE
// =====================================================

void showGoodbyeMessage() {

  display.clearDisplay();

  display.setTextColor(SH110X_WHITE);

  display.setTextSize(2);
  display.setCursor(18, 38);
  display.println("Goodbye!");

  display.setTextSize(1);
  display.setCursor(16, 68);
  display.println("See you soon!");

  display.display();
}

void startGoodbye() {

  if (goodbyeActive) return;

  goodbyeActive = true;
  goodbyeStarted = millis();

  timedExpressionActive = false;

  Serial.println("Showing goodbye message.");

  stopAudio();

  playMelody(
    goodbyeBeep,
    sizeof(goodbyeBeep) / sizeof(Note)
  );

  showGoodbyeMessage();
}

void updateGoodbye() {

  if (!goodbyeActive) return;

  if (millis() - goodbyeStarted >= GOODBYE_TIME) {

    goodbyeActive = false;

    petCount = 0;

    Serial.println("Goodbye message finished.");
    Serial.println("Turning off OLED.");

    display.oled_command(SH110X_DISPLAYOFF);

    screenIsOff = true;
    readyForLightSleep = true;
  }
}

// =====================================================
// IDLE BEHAVIOR
// =====================================================

void updateIdleBehavior() {

  unsigned long idleTime = millis() - lastInteraction;

  if (goodbyeActive || readyForLightSleep) {
    return;
  }

  // After 90 seconds: show goodbye.
  if (idleTime >= LIGHT_SLEEP_TIME) {
    startGoodbye();
    return;
  }

  // After 60 seconds: enter Sleeping.
  if (idleTime >= IDLE_SLEEP_TIME) {

    if (!isSleeping) {

      isSleeping = true;
      isSleepy = false;

      timedExpressionActive = false;

      stopAudio();

      setExpression(SLEEPING, false);

      // Start the full lullaby and repeat it.
      playMelody(
        sleepMelody,
        sizeof(sleepMelody) / sizeof(Note),
        true
      );

      Serial.println("Robot is sleeping.");
    }

    return;
  }

  // After 30 seconds: enter Sleepy.
  if (idleTime >= IDLE_SLEEPY_TIME) {

    if (!isSleepy) {

      isSleepy = true;

      timedExpressionActive = false;

      stopAudio();

      setExpression(SLEEPY);

      Serial.println("Robot is getting sleepy.");
    }
  }
}

// =====================================================
// TOUCH HANDLING
// =====================================================

void processPet() {

  lastInteraction = millis();

  // Cancel Goodbye and pending light sleep.
  goodbyeActive = false;
  readyForLightSleep = false;

  // Restore the screen if necessary.
  if (screenIsOff) {
    display.oled_command(SH110X_DISPLAYON);
    screenIsOff = false;
  }

  bool wasSleeping = isSleeping;

  // IMPORTANT:
  // Every pet resets both idle states.
  // This fixes the bug where the robot skipped Sleepy
  // and went directly to Sleeping after being petted.
  isSleepy = false;
  isSleeping = false;

  // Stop the sleep melody if it was playing.
  if (wasSleeping) {
    stopAudio();
  }

  // Cancel any previous timed expression.
  timedExpressionActive = false;

  // Count this pet.
  petCount++;

  Serial.print("Pet counter: ");
  Serial.println(petCount);

  // If the robot was sleeping, show Surprise first.
  if (wasSleeping) {

    showTimedExpression(
      SURPRISED,
      SLEEP_WAKE_SURPRISE_TIME,
      SHOW_COUNTED_RESPONSE
    );

    Serial.println("Robot woke from sleeping.");

    return;
  }

  // Otherwise, respond according to the pet counter.
  showPetResponse();
}

void handleTouch() {

  bool touched = digitalRead(TOUCH_PIN) == HIGH;

  // New touch detected.
  if (touched && !touchWasActive) {

    touchWasActive = true;

    if (ignoreTouchUntilRelease) {
      return;
    }

    processPet();
  }

  // Touch released.
  if (!touched && touchWasActive) {

    touchWasActive = false;

    if (ignoreTouchUntilRelease) {
      ignoreTouchUntilRelease = false;
    }
  }
}

// =====================================================
// FACIAL ANIMATIONS
// =====================================================

void scheduleNextBlink() {

  nextBlink = millis() + random(
    BLINK_MIN_INTERVAL,
    BLINK_MAX_INTERVAL
  );
}

void startBlink() {

  if (isSleeping || isSleepy ||
      goodbyeActive || isBlinking ||
      currentExpression == SURPRISED) {
    return;
  }

  isBlinking = true;
  blinkStart = millis();

  display.fillRect(10, 20, 53, 65, SH110X_BLACK);
  display.fillRect(65, 20, 53, 65, SH110X_BLACK);

  drawThickLine(17, 52, 55, 52, 4);
  drawThickLine(73, 52, 111, 52, 4);

  display.display();
}

void updateBlink() {

  unsigned long now = millis();

  if (isBlinking) {

    if (now - blinkStart >= 130) {

      isBlinking = false;

      drawFace(currentExpression);

      scheduleNextBlink();
    }

    return;
  }

  if (now >= nextBlink &&
      !isSleeping &&
      !isSleepy &&
      !touchWasActive &&
      !goodbyeActive &&
      !timedExpressionActive) {

    startBlink();
  }
}

void updateEyeMovement() {

  unsigned long now = millis();

  if (isSleeping ||
      isSleepy ||
      isBlinking ||
      touchWasActive ||
      goodbyeActive ||
      timedExpressionActive) {
    return;
  }

  if (!eyeReturning && now >= nextEyeMovement) {

    int direction = random(0, 2);

    targetEyeOffsetX = direction == 0 ? -3 : 3;

    eyeReturning = true;
    eyeReturnTime = now + 550;
  }

  if (eyeReturning && now >= eyeReturnTime) {

    targetEyeOffsetX = 0;
    eyeReturning = false;

    nextEyeMovement = now + random(5000, 9000);
  }

  if (now - lastEyeMovement >= 45) {

    lastEyeMovement = now;

    if (eyeOffsetX < targetEyeOffsetX) {
      eyeOffsetX++;
    } else if (eyeOffsetX > targetEyeOffsetX) {
      eyeOffsetX--;
    }

    if (eyeOffsetX != targetEyeOffsetX) {
      drawFace(currentExpression);
    }
  }
}

// =====================================================
// ESP32 LIGHT SLEEP
// =====================================================

void enterLightSleep() {

  if (!readyForLightSleep) return;

  // Do not sleep while the touch sensor is active.
  if (digitalRead(TOUCH_PIN) == HIGH) {
    return;
  }

  Serial.println("Entering ESP32 light sleep...");

  stopAudio();

  Serial.flush();

  // GPIO12 wakes the classic ESP32 when it becomes HIGH.
  esp_err_t result = esp_sleep_enable_ext0_wakeup(
    GPIO_NUM_12,
    1
  );

  if (result != ESP_OK) {

    Serial.println("Could not configure wake-up source.");

    display.oled_command(SH110X_DISPLAYON);

    screenIsOff = false;
    readyForLightSleep = false;

    return;
  }

  esp_err_t sleepResult = esp_light_sleep_start();

  if (sleepResult != ESP_OK) {
    Serial.println("Light sleep failed.");
  }

  // Execution resumes here after wake-up.
  Serial.println("ESP32 woke up!");

  display.oled_command(SH110X_DISPLAYON);

  screenIsOff = false;
  readyForLightSleep = false;

  lastInteraction = millis();

  isSleepy = false;
  isSleeping = false;
  goodbyeActive = false;

  petCount = 0;

  // Ignore the touch that woke the ESP32 until release.
  touchWasActive = true;
  ignoreTouchUntilRelease = true;

  // Surprise for five seconds, then Happy.
  showTimedExpression(
    SURPRISED,
    LIGHT_WAKE_SURPRISE_TIME,
    SHOW_HAPPY_AFTER_WAKE
  );

  Serial.println("Robot woke from light sleep.");
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  randomSeed(esp_random());

  // Touch sensor.
  pinMode(TOUCH_PIN, INPUT);

  // Passive buzzer.
  buzzerReady = ledcAttach(
    BUZZER_PIN,
    2000,
    BUZZER_RESOLUTION
  );

  if (buzzerReady) {

    ledcWrite(BUZZER_PIN, 0);

    Serial.println("Buzzer initialized.");

  } else {

    Serial.println("Buzzer initialization failed!");
  }

  // OLED.
  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(OLED_ADDRESS, true)) {

    Serial.println("OLED initialization failed!");

    while (true) {
      delay(100);
    }
  }

  display.setRotation(0);
  display.clearDisplay();
  display.display();

  // Initial state.
  petCount = 0;

  lastInteraction = millis();

  nextEyeMovement = millis() + random(4000, 8000);

  scheduleNextBlink();

  Serial.println("Desktop companion robot started.");

  setExpression(NEUTRAL, false);
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop() {

  handleTouch();

  updateTimedExpression();

  updateIdleBehavior();

  updateGoodbye();

  updateBlink();

  updateEyeMovement();

  updateAudio();

  enterLightSleep();
}
