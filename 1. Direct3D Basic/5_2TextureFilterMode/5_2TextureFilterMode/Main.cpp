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

//VOID setupViewPort(){
//	RECT rect;
//	GetClientRect(g_hWnd, &rect);
//	D3DVIEWPORT9 vp;
//	vp.X = 0;
//	vp.Y = 0;
//	vp.Width = rect.right;
//	vp.Height = rect.bottom;
//	vp.MinZ = 0.0f;
//	vp.MaxZ = 1.0f;
//
//	g_pDevice->SetViewport(&vp);
//}

///Create Direct3D Device
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
	if(FAILED(D3DXCreateTextureFromFile(g_pDevice,L"texture.jpg", &g_pTexture)))
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

///Set light & material
VOID setLight(){
	//设置材料属性, 只反射红光分量和绿光分量, 整体上看该物体材料反射黄色光
	D3DMATERIAL9 mtrl;
	ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );

	mtrl.Ambient.r = 1.0f;
	mtrl.Ambient.g = 1.0f;
	mtrl.Ambient.b = 0.0f;
	mtrl.Ambient.a = 1.0f;

	mtrl.Diffuse.r = 1.0f;
	mtrl.Diffuse.g = 1.0f;
	mtrl.Diffuse.b = 0.0f;
	mtrl.Diffuse.a = 0.5f;

	mtrl.Specular.r = 1.0f;
	mtrl.Specular.g = 1.0f;
	mtrl.Specular.b = 0.0f;
	mtrl.Specular.a = 1.0f;

	mtrl.Emissive.r = 0.0f;
	mtrl.Emissive.g= 0.75f;
	mtrl.Emissive.b = 0.0f;
	mtrl.Emissive.a = 0.0f;

	g_pDevice->SetMaterial( &mtrl );
	g_pDevice->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);

	//设置一号光源为方向光，颜色为绿色
	D3DXVECTOR3 vecDir;
	D3DLIGHT9 light1;
	ZeroMemory( &light1, sizeof(D3DLIGHT9) );
	light1.Type       = D3DLIGHT_DIRECTIONAL;
	light1.Diffuse.r  = 0.0f;
	light1.Diffuse.g  = 1.0f;
	light1.Diffuse.b  = 0.0f;
	vecDir = D3DXVECTOR3(-10, 0 ,10); //方向光方向

	D3DXVec3Normalize( (D3DXVECTOR3*)&light1.Direction, &vecDir );
	g_pDevice->SetLight( 0, &light1 );

	//二号光源为点光源，颜色为红色
	D3DXVECTOR3  vecPos2;
	D3DLIGHT9 light2;
	ZeroMemory( &light2, sizeof(D3DLIGHT9) );
	light2.Type       = D3DLIGHT_POINT;  
	light2.Diffuse.r  = 1.0f;
	light2.Diffuse.g  = 0.0f;
	light2.Diffuse.b  = 0.0f;

	light2.Position= D3DXVECTOR3(10*sinf(timeGetTime()/350.0f) ,
		0,
		10*cosf(timeGetTime()/350.0f) );

	light2.Range        = 100.0f;
	light2.Attenuation0 = 1.0f;
	g_pDevice->SetLight( 1, &light2 );

	//三号光源为方向光,颜色为绿色, *镜面反射*
	D3DXVECTOR3 vecDir3;
	D3DLIGHT9 light3;
	ZeroMemory( &light3, sizeof(D3DLIGHT9) );
	light3.Type       = D3DLIGHT_DIRECTIONAL; 
	light3.Specular.r  = 0.0f;
	light3.Specular.g  = 1.0f;
	light3.Specular.b  = 0.0f;
	light3.Specular.a  = 1.0f;
	vecDir3 = D3DXVECTOR3(-10,0,10);

	D3DXVec3Normalize( (D3DXVECTOR3*)&light3.Direction, &vecDir3 );
	g_pDevice->SetLight( 2, &light3 );

	//设置一定的环境光
	g_pDevice->SetRenderState( D3DRS_AMBIENT, 0x00505050);
}

//HRESULT initGeometry(){
//	if(FAILED(g_pDevice->CreateVertexBuffer(50 * 2 * sizeof(CUSTOMVERTEX),
//		0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB, NULL)))
//		return E_FAIL;
//
//	CUSTOMVERTEX* pVertices;
//	if(FAILED(g_pVB->Lock(0,0,(void**)&pVertices,0)))
//		return E_FAIL;
//	for(DWORD i=0; i<50; ++i){
//		FLOAT theta = (2*D3DX_PI*i)/(50-1);
//		pVertices[2*i+0].position = D3DXVECTOR3( sinf(theta),-1.0f, cosf(theta) );
//		pVertices[2*i+0].normal   = D3DXVECTOR3( sinf(theta), 0.0f, cosf(theta) );
//		pVertices[2*i+1].position = D3DXVECTOR3( sinf(theta), 1.0f, cosf(theta) );
//		pVertices[2*i+1].normal   = D3DXVECTOR3( sinf(theta), 0.0f, cosf(theta) );
//	}
//	g_pVB->Unlock();
//
//	return S_OK;
//};

///Create Vertex Buffer
//HRESULT initVB(){
//	CUSTOMVERTEX vertices[] = {
//		{ 100.0f, 400.0f, 0.5f, 1.0f, 0xffff0000, },
//		{ 300.0f,  50.0f, 0.5f, 1.0f, 0xff00ff00, }, 
//		{ 500.0f, 400.0f, 0.5f, 1.0f, 0xff0000ff, },
//	};
//	//Vertex Buffer
//	if(FAILED(g_pDevice->CreateVertexBuffer(3 * sizeof(CUSTOMVERTEX),
//		0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB, NULL)))
//		return E_FAIL;
//
//	VOID* pVertices;
//	if(FAILED(g_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0)))
//		return E_FAIL;
//	memcpy(pVertices, vertices, sizeof(vertices));
//	g_pVB->Unlock();
//
//	return S_OK;
//}

VOID cleanup(){
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
		g_pDevice->SetTexture(0, g_pTexture);
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
			switch(wParam){
			case 49:	//最近点采样纹理过滤
				g_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
				g_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
				break;
			case 50:
				g_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
				g_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
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

	HWND hWnd = CreateWindow(L"ClassName", L"TextureFilterMode Demo",
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