g:\local\tc\bin\tcc -c -DTURBOC -Iv: -Ig:\local\tc\include v:*.c
g:\local\tc\tasm /Mx v:*.asm
copy v:proto.lib xinu.lib
for %%j in (*.obj) do g:\local\tc\bin\tlib /C xinu.lib+%%j
