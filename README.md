# Listen to your data &mdash; convert arbitrary files to WAVE audio
Writes the bytes in **source file** to the data section of a WAVE file with the desired name (creating or overwriting as necessary). Repeats each ~~byte~~ sample-length sequence of bytes **stretch factor** many times in the output in order to aid perception (at a typical modern sample rate most data will just sound like noise).

## Usage
Compile the source and invoke in the form `./filesound [-s] [source_file] [target_file] [stretch_factor]`.

The `-s` flag tells filesound to turn each byte in the source file to a pure sine of fixed duration (for the moment, not adjustable); with no flags, filesound defaults to writing the raw bytes (each `stretch_factor` many times).

If no arguments are given, it defaults to reading from STDIN and writing to STDOUT (for use as part of a pipeline). For example:

`for i in {1..1000}; do ps -A; sleep 0.01; done | ./filesound > processes.wav`

`xxd ~/somefile | ./filesound > dump.wav`

or go crazy...

`cat ~/somefile | xxd | xxd | xxd | ./filesound > iterateddump.wav`

### PROTIP:
- try listening to bitmapped images &mdash; they sound much cooler than more sophisticated formats.
- try running filesound on /bin/bash or some other binary. maybe even run it on itself!
- sounds cool on stuff from /var/log/
