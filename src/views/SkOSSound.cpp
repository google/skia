#include "SkOSSound.h"

#ifdef SK_BUILD_FOR_WIN

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#include <Mmreg.h>
#if defined _WIN32 && _MSC_VER >= 1300	// disable nameless struct/union
#pragma warning ( push )
#pragma warning ( disable : 4201 )
#endif
#include <Mmsystem.h>
#if defined _WIN32 && _MSC_VER >= 1300	
#pragma warning ( pop )
#endif
#include <stdio.h>

class CWaveFile {
public:
	BOOL Open(const char path[]);
	void Close();

	long Read(char* pData, long nLength);

	long GetLength() const {return m_nLength;}
	WAVEFORMATEX* GetWaveFormat()	{return (&m_Format);}

protected:
	FILE* m_pFile;
	long m_nLength;
	WAVEFORMATEX m_Format;

private:
	enum {
		WF_OFFSET_FORMATTAG	=		20,
		WF_OFFSET_CHANNELS =			22,
		WF_OFFSET_SAMPLESPERSEC =		24,
		WF_OFFSET_AVGBYTESPERSEC =	28,
		WF_OFFSET_BLOCKALIGN =		32,
		WF_OFFSET_BITSPERSAMPLE =		34,
		WF_OFFSET_DATASIZE =			40,
		WF_OFFSET_DATA =				44,
		WF_HEADER_SIZE = WF_OFFSET_DATA
	};
};

BOOL CWaveFile::Open(const char path[])
{
	BYTE aHeader[WF_HEADER_SIZE];

/*	hResInfo = FindResource (hInst, lpName, "WAVE"); 

	if (hResInfo == NULL) 
	return FALSE; 

	// Load the wave resource. 
	hRes = LoadResource (hInst, hResInfo); 

	if (hRes == NULL) 
		return FALSE; 

	// Lock the wave resource and play it. 
	lpRes = LockResource (0);
*/


	// open file
//	m_pFile = _tfopen(szFileName, TEXT("rb"));
	m_pFile = fopen(path, "rb");
	if (!m_pFile) {
		return FALSE;
	}

	// set file length
	fseek(m_pFile, 0, SEEK_END);
	m_nLength = ftell(m_pFile) - WF_HEADER_SIZE;

	// set the format attribute members
	fseek(m_pFile, 0, SEEK_SET);
	fread(aHeader, 1, WF_HEADER_SIZE, m_pFile);
	m_Format.wFormatTag = *((WORD*) (aHeader + WF_OFFSET_FORMATTAG));
	m_Format.nChannels = *((WORD*) (aHeader + WF_OFFSET_CHANNELS));
	m_Format.nSamplesPerSec = *((DWORD*) (aHeader + WF_OFFSET_SAMPLESPERSEC));
	m_Format.nAvgBytesPerSec = *((DWORD*) (aHeader + WF_OFFSET_AVGBYTESPERSEC));
	m_Format.nBlockAlign = *((WORD*) (aHeader + WF_OFFSET_BLOCKALIGN));
	m_Format.wBitsPerSample = *((WORD*) (aHeader + WF_OFFSET_BITSPERSAMPLE));

	return TRUE;
}

void CWaveFile::Close()
{
	fclose(m_pFile);
}

long CWaveFile::Read(char* pData, long nLength)
{
	return fread(pData, 1, nLength, m_pFile);
}

////////////////////////////////////////////////////////////////////////////////////////

struct SkOSSoundWave {
	HWAVEOUT hwo;
	WAVEHDR whdr;
	DWORD dwOldVolume;
	CWaveFile waveFile;
	HANDLE hDoneEvent;
};

static SkOSSoundWave gWave;
static bool			 gWavePaused;
static U8			 gVolume;
static bool			 gInited = false;

static void init_wave()
{
	if (gInited == false)
	{
		gWave.hwo = NULL;
		gWavePaused = false;
		gVolume = 0x80;
		gInited = true;
	}
}

MMRESULT StartWave(const char path[], SkOSSoundWave* wave, U32 vol);
MMRESULT EndWave(SkOSSoundWave* wave);

#define MAX_ERRMSG 256

//#include "SkOSFile.h"	// for utf16

void SkOSSound::Play(const char path[]) 
{
	init_wave();

	if (gWave.hwo != NULL)
		SkOSSound::Stop();

	U32 v32 = (gVolume << 8) | gVolume;	// fill it out to 16bits
	v32 |= v32 << 16;					// set the left and right channels

	StartWave(path, &gWave, v32);
	gWavePaused = false;
}

bool SkOSSound::TogglePause() 
{
	init_wave();

	if (gWavePaused) 
		SkOSSound::Resume();
	else 
		SkOSSound::Pause(); 
	return !gWavePaused;
}


void SkOSSound::Pause() 
{
	init_wave();

	if (gWave.hwo == NULL || (gWave.whdr.dwFlags & WHDR_DONE))
		return;
	waveOutPause(gWave.hwo);
	gWavePaused = true;
}

void SkOSSound::Resume() 
{
	init_wave();

	if (gWave.hwo == NULL || (gWave.whdr.dwFlags & WHDR_DONE))
		return;
	waveOutRestart(gWave.hwo);
	gWavePaused = false;
}

void SkOSSound::Stop() 
{
	init_wave();

//	if (gWave.hwo == NULL || (gWave.whdr.dwFlags & WHDR_DONE))
	if (gWave.hwo == NULL)
		return;
	waveOutReset(gWave.hwo);
	EndWave(&gWave);
	gWavePaused = false;
	gWave.hwo = NULL;
}

U8 SkOSSound::GetVolume()
{
	init_wave();
	return gVolume;
}

void SkOSSound::SetVolume(U8CPU vol)
{
	if ((int)vol < 0)
		vol = 0;
	else if (vol > 255)
		vol = 255;

	init_wave();
	gVolume = SkToU8(vol);

	if (gWave.hwo)
	{
		unsigned long v32 = (vol << 8) | vol;	// fill it out to 16bits
		v32 |= v32 << 16;						// set the left and right channels
		waveOutSetVolume(gWave.hwo, v32);
	}
}

#if 0
unsigned long SoundManager::GetPosition()
{
	if (fWave.hwo == NULL)
		return 0;
	MMTIME time;
	time.wType = TIME_MS;
	if (waveOutGetPosition(fWave.hwo, &time, sizeof(time)) == MMSYSERR_NOERROR &&
		time.wType == TIME_MS)
	{
		return time.u.ms;
	}
	return 0;
}
#endif

MMRESULT StartWave(const char path[], SkOSSoundWave* wave, U32 vol)
{
	HWAVEOUT hwo = NULL;
//	WAVEHDR whdr;
	MMRESULT mmres = 0;
//	CWaveFile waveFile;
//	HANDLE hDoneEvent = wave.hDoneEvent = 
//		CreateEvent(NULL, FALSE, FALSE, TEXT("DONE_EVENT"));
	UINT devId;
//	DWORD dwOldVolume;

	// Open wave file
	if (!wave->waveFile.Open(path)) {
//		TCHAR szErrMsg[MAX_ERRMSG];
//		_stprintf(szErrMsg,	TEXT("Unable to open file: %s\n"), szWavFile);
//		MessageBox(NULL, szErrMsg, TEXT("File I/O Error"), MB_OK);
		return MMSYSERR_NOERROR;
	}

	// Open audio device
	for (devId = 0; devId < waveOutGetNumDevs(); devId++)
	{
		mmres = waveOutOpen(&hwo, devId, wave->waveFile.GetWaveFormat(), 0, 0, CALLBACK_NULL);
		if (mmres == MMSYSERR_NOERROR)
		{
			wave->hwo = hwo;
			break;
		}
	}
	if (mmres != MMSYSERR_NOERROR)
	{
		SkDEBUGCODE(SkDebugf("waveOutOpen(%s) -> %d\n", path, mmres);) 
		return mmres;
	}

	// Set volume
	mmres = waveOutGetVolume(hwo, &wave->dwOldVolume);
	if (mmres != MMSYSERR_NOERROR) {
		return mmres;
	}

	waveOutSetVolume(hwo, vol);
	if (mmres != MMSYSERR_NOERROR) {
		return mmres;
	}

	// Initialize wave header
	ZeroMemory(&wave->whdr, sizeof(WAVEHDR));
	wave->whdr.lpData = new char[wave->waveFile.GetLength()];
	wave->whdr.dwBufferLength = wave->waveFile.GetLength();
	wave->whdr.dwUser = 0;
	wave->whdr.dwFlags = 0;
	wave->whdr.dwLoops = 0;
	wave->whdr.dwBytesRecorded = 0;
	wave->whdr.lpNext = 0;
	wave->whdr.reserved = 0;

	// Play buffer
	wave->waveFile.Read(wave->whdr.lpData, wave->whdr.dwBufferLength);

	mmres = waveOutPrepareHeader(hwo, &wave->whdr, sizeof(WAVEHDR));	
	if (mmres != MMSYSERR_NOERROR) {
		return mmres;
	}

	mmres = waveOutWrite(hwo, &wave->whdr, sizeof(WAVEHDR));	
//	if (mmres != MMSYSERR_NOERROR) {
		return mmres;
//	}
}

#if 0
void IdleWave(Wave& wave)
{
	// Wait for audio to finish playing
	while (!(wave.whdr.dwFlags & WHDR_DONE)) {
		WaitForSingleObject(wave.hDoneEvent, INFINITE);
	}
}
#endif

MMRESULT EndWave(SkOSSoundWave* wave)
{
	HWAVEOUT hwo = wave->hwo;
	MMRESULT mmres;
	// Clean up
	mmres = waveOutUnprepareHeader(hwo, &wave->whdr, sizeof(WAVEHDR));	
	if (mmres != MMSYSERR_NOERROR) {
		return mmres;
	}

	waveOutSetVolume(hwo, wave->dwOldVolume);
	if (mmres != MMSYSERR_NOERROR) {
		return mmres;
	}

	mmres = waveOutClose(hwo);
	if (mmres != MMSYSERR_NOERROR) {
		return mmres;
	}

	delete [] wave->whdr.lpData;
	wave->waveFile.Close();

	return MMSYSERR_NOERROR;
}

#endif	/* SK_BUILD_FOR_WIN */

