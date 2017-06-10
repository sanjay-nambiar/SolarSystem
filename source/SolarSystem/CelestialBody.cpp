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
		mMeanDistance(0), mRotationPeriod(0), mOrbitalPeriod(0), mAxialTilt(0), mDiameter(0), mAlbedo(0),
		mRotationRate(0), mOrbitalRate(0),
		mID(reinterpret_cast<std::uint64_t>(this))
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

		mRotationRate = XM_2PI / (sRotationPeriod * mRotationPeriod);
		mOrbitalRate = XM_2PI / (sOrbitalPeriod * mOrbitalPeriod);
		mTranslation = DirectX::XMMatrixTranslation(sMeanDistance * mMeanDistance, 0, 0);

		GameTime gameTime;
		Update(gameTime);
	}

	void CelestialBody::Adopt(CelestialBody& body)
	{
		if (mChildBodies.find(body.ID()) == mChildBodies.end())
		{
			mChildBodies.insert({body.ID(), std::shared_ptr<CelestialBody>(&body)});
			body.mParent = std::shared_ptr<CelestialBody>(this);
		}
	}

	std::uint64_t CelestialBody::ID() const
	{
		return mID;
	}

	const DirectX::XMFLOAT4& CelestialBody::Position() const
	{
		return mPosition;
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
		const float elapsedSeconds = gameTime.ElapsedGameTimeSeconds().count();
		static float rotationAngle = 0.0f;
		rotationAngle += elapsedSeconds * mRotationRate;
		if (rotationAngle >= XM_2PI || rotationAngle <= -XM_2PI)
		{
			rotationAngle = 0.0f;
		}

		static float orbitalAngle = 0.0f;
		orbitalAngle += elapsedSeconds * mOrbitalRate;
		if (orbitalAngle >= XM_2PI || orbitalAngle <= -XM_2PI)
		{
			orbitalAngle = 0.0f;
		}

		static float diameter = sDiameter * mDiameter;

		XMMATRIX transform = XMMatrixScaling(diameter, diameter, diameter);
		transform = XMMatrixMultiply(transform, XMMatrixRotationY(rotationAngle));
		transform = XMMatrixMultiply(transform, XMMatrixRotationZ(XMConvertToRadians(mAxialTilt)));
		transform = XMMatrixMultiply(transform, mTranslation);
		transform = XMMatrixMultiply(transform, XMMatrixRotationY(orbitalAngle));
		XMStoreFloat4x4(&mWorldTransform, transform);
	}

	void CelestialBody::SetConstantParams(float meanDistance, float rotationPeriod, float orbitalPeriod, float diameter)
	{
		sMeanDistance = meanDistance;
		sRotationPeriod = rotationPeriod;
		sOrbitalPeriod = orbitalPeriod;
		sDiameter = diameter;
	}
}
