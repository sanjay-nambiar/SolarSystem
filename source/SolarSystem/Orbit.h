#pragma once

#include "DrawableGameComponent.h"
#include <wrl.h>
#include <d3d11_2.h>
#include <DirectXMath.h>

namespace Rendering
{
	class Orbit final : public Library::DrawableGameComponent
	{
		RTTI_DECLARATIONS(Orbit, DrawableGameComponent)

	public:
		Orbit(Library::Game& game, const std::shared_ptr<Library::Camera>& camera);

		Orbit() = delete;
		Orbit(const Orbit&) = delete;
		Orbit& operator=(const Orbit&) = delete;

		void Initialize() override;
		void SetParams(float radius, float vertexPerUnit, const DirectX::XMFLOAT4X4& transform, const DirectX::XMFLOAT4& color);
		void Draw(const Library::GameTime& gameTime) override;
	private:
		struct VertexCBufferPerObject
		{
			DirectX::XMFLOAT4X4 WorldViewProjection;

			VertexCBufferPerObject() { }
			VertexCBufferPerObject(const DirectX::XMFLOAT4X4& wvp) : WorldViewProjection(wvp) { }
		};

		static const DirectX::XMFLOAT4 DefaultColor;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexCBufferPerObject;
		VertexCBufferPerObject mVertexCBufferPerObjectData;

		DirectX::XMFLOAT4 mColor;
		float mRadius;
		DirectX::XMFLOAT4X4 mTransform;

		DirectX::XMFLOAT4X4 mWorldMatrix;
		uint32_t mVertexCount;
	};
}
