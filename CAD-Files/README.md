# CAD Files

This folder contains the mechanical design files for the Toilet Whisper build.

## Files in This Folder

| File | Purpose |
|---|---|
| `CAD-Toilet-Whisper.f3z` | Main editable Fusion 360 project archive |
| `STL-Toilet-Whisper.zip` | Exported STL files for printing the physical parts |

## Who This Folder Is For

Use this folder if you want to:
- print the enclosure
- modify the enclosure in Fusion 360
- inspect how the physical parts fit together
- adapt the design for different hardware dimensions

## Recommended Workflow

1. Open `CAD-Toilet-Whisper.f3z` in Fusion 360 if you want to edit the design
2. Use `STL-Toilet-Whisper.zip` if you only want printable files
3. Dry-fit the printed parts with your PCB, ESP32 board, fan, and wiring before final assembly

## Printing Material

I printed the enclosure parts in `PETG` because the final device is intended for a humid, higher-temperature bathroom environment.

## Print Settings Used

These are the settings I used successfully:

- `0.2 mm` layer height
- `3` walls
- `4` top layers
- `4` bottom layers
- `25%` gyroid infill

These settings gave me a good balance between strength, print time, and durability for a bathroom-use enclosure.

## Hardware Used for Assembly

- `4 x M3 35 mm SHCS` for securing the fan to the main enclosure body
- `1 x M3 8 mm SHCS` for securing the PCB to the circuit board frame
- `2 x M3 8 mm SHCS` for fastening the enclosure feet to the assembled body and frame
- `2 x M3 8 mm SHCS` for securing the backplate

## Assembly Order

### Step 1: Mount the fan
Secure the fan to the main enclosure body using `4 x M3 35 mm SHCS`.

Make sure the fan inlet is facing toward the designed vent direction in the enclosure.

### Step 2: Mount the PCB
Secure the PCB onto the circuit board frame using `1 x M3 8 mm SHCS`.

### Step 3: Connect internal wiring
Connect the battery and fan connectors to the circuit board assembly before inserting it into the enclosure.

### Step 4: Install the electronics assembly
Insert the assembled circuit board frame into the enclosure body.

### Step 5: Attach the enclosure feet
Use `2 x M3 8 mm SHCS` to fasten the enclosure feet to the assembled enclosure body and circuit board frame.

### Step 6: Test before closing
Before installing the backplate, verify:
- battery connection
- fan connection
- board power-up
- overall device functionality

This is the best time to catch wiring issues without reopening the whole enclosure.

### Step 7: Close the enclosure
Once everything is working, install the backplate using `2 x M3 8 mm SHCS`.

## Summary

This folder contains both the editable Fusion 360 design and the STL exports used to print the Toilet Whisper enclosure, along with the print settings and assembly process used in the final build.
