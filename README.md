# splitmerge
A program that splits files into 8MB or 100MB chunks to transfer over discord and merge them back together.

----

# Usage
## splitmerge_split
Drag and drop the files into the executable. 
Or put the path to each file as an argument to the executable in the command line.  
Make sure that the folder `split_output` is in the same folder as the executable.  
  
Example: `splitmerge_split.exe my_file.wav`

## splitmerge_split_nitro
Same as splitmerge_split but it splits files into 100MB chunks instead of 8MB.

## splitmerge_merge
Drag and drop the split files into the executable.  
Or put the path to each split file as an argument to the executable in the command line.  
Make sure that the folder `merged_output` is in the same folder as the executable.  
  
Example: `splitmerge_merge.exe 0x7AF001C3_0.spltmrg 0x7AF001C3_1.spltmrg 0x7AF001C3_2.spltmrg`

----

# Compilation
Compile `splitmerge_split.c`, `splitmerge_split_nitro.c`, and `splitmerge_merge.c` separately.  
Definining `SPLITMERGE_WIN32` will use the Windows API instead of the C runtime library.  
Create the folders `split_output` and `merged_output` and make sure that they are in the same folder as their respective executable.
