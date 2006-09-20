#ifndef SkOSSound_DEFINED
#define SkOSSound_DEFINED

#include "SkTypes.h"

class SkOSSound {
public:
	static void Play(const char path[]);
	static void Pause();
	static void Resume();
	static bool TogglePause();	// returns true if we are now playing, or false if we're now paused
	static void Stop();

	//	volume runs from 0 (silent) to 0xFF (max-volume)
	static U8	GetVolume();
	static void SetVolume(U8CPU volume);
};

#endif

