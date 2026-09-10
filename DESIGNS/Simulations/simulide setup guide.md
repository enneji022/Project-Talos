# Setting Up and Running a Simulation in SimulIDE

SimulIDE is a free, real-time electronic circuit simulator that supports analog/digital circuits and microcontrollers (AVR, PIC, Arduino). This guide covers the general workflow from installation to running a simulation.

## 1. Download and Install

1. Go to the [SimulIDE website](https://simulide.com) or its GitHub releases page.
2. Download the version for your OS (Windows, Linux, or macOS). On Linux, it's also available as a Flatpak.
3. Extract the downloaded archive to a folder of your choice — no formal installation is required.
4. Launch the executable:
   - **Windows:** double-click `simulide.exe` inside the `bin` folder.
   - **Linux/macOS:** run the executable from the extracted `SimulIDE_x.x.x` folder, either by double-clicking it or launching it from a terminal (useful if you want to see log messages).

## 2. Get Familiar with the Interface

The SimulIDE window is divided into three main panels:

- **Component list** (left) — browse and select components to add to your circuit.
- **Circuit canvas** (center) — where you build and simulate your circuit; includes a toolbar with Power, Pause, and settings icons.
- **File explorer / Info / Message panels** (bottom or right) — browse files, view simulation info, and see debug messages or errors.

## 3. Build Your Circuit

1. Start a new circuit file (or open an existing `.sim1` file).
2. Search for the components you need in the component list (e.g., Arduino Uno, LEDs, resistors, batteries).
3. Drag and drop each component onto the canvas.
4. Wire components together by clicking and dragging between terminals. Right-click a wire to delete it if needed.
5. Set component properties (values, models) by right-clicking a part and choosing its properties/edit option.

## 4. Add Firmware or Code (for Microcontroller Simulations)

If your circuit includes a microcontroller (e.g., Arduino Uno):

1. Write your code either:
   - In an external IDE (e.g., Arduino IDE), compiling it to a `.hex` file, or
   - Directly in SimulIDE's built-in code editor (create a new sketch file).
2. If compiling externally, note the Arduino IDE's build output folder (or export the compiled binary).
3. If using SimulIDE's built-in editor/compiler, right-click your sketch file and set the **Compiler Path** to your toolchain (e.g., Arduino installation folder).
4. Right-click the microcontroller component on the canvas and choose **Load Firmware**, then select the `.hex` file.
   - Alternatively, open the component's **Properties**, set the firmware path, and enable **Reload HEX at Simulation Start** so it auto-loads on every run.

## 5. Run the Simulation

1. Double-check your circuit connections and component settings.
2. Click the **Power** button in the toolbar to start the simulation.
3. Observe the circuit behavior in real time — LEDs light up, signals change, etc.
4. Use the **Pause** button to temporarily halt the simulation, and the **Power** button again to stop it.
5. For microcontroller projects, you can right-click the chip to open a **Serial Monitor** or **MCU Monitor** to view serial output or register/variable values while the simulation runs.

## 6. Iterate

- You can adjust component values or wiring while the simulation is running or paused.
- If you update your firmware/code, recompile it and either reload the `.hex` manually or restart the simulation (if auto-reload is enabled) to apply changes.

## Tips

- SimulIDE is optimized for speed and ease of use rather than precise circuit analysis — good for learning and prototyping, not professional-grade analysis.
- Save your circuit periodically as a `.sim1` file so you can reopen and continue working on it later.
