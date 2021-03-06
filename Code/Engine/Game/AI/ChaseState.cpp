// Estado de persecución del monstruo

#include "ChaseState.h"
#include "State.h"
#include "StateMachine.h"
#include "Character.h"
#include "Node.h"
#include "NodeManager.h"
#include "PhysicsManager.h"
#include "Core.h"
#include "Renderer\RenderableObjectsLayersManager.h"
#include "Characters\Monster.h"
#include "Base.h"

CChaseState::~CChaseState()
{
	/*for(unsigned int i=0; i<m_Path.size(); i++)
	{
		CHECKED_DELETE(m_Path[i]);
	}
	m_Path.clear();*/
}

void CChaseState::Update(float ElapsedTime)
{
	const float rotationSpeed = mathUtils::Deg2Rad( 90.0f );

	//m_Time = m_Time + ElapsedTime;
	float l_sqdistance = m_Owner->GetPosition().SqDistance(m_Player->GetPosition());
	if (l_sqdistance < 1.5f)
	{
		m_StateMachine->ChangeState("ATTACK");
		return;
	}
	Vect3f l_ras = m_Owner->GetPosition();
	/*l_ras.y = 0.0f;
	m_Owner->SetPosition(l_ras);*/
	
	if(isSeen() || isHeared())
	{
		m_Owner->ChangeCharacterAnimation(WALK_ANIM, 0.3f);
		if(isReachable())
		{
			Init();
			if(l_sqdistance<4.0f)
			{
				m_StateMachine->ChangeState("ATTACK");
				return;
			}
			if(isFaced())
			{
				Vect3f l_dir = (m_Player->GetPosition() - m_Owner->GetPosition()).Normalize();
				m_Owner->SetYaw(atan2(-l_dir.x, l_dir.z));
				//m_Owner->SetPosition(m_Owner->GetPosition() + m_Owner->GetFront( ) * ElapsedTime *1.30f);
				m_Owner->SetPosition(m_Owner->GetFront()*m_WalkSpeed, ElapsedTime);
				return;
			}
			else
			{
				if( isAtLeft())
				{
					m_Owner->SetYaw(m_Owner->GetYaw() + rotationSpeed*ElapsedTime);
				} 
				else{ 				
					m_Owner->SetYaw(m_Owner->GetYaw() - rotationSpeed*ElapsedTime);
				}
				return;
			}
		}
		else{
			if(!m_LPos)
			{
				m_LastPos = m_Player->GetPosition();
				m_LPos =true;
			}
			if(!m_Lost)
			{
				if(isFaced(m_LastPos))
				{
					m_Time = m_Time + ElapsedTime;
					if(m_Time < 2.0f)
					{
						Vect3f l_dir = (m_LastPos - m_Owner->GetPosition()).Normalize();
						m_Owner->SetYaw(atan2(-l_dir.x, l_dir.z));
						//m_Owner->SetPosition(m_Owner->GetPosition() + m_Owner->GetFront( ) * ElapsedTime *1.30f);
						m_Owner->SetPosition(m_Owner->GetFront()*m_WalkSpeed, ElapsedTime);
						return;
					}
					else{
						m_Time=0;
						if(CORE->GetNodeManager()->GetNumNodes() == 0)
						{
							m_Owner->ChangeCharacterAnimation(WALK_ANIM, 0.3f);
							m_StateMachine->ChangeState("WANDER");
							return;
						}
						CNode * l_start = CORE->GetNodeManager()->GetNode(CORE->GetNodeManager()->NearestNode(m_Owner->GetPosition()));
						CNode * l_end = CORE->GetNodeManager()->GetNode(CORE->GetNodeManager()->NearestNode(m_Player->GetPosition()));
						m_Path = CORE->GetNodeManager()->AStarAlgorithm(l_start, l_end);

						if(m_Path.empty())
						{
							m_Owner->ChangeCharacterAnimation(WALK_ANIM, 0.3f);
							m_StateMachine->ChangeState("WANDER");
							return;
						}

						if(isBetterNext())
						{
							std::vector<CNode*>::iterator it = m_Path.begin();
							m_Path.erase(it);
						}
						m_Lost = true;
					}
				}
				else
				{
					if( isAtLeft(m_LastPos))
					{
						m_Owner->SetYaw(m_Owner->GetYaw() + rotationSpeed*ElapsedTime);
					} 
					else{ 				
						m_Owner->SetYaw(m_Owner->GetYaw() - rotationSpeed*ElapsedTime);
					}
					return;
				}
			}
		}
	}
	else{
		if(m_Path.empty() && !m_Lost)
		{
			m_Time=0;
			if(CORE->GetNodeManager()->GetNumNodes() == 0)
			{
				m_Owner->ChangeCharacterAnimation(WALK_ANIM, 0.3f);
				m_StateMachine->ChangeState("WANDER");
				return;
			}
			CNode * l_start = CORE->GetNodeManager()->GetNode(CORE->GetNodeManager()->NearestNode(m_Owner->GetPosition()));
			CNode * l_end = CORE->GetNodeManager()->GetNode(CORE->GetNodeManager()->NearestNode(m_Player->GetPosition()));
			m_Path = CORE->GetNodeManager()->AStarAlgorithm(l_start, l_end);
			if(m_Path.empty())
			{
				m_Owner->ChangeCharacterAnimation(WALK_ANIM, 0.3f);
				m_StateMachine->ChangeState("WANDER");
				return;
			}

			if(isBetterNext())
			{
				std::vector<CNode*>::iterator it = m_Path.begin();
				m_Path.erase(it);
			}
			m_Lost = true;
		}
	}
	
	if(isFaced(m_Path[0]->GetPos()))
	{
		Vect3f l_dir = (m_Path[0]->GetPos() - m_Owner->GetPosition()).Normalize();
		m_Owner->SetYaw(atan2(-l_dir.x, l_dir.z));
		m_Owner->SetPosition(m_Owner->GetPosition() + m_Owner->GetFront( ) * ElapsedTime*1.30f);
		Vect3f l_new = m_Owner->GetPosition() +m_Owner->GetFront() * ElapsedTime * m_WalkSpeed;
		Vect3f l_post = (m_Path[0]->GetPos() - l_new).Normalize();

		float l_dist = l_new.SqDistance(m_Path[0]->GetPos()); 
		//if((l_dir * l_post) > 0.0f)
		if(l_dist > 0.01f)
		{
			m_Owner->SetPosition(m_Owner->GetFront()*m_WalkSpeed, ElapsedTime);
		}
		else{
			//m_Owner->SetPosition(m_Path[0]->GetPos());
			l_dir = m_Path[0]->GetPos() - m_Owner->GetPosition();
			l_dir.y=0.0f;
			m_Owner->SetPosition(l_dir, ElapsedTime);
			std::vector<CNode*>::iterator it = m_Path.begin();
			m_Path.erase(it);
			if(m_Path.size()==0)
			{
				m_Owner->ChangeCharacterAnimation(WALK_ANIM, 0.3f);
				m_StateMachine->ChangeState("WANDER");
				return;
			}
		}
	}
	else{
		m_fLostTime += ElapsedTime;

		m_Owner->SetPosition(m_Owner->GetFront()*m_WalkSpeed, ElapsedTime);
		if (m_fLostTime <= 1.5f)
		{
			if(isAtLeft(m_Path[0]->GetPos()))
			{
				m_Owner->SetYaw(m_Owner->GetYaw() + rotationSpeed*ElapsedTime);
			} 
			else{ 				
				m_Owner->SetYaw(m_Owner->GetYaw() - rotationSpeed*ElapsedTime);
			}
		}
		else
		{
			m_fLostTime = 0.0f;
			m_StateMachine->ChangeState("WANDER"); 
		}
		return;
	}

}

void CChaseState::Create()
{
	m_Time = 0.0f;
	m_Name = "CHASE";
	m_Path.clear();
	m_Lost = false;
	m_LPos = false;
}

void CChaseState::Init()
{
	m_Lost = false;
	m_LPos = false;
	m_Time = 0.0f;
	m_Path.clear();
}

void CChaseState::Reset()
{
	m_Lost = false;
	m_Name = "CHASE";
	m_LPos = false;
	m_Time = 0.0f;
	m_Path.clear();
}

bool CChaseState::isFaced()
{
	Vect3f l_aux = (m_Player->GetPosition() - m_Owner->GetPosition()).Normalize();
	
	float l_vSuma = m_Owner->GetFront() * l_aux;
	if(l_vSuma < 0.95f)
		return false;

	return true;
}

bool CChaseState::isFaced(Vect3f pos)
{
	Vect3f l_pos = m_Owner->GetPosition();
	Vect3f l_aux = (pos - m_Owner->GetPosition()).Normalize();
	
	float l_vSuma = m_Owner->GetFront() * l_aux;
	if(l_vSuma < 0.95f)
		return false;

	return true;
}

bool CChaseState::isHeared()
{
	Vect3f l_aux = (m_Player->GetPosition() - m_Owner->GetPosition());		
	if(l_aux.Length() > m_HearDistance)
		return false;
	return true;
}

bool CChaseState::isSeen()
{
	Vect3f l_aux = (m_Player->GetPosition() - m_Owner->GetPosition());		
	if(l_aux.Length() > m_VisionDistance)
		return false;
	l_aux.Normalize();
	float l_vSuma = m_Owner->GetFront() * l_aux;
	if(l_vSuma < degToDot(m_VisionAngle/2.0f))
		return false;
	return true;
}

bool CChaseState::isReachable()
{
	Vect3f l_up(0.0f, 1.0f, 0.0f);
	Vect3f l_right = l_up ^ m_Owner->GetFront();
	
	CPhysicUserData * l_user;	
	CPhysicUserData * l_pla = m_Player->GetPhysicUserData();
	SCollisionInfo l_info;
	Vect3f l_fron = m_Owner->GetFront();
	Vect3f l_play = m_Player->GetPosition();
	Vect3f l_dir = (m_Player->GetPosition() - m_Owner->GetPosition()).Normalize();
	Vect3f l_pos = m_Owner->GetPosition();

	l_user = CORE->GetPhysicsManager()->RaycastClosestActor(l_pos + l_up * 1.05f +l_dir*0.2f + l_right * 0.1f, l_dir , 0x1, l_info);
	if(l_user == m_Player->GetPhysicUserData())
	{
		return true;
	}
	l_user = CORE->GetPhysicsManager()->RaycastClosestActor(l_pos + l_up * 0.95f +l_dir*0.2f - l_right * 0.1f, l_dir, 0x1, l_info);
	if(l_user == m_Player->GetPhysicUserData())
	{
		return true;
	}
	l_user = CORE->GetPhysicsManager()->RaycastClosestActor(l_pos + l_up * 1.05f +l_dir*0.2f - l_right * 0.1f, l_dir, 0x1, l_info);
	if(l_user == m_Player->GetPhysicUserData())
	{
		return true;
	}
	l_user = CORE->GetPhysicsManager()->RaycastClosestActor(l_pos + l_up * 0.95f +l_dir*0.2f + l_right * 0.1f, l_dir, 0x1, l_info);
	if(l_user == m_Player->GetPhysicUserData())
	{
		return true;
	}

	return false;
}

bool CChaseState::isAtLeft()
{
	Vect3f l_up(0.0f, 1.0f, 0.0f);
	Vect3f l_right = l_up ^ m_Owner->GetFront();
	l_right.Normalize();
	float l_d = -(l_right * m_Owner->GetPosition());

	float v( (l_right * m_Player->GetPosition()) + l_d );

	return v <= 0.0f;
}

bool CChaseState::isAtLeft(Vect3f pos)
{
	Vect3f l_up(0.0f, 1.0f, 0.0f);
	Vect3f l_right = l_up ^ m_Owner->GetFront();
	l_right.Normalize();
	float l_d = -(l_right * m_Owner->GetPosition());

	float v( (l_right * pos) + l_d );

	return v <= 0.0f;
}

bool CChaseState::isBetterNext()
{
	if(m_Path.size() < 2)
		return false;
	Vect3f l_front = m_Owner->GetFront();
	float l_d = -(l_front * m_Owner->GetPosition());

	float v = (l_front * m_Path[0]->GetPos()) + l_d;
	if(v>0.0f)
		return false;
	v = (l_front * m_Path[1]->GetPos()) + l_d;
	if(v<0.0f)
		return false;
	return true;
}