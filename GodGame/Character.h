#pragma once

#ifndef __CHARACTER
#define __CHARACTER

#include "Object.h"
#include "GameInfo.h"
#include "EffectEntity.h"

class CCharacter : public CAnimatedObject
{
protected:
	XMFLOAT3 m_xv3BeforePos;
	XMFLOAT3 m_xv3Position;
	XMFLOAT3 m_xv3Right;
	XMFLOAT3 m_xv3Up;
	XMFLOAT3 m_xv3Look;

public:
	virtual XMFLOAT3 GetPosition() const { return m_xv3Position; }
	XMFLOAT3 & GetPosition()         { return(m_xv3Position); }
	XMFLOAT3 & GetLookVector()       { return(m_xv3Look); }
	XMFLOAT3 & GetUpVector()         { return(m_xv3Up); }
	XMFLOAT3 & GetRightVector()      { return(m_xv3Right); }

	XMFLOAT3 GetLookVectorInverse()  { return XMFLOAT3(-m_xv3Look.x, -m_xv3Look.y, -m_xv3Look.z); }
	XMFLOAT3 GetUpVectorInverse()    { return XMFLOAT3(-m_xv3Up.x, -m_xv3Up.y, -m_xv3Up.z); }
	XMFLOAT3 GetRightVectorInverse() { return XMFLOAT3(-m_xv3Right.x, -m_xv3Right.y, -m_xv3Right.z); }

	void SetDamagedEntity(CEntity * pEntity) { m_pDamagedEntity = pEntity; }
	CEntity * GetDamagedEntity() { return m_pDamagedEntity; }

protected:
	CEntity * m_pDamagedEntity;

	float m_fGravity;
	XMFLOAT3 m_xv3ExternalPower;
	//xz-��鿡�� (�� ������ ����) �÷��̾��� �̵� �ӷ��� �ִ밪�� ��Ÿ����.
	float    m_fMaxVelocityXZ;
	//y-�� �������� (�� ������ ����) �÷��̾��� �̵� �ӷ��� �ִ밪�� ��Ÿ����.
	float    m_fMaxVelocityY;
	//�÷��̾ �ۿ��ϴ� �������� ��Ÿ����.
	float    m_fFriction;
	//�÷��̾��� ��ġ�� �ٲ� ������ ȣ��Ǵ� OnPlayerUpdated() �Լ����� ����ϴ� �������̴�.
	LPVOID   m_pUpdatedContext;

	StatusInfo		m_Status;

public:
	void SetFriction(float fFriction)           { m_fFriction = fFriction; }
	void SetGravity(const float fGravity)		{ m_fGravity = fGravity; }
	void SetMaxVelocityXZ(float fMaxVelocity)   { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity)    { m_fMaxVelocityY = fMaxVelocity; }
	void SetExternalPower(XMFLOAT3 & xmf3Power);

	XMFLOAT3& GetExternalPower() { return m_xv3ExternalPower; }

	StatusInfo& GetStatus() {return m_Status;}
	void CalculateFriction(float fTimeElapsed);

public:
	CCharacter(int nMeshes);
	virtual ~CCharacter();

	virtual void BuildObject() {}
	virtual void InitializeAnimCycleTime(){}

	virtual void OnPrepareRender();
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera);
	virtual void Animate(float fTimeElapsed);

public:
	virtual void Attack(CCharacter * pToChar, short stDamage);
	virtual void AttackSuccess(CCharacter * pToChar, short stDamage);
	virtual void Damaged(CCharacter * pByChar, short stDamage);
	virtual void Reset() { m_Status.ResetStatus(); }
	virtual void Revive() { Reset(); }
	virtual void Collide(CEntity * pEntity) {}

public:
	virtual void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3& xv3Position);

	void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
	void Move(XMFLOAT3& xv3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);

	virtual void Rotate(float x, float y, float z);
	virtual void Rotate(XMFLOAT3 & xmf3RotAxis, float fAngle);

	void LookToTarget(const CEntity * pTarget);
	//��ġ�� ȸ�� ������ ��� �ð��� ���� �����ϴ� �Լ��̴�.
	void Update(float fTimeElapsed);

	//�÷��̾��� ��ġ�� �ٲ� ������ ȣ��Ǵ� �Լ��� �� �Լ����� ����ϴ� ������ �����ϴ� �Լ��̴�.
	virtual void OnContextUpdated(float fTimeElapsed);
	void SetUpdatedContext(LPVOID pContext) { m_pUpdatedContext = pContext; }
};

class CMonster : public CCharacter
{
private:
	float m_fUpdateTargetTime;
	bool m_bUpdateTaget;
	const float mfONE_CYCLE_UPDATE_TIME = 0.5f;

protected:
	CCharacter * m_pTarget;

public:
	CMonster(int nMeshes); 
	virtual ~CMonster();
	virtual void BuildObject(CCharacter * pTarget) {SetTarget(pTarget);}

public:
	void SetTarget(CCharacter * pTarget) { m_pTarget = pTarget; }
	CCharacter* GetTarget() { return m_pTarget; }

	void Update(float fTimeElapsed);
	virtual void GetGameMessage(CEntity * byObj, eMessage eMSG, void * extra = nullptr);
	virtual void Collide(CEntity * pEntity);
};

class CSkeleton : public CMonster
{
private:
	CStateMachine<CSkeleton>* m_pStateMachine;

public:
	CSkeleton(int nMeshes);
	virtual ~CSkeleton();

	CStateMachine<CSkeleton>* GetFSM() { return m_pStateMachine;}
};

class CDistanceEvaluator;


class CWarrock : public CMonster
{
private:
	const float mfIDLE_ANIM    = 1.0f;
	const float mfRUN_ANIM     = 1.2f;
	const float mfROAR_ANIM    = 3.0f;
	const float mfPUNCH_ANIM   = 1.0f;
	const float mfSWIP_ANIM    = 2.0f;
	const float mfDEATH_ANIM   = 2.0f;

private:
	const float mfMAX_HEALTH     = 50.0f;
	const float mfSWIPING_DAMAGE = 10.0f;
	const float mfPUNCH_DAMAGE   = 5.0f;

public:
	enum eWarrockAnim : WORD
	{
		eANI_WARROCK_IDLE = 0,
		eANI_WARROCK_RUN,
		eANI_WARROCK_ROAR,
		eANI_WARROCK_PUNCH,
		eANI_WARROCK_SWIPING,
		eANI_WARROCK_DEATH,
		eANI_WARROCK_ANIM_NUM
	};

private:
	CStateMachine<CWarrock>* m_pStateMachine;

	CDistanceEvaluator mEvaluator;
	//CTargetDotEvaluator mDotEvaluator;

public:
	CWarrock(int nMeshes);
	virtual ~CWarrock();

	virtual void BuildObject(CCharacter * pTarget);
	virtual void InitializeAnimCycleTime();

	//virtual void OnPrepareRender();
	//virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera);
	virtual void Animate(float fTimeElapsed);
	virtual void GetGameMessage(CEntity * byObj, eMessage eMSG, void * extra = nullptr);

public:
	virtual void Attack(CCharacter * pToChar, short stDamage);
	virtual void AttackSuccess(CCharacter * pToChar, short stDamage);
	virtual void Damaged(CCharacter * pByChar, short stDamage);
	virtual void Reset();

public:
	CStateMachine<CWarrock>* GetFSM() { return m_pStateMachine; }

	float GetPunchDamage()   { return mfPUNCH_DAMAGE; }
	float GetSwipingDamage() { return mfSWIPING_DAMAGE; }
};


#endif