#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "winmm.lib")

LPDIRECT3D9					g_pD3D = NULL;
LPDIRECT3DDEVICE9		g_pDevice = NULL;
LPDIRECT3DVERTEXBUFFER9	g_pVB = NULL;
\
\
struct CUSTOMVERTEX{
	FLOAT x, y, z, rhw;
	DWORD color;
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)


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


	g_pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	g_pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	g_pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

\
	return S_OK;
}

HRESULT initVB(){
	CUSTOMVERTEX vertices[] ={
		{ 100.0f, 400.0f, 0.5f, 1.0f, 0x50ff8000, },	//a-r-g-b
		{ 300.0f,  50.0f, 0.5f, 1.0f, 0x50ff0060, }, 
		{ 500.0f, 400.0f, 0.5f, 1.0f, 0x50400000, },
	};

	if(FAILED(g_pDevice->CreateVertexBuffer(3 * sizeof(CUSTOMVERTEX), 0,
		D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB, 0)))
		return E_FAIL;
	//Fill the data
	VOID* pVertices;
	if(FAILED(g_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0)))
		return E_FAIL;
	memcpy(pVertices, vertices, sizeof(vertices));
	g_pVB->Unlock();

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
		g_pDevice->SetStreamSource(0, g_pVB, 0,sizeof(CUSTOMVERTEX));
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
		case WM_KEYDOWN:
			switch(wParam){
		case 48:
			g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			break;
		case 49:
			g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			break;
			}
			return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT){
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, 
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		L"ClassName", NULL };
	RegisterClassEx( &wc );

	HWND hWnd = CreateWindow( L"ClassName", L"Vertex Alpha Blend", 
		WS_OVERLAPPEDWINDOW, 200, 100, 600, 480,
		GetDesktopWindow(), NULL, wc.hInstance, NULL );

	if( SUCCEEDED( init3D( hWnd ) ) ){ 
		//创建场景图形
		if( SUCCEEDED( initVB() ) ){
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