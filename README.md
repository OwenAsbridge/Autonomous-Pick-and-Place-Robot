# Autonomous Pick-and-Place Robot

Wheeled autonomous robot that identifies, picks up, and delivers three colored cubes to matching drop-off cups without human intervention. Built for ME351 Mechatronics at South Dakota School of Mines, Spring 2026. Full three-block delivery cycle completes in **65 seconds**.

![Full robot assembly](media/full_assembly.png)

## Demo

[![Watch the demo](https://img.youtube.com/vi/497uHKiroMU/maxresdefault.jpg)](https://www.youtube.com/watch?v=497uHKiroMU)

*Click to watch the full run on YouTube.*

## Overview

The task: design a fully autonomous robot that navigates between two locations, identifies three colored cubes at Spot A, and delivers them to matching-colored cups at Spot B. No cameras, no vision systems, no human intervention after start, onboard power only, $100 total budget.

Our solution is a four-wheeled mobile base with a vertical ball-screw arm carrying a parallel-gripper claw. Two color sensors work in tandem: a TCS3200 mounted below the claw reads the block being held, and an AS7262 six-channel spectral sensor mounted underneath the chassis reads colored ground markers that trigger state changes in the control logic.

## Key Results

| Quantity | Value |
|---|---|
| Full 3-block delivery cycle time | **65 seconds** |
| Blocks delivered per run (consistent) | **2–3 of 3** |
| Sensing channels (color + spectral) | **9 wavelengths total** |
| Actuators | 2× DC drive, 1× stepper, 1× servo |
| Autonomous after start? | **Yes** |
| Budget | **$100 (met)** |

## System Architecture

**Mechanical**
- Four-wheel chassis, two-wheel drive (differential steering via speed offset)
- Purchased ball-screw stack (bought over machined to save budget), custom PETG guide to keep it straight under load
- Parallel-linkage gripper claw driven by a single servo through a spur-gear pair
- Full assembly 3D-printed in PLA and PETG
- Custom-designed chassis to mount and route all subsystems

**Sensing**
- **TCS3200** color sensor (5V, 3 channels: 470/550/665 nm) mounted below the claw to identify the block being carried
- **AS7262** six-channel spectral sensor (3.3V, 400/470/550/600/630/665 nm) mounted underneath the chassis for line following and ground-marker detection

**Actuation**

| Component | Function | Driver | Supply |
|---|---|---|---|
| 2× VEX EDR 393 geared DC | Drive wheels | Cytron MDD3A | 7.4 V LiPo |
| SY42STH38-1684A stepper | Ball-screw vertical axis | A4988 | 9 V |
| DS3218 20 kg servo | Gripper claw | Arduino direct | 4× AA (5.8 V) |

**Control**
- **Arduino MEGA 2560** (upgraded from UNO mid-project after PWM pin conflicts between `Servo.h` and the four wheel-motor PWM channels forced the change)
- Custom C++ firmware implementing a state machine driven by ground-marker detection
- Common ground across all subsystems; independent power supplies to isolate motor draw from logic

![Gripper claw with color sensor detail](media/gripper_detail.png)

## Design Decisions

Selection between three arm architectures (articulated, ball-screw, extending) and three gripper concepts (vice-grip, dual-servo, parallel-linkage) was driven by weighted decision matrices on cost, ease of manufacture, redundancy, and reliability:

- **Ball-screw arm** was chosen over an articulated arm for its mechanical simplicity, single-actuator drive, and superior repeatability in vertical positioning. It scored highest on reliability with only a modest cost penalty vs. the extending arm.
- **Parallel-linkage gripper** was chosen over a vice-grip design because it was lighter (less load on the ball-screw stepper), cheaper, and required only one servo. It traded a small reliability margin for meaningful weight and complexity savings.

## Results

- Robot met all project requirements: fully autonomous after start, delivered all three colored cubes from Spot A to matching cups at Spot B, onboard power only, no vision.
- Full three-block cycle in **65 seconds**.
- Successfully handled all color orderings without reprogramming, using the state machine to defer pickup based on the current block-count integer.
- **Stopping repeatability of ±2–6 cm** at each ground marker, driven by two coupled hardware limitations: the sensor refresh rate introduced detection latency between the marker crossing the sensor and the control loop reacting, and cycle-to-cycle drive-motor speed variation (from battery voltage sag) meant the distance covered during that latency window was inconsistent. The combined effect occasionally caused the robot to overshoot or stop short of a block or cup by enough to miss it. Higher-refresh-rate sensors and encoded drive motors would eliminate both error sources, but neither upgrade was feasible within the $100 project budget.
## What I'd Do Differently

- **Closed-loop navigation instead of open-loop time/speed estimation.** The robot has no way to correct heading if it drifts off the line — we compensated by tuning the two drive motors to slightly different speeds to counteract weight-distribution drift, but a proper differential-drive chassis with line-tracking feedback would eliminate the fragility entirely.
- **Better power system.** Battery voltage sag between cycles was the single largest source of run-to-run variability. Dedicated, higher-capacity packs (and voltage monitoring on the logic side) would meaningfully improve repeatability.
- **Chassis stress relief.** Several bolt-hole locations developed stress concentrations and cracked over the project. Filleting corners and adding local reinforcement in the initial CAD would have prevented most of the mid-project repairs.
- **Cable management.** Loose wiring caused several intermittent disconnects during testing. A proper harness plan from day one — even just labeled zip-tie routes — would have saved hours of debug time.

## My Role

Team lead and mechanical design lead on a 5-person team. Responsible for:
- **All mechanical subsystems** — CAD, design, fabrication, and assembly (chassis, ball-screw stack, gripper claw, sensor mounts)
- **System integration** — bringing electrical, sensing, and mechanical subsystems together into a working unit
- **Testing and final debug** — ran full-system testing, made late-stage code updates to hit target functionality, led problem-solving during the final integration phase
- **Team coordination** — scheduling, task assignment, keeping the team on track through the project

## Tools

- **Mechanical:** SolidWorks (CAD), 3D printing (PLA and PETG, FDM)
- **Electronics:** Arduino MEGA 2560, A4988 stepper driver, Cytron MDD3A DC motor driver, TCS3200, AS7262
- **Software:** Arduino C++, `Servo.h`, `Wire.h`, `Adafruit_AS726x`

## Repository Structure

```
├── README.md
├── src/
│   └── Pick_and_Place_Code_Final.ino     # Arduino MEGA firmware
├── docs/
│   └── ME351_Mechatronics_Pick_and_Place_Report.pdf
└── media/
    ├── full_assembly.png                 # SolidWorks render — full system
    └── gripper_detail.png                # Claw + color sensor detail
```

## Team

- **Owen Asbridge** — Mechanical Design & Integration, Team Lead
- **Cadyn Chaput** — Actuation & Power Electronics
- **Jakson Santi** — Sensing & Measurement
- **Sarah Mitzel** — Control, Logic & Software
- **Colton Ackerman** — Theoretical Analysis & System Modeling

Course: ME351 Mechatronics, South Dakota School of Mines and Technology, Spring 2026.
