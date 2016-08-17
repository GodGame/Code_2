#pragma once

#ifndef __COLLISION_H
#define __COLLISION_H

#include "stdafx.h"

enum ePartition { NONE = -1, CULL_OUT = 0, CONTAIN_PART, CONTAIN_ALL };

struct MoveVelocity
{
	XMFLOAT3 xmf3InitPos;
	XMFLOAT3 xmf3Velocity;
	XMFLOAT3 xmf3Accelate;
	float	 fWeightSpeed;

	MoveVelocity()
	{
		xmf3InitPos  = XMFLOAT3(0, 0, 0);
		xmf3Velocity = XMFLOAT3(0, 0, 0);
		xmf3Accelate = XMFLOAT3(0, 0, 0);
		fWeightSpeed = 1.0f;
	}
};

class AABB
{
public:
	//�ٿ�� �ڽ��� �ּ����� �ִ����� ��Ÿ���� �����̴�.
	XMFLOAT3 m_xv3Minimum;
	XMFLOAT3 m_xv3Maximum;

public:
	AABB() { m_xv3Minimum = XMFLOAT3(+FLT_MAX, +FLT_MAX, +FLT_MAX); m_xv3Maximum = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX); }
	AABB(XMFLOAT3 xv3Minimum, XMFLOAT3 xv3Maximum) { m_xv3Minimum = xv3Minimum; m_xv3Maximum = xv3Maximum; }

	//�� ���� �ٿ�� �ڽ��� ���Ѵ�.
	void Union(XMFLOAT3& xv3Minimum, XMFLOAT3& xv3Maximum);
	void Union(AABB *pAABB);
	//�ٿ�� �ڽ��� 8���� �������� ��ķ� ��ȯ�ϰ� �ּ����� �ִ����� �ٽ� ����Ѵ�.
	void Update(XMFLOAT4X4 & xmtxTransform, AABB * bbMesh = nullptr);
	void Update(XMFLOAT3 & xmvPostion, float uSize);

	static bool CollisionAABB(AABB & one, AABB & two);
	static ePartition IsIncludeAABB(AABB & bbSmall, AABB & bbLarge, bool bCollideCheck);
	static bool CollisionAABBBy2D(AABB & one, AABB & two);
	static ePartition IsIncludeAABBBy2D(AABB & bbSmall, AABB & bbLarge, bool bCollideCheck);

};

ostream& operator<<(ostream& os, AABB & bb);

class CEntity;

struct CPartitionNode
{
	unsigned int		 m_uIndex;
	AABB				 m_BoundingBox;
	vector<CEntity*> m_vpObjects;
};

class DirectQuadTree
{
	CPartitionNode * m_pNodesArray;
	unsigned int m_nTreeLevels : 8;
	unsigned int m_nMapWidth : 12;
	unsigned int m_nMapLength : 12;

	unsigned int m_nNodes;

	float m_fXScaleInverse;
	float m_fZScaleInverse;

public:
	DirectQuadTree();
	~DirectQuadTree();

	void BuildQuadTree(BYTE nTreeLevels, UINT MapWidth, UINT MapLength);

public:
	int GetNodeContainingIndex(AABB & bb);

	CPartitionNode * GetNodeContaining(AABB & objBoundingBox);
	CPartitionNode * GetNodeContaining(float fLeft, float fRight, float fNear, float fFar);

	int EntryObject(CEntity * pObject);
	void EntryObjects(CEntity ** ppObjectArrays, int nObjects);
public:
	//CPartitionNode * GetChildNode(int index);
};

enum Location{ LOC_NONE = -1, LOC_LB, LOC_RB, LOC_LT, LOC_RT, LOC_PARENT, LOC_ALL };

class CCamera;
class QuadTree
{
public:

private:
	list<CEntity*> m_vpObjectList;
	QuadTree * m_pNodes[5];

private:
	UINT m_uHalfWidth : 15;
	UINT m_uHalfLength : 15;
	UINT m_bLeaf : 1;
	bool m_bCulled : 1;

	XMINT3 m_xmi3Center;

public:		
	UINT  m_uTreeNum : 16;
	UINT  m_uTreeLevel : 16;

	QuadTree();
	~QuadTree();

	void BuildNodes(XMFLOAT3 & xmf3Center, UINT uWidth, UINT uLength, QuadTree * pParent, UINT uLevel);
	static QuadTree * CreateQuadTrees(XMFLOAT3 & xmf3Center, UINT uWidth, UINT uLength, QuadTree * pParent, UINT uLevel);

	QuadTree * GetParentNode() { return m_pNodes[LOC_PARENT]; }

public:
	void FrustumCulling(CCamera * pCamera);
	void PreCutCulling();
	bool IsCulled() { return m_bCulled; }

	Location IsContained(CEntity * pObject, bool bCheckCollide);
	void FindContainedObjects_InChilds(CEntity * pObject, vector<CEntity*> & vcArray);

	QuadTree* InsertEntity(CEntity * pObject);
	QuadTree* RenewalEntity(CEntity * pObject, bool bStart = true);
	void DeleteEntity(CEntity * pObject);
	void EraseEntity(CEntity * pObject);

public:	// �浹üũ
	bool SphereCollision(CEntity * pTarget, vector<CEntity*> * pContainedArray);
	bool SphereCollision(CEntity * pTarget, vector<CEntity*> * pContainedArray, bool bIsUp);
//s	bool ReleaseTree();
};

class CCamera;
class CQuadTreeManager
{
public:
#define QAUD_MIN_UNIT 128
	typedef pair<QuadTree*, CEntity*> DynamicInfo;

private:
	CQuadTreeManager();
	~CQuadTreeManager();
	CQuadTreeManager& operator=(const CQuadTreeManager&);

private:
	QuadTree * m_pRootTree;
	
	vector<CEntity*> m_vcContainedArray;
	vector<DynamicInfo> m_vcDynamicArray;

public:
	static CQuadTreeManager& GetInstance();
	void BuildQuadTree(XMFLOAT3 & xmf3Center, UINT uWidth, UINT uLength, QuadTree * pParent);
	void ReleaseQuadTree();

	QuadTree * GetRootTree() { return m_pRootTree; }
	QuadTree* InsertStaticEntity(CEntity* pObject);
	QuadTree* InsertDynamicEntity(CEntity* pObject);

	void DeleteStaticEntity(CEntity* pObject);
	void DeleteDynamicEntity(CEntity* pObject);

public:
	UINT RenewalDynamicObjects();
	void FrustumCullObjects(CCamera * pCamera);

	vector<CEntity*>& CollisionCheckList(CEntity * pObject);
	bool CollisionCheck(CEntity * pObject);
	// �浹 ó�� �� �� ��� �����ֱ� ���� �Լ�
	UINT ContainedErase();

	vector<CEntity*>* GetContainedObjectList(CEntity * pObject);
public:
	void Update(CCamera * pCamera);

	vector<DynamicInfo>::iterator GetDynamicInfo(CEntity * pObject);
};
#define QUADMgr CQuadTreeManager::GetInstance()

class CCollisionMgr
{
	CCollisionMgr();
	~CCollisionMgr();
	CCollisionMgr& operator=(const CCollisionMgr&);

	CQuadTreeManager * m_pQuadMgr;
	BoundingSphere m_bbSphereTarget, m_bbSphereOther;

public:
	static CCollisionMgr& GetInstance();

	CEntity* SphereCollisionObject(CEntity * pTarget, list<CEntity*>& vcObjList);
	bool SphereCollisionOneToMul(CEntity * pTarget, list<CEntity*>& vcObjList);
	bool SphereCollisionOneToMul(CEntity * pTarget, list<CEntity*>& vcObjList, vector<CEntity*>& vcContained);
};
#define COLLISION CCollisionMgr::GetInstance()

#endif