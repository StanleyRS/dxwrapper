/**
* Copyright (C) 2018 Elisha Riedlinger
*
* This software is  provided 'as-is', without any express  or implied  warranty. In no event will the
* authors be held liable for any damages arising from the use of this software.
* Permission  is granted  to anyone  to use  this software  for  any  purpose,  including  commercial
* applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*   1. The origin of this software must not be misrepresented; you must not claim that you  wrote the
*      original  software. If you use this  software  in a product, an  acknowledgment in the product
*      documentation would be appreciated but is not required.
*   2. Altered source versions must  be plainly  marked as such, and  must not be  misrepresented  as
*      being the original software.
*   3. This notice may not be removed or altered from any source distribution.
*/

#include "ddraw.h"

HRESULT m_IDirect3DViewportX::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
	if (ProxyDirectXVersion > 3)
	{
		if ((riid == IID_IDirect3DViewport || riid == IID_IDirect3DViewport2 || riid == IID_IDirect3DViewport3 || riid == IID_IUnknown) && ppvObj)
		{
			AddRef();

			*ppvObj = this;

			return S_OK;
		}
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID, WrapperInterface);
}

ULONG m_IDirect3DViewportX::AddRef()
{
	if (ProxyDirectXVersion > 3)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirect3DViewportX::Release()
{
	LONG ref;

	if (ProxyDirectXVersion > 3)
	{
		ref = InterlockedDecrement(&RefCount);
	}
	else
	{
		ref = ProxyInterface->Release();
	}

	if (ref == 0)
	{
		if (WrapperInterface)
		{
			WrapperInterface->DeleteMe();
		}
		else
		{
			delete this;
		}
	}

	return ref;
}

HRESULT m_IDirect3DViewportX::Initialize(LPDIRECT3D lpDirect3D)
{
	if (ProxyDirectXVersion > 3)
	{
		return D3D_OK;
	}

	if (lpDirect3D)
	{
		lpDirect3D = static_cast<m_IDirect3D *>(lpDirect3D)->GetProxyInterface();
	}

	return ProxyInterface->Initialize(lpDirect3D);
}

HRESULT m_IDirect3DViewportX::GetViewport(LPD3DVIEWPORT lpData)
{
	if (ProxyDirectXVersion > 3)
	{
		if (ViewPortSet)
		{
			ConvertViewport(*lpData, ViewPort);

			return D3D_OK;
		}

		if (ViewPort2Set)
		{
			ConvertViewport(*lpData, ViewPort2);

			return D3D_OK;
		}

		return D3DERR_NOVIEWPORTS;
	}

	return ProxyInterface->GetViewport(lpData);
}

HRESULT m_IDirect3DViewportX::SetViewport(LPD3DVIEWPORT lpData)
{
	if (ProxyDirectXVersion > 3)
	{
		ViewPortSet = true;

		ConvertViewport(ViewPort, *lpData);

		if (lpData->dwSize > 44 && (lpData->dvScaleX != 0 || lpData->dvScaleY != 0))
		{
			Logging::Log() << __FUNCTION__ << " 'Scale homogeneous' Not Implemented";
		}

		D3DVIEWPORT7 ViewPort7;

		ConvertViewport(ViewPort7, *lpData);		

		return D3DDeviceInterface->SetViewport(&ViewPort7);
	}

	return ProxyInterface->SetViewport(lpData);
}

HRESULT m_IDirect3DViewportX::TransformVertices(DWORD dwVertexCount, LPD3DTRANSFORMDATA lpData, DWORD dwFlags, LPDWORD lpOffscreen)
{
	if (ProxyDirectXVersion > 3)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->TransformVertices(dwVertexCount, lpData, dwFlags, lpOffscreen);
}

HRESULT m_IDirect3DViewportX::LightElements(DWORD dwElementCount, LPD3DLIGHTDATA lpData)
{
	if (ProxyDirectXVersion > 3)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->LightElements(dwElementCount, lpData);
}

HRESULT m_IDirect3DViewportX::SetBackground(D3DMATERIALHANDLE hMat)
{
	if (ProxyDirectXVersion > 3)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->SetBackground(hMat);
}

HRESULT m_IDirect3DViewportX::GetBackground(LPD3DMATERIALHANDLE lphMat, LPBOOL lpValid)
{
	if (ProxyDirectXVersion > 3)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetBackground(lphMat, lpValid);
}

HRESULT m_IDirect3DViewportX::SetBackgroundDepth(LPDIRECTDRAWSURFACE lpDDSurface)
{
	if (ProxyDirectXVersion > 3)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (lpDDSurface)
	{
		lpDDSurface = static_cast<m_IDirectDrawSurface *>(lpDDSurface)->GetProxyInterface();
	}

	return ProxyInterface->SetBackgroundDepth(lpDDSurface);
}

HRESULT m_IDirect3DViewportX::GetBackgroundDepth(LPDIRECTDRAWSURFACE * lplpDDSurface, LPBOOL lpValid)
{
	if (ProxyDirectXVersion > 3)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	HRESULT hr = ProxyInterface->GetBackgroundDepth(lplpDDSurface, lpValid);

	if (SUCCEEDED(hr) && lplpDDSurface)
	{
		*lplpDDSurface = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface>(*lplpDDSurface);
	}

	return hr;
}

HRESULT m_IDirect3DViewportX::Clear(DWORD dwCount, LPD3DRECT lpRects, DWORD dwFlags)
{
	if (ProxyDirectXVersion > 3)
	{
		return D3DDeviceInterface->Clear(dwCount, lpRects, dwFlags, 0, 0, 0);
	}

	return ProxyInterface->Clear(dwCount, lpRects, dwFlags);
}

HRESULT m_IDirect3DViewportX::AddLight(LPDIRECT3DLIGHT lpDirect3DLight)
{
	if (ProxyDirectXVersion > 3)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (lpDirect3DLight)
	{
		lpDirect3DLight = static_cast<m_IDirect3DLight *>(lpDirect3DLight)->GetProxyInterface();
	}

	return ProxyInterface->AddLight(lpDirect3DLight);
}

HRESULT m_IDirect3DViewportX::DeleteLight(LPDIRECT3DLIGHT lpDirect3DLight)
{
	if (ProxyDirectXVersion > 3)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (lpDirect3DLight)
	{
		lpDirect3DLight = static_cast<m_IDirect3DLight *>(lpDirect3DLight)->GetProxyInterface();
	}

	return ProxyInterface->DeleteLight(lpDirect3DLight);
}

HRESULT m_IDirect3DViewportX::NextLight(LPDIRECT3DLIGHT lpDirect3DLight, LPDIRECT3DLIGHT * lplpDirect3DLight, DWORD dwFlags)
{
	if (ProxyDirectXVersion > 3)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (lpDirect3DLight)
	{
		lpDirect3DLight = static_cast<m_IDirect3DLight *>(lpDirect3DLight)->GetProxyInterface();
	}

	HRESULT hr = ProxyInterface->NextLight(lpDirect3DLight, lplpDirect3DLight, dwFlags);

	if (SUCCEEDED(hr) && lplpDirect3DLight)
	{
		*lplpDirect3DLight = ProxyAddressLookupTable.FindAddress<m_IDirect3DLight>(*lplpDirect3DLight);
	}

	return hr;
}

HRESULT m_IDirect3DViewportX::GetViewport2(LPD3DVIEWPORT2 lpData)
{
	if (ProxyDirectXVersion > 3)
	{
		if (ViewPort2Set)
		{
			ConvertViewport(*lpData, ViewPort2);

			return D3D_OK;
		}

		if (ViewPortSet)
		{
			ConvertViewport(*lpData, ViewPort);

			return D3D_OK;
		}

		return D3DERR_NOVIEWPORTS;
	}

	return ProxyInterface->GetViewport2(lpData);
}

HRESULT m_IDirect3DViewportX::SetViewport2(LPD3DVIEWPORT2 lpData)
{
	if (ProxyDirectXVersion > 3)
	{
		ViewPort2Set = true;

		ConvertViewport(ViewPort2, *lpData);

		if (lpData->dwSize > 44 && (lpData->dvClipX != 0 || lpData->dvClipY != 0))
		{
			Logging::Log() << __FUNCTION__ << " 'clip volume' Not Implemented";
		}

		D3DVIEWPORT7 ViewPort7;

		ConvertViewport(ViewPort7, *lpData);

		return D3DDeviceInterface->SetViewport(&ViewPort7);
	}

	return ProxyInterface->SetViewport2(lpData);
}

HRESULT m_IDirect3DViewportX::SetBackgroundDepth2(LPDIRECTDRAWSURFACE4 lpDDS)
{
	if (ProxyDirectXVersion > 3)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (lpDDS)
	{
		lpDDS = static_cast<m_IDirectDrawSurface4 *>(lpDDS)->GetProxyInterface();
	}

	return ProxyInterface->SetBackgroundDepth2(lpDDS);
}

HRESULT m_IDirect3DViewportX::GetBackgroundDepth2(LPDIRECTDRAWSURFACE4 * lplpDDS, LPBOOL lpValid)
{
	if (ProxyDirectXVersion > 3)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	HRESULT hr = ProxyInterface->GetBackgroundDepth2(lplpDDS, lpValid);

	if (SUCCEEDED(hr) && lplpDDS)
	{
		*lplpDDS = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface4>(*lplpDDS);
	}

	return hr;
}

HRESULT m_IDirect3DViewportX::Clear2(DWORD dwCount, LPD3DRECT lpRects, DWORD dwFlags, D3DCOLOR dwColor, D3DVALUE dvZ, DWORD dwStencil)
{
	if (ProxyDirectXVersion > 3)
	{
		return D3DDeviceInterface->Clear(dwCount, lpRects, dwFlags, dwColor, dvZ, dwStencil);
	}

	return ProxyInterface->Clear2(dwCount, lpRects, dwFlags, dwColor, dvZ, dwStencil);
}
