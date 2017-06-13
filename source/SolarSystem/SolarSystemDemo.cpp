#include "pch.h"
#include "ConfigData.h"

using namespace std;
using namespace Library;
using namespace DirectX;

namespace Rendering
{
	RTTI_DEFINITIONS(SolarSystemDemo)

	const float SolarSystemDemo::LightModulationRate = 10000000;
	const float SolarSystemDemo::SunLightDefaultIntensity = 93300000.0f * 100000;

	SolarSystemDemo::SolarSystemDemo(Game & game, const shared_ptr<Camera>& camera) :
		DrawableGameComponent(game, camera), mRootBody(nullptr), mSunLight(game, XMFLOAT3(0.0f, 0.0f, 0.0f), SunLightDefaultIntensity),
		mRenderStateHelper(game), mKeyboard(nullptr), mIndexCount(0), mTextPosition(0.0f, 40.0f), mAnimationEnabled(false), mIsOrbitsEnabled(true),
		mActiveBodyIndex(0), mIsCameraLocked(false), mIsInfoDisplayOn(true)
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
		const CelestialBodyData& data = mConfigData.GetConstantsData();
		CelestialBody::SetConstantParams(data.mMeanDistance, data.mRotationPeriod, data.mOrbitalPeriod, data.mDiameter);

		// Load a compiled vertex shader
		vector<char> compiledVertexShader;
		Utility::LoadBinaryFile(L"Content\\Shaders\\SolarSystemDemoVS.cso", compiledVertexShader);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.ReleaseAndGetAddressOf()),
			"ID3D11Device::CreatedVertexShader() failed.");

		// Load a compiled pixel shader
		vector<char> compiledPixelShader;
		Utility::LoadBinaryFile(L"Content\\Shaders\\SolarSystemDemoPS.cso", compiledPixelShader);
		ThrowIfFailed(mGame->Direct3DDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.ReleaseAndGetAddressOf()),
			"ID3D11Device::CreatedPixelShader() failed.");

		// Create an input layout
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		ThrowIfFailed(mGame->Direct3DDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(),
			mInputLayout.ReleaseAndGetAddressOf()), "ID3D11Device::CreateInputLayout() failed.");

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
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVSCBufferPerFrame.ReleaseAndGetAddressOf()),
			"ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(VSCBufferPerObject);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVSCBufferPerObject.ReleaseAndGetAddressOf()),
			"ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(PSCBufferPerFrame);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mPSCBufferPerFrame.ReleaseAndGetAddressOf()),
			"ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(PSCBufferPerObject);
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mPSCBufferPerObject.ReleaseAndGetAddressOf()),
			"ID3D11Device::CreateBuffer() failed.");

		unordered_map<string, CelestialBody*> bodiesMap;
		vector<string> parents;
		mCelestialBodies.reserve(mConfigData.GetAllData().size());
		// Load textures for the color and specular maps
		for (const auto& section : mConfigData.GetAllData())
		{
			if (mColorTextures.find(section.mTextureName) == mColorTextures.end())
			{
				wstring textureName = L"Content\\Textures\\" + Utility::ToWideString(section.mTextureName);
				Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;
				ThrowIfFailed(CreateWICTextureFromFile(mGame->Direct3DDevice(), textureName.c_str(), nullptr,
					textureView.ReleaseAndGetAddressOf()), "CreateWICTextureFromFile() failed.");
				mColorTextures.insert({section.mTextureName, textureView});
			}
			CelestialBody body(*mGame, mCamera);
			body.SetParams(section);
			mCelestialBodies.push_back(body);
			bodiesMap.insert({section.mName, &mCelestialBodies.back()});
			parents.push_back(section.mParent);
		}

		// Create text rendering helpers
		mSpriteBatch = make_unique<SpriteBatch>(mGame->Direct3DDeviceContext());
		mSpriteFont = make_unique<SpriteFont>(mGame->Direct3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont");

		// Retrieve the keyboard service
		mKeyboard = reinterpret_cast<KeyboardComponent*>(mGame->Services().GetService(KeyboardComponent::TypeIdClass()));

		// Setup the point light
		mVSCBufferPerFrameData.LightPosition = mSunLight.Position();
		mVSCBufferPerFrameData.LightIntensity = mSunLight.Intensity();
		mPSCBufferPerFrameData.LightPosition = mSunLight.Position();
		mPSCBufferPerFrameData.LightColor = ColorHelper::ToFloat3(mSunLight.Color(), true);

		// Update the vertex and pixel shader constant buffers
		mGame->Direct3DDeviceContext()->UpdateSubresource(mVSCBufferPerFrame.Get(), 0, nullptr, &mVSCBufferPerFrameData, 0, 0);
		mGame->Direct3DDeviceContext()->UpdateSubresource(mPSCBufferPerFrame.Get(), 0, nullptr, &mPSCBufferPerFrameData, 0, 0);

		uint32_t index = 0;
		for (auto& body : mCelestialBodies)
		{
			if (parents[index] == "")
			{
				mRootBody = &body;
			}
			else
			{
				CelestialBody* parent = bodiesMap.at(parents[index]);
				parent->Adopt(body);
			}
			++index;
		}
	}

	void SolarSystemDemo::Update(const GameTime& gameTime)
	{
		bool shouldUpdateCamera = false;
		if (mKeyboard != nullptr)
		{
			if (mKeyboard->WasKeyPressedThisFrame(Keys::Space))
			{
				ToggleAnimation();
			}

			if (mKeyboard->WasKeyPressedThisFrame(Keys::O))
			{
				mIsOrbitsEnabled = !mIsOrbitsEnabled;
			}

			bool updateCBuffersPerFrame = UpdateCelestialLight(gameTime);
			if (updateCBuffersPerFrame)
			{
				mGame->Direct3DDeviceContext()->UpdateSubresource(mVSCBufferPerFrame.Get(), 0, nullptr, &mVSCBufferPerFrameData, 0, 0);
			}

			if (mKeyboard->WasKeyPressedThisFrame(Keys::Left))
			{
				mActiveBodyIndex = (mActiveBodyIndex == 0) ? static_cast<std::uint32_t>(mCelestialBodies.size() - 1) : (mActiveBodyIndex - 1);
				shouldUpdateCamera = true;
				mCamera->Reset();
			}

			if (mKeyboard->WasKeyPressedThisFrame(Keys::Right))
			{
				mActiveBodyIndex = (mActiveBodyIndex + 1) % mCelestialBodies.size();
				shouldUpdateCamera = true;
				mCamera->Reset();
			}

			if (mKeyboard->WasKeyPressedThisFrame(Keys::L))
			{
				mIsCameraLocked = !mIsCameraLocked;
				mCamera->Reset();
			}

			if (mKeyboard->WasKeyPressedThisFrame(Keys::I))
			{
				mIsInfoDisplayOn = !mIsInfoDisplayOn;
			}
		}

		std::vector<CelestialBody*> stack = { mRootBody };
		if (mAnimationEnabled)
		{
			while (!stack.empty())
			{
				CelestialBody* body = stack.back();
				stack.pop_back();
				for(auto& child : body->Children())
				{
					stack.push_back(child);
				}
				body->Update(gameTime);
			}
		}

		if (shouldUpdateCamera || mIsCameraLocked)
		{
			UpdateCameraPosition();
		}
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

		for (auto& body : mCelestialBodies)
		{
			XMMATRIX worldMatrix = XMLoadFloat4x4(&body.WorldTransform());
			XMMATRIX wvp = worldMatrix * mCamera->ViewProjectionMatrix();
			wvp = XMMatrixTranspose(wvp);
			XMStoreFloat4x4(&mVSCBufferPerObjectData.WorldViewProjection, wvp);
			XMStoreFloat4x4(&mVSCBufferPerObjectData.World, XMMatrixTranspose(worldMatrix));

			direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, &mVSCBufferPerObjectData, 0, 0);
			ID3D11Buffer* VSConstantBuffers[] = {mVSCBufferPerFrame.Get(), mVSCBufferPerObject.Get()};
			direct3DDeviceContext->VSSetConstantBuffers(0, ARRAYSIZE(VSConstantBuffers), VSConstantBuffers);

			const auto& data = body.Data();
			mPSCBufferPerObjectData.LightingCoefficient = data.mIsLit;
			mPSCBufferPerObjectData.Reflectance = data.mReflectance;
			direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.Get(), 0, nullptr, &mPSCBufferPerObjectData, 0, 0);
			ID3D11Buffer* PSConstantBuffers[] = {mPSCBufferPerFrame.Get(), mPSCBufferPerObject.Get()};
			direct3DDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(PSConstantBuffers), PSConstantBuffers);

			ID3D11ShaderResourceView* PSShaderResources[] = { mColorTextures[data.mTextureName].Get() };
			direct3DDeviceContext->PSSetShaderResources(0, ARRAYSIZE(PSShaderResources), PSShaderResources);
			direct3DDeviceContext->PSSetSamplers(0, 1, SamplerStates::TrilinearWrap.GetAddressOf());

			direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);
		}

		if (mIsOrbitsEnabled)
		{
			for (auto& body : mCelestialBodies)
			{
				if (body.GetOrbit())
				{
					body.GetOrbit()->Draw(gameTime);
				}
			}
		}

		if (mIsInfoDisplayOn)
		{
			// Draw help text
			mRenderStateHelper.SaveAll();
			mSpriteBatch->Begin();

			wostringstream helpLabel;
			helpLabel << L"Active Body: " << Utility::ToWideString(mCelestialBodies[mActiveBodyIndex].Data().mName) << "\n";
			helpLabel << L"Camera Controls(WASD QE + Left Mouse)" << "\n";
			helpLabel << L"Sun Light Intensity (+V/-B): " << mVSCBufferPerFrameData.LightIntensity << "\n";
			helpLabel << L"Camera Movement Speed (+/-): " << static_cast<FirstPersonCamera*>(mCamera.get())->MovementRate() << "\n";
			helpLabel << L"Toggle Animation (Space)" << "\n";
			helpLabel << L"Toggle Orbits (O)" << "\n";
			helpLabel << L"Toggle Skybox (Y)" << "\n";
			helpLabel << L"Next Body / Previous Body (Left / Right)" << "\n";
			helpLabel << L"Toggle Camera Locking (L)" << "\n";
			helpLabel << L"Toggle Info Display (I)" << "\n";
			helpLabel << L"Exit (Esc)" << "\n";

			mSpriteFont->DrawString(mSpriteBatch.get(), helpLabel.str().c_str(), mTextPosition);
			mSpriteBatch->End();
			mRenderStateHelper.RestoreAll();
		}
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

	bool SolarSystemDemo::UpdateCelestialLight(const GameTime& gameTime)
	{
		bool updateCBuffer = false;
		float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();

		// Update the light's radius
		if (mKeyboard->IsKeyDown(Keys::V))
		{
			float radius = mSunLight.Intensity() + LightModulationRate * elapsedTime;
			mSunLight.SetIntensity(radius);
			mVSCBufferPerFrameData.LightIntensity = mSunLight.Intensity();
			updateCBuffer = true;
		}

		if (mKeyboard->IsKeyDown(Keys::B))
		{
			float radius = mSunLight.Intensity() - LightModulationRate * elapsedTime;
			radius = max(radius, 0.0f);
			mSunLight.SetIntensity(radius);
			mVSCBufferPerFrameData.LightIntensity = mSunLight.Intensity();
			updateCBuffer = true;
		}

		return updateCBuffer;
	}

	void SolarSystemDemo::UpdateCameraPosition()
	{
		CelestialBody& body = mCelestialBodies[mActiveBodyIndex];
		XMFLOAT4 origin(0.0f, 0.0f, 0.0f, 1.0f);
		XMVECTOR originVector = XMLoadFloat4(&origin);
		XMVECTOR bodyPosition = XMVector4Transform(originVector, XMLoadFloat4x4(&body.WorldTransform()));
		XMFLOAT4 offset(0.0f, 0.0f, body.Data().mDiameter * 20.0f, 1.0f);
		mCamera->SetPosition(bodyPosition + XMLoadFloat4(&offset));
	}
}
