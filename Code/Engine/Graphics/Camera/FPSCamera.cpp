//#include "__PCH_Graphics.h"
#include "FPSCamera.h"
#include "Assert.h"

CFPSCamera::CFPSCamera(float zn, float zf, float fov, float aspect, CObject3D* object3D)
: CCamera( zn, zf, fov, aspect, object3D, TC_FPS)
{}

CFPSCamera::CFPSCamera()
: CCamera()
{}



Vect3f CFPSCamera::GetDirection () const
{
	assert(m_pObject3D);

	return (GetLookAt() - GetEye());
}

Vect3f CFPSCamera::GetLookAt () const
{
	assert(m_pObject3D);

	float yaw		= m_pObject3D->GetYaw();
	float pitch	= m_pObject3D->GetPitch();
	Vect3f pos	= m_pObject3D->GetPosition();

	//Pasamos de coordenadas esfericas a coordenadas cartesianas
	Vect3f vEyePt((float)(cos(yaw) * cos(pitch)),
		(float)(sin(pitch)),
		(float)(sin(yaw) * cos(pitch) ));

	return (pos + vEyePt); 
}

Vect3f CFPSCamera::GetEye () const
{
	assert(m_pObject3D);

	return m_pObject3D->GetPosition();


}

Vect3f CFPSCamera::GetVecUp () const
{
	assert(m_pObject3D);

	float yaw		= m_pObject3D->GetYaw();
	float pitch	= m_pObject3D->GetPitch();

	Vect3f vUpVec((float)(-cos(yaw) * sin(pitch)),
		(float)cos(pitch),
		(float)(-sin(yaw) * sin(pitch) ));

	return vUpVec;
}