#pragma once

#include "Game.h"
#include "RenderStateHelper.h"
#include <windows.h>
#include <functional>

namespace Library
{
	class KeyboardComponent;
	class MouseComponent;
	class GamePadComponent;
	class FpsComponent;
	class Camera;
	class Grid;
}

namespace Rendering
{
	class SolarSystemDemo;

	class RenderingGame final : public Library::Game
	{
	public:
		RenderingGame(std::function<void*()> getWindowCallback, std::function<void(SIZE&)> getRenderTargetSizeCallback);

		void Initialize() override;
		void Update(const Library::GameTime& gameTime) override;
		void Draw(const Library::GameTime& gameTime) override;
		void Shutdown() override;

		void Exit();

	private:
		static const DirectX::XMVECTORF32 BackgroundColor;
		static const DirectX::XMFLOAT3 CameraStart;
		static const float MinCameraMovementRate;
		static const float MaxCameraMovementRate;
		static const float MovementRateDelta;

		Library::RenderStateHelper mRenderStateHelper;
		std::shared_ptr<Library::KeyboardComponent> mKeyboard;
		std::shared_ptr<Library::MouseComponent> mMouse;
		std::shared_ptr<Library::GamePadComponent> mGamePad;
		std::shared_ptr<Library::FpsComponent> mFpsComponent;
		std::shared_ptr<Library::FirstPersonCamera> mCamera;
		std::shared_ptr<SolarSystemDemo> mSolarSystemDemo;
		std::shared_ptr<Library::Skybox> mSkybox;
	};
}
