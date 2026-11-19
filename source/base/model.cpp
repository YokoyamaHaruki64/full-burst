#include "directx.h"
#include "texture.h"
#include "model.h"
#include "sprite.h"
#include "DirectXTex.h"
#include "shader.h"

// 埋め込みか外部ファイルか設定
#define USE_EMBEDDED_TEXTURE
// #define USE_EXTERNAL_FILE_TEXTURE

MODEL* ModelLoad( const char *FileName)
{
	MODEL* model = new MODEL;

	const std::string modelPath( FileName );

	model->AiScene = aiImportFile(FileName, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);
	assert(model->AiScene);

	model->VertexBuffer = new ID3D11Buffer*[model->AiScene->mNumMeshes];
	model->VertexBufferShadow = new ID3D11Buffer* [model->AiScene->mNumMeshes];
	model->IndexBuffer = new ID3D11Buffer*[model->AiScene->mNumMeshes];


	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = model->AiScene->mMeshes[m];

		// 頂点バッファ生成
		{
			Vertex* vertex = new Vertex[mesh->mNumVertices];
			VertexShadow* vertexShadow = new VertexShadow[mesh->mNumVertices];

			for (unsigned int v = 0; v < mesh->mNumVertices; v++)
			{
				vertex[v].pos = XMFLOAT3(mesh->mVertices[v].x, -mesh->mVertices[v].z, mesh->mVertices[v].y);
				vertexShadow[v].pos = XMFLOAT3(mesh->mVertices[v].x, -mesh->mVertices[v].z, mesh->mVertices[v].y);
				// テクスチャ座標が存在するかチェック
				if (mesh->mTextureCoords[0] != nullptr)
				{
					vertex[v].texcoord = XMFLOAT2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
				}
				else
				{
					// デフォルトのUV座標を設定
					vertex[v].texcoord = XMFLOAT2(0.0f, 0.0f);
				}
				
				vertex[v].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
				
				// 法線も同様にチェックすることを推奨
				if (mesh->mNormals != nullptr)
				{
					vertex[v].normal = XMFLOAT3(mesh->mNormals[v].x, -mesh->mNormals[v].z, mesh->mNormals[v].y);
				}
				else
				{
					vertex[v].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
				}
			}

			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.ByteWidth = sizeof(Vertex) * mesh->mNumVertices;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			D3D11_SUBRESOURCE_DATA sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.pSysMem = vertex;

			DirectXGetDevice()->CreateBuffer(&bd, &sd, &model->VertexBuffer[m]);

			bd.ByteWidth = sizeof(VertexShadow) * mesh->mNumVertices;
			sd.pSysMem = vertexShadow;

			DirectXGetDevice()->CreateBuffer(&bd, &sd, &model->VertexBufferShadow[m]);

			delete[] vertex;
			delete[] vertexShadow;
		}


		// インデックスバッファ生成
		{
			unsigned int* index = new unsigned int[mesh->mNumFaces * 3];

			for (unsigned int f = 0; f < mesh->mNumFaces; f++)
			{
				const aiFace* face = &mesh->mFaces[f];

				//assert(face->mNumIndices == 3);

				index[f * 3 + 0] = face->mIndices[0];
				index[f * 3 + 1] = face->mIndices[1];
				index[f * 3 + 2] = face->mIndices[2];
			}

			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(unsigned int) * mesh->mNumFaces * 3;
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.CPUAccessFlags = 0;

			D3D11_SUBRESOURCE_DATA sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.pSysMem = index;

			DirectXGetDevice()->CreateBuffer(&bd, &sd, &model->IndexBuffer[m]);

			delete[] index;
		}

	}

#ifdef USE_EMBEDDED_TEXTURE
	// テクスチャ読み込み
	// 埋め込みの簡易的な読み込み
	for (int i = 0; i < model->AiScene->mNumTextures; i++)
	{
		aiTexture* aitexture = model->AiScene->mTextures[i];

		ID3D11ShaderResourceView* texture;
		TexMetadata metadata;
		ScratchImage image;
		LoadFromWICMemory(aitexture->pcData, aitexture->mWidth, WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(DirectXGetDevice(), image.GetImages(), image.GetImageCount(), metadata, &texture);
		assert(texture);

		model->Texture[aitexture->mFilename.data] = texture;
	}
#endif // USE_EMBEDDED_TEXTURE

#ifdef USE_EXTERNAL_FILE_TEXTURE


	// ---------- テクスチャ読み込み（外部ファイル + 埋め込み対応、複数テクスチャ対応） ----------
	// マテリアル内の各種テクスチャスロットを走査してロードする（重複は避ける）
	const aiTextureType textureTypes[] = {
		aiTextureType_DIFFUSE,
		aiTextureType_SPECULAR,
		aiTextureType_HEIGHT,
		aiTextureType_NORMALS,
		aiTextureType_EMISSIVE,
		aiTextureType_UNKNOWN
	};

	for (unsigned int mi = 0; mi < model->AiScene->mNumMaterials; mi++)
	{
		// マテリアルを取得
		aiMaterial* material = model->AiScene->mMaterials[mi];

		// 各種テクスチャスロットを走査
		for (auto tt : textureTypes)
		{
			// スロット内のテクスチャ数を取得
			unsigned int count = material->GetTextureCount(tt);
			for (int ti = 0; ti < count;ti++)
			{
				// テクスチャ名を取得
				aiString texPath;
				if (material->GetTexture(tt, ti, &texPath) != AI_SUCCESS) continue;
				// stringに変換
				std::string key = texPath.C_Str();
				if (key.empty()) continue;

				// すでに読み込んでいたらスキップ
				if (model->Texture.find(key) != model->Texture.end()) continue;

				ID3D11ShaderResourceView* srv = nullptr;

				// 外部ファイルパスを作る
				std::string filePath = "asset/model/texture/" + key;

				// 文字列→ワイド文字列変換（簡易）
				std::wstring wfile(filePath.begin(), filePath.end());

				// WIC 対応フォーマットを想定してファイルから読み込み
				TexMetadata metadata;
				ScratchImage image;
				HRESULT hr = LoadFromWICFile(wfile.c_str(), WIC_FLAGS_NONE, &metadata, image);
				// 読み込み成功ならシェーダーリソースビューを作成
				if (SUCCEEDED(hr))
				{
					hr = CreateShaderResourceView(DirectXGetDevice(), image.GetImages(), image.GetImageCount(), metadata, &srv);
					assert(SUCCEEDED(hr) && srv);
				}
				else
				{
					// 失敗したら試しに拡張子を大文字小文字変えてみる、絶対パスにする、などのフォールバックを行ってください
				}

				// 成功したら map に登録（キーは元の aiString）
				if (srv)
				{
					model->Texture[key] = srv;
				}
			}
		}
	}
#endif // USE_EXTERNAL_FILE_TEXTURE

	return model;
}


void ModelRelease(MODEL* model)
{
	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		model->VertexBuffer[m]->Release();
		model->VertexBufferShadow[m]->Release();
		model->IndexBuffer[m]->Release();
	}

	delete[] model->VertexBuffer;
	delete[] model->VertexBufferShadow;
	delete[] model->IndexBuffer;


	for (std::pair<const std::string, ID3D11ShaderResourceView*> pair : model->Texture)
	{
		pair.second->Release();
	}


	aiReleaseImport(model->AiScene);


	delete model;
}


void ModelDraw(MODEL* model)
{
	// プリミティブトポロジ設定
	DirectXGetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = model->AiScene->mMeshes[m];

		// テクスチャ設定
		aiString texture;  
		aiMaterial* aimaterial = model->AiScene->mMaterials[mesh->mMaterialIndex];
		aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texture);

		if (texture != aiString(""))
			DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &model->Texture[texture.data]);

		// 頂点バッファ設定
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		DirectXGetDeviceContext()->IASetVertexBuffers(0, 1, &model->VertexBuffer[m], &stride, &offset);

		// インデックスバッファ設定
		DirectXGetDeviceContext()->IASetIndexBuffer(model->IndexBuffer[m], DXGI_FORMAT_R32_UINT, 0);

		// ポリゴン描画
		DirectXGetDeviceContext()->DrawIndexed(mesh->mNumFaces * 3, 0, 0);
	}
}

void ModelDrawInstanced(
	MODEL* model,
	ID3D11Buffer* instanceBuffer,
	UINT instanceCount
)
{
	DirectXGetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = model->AiScene->mMeshes[m];

		// テクスチャ設定
		aiString texture;
		aiMaterial* aimaterial = model->AiScene->mMaterials[mesh->mMaterialIndex];
		aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texture);

		if (texture != aiString(""))
			DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &model->Texture[texture.data]);
		
		// 頂点バッファ設定
		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		// 1番目のスロットにモデル頂点バッファを設定
		DirectXGetDeviceContext()->IASetVertexBuffers(0, 1, &model->VertexBuffer[m], &stride, &offset);

		// 2番目のスロットにインスタンスデータ用頂点バッファを設定
		stride = sizeof(DirectX::XMFLOAT4X4);	// インスタンスデータのサイズ
		DirectXGetDeviceContext()->IASetVertexBuffers(1,1, &instanceBuffer, &stride, &offset);

		// インデックスバッファ設定
		DirectXGetDeviceContext()->IASetIndexBuffer(model->IndexBuffer[m], DXGI_FORMAT_R32_UINT, 0);

		// インスタンシング描画
		DirectXGetDeviceContext()->DrawIndexedInstanced(
			mesh->mNumFaces * 3,
			instanceCount,
			0, 0, 0);
	}
}

// インスタンシング用のAPI
ID3D11ShaderResourceView* ModelGetTexture(MODEL* model)
{
	// 最初のマテリアルのディフューズテクスチャを返す
	if (model->AiScene->mNumMeshes > 0)
	{
		aiMesh* mesh = model->AiScene->mMeshes[0];
		aiMaterial* aimaterial = model->AiScene->mMaterials[mesh->mMaterialIndex];
		aiString texture;
		aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texture);
		if (texture != aiString(""))
		{
			return model->Texture[texture.data];
		}
	}

	return nullptr;
}

void ModelDrawShadow(MODEL* model)
{
	// プリミティブトポロジ設定
	DirectXGetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (unsigned int m = 0; m < model->AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = model->AiScene->mMeshes[m];
		// 頂点バッファ設定
		UINT stride = sizeof(VertexShadow);
		UINT offset = 0;
		DirectXGetDeviceContext()->IASetVertexBuffers(0, 1, &model->VertexBufferShadow[m], &stride, &offset);
		// インデックスバッファ設定
		DirectXGetDeviceContext()->IASetIndexBuffer(model->IndexBuffer[m], DXGI_FORMAT_R32_UINT, 0);
		// シャドウマップ書き込み
		DirectXGetDeviceContext()->DrawIndexed(mesh->mNumFaces * 3, 0, 0);
	}
}
