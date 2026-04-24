# How I Implemented the PCB and Schematic for This ESP32-C3 Fan Controller

This project is a custom `ESP32-C3` fan-control board built around a simple idea: keep the high-power fan wiring separate from the low-voltage sensing and control side, then let the firmware make smart decisions using light, air-quality, temperature, humidity, and Bluetooth-based triggers.

I wrote this guide in an Instructables style so a new user can understand not just what is connected, but why I arranged the schematic and PCB this way.

## What This Board Is Meant To Do

The board combines these functions:

- accept a `12V` input for the fan side
- drive a fan through a `TRIG/PWM` control path
- power the controller side from a regulated low-voltage rail
- host an `ESP32-C3 DevKitM-1`
- connect I2C sensors on a shared bus
- support firmware that exposes the whole system through `ESP RainMaker`

From the current hardware and firmware together, the intended sensor set is:

- `VEML7700` light sensor
- `ENS160` air-quality sensor
- `AHT21` temperature/humidity sensor

The firmware pin mapping used by the board is:

| Function | ESP32-C3 pin |
|---|---|
| Fan PWM / trigger output | `GPIO3` |
| I2C SDA | `GPIO6` |
| I2C SCL | `GPIO7` |
| Low-voltage supply reference | `3V3` / low-voltage rail |
| Ground | `GND` |

The firmware also probes these I2C addresses:

- `VEML7700`: `0x10`
- `ENS160`: `0x52`
- `AHT21`: `0x38`

## Supplies

- `ESP32-C3 DevKitM-1`
- `12V` fan
- fan control stage or MOSFET/PWM module
- `LM2596` buck converter module
- `TP4056 USB-C` charger module
- `3.7V` Li-ion battery, shown as `1200 mAh` in the schematic
- `VEML7700` module
- `ENS160 + AHT21` style I2C sensor module, or separate compatible modules
- PCB CAD tool of your choice
- soldering tools and hookup wire for testing

## Step 1: Break the Design Into Functional Blocks

Before routing anything, I split the design into five easy-to-manage blocks:

1. `12V` input and fan power path
2. low-voltage power conversion and battery section
3. `ESP32-C3 DevKitM-1` controller core
4. fan control interface
5. I2C sensor interface

This made the schematic much easier to read, and it also shaped the PCB placement later.

## Step 2: Build the Power Tree First

The schematic shows two power domains:

- a `12V` domain for the fan side
- a low-voltage domain for the controller, sensors, and battery-backed section

Here is the flow I implemented:

1. `VIN` and `VIN_GND` bring `12V` onto the board.
2. That `12V` rail feeds the fan side and also feeds the `LM2596` buck converter input.
3. The buck converter creates the lower-voltage rail labeled `3.7V` in the schematic.
4. The `TP4056` charging module is tied to the battery section.
5. The battery and low-voltage rail then support the controller side of the design.

The reason I started here is simple: if the power rails are messy, the rest of the board becomes difficult to debug.

## Step 3: Choose the ESP32-C3 as the Center of the Board

I placed the `ESP32-C3 DevKitM-1` in the middle of the design because it is the hub for everything:

- PWM leaves from the ESP32 to the fan-control stage
- I2C leaves from the ESP32 to the sensor modules
- power enters the ESP32 from the low-voltage section

Centering the dev board on the PCB helped keep:

- the fan-control trace short
- the I2C traces simple
- the board easy to probe during testing

## Step 4: Assign the Pins Before Routing

I matched the schematic to the firmware so the hardware and software agree from the start.

These are the key pins used in code:

- `GPIO3` for fan `TRIG/PWM`
- `GPIO6` for `SDA`
- `GPIO7` for `SCL`

This is important because the current firmware in [main/app_priv.h](C:/esp/v5.4.4/fan_controller/main/app_priv.h:16) and [main/app_main.c](C:/esp/v5.4.4/fan_controller/main/app_main.c:24) is already written around those assignments.

## Step 5: Add the Fan-Control Stage

Instead of driving the fan directly from the microcontroller, I used a separate control stage between the `ESP32-C3` and the fan.

On the schematic and PCB, the fan section is built like this:

- the fan receives its power from the `12V` side
- the control module receives a `TRIG/PWM` signal from `GPIO3`
- grounds are shared so the control signal has a valid reference

Why I did it this way:

- the ESP32 pin only handles logic-level control
- the fan current stays out of the microcontroller pins
- the power path is easier to size and route safely

If your fan is a plain DC motor and not a 4-wire PWM fan, make sure your driver stage includes the protection it needs. Do not connect a high-current fan directly to an ESP32 GPIO.

## Step 6: Put All Sensors on One Shared I2C Bus

The board uses a shared I2C bus for the environmental sensors. In the schematic, both sensor modules are connected to the same bus, and in firmware they are initialized as a shared sensor interface.

The shared bus is:

- `GPIO6` -> `SDA`
- `GPIO7` -> `SCL`

I used this arrangement because it saves pins and keeps the PCB compact. It also makes the software architecture cleaner because all the sensors can be scanned and read from the same bus.

The sensor roles are:

- `VEML7700` for ambient light
- `ENS160` for air quality
- `AHT21` for temperature and humidity

## Step 7: Name the Nets Clearly in the Schematic

One of the biggest things that makes a custom board easier for a new user is readable net naming.

I used labels like:

- `VIN`
- `VIN_GND`
- `12V`
- `GND_12V`
- `3.7V`
- `GND_3V3`
- `TRIG/PWM`
- `SDA`
- `SCL`

Using named nets helps in three ways:

- the schematic is easier to follow
- the PCB routing is easier to verify
- firmware-to-hardware mapping becomes obvious

## Step 8: Place the Parts on the PCB by Function

The PCB layout follows the same block logic as the schematic:

- top-left: fan connection area
- left side: fan-control stage and `12V` routing
- center: `ESP32-C3 DevKitM-1`
- right side: sensor headers/modules
- bottom: power-conversion and battery section

This placement was intentional. I wanted the board to read naturally from power input to control to output.

It also helped keep noisy power traces away from the sensor side.

## Step 9: Route High-Current Traces First

When I moved from schematic to PCB, I routed the high-current paths before the signal traces:

1. `VIN` and fan power path
2. buck-converter input and output
3. ground return paths
4. fan trigger/PWM signal
5. I2C sensor signals

That order matters because the power traces usually need:

- more width
- cleaner return paths
- fewer compromises

Then I routed the lower-current logic signals after the heavy paths were locked in.

## Step 10: Keep the Sensor Side Clean

The right side of the PCB is mostly dedicated to the sensor connections. I kept that area cleaner and simpler than the power side because I2C is more reliable when:

- traces are short
- routing is direct
- shared lines are easy to inspect

This is also why the sensors sit close to the ESP32 pins that serve the bus.

## Step 11: Use a Ground Fill To Simplify Returns

The PCB uses a large copper fill on the board, which helps with:

- giving signals an easy return path
- reducing routing clutter
- making the board easier to complete in a compact outline

Even with a fill, I still treated the power side and logic side differently in placement so the fan-current path would not dominate the sensor area.

## Step 12: Add Test-Friendly Connectors and Labels

A new user should be able to look at the board and understand where the important rails are, so I kept visible labels for:

- `VIN`
- `VIN_GND`
- `VOUT`
- `VOUT_GND`
- `12V`
- `TRIG/PWM`
- `SDA`
- `SCL`

That makes bring-up much easier with a multimeter.

## Step 13: Match the Firmware to the Hardware

This hardware was not designed in isolation. The board layout follows the firmware architecture already present in the repo:

- `GPIO3` is configured for PWM fan output in [main/app_driver.c](C:/esp/v5.4.4/fan_controller/main/app_driver.c:25)
- `GPIO6` and `GPIO7` are used as the shared sensor I2C bus in [main/app_main.c](C:/esp/v5.4.4/fan_controller/main/app_main.c:352)
- the software expects `VEML7700`, `ENS160`, and `AHT21` on that bus

That means a new user does not need to guess the intended wiring. The code already documents the electrical plan.

## Step 14: Bring Up the Board Safely

If you are recreating this board, test it in this order:

1. Check continuity between power and ground before powering anything.
2. Power only the low-voltage section first and verify the regulated rail.
3. Confirm the `ESP32-C3` boots by USB or board power.
4. Check that the I2C devices respond.
5. Test the `TRIG/PWM` output without the full fan load if possible.
6. Connect the fan and verify control behavior.

Once the hardware is ready, you can build and flash the firmware with:

```powershell
. C:\esp\v5.4.4\export.ps1
cd C:\esp\v5.4.4\fan_controller
idf.py set-target esp32c3
idf.py build
idf.py -p COM3 flash monitor
```

## Step 15: What I Would Tell a New User Before Copying It Exactly

This board reflects the current implementation shown in the schematic and PCB screenshot, but there are a few things worth double-checking before manufacturing a revision:

- verify the exact input-voltage expectations of your `ESP32-C3 DevKitM-1`
- verify the output setting of the `LM2596` module before connecting the ESP32 and sensors
- confirm the fan-control module matches your fan type
- confirm your sensor modules use the same I2C addresses expected by the firmware

That is not a problem with the concept of the design. It is just good hardware practice whenever a board mixes modules from different vendors.

## Why I Laid Out the Board This Way

In one sentence: I separated the board by job.

- power and fan current on one side
- controller in the middle
- sensors on the opposite side

That made the routing easier, the schematic easier to explain, and the firmware mapping much cleaner.

## Files Relevant to the Hardware/Firmware Match

- [README.md](C:/esp/v5.4.4/fan_controller/README.md)
- [main/app_priv.h](C:/esp/v5.4.4/fan_controller/main/app_priv.h:16)
- [main/app_driver.c](C:/esp/v5.4.4/fan_controller/main/app_driver.c:25)
- [main/app_main.c](C:/esp/v5.4.4/fan_controller/main/app_main.c:24)

## Final Notes

If you are new to this project, the main thing to understand is that the schematic, PCB, and firmware were all built around the same three ideas:

- `12V` fan-side power stays separate from delicate logic signals
- the `ESP32-C3` acts as the center of control
- all sensors share one clean I2C bus

If you preserve those three ideas, you can shrink the board, change modules, or redesign the layout and still keep the same overall system working.
