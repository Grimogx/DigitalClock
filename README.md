# Digital Clock

A native Windows desktop clock and stopwatch made as a C microproject. The
application uses the Win32 API directly; it does not use GTK, another GUI
framework, or a separately installed application runtime.

## Features

- Live local time in 12-hour or 24-hour format, with AM/PM and date.
- Application-only time adjustment. The actual Windows system time is shown
  separately and is never changed.
- Stopwatch with start, pause, resume, and reset. Elapsed time is measured
  using the system's high-resolution performance counter.
- Six clock faces: minimal and classic digital, analog, Roman numeral analog,
  minimal analog, and creative digital.
- Light and dark themes.
- Clock, stopwatch, and settings screens.

Settings and the adjusted time last for the current run of the application.
Restarting restores the defaults.

## Technologies and project layout

- C11 for the application and time/stopwatch logic.
- The Windows SDK and native Win32 API (`windows.h`) for windows, controls,
  messages, and drawing.
- Microsoft Visual C compiler (`cl.exe`) for building.

```text
DigitalClock/
├── .vscode/
│   └── tasks.json
├── assets/
├── build/
├── src/
│   ├── main.c
│   ├── clock.c
│   ├── clock.h
│   ├── stopwatch.c
│   ├── stopwatch.h
│   ├── settings.c
│   ├── settings.h
│   ├── gui.c
│   └── gui.h
├── build.bat
└── README.md
```

## Windows setup

Install **Visual Studio Build Tools** with the **Desktop development with C++**
workload and a Windows SDK. The project itself is C; the workload provides
Microsoft's C compiler and linker.

Open **x64 Native Tools Command Prompt for Visual Studio** (or the matching
Developer Command Prompt for your target architecture), then change directory
to the project folder.

## Compile

Run the included build script:

```bat
build.bat
```

Equivalent direct compiler command:

```bat
if not exist build mkdir build
cl /nologo /std:c11 /W4 /O2 /MT /Isrc src\main.c src\clock.c src\stopwatch.c src\settings.c src\gui.c /Fe:build\DigitalClock.exe /link /SUBSYSTEM:WINDOWS user32.lib gdi32.lib
```

The finished application is `build\DigitalClock.exe`.

In VS Code, use **Tasks: Run Task** and select **Build Digital Clock** or
**Run Digital Clock**. These tasks expect the Visual Studio Developer Command
Prompt environment to be available.

## Run and distribute

Run `build\DigitalClock.exe`, or use the **Run Digital Clock** VS Code task.
The build uses `/MT` to statically link the Microsoft C runtime. The resulting
EXE does not need GTK, a GUI framework, or a Visual C++ redistributable DLL.
It uses Windows system libraries (`user32.dll`, `gdi32.dll`, and normal
Windows system components), which are part of Windows. The target computer
does not need a compiler or development tools installed.

## How the C modules work

### `clock.c` / `clock.h`

`Clock` stores a time offset in seconds. The display time is calculated by
adding this offset to the current timestamp. The unadjusted system time is
available separately. The GUI only changes this application value; it never
sets the Windows clock. On Windows, `localtime_s` converts timestamps to
local calendar time.

### `stopwatch.c` / `stopwatch.h`

The stopwatch stores its running state, accumulated elapsed time, and the
timestamp when its current run began. On Windows it uses
`QueryPerformanceCounter`, a monotonic high-resolution timer, so system clock
changes do not affect elapsed time. Pause saves the elapsed duration; resume
starts another timed interval.

### `settings.c` / `settings.h`

Enums store the selected time format, theme, and clock face. `settings_init`
sets the initial 12-hour, dark-theme, minimal-digital defaults.

### `gui.c` / `gui.h`

The GUI creates standard Win32 windows, buttons, static labels, and dropdown
controls. Its window procedure handles button clicks, setting changes, timer
messages, and resizing. A Windows timer refreshes the clock and stopwatch;
GDI draws the analog clock faces and styled controls.

### `main.c`

`WinMain` initializes the clock, stopwatch, and settings structs, then starts
the native Windows message loop through `gui_run`.
