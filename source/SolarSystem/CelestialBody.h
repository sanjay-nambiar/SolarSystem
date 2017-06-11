#pragma once

#include <DirectXMath.h>

namespace Rendering
{
	class CelestialBody : Library::GameComponent
	{
	public:
		CelestialBody();
		virtual ~CelestialBody() = default;

		void SetParams(const CelestialBodyData& data);
		void Adopt(CelestialBody& body);

		const CelestialBodyData& Data() const;
		const DirectX::XMFLOAT4& Position() const;
		const DirectX::XMFLOAT4X4& WorldTransform() const;
		CelestialBody* Parent() const;

		void Initialize() override;
		void Update(const Library::GameTime& gameTime) override;

		static void SetConstantParams(float meanDistance, float rotationPeriod, float orbitalPeriod, float diameter);
	private:
		DirectX::XMFLOAT4X4 mWorldTransform;
		DirectX::XMFLOAT4 mPosition;

		std::vector<CelestialBody*> mChildBodies;
		CelestialBody* mParent;

		CelestialBodyData mData;
		float mRotationRate;
		float mOrbitalRate;
		float mRotationAngle;
		float mOrbitalAngle;
		DirectX::XMFLOAT4X4 mTranslation;

		const std::uint64_t mID;

		// Constants
		static float sMeanDistance;
		static float sRotationPeriod;
		static float sOrbitalPeriod;
		static float sDiameter;
	};
}
