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
		mWorldTransform(MatrixHelper::Identity), mParent(nullptr),
		mRotationRate(0), mOrbitalRate(0), mRotationAngle(0), mOrbitalAngle(0),
		mID(reinterpret_cast<std::uint64_t>(this))
	{
	}

	void CelestialBody::SetParams(const CelestialBodyData& data)
	{
		mData = data;
		float netRotationPeriod = (sRotationPeriod * mData.mRotationPeriod);
		mRotationRate = (netRotationPeriod == 0) ? 0 : (XM_2PI / netRotationPeriod);
		mRotationAngle = 0.0f;

		float netOrbitalPeriod = (sOrbitalPeriod * mData.mOrbitalPeriod);
		mOrbitalRate = (netOrbitalPeriod == 0) ? 0 : (XM_2PI / netOrbitalPeriod);
		mOrbitalAngle = 0.0f;
		XMStoreFloat4x4(&mTranslation, XMMatrixTranslation(sMeanDistance * mData.mMeanDistance, 0, 0));
		Initialize();
	}

	void CelestialBody::Adopt(CelestialBody& body)
	{
		mChildBodies.push_back(&body);
		body.mParent = this;

		// calculate distance from the outer edge of the parent instead of origin
		float childDistanceFromOrigin = (sMeanDistance * body.mData.mMeanDistance) + (mData.mDiameter / 2) * 20;
		XMStoreFloat4x4(&body.mTranslation, XMMatrixTranslation(childDistanceFromOrigin, 0, 0));
		body.Initialize();
	}

	const CelestialBodyData& CelestialBody::Data() const
	{
		return mData;
	}

	const XMFLOAT4& CelestialBody::Position() const
	{
		return mPosition;
	}

	const DirectX::XMFLOAT4X4& CelestialBody::WorldTransform() const
	{
		return mWorldTransform;
	}

	const DirectX::XMFLOAT4X4& CelestialBody::WorldTranslation() const
	{
		return mTranslation;
	}

	CelestialBody* CelestialBody::Parent() const
	{
		return mParent;
	}

	void CelestialBody::Initialize()
	{
		GameComponent::Initialize();

		float diameter = sDiameter * mData.mDiameter;
		XMMATRIX transform = XMMatrixScaling(diameter, diameter, diameter);
		transform = XMMatrixMultiply(transform, XMMatrixRotationY(mRotationAngle));
		transform = XMMatrixMultiply(transform, XMMatrixRotationZ(XMConvertToRadians(mData.mAxialTilt)));
		transform = XMMatrixMultiply(transform, XMLoadFloat4x4(&mTranslation));
		XMStoreFloat4x4(&mWorldTransform, transform);
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

		float diameter = sDiameter * mData.mDiameter;
		XMMATRIX transform = XMMatrixScaling(diameter, diameter, diameter);
		transform = XMMatrixMultiply(transform, XMMatrixRotationY(mRotationAngle));
		transform = XMMatrixMultiply(transform, XMMatrixRotationZ(XMConvertToRadians(mData.mAxialTilt)));
		transform = XMMatrixMultiply(transform, XMLoadFloat4x4(&mTranslation));
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
