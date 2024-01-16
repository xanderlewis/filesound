# Listen to your data &mdash; convert arbitrary files to WAVE audio
Writes given file data to the data section of a WAVE file with the desired name (creating or overwriting as necessary). Repeats each ~~byte~~ *sample-length sequence of bytes* **stretch factor** many times in the output in order to aid perception (at a typical modern sample rate most data will just sound like noise).

## Usage
Compile the source and invoke in the form
`./filesound [-s bytes_per_second] [source_file] [target_file] [stretch_factor]`

The `-s` flag tells filesound to turn each byte in the source file to a pure sine tone (of duration 1 / `bytes_per_second` seconds); with no flags, filesound defaults to writing the raw bytes into the file (each `stretch_factor` many times).

If no arguments are given, it defaults to reading from STDIN and writing to STDOUT (for use as part of a pipeline). For example:

`for i in {1..1000}; do ps -A; sleep 0.01; done | ./filesound > processes.wav`

`xxd ~/somefile | ./filesound > dump.wav`

or go crazy...

`cat ~/somefile | xxd | xxd | xxd | ./filesound > iterateddump.wav`

### PROTIP:
- try listening to bitmapped images &mdash; they sound much cooler than more sophisticated formats.
- try running filesound on /bin/bash or some other binary. maybe even run it on itself!
- sounds cool on stuff from /var/log/

### IDEA
It *might* actually be a nicer idea, particularly for use on the command line, to split this up into programs that handle each specific task:
- one program that takes in bytes and creates the appropriate header (basically just wraps the raw bytes into a WAVE file)
- another that converts bytes to sine tones
- who knows.. maybe another that converts bytes to some other kind of representation

Then we'd be able to do things like
`[some raw data coming in here --> ] | sine | wave > file.wav`
