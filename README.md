# Listen to your data -- convert arbitrary files to WAVE audio

Compile the source and invoke in the form `filesound source file [target file] [stretch factor]`.

Writes the bytes in **source file** to the data section of a WAVE file with the desired name (creating or overwriting as necessary). Repeats each byte **stretch factor** many times in the output in order to aid perception (at a typical modern sample rate most data will just sound like noise).
