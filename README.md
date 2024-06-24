# Spectral <img src="src/res/img/noto_1f47b.png" width="5%" height="5%" />
Sinclair ZX Spectrum emulator from the 80s.

![image](https://github.com/r-lyeh/spectral/assets/35402248/8ae5f8d4-0a7c-41ee-9112-2e86bacdb262)
![image](https://github.com/r-lyeh/Spectral/assets/35402248/2575bc40-d19d-43a3-81d3-6d638e9a94d1)

## About
Spectral is an experimental emulator that I have been randomly assembling [since the pandemic days](https://twitter.com/r_rlyeh/status/1280964279903158273), inspired by my old fZX32 emulator. Accuracy and performance are long-term goals, but the primary focus is just having fun with this thing. Hardcore ZX users will find little value in this emulator right now, but I hope newbies may find its ease of use somehow appealing to try.
That being said, Spectral has a very compatible TAP/TZX loader and some other interesting features that provide me some fun in these days.
Code is highly experimental and prone to change in the future. I will keep altering/breaking things randomly for good sake.

## Features and wishlist
- [x] Single executable.
- [x] Z80. Z80 Disassembler.
- [x] 16, 48, 128, +2, +2A, +3, Pentagon128 models.
- [x] Issue 2/3 keyboards.
- [x] ULA/ULA+ graphics.
- [x] Beeper/AY chips.
- [x] Kempston mouse. <!-- @todo: AMX mouse.-->
- [x] Kempston/Fuller/Cursor/Sinclair joysticks.
- [x] RF/CRT experience (not physically accurate though).
- [x] TAP/TZX/CSW tapes. Z80/SNA snaps. ROM/IF2 roms. <!-- @todo: tzx info on window title -->
- [x] DSK/EDSK/TRD/SCL/FDI/MGT/IMG/HOBETA disks.
- [x] SCR/PNG screenshots. <!-- ulaplus screenshots. video recording -->
- [x] ZIP/RAR/GZ archives.
- [x] AY tunes player.
- [x] µ765/Betadisk interfaces.
- [x] Auto load games. Auto play/stop tape. TurboROM.
- [x] Game browser. <!-- @todo: rewrite this -->
- [x] Graphical tape browser.
- [x] 50/60Hz fps lock.
- [x] Run-a-head.
- [x] POK support.  <!-- @todo: cheats finder --> 
- [x] Gunstick, Lightgun. <!-- Cheetah Defender Lightgun, Magnum Light Phaser, Stack Light Rifle -->
- [x] External shaders support.
- [x] Internal savestates.
- [x] Graphical User Interface.
- [x] Portable: Windows, Linux, MacOS.
- [x] Embedded ZXDB.
- [ ] Gallery marquee. <!-- Flex. Tape cases. ZX catalog on demand. -->
- [ ] Extra accurate Z80 backend. <!-- @todo: contended mem, contended ports, memptr, snow, Q, floating bus (+2a/+3) -->
- [ ] Cycle accurate (border, multicolor, etc).
- [ ] RZX support. <!-- @todo: rzx loadsave http://ramsoft.bbk.org.omegahg.com/rzxform.html -->
- [ ] Gamepad support. <!-- Invert joystick/mouse axes/buttons -->
- [ ] MP3s.
- [ ] Netplay.
- [ ] Optimized.
- [x] Unlicensed.

## Downloads
Download any binary release from the [bin/ folder](bin/).

Alternatively, you can build the emulator:
- Windows users double click `MAKE.bat` file.
- OSX/Linux users can run `sh MAKE.bat`.

## Usage
Spectral can be configured with a mouse.

Here are some keyboard shortcuts, though:
- ESC: Game browser
- F1: CPU throttle (hold)
- F2: Start/stop tape
- F3/F4: Rewind/advance tape
- F5: Reload game
- F6: Toggle input latency (Run-a-head)
- F7: Toggle issue 2/3
- F8: Toggle tape speed
- F9: Toggle TV/RF (4 modes)
- F9+SHIFT: Toggle AY core (2 modes)
- F11/F12: Quick save/load
- ALT+ENTER: Fullscreen
- TAB+CURSORS: Joysticks

## Credits
Andre Weissflog, for their many single-header libraries! (Zlib licensed). Peter Sovietov and wermipls, for their accurate AY chip emulator (MIT licensed). Ulrich Doewich and Colin Pitrat, for their uPD765A floppy disk controller (GPL licensed). Marat Fayzullin for their WD1793/FDI controllers (Proprietary). Sergey Bulba for their ay2sna tool (Public Domain). Potapov Vsevolod Viktorovich for their rusfaq website. Andrew Owen and Geoff Wearmouth for their custom ROMs. Simon Owen for their DSK technical websites. Santiago Romero, Philip Kendall, James McKay for their FOSS emulators. Damian Vila for their BESCII truetype font (CC-1.0). lalaoopybee, for their lovely tube shader. Günter Woigk, Juan Carlos González Amestoy and David Colmenero for their floppy sound recordings. The ZXDB devs. The ZX Spectrum Discord folks. All the ZX community!

## Unlicense
This software is released into the [public domain](https://unlicense.org/). Also dual-licensed as [0-BSD](https://opensource.org/licenses/0BSD) or [MIT (No Attribution)](https://github.com/aws/mit-0) for those countries where public domain is a concern (sigh). Any contribution to this repository is implicitly subjected to the same release conditions aforementioned.

## Links
- [Introduction to the ZX Spectrum](https://en.wikipedia.org/wiki/ZX_Spectrum), entry on Wikipedia.
- [SpecEmu](https://specemu.zxe.io/), my favourite ZX emulator on Windows.
- [SpectrumComputing](https://spectrumcomputing.co.uk/), [WorldOfSpectrum](https://worldofspectrum.net/), [ZXArt](https://zxart.ee/), [Virtual TRDOS](https://vtrd.in/) and [ZXInfo](https://zxinfo.dk/) are the best online resources (imho).
- [Crash](https://archive.org/details/crash-magazine), [YourSinclair](https://archive.org/details/your-sinclair-magazine), [SinclairUser](https://archive.org/details/sinclair-user-magazine) and [MicroHobby(ES)](https://archive.org/details/microhobby-magazine) are great old paper magazines.
- [ZXDB](https://github.com/zxdb/ZXDB), [game maps](https://maps.speccy.cz/), [game cheats](https://www.the-tipshop.co.uk/) and [game longplays](https://www.youtube.com/@rzxarchive).
- [Daily ZX videos](https://www.youtube.com/results?search_query=zx+spectrum&sp=CAI%253D), on YouTube.

[![](https://github.com/r-lyeh/Spectral/actions/workflows/build.yml/badge.svg)](https://github.com/r-lyeh/Spectral/actions/workflows/build.yml)
