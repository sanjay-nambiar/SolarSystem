#pragma once

#include "DrawableGameComponent.h"
#include "RenderStateHelper.h"
#include <DirectXMath.h>
#include "ConfigData.h"
#include "CelestialBody.h"
#include <unordered_map>

namespace Library
{
	class ProxyModel;
	class KeyboardComponent;
}

namespace DirectX
{
	class SpriteBatch;
	class SpriteFont;
}

namespace Rendering
{
	class SolarSystemDemo final : public Library::DrawableGameComponent
	{
		RTTI_DECLARATIONS(SolarSystemDemo, Library::DrawableGameComponent)

	public:
		SolarSystemDemo(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);

		bool AnimationEnabled() const;
		void SetAnimationEnabled(bool enabled);

		void Initialize() override;
		void Update(const Library::GameTime& gameTime) override;
		void Draw(const Library::GameTime& gameTime) override;

	private:
		struct VSCBufferPerFrame
		{
			DirectX::XMFLOAT3 LightPosition;
			float LightIntensity;

			VSCBufferPerFrame() :
				LightPosition(Library::Vector3Helper::Zero), LightIntensity(50.0f) { }
			VSCBufferPerFrame(const DirectX::XMFLOAT3 lightPosition, float lightRadius) :
				LightPosition(lightPosition), LightIntensity(lightRadius) { }
		};

		struct VSCBufferPerObject
		{
			DirectX::XMFLOAT4X4 WorldViewProjection;
			DirectX::XMFLOAT4X4 World;

			VSCBufferPerObject() = default;
			VSCBufferPerObject(const DirectX::XMFLOAT4X4& wvp, const DirectX::XMFLOAT4X4& world) :
				WorldViewProjection(wvp), World(world) { }
		};

		struct PSCBufferPerFrame
		{
			DirectX::XMFLOAT3 LightPosition;
			float Padding1;
			DirectX::XMFLOAT3 LightColor;
			float Padding2;

			PSCBufferPerFrame() :
				LightPosition(Library::Vector3Helper::Zero), LightColor(Library::Vector3Helper::Zero)
			{
			}

			PSCBufferPerFrame(const DirectX::XMFLOAT3& lightPosition, const DirectX::XMFLOAT3& lightColor) :
				LightPosition(lightPosition), LightColor(lightColor)
			{
			}
		};

		struct PSCBufferPerObject
		{
			float LightingCoefficient;
			float Reflectance;;
			float Padding1;
			float Padding2;

			PSCBufferPerObject() :
				LightingCoefficient(1.0f), Reflectance(1.0f)
			{
			}

			PSCBufferPerObject(float lightingCoefficient, float reflectance) :
				LightingCoefficient(lightingCoefficient), Reflectance(reflectance)
			{
			}
		};

		void CreateVertexBuffer(const Library::Mesh& mesh, ID3D11Buffer** vertexBuffer) const;
		void ToggleAnimation();
		bool UpdateCelestialLight(const Library::GameTime& gameTime);

		static const float LightModulationRate;

		ConfigData mConfigData;
		std::vector<CelestialBody> mCelestialBodies;
		CelestialBody* mRootBody;
		std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> mColorTextures;

		VSCBufferPerFrame mVSCBufferPerFrameData;
		VSCBufferPerObject mVSCBufferPerObjectData;
		PSCBufferPerFrame mPSCBufferPerFrameData;
		PSCBufferPerObject mPSCBufferPerObjectData;
		Rendering::CelestialLight mSunLight;
		Library::RenderStateHelper mRenderStateHelper;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVSCBufferPerFrame;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVSCBufferPerObject;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPSCBufferPerFrame;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPSCBufferPerObject;
		Library::KeyboardComponent* mKeyboard;
		std::uint32_t mIndexCount;
		std::unique_ptr<DirectX::SpriteBatch> mSpriteBatch;
		std::unique_ptr<DirectX::SpriteFont> mSpriteFont;
		DirectX::XMFLOAT2 mTextPosition;
		bool mAnimationEnabled;
	};
}
