#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

LPDIRECT3D9							g_pD3D = NULL;
LPDIRECT3DDEVICE9				g_pDevice = NULL;
LPDIRECT3DVERTEXBUFFER9	g_pVB = NULL;
int											g_iType = 1;		//primitive type

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

	//set cull mode to cull none
	g_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	//set fill mode to wire frame
	g_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	return S_OK;
}
///Create Vertex Buffer
HRESULT initVB(){
	CUSTOMVERTEX vertices[] = {
		{  50.0f, 250.0f, 0.5f, 1.0f, 0xffff0000, },
		{ 150.0f,  50.0f, 0.5f, 1.0f, 0xffff0000, }, 
		{ 250.0f, 250.0f, 0.5f, 1.0f, 0xffff0000, },


		{ 350.0f,  50.0f, 0.5f, 1.0f, 0xffff0000, }, 
		{ 450.0f, 250.0f, 0.5f, 1.0f, 0xffff0000, },
		{ 550.0f, 50.0f,  0.5f, 1.0f, 0xffff0000, }
	};

	if(FAILED(g_pDevice->CreateVertexBuffer(6 * sizeof(CUSTOMVERTEX),
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
	g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(192,192,192), 1.0f, 0);
	if(SUCCEEDED(g_pDevice->BeginScene())){
		//Render to the back-buffer
		g_pDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
		g_pDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
		switch(g_iType){
			case 1:// triangle strip
				g_pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0,4);
				break;
			case 2://triangle list顶点至少为3，并且顶点个数必须能被3整除
				g_pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
				break;
			case 3://line strip
				g_pDevice->DrawPrimitive(D3DPT_LINESTRIP, 0, 5);
				break;
			case 4://line list顶点个数必须是>=2的偶数
				g_pDevice->DrawPrimitive(D3DPT_LINELIST, 0, 3);
				break;
			case 5://point list
				g_pDevice->DrawPrimitive(D3DPT_POINTLIST, 0, 6);
				break;
			case 6://triangle fan
				g_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 4);
				break;
		}
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
		case 50:
		case 51:
		case 52:
		case 53:
		case 54:
			g_iType = static_cast<int>(wParam) - 48;
			break;
			}
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT){
	WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L,0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		L"ClassName", NULL};
	RegisterClassEx(&wc);

	HWND hWnd = CreateWindow(L"ClassName", L"Basic Primitive Demo",
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
					render();  //渲染图形
				}
			}
		}
	}
	UnregisterClass(L"ClassName", wc.hInstance);
	return 0;
}