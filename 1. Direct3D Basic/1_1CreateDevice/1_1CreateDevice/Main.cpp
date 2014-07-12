#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pDevice = NULL;

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

VOID cleanup(){
	if(g_pDevice != NULL)
		g_pDevice->Release();
	if(g_pD3D != NULL)
		g_pD3D->Release();
}

VOID render(){
	g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(192,192,192), 1.0f, 0);
	if(SUCCEEDED(g_pDevice->BeginScene())){
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
		case WM_PAINT:
			render();
			ValidateRect(hWnd, NULL);
			return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT){
	WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L,0L,
	GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
	L"ClassName", NULL};
	RegisterClassEx(&wc);

	HWND hWnd = CreateWindow(L"ClassName", L"Direct3D",
		WS_OVERLAPPEDWINDOW, 200,100,600,500,NULL, NULL,
		wc.hInstance, NULL);

	if(SUCCEEDED(initD3D(hWnd))){
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
	UnregisterClass(L"ClassName", wc.hInstance);
	return 0;
}