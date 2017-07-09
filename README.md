# x11_gl_alsa_freetype_challenge
Making a game for Linux with the system-provided APIs, in two days.

**NOTE**: For reasons against my will, Day 2 was actually split into small periods of time the few following days. After that, I completely forgot to update the README, and today I've forgotten too many details to complete this seriously.  
The remainder of the README was written by the "Day-2" me who knew a lot more at the time.  

# The challenge

Make a game for Linux with no "external" run-time dependencies such as
SDL2, within a normal week-end.

At least C11 with the `-Wall` option or similar (and compilation should not
generate warnings);

Portability should be considered, but is not mandatory;

Write everything, including the Makefile, by hand;

When looking for docs/references, try each of these in order :
1. man pages;
2. The sources for official examples (e.g `xev`, `aplay`);
3. Official docs on the Web;
4. The source of external libraries (e.g SDL2, SFML);
5. Other online resources such as StackOverflow and forums.

Avoid copy-and-pasting.

More or less follow Rust's style with regards to case for type vs. variable
names, etc.

# Day 1

The game's codename is "dsky", short for "dracosky", a far earlier attempt at
a game jam with SDL2.

The game successfully:
- Creates and configures an X11 window and cursor;
- Receives and handles events;
- Creates a modern (3.0+) compatibility-profile debug OpenGL context, and
  enables late-swap-tearing, or Vsync if it fails;
- Loads a 1024x1024 texture from raw RGBA data exported with GIMP;
- Compiles a basic GLSL 1.30 program and uses it to display the background 
  image;
- Loads a 16-bit PCM WAV file exported with Audacity;
- Plays the WAV file with ALSA's user-space API, using a blocking write on 
  a separate thread.

What's left to fix for day 2:
- The audio content is not mine, neither is the background image;
- The screen's size, the window's size, the texture's size, 
  some settings of the WAV and the GLX presence and version 
  are just assumed and hard-coded;
- Proper platform-specific code isolation is not achieved;

Extra features I'd like to add:
- Full-screen support;
- Camera shaking effect;
- Some text rendering;

The game avoids matrices like the plague because I'm too lazy to implement them,
but obviously I'd care if it was something I would ship.

# Dependencies
You should be fine _running_ the executable (if I distribute one)
out-of-the-box on amd64, however for compiling you'll need (provided I haven't left out anything):

- Ubuntu: `sudo apt install -y libx11-dev libasound2-dev libfreetype6-dev libgl1-mesa-dev
