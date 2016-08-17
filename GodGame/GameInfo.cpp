#include "stdafx.h"
#include "GameInfo.h"
#include "ObjectsList.h"
#include "Protocol.h"
CDeBuff::CDeBuff()
{
	Reset();

	m_pMaster = nullptr;
}

CDeBuff::~CDeBuff()
{
}

void CDeBuff::Reset()
{
	m_eDebuff = 0;
	m_fPlusDebuffTime = 0.0f;
}

void CDeBuff::PlayerDelayMessage(eDeBuff eType, float fDebuffTime)
{
	if (m_pMaster)
	{
		m_DebuffMessageList.push_back(eType);
		void * DebuffMsg = &(*(--m_DebuffMessageList.end()));

		EVENTMgr.InsertDelayMessage(fDebuffTime, eMessage::MSG_DEBUFF_OFF, CGameEventMgr::MSG_TYPE_ENTITY,
			(void*) this, nullptr, DebuffMsg);
	}
}

void CDeBuff::DebuffPop()
{
	//eDeBuff debuff = *((eDeBuff*)msg);
	auto it = m_DebuffMessageList.begin();
	m_DebuffMessageList.pop_front();
	m_eDebuff &= ~(*it);
}

///////////////////////////////////////////////////////////////////
CBuff::CBuff()
{
	Reset();

	m_pMaster = nullptr;
}

CBuff::~CBuff()
{
}

void CBuff::Reset()
{
	m_eBuff = 0;
	m_fPlusDebuffTime = 0.0f;
}

void CBuff::PlayerDelayMessage(eBuff eType, float fBuffTime)
{
	if (m_pMaster)
	{
		m_BuffMessageList.push_back(eType);
		void * DebuffMsg = &(*(--m_BuffMessageList.end()));

		EVENTMgr.InsertDelayMessage(fBuffTime, eMessage::MSG_BUFF_OFF, CGameEventMgr::MSG_TYPE_ENTITY,
			(void*) this, nullptr, DebuffMsg);
	}
}

void CBuff::BuffPop()
{
	//eDeBuff debuff = *((eDeBuff*)msg);
	auto it = m_BuffMessageList.begin();
	m_BuffMessageList.pop_front();
	m_eBuff &= ~(*it);
}

//////////////////////////////////////////////
StatusInfo::StatusInfo()
{
	ResetStatus();
}

StatusInfo::~StatusInfo()
{
}

void StatusInfo::ResetStatus()
{
	m_uGamePoint    = 0;
	m_uGold         = 0;

	m_uStamina      = 0;
	m_nShields      = 0;
	m_uAttackDamage = 0;
	m_bRange        = false;
	m_bUnbeatable   = false;
	m_bCanJump      = true;
	m_bCanMove      = true;
	m_bAlive        = true;
	m_sHP           = 0;

	m_fAttackSpeed  = 0.0f;
	m_fAttackRange  = 0.0f;

	m_Buff.Reset();
	m_Debuff.Reset();
}

void StatusInfo::RoundReady()
{
	m_bUnbeatable = true;
	m_bCanMove = false;
	m_bCanJump = false;
}

void StatusInfo::RoundStart()
{
	m_bUnbeatable = false;
	m_bCanMove = true;
	m_bCanJump = true;
}

///////////////////////////////////////////

ostream& operator<<(ostream& os, ElementEnergy & element)
{
	os << "�� " << (UINT)element.m_nSum << " // ";
	os << "�� : " << (UINT)element.m_nEnergies[ELEMENT_LIGHT] << " // ";
	os << "�� : " << (UINT)element.m_nEnergies[ELEMENT_FIRE] << endl;
	os << "�ٶ� : " << (UINT)element.m_nEnergies[ELEMENT_WIND] << " // ";
	os << "���� : " << (UINT)element.m_nEnergies[ELEMENT_ICE] << " // ";
	os << "��� : " << (UINT)element.m_nEnergies[ELEMENT_DARK] << " // ";
	os << "���� : " << (UINT)element.m_nEnergies[ELEMENT_ELECTRIC];
	return os;
}
