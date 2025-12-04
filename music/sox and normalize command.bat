mkdir tempaudio
for %%I in (*.flac) do ffmpeg-normalize "%%I" -o "tempaudio\%%~nI.wav" -lrt 2.5
for %%I in (tempaudio\*.wav) do sox "%%I" -R -G -b 8 -c 1 "%%~nI.wav" rate -v -L 44100 dither
for %%I in (*.wav) do wav_to_header.py "%%I" "%%~nI.h"
for %%I in (*.wav) do del "%%I"
rmdir /s /q tempaudio