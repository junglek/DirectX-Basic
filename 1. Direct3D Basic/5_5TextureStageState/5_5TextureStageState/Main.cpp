#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "winmm.lib")

LPDIRECT3D9							g_pD3D = NULL;
LPDIRECT3DDEVICE9				g_pDevice = NULL;
LPDIRECT3DVERTEXBUFFER9	g_pVB = NULL;
LPDIRECT3DTEXTURE9				g_pTexture = NULL;



struct CUSTOMVERTEX{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 tex;
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_NORMAL |D3DFVF_TEX1)

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
	D3DXVECTOR3 vEyePos(0.0f, 3.0f, -5.0f);
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

	g_pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE ); //剔除模式设置
	g_pDevice->SetRenderState( D3DRS_ZENABLE, TRUE );          //启用深度测试
	g_pDevice->SetRenderState(D3DRS_SPECULARENABLE, TRUE);     //启用镜面反射光照模型

	//设置纹理渲染状态
	g_pDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );  //默认设置
	g_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	g_pDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

	g_pDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

	//设置纹理过滤方式
	g_pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	g_pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

	setupViewProjMatrix();
	return S_OK;
}

VOID setLight(){
	//Set material
	D3DMATERIAL9 mtrl;
	ZeroMemory(&mtrl, sizeof(D3DMATERIAL9));
	mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
	mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
	mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
	mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
	g_pDevice->SetMaterial(&mtrl);

	//Set light
	D3DXVECTOR3 vecDir;
	D3DLIGHT9 light;
	ZeroMemory( &light, sizeof(D3DLIGHT9) );
	light.Type       = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r  = 1.0f;
	light.Diffuse.g  = 1.0f;
	light.Diffuse.b  = 1.0f;
	vecDir = D3DXVECTOR3(cosf(timeGetTime()/350.0f),
		1.0f,
		sinf(timeGetTime()/350.0f) );
	D3DXVec3Normalize( (D3DXVECTOR3*)&light.Direction, &vecDir );
	light.Range       = 1000.0f;
	g_pDevice->SetLight( 0, &light );
	g_pDevice->LightEnable( 0, TRUE );
	g_pDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
	//Set Ambient
	g_pDevice->SetRenderState(D3DRS_AMBIENT, 0x00808080);
}

HRESULT initGeometry(){
	if( FAILED( D3DXCreateTextureFromFile( g_pDevice, L"texture.jpg", &g_pTexture ) ) )
	{
		MessageBox(NULL, L"创建纹理失败", L"Texture.exe", MB_OK);
		return E_FAIL;
	}

	//创建顶点缓冲区
	if( FAILED( g_pDevice->CreateVertexBuffer( 50*2*sizeof(CUSTOMVERTEX),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &g_pVB, NULL ) ) )
	{
		return E_FAIL;
	}

	//填充顶点缓冲区
	CUSTOMVERTEX* pVertices;
	if( FAILED( g_pVB->Lock( 0, 0, (void**)&pVertices, 0 ) ) )
		return E_FAIL;
	for( DWORD i=0; i<50; i++ )
	{
		FLOAT theta = (2*D3DX_PI*i)/(50-1);
		pVertices[2*i+0].position = D3DXVECTOR3( sinf(theta),-1.0f, cosf(theta) );
		pVertices[2*i+0].normal   = D3DXVECTOR3( sinf(theta), 0.0f, cosf(theta) );
		pVertices[2*i+0].tex.x       = ((FLOAT)i)/(50-1);
		pVertices[2*i+0].tex.y       = 1.0f;

		pVertices[2*i+1].position = D3DXVECTOR3( sinf(theta), 1.0f, cosf(theta) );
		pVertices[2*i+1].normal   = D3DXVECTOR3( sinf(theta), 0.0f, cosf(theta) );
		pVertices[2*i+1].tex.x       = ((FLOAT)i)/(50-1);
		pVertices[2*i+1].tex.y       = 0.0f;
	}
	g_pVB->Unlock();

	return S_OK;
}

VOID cleanup(){
	if(g_pTexture != NULL)
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
		setLight();
		g_pDevice->SetTexture(0, g_pTexture);
		//Render to the back-buffer
		g_pDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
		g_pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
		g_pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2*50-2);

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
	WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L,0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		L"ClassName", NULL};
	RegisterClassEx(&wc);

	HWND hWnd = CreateWindow(L"ClassName", L"TextureStageState Demo",
		WS_OVERLAPPEDWINDOW, 200,100,600,500,NULL, NULL,
		wc.hInstance, NULL);

	if(SUCCEEDED(initD3D(hWnd))){
		if(SUCCEEDED(initGeometry())){
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