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
			float axialTilt, float diameter, float albedo);
		void SetParent(const std::shared_ptr<CelestialBody>& parent);

		const DirectX::XMFLOAT4X4& WorldTransform() const;
		const std::string& TextureName() const;

		void Update(const Library::GameTime& gameTime) override;

		static void SetConstantParams(float meanDistance, float rotationPeriod, float orbitalPeriod, float diameter);
	private:
		DirectX::XMFLOAT4X4 mWorldTransform;
		std::string mTextureName;
		float mMeanDistance;
		float mRotationPeriod;
		float mOrbitalPeriod;
		float mAxialTilt;
		float mDiameter;
		float mAlbedo;
		std::shared_ptr<CelestialBody> mParent;

		// Constants
		static float sMeanDistance;
		static float sRotationPeriod;
		static float sOrbitalPeriod;
		static float sDiameter;
	};
}
