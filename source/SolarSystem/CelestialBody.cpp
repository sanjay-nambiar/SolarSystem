#include "pch.h"
#include "CelestialBody.h"

using namespace std;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	float CelestialBody::sMeanDistance;
	float CelestialBody::sRotationPeriod;
	float CelestialBody::sOrbitalPeriod;
	float CelestialBody::sDiameter;

	CelestialBody::CelestialBody() :
		mWorldTransform(MatrixHelper::Identity),
		mMeanDistance(0), mRotationPeriod(0), mOrbitalPeriod(0), mAxialTilt(0), mDiameter(0), mAlbedo(0)
	{
	}

	void CelestialBody::SetParams(const std::string& textureName, float meanDistance, float rotationPeriod, float orbitalPeriod,
		float axialTilt, float diameter, float albedo)
	{
		mTextureName = textureName;
		mMeanDistance = meanDistance;
		mRotationPeriod = rotationPeriod;
		mOrbitalPeriod = orbitalPeriod;
		mAxialTilt = axialTilt;
		mDiameter = diameter;
		mAlbedo = albedo;
	}

	void CelestialBody::SetParent(const std::shared_ptr<CelestialBody>& parent)
	{
		mParent = parent;
	}

	const DirectX::XMFLOAT4X4& CelestialBody::WorldTransform() const
	{
		return mWorldTransform;
	}

	const std::string& CelestialBody::TextureName() const
	{
		return mTextureName;
	}

	void CelestialBody::Update(const GameTime& gameTime)
	{
		static const float rotationRate = XM_2PI / (sRotationPeriod * mRotationPeriod);
		static const float orbitalRate = XM_2PI / (sOrbitalPeriod * mOrbitalPeriod);
		static const auto translation = XMMatrixTranslation(sMeanDistance * mMeanDistance, 0, 0);

		static float rotationAngle = 0.0f;
		rotationAngle += gameTime.ElapsedGameTimeSeconds().count() * rotationRate;
		if (rotationAngle >= XM_2PI || rotationAngle <= -XM_2PI)
		{
			rotationAngle = 0.0f;
		}

		static float orbitalAngle = 0.0f;
		orbitalAngle += gameTime.ElapsedGameTimeSeconds().count() * orbitalRate;
		if (orbitalAngle >= XM_2PI || orbitalAngle <= -XM_2PI)
		{
			orbitalAngle = 0.0f;
		}

		static float diameter = sDiameter * mDiameter;

		XMMATRIX transform = XMMatrixScaling(diameter, diameter, diameter);
		transform = XMMatrixMultiply(transform, XMMatrixRotationY(rotationAngle));
		transform = XMMatrixMultiply(transform, XMMatrixRotationZ(XMConvertToRadians(mAxialTilt)));
		transform = XMMatrixMultiply(transform, translation);
		transform = XMMatrixRotationZ(orbitalAngle);
		XMLoadFloat4x4(&mWorldTransform);
	}

	void CelestialBody::SetConstantParams(float meanDistance, float rotationPeriod, float orbitalPeriod, float diameter)
	{
		sMeanDistance = meanDistance;
		sRotationPeriod = rotationPeriod;
		sOrbitalPeriod = orbitalPeriod;
		sDiameter = diameter;
	}
}
