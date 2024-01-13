# Listen to your data &mdash; convert arbitrary files to WAVE audio

Compile the source and invoke in the form `filesound source_file [target_file] [stretch_factor]`.

Writes the bytes in **source file** to the data section of a WAVE file with the desired name (creating or overwriting as necessary). Repeats each ~~byte~~ sample-length sequence of bytes **stretch factor** many times in the output in order to aid perception (at a typical modern sample rate most data will just sound like noise).

### PROTIP:
- try listening to bitmapped images &mdash; they sound much cooler than more sophisticated formats.
- try running filesound on /bin/bash or some other binary. maybe even run it on itself!
