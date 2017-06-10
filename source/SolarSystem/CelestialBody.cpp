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
		mMeanDistance(0), mRotationPeriod(0), mOrbitalPeriod(0), mAxialTilt(0), mDiameter(0), mLit(0),
		mRotationRate(0), mOrbitalRate(0), mRotationAngle(0), mOrbitalAngle(0),
		mID(reinterpret_cast<std::uint64_t>(this))
	{
	}

	void CelestialBody::SetParams(const std::string& textureName, float meanDistance, float rotationPeriod, float orbitalPeriod,
		float axialTilt, float diameter, float lit)
	{
		mTextureName = textureName;
		mMeanDistance = meanDistance;
		mRotationPeriod = rotationPeriod;
		mOrbitalPeriod = orbitalPeriod;
		mAxialTilt = axialTilt;
		mDiameter = diameter;
		mLit = lit;

		float netRotationPeriod = (sRotationPeriod * mRotationPeriod);
		mRotationRate = (netRotationPeriod == 0) ? 0 : (XM_2PI / netRotationPeriod);
		mRotationAngle = 0.0f;

		float netOrbitalPeriod = (sOrbitalPeriod * mOrbitalPeriod);
		mOrbitalRate = (netOrbitalPeriod == 0) ? 0 : (XM_2PI / netOrbitalPeriod);
		mOrbitalAngle = 0.0f;

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

	float CelestialBody::Lit() const
	{
		return mLit;
	}

	void CelestialBody::Update(const GameTime& gameTime)
	{
		const float elapsedSeconds = gameTime.ElapsedGameTimeSeconds().count();
		mRotationAngle += elapsedSeconds * mRotationRate;
		if (mRotationAngle >= XM_2PI || mRotationAngle <= -XM_2PI)
		{
			mRotationAngle = 0.0f;
		}

		mOrbitalAngle += elapsedSeconds * mOrbitalRate;
		if (mOrbitalAngle >= XM_2PI || mOrbitalAngle <= -XM_2PI)
		{
			mOrbitalAngle = 0.0f;
		}

		float diameter = sDiameter * mDiameter;

		XMMATRIX transform = XMMatrixScaling(diameter, diameter, diameter);
		transform = XMMatrixMultiply(transform, XMMatrixRotationY(mRotationAngle));
		transform = XMMatrixMultiply(transform, XMMatrixRotationZ(XMConvertToRadians(mAxialTilt)));
		transform = XMMatrixMultiply(transform, mTranslation);
		transform = XMMatrixMultiply(transform, XMMatrixRotationY(mOrbitalAngle));
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
