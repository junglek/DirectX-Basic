#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "winmm.lib")

LPDIRECT3D9							g_pD3D = NULL;
LPDIRECT3DDEVICE9				g_pDevice = NULL;
LPDIRECT3DVERTEXBUFFER9	g_pVB = NULL;
LPDIRECT3DTEXTURE9				g_pTexture1 = NULL;
LPDIRECT3DTEXTURE9				g_pTexture2 = NULL;
int											g_iTextureAddressMode = 1;


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
	D3DXVECTOR3 vEyePos(0.0f, 0.0f, -10.0f);
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

	//texture render state
	g_pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	g_pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	g_pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	//texture filter mode
	g_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	
	setupViewProjMatrix();
	return S_OK;
}

HRESULT initGraphics(){
	if(FAILED(D3DXCreateTextureFromFile(g_pDevice, L"texture1.bmp", &g_pTexture1)))
		return E_FAIL;
	if(FAILED(D3DXCreateTextureFromFile(g_pDevice, L"texture2.bmp", &g_pTexture2)))
		return E_FAIL;

	CUSTOMVERTEX vertices[] = {
		{ D3DXVECTOR3(-3,   -3,  0.0f),  D3DXVECTOR2(0.0f, 3.0f)},   
		{ D3DXVECTOR3(-3,    3,  0.0f),  D3DXVECTOR2(0.0f, 0.0f)},	
		{ D3DXVECTOR3(3,   -3,  0.0f),  D3DXVECTOR2(3.0f, 3.0f)},	
		{ D3DXVECTOR3(3,    3,  0.0f),  D3DXVECTOR2(3.0f, 0.0f) }
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
	if(g_pTexture1 != NULL)
		g_pTexture1->Release();
	if(g_pTexture2 != NULL)
		g_pTexture2->Release();
	if(g_pVB != NULL)
		g_pVB->Release();
	if(g_pDevice != NULL)
		g_pDevice->Release();
	if(g_pD3D != NULL)
		g_pD3D->Release();
}

VOID setTexture(){
	switch(g_iTextureAddressMode){
		case 1:
			g_pDevice->SetTexture(0, g_pTexture1);

			g_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
			g_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
			break;

		case 2:
			g_pDevice->SetTexture(0, g_pTexture1);

			g_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
			g_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
			break;

		case 3:
			g_pDevice->SetTexture(0, g_pTexture2);

			g_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			g_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
			break;

		case 4:
			g_pDevice->SetTexture(0, g_pTexture2);
			
			g_pDevice->SetSamplerState(0, D3DSAMP_BORDERCOLOR, 0xffff0000);
			g_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
			g_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
			break;
	}
};

VOID render(){

	g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET| D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(192,192,192), 1.0f, 0);
	if(SUCCEEDED(g_pDevice->BeginScene())){
		setupWorldMatrix();
		setTexture();
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
				g_iTextureAddressMode++;
				if(g_iTextureAddressMode > 4)
					g_iTextureAddressMode = 1;
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

	HWND hWnd = CreateWindow(L"ClassName", L"TextureAddressMode Demo",
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