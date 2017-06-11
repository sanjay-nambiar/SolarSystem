#include "pch.h"

using namespace std;
using namespace DirectX;
using namespace Library;

namespace Rendering
{
	const XMVECTORF32 RenderingGame::BackgroundColor = Colors::Black;
	const XMFLOAT3 RenderingGame::CameraStart = XMFLOAT3(0.0f, 2.5f, 1000.0f);
	const float RenderingGame::MinCameraMovementRate = 10;
	const float RenderingGame::MaxCameraMovementRate = 1000;
	const float RenderingGame::MovementRateDelta = 10;

	RenderingGame::RenderingGame(std::function<void*()> getWindowCallback, std::function<void(SIZE&)> getRenderTargetSizeCallback) :
		Game(getWindowCallback, getRenderTargetSizeCallback, false), mRenderStateHelper(*this)
	{
	}

	void RenderingGame::Initialize()
	{
		RasterizerStates::Initialize(mDirect3DDevice.Get());
		SamplerStates::Initialize(mDirect3DDevice.Get());

		mKeyboard = make_shared<KeyboardComponent>(*this);
		mComponents.push_back(mKeyboard);
		mServices.AddService(KeyboardComponent::TypeIdClass(), mKeyboard.get());

		mMouse = make_shared<MouseComponent>(*this);
		mComponents.push_back(mMouse);
		mServices.AddService(MouseComponent::TypeIdClass(), mMouse.get());

		mGamePad = make_shared<GamePadComponent>(*this);
		mComponents.push_back(mGamePad);
		mServices.AddService(GamePadComponent::TypeIdClass(), mGamePad.get());

		mCamera = make_shared<FirstPersonCamera>(*this);
		mComponents.push_back(mCamera);
		mServices.AddService(Camera::TypeIdClass(), mCamera.get());

		mSolarSystemDemo = make_shared<SolarSystemDemo>(*this, mCamera);
		mComponents.push_back(mSolarSystemDemo);

		Game::Initialize();

		mFpsComponent = make_shared<FpsComponent>(*this);
		mFpsComponent->Initialize();

		mCamera->SetPosition(CameraStart);
	}

	void RenderingGame::Update(const GameTime &gameTime)
	{
		mFpsComponent->Update(gameTime);

		if (mKeyboard->WasKeyPressedThisFrame(Keys::Escape) || mGamePad->WasButtonPressedThisFrame(GamePadButtons::Back))
		{
			Exit();
		}

		if (mKeyboard->WasKeyDown(Keys::OemPlus))
		{
			if (mCamera->MovementRate() < MaxCameraMovementRate)
			{
				mCamera->MovementRate() += MovementRateDelta;
			}
		}
		if (mKeyboard->WasKeyDown(Keys::OemMinus))
		{
			if (mCamera->MovementRate() > MinCameraMovementRate)
			{
				mCamera->MovementRate() -= MovementRateDelta;
			}
		}

		Game::Update(gameTime);
	}

	void RenderingGame::Draw(const GameTime &gameTime)
	{
		mDirect3DDeviceContext->ClearRenderTargetView(mRenderTargetView.Get(), reinterpret_cast<const float*>(&BackgroundColor));
		mDirect3DDeviceContext->ClearDepthStencilView(mDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		Game::Draw(gameTime);

		mRenderStateHelper.SaveAll();
		mFpsComponent->Draw(gameTime);
		mRenderStateHelper.RestoreAll();

		HRESULT hr = mSwapChain->Present(1, 0);

		// If the device was removed either by a disconnection or a driver upgrade, we must recreate all device resources.
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			HandleDeviceLost();
		}
		else
		{
			ThrowIfFailed(hr, "IDXGISwapChain::Present() failed.");
		}
	}

	void RenderingGame::Shutdown()
	{
		SamplerStates::Shutdown();
		RasterizerStates::Shutdown();
	}

	void RenderingGame::Exit()
	{
		PostQuitMessage(0);
	}
}