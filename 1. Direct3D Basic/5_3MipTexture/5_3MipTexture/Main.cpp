#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "winmm.lib")

LPDIRECT3D9							g_pD3D = NULL;
LPDIRECT3DDEVICE9				g_pDevice = NULL;
LPDIRECT3DVERTEXBUFFER9	g_pVB = NULL;
LPDIRECT3DTEXTURE9				g_pMipTexture = NULL;
LPDIRECT3DTEXTURE9				g_pTexture = NULL;
float											g_fDisntanceToCamera = -10.0f;
bool											g_bUseMipTexture = true;


struct CUSTOMVERTEX{
	D3DXVECTOR3 position;
	D3DXVECTOR2 tex;
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_TEX1)

VOID setupWorldMatrix(){
	D3DXMATRIX matWorld;
	/*UINT iTime = timeGetTime() % 2500;
	FLOAT fAngle = iTime * (2.0f * D3DX_PI) / 2500.0f;
	D3DXMatrixIdentity(&matWorld);
	D3DXMatrixRotationX(&matWorld, fAngle);*/

	D3DXMatrixIdentity(&matWorld);
	g_pDevice->SetTransform(D3DTS_WORLD, &matWorld);
}

VOID setupViewProjMatrix(){
	//View matrix
	D3DXVECTOR3 vEyePos(0.0f, 0.0f, g_fDisntanceToCamera);
	D3DXVECTOR3 vLookatPos(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
	D3DXMATRIX matView;
	D3DXMatrixLookAtLH(&matView, &vEyePos, &vLookatPos, &vUpVec);

	g_pDevice->SetTransform(D3DTS_VIEW, &matView);

	//Projection matrix
	D3DXMATRIX matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);
	g_pDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}

HRESULT initD3D(HWND hWnd){

	if(NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&d3dpp, &g_pDevice ) ) ){
			return E_FAIL;
	}

	g_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	setupViewProjMatrix();
	return S_OK;
}

HRESULT initGraphics(){
	///Mip texture
	if(FAILED(D3DXCreateTextureFromFileEx(g_pDevice,
		L"texture.jpg",0,0,0,0,
		D3DFMT_X8B8G8R8, D3DPOOL_MANAGED,
		D3DX_DEFAULT, D3DX_DEFAULT, 0xff000000, 0,0,
		&g_pMipTexture)))
		return E_FAIL;

	if(FAILED(D3DXCreateTextureFromFileEx(g_pDevice,
		L"texture.jpg",0,0,1,0,
		D3DFMT_X8B8G8R8, D3DPOOL_MANAGED,
		D3DX_DEFAULT, D3DX_DEFAULT, 0xff000000, 0,0,
		&g_pTexture)))
		return E_FAIL;


	CUSTOMVERTEX vertices[] = {
		{ D3DXVECTOR3(-3,   -3,  0.0f),  D3DXVECTOR2(0.0f, 1.0f)},   
		{ D3DXVECTOR3(-3,    3,  0.0f),  D3DXVECTOR2(0.0f, 0.0f)},	
		{ D3DXVECTOR3(3,   -3,  0.0f),  D3DXVECTOR2(1.0f, 1.0f)},	
		{ D3DXVECTOR3(3,    3,  0.0f),  D3DXVECTOR2(1.0f, 0.0f) }
	};

	if( FAILED( g_pDevice->CreateVertexBuffer( 4*sizeof(CUSTOMVERTEX),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_MANAGED, &g_pVB,NULL ) ) ){
			return E_FAIL;
	}

	//填充顶点缓冲区
	VOID* pVertices;
	if( FAILED( g_pVB->Lock( 0, sizeof(vertices), (void**)&pVertices, 0 ) ) )
		return E_FAIL;
	memcpy( pVertices, vertices, sizeof(vertices) );
	g_pVB->Unlock();

	return S_OK;
}


VOID cleanup(){
	if(g_pMipTexture != NULL)
		g_pMipTexture->Release();
	if(g_pTexture!= NULL)
		g_pTexture->Release();
	if(g_pVB != NULL)
		g_pVB->Release();
	if(g_pDevice != NULL)
		g_pDevice->Release();
	if(g_pD3D != NULL)
		g_pD3D->Release();
}

VOID render(){

	g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET| D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(192,192,192), 1.0f, 0);
	if(SUCCEEDED(g_pDevice->BeginScene())){
		setupWorldMatrix();
		setupViewProjMatrix();
		
		if(g_bUseMipTexture)
			g_pDevice->SetTexture(0, g_pMipTexture);
		else	g_pDevice->SetTexture(0, g_pTexture);
		
		//Render to the back-buffer
		g_pDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
		g_pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
		g_pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

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
			switch(wParam)
			{
			case 49:    //“1”键, 使用多级渐进纹理
				g_bUseMipTexture = true;
				break;

			case 50:    //“2”键, 不使用多级渐进纹理
				g_bUseMipTexture = false;
				break;

			case VK_DOWN:
				g_fDisntanceToCamera -= 0.1f;
				break;

			case VK_UP:
				g_fDisntanceToCamera += 0.1f;
				break;
			}
			return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT){
	WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L,0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		L"ClassName", NULL};
	RegisterClassEx(&wc);

	HWND hWnd = CreateWindow(L"ClassName", L"MipTexture Demo",
		WS_OVERLAPPEDWINDOW, 200,100,600,500,NULL, NULL,
		wc.hInstance, NULL);

	if(SUCCEEDED(initD3D(hWnd))){
		if(SUCCEEDED(initGraphics())){
			ShowWindow(hWnd, SW_SHOWDEFAULT);//Maximized the window
			UpdateWindow(hWnd);

			MSG msg;
			ZeroMemory(&msg, sizeof(MSG));
			while( msg.message!=WM_QUIT )
			{
				if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) ){
					TranslateMessage( &msg );
					DispatchMessage( &msg );
				}else{
					render();  //渲染图形
				}
			}
		}
	}
	UnregisterClass(L"ClassName", wc.hInstance);
	return 0;
}