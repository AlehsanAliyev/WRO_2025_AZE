# Round 1 & Round 2 Code Guide

Let me walk you through what the two Arduino sketches are doing and how they make the robot behave on the field.

## Common Hardware & Ideas
- **Drive**: one DC motor (`in1`, `in2`, `en`) for forward/backward plus a servo on `servoPin` that steers the front axle.
- **Color sensing**: the TCS3200 module on `s0`-`s3` and `out_pin` spots track colors. The helper `direction()` maps the pulse widths and returns:
  - `1` when it spots the blue pad (signal to turn left / counter-clockwise).
  - `2` when it spots the orange pad (turn right / clockwise).
  - `0` for the default white floor.
- **Infrared proximity**: `irleft`, `irright`, `irback` are used like bumpers to see walls or parking borders.
- **Start logic**: both rounds chill in a `while (start)` loop until the button on pin `8` goes HIGH, so the car never jumps off the line accidentally.
- **Turn bookkeeping**: `turns` counts how many colored pads have been handled. Once we log twelve of them, the bot drives forward for a second and stops forever so we finish neatly in the last sector.
- **Debounce distance**: every straight-line burst increments `distance`. Only after about 50 ticks will the code listen to the next color pad so we do not react twice to the same marker.
- **Clockwise flag**: `clockwise` remembers the last full turn direction and prevents the robot from repeating the same turn back-to-back unless it has had time to drive between pads.

## How a Turn Actually Happens
1. `direction()` spots the blue or orange pad.
2. If `distance > 50` and the requested turn is different from the previous one, the code locks in `clockwise` (1 for right, -1 for left).
3. It then runs a choreographed set of `forward()` and `backward()` pulses while sweeping the steering servo: shimmy forward into the intersection, swing the nose away, reverse to pivot around the pad, then drive forward longer to line up with the new lane. The steering finally snaps close to `mid` so we exit straight.
4. `turns++` and `distance = 0` so the next pad cannot trigger immediately.

Both files share that core routine; the main differences are in the sections below.

## Round 1 Upshot
- **Tracks only**: the vehicle relies on the color pad plus the three IR sensors. There is no separate front distance sensor, so dodging is simpler.
- **Wall handling**:
  - If the left IR goes LOW (meaning it sees the border) the robot either nudges forward-right if the rear IR also sees the wall, or it backs up with the wheels turned left to peel away (`round1.ino` lines 123-140).
  - The right side does the symmetric thing (`round1.ino` lines 142-159).
- **Parking helper**: `leave_parking()` exists but is not called in the main loop any more; it backs the car out, swings the steering twice, then sets `inparking = false`.

## Round 2 Upgrades
Round 2 keeps everything above, and layers in a few more checks for a busier map.

- **Extra "ras" sensors**: digital inputs on pins `2`, `3`, `4` act like front/left/right beacons (from a Raspberry Pi). They let the bot notice opponents driving the wrong way or blocking the lane.
- **Timers as memory**:
  - `lefttimer` and `righttimer` store how many 30 ms steps we spent squeezing around an obstacle. Afterwards the main loop keeps steering away for a scaled-down amount of time so the robot drifts smoothly back to the center (`round2.ino` lines 95-137).
  - `left_obstacle_timer` and `right_obstacle_timer` count how long an IR sensor stays triggered. If either hits `30`, the code assumes we are rubbing the wall and executes a short "reverse and swing out" escape move (`round2.ino` lines 139-170).
- **Front watchdog**: `front()` watches `rasback` (the naming is historical; it is really looking ahead). If it sees a HIGH signal, the car stops progressing, keeps the wheels straight, and backs up until the path is clear before making another attempt (`round2.ino` lines 178-188).
- **Turn exit tweak**: after finishing the usual steer-dance, Round 2 immediately calls `backward()` with a slightly higher speed to pull the rear axle fully into the lane. That extra step makes the car settle faster before releasing control back to the main loop (`round2.ino` lines 192-242).

## Quick Reference Table
| Check | Sensor & pin(s) | Trigger | Reaction |
| ----- | ---------------- | ------- | -------- |
| Track color | TCS3200 on `out_pin` | Blue pad (`direction() == 1`) | Full left turn routine |
| Track color | TCS3200 on `out_pin` | Orange pad (`direction() == 2`) | Full right turn routine |
| Side wall | `irleft == LOW` | Left side too close | Steer right or run escape shimmy |
| Side wall | `irright == LOW` | Right side too close | Mirror of the left logic |
| Rear wall | `irback == LOW` | Something behind | Helps decide whether to escape forward or backward |
| Wrong-way car (Round 2) | `rasleft` / `rasright` HIGH | Someone on the wrong side | Hug the safe side for a timed interval |
| Blocked front (Round 2) | `rasback` HIGH | Traffic jam ahead | Reverse until clear, then resume |

## Related Docs
- `../README.md`  project overview, hardware snapshot, and quick links.
- `../schemas/POWER_SENSE.md`  wiring, batteries, and signal mapping between the Pi, Nano, and motor driver.


