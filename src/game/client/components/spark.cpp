#include <base/math.h>
#include <engine/graphics.h>
#include <engine/demo.h>

#include <game/generated/client_data.h>
#include <game/client/render.h>
#include <game/gamecore.h>
#include "spark.h"

CSpark::CSpark()
{
	OnReset();
	m_RenderSpark.m_pParts = this;
}


void CSpark::OnReset()
{
	// reset blood
	for(int i = 0; i < MAX_SPARKS; i++)
	{
		m_aSpark[i].m_PrevPart = i-1;
		m_aSpark[i].m_NextPart = i+1;
	}

	m_aSpark[0].m_PrevPart = 0;
	m_aSpark[MAX_SPARKS-1].m_NextPart = -1;
	m_FirstFree = 0;

	for(int i = 0; i < NUM_GROUPS; i++)
		m_aFirstPart[i] = -1;
}

void CSpark::Add(int Group, CSinglespark *pPart)
{
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		const IDemoPlayer::CInfo *pInfo = DemoPlayer()->BaseInfo();
		if(pInfo->m_Paused)
			return;
	}
	else
	{
		if(m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED)
			return;
	}

	if (m_FirstFree == -1)
		return;

	// remove from the free list
	int Id = m_FirstFree;
	m_FirstFree = m_aSpark[Id].m_NextPart;
	if(m_FirstFree != -1)
		m_aSpark[m_FirstFree].m_PrevPart = -1;

	// copy data
	m_aSpark[Id] = *pPart;

	// insert to the group list
	m_aSpark[Id].m_PrevPart = -1;
	m_aSpark[Id].m_NextPart = m_aFirstPart[Group];
	if(m_aFirstPart[Group] != -1)
		m_aSpark[m_aFirstPart[Group]].m_PrevPart = Id;
	m_aFirstPart[Group] = Id;

	// set some parameters
	m_aSpark[Id].m_Life = 0;
}

void CSpark::Update(float TimePassed)
{
	for(int g = 0; g < NUM_GROUPS; g++)
	{
		int i = m_aFirstPart[g];
		while(i != -1)
		{
			int Next = m_aSpark[i].m_NextPart;

			// move the point
			vec2 Vel = m_aSpark[i].m_Vel*TimePassed;
			
			
			m_aSpark[i].m_Pos += Vel;
			
			m_aSpark[i].m_Vel = Vel* (1.0f/TimePassed);

			m_aSpark[i].m_Life += TimePassed;
			
			
			if (abs(m_aSpark[i].m_Vel.x) + abs(m_aSpark[i].m_Vel.y) > 60.0f)
			{
				if (m_aSpark[i].m_Rotspeed == 0.0f)
					m_aSpark[i].m_Rot = GetAngle(m_aSpark[i].m_Vel);
				else
				{
					m_aSpark[i].m_Rot += TimePassed * m_aSpark[i].m_Rotspeed;
				}
			}
				
			// check blood death
			if(m_aSpark[i].m_Life > m_aSpark[i].m_LifeSpan)
			{
				// remove it from the group list
				if(m_aSpark[i].m_PrevPart != -1)
					m_aSpark[m_aSpark[i].m_PrevPart].m_NextPart = m_aSpark[i].m_NextPart;
				else
					m_aFirstPart[g] = m_aSpark[i].m_NextPart;

				if(m_aSpark[i].m_NextPart != -1)
					m_aSpark[m_aSpark[i].m_NextPart].m_PrevPart = m_aSpark[i].m_PrevPart;

				// insert to the free list
				if(m_FirstFree != -1)
					m_aSpark[m_FirstFree].m_PrevPart = i;
				m_aSpark[i].m_PrevPart = -1;
				m_aSpark[i].m_NextPart = m_FirstFree;
				m_FirstFree = i;
			}

			i = Next;
		}
	}
}

void CSpark::OnRender()
{
	if(Client()->State() < IClient::STATE_ONLINE)
		return;

	static int64 LastTime = 0;
	int64 t = time_get();

	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		const IDemoPlayer::CInfo *pInfo = DemoPlayer()->BaseInfo();
		if(!pInfo->m_Paused)
			Update((float)((t-LastTime)/(double)time_freq())*pInfo->m_Speed);
	}
	else
	{
		if(m_pClient->m_Snap.m_pGameInfoObj && !(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
			Update((float)((t-LastTime)/(double)time_freq()));
	}

	LastTime = t;
}

void CSpark::RenderGroup(int Group)
{
	Graphics()->BlendNormal();
	//gfx_blend_additive();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();

	int i = m_aFirstPart[Group];
	while(i != -1)
	{
		RenderTools()->SelectSprite(m_aSpark[i].m_Spr);
		float a = m_aSpark[i].m_Life / m_aSpark[i].m_LifeSpan;
		vec2 p = m_aSpark[i].m_Pos;
		float Size = m_aSpark[i].m_Size;

		Graphics()->QuadsSetRotation(m_aSpark[i].m_Rot);

		Graphics()->SetColor(
			m_aSpark[i].m_Color.r,
			m_aSpark[i].m_Color.g,
			m_aSpark[i].m_Color.b,
			1.2f-a); // pow(a, 0.75f) *

		IGraphics::CQuadItem QuadItem(p.x, p.y, Size, Size);
		Graphics()->QuadsDraw(&QuadItem, 1);

		i = m_aSpark[i].m_NextPart;
	}
	Graphics()->QuadsEnd();
	Graphics()->BlendNormal();
}
