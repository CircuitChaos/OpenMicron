# Open Micron

An utility to program **CRT Micron UV** (and possibly **AnyTone AT-778UV** and **Retevis RT-95** too) transceivers under Linux.

## Objective

**CRT Micron UV** is an inexpensive dual-band 2m and 70cm FM transceiver. It is possible to order (or make: http://iz3zvo.altervista.org/build-program-cable-anytone-at-5888uv-intek-hr-2040/) a cable for it and program it from the computer.

Unfortunately, at the time of writing there's no known way to program these radios under Linux. There's a proprietary, Windows-only software available and it looks like some people succeeded running it under Wine, but then radio communication does not work. Even under Windows, adding and editing channels in this program is quite counterintuitive, to say the least.

Because of it, I decided to reverse-engineer the programming protocol and radio memory map and write a replacement for the proprietary software, especially as I need to be able to program my radio from the Raspberry Pi.

As far as I know, **CRT Micron UV** is also available under **AnyTone AT-778UV** and **Retevis RT-95** brand names, but I have absolutely no idea if this utility will be able to handle them. Testers are more than welcome!

## Disclaimer

This needs to be said. This software is experimental. It seems to work for me (I wouldn't release it otherwise), but it can, possibly, brick your transceiver, and I can't be held responsible for that. **This software comes with ABSOLUTELY NO WARRANTY.**

Know what you're doing and proceed at your own risk (but by all means report bugs!).

## How to install

The program is developed to work under Linux. Porting it to other POSIX-compatible systems shouldn't be hard (and might be done if there's enough interest in the project), but for now it was only tested under Linux.

To install it, you will need:

* **git** utility – to download the repository and to append commit hash to the software version string
* **scons** utility – scons is a nice, Python-based replacement for makefiles. It is needed to compile and install the program
* **g++** compiler with support for C++11 – as software is written in this C++ dialect
* **libcsv3** and **libcsv-dev** packages – as software needs them to parse .csv files (more on that below)

On Debian, Raspbian, Ubuntu and other Debian-based systems you should be able to install these packages with:

`apt-get install git scons g++ libcsv3 libcsv-dev`

To download, compile and install the program, you can use the following one-liner:

`git clone https://github.com/CircuitChaos/OpenMicron && cd OpenMicron && scons && sudo scons install`

If everything goes right, the program binary, called **omi**, will be installed in the */usr/local/bin* directory – although, if you don't want to install it system-wide, you don't have to (just skip the `&& sudo scons install` part above). In this case, the binary will be available in *build* directory of the downloaded repository.

## How to use

My idea was to separate radio communication from the memory editing. Therefore, **omi** contains four so-called *applets* (subprograms).

* `omi read` reads the radio memory into file (.omi file)
* `omi export` exports channels and configuration from the .omi file into .csv file (to be edited with your favorite spreadsheet editor; I'm using LibreOffice Calc) or a tab-separated text file (to be edited with your favorite text editor; might be useful in terminal-only setups)
* `omi import` combines the .omi file and edited .csv or text file and produces new .omi file with your changes
* `omi write` writes the new .omi file to the radio

Applet name has to be the first argument. After it, general and applet-specific arguments follow.

Issue `omi -h` to see the command line summary for all applets, and `omi <applet name> -h` (for example: `omi read -h`) to see more detailed usage of any given applet.

### Note on differential write mode

**omi write** allows you to use *-r* to specify an optional reference file. This file is the original file, as read by **omi read**, before any changes have been made with **omi import**, and can be used to upload only changes instead of full memory data, which considerably speeds up the process. If radio was not used or programmed between reading memory with **omi read** and using this dump as a reference for **omi write**, then everything should be fine, but if not, you can possibly end up with garbled memory and bricked radio. Proceed with caution.

### Note on radio progress bar

Note that the progress bar displayed on the radio during reading and writing is not fully reliable, as it displays 100% after channel table has been read. **omi read** reads full memory, so the progress bar will stay at 100% for some time. This is normal. To observe true progress on the computer, use *-v* option.

## Text and .csv file formats

Both text and .csv files exported by **omi export** and imported by **omi import** represent the same data in tabular form, just the internal file format is different, the former being more suited for console-based environments and the latter for editing in a spreadsheet editor.

Both files consist of lines (rows), and each line consists of different fields (represented as columns in a .csv file). First field in the line is the commmand and subsequent fields are its parameters. No command is mandatory – only these which are present will be processed.

### Text file format

This format is quite straightforward. Lines are separated by the newline character (**omi export** outputs lines with LF line endings, but **omi import** accepts both CRLF and LF endings) and fields by the single tabulation (0x09) character. There's no escaping of data within fields – it's not needed.

### .csv file format

CSV is a portable, comma-separated file format used to store tabular data. When importing it into LibreOffice or other spreadsheet editor, set the following CSV parameters:

* Separated by: comma
* Text delimiter: "
* Column type for all fields: Text (select all fields and select *Text* in *Column type* box)

### **comment** command

**comment** is used to denote a comment. Everything in comment line is ignored.

### **welcome message** command

### **key** command

**key** is used to assign specific functions to programmable keys. First field contains the key name, second – the function.

Key names:

* P1…P6 – radio keys from the default set
* p1…p6 – radio keys from the alternate set (switched by the **FUNC** key)
* PA…PD – microphone keys

Function names:

* **OFF** – disables the key (cannot be assigned to microphone keys)
* **A/B** – toggles between the primary and secondary channel; it can only be assigned to radio keys, as microphone has a designated A/B key
* **V/M** – toggles between the VFO and memory mode
* **SQL** – allows to adjust squelch level
* **VOL** – allows to adjust volume level with the knob; you can change this function to default and then VOL key will change to CH key (to select channel), but the name in Open Micron remains
* **POW** – cycles between power levels
* **CDT** – allows to select RX and TX decoders
* **REV** – toggles reverse function
* **SCN** – toggles scanning function
* **CAL** – ?
* **TAL** – ? (talkaround?)
* **BND** – cycles between channel bandwidths
* **SFT** – sets TX shift (direction and offset)
* **MON** – enables / disables monitor (it can be set in options if monitor should be momentary or permanent)
* **DIR** – toggles screen direction
* **TRF** – ?
* **RDW** – toggles dual watch

### **channel** command

First field contains the channel number. Subsequent fields can be empty for the channel to be removed, or can be set as follows:

* Channel frequency. It can either be a single frequency (for example, *144.800* for 144.800 MHz simplex channel), or a frequency with shift direction and offset (for example, *439.35-7.6* for the SR5WA repeater)
* RX decoder setting. Allowed values:
    * **off** – channel receiver is not locked with CTCSS or DCS
    * **ctcss:<value>** – channel receiver is locked with given CTCSS value (for example: *ctcss:127.3*). There's a special value, „def” (**ctcss:def**), which sets CTCSS to the value set in „Define CTCSS” field
    * **dcs:<value>** – channel receiver is locked with given DCS value, which can be normal (for example: *dcs:123*) or inverted (for example: *dcs:i123*)
* TX encoder setting. Values are the same as for the RX decoder
* Squelch mode. Allowed values: **carrier**, **ctcss**, **dcs**, **optsig** (for Optional Signaling)
* TX power. Allowed values: **off** (TX disabled), **low**, **medium**, **high*
* Channel bandwidth. Allowed values: **narrow** (12.5 kHz), **medium** (20 kHz), **wide** (25 kHz)
* Busy Channel Lockout. Allowed values: **off**, **rpt** (repeater), **busy**
* PTT ID. Allowed values: **dtmf:begin**, **dtmf:end**, **dtmf:both**, **5tone:begin**, **5tone:end**, **5tone:both**
* Optional Signaling. Allowed values:
    * **off** – disabled
    * **dtmf:M<value>** – use specified DTMF code (for example: dtmf:M01)
    * **5tone:<value> ** – use specified 5tone code (for example: 5tone:0)
* Define CTCSS. This is CTCSS used when CTCSS in encoder or decoder is set to **def**
* Flags. One of the following, colon-separated:
    * **scan** – set to enable scanning
    * **talk** – set to enable talkaround
    * **rev** – set to enable reverse frequency mode

## .omi file format

File format is explained in detail in *src/omifile.h* header file, so there's no need to duplicate it here. I'll write the more high-level description instead.

.omi file contains, among others:

* radio model name (as reported during reading)
* memory size (currently fixed to 16 KiB)
* starting offset (currently fixed to zero)

Before writing memory, software queries the radio for its model name and refuses to upload the .omi file if model name does not match one stored in the file (for example, if you have an .omi file from CRT Micron UV, but want to program AT-778UV with it). Maybe it's harmless and should be removed, maybe it's not. It needs to be verified.

.omi file header is 32 bytes long. Therefore, when working on the memory map and reading the .omi file, remove the header with `dd bs=32 skip=1 if=file.omi of=file.bin` and work with binary file instead.

## Reporting bugs

If something doesn't work as expected, please report an issue using the GitHub issue tracker, but be sure to run the utility with *-d* option to produce debug output. If your radio gets bricked, I might or might not be able to help, but by all means report what happened, so I can react somehow (write a huge warning here, add more protections to the software, or something else).

I didn't disassemble my radio (yet), but **if** it keeps its data in the serially-accessible EEPROM (like all other radios that I disassembled did), it might be possible to bring it back to life by externaly reprogramming the memory, with or without unsoldering. It will need to be investigated if such situation happens, but I can't promise anything.

## Radio configuration

At the time of writing, only channel configuration, welcome message and assignments of programmable keys can be programmed. Other configuration options can definitely be added, but it needs some work. If you need some particular option, please report it and I'll do my best.

## TODO (and call for volunteers!)

What's left to be done:

* Investigate the situation with **AnyTone AT-778UV**, **Retevis RT-95** and/or other similar radios. Does the software work with them? Is the memory map the same? It should be, but you can never know
* Write proper manpage and – if software proves to be reliable – work with Linux distribution maintainers to integrate it into their repositories
* Work with **Chirp** maintainers to add the radio to Chirp (the memory map I created should help!)
* Make the utility portable to other POSIX-compatible systems
* Add support for full radio configuration editing
* Confirm that 5Tone setting for Optional Signalling is kept in the same place as DTMF
* Add validation of RX and TX frequencies during import (if they fall within the range supported by the radio)
* Extensively test all functions (I'm not using DTMF or 5Tone, for instance)

If you want to help adding particular configuration function, the following will be most helpful:

1. Read radio memory with `omi read`
2. Read radio memory in the proprietary programming software
3. Make the change (only single change, so it's easy to single it out later) in configuration
4. Write radio memory in the proprietary programming software
5. Again read full radio memory with `omi read`
6. Send me two .omi files (or hex diff between them + model name) and information about what exactly did you change

Alternatively, you can always submit a pull request :)

## Further documentation

There's some documentation in *doc* directory which might be helpful.

* **comm-protocol.txt** – protocol used to communicate with the radio. I reverse-engineered it using PortMon utility on Windows.
* **memory-map.txt** – incomplete, reverse-engineered memory map of CRT Micron UV. It's still work in progress!
* **dat-file-format.txt** – incomplete, reverse-engineered format of the .dat file, used by the proprietary software. Probably not needed, but I did it before I had a chance to actually read the radio memory, so I keep it here.

## Contact with the author

I have a name and a HAM radio call sign, but I prefer to stay anonymous :) Please use the GitHub issue reporting system for contact.
