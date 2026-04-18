// 4-Part Harmony (Pin-based)

#include <Arduino.h>
#include "pitches.h"
#include <math.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* WIFI_SSID = "";
const char* WIFI_PASSWORD = "";

const char* MQTT_BROKER = "broker.emqx.io";
const int   MQTT_PORT   = 1883;

const char* DATA_TOPIC = "joshandjoe/finalproject/esp32wroom#1/system/status";
const char* CMD_TOPIC  = "joshandjoe/finalproject/esp32wroom#1/system/cmd";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

constexpr uint16_t BPM = 120;

// -------------------- ADDED: PIR + LED --------------------
const uint8_t PIR_PIN = 27;
const uint8_t LED_PIN = 26;

bool motionDetected = false;
bool wasMotionActive = false;
uint32_t ledTimer = 0;
bool ledState = false;
// ----------------------------------------------------------

// Pins
const uint8_t BUZZER1_PIN = 32;
const uint8_t BUZZER2_PIN = 14;
const uint8_t BUZZER3_PIN = 21;
const uint8_t BUZZER4_PIN = 19;

// Channels
const int8_t CH1 = 0;
const int8_t CH2 = 2;
const int8_t CH3 = 4;
const int8_t CH4 = 6;

// Seed params
const uint32_t V1_SEED_RES  = 12;
const uint8_t  V1_SEED_FREQ = 19531;

const uint8_t  V2_SEED_RES  = 13;
const uint32_t V2_SEED_FREQ = 9765;

const uint32_t V3_SEED_RES  = 14;
const uint8_t  V3_SEED_FREQ = 15000;

const uint32_t V4_SEED_RES  = 15;
const uint8_t  V4_SEED_FREQ = 5000;

// Timing helpers
#define MS_PER_Q   (60000UL / BPM)
#define DUR(n)     ((4UL * MS_PER_Q) / (n))
#define DOT(x)     ((x) + (x)/2)
#define DBL_DOT(x) ((x) + (x)/2 + (x)/4)
#define TRIP(x)    (((x) * 2) / 3)

#define W  DUR(1)
#define H  DUR(2)
#define Q  DUR(4)
#define E  DUR(8)
#define S  DUR(16)

// MIDI → frequency
static const int REST = -1;

static double mtof(int midi) {
  if (midi == REST) return 0.0;
  return 440.0 * pow(2.0, (midi - 69) / 12.0);
}

// -------------------- STRUCTS (FIXED ORDER) --------------------
enum Articulation : uint8_t { LEGATO=0, NORMAL=1, STACCATO=2 };

struct PartConfig {
  const char*   name;
  uint8_t       pin;
  uint8_t       channel;
  uint8_t       resolutionBits;
  Articulation  articulation = NORMAL;
  uint8_t       gapNormalPct = 10;
  uint8_t       gapStaccatoPct = 50;
};

struct NoteEvent {
  double    freqHz;
  uint32_t  durMs;
  uint8_t   velocity;
  int8_t    artOverride;
};

enum Phase : uint8_t { PHASE_PLAY=0, PHASE_GAP=1, PHASE_DONE=2 };

struct PartState {
  PartConfig       cfg;
  const NoteEvent* seq = nullptr;
  uint16_t         length = 0;
  uint16_t         index = 0;
  Phase            phase = PHASE_DONE;
  uint32_t         phaseEndMs = 0;

  double           currentFreqHz = 0.0;
  uint8_t          currentVel = 0;
  uint32_t         soundMs = 0;
  uint32_t         gapMs = 0;
};

// Function Prototypes (needed for compiler)
static inline void toneOn(const PartConfig& cfg, double freqHz);
static inline void toneOff(const PartConfig& cfg);
static Articulation effectiveArt(const PartConfig& cfg, int8_t artOverride);
static void splitArticulation(const PartConfig& cfg, int8_t artOverride,
                              uint32_t noteDurMs, uint32_t& soundMs, uint32_t& gapMs);
static void servicePart(PartState& st);
static void startPart(PartState& st);

// -------------------- FUNCTIONS --------------------
static inline void toneOn(const PartConfig& cfg, double freqHz) {
  ledcWriteTone(cfg.pin, (freqHz <= 0.0) ? 0 : freqHz);
}

static inline void toneOff(const PartConfig& cfg) {
  ledcWriteTone(cfg.pin, 0);
}

static Articulation effectiveArt(const PartConfig& cfg, int8_t artOverride) {
  if (artOverride >= 0 && artOverride <= 2) return (Articulation)artOverride;
  return cfg.articulation;
}

static void splitArticulation(const PartConfig& cfg, int8_t artOverride,
                              uint32_t noteDurMs, uint32_t& soundMs, uint32_t& gapMs) {
  Articulation art = effectiveArt(cfg, artOverride);
  uint8_t gapPct = 0;

  if (art == NORMAL) gapPct = cfg.gapNormalPct;
  else if (art == STACCATO) gapPct = cfg.gapStaccatoPct;

  if (gapPct > 100) gapPct = 100;

  gapMs   = (noteDurMs * gapPct) / 100;
  soundMs = noteDurMs - gapMs;
}

static void servicePart(PartState& st) {
  const uint32_t now = millis();
  if (st.phase == PHASE_DONE || now < st.phaseEndMs) return;

  if (st.phase == PHASE_PLAY) {
    if (st.gapMs > 0) {
      toneOff(st.cfg);
      st.phase = PHASE_GAP;
      st.phaseEndMs = now + st.gapMs;
    } else {
      st.index++;
      if (st.index >= st.length) {
        st.phase = PHASE_DONE;
        toneOff(st.cfg);
      } else {
        const NoteEvent& ev = st.seq[st.index];
        splitArticulation(st.cfg, ev.artOverride, ev.durMs, st.soundMs, st.gapMs);

        if (ev.freqHz <= 0.0 || ev.velocity == 0) toneOff(st.cfg);
        else toneOn(st.cfg, ev.freqHz);

        st.phase = PHASE_PLAY;
        st.phaseEndMs = now + st.soundMs;
      }
    }
  } else {
    st.index++;
    if (st.index >= st.length) {
      st.phase = PHASE_DONE;
      toneOff(st.cfg);
    } else {
      const NoteEvent& ev = st.seq[st.index];
      splitArticulation(st.cfg, ev.artOverride, ev.durMs, st.soundMs, st.gapMs);

      if (ev.freqHz <= 0.0 || ev.velocity == 0) toneOff(st.cfg);
      else toneOn(st.cfg, ev.freqHz);

      st.phase = PHASE_PLAY;
      st.phaseEndMs = now + st.soundMs;
    }
  }
}

static void startPart(PartState& st) {
  st.index = 0;
  st.phase = (st.length == 0) ? PHASE_DONE : PHASE_PLAY;

  if (st.phase == PHASE_DONE) {
    toneOff(st.cfg);
    return;
  }

  const NoteEvent& ev = st.seq[0];
  splitArticulation(st.cfg, ev.artOverride, ev.durMs, st.soundMs, st.gapMs);
  st.currentFreqHz = ev.freqHz;
  st.currentVel    = ev.velocity;

  if (ev.freqHz <= 0.0 || ev.velocity == 0) toneOff(st.cfg);
  else toneOn(st.cfg, ev.freqHz);

  st.phaseEndMs = millis() + st.soundMs;
}

// Use the MIDI value, such as mtof(78), or a note name such as: NOTE_FS5  (the S means "sharp")
const NoteEvent S_PART[] = {
  { NOTE_C5 ,  Q,            110, -1 }, // C5 quarter
  { mtof(74),  Q,            100, -1 }, // D5 quarter
  { mtof(76),  E,            120, -1 }, // E5 eighth
  { mtof(77),  E,            110,  0 }, // F5 eighth, LEGATO override
  { mtof(79),  DOT(Q),       127, -1 }, // G5 dotted quarter
  { mtof(77),  E,             90,  2 }, // F5 eighth, STACCATO override
  { mtof(76),  Q,            105, -1 }, // E5 quarter
  { 0.0,       E,              0, -1 }, // rest eighth
  { mtof(76),  Q,            110, -1 }, // E5 quarter
  { mtof(72),  H,            120, -1 }, // C5 half
  { 0.0     ,  Q,            100, -1 },
};

const NoteEvent A_PART[] = {
  { mtof(67),  Q,             95, -1 },
  { mtof(69),  Q,             95, -1 },
  { mtof(67),  Q,             95, -1 },
  { mtof(69),  Q,             95, -1 },
  { mtof(71),  DOT(Q),       100, -1 },
  { mtof(69),  E,             90, -1 },
  { mtof(67),  Q,             95, -1 },
  { 0.0,       E,              0, -1 },
  { mtof(67),  Q,             95, -1 },
  { mtof(67),  H,            100, -1 },
  { 0.0     ,  Q,            100, -1 },
};

const NoteEvent T_PART[] = {
  { mtof(60),  Q,             85, -1 },
  { mtof(62),  Q,             85, -1 },
  { mtof(64),  Q,             90, -1 },
  { mtof(65),  Q,             90, -1 },
  { mtof(67),  DOT(Q),        95, -1 },
  { mtof(65),  E,             85, -1 },
  { mtof(64),  Q,             90, -1 },
  { 0.0,       E,              0, -1 },
  { mtof(64),  Q,             90, -1 },
  { mtof(60),  H,             95, -1 },
  { 0.0     ,  Q,            100, -1 },
};

const NoteEvent B_PART[] = {
  { mtof(48),  Q,             90, -1 },
  { mtof(48),  Q,             90, -1 },
  { mtof(52),  Q,             95, -1 },
  { mtof(53),  Q,             95, -1 },
  { mtof(55),  DOT(Q),       100, -1 },
  { mtof(53),  E,             90, -1 },
  { mtof(52),  Q,             95, -1 },
  { 0.0,       E,              0, -1 },
  { mtof(52),  Q,             95, -1 },
  { mtof(48),  H,            100, -1 },
  { 0.0     ,  Q,            100, -1 },
};

// ------------------------------ Part wiring -------------------------------------
// NOTE!!! Soprano is set to STACCATO, bass to NORMAL, others to LEGATO
// This can be changed here, or overridden for each note in each part
PartState soprano{
  .cfg = PartConfig{
    .name="S", .pin=BUZZER1_PIN, .channel=CH1, .resolutionBits=(uint8_t)V1_SEED_RES,
    .articulation=STACCATO, .gapNormalPct=10, .gapStaccatoPct=55
  },
  .seq=S_PART, .length=(uint16_t)(sizeof(S_PART)/sizeof(S_PART[0]))
};

PartState alto{
  .cfg = PartConfig{
    .name="A", .pin=BUZZER2_PIN, .channel=CH2, .resolutionBits=(uint8_t)V2_SEED_RES,
    .articulation=LEGATO, .gapNormalPct=10, .gapStaccatoPct=55
  },
  .seq=A_PART, .length=(uint16_t)(sizeof(A_PART)/sizeof(A_PART[0]))
};

PartState tenor{
  .cfg = PartConfig{
    .name="T", .pin=BUZZER3_PIN, .channel=CH3, .resolutionBits=(uint8_t)V3_SEED_RES,
    .articulation=LEGATO, .gapNormalPct=10, .gapStaccatoPct=55
  },
  .seq=T_PART, .length=(uint16_t)(sizeof(T_PART)/sizeof(T_PART[0]))
};

PartState bass{
  .cfg = PartConfig{
    .name="B", .pin=BUZZER4_PIN, .channel=CH4, .resolutionBits=(uint8_t)V4_SEED_RES,
    .articulation=NORMAL, .gapNormalPct=10, .gapStaccatoPct=55
  },
  .seq=B_PART, .length=(uint16_t)(sizeof(B_PART)/sizeof(B_PART[0]))
};


//Put in any additional functions here!

// -------------------- SETUP --------------------
void setup() {
  ledcSetClockSource((ledc_clk_cfg_t)LEDC_APB_CLK);

  ledcAttachChannel(BUZZER1_PIN, V1_SEED_FREQ, V1_SEED_RES, CH1);
  ledcAttachChannel(BUZZER2_PIN, V2_SEED_FREQ, V2_SEED_RES, CH2);
  ledcAttachChannel(BUZZER3_PIN, V3_SEED_FREQ, V3_SEED_RES, CH3);
  ledcAttachChannel(BUZZER4_PIN, V4_SEED_FREQ, V4_SEED_RES, CH4);

  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  delay(30);
}

// -------------------- LOOP --------------------
void loop() {
  motionDetected = digitalRead(PIR_PIN);

  // LED blink
  if (motionDetected) {
    if (millis() - ledTimer > 200) {
      ledTimer = millis();
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
    }
  } else {
    digitalWrite(LED_PIN, LOW);
  }

  // Music control
  if (motionDetected) {

    if (!wasMotionActive) {
      startPart(soprano);
      startPart(alto);
      startPart(tenor);
      startPart(bass);
      wasMotionActive = true;
    }

    servicePart(soprano);
    servicePart(alto);
    servicePart(tenor);
    servicePart(bass);

  } else {
    if (wasMotionActive) {
      toneOff(soprano.cfg);
      toneOff(alto.cfg);
      toneOff(tenor.cfg);
      toneOff(bass.cfg);

      soprano.phase = PHASE_DONE;
      alto.phase    = PHASE_DONE;
      tenor.phase   = PHASE_DONE;
      bass.phase    = PHASE_DONE;

      wasMotionActive = false;
    }
  }

  if (wasMotionActive &&
      soprano.phase==PHASE_DONE && alto.phase==PHASE_DONE &&
      tenor.phase==PHASE_DONE && bass.phase==PHASE_DONE) {

    delay(800);

    startPart(soprano);
    startPart(alto);
    startPart(tenor);
    startPart(bass);
  }
}