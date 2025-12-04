#ifndef __SOUND_H__
#define __SOUND_H__

#define SILENCE_SONG 0
#define THEMEA_SONG 1
#define TITLE_SONG 2

#define SILENCE_SFX 0
#define CLEAR_SFX 1
#define CLEAR4_SFX 2
#define GAME_OVER_SFX 3
#define GAME_START_SFX 4
#define MOVE_SFX 5
#define ROTATE_SFX 6
#define PIECE_LOCK_SFX 7
#define SELECT_OPTION_SFX 8
#define SWITCH_OPTION_SFX 9
#define STAGE_CLEAR_SFX 10


void init_audio();
void play_audio(int id, int is_sfx);

#endif