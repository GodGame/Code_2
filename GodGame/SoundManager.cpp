#include "stdafx.h"
#include "SoundManager.h"


CSoundManager::CSoundManager()
{
	
}


CSoundManager::~CSoundManager()
{
	FMOD_System_Close(m_pSystem);
	FMOD_System_Release(m_pSystem);
}


void CSoundManager::CreateBGSound(int nCount, string *SoundFileName)
{
	// 효과 사운드 생성
	m_nBGSoundCount = nCount;
	m_ppBGSound = new FMOD_SOUND*[nCount];
	m_ppBGChannel = new FMOD_CHANNEL*[nCount];
	for (int i = 0; i < nCount; i++)
		FMOD_System_CreateSound(m_pSystem, SoundFileName[i].data(),
			FMOD_LOOP_NORMAL, 0, &m_ppBGSound[i]);
	FMOD_System_CreateChannelGroup(m_pSystem, "BackGroundMusic", &m_pBGChannelGroup);
}

CSoundManager & CSoundManager::GetInstance()
{
	static CSoundManager instance; return instance;
}

void CSoundManager::Initialize()
{
	FMOD_System_Create(&m_pSystem);
	FMOD_System_Init(m_pSystem, 32, FMOD_INIT_NORMAL, NULL);
	LoadFile();
	CreateBGSound(m_vBGMusicList.size(), m_vBGMusicList.data());
	CreateEffectSound(m_vEFFMusicList.size(), m_vEFFMusicList.data());
}

void CSoundManager::LoadFile()
{
	ifstream BGMFile("BGMList.txt");		// 불러들일 파일 이름을 통한 ifstream 정의
	string BGMName;

	if (!BGMFile) {						// 파일 열기 실패
		cerr << "파일을 열수 없습니다. 파일이 존재하는지 확인해주세요." << endl;
		exit(EXIT_FAILURE);
	}
	m_vBGMusicList.clear();					// 데이터를 받기전 기존 벡터 데이터 삭제
	cout << "------------ BackGroundMusic Load ------------" << endl;
	while (!BGMFile.eof()) {

		BGMFile >> BGMName;
		cout << "Load File [ " << BGMName << " ]" << endl;
		m_vBGMusicList.push_back(BGMName);
	}
	cout << "------------ Effect Sound Load ------------" << endl;
	ifstream EffectSound("EffectSoundList.txt");		// 불러들일 파일 이름을 통한 ifstream 정의
	string EffectName;
	if (!EffectSound) {						// 파일 열기 실패
		cerr << "파일을 열수 없습니다. 파일이 존재하는지 확인해주세요." << endl;
		exit(EXIT_FAILURE);
	}
	m_vEFFMusicList.clear();					// 데이터를 받기전 기존 벡터 데이터 삭제
	while (!EffectSound.eof()) {

		EffectSound >> EffectName;
		cout << "Load File [ " << EffectName << " ]" << endl;
		m_vEFFMusicList.push_back(EffectName);
	}
	cout << "사운드 파일을 정상적으로 불러왔습니다." << endl;
}

void CSoundManager::CreateEffectSound(int nCount, string *SoundFileName)
{
	// 백 그라운드 사운드 생성
	m_nEFSoundCount = nCount;
	m_ppEFFSound = new FMOD_SOUND*[nCount];

	for (int i = 0; i < nCount; i++)
		FMOD_System_CreateSound(m_pSystem, SoundFileName[i].data(),
			FMOD_DEFAULT, 0, &m_ppEFFSound[i]);
	FMOD_System_CreateChannelGroup(m_pSystem, "Effect", &m_pEFFChannelGroup);
}

void CSoundManager::PlaySoundEffect(int nIndex)
{
	if (nIndex < m_nEFSoundCount)
	{
		FMOD_CHANNEL *pChannel;
		FMOD_System_PlaySound(
			m_pSystem,
			m_ppEFFSound[nIndex],
			m_pEFFChannelGroup,
			0,
			&pChannel
		);
	}
}

void CSoundManager::PlaySoundBG(int nIndex)
{
	if (nIndex < m_nBGSoundCount)
	{
		FMOD_CHANNEL *pChannel;
		FMOD_System_PlaySound(
			m_pSystem,
			m_ppBGSound[nIndex],
			m_pBGChannelGroup,
			0,
			&pChannel
		);
	}
}

void CSoundManager::StopSoundBG(int nIndex)
{
	if (nIndex < m_nBGSoundCount)
	{
		//	FMOD_Channel_Stop(m_ppBGChannel[nIndex]);
		FMOD_ChannelGroup_Stop(m_pBGChannelGroup);
	}
}

void CSoundManager::ReleaseSound()
{
	int i;

	delete[] m_ppBGChannel;

	for (i = 0; i < m_nBGSoundCount; i++)
		FMOD_Sound_Release(m_ppBGSound[i]);
	delete[] m_ppBGSound;

	for (i = 0; i < m_nEFSoundCount; i++)
		FMOD_Sound_Release(m_ppEFFSound[i]);
	delete[] m_ppEFFSound;
}

void CSoundManager::Update()
{
	if (!m_pSystem)
		FMOD_System_Update(m_pSystem);
}