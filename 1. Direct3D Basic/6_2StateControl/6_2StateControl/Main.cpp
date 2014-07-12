#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "winmm.lib")

LPDIRECT3D9					g_pD3D = NULL;
LPDIRECT3DDEVICE9		g_pDevice = NULL;

LPD3DXMESH					g_pMesh = NULL;
D3DMATERIAL9*				g_pMeshMaterials = NULL;
LPDIRECT3DTEXTURE9*	g_pMeshTextures = NULL;
DWORD							g_dwNumMaterials = 0L;

D3DXVECTOR3					g_vCenter;	//the center of the mesh
BYTE									g_bKey[256];
D3DXMATRIX					g_matWorld;

VOID setWorldMatrix(){
	static LONG curTime = 0;
	static FLOAT elapsedTime = 0;

	elapsedTime = (timeGetTime() - curTime) / 1000.0f;
	curTime = timeGetTime();

	//create & setup world matrix
	FLOAT fRoll, fPitch, fYaw;
	fRoll = fPitch = fYaw = 0.0f;

	if(g_bKey['D'])	fRoll -= 3 * elapsedTime;
	if(g_bKey['A'])	fRoll += 3 * elapsedTime;
	if(g_bKey['S'])	fPitch -= 3 * elapsedTime;
	if(g_bKey['W'])	fPitch += 3 * elapsedTime;
	if(g_bKey['Q'])	fYaw -= 3 * elapsedTime;
	if(g_bKey['E'])	fYaw += 3 * elapsedTime;

	//Update the mesh
	static D3DXVECTOR3 vRight, vUp, vLook, vPos;
	
	vRight.x = g_matWorld._11;
	vRight.y = g_matWorld._12;
	vRight.z = g_matWorld._13;
	vUp.x    = g_matWorld._21;
	vUp.y    = g_matWorld._22;
	vUp.z    = g_matWorld._23;
	vLook.x  = g_matWorld._31;
	vLook.y  = g_matWorld._32;
	vLook.z  = g_matWorld._33;
	vPos.x   = g_matWorld._41;
	vPos.y   = g_matWorld._42;
	vPos.z   = g_matWorld._43;

	D3DXVec3Normalize(&vLook, &vLook);
	D3DXVec3Cross(&vRight, &vUp, &vLook);
	D3DXVec3Normalize(&vRight, &vRight);
	D3DXVec3Cross(&vUp, &vLook, &vRight);
	D3DXVec3Normalize(&vUp, &vUp);

	static D3DXMATRIX matPitch, matYaw, matRoll;

	D3DXMatrixRotationAxis(&matYaw, &vUp, fYaw);
	D3DXVec3TransformCoord(&vLook, &vLook, &matYaw);
	D3DXVec3TransformCoord(&vRight, &vRight, &matYaw);

	D3DXMatrixRotationAxis(&matRoll, &vLook, fRoll);
	D3DXVec3TransformCoord(&vRight, &vRight, &matRoll);
	D3DXVec3TransformCoord(&vUp,    &vUp, &matRoll);

	D3DXMatrixRotationAxis(&matPitch, &vRight, fPitch);
	D3DXVec3TransformCoord(&vLook, &vLook, &matPitch);
	D3DXVec3TransformCoord(&vUp,   &vUp,  &matPitch);

	g_matWorld._11 = vRight.x;
	g_matWorld._12 = vRight.y;
	g_matWorld._13 = vRight.z;
	g_matWorld._21 = vUp.x ;
	g_matWorld._22 = vUp.y  ;
	g_matWorld._23 = vUp.z;
	g_matWorld._31 = vLook.x;
	g_matWorld._32 = vLook.y;
	g_matWorld._33 = vLook.z;

	//向前移动
	if (g_bKey['F']){
		g_matWorld._41 += 30*elapsedTime * vLook.x;
		g_matWorld._42 += 30*elapsedTime * vLook.y;
		g_matWorld._43 += 30*elapsedTime * vLook.z;
	}

	//向后移动
	if (g_bKey['V']){
		g_matWorld._41 -= 30*elapsedTime * vLook.x;
		g_matWorld._42 -= 30*elapsedTime * vLook.y;
		g_matWorld._43 -= 30*elapsedTime * vLook.z;
	}

	g_pDevice->SetTransform(D3DTS_WORLD, &g_matWorld);
}

VOID setViewProjMatrix(){
	//View matrix
	D3DXVECTOR3 vEyePt(0.0f, 10.0f, -20.0f);
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
	D3DXMATRIX matView;
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	g_pDevice->SetTransform(D3DTS_VIEW, &matView);
	//Projection Matrix
	D3DXMATRIX matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 500.0f);
	g_pDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}

HRESULT init3D(HWND hWnd){
	if(NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	D3DPRESENT_PARAMETERS d3dpp; 
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, &g_pDevice ) ) )
		return E_FAIL;
	//texture filter state
	g_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	g_pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
	g_pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	g_pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

	setViewProjMatrix();
	ZeroMemory(g_bKey, 256);

	D3DXMatrixIdentity(&g_matWorld);

	return S_OK;
}

///从绝对路径中提取纹理文件名
VOID removePathFromFileName(LPSTR fullPath, LPWSTR fileName){
	//先将fullPath的类型变换为LPWSTR
	WCHAR wszBuf[MAX_PATH];
	MultiByteToWideChar( CP_ACP, 0, fullPath, -1, wszBuf, MAX_PATH );
	wszBuf[MAX_PATH-1] = L'\0';

	WCHAR* wszFullPath = wszBuf;

	//从绝对路径中提取文件名
	LPWSTR pch=wcsrchr(wszFullPath,'\\');
	if (pch)
		lstrcpy(fileName, ++pch);
	else
		lstrcpy(fileName, wszFullPath);
}

HRESULT initGeometry(){
	LPD3DXBUFFER pD3DXMtrlBuffer;
	if(FAILED(D3DXLoadMeshFromX(L"airplane.x", D3DXMESH_MANAGED,
		g_pDevice, NULL, &pD3DXMtrlBuffer, NULL, &g_dwNumMaterials, &g_pMesh)))
		return E_FAIL;
	//Extract material & texture
	D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
	g_pMeshMaterials = new D3DMATERIAL9[g_dwNumMaterials];

	if(g_pMeshMaterials == NULL)
		return E_OUTOFMEMORY;

	g_pMeshTextures = new LPDIRECT3DTEXTURE9[g_dwNumMaterials];
	if(g_pMeshTextures == NULL)
		return E_OUTOFMEMORY;
	//Extract
	for(DWORD i=0; i<g_dwNumMaterials; ++i){
		g_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;
		g_pMeshMaterials[i].Ambient = g_pMeshMaterials[i].Diffuse;

		g_pMeshTextures[i] = NULL;
		if(d3dxMaterials[i].pTextureFilename != NULL && strlen(d3dxMaterials[i].pTextureFilename) > 0){
			WCHAR name[256];
			removePathFromFileName(d3dxMaterials[i].pTextureFilename, name);

			if(FAILED(D3DXCreateTextureFromFile(g_pDevice, name, &g_pMeshTextures[i]))){
				MessageBox(NULL, L"Cound not find texture file", L"initGeometry()", MB_OK);
			}
		}
	}
	pD3DXMtrlBuffer->Release();

	return S_OK;
};

VOID cleanup(){
	if(g_pMeshMaterials != NULL)
		delete [] g_pMeshMaterials;

	if(g_pMeshTextures){
		for(DWORD i=0; i<g_dwNumMaterials; ++i){
			if(g_pMeshTextures[i])
				g_pMeshTextures[i]->Release();
		}
		delete [] g_pMeshTextures;
	}

	if(g_pMesh != NULL)
		g_pMesh->Release();

	if(g_pDevice != NULL)
		g_pDevice->Release();

	if(g_pD3D != NULL)
		g_pD3D->Release();
}

VOID render(){
	g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(192, 192, 192), 1.0f, 0);

	if(SUCCEEDED(g_pDevice->BeginScene())){
		setWorldMatrix();
		//Render the Mesh
		for(DWORD i=0; i<g_dwNumMaterials; ++i){
			g_pDevice->SetMaterial(&g_pMeshMaterials[i]);
			g_pDevice->SetTexture(0, g_pMeshTextures[i]);

			g_pMesh->DrawSubset(i);
		}//Render the Mesh
		g_pDevice->EndScene();
	}
	g_pDevice->Present(NULL, NULL, NULL, NULL);
}

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	switch(msg){
		case WM_DESTROY:
			cleanup();
			PostQuitMessage(0);
			return 0;
		case WM_KEYDOWN:
			g_bKey[wParam] = 1;
			return 0;
		case WM_KEYUP:
			g_bKey[wParam] = 0;
			return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT){
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, 
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		L"ClassName", NULL };
	RegisterClassEx( &wc );

	HWND hWnd = CreateWindow( L"ClassName", L"Control Mesh Use of Matrix", 
		WS_OVERLAPPEDWINDOW, 200, 100, 600, 480,
		GetDesktopWindow(), NULL, wc.hInstance, NULL );

	if( SUCCEEDED( init3D( hWnd ) ) ){ 
		//创建场景图形
		if( SUCCEEDED( initGeometry() ) ){
			//显示窗口
			ShowWindow( hWnd, SW_SHOWDEFAULT );
			UpdateWindow( hWnd );

			//进入消息循环
			MSG msg; 
			ZeroMemory( &msg, sizeof(msg) );
			while( msg.message!=WM_QUIT ){
				if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ){
					TranslateMessage( &msg );
					DispatchMessage( &msg );
				}else{
					render();  //渲染场景
				}
			}
		}
	}

	UnregisterClass( L"ClassName", wc.hInstance );
	return 0;
}