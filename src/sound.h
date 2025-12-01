#ifndef __SOUND_H__
#define __SOUND_H__

#define SILENCE_SONG 0
#define SONGA_SONG 1
#define SONGB_SONG 2
#define SONGC_SONG 3
#define TETO_SONG 4
#define END_SONG 5
#define TITLE_SONG 6

#define SILENCE_SFX 0
#define CLEAR_SFX 1
#define CLEAR4_SFX 2
#define GAMEOVER_SFX 3


void init_audio();
void play_audio(int id, int is_sfx);

#endif