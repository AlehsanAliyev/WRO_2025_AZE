## Power and Sense

Pair this sheet with the overviews in `../README.md` and `../codes/README.md` when you are wiring or debugging the robot on the table.

### Batteries
- **Raspberry Pi 5**: USB-C power bank.
- **Actuation** (L298N + 2A DC motors + SG90): **3S 1100 mAh LiPo**.
- **Arduino Nano**: **3S 500 mAh LiPo**.
- **Common ground:** Pi, Nano, and L298N share the same reference.

### Structure & Purpose
- **Compute (Pi 5):** camera + vision; outputs intent (`rasback`, `rasleft`, `rasright`) to the Nano.
- **Controller (Arduino Nano):** reads intent + local sensors and drives motors/steering.
- **Drive (L298N):** motor direction + speed.
- **Steering (Servo):** front wheel angle.
- **Sensing:** IR (obstacles) + TCS3200 (line color).
- **Human I/O:** Start button and RUN/SAFE toggle.

### Pin Map (Arduino Nano)
| Block / Signal              | Pins | Purpose |
|---                          |---   |---|
| **L298N**                   | `EN=D5`, `IN1=D6`, `IN2=D7` | Motor enable + direction |
| **Servo**                   | `D9` | PWM for steering |
| **IR sensors**              | `A1` (left), `A0` (right), `A2` (back) | Obstacle detection |
| **TCS3200 (color sensor)**  | `S0=D11`, `S1=D12`, `S2=D13`, `S3=A3`, `OUT=D10` | Line color read |
| **Pi <-> Nano intent**      | `rasback=D2`, `rasleft=D3`, `rasright=D4` | Commands from Pi |
| **Start button**            | `D8` | Arm/start |
| **Toggle (RUN/SAFE)**       | `A4` | Safety interlock |
| **Ground**                  | `GND` | Common reference (Pi, Nano, L298N) |
