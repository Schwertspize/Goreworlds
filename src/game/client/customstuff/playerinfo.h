#ifndef GAME_CLIENT_CUSTOMSTUFF_PLAYERINFO_H
#define GAME_CLIENT_CUSTOMSTUFF_PLAYERINFO_H

#include <base/vmath.h>
#include <base/system.h>

#include <engine/graphics.h>
#include <game/client/gameclient.h>

#define MAX_TEESPLATTER 20


struct CTeeSplatter
{
public:
	float m_Life;
	float m_StartLife;
	float m_Angle;
	int m_Flip;
	int m_Sprite;
	
	
	void Reset()
	{
		m_Life = 0;
		m_StartLife = 240;
	}
	
	void Tick()
	{
		if (m_Life > 0)
			m_Life--;
	}
};


class CPlayerInfo
{
private:
	CTeeSplatter m_aTeeSplatter[MAX_TEESPLATTER];
	
	int m_NextTeeSplatter;
	
	vec2 m_Pos;
	
	// if > 5, !InUse
	int m_UpdateTimer;
	
	int m_aWeaponSprite[NUM_WEAPONS];
	
public:
	CPlayerInfo();
	
	void Reset();
	void Tick();
	
	void Update(vec2 Pos);
	
	vec2 Pos(){ return m_Pos; }
	
	int GetWeaponSprite(int Weapon){ return m_aWeaponSprite[Weapon]; }
	
	
	void SetWeaponSprite(int Weapon, int Sprite)
	{
		m_aWeaponSprite[Weapon] = Sprite;
	}
	
	
	void AddTeeSplatter(float Angle);
	
	void RenderTeeSplatter(class IGraphics *Graphics, class CRenderTools *RenderTools);
	
	bool m_InUse;
};



#endif