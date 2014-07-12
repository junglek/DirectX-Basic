#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "winmm.lib")

LPDIRECT3D9					g_pD3D = NULL;
LPDIRECT3DDEVICE9		g_pDevice = NULL;

LPD3DXFONT					g_pFont = NULL;
WCHAR*							g_strText = L"Direct3D";
RECT									g_rcClientRect;
\


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

	if(FAILED(D3DXCreateFont(g_pDevice, 0,0,0,
		0,0,0,0,0,0,L"Arial", &g_pFont)))
		return E_FAIL;
	GetClientRect(hWnd, &g_rcClientRect);
	
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


VOID cleanup(){
	

	if(g_pFont != NULL)
		g_pFont->Release();

	if(g_pDevice != NULL)
		g_pDevice->Release();

	if(g_pD3D != NULL)
		g_pD3D->Release();
}

VOID render(){
	g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(192, 192, 192), 1.0f, 0);

	if(SUCCEEDED(g_pDevice->BeginScene())){

		g_pFont->DrawText(NULL, g_strText, (int)wcslen(g_strText), &g_rcClientRect, DT_SINGLELINE|DT_NOCLIP|DT_CENTER|DT_VCENTER,
			0xffffffff);

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

	HWND hWnd = CreateWindow( L"ClassName", L"Font", 
		WS_OVERLAPPEDWINDOW, 200, 100, 600, 480,
		GetDesktopWindow(), NULL, wc.hInstance, NULL );

	if( SUCCEEDED( init3D( hWnd ) ) ){ 
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

	UnregisterClass( L"ClassName", wc.hInstance );
	return 0;
}