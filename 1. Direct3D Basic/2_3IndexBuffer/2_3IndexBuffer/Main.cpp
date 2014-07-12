#include <d3d9.h>
#include <math.h>
#pragma comment(lib, "d3d9.lib")

LPDIRECT3D9							g_pD3D = NULL;
LPDIRECT3DDEVICE9				g_pDevice = NULL;
LPDIRECT3DVERTEXBUFFER9	g_pVB = NULL;
LPDIRECT3DINDEXBUFFER9		g_pIB = NULL;

struct CUSTOMVERTEX{
	FLOAT x,y,z,rhw;
	DWORD color;
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)
///Create Direct3D Device
HRESULT initD3D(HWND hWnd){
	if(NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	if(FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, &g_pDevice)))
		return E_FAIL;

	return S_OK;
}
///Create Vertex Buffer
HRESULT initVB(){
	CUSTOMVERTEX vertices[9];
	vertices[0].x = 300;
	vertices[0].y = 250;
	vertices[0].z = 0.5f;
	vertices[0].rhw = 1.0f;
	vertices[0].color = 0xffff0000;
	for(int i=0; i<8; ++i){
		vertices[i+1].x = (float)(200*sin(i*3.14159/4.0)) + 300;
		vertices[i+1].y = -(float)(200*cos(i*3.14159/4.0)) + 250;
		vertices[i+1].z = 0.5f;
		vertices[i+1].rhw = 1.0f;
		vertices[i+1].color = 0xff00ff00;
	}
	WORD indices[] = { 0,1,2, 0,2,3, 0,3,4, 0,4,5, 0,5,6, 0,6,7, 0,7,8, 0,8,1 };
	//Vertex Buffer
	if(FAILED(g_pDevice->CreateVertexBuffer(9 * sizeof(CUSTOMVERTEX),
		0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB, NULL)))
		return E_FAIL;

	VOID* pVertices;
	if(FAILED(g_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0)))
		return E_FAIL;
	memcpy(pVertices, vertices, sizeof(vertices));
	g_pVB->Unlock();
	///Index Buffer
	if(FAILED(g_pDevice->CreateIndexBuffer(24* sizeof(WORD), 0, D3DFMT_INDEX16,
		D3DPOOL_DEFAULT, &g_pIB, NULL)))
		return E_FAIL;

	VOID* pIndices;
	if(FAILED(g_pIB->Lock(0, sizeof(indices), (void**)&pIndices, 0)))
		return E_FAIL;
	memcpy(pIndices, indices, sizeof(indices));
	g_pIB->Unlock();


	return S_OK;
}

VOID cleanup(){
	if(g_pVB != NULL)
		g_pVB->Release();
	if(g_pIB != NULL)
		g_pIB->Release();
	if(g_pDevice != NULL)
		g_pDevice->Release();
	if(g_pD3D != NULL)
		g_pD3D->Release();
}

VOID render(){
	g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(192,192,192), 1.0f, 0);
	if(SUCCEEDED(g_pDevice->BeginScene())){
		//Render to the back-buffer
		g_pDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
		g_pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
		g_pDevice->SetIndices(g_pIB);
		g_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0,0,9,0,8);

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

	HWND hWnd = CreateWindow(L"ClassName", L"Index Buffer Demo",
		WS_OVERLAPPEDWINDOW, 200,100,600,500,NULL, NULL,
		wc.hInstance, NULL);

	if(SUCCEEDED(initD3D(hWnd))){
		if(SUCCEEDED(initVB())){
			ShowWindow(hWnd, SW_SHOWDEFAULT);
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