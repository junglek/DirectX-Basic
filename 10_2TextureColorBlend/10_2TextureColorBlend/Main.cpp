#include <d3dx9.h>
#include <stdio.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "winmm.lib")

LPDIRECT3D9					g_pD3D = NULL;
LPDIRECT3DDEVICE9		g_pDevice = NULL;
\
LPDIRECT3DVERTEXBUFFER9	g_pVB = NULL;
LPDIRECT3DTEXTURE9				g_pBaseTexture = NULL;
LPDIRECT3DTEXTURE9				g_pDarkTexture = NULL;
LPDIRECT3DTEXTURE9				g_pDetailTexture = NULL;

D3DLIGHT9								g_pLight;
D3DMATERIAL9						g_pMtrl;

int											g_iBlendFlag = 1;

LPD3DXFONT							g_pFont = NULL;
LPD3DXSPRITE							g_pTexSprite = NULL;
WCHAR*									g_strText = NULL;

FLOAT g_Test = 0.0f;

\
struct CUSTOMVERTEX{
	FLOAT x,y,z;
	FLOAT u,v;
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_TEX1)
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

	g_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	g_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

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
	if(FAILED(D3DXCreateTextureFromFile(g_pDevice, L"Wall.bmp", &g_pBaseTexture)))
		return E_FAIL;
	if(FAILED(D3DXCreateTextureFromFile(g_pDevice, L"dark.bmp", &g_pDarkTexture)))
		return E_FAIL;
	if(FAILED(D3DXCreateTextureFromFile(g_pDevice, L"detail.bmp", &g_pDetailTexture)))
		return E_FAIL;
	//Create Vertex buffer & Fill Data
	CUSTOMVERTEX verts[] = {
		{ -3, -3, 0.0f,  0, 1 },   
		{ -3,  3, 0.0f,  0, 0 },	
		{  3, -3, 0.0f,  1, 1 },	
		{  3,  3, 0.0f,  1, 0 }
	};
	if(FAILED(g_pDevice->CreateVertexBuffer(4 * sizeof(CUSTOMVERTEX),
		0, D3DFVF_CUSTOMVERTEX, D3DPOOL_MANAGED, &g_pVB,0)))
		return E_FAIL;
	VOID* pVerts;
	if(FAILED(g_pVB->Lock(0, sizeof(verts), (void**)&pVerts, 0)))
		return E_FAIL;
	memcpy(pVerts, verts, sizeof(verts));
	g_pVB->Unlock();
	
	//Material
	ZeroMemory(&g_pMtrl, sizeof(D3DMATERIAL9));
	g_pMtrl.Ambient.r = 1.0f;
	g_pMtrl.Ambient.g = 1.0f;
	g_pMtrl.Ambient.b = 1.0f;
	g_pMtrl.Ambient.a = 1.0f;
	g_pMtrl.Diffuse.r = 0.7f;
	g_pMtrl.Diffuse.g = 0.7f;
	g_pMtrl.Diffuse.b = 0.7f;
	g_pMtrl.Diffuse.a = 0.5f;
	//Light
	D3DXVECTOR3 vecDir;
	ZeroMemory( &g_pLight, sizeof(D3DLIGHT9) );
	g_pLight.Type       = D3DLIGHT_DIRECTIONAL;
	g_pLight.Diffuse.r  = 0.5f;
	g_pLight.Diffuse.g  = 0.5f;
	g_pLight.Diffuse.b  = 0.5f;
	vecDir = D3DXVECTOR3(0, 0, 10);    //方向光方向
	D3DXVec3Normalize( (D3DXVECTOR3*)&g_pLight.Direction, &vecDir );

	return S_OK;
};

VOID cleanup(){
	if(g_pBaseTexture != NULL)
		g_pBaseTexture->Release();

	if(g_pDarkTexture != NULL)
		g_pDarkTexture->Release();

	if(g_pDetailTexture != NULL)
		g_pDetailTexture->Release();


	if(g_pFont != NULL)
		g_pFont->Release();


	if(g_pVB != NULL)
		g_pVB->Release();

	if(g_pDevice != NULL)
		g_pDevice->Release();

	if(g_pD3D != NULL)
		g_pD3D->Release();
}

VOID update(FLOAT delta, FLOAT fTime){
	HRESULT hr = S_OK;
	//Dark Mapping
	if(g_iBlendFlag == 1){

		g_strText = L"Dark Mapping";

		g_pDevice->SetTexture( 0, g_pBaseTexture );
		g_pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
		g_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		g_pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );

		g_pDevice->SetTexture( 1, g_pDarkTexture );
		g_pDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
		g_pDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		g_pDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
		g_pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE );
	}else if(g_iBlendFlag == 2){	//Dark Animation
		g_strText = L"Dark Animation";

		g_pDevice->SetTexture( 0, g_pBaseTexture); 
		g_pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
		g_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		g_pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1); 

		g_pDevice->SetTexture(1, g_pDarkTexture); 
		g_pDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0 );
		g_pDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		g_pDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT );  

		static double fLastTime = 0.0f;
		static double interval = 0.0f;
		interval = fTime - fLastTime;
		
		g_Test = interval;		///Debug
		
		if(interval<0.5f)
		{
			g_pDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
		}
		else if (interval>0.5f && interval<1.0f)
		{
			g_pDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
		}
		else if (interval>1.0f && interval<1.5f)
		{
			g_pDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE4X);
		}
		else if (interval > 1.5f)
		{
			fLastTime = fTime;
		}
	}else if(g_iBlendFlag == 3){	//Blend texture & material diffuse
		g_strText = L"Blend Texture & Material's Diffuse Color";

		g_pDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
		g_pDevice->SetRenderState( D3DRS_AMBIENT, 0x00808080 );
		g_pDevice->SetLight( 0, &g_pLight );
		g_pDevice->LightEnable( 0, TRUE );
		g_pDevice->SetMaterial( &g_pMtrl );

		g_pDevice->SetTexture( 0, g_pBaseTexture ); 
		g_pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
		g_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		g_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		g_pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_ADD );
		g_pDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	}else if(g_iBlendFlag == 4){	//Blend Dark texture & material diffuse
		g_strText = L"Blend Dark Texture & Material's Diffuse Color";

		g_pDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
		g_pDevice->SetRenderState( D3DRS_AMBIENT, 0x00808080 );
		g_pDevice->SetLight( 0, &g_pLight );
		g_pDevice->LightEnable( 0, TRUE );
		g_pDevice->SetMaterial( &g_pMtrl );

		g_pDevice->SetTexture( 0, g_pBaseTexture); 
		g_pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
		g_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ); 
		g_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE); 
		g_pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);

		g_pDevice->SetTexture(1, g_pDarkTexture); 
		g_pDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0 );
		g_pDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE); 
		g_pDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
		g_pDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	}else if(g_iBlendFlag == 5){	//glow mapping
		g_strText = L"Glow Mapping";

		g_pDevice->SetTexture( 0, g_pBaseTexture );
		g_pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
		g_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		g_pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );

		g_pDevice->SetTexture( 1, g_pDarkTexture );
		g_pDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
		g_pDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		g_pDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
		g_pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADD );
	}else if(g_iBlendFlag == 6){	//Detail Mapping
		g_strText = L"Detail Mapping";

		g_pDevice->SetTexture( 0, g_pBaseTexture );
		g_pDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
		g_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		g_pDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );

		g_pDevice->SetTexture( 1, g_pDetailTexture );
		g_pDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
		g_pDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		g_pDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
		hr = g_pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADDSIGNED );
		if( FAILED( hr ) )
			g_pDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE2X );
	}
}

VOID render(){
	g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(192, 192, 192), 1.0f, 0);

	if(SUCCEEDED(g_pDevice->BeginScene())){

		setWorldMatrix();

		g_pDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
		g_pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
		g_pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0,2);

		RECT drawRect = {20,10, 150,10};
		g_pFont->DrawText(NULL, g_strText, -1, &drawRect,  DT_SINGLELINE|DT_NOCLIP|DT_LEFT|DT_VCENTER, 0xffffffff);


		char szText[512] = {0};
		sprintf(szText, "interval: %.2f", g_Test);
		RECT rc = {280,10, 360,10};
		WCHAR* inter = new WCHAR[256];
		mbstowcs(inter, szText, sizeof(szText));
		g_pFont->DrawText(NULL, inter, -1, &rc,  DT_SINGLELINE|DT_NOCLIP|DT_LEFT|DT_VCENTER, 0xffffffff);

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
			switch(wParam){
		case 49:
			g_iBlendFlag = 1;
			break;
		case 50:
			g_iBlendFlag = 2;
			break;
		case 51:
			g_iBlendFlag = 3;
			break;
		case 52:
			g_iBlendFlag = 4;
			break;
		case 53:
			g_iBlendFlag = 5;
			break;
		case 54:
			g_iBlendFlag = 6;
			break;
			}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT){
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, 
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		L"ClassName", NULL };
	RegisterClassEx( &wc );

	HWND hWnd = CreateWindow( L"ClassName", L"TextureColorBlend", 
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
			DWORD strartTime = timeGetTime();
			ZeroMemory( &msg, sizeof(msg) );
			while( msg.message!=WM_QUIT ){
				if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ){
					TranslateMessage( &msg );
					DispatchMessage( &msg );
				}else{
					DWORD t = timeGetTime();
					FLOAT deltaTime = (t - strartTime) * 0.001f;

					update(deltaTime, t / 1000.0f);
					render();  //渲染场景

					strartTime = t;
				}
			}
		}
	}

	UnregisterClass( L"ClassName", wc.hInstance );
	return 0;
}