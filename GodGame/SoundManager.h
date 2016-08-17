#pragma once

class CSoundManager
{
private:
	FMOD_SYSTEM* m_pSystem;
	FMOD_SOUND** m_ppBGSound;
	FMOD_SOUND** m_ppEFFSound;
	FMOD_CHANNEL** m_ppBGChannel;
	FMOD_CHANNEL** m_ppEFFChannel;
	FMOD_CHANNELGROUP* m_pBGChannelGroup;
	FMOD_CHANNELGROUP* m_pEFFChannelGroup;
	vector<string> m_vBGMusicList;
	vector<string> m_vEFFMusicList;
	int m_nEFSoundCount;
	int m_nBGSoundCount;

public:
	static CSoundManager & GetInstance();
	void Initialize();
	void LoadFile();
	void CreateEffectSound(int nCount, string *SoundFileName);
	void CreateBGSound(int nCount, string *SoundFileName);
	void PlaySoundEffect(int nIndex);
	void PlaySoundBG(int nIndex);
	void StopSoundBG(int nIndex);
	void ReleaseSound();
	void Update();
public:
	CSoundManager();
	~CSoundManager();
};

#define SOUND_MGR CSoundManager::GetInstance()
