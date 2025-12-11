#ifndef __SOUND_H__
#define __SOUND_H__

#define SILENCE_SONG 0
#define THEMEA_SONG 1
#define THEMEB_SONG 2
#define THEMEC_SONG 3
#define TITLE_SONG 4

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
#define TOUCH_SURFACE_SFX 11
#define GARBAGE_SFX 12
#define PAUSE_SFX 13
#define SOFT_DROP_SFX 14
#define GAME_WIN_SFX 15

#define SONG_VOLUME_DEFAULT 70
#define SFX_VOLUME_DEFAULT 100
extern int song_volume;
extern int sfx_volume;
extern bool song_paused;
extern int song_choice;

void init_audio();
void play_audio(int id, int is_sfx);

#endif