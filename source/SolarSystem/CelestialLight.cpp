#include "pch.h"

using namespace DirectX;
using namespace Library;

namespace Rendering
{
	RTTI_DEFINITIONS(CelestialLight)

		const float CelestialLight::DefaultIntensity = 100000.0f;

	CelestialLight::CelestialLight(Game& game) :
		CelestialLight(game, Vector3Helper::Zero, DefaultIntensity)
	{}

	CelestialLight::CelestialLight(Game& game, const DirectX::XMFLOAT3& position, float intensity) :
		Light(game), mPosition(position), mIntensity(intensity)
	{}

	XMFLOAT3& CelestialLight::Position()
	{
		return mPosition;
	}

	XMVECTOR CelestialLight::PositionVector() const
	{
		return XMLoadFloat3(&mPosition);
	}

	float CelestialLight::Intensity() const
	{
		return mIntensity;
	}

	void CelestialLight::SetPosition(float x, float y, float z)
	{
		XMVECTOR position = XMVectorSet(x, y, z, 1.0f);
		SetPosition(position);
	}

	void CelestialLight::SetPosition(FXMVECTOR position)
	{
		XMStoreFloat3(&mPosition, position);
	}

	void CelestialLight::SetPosition(const XMFLOAT3& position)
	{
		mPosition = position;
	}

	void CelestialLight::SetIntensity(float value)
	{
		mIntensity = value;
	}
}
