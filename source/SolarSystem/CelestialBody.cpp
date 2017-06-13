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

	CelestialBody::CelestialBody(Library::Game& game, const std::shared_ptr<Library::Camera>& camera) :
		DrawableGameComponent(game, camera),
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
		body.InitializeOrbit();
	}

	const CelestialBodyData& CelestialBody::Data() const
	{
		return mData;
	}

	float CelestialBody::Radius() const
	{
		XMFLOAT3 translation(mTranslation._41, mTranslation._42, mTranslation._43);
		return XMVectorGetByIndex(XMVector3Length(XMLoadFloat3(&translation)), 1);
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

	const std::vector<CelestialBody*>& CelestialBody::Children() const
	{
		return mChildBodies;
	}

	void CelestialBody::Initialize()
	{
		GameComponent::Initialize();

		float diameter = sDiameter * mData.mDiameter;
		XMMATRIX transform = XMMatrixScaling(diameter, diameter, diameter);
		transform = XMMatrixMultiply(transform, XMMatrixRotationY(mRotationAngle));
		transform = XMMatrixMultiply(transform, XMMatrixRotationZ(XMConvertToRadians(mData.mAxialTilt)));
		transform = XMMatrixMultiply(transform, XMLoadFloat4x4(&mTranslation));
		if (mParent != nullptr)
		{
			XMFLOAT4 origin(0.0f, 0.0f, 0.0f, 1.0f);
			XMVECTOR position = XMLoadFloat4(&origin);
			XMVECTOR transformed = XMVector4Transform(position, XMLoadFloat4x4(&mParent->WorldTransform()));
			transform = XMMatrixMultiply(transform, XMMatrixTranslationFromVector(transformed));
		}
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
		if (mParent != nullptr)
		{
			XMFLOAT4 origin(0.0f, 0.0f, 0.0f, 1.0f);
			XMVECTOR position = XMLoadFloat4(&origin);
			XMVECTOR transformed = XMVector4Transform(position, XMLoadFloat4x4(&mParent->WorldTransform()));
			transform = XMMatrixMultiply(transform, XMMatrixTranslationFromVector(transformed));
		}
		XMStoreFloat4x4(&mWorldTransform, transform);
		if (mOrbit)
		{
			mOrbit->Update(gameTime);
		}
	}

	std::shared_ptr<Orbit>& CelestialBody::GetOrbit()
	{
		return mOrbit;
	}

	void CelestialBody::SetConstantParams(float meanDistance, float rotationPeriod, float orbitalPeriod, float diameter)
	{
		sMeanDistance = meanDistance;
		sRotationPeriod = rotationPeriod;
		sOrbitalPeriod = orbitalPeriod;
		sDiameter = diameter;
	}

	void CelestialBody::InitializeOrbit()
	{
		mOrbit = std::make_shared<Orbit>(*mGame, mCamera, *this);
		mOrbit->Initialize();
		XMFLOAT3 translation(mTranslation._41, mTranslation._42, mTranslation._43);
		float radius = XMVectorGetByIndex(XMVector3Length(XMLoadFloat3(&translation)), 0);
		mOrbit->SetParams(radius, 10, XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f));
	}
}
