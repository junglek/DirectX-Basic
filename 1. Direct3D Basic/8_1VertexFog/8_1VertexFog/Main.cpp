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
\

inline FLOAT heightField(FLOAT x, FLOAT z){
	FLOAT y = 0.0f;
	y += 10.0f * cosf( 0.051f*x + 0.0f ) * sinf( 0.055f*x + 0.0f );
	y += 10.0f * cosf( 0.053f*z + 0.0f ) * sinf( 0.057f*z + 0.0f );
	y += 2.0f * cosf( 0.101f*x + 0.0f ) * sinf( 0.105f*x + 0.0f );
	y += 2.0f * cosf( 0.103f*z + 0.0f ) * sinf( 0.107f*z + 0.0f );
	y += 2.0f * cosf( 0.251f*x + 0.0f ) * sinf( 0.255f*x + 0.0f );
	y += 2.0f * cosf( 0.253f*z + 0.0f ) * sinf( 0.257f*z + 0.0f );
	return y;
}

VOID setWorldMatrix(){
	D3DXMATRIX matWorld;
	D3DXMatrixIdentity(&matWorld);
	D3DXMatrixRotationY(&matWorld,  timeGetTime() / 1000.0f);
	g_pDevice->SetTransform(D3DTS_WORLD, &matWorld);
}

VOID setViewProjMatrix(){
	//View matrix
	D3DXVECTOR3 vEyePt(0.0f, 30.0f, -100.0f);
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
	D3DXMATRIX matView;
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	g_pDevice->SetTransform(D3DTS_VIEW, &matView);
	//Projection Matrix
	D3DXMATRIX matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 200.0f);
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


	g_pDevice->SetRenderState(D3DRS_AMBIENT, 0xffffffff);
	g_pDevice->SetRenderState(D3DRS_FOGCOLOR, 0xc0c0c0c0);
	
	g_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	g_pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
	g_pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	g_pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

	setViewProjMatrix();
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
	if(FAILED(D3DXLoadMeshFromX(L"seafloor.x", D3DXMESH_MANAGED,
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

	//Modify the mesh
	LPDIRECT3DVERTEXBUFFER9 pVB;
	if(SUCCEEDED(g_pMesh->GetVertexBuffer(&pVB))){
		struct VERTEX{FLOAT x,y,z,tu,tv;};
		VERTEX* pVertices;
		DWORD dwNumVertices = g_pMesh->GetNumVertices();

		pVB->Lock(0,0,(void**)&pVertices, 0);

		for(DWORD i=0; i<dwNumVertices; ++i)
			pVertices[i].y = heightField(pVertices[i].x, pVertices[i].z);

		pVB->Unlock();
		pVB->Release();
	}

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

		for(DWORD i=0; i<g_dwNumMaterials; ++i){

			g_pDevice->SetMaterial(&g_pMeshMaterials[i]);
			g_pDevice->SetTexture(0, g_pMeshTextures[i]);

			g_pMesh->DrawSubset(i);

		}

		g_pDevice->EndScene();
	}
	g_pDevice->Present(NULL, NULL, NULL, NULL);
}

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){

	static FLOAT fogStart = 50.0f;
	static FLOAT fogEnd  =200.0f;
	static FLOAT fogDensity = 0.01f;

	switch(msg){
		case WM_DESTROY:
			cleanup();
			PostQuitMessage(0);
			return 0;
		case WM_KEYDOWN:
			switch(wParam){
		case 48:
			g_pDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
			break;
		case 49:
			g_pDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
			g_pDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
			g_pDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD*)&fogStart);
			g_pDevice->SetRenderState(D3DRS_FOGEND, *(DWORD*)&fogEnd);
			break;
		case 50:
			g_pDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
			g_pDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_EXP);
			g_pDevice->SetRenderState(D3DRS_FOGDENSITY, *(DWORD*)&fogDensity);
			break;
		case 51:
			g_pDevice->SetRenderState(D3DRS_FOGENABLE, TRUE);
			g_pDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_EXP2);
			g_pDevice->SetRenderState(D3DRS_FOGDENSITY, *(DWORD*)&fogDensity);
			break;
			}
			return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT){
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, 
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		L"ClassName", NULL };
	RegisterClassEx( &wc );

	HWND hWnd = CreateWindow( L"ClassName", L"Vertex Fog", 
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