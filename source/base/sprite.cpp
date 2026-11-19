#include "directx.h"
#include "shader.h"
#include "texture.h"
#include "sprite.h"
#include "camera.h"

static ID3D11Buffer* g_VertexBuffer;	// 頂点バッファのポインタ
static ID3D11Buffer* g_BillboardBuffer;	// 頂点バッファのポインタ

void SpriteInitialize()
{
	//頂点バッファ生成
	D3D11_BUFFER_DESC bd{};		//DESC→デスクリプション
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(Vertex) * 4;			//バッファのサイズ(この場合頂点4つ分、四角形)
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;	//頂点バッファとして使うフラグ
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	DirectXGetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);	//頂点バッファをGPU上に作る(第3引数の変数を介してアクセスできるようになる)
	DirectXGetDevice()->CreateBuffer(&bd, NULL, &g_BillboardBuffer);	//頂点バッファをGPU上に作る(第3引数の変数を介してアクセスできるようになる)

 }

void SpriteFinalize()
{
	SAFE_RELEASE(g_VertexBuffer);
	SAFE_RELEASE(g_BillboardBuffer);
}

void SpriteDraw(Vector3 pos, Vector3 size, Vector3 scale, float rot, Color color,int texID,
				float u,float V,float uw,float vh)
{
	// 頂点バッファにデータを設定
	D3D11_MAPPED_SUBRESOURCE msr;
	DirectXGetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);	//（最初と最後の引数）アクセス先の頂点バッファ、帰ってくる情報
	// 正確にはアクセスするためにメモリ上に退避している(後述のUnmapで戻す)

	Vertex* v = (Vertex*)msr.pData;	// Vertexのポインタ型にキャストしたデータ(上の行でmsr.pDateにアクセスできる情報が入っている)
	v[0].pos = { - size.x / 2.0f, - size.y / 2.0f,  0.0f };
	v[1].pos = { + size.x / 2.0f, - size.y / 2.0f,  0.0f };
	v[2].pos = { - size.x / 2.0f, + size.y / 2.0f,  0.0f };
	v[3].pos = { + size.x / 2.0f, + size.y / 2.0f,  0.0f };

	XMFLOAT4 c = color.ToXMFLOAT4();
	v[0].color = c;
	v[1].color = c;
	v[2].color = c;
	v[3].color = c;

	v[0].texcoord = { u, V };
	v[1].texcoord = { u + uw, V };
	v[2].texcoord = { u, V + vh };
	v[3].texcoord = { u + uw, V + vh };

	DirectXGetDeviceContext()->Unmap(g_VertexBuffer, 0);

	//シェーダー設定
	Shader_Begin_2D();

	//頂点シェーダーに変換行列を設定
	XMMATRIX mtx;
	mtx = XMMatrixIdentity();// 初期値入れてくれる

	// GGでやった行列の掛け算
	mtx *= XMMatrixScaling(scale.x,scale.y,scale.z);

	mtx *= XMMatrixRotationZ(XMConvertToRadians(rot));

	mtx *= XMMatrixTranslation(pos.x, pos.y, pos.z);


	mtx *= XMMatrixOrthographicOffCenterLH(0.0f,WINDOW_WIDTH,
			WINDOW_HEIGHT,0.0f, 
			0.0f, 1.0f);


	Shader_SetMatrix_2D(mtx);

	//頂点バッファ設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	DirectXGetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	//プリミティブトポロジ設定
	DirectXGetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//テクスチャ設定
	ID3D11ShaderResourceView* texture = GetTexture(texID);
	DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &texture);

	//ポリゴン描画
	DirectXGetDeviceContext()->Draw(4, 0);

}

void SpriteDraw(Vector3 pos, Vector3 size, Vector3 scale, float rot, Color color, ID3D11ShaderResourceView* const* texture,
				float u,float V,float uw,float vh)
{
	// 頂点バッファにデータを設定
	D3D11_MAPPED_SUBRESOURCE msr;
	DirectXGetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);	//（最初と最後の引数）アクセス先の頂点バッファ、帰ってくる情報
	// 正確にはアクセスするためにメモリ上に退避している(後述のUnmapで戻す)

	Vertex* v = (Vertex*)msr.pData;	// Vertexのポインタ型にキャストしたデータ(上の行でmsr.pDateにアクセスできる情報が入っている)
	v[0].pos = { - size.x / 2.0f, - size.y / 2.0f,  0.0f };
	v[1].pos = { + size.x / 2.0f, - size.y / 2.0f,  0.0f };
	v[2].pos = { - size.x / 2.0f, + size.y / 2.0f,  0.0f };
	v[3].pos = { + size.x / 2.0f, + size.y / 2.0f,  0.0f };

	XMFLOAT4 c = color.ToXMFLOAT4();
	v[0].color = c;
	v[1].color = c;
	v[2].color = c;
	v[3].color = c;

	v[0].texcoord = { u, V };
	v[1].texcoord = { u + uw, V };
	v[2].texcoord = { u, V + vh };
	v[3].texcoord = { u + uw, V + vh };

	DirectXGetDeviceContext()->Unmap(g_VertexBuffer, 0);

	//シェーダー設定
	Shader_Begin_2D();

	//頂点シェーダーに変換行列を設定
	XMMATRIX mtx;
	mtx = XMMatrixIdentity();// 初期値入れてくれる

	// GGでやった行列の掛け算
	mtx *= XMMatrixScaling(scale.x,scale.y,scale.z);

	mtx *= XMMatrixRotationZ(XMConvertToRadians(rot));

	mtx *= XMMatrixTranslation(pos.x, pos.y, pos.z);


	mtx *= XMMatrixOrthographicOffCenterLH(0.0f,WINDOW_WIDTH,
			WINDOW_HEIGHT,0.0f, 
			0.0f, 1.0f);


	Shader_SetMatrix_2D(mtx);

	//頂点バッファ設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	DirectXGetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	//プリミティブトポロジ設定
	DirectXGetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//テクスチャ設定
	DirectXGetDeviceContext()->PSSetShaderResources(0, 1, texture);

	//ポリゴン描画
	DirectXGetDeviceContext()->Draw(4, 0);

}

void BillboardDraw(Vector3 pos,Vector3 scale, float rot, Color color, int texID,
	Vector3 offset,	float u, float V, float uw, float vh)
{
	// 頂点バッファにデータを設定
	D3D11_MAPPED_SUBRESOURCE msr;
	DirectXGetDeviceContext()->Map(g_BillboardBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	Vertex* v = (Vertex*)msr.pData;

	v[0].pos = XMFLOAT3( -0.5f, +0.5f,  0.0f );
	v[1].pos = XMFLOAT3( +0.5f, +0.5f,  0.0f );
	v[2].pos = XMFLOAT3( -0.5f, -0.5f,  0.0f );
	v[3].pos = XMFLOAT3( +0.5f, -0.5f,  0.0f );

	XMFLOAT4 c = color.ToXMFLOAT4();
	v[0].color = c;
	v[1].color = c;
	v[2].color = c;
	v[3].color = c;

	v[0].texcoord = { u, V };
	v[1].texcoord = { u + uw, V };
	v[2].texcoord = { u, V + vh };
	v[3].texcoord = { u + uw, V + vh };

	v[0].normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	v[1].normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	v[2].normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	v[3].normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	DirectXGetDeviceContext()->Unmap(g_BillboardBuffer, 0);

	// シェーダー設定
	Shader_Begin_3D();
	Rimlight rim;
	Shader_SetRimlight(rim);

	Matrix mtx;
	mtx.world = XMMatrixIdentity();

	mtx.world *= XMMatrixScaling(scale.x, scale.y, scale.z);
	mtx.world *= XMMatrixTranslation(offset.x, offset.y, offset.z);

	mtx.world *= GetBillboardMatrix();
	mtx.world *= XMMatrixRotationZ(XMConvertToRadians(rot));
	mtx.world *= XMMatrixTranslation(pos.x, pos.y, pos.z);

	mtx.matrix = mtx.world;

	mtx.matrix *= GetCameraViewMatrix();

	mtx.matrix *= GetProjectionMatrix();

	Shader_SetMatrix_3D(mtx);

	//頂点バッファ設定
	UINT v_stride = sizeof(Vertex);
	UINT v_offset = 0;
	DirectXGetDeviceContext()->IASetVertexBuffers(0, 1, &g_BillboardBuffer, &v_stride, &v_offset);

	//プリミティブトポロジ設定
	DirectXGetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//テクスチャ設定
	ID3D11ShaderResourceView* texture = GetTexture(texID);
	DirectXGetDeviceContext()->PSSetShaderResources(0, 1, &texture);

	//ポリゴン描画
	DirectXGetDeviceContext()->Draw(4, 0);

}

