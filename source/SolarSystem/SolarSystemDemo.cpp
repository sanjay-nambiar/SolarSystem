#include "pch.h"
#include "ConfigData.h"

using namespace std;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	RTTI_DEFINITIONS(SolarSystemDemo)

	const float SolarSystemDemo::LightModulationRate = UCHAR_MAX;

	SolarSystemDemo::SolarSystemDemo(Game & game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera), mPointLight(game, XMFLOAT3(5.0f, 0.0f, 10.0f), 50.0f),
		mRenderStateHelper(game), mIndexCount(0), mTextPosition(0.0f, 40.0f), mAnimationEnabled(false)
	{
	}

	bool SolarSystemDemo::AnimationEnabled() const
	{
		return mAnimationEnabled;
	}

	void SolarSystemDemo::SetAnimationEnabled(bool enabled)
	{
		mAnimationEnabled = enabled;
	}

	void SolarSystemDemo::Initialize()
	{
		mConfigData.LoadConfigData("Content\\CelestialBodies.ini");
		const SectionData& data = mConfigData.GetConstantsData();
		CelestialBody::SetConstantParams(data.mMeanDistance, data.mRotationPeriod, data.mOrbitalPeriod, data.mDiameter);

		// Load a compiled vertex shader
		vector<char> compiledVertexShader;
		Utility::LoadBinaryFile(L"Content\\Shaders\\PointLightDemoVS.cso", compiledVertexShader);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedVertexShader() failed.");

		// Load a compiled pixel shader
		vector<char> compiledPixelShader;
		Utility::LoadBinaryFile(L"Content\\Shaders\\PointLightDemoPS.cso", compiledPixelShader);
		ThrowIfFailed(mGame->Direct3DDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedPixelShader() failed.");

		// Create an input layout
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		ThrowIfFailed(mGame->Direct3DDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), mInputLayout.ReleaseAndGetAddressOf()), "ID3D11Device::CreateInputLayout() failed.");

		// Load the model
		Library::Model model("Content\\Models\\Sphere.obj.bin");

		// Create vertex and index buffers for the model
		Library::Mesh* mesh = model.Meshes().at(0).get();
		CreateVertexBuffer(*mesh, mVertexBuffer.ReleaseAndGetAddressOf());
		mesh->CreateIndexBuffer(*mGame->Direct3DDevice(), mIndexBuffer.ReleaseAndGetAddressOf());
		mIndexCount = static_cast<uint32_t>(mesh->Indices().size());

		// Create constant buffers
		D3D11_BUFFER_DESC constantBufferDesc = { 0 };
		constantBufferDesc.ByteWidth = sizeof(VSCBufferPerFrame);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVSCBufferPerFrame.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(VSCBufferPerObject);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVSCBufferPerObject.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(PSCBufferPerFrame);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mPSCBufferPerFrame.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		// Load textures for the color and specular maps
		for (const auto& sectionEntry : mConfigData.GetAllData())
		{
			const auto& section = sectionEntry.second;
			if (mColorTextures.find(section.mTextureName) == mColorTextures.end())
			{
				wstring textureName = L"Content\\Textures\\" + Utility::ToWideString(section.mTextureName);
				Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;
				ThrowIfFailed(CreateWICTextureFromFile(mGame->Direct3DDevice(), textureName.c_str(), nullptr,
					textureView.ReleaseAndGetAddressOf()), "CreateWICTextureFromFile() failed.");
				mColorTextures.insert({section.mTextureName, textureView});
			}
			CelestialBody body;
			body.SetParams(section.mTextureName, section.mMeanDistance, section.mRotationPeriod, section.mOrbitalPeriod,
				section.mAxialTilt, section.mDiameter, section.mAlbedo);
			mCelestialBodies.push_back(body);
		}

		// Create text rendering helpers
		mSpriteBatch = make_unique<SpriteBatch>(mGame->Direct3DDeviceContext());
		mSpriteFont = make_unique<SpriteFont>(mGame->Direct3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");

		// Retrieve the keyboard service
		mKeyboard = reinterpret_cast<KeyboardComponent*>(mGame->Services().GetService(KeyboardComponent::TypeIdClass()));
		
		// Setup the point light
		mVSCBufferPerFrameData.LightPosition = mPointLight.Position();
		mVSCBufferPerFrameData.LightRadius = mPointLight.Radius();
		mPSCBufferPerFrameData.LightPosition = mPointLight.Position();
		mPSCBufferPerFrameData.LightColor = ColorHelper::ToFloat3(mPointLight.Color(), true);

		// Update the vertex and pixel shader constant buffers
		mGame->Direct3DDeviceContext()->UpdateSubresource(mVSCBufferPerFrame.Get(), 0, nullptr, &mVSCBufferPerFrameData, 0, 0);
		mGame->Direct3DDeviceContext()->UpdateSubresource(mPSCBufferPerFrame.Get(), 0, nullptr, &mPSCBufferPerFrameData, 0, 0);

		// Load a proxy model for the point light
		mProxyModel = make_unique<ProxyModel>(*mGame, mCamera, "Content\\Models\\PointLightProxy.obj.bin", 0.5f);
		mProxyModel->Initialize();
		mProxyModel->SetPosition(mPointLight.Position());
	}

	void SolarSystemDemo::Update(const GameTime& gameTime)
	{
		if (mKeyboard != nullptr)
		{
			if (mKeyboard->WasKeyPressedThisFrame(Keys::Space))
			{
				ToggleAnimation();
			}

			bool updatePSCBufferPerFrame = UpdateAmbientLight(gameTime);
			bool updateCBuffersPerFrame = UpdatePointLight(gameTime);

			if (updateCBuffersPerFrame)
			{
				mGame->Direct3DDeviceContext()->UpdateSubresource(mVSCBufferPerFrame.Get(), 0, nullptr, &mVSCBufferPerFrameData, 0, 0);
				mGame->Direct3DDeviceContext()->UpdateSubresource(mPSCBufferPerFrame.Get(), 0, nullptr, &mPSCBufferPerFrameData, 0, 0);
			}
			else if(updatePSCBufferPerFrame)
			{
				mGame->Direct3DDeviceContext()->UpdateSubresource(mPSCBufferPerFrame.Get(), 0, nullptr, &mPSCBufferPerFrameData, 0, 0);
			}
		}

		if (mAnimationEnabled)
		{
			for (auto& body : mCelestialBodies)
			{
				body.Update(gameTime);
			}
		}
		mProxyModel->Update(gameTime);
	}

	void SolarSystemDemo::Draw(const GameTime& gameTime)
	{
		UNREFERENCED_PARAMETER(gameTime);
		assert(mCamera != nullptr);

		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());

		UINT stride = sizeof(VertexPositionTextureNormal);
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

		ID3D11Buffer* PSConstantBuffers[] = {mPSCBufferPerFrame.Get()};
		direct3DDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(PSConstantBuffers), PSConstantBuffers);

		for (const auto& body : mCelestialBodies)
		{
			XMMATRIX worldMatrix = XMLoadFloat4x4(&body.WorldTransform());
			XMMATRIX wvp = worldMatrix * mCamera->ViewProjectionMatrix();
			wvp = XMMatrixTranspose(wvp);
			XMStoreFloat4x4(&mVSCBufferPerObjectData.WorldViewProjection, wvp);
			XMStoreFloat4x4(&mVSCBufferPerObjectData.World, XMMatrixTranspose(worldMatrix));

			direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, &mVSCBufferPerObjectData, 0, 0);
			ID3D11Buffer* VSConstantBuffers[] = {mVSCBufferPerFrame.Get(), mVSCBufferPerObject.Get()};
			direct3DDeviceContext->VSSetConstantBuffers(0, ARRAYSIZE(VSConstantBuffers), VSConstantBuffers);

			ID3D11ShaderResourceView* PSShaderResources[] = { mColorTextures[body.TextureName()].Get()};
			direct3DDeviceContext->PSSetShaderResources(0, ARRAYSIZE(PSShaderResources), PSShaderResources);
			direct3DDeviceContext->PSSetSamplers(0, 1, SamplerStates::TrilinearWrap.GetAddressOf());

			direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
		}

		mProxyModel->Draw(gameTime);

		// Draw help text
		mRenderStateHelper.SaveAll();
		mSpriteBatch->Begin();

		wostringstream helpLabel;
		helpLabel << "Ambient Intensity (+PgUp/-PgDn): " << mPSCBufferPerFrameData.AmbientColor.x << "\n";
		helpLabel << L"Point Light Intensity (+Home/-End): " << mPSCBufferPerFrameData.LightColor.x << "\n";
		helpLabel << L"Point Light Radius (+V/-B): " << mVSCBufferPerFrameData.LightRadius << "\n";
		helpLabel << L"Move Point Light (8/2, 4/6, 3/9)" << "\n";
		helpLabel << L"Toggle Grid (G)" << "\n";
		helpLabel << L"Toggle Animation (Space)" << "\n";
	
		mSpriteFont->DrawString(mSpriteBatch.get(), helpLabel.str().c_str(), mTextPosition);
		mSpriteBatch->End();
		mRenderStateHelper.RestoreAll();
	}

	void SolarSystemDemo::CreateVertexBuffer(const Mesh& mesh, ID3D11Buffer** vertexBuffer) const
	{
		const vector<XMFLOAT3>& sourceVertices = mesh.Vertices();
		const vector<XMFLOAT3>& sourceNormals = mesh.Normals();
		const auto& sourceUVs = mesh.TextureCoordinates().at(0);

		vector<VertexPositionTextureNormal> vertices;
		vertices.reserve(sourceVertices.size());
		for (UINT i = 0; i < sourceVertices.size(); i++)
		{
			const XMFLOAT3& position = sourceVertices.at(i);
			const XMFLOAT3& uv = sourceUVs->at(i);
			const XMFLOAT3& normal = sourceNormals.at(i);

			vertices.push_back(VertexPositionTextureNormal(XMFLOAT4(position.x, position.y, position.z, 1.0f), XMFLOAT2(uv.x, uv.y), normal));
		}
		D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
		vertexBufferDesc.ByteWidth = sizeof(VertexPositionTextureNormal) * static_cast<UINT>(vertices.size());
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData = { 0 };
		vertexSubResourceData.pSysMem = &vertices[0];
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer), "ID3D11Device::CreateBuffer() failed.");
	}

	void SolarSystemDemo::ToggleAnimation()
	{
		mAnimationEnabled = !mAnimationEnabled;
	}

	bool SolarSystemDemo::UpdateAmbientLight(const GameTime& gameTime)
	{
		static float ambientIntensity = mPSCBufferPerFrameData.AmbientColor.x;

		assert(mKeyboard != nullptr);

		if (mKeyboard->IsKeyDown(Keys::PageUp) && ambientIntensity < 1.0f)
		{
			ambientIntensity += gameTime.ElapsedGameTimeSeconds().count();
			ambientIntensity = min(ambientIntensity, 1.0f);

			mPSCBufferPerFrameData.AmbientColor = XMFLOAT3(ambientIntensity, ambientIntensity, ambientIntensity);
			return true;
		}
		else if (mKeyboard->IsKeyDown(Keys::PageDown) && ambientIntensity > 0.0f)
		{
			ambientIntensity -= gameTime.ElapsedGameTimeSeconds().count();
			ambientIntensity = max(ambientIntensity, 0.0f);

			mPSCBufferPerFrameData.AmbientColor = XMFLOAT3(ambientIntensity, ambientIntensity, ambientIntensity);
			return true;
		}
		return false;
	}

	bool SolarSystemDemo::UpdatePointLight(const GameTime& gameTime)
	{
		static float lightIntensity = mPSCBufferPerFrameData.LightColor.x;

		assert(mKeyboard != nullptr);

		float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();
		bool updateCBuffer = false;

		// Update point light intensity
		if (mKeyboard->IsKeyDown(Keys::Home) && lightIntensity < 1.0f)
		{
			lightIntensity += elapsedTime;
			lightIntensity = min(lightIntensity, 1.0f);

			mPSCBufferPerFrameData.LightColor = XMFLOAT3(lightIntensity, lightIntensity, lightIntensity);
			mPointLight.SetColor(mPSCBufferPerFrameData.LightColor.x, mPSCBufferPerFrameData.LightColor.y, mPSCBufferPerFrameData.LightColor.z, 1.0f);
			updateCBuffer = true;
		}
		else if (mKeyboard->IsKeyDown(Keys::End) && lightIntensity > 0.0f)
		{
			lightIntensity -= elapsedTime;
			lightIntensity = max(lightIntensity, 0.0f);

			mPSCBufferPerFrameData.LightColor = XMFLOAT3(lightIntensity, lightIntensity, lightIntensity);
			mPointLight.SetColor(mPSCBufferPerFrameData.LightColor.x, mPSCBufferPerFrameData.LightColor.y, mPSCBufferPerFrameData.LightColor.z, 1.0f);
			updateCBuffer = true;
		}

		// Update the light's radius
		if (mKeyboard->IsKeyDown(Keys::V))
		{
			float radius = mPointLight.Radius() + LightModulationRate * elapsedTime;
			mPointLight.SetRadius(radius);
			mVSCBufferPerFrameData.LightRadius = mPointLight.Radius();
			updateCBuffer = true;
		}

		if (mKeyboard->IsKeyDown(Keys::B))
		{
			float radius = mPointLight.Radius() - LightModulationRate * elapsedTime;
			radius = max(radius, 0.0f);
			mPointLight.SetRadius(radius);
			mVSCBufferPerFrameData.LightRadius = mPointLight.Radius();
			updateCBuffer = true;
		}

		if (updateCBuffer)
		{
			return true;
		}
		return false;
	}
}
