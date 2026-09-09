# Sensor Board — NAMI Firmware

## How to run this (VS Code + PlatformIO + Wokwi)

1. Unzip this folder somewhere on your computer if you haven't already.
2. Open VS Code. File -> Open Folder -> select this `sensor_board` folder
   (the one this README is in).
3. Click the PlatformIO icon in the left sidebar. It should now show
   "esp32doit-devkit-v1" under PROJECT TASKS — if it says "You have not yet
   opened a PlatformIO project" instead, you opened the wrong folder; make
   sure you picked the one containing `platformio.ini`.
4. Click the checkmark (✓) icon in the blue bar at the very bottom of the
   VS Code window. This builds the firmware. Wait for it to finish — first
   build can take a few minutes while it downloads the MPU6050 library.
5. If it says SUCCESS: press F1, type "Wokwi: Start Simulator", press Enter.
6. Watch the Serial Monitor tab that opens. You should see a line printed
   every loop, like:
   `[CRUISE] F=180 L45=200 R45=195 L90=250 R90=245 IR=0 | pose=(0.0,0.0,0) | - -> speed=80 steer=0`
7. In the Wokwi window, drag one of the ultrasonic sensors' distance sliders
   down to simulate an obstacle. The state should flip to `[DECISION]` and
   pick a steer direction — that's proof the logic is working.

## What's in here
```
sensor_board/
├── platformio.ini      # tells PlatformIO which board/libraries to use
├── wokwi.toml           # points Wokwi at the compiled firmware
├── diagram.json          # the simulated circuit (5x ultrasonic, IMU, IR button)
├── ALGORITHM.md          # the NAMI logic explained, with a flowchart
└── src/
    ├── config.h          # ALL pin numbers and tunable thresholds live here
    ├── Ultrasonic.h/.cpp  # reads the 5-sensor array + IR backup
    ├── Pose.h/.cpp        # position/heading tracking (IMU + encoders)
    ├── NAMI.h/.cpp        # the actual decision logic
    ├── UartLink.h/.cpp    # talks to the Motor board (binary protocol, locked)
    └── main.cpp           # ties it all together, runs every loop
```

## Known gaps
- UART protocol to the Motor board is locked to a binary struct
  (see the comment block at the top of `src/UartLink.h`) — this matches
  what `motor_board.ino` expects, confirmed 2026-09-09.
- Pin numbers in `config.h` and `diagram.json` are placeholders — confirm
  against real hardware wiring before flashing.
- `ENCODER_TICKS_PER_REV` / `WHEEL_DIAMETER_CM` in `config.h` are
  placeholders — set to your actual encoders/wheels.
