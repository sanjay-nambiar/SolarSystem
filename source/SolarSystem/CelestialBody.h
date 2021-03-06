#pragma once

#include <DirectXMath.h>
#include "Orbit.h"

namespace Rendering
{
	class CelestialBody : Library::DrawableGameComponent
	{
	public:
		CelestialBody(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);
		virtual ~CelestialBody() = default;

		void SetParams(const CelestialBodyData& data);
		void Adopt(CelestialBody& body);

		const CelestialBodyData& Data() const;
		float Radius() const;
		const DirectX::XMFLOAT4& Position() const;
		const DirectX::XMFLOAT4X4& WorldTransform() const;
		const DirectX::XMFLOAT4X4& WorldTranslation() const;
		CelestialBody* Parent() const;
		const std::vector<CelestialBody*>& Children() const;

		void Initialize() override;
		void Update(const Library::GameTime& gameTime) override;
		
		std::shared_ptr<Orbit>& GetOrbit();
		static void SetConstantParams(float meanDistance, float rotationPeriod, float orbitalPeriod, float diameter);
	private:
		void InitializeOrbit();
		void UpdateWorldMatrix(const DirectX::XMFLOAT4X4& transform);
		void UpdateWorldMatrix(const DirectX::XMMATRIX& transform);

		DirectX::XMFLOAT4X4 mWorldTransform;
		DirectX::XMFLOAT4 mPosition;

		std::vector<CelestialBody*> mChildBodies;
		CelestialBody* mParent;
		std::shared_ptr<Orbit> mOrbit;

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

	public:
		static float MeanDistance();
		static float RotationPeriod();
		static float OrbitalPeriod();
		static float Diameter();
	};
}
