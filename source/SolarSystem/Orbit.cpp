#include "pch.h"

using namespace DirectX;
using namespace Library;

namespace Rendering
{
	RTTI_DEFINITIONS(Orbit)

	const XMFLOAT4 Orbit::DefaultColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	Orbit::Orbit(Game& game, const std::shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera), mVertexShader(nullptr), mPixelShader(nullptr), mInputLayout(nullptr),
		mVertexBuffer(nullptr), mVertexCBufferPerObject(nullptr), mVertexCBufferPerObjectData(), mColor(DefaultColor),
		mRadius(0), mTransform(MatrixHelper::Identity), mWorldMatrix(MatrixHelper::Identity), mVertexCount(0)
	{
	}

	void Orbit::Initialize()
	{
		// Load a compiled vertex shader
		std::vector<char> compiledVertexShader;
		Utility::LoadBinaryFile(L"Content\\Shaders\\BasicVS.cso", compiledVertexShader);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.GetAddressOf()),
			"ID3D11Device::CreatedVertexShader() failed.");

		// Load a compiled pixel shader
		std::vector<char> compiledPixelShader;
		Utility::LoadBinaryFile(L"Content\\Shaders\\BasicPS.cso", compiledPixelShader);
		ThrowIfFailed(mGame->Direct3DDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.GetAddressOf()),
			"ID3D11Device::CreatedPixelShader() failed.");

		// Create an input layout
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		ThrowIfFailed(mGame->Direct3DDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0],
			compiledVertexShader.size(), mInputLayout.GetAddressOf()), "ID3D11Device::CreateInputLayout() failed.");

		// Create constant buffers
		D3D11_BUFFER_DESC constantBufferDesc = {0};
		constantBufferDesc.ByteWidth = sizeof(VertexCBufferPerObject);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVertexCBufferPerObject.GetAddressOf()),
			"ID3D11Device::CreateBuffer() failed.");
	}

	void Orbit::SetParams(float radius, float vertexPerUnit, const DirectX::XMFLOAT4X4& transform, const DirectX::XMFLOAT4& color)
	{
		mRadius = radius;
		mTransform = transform;
		mColor = color;

		mVertexBuffer.Reset();
		mVertexCount = static_cast<std::uint32_t>(ceil((2 * XM_PI * mRadius) * vertexPerUnit));
		ID3D11Device* direct3DDevice = GetGame()->Direct3DDevice();
		int size = sizeof(VertexPositionColor) * mVertexCount;
		std::unique_ptr<VertexPositionColor> vertexData(new VertexPositionColor[mVertexCount]);
		VertexPositionColor* vertices = vertexData.get();

		float angle = 0;
		float angleIncrement = (XM_2PI / mVertexCount);
		for (std::uint32_t index = 0; index < mVertexCount; ++index)
		{
			vertices[index] = VertexPositionColor(XMFLOAT4(mRadius * cos(angle), 0.0f, mRadius * sin(angle), 1.0f), mColor);
			angle += angleIncrement;
		}

		D3D11_BUFFER_DESC vertexBufferDesc = {0};
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = size;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData = { 0 };
		vertexSubResourceData.pSysMem = vertices;

		ThrowIfFailed(direct3DDevice->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, mVertexBuffer.GetAddressOf()),
			"ID3D11Device::CreateBuffer() failed");
	}

	void Orbit::Draw(const GameTime&)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
		direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());

		UINT stride = sizeof(VertexPositionColor);
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);

		direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

		XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
		XMMATRIX wvp = worldMatrix * mCamera->ViewProjectionMatrix();
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.WorldViewProjection, XMMatrixTranspose(wvp));

		direct3DDeviceContext->UpdateSubresource(mVertexCBufferPerObject.Get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->VSSetConstantBuffers(0, 1, mVertexCBufferPerObject.GetAddressOf());

		direct3DDeviceContext->Draw(mVertexCount, 0);
	}
}
