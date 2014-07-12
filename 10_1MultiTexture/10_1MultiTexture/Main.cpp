#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "winmm.lib")

LPDIRECT3D9					g_pD3D = NULL;
LPDIRECT3DDEVICE9		g_pDevice = NULL;
\
LPDIRECT3DVERTEXBUFFER9	g_pVB = NULL;
LPDIRECT3DTEXTURE9				g_pTexture1 = NULL;
LPDIRECT3DTEXTURE9				g_pTexture2 = NULL;

LPD3DXFONT							g_pFont = NULL;
\
struct CUSTOMVERTEX{
	FLOAT x,y,z;
	DWORD color;
	FLOAT u,v;
	FLOAT u1, v1;
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX2)
\
VOID setWorldMatrix(){
	D3DXMATRIX matWorld;
	D3DXMatrixIdentity(&matWorld);
	g_pDevice->SetTransform(D3DTS_WORLD, &matWorld);
}

VOID setViewProjMatrix(){
	//View matrix
	D3DXVECTOR3 vEyePt(0.0f, 0.0f, -8.0f);
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
	D3DXMATRIX matView;
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	g_pDevice->SetTransform(D3DTS_VIEW, &matView);
	//Projection Matrix
	D3DXMATRIX matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);
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

	//设置第0层纹理、纹理阶段状态和纹理坐标索引
	//g_pDevice->SetTexture( 0, g_pTexture1 );
	g_pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	g_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	g_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	g_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
	g_pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX,   0 );
	g_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	g_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

	//设置第1层纹理、纹理阶段状态和纹理坐标索引
	//g_pDevice->SetTexture( 1, g_pTexture2 );
	g_pDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_ADD);
	g_pDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	g_pDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	g_pDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
	g_pDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX,   1 );
	g_pDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	g_pDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

	g_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

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
	//Create Font
	if(FAILED(D3DXCreateFont(g_pDevice, 15,0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		L"Arial", &g_pFont)))
		return E_FAIL;
	//Create Texture 1 & 2
	if(FAILED(D3DXCreateTextureFromFile(g_pDevice, L"Wall.bmp", &g_pTexture1)))
		return E_FAIL;
	if(FAILED(D3DXCreateTextureFromFile(g_pDevice, L"light.jpg", &g_pTexture2)))
		return E_FAIL;
	//Create Vertex buffer & Fill Data
	CUSTOMVERTEX verts[] = {
		{ -3.0f, -3.0f,  0.0f,  0xffffffff, 0.0f, 1.0f, 0.0f, 1.0f},
		{ -3.0f,  3.0f,  0.0f,  0xffffffff, 0.0f, 0.0f, 0.0f, 0.0f},
		{  3.0f, -3.0f,  0.0f,  0xffffffff, 1.0f, 1.0f, 1.0f, 1.0f},
		{  3.0f,  3.0f,  0.0f,  0xffffffff, 1.0f, 0.0f, 1.0f, 0.0f}
	};
	if(FAILED(g_pDevice->CreateVertexBuffer(4 * sizeof(CUSTOMVERTEX),
		0, D3DFVF_CUSTOMVERTEX, D3DPOOL_MANAGED, &g_pVB,0)))
		return E_FAIL;
	VOID* pVerts;
	if(FAILED(g_pVB->Lock(0, sizeof(verts), (void**)&pVerts, 0)))
		return E_FAIL;
	memcpy(pVerts, verts, sizeof(verts));
	g_pVB->Unlock();

	return S_OK;
};

VOID cleanup(){
	if(g_pTexture1 != NULL)
		g_pTexture1->Release();

	if(g_pTexture2 != NULL)
		g_pTexture2->Release();

	if(g_pFont != NULL)
		g_pFont->Release();


	if(g_pVB != NULL)
		g_pVB->Release();

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
		
		g_pDevice->SetTexture(0, g_pTexture1);
		g_pDevice->SetTexture(1, g_pTexture2);
		g_pDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
		g_pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
		g_pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0,2);
		
		RECT drawRect = {10,10, 90,10};
		g_pFont->DrawText(NULL, L"Multi-Texture", -1, &drawRect,  DT_SINGLELINE|DT_NOCLIP|DT_CENTER|DT_VCENTER, 0xffffffff);

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
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT){
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, 
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		L"ClassName", NULL };
	RegisterClassEx( &wc );

	HWND hWnd = CreateWindow( L"ClassName", L"Multi-Texture", 
		WS_OVERLAPPEDWINDOW, 200, 100, 640, 480,
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