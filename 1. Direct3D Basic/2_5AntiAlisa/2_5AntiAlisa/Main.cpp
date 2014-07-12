#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

LPDIRECT3D9							g_pD3D = NULL;
LPDIRECT3DDEVICE9				g_pDevice = NULL;
LPDIRECT3DVERTEXBUFFER9	g_pVB = NULL;
bool											g_bAntiAlisa = true;


struct CUSTOMVERTEX{
	FLOAT x,y,z,rhw;
	DWORD color;
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)
///Create Direct3D Device
HRESULT initD3D(HWND hWnd){

	HRESULT hr = S_OK;

	if(NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;
	if(FAILED(g_pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, FALSE, D3DMULTISAMPLE_4_SAMPLES,
		NULL))){
			MessageBox(hWnd, L"Hardware do not support multi-sample reference device", L"Warning", 0);
			hr = g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd,
				D3DCREATE_SOFTWARE_VERTEXPROCESSING,
				&d3dpp, &g_pDevice );
	}else{
		hr = g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&d3dpp, &g_pDevice );
	}

	return hr;
}
///Create Vertex Buffer
HRESULT initVB(){
	CUSTOMVERTEX vertices[] = {
		{ 100.0f, 400.0f, 0.5f, 1.0f, 0xffff0000, },
		{ 300.0f,  50.0f, 0.5f, 1.0f, 0xff00ff00, }, 
		{ 500.0f, 400.0f, 0.5f, 1.0f, 0xff0000ff, },
	};
	//Vertex Buffer
	if(FAILED(g_pDevice->CreateVertexBuffer(3 * sizeof(CUSTOMVERTEX),
		0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB, NULL)))
		return E_FAIL;

	VOID* pVertices;
	if(FAILED(g_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0)))
		return E_FAIL;
	memcpy(pVertices, vertices, sizeof(vertices));
	g_pVB->Unlock();

	return S_OK;
}

VOID cleanup(){
	if(g_pVB != NULL)
		g_pVB->Release();
	if(g_pDevice != NULL)
		g_pDevice->Release();
	if(g_pD3D != NULL)
		g_pD3D->Release();
}

VOID render(){
	if(g_bAntiAlisa)
		g_pDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	else	g_pDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);

	g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(192,192,192), 1.0f, 0);
	if(SUCCEEDED(g_pDevice->BeginScene())){
		//Render to the back-buffer
		g_pDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
		g_pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
		g_pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0,1);

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
		case WM_LBUTTONDOWN:
			g_bAntiAlisa = !g_bAntiAlisa;
			return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT){
	WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L,0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		L"ClassName", NULL};
	RegisterClassEx(&wc);

	HWND hWnd = CreateWindow(L"ClassName", L"Anti Alisa Demo",
		WS_OVERLAPPEDWINDOW, 200,100,600,500,NULL, NULL,
		wc.hInstance, NULL);

	if(SUCCEEDED(initD3D(hWnd))){
		if(SUCCEEDED(initVB())){
			ShowWindow(hWnd, SW_SHOWMAXIMIZED);//Maximized the window
			UpdateWindow(hWnd);

			MSG msg;
			ZeroMemory(&msg, sizeof(MSG));
			while( msg.message!=WM_QUIT )
			{
				if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) ){
					TranslateMessage( &msg );
					DispatchMessage( &msg );
				}else{
					render();  //‰÷»æÕº–Œ
				}
			}
		}
	}
	UnregisterClass(L"ClassName", wc.hInstance);
	return 0;
}