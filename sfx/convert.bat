for %%A in (*.wav) do (
   sox %%A -r 44000 -s -b 16 -c 1 %%~nA.raw
)
pause