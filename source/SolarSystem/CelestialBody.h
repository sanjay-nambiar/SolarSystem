#pragma once

#include <DirectXMath.h>

namespace Rendering
{
	class CelestialBody : Library::GameComponent
	{
	public:
		CelestialBody();
		virtual ~CelestialBody() = default;

		void SetParams(const std::string& name, const std::string& textureName, float meanDistance, float rotationPeriod, float orbitalPeriod,
			float axialTilt, float diameter, float lit);
		void Adopt(CelestialBody& body);

		std::string Name() const;
		const DirectX::XMFLOAT4& Position() const;
		const DirectX::XMFLOAT4X4& WorldTransform() const;
		const std::string& TextureName() const;
		float Lit() const;
		CelestialBody* Parent() const;

		void Update(const Library::GameTime& gameTime) override;

		static void SetConstantParams(float meanDistance, float rotationPeriod, float orbitalPeriod, float diameter);
	private:
		DirectX::XMFLOAT4X4 mWorldTransform;
		DirectX::XMFLOAT4 mPosition;

		std::vector<CelestialBody*> mChildBodies;
		CelestialBody* mParent;

		std::string mName;
		std::string mTextureName;
		float mMeanDistance;
		float mRotationPeriod;
		float mOrbitalPeriod;
		float mAxialTilt;
		float mDiameter;
		float mLit;
		float mRotationRate;
		float mOrbitalRate;
		float mRotationAngle;
		float mOrbitalAngle;
		DirectX::XMMATRIX mTranslation;

		const std::uint64_t mID;

		// Constants
		static float sMeanDistance;
		static float sRotationPeriod;
		static float sOrbitalPeriod;
		static float sDiameter;
	};
}
