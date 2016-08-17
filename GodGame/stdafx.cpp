// stdafx.cpp : ǥ�� ���� ���ϸ� ��� �ִ� �ҽ� �����Դϴ�.
// GodGame.pch�� �̸� �����ϵ� ����� �˴ϴ�.
// stdafx.obj���� �̸� �����ϵ� ���� ������ ���Ե˴ϴ�.

#include "stdafx.h"

// TODO: �ʿ��� �߰� �����
// �� ������ �ƴ� STDAFX.H���� �����մϴ�.

ostream& operator<<(ostream& os, POINT & pt)
{
	os << pt.x << ", " << pt.y;
	return os;
}

ostream& operator<<(ostream& os, RECT & rect)
{
	os << rect.left << ", " << rect.bottom << " // " << rect.right << ", " << rect.top;
	return os;
}

ostream& operator<<(ostream& os, LPRECT & rect)
{
	os << rect->left << ", " << rect->bottom << " // " << rect->right << rect->top;
	return os;
}


ostream& operator<<(ostream& os, XMFLOAT2 & xmf2)
{
	os << xmf2.x << ", " << xmf2.y;
	return os;
}

ostream& operator<<(ostream& os, XMFLOAT3 & xmf3)
{
	os << xmf3.x << ", " << xmf3.y << ", " << xmf3.z;
	return os;
}

ostream & operator<<(ostream & os, XMFLOAT4 & xmf4)
{
	os << xmf4.x << ", " << xmf4.y << ", " << xmf4.z <<", " << xmf4.w;
	return os;
}

ostream & operator<<(ostream & os, XMFLOAT4X4 & mtx)
{
	os << mtx._11 << ", " << mtx._12 << ", " << mtx._13 << ", " << mtx._14 << endl;
	os << mtx._21 << ", " << mtx._22 << ", " << mtx._23 << ", " << mtx._24 << endl;
	os << mtx._31 << ", " << mtx._32 << ", " << mtx._33 << ", " << mtx._34 << endl;
	os << mtx._41 << ", " << mtx._42 << ", " << mtx._43 << ", " << mtx._44;
	return os;
}
