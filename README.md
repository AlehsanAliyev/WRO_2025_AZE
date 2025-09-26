# WRO 2025 AZE Robot Playbook

This repo packs everything we are using for the WRO 2025 autonomous car challenge -- code, wiring notes and media from test runs. Use this top-level guide as your jumping-off point.

## Repo Quick Links
- `codes/README.md` -- friendly walkthrough of the Round 1 and Round 2 Arduino sketches.
- `schemas/POWER_SENSE.md` -- wiring, batteries, and signal map between the Pi, Nano, and motor driver.
- `schemas/initial_schema.png` -- single-page wiring schematic for fast visual checks.
- `RobotImages/` & `RobotVideos/` -- field photos and practice footage for quick context.

## Robot Snapshot
- Compute: Raspberry Pi 5 handles vision and high-level intent; Arduino Nano closes the loop on steering and drive.
- Actuation: L298N motor driver feeds the DC propulsion motor; an SG90 servo on pin D9 steers the nose.
- Power: dual LiPo packs (3S 1100 mAh for drive, 3S 500 mAh for the Nano) plus a USB-C bank for the Pi. Grounds are tied together -- details in `schemas/POWER_SENSE.md`.
- Sensing: TCS3200 color sensor watches floor pads while three IR modules (A0/A1/A2) act like bumpers. The Pi can nudge the Nano with `rasback`, `rasleft`, and `rasright` signals when it spots trouble.
- Human interface: a start button on D8 and a RUN/SAFE toggle on A4 so we can arm the car safely.

## Software Rounds Overview
Both rounds share the same hardware interface but handle the field a little differently. The full choreography lives in `codes/README.md` if you want the step-by-step breakdown.
- **Round 1 (`codes/round1.ino`)** keeps things simple: react to color pads, hug the lane with IR bumpers, and count turns until we finish the twelfth sector.
- **Round 2 (`codes/round2.ino`)** layers in the Pi handoff. Extra timers smooth lane changes, `front()` watches for head-on blockages, and the turn routine includes a sharper "back out" move to settle faster.

## Power & Sensor Layout (cheat sheet)
Pull the full table from `schemas/POWER_SENSE.md`, but here are the pieces you'll reach for most often:
- Motor driver: `EN=D5`, `IN1=D6`, `IN2=D7`.
- Servo: `D9` PWM.
- Color sensor: `S0=D11`, `S1=D12`, `S2=D13`, `S3=A3`, `OUT=D10`.
- IR bumpers: left A1, right A0, back A2.
- Pi intent lines: `rasback=D2`, `rasleft=D3`, `rasright=D4`.
- Safety toggle: `A4` (pull it LOW to keep the bot in SAFE).

## How to Run a Round
1. Charge both LiPos and the Pi's bank; confirm all grounds are common.
2. Flip the SAFE toggle, power up the Pi and Nano, then upload either `round1.ino` or `round2.ino` from the Arduino IDE.
3. Place the robot in its parking bay, hold it steady, and press the start button. The sketch waits for that HIGH edge before moving.
4. Watch the telemetry in the Arduino Serial Monitor (9600 baud) if you need to confirm color readings or obstacle flags.



