# Robo Grand Prix — Enigma

**Project Name:** Talos

## Category
Robo Grand Prix — Elimination Round.

## Project Summary
An autonomous 4WD skid-steer robot that navigates a tunnel-style track
(walls on both sides, static obstacles on top) without human control,
using ultrasonic distance sensing and dead-reckoned position tracking to
avoid walls, obstacles, and repeated loops.

## Strategy — NAMI (Navigational Avoidance and Manoeuvrability Interface)
The robot cruises at top speed by default. When a front-mounted sensor
array detects an obstacle or an approaching corner, it slows down and
compares open space on its left vs right using 5 ultrasonic sensors
(0°, ±45°, ±90°), steering toward whichever side has more room. If space
is roughly equal on both sides, it picks randomly rather than stalling
on an unclear reading. To avoid getting stuck circling the same section
of track, the robot tracks its own position (IMU heading + wheel-encoder
distance) and is forced to choose the opposite direction if it detects
it's returned to a spot it already made a decision at.

The system is split across two ESP32 boards: a **Sensor board** that runs
all NAMI decision logic, and a **Motor board** that just executes
whatever drive command it's sent — keeping the safety-critical motor
control simple and isolated from the decision logic.

See `Folder A/sensor_board/ALGORITHM.md` for the full logic writeup and
flowchart.

## Testing & Verification

**Sensor board (`sensor_board`)** — Built with PlatformIO
(`esp32doit-devkit-v1`, Espressif32 platform 7.1.2) with zero warnings.
Compiled firmware uses 6.8% of RAM (22,280 / 327,680 bytes) and 23.8% of
flash (311,617 / 1,310,720 bytes), well within limits. The firmware was
then run in the Wokwi simulator against `diagram.json` (5x ultrasonic +
MPU6050 + IR). With no obstacle present it holds `[CRUISE]` and drives
straight at `CRUISE_SPEED_PCT`; dragging an ultrasonic sensor's distance
below `OBSTACLE_TRIGGER_CM` flips the logged state to `[DECISION]` and
NAMI picks a steer direction toward the side with more open space —
confirming the core sensing → decision pipeline runs correctly end to
end. See `Folder A/sensor_board/test-evidence.png` for the Serial
Monitor output showing this `[CRUISE]` → `[DECISION]` transition.

**Motor board (`motor_board.ino`)** — Built independently with
PlatformIO on the same board target. Initial build caught a real
compatibility issue: `ledcAttach()` (arduino-esp32 3.x LEDC API) wasn't
available on the installed toolchain, which only exposed the older
`ledcSetup`/`ledcAttachPin` pair. Fixed with a version-guarded macro
(`ESP_ARDUINO_VERSION_MAJOR`) so the firmware builds against either LEDC
API without manual changes. Final build: zero errors, 6.7% RAM,
21.3% flash used.

**What this does and doesn't prove:** the sensing, obstacle/corner
detection, and space-comparison logic are demonstrated live. The
tie-break coin-flip and loop-prevention override are implemented per
spec and code-reviewed against the algorithm description, but weren't
individually exercised in this test session — both depend on specific
conditions (near-equal L/R readings; revisiting a previous position)
that a single manual obstacle trigger doesn't produce. The two boards'
UART link was verified by direct comparison of the `SteerCommand`
struct on both sides (must byte-match — see `UartLink.h`), not by a
live combined simulation, since Wokwi has no native part for the
TB6612FNG driver.

## Directory Map

```
├── Folder A/                    # Source Code
│   ├── sensor_board/             # Decision-brain firmware (NAMI logic)
│   │   ├── src/                  # Sensor reading, pose tracking, NAMI, UART
│   │   ├── ALGORITHM.md          # Full NAMI logic explanation + flowchart
│   │   └── README.md             # Build/simulate instructions for this board
│   └── motor_board/               # Motor-executor firmware
│       └── motor_board.ino        # Skid-steer drive, encoders, e-stop failsafe
├── Folder B/                    # Designs (CAD, wiring diagrams, etc.)
└── Folder C/                    # [fill in: any remaining rubric-required folder]
```

## Hardware
4WD skid-steer chassis, JGB37-520 motors with encoders, 2× TB6612FNG motor
drivers, 5-sensor ultrasonic front array, 1–2 digital IR backup sensors,
MPU6050 IMU, split across two ESP32 boards linked by UART, with a dual
e-stop (physical + RF) wired to a shared cutoff relay.

**Weight:** [TBD]
**Dimensions:** [TBD]

## Team
- **Nakokutemwa Mapala** — Lead, Background Research, 3D Modelling, Coding
- **Emmanuel Sibuyi** — Engineering
- **Fanuel Estifanos** — Assistant Researcher, Coding
- **Mfundo Motse** — Diagrams, Slide Deck, Technical Report, Documentation Lead
