# Primary Flight Display Simulation (PFD_Sim)

A simulation of a Boeing 737-800 Primary Flight Display (PFD), written in C++. This project visually replicates the key features of the aircraft's PFD, allowing users to interact and manipulate flight parameters via keyboard inputs.

## Features

- **Attitude Indicator:** Displays aircraft pitch and roll.
- **Airspeed Tape:** Shows current airspeed with moving tape.
- **Altitude Tape:** Displays altitude with moving tape.
- **Heading Indicator:** Visualized compass with heading bug.
- **Vertical Speed Indicator:** Real-time vertical speed updates.
- **Flight Modes:** Indications for autopilot/flight director status (if implemented).
- **Keyboard Controls:** Change values such as pitch, roll, airspeed, altitude, and heading interactively.

## Getting Started

### Prerequisites

- C++ compiler (supporting C++11 or newer)
- [SFML](https://www.sfml-dev.org/) library for graphics (typically SFML 2.x)
- CMake (recommended) or make

### Building the Project

1. **Clone the repository:**
   ```bash
   git clone https://github.com/n0m4official/Primary-FLight-Display-Simulation.git
   cd Primary-FLight-Display-Simulation
   ```

2. **Install SFML**  
   On Ubuntu:
   ```bash
   sudo apt-get install libsfml-dev
   ```
   On Mac (Homebrew):
   ```bash
   brew install sfml
   ```
   Or follow [official SFML instructions](https://www.sfml-dev.org/download.php) for your platform.

3. **Build with CMake:**
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

   Alternatively, use your IDE to configure and build the project.

### Running the Simulation

After building, run the executable:
```bash
./PFD_Sim
```
A window will open, displaying the simulated PFD.

## Controls

| Key                | Function                  |
|--------------------|--------------------------|
| Arrow Up/Down      | Increase/Decrease pitch  |
| Arrow Left/Right   | Bank left/right (roll)   |
| A / Z              | Increase/Decrease airspeed |
| Q / W              | Increase/Decrease altitude |
| H / J              | Change heading           |
| V / B              | Change vertical speed    |
| R                  | Reset all parameters     |
| Esc                | Exit simulation          |

*Exact controls may vary; check the source code for the latest mappings.*

## Project Structure

- `src/` — C++ source files for display and logic
- `include/` — Header files
- `assets/` — Fonts, images, or other resources
- `CMakeLists.txt` — Build configuration
- `README.md` — This file

## Contributing

Pull requests are welcome! Please open an issue to discuss your changes before submitting a PR.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

## Author

- [n0m4official](https://github.com/n0m4official)

---

*This project is for educational and simulation purposes only, and is not intended for real-world flight training or use.*
