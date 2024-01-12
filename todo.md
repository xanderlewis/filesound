# TODO FOR FILE --> WAV APP
- write a function for each subchunk (it takes the relevant params including amount of data and 'returns' the bytes that make up that subchunk
- work out how to write the file's bytes into the new file and seek back to put in the correct info into the initial subchunk(s) (of course, counting bytes whilst reading and writing them (potentially with a stretch factor)) (fseek from <stdio.h>?)

- i'd like the program to accept: source file [target file] [stretch factor]

if target file is provided, we write to it (possibly creating/overwriting it)

if not, we write to stdout.

...maybe?

stretch factor is also optional. if not present, use stretch factor 1.
