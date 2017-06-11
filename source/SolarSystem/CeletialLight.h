#pragma once

#include "Light.h"

namespace Rendering
{
	class CelestialLight : public Library::Light
	{
		RTTI_DECLARATIONS(CelestialLight, Light)

	public:
		CelestialLight(Library::Game& game);
		CelestialLight(Library::Game& game, const DirectX::XMFLOAT3& position, float radius);
		CelestialLight(const CelestialLight&) = default;
		CelestialLight& operator=(const CelestialLight&) = default;
		CelestialLight& operator=(CelestialLight&&) = default;
		CelestialLight(CelestialLight&&) = default;
		virtual ~CelestialLight() = default;

		DirectX::XMFLOAT3& Position();
		DirectX::XMVECTOR PositionVector() const;
		FLOAT Intensity() const;

		virtual void SetPosition(float x, float y, float z);
		virtual void SetPosition(DirectX::FXMVECTOR position);
		virtual void SetPosition(const DirectX::XMFLOAT3& position);
		virtual void SetIntensity(float value);

		static const float DefaultIntensity;

	protected:
		DirectX::XMFLOAT3 mPosition;
		float mIntensity;
	};
}
