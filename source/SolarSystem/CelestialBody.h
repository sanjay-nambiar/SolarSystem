#pragma once

#include <DirectXMath.h>

namespace Rendering
{
	class CelestialBody : Library::GameComponent
	{
	public:
		CelestialBody();
		virtual ~CelestialBody() = default;

		void SetParams(const std::string& textureName, float meanDistance, float rotationPeriod, float orbitalPeriod,
			float axialTilt, float diameter, float lit);
		void Adopt(CelestialBody& body);

		std::uint64_t ID() const;
		const DirectX::XMFLOAT4& Position() const;
		const DirectX::XMFLOAT4X4& WorldTransform() const;
		const std::string& TextureName() const;
		float Lit() const;

		void Update(const Library::GameTime& gameTime) override;

		static void SetConstantParams(float meanDistance, float rotationPeriod, float orbitalPeriod, float diameter);
	private:
		DirectX::XMFLOAT4X4 mWorldTransform;
		DirectX::XMFLOAT4 mPosition;

		std::unordered_map<std::uint64_t, std::shared_ptr<CelestialBody>> mChildBodies;
		std::shared_ptr<CelestialBody> mParent;

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
