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
*
* Code taken from: https://github.com/strangebytes/diablo-ddrawwrapper
*/

#include "ddraw.h"

/************************/
/*** IUnknown methods ***/
/************************/

HRESULT m_IDirectDrawSurfaceX::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
	if (Config.Dd7to9)
	{
		if ((riid == IID_IDirectDrawSurface || riid == IID_IDirectDrawSurface2 || riid == IID_IDirectDrawSurface3 || riid == IID_IDirectDrawSurface4 || riid == IID_IDirectDrawSurface7 || riid == IID_IUnknown) && ppvObj)
		{
			AddRef();

			*ppvObj = this;

			return S_OK;
		}
	}
	else
	{
		if (ProxyDirectXVersion > 4 && ppvObj && (riid == IID_IDirect3DTexture || riid == IID_IDirect3DTexture2))
		{
			if (lpCurrentD3DDevice)
			{
				ProxyInterface->AddRef();

				*ppvObj = new m_IDirect3DTextureX(lpCurrentD3DDevice, 7, ProxyInterface);

				return S_OK;
			}
		}
	}

	return ProxyQueryInterface(ProxyInterface, riid, ppvObj, WrapperID, WrapperInterface);
}

ULONG m_IDirectDrawSurfaceX::AddRef()
{
	if (Config.Dd7to9)
	{
		return InterlockedIncrement(&RefCount);
	}

	return ProxyInterface->AddRef();
}

ULONG m_IDirectDrawSurfaceX::Release()
{
	ULONG ref;

	if (Config.Dd7to9)
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

/**********************************/
/*** IDirectDrawSurface methods ***/
/**********************************/

HRESULT m_IDirectDrawSurfaceX::AddAttachedSurface(LPDIRECTDRAWSURFACE7 lpDDSurface)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (lpDDSurface)
	{
		lpDDSurface = static_cast<m_IDirectDrawSurface7 *>(lpDDSurface)->GetProxyInterface();
	}

	return ProxyInterface->AddAttachedSurface(lpDDSurface);
}

HRESULT m_IDirectDrawSurfaceX::AddOverlayDirtyRect(LPRECT lpRect)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->AddOverlayDirtyRect(lpRect);
}

HRESULT m_IDirectDrawSurfaceX::Blt(LPRECT lpDestRect, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx)
{
	if (Config.Dd7to9)
	{
		// Check for required DDBLTFX structure
		if (!lpDDBltFx && (dwFlags & (DDBLT_DDFX | DDBLT_COLORFILL | DDBLT_DEPTHFILL | DDBLT_DDROPS | DDBLT_KEYDESTOVERRIDE | DDBLT_KEYSRCOVERRIDE | DDBLT_ROP | DDBLT_ROTATIONANGLE)))
		{
			Logging::Log() << __FUNCTION__ << " DDBLTFX structure not found";
			return DDERR_INVALIDPARAMS;
		}

		// Check for device
		if (!d3d9Device || !*d3d9Device || !ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " D3d9 Device not setup.";
			return DDERR_GENERIC;
		}

		// Do color fill
		if (dwFlags & DDBLT_COLORFILL)
		{
			return ColorFill(lpDestRect, lpDDBltFx->dwFillColor);
		}

		// Check for DDROP flag
		if (dwFlags & DDBLT_DDROPS)
		{
			Logging::Log() << __FUNCTION__ << " DDROP Not Implemented";
			return DDERR_NODDROPSHW;
		}

		// Check for raster operations flag
		if (dwFlags & DDBLT_ROP)
		{
			Logging::Log() << __FUNCTION__ << " Raster operations Not Implemented";
			return DDERR_NORASTEROPHW;
		}

		// Check for ZBuffer flags
		if (dwFlags & DDBLT_DEPTHFILL)
		{
			Logging::Log() << __FUNCTION__ << " Depth Fill Not Implemented";
			return DDERR_NOZBUFFERHW;
		}

		// Check for ZBuffer flags
		if ((dwFlags & DDBLT_ZBUFFER) || ((dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & (DDBLTFX_ZBUFFERBASEDEST | DDBLTFX_ZBUFFERRANGE))))
		{
			Logging::Log() << __FUNCTION__ << " ZBuffer Not Implemented";
			return DDERR_NOZBUFFERHW;
		}

		// Check for rotation flags
		if ((dwFlags & DDBLT_ROTATIONANGLE) || ((dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & (DDBLTFX_ROTATE180 | DDBLTFX_ROTATE270 | DDBLTFX_ROTATE90))))
		{
			Logging::Log() << __FUNCTION__ << " Rotation operations Not Implemented";
			return DDERR_NOROTATIONHW;
		}

		// Check for FX mirror flags
		if ((dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & (DDBLTFX_MIRRORLEFTRIGHT | DDBLTFX_MIRRORUPDOWN)))
		{
			Logging::Log() << __FUNCTION__ << " Mirror operations Not Implemented";
			return DDERR_NOMIRRORHW;
		}

		// Check for FX flag
		if ((dwFlags & DDBLT_DDFX) && (lpDDBltFx->dwDDFX & DDBLTFX_ARITHSTRETCHY))
		{
			Logging::Log() << __FUNCTION__ << " Stretch operations Not Implemented";
			return DDERR_NOSTRETCHHW;
		}

		// Check for color key flags
		if (dwFlags & (DDBLT_KEYDESTOVERRIDE | DDBLT_KEYSRCOVERRIDE | DDBLT_KEYDEST | DDBLT_KEYSRC))
		{
			Logging::Log() << __FUNCTION__ << " Color Key Not Implemented";
			return DDERR_UNSUPPORTED;
		}

		// Unused flags (can be safely ignored?)
		// DDBLT_ALPHA
		// DDBLT_ASYNC
		// DDBLT_DONOTWAIT
		// DDBLT_WAIT
		// DDBLTFX_NOTEARING

		// Check if source Surface exists
		m_IDirectDrawSurfaceX *lpDDSrcSurfaceX = (m_IDirectDrawSurfaceX*)lpDDSrcSurface;
		if (!lpDDSrcSurfaceX)
		{
			lpDDSrcSurfaceX = this;
		}
		else if (!ddrawParent->DoesSurfaceExist(lpDDSrcSurfaceX))
		{
			Logging::Log() << __FUNCTION__ << " Error, could not find source surface";
			return DDERR_INVALIDPARAMS;
		}

		// Check and copy destination and source rect
		RECT DestRect, SrcRect;
		if (!CheckSurfaceRect(&DestRect, lpDestRect) || !lpDDSrcSurfaceX->CheckSurfaceRect(&SrcRect, lpSrcRect))
		{
			Logging::Log() << __FUNCTION__ << " Error, invalid rect size";
			return DDERR_INVALIDRECT;
		}

		// Check if destination surface is not locked then lock it
		bool UnlockDest = false;
		if (NeedsLock())
		{
			HRESULT hr = SetLock(nullptr, 0);
			if (FAILED(hr))
			{
				Logging::Log() << __FUNCTION__ << " Error, could not lock dest surface";
				return DDERR_GENERIC;
			}
			UnlockDest = true;
		}

		// Destination and source varables
		D3DLOCKED_RECT DestLockRect, SrcLockRect;
		D3DFORMAT DestFormat, SrcFormat;
		DWORD DestBitCount, SrcBitCount;

		// Get destination surface information
		HRESULT hr = GetSurfaceInfo(&DestLockRect, &DestBitCount, &DestFormat);

		// Check source surface
		bool UnlockSrc = false;
		if (SUCCEEDED(hr) && lpDDSrcSurfaceX != this)
		{
			// Check if source surface is not locked then lock it
			if (lpDDSrcSurfaceX->NeedsLock())
			{
				hr = lpDDSrcSurfaceX->SetLock(nullptr, 0);
				if (FAILED(hr))
				{
					Logging::Log() << __FUNCTION__ << " Error, could not lock src surface";
					hr = DDERR_GENERIC;
				}
				else
				{
					UnlockSrc = true;
				}
			}

			// Get source surface information
			hr = lpDDSrcSurfaceX->GetSurfaceInfo(&SrcLockRect, &SrcBitCount, &SrcFormat);
		}
		else
		{
			SrcLockRect.pBits = DestLockRect.pBits;
			SrcLockRect.Pitch = DestLockRect.Pitch;
			SrcBitCount = DestBitCount;
			SrcFormat = DestFormat;
		}

		// Verify that Format is the same
		if (SUCCEEDED(hr) && SrcFormat != DestFormat)
		{
			Logging::Log() << __FUNCTION__ << " Error, Blt using different surface formats not Implemented";
			hr = DDERR_GENERIC;
		}

		// Verify that bit count is the same
		if (SUCCEEDED(hr) && SrcBitCount != DestBitCount)
		{
			Logging::Log() << __FUNCTION__ << " Error, Blt using different bit count not Implemented";
			hr = DDERR_GENERIC;
		}

		// Do basic Blt operation
		if (SUCCEEDED(hr))
		{
			// Get width and height of blt
			LONG BltWidth = min(DestRect.right - DestRect.left, SrcRect.right - SrcRect.left);
			LONG BltHeight = min(DestRect.bottom - DestRect.top, SrcRect.bottom - SrcRect.top);

			// Do surface Blt
			DWORD ByteCount = DestLockRect.Pitch / surfaceDesc2.dwWidth;
			for (LONG y = 0; y < BltHeight; y++)
			{
				memcpy((BYTE*)DestLockRect.pBits + ((y + DestRect.top) * DestLockRect.Pitch) + (DestRect.left * ByteCount),	// Destination video memory address
					(BYTE*)SrcLockRect.pBits + ((y + SrcRect.top) * SrcLockRect.Pitch) + (SrcRect.left * ByteCount),		// Source video memory address
					BltWidth * ByteCount);																					// Size of bytes to write
			}
		}

		// Unlock surfaces if needed
		if (UnlockDest)
		{
			SetUnLock();
		}
		if (UnlockSrc)
		{
			lpDDSrcSurfaceX->SetUnLock();
		}

		// If Blt to or from primary surface then prepare for EndScene
		PrepareEndScene();
		lpDDSrcSurfaceX->PrepareEndScene();

		// Return
		return hr;
	}

	if (lpDDSrcSurface)
	{
		lpDDSrcSurface = static_cast<m_IDirectDrawSurface7 *>(lpDDSrcSurface)->GetProxyInterface();
	}

	return ProxyInterface->Blt(lpDestRect, lpDDSrcSurface, lpSrcRect, dwFlags, lpDDBltFx);
}

HRESULT m_IDirectDrawSurfaceX::BltBatch(LPDDBLTBATCH lpDDBltBatch, DWORD dwCount, DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		if (!lpDDBltBatch)
		{
			return DDERR_INVALIDPARAMS;
		}

		HRESULT hr;

		for (DWORD x = 0; x < dwCount; x++)
		{
			hr = Blt(lpDDBltBatch[x].lprDest, (LPDIRECTDRAWSURFACE7)lpDDBltBatch[x].lpDDSSrc, lpDDBltBatch[x].lprSrc, lpDDBltBatch[x].dwFlags, lpDDBltBatch[x].lpDDBltFx);
			if (FAILED(hr))
			{
				return hr;
			}
		}

		return DD_OK;
	}

	if (lpDDBltBatch && lpDDBltBatch->lpDDSSrc)
	{
		lpDDBltBatch->lpDDSSrc = static_cast<m_IDirectDrawSurface *>(lpDDBltBatch->lpDDSSrc)->GetProxyInterface();
	}

	return ProxyInterface->BltBatch(lpDDBltBatch, dwCount, dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::BltFast(DWORD dwX, DWORD dwY, LPDIRECTDRAWSURFACE7 lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		// Convert BltFast flags into Blt flags
		DWORD Flags = 0;
		if (dwFlags & DDBLTFAST_SRCCOLORKEY)
		{
			Flags |= DDBLT_KEYSRC;
		}
		else if (dwFlags & DDBLTFAST_DESTCOLORKEY)
		{
			Flags |= DDBLT_KEYDEST;
		}
		if (dwFlags & DDBLTFAST_WAIT)
		{
			Flags |= DDBLT_WAIT;
		}

		// Get SrcRect
		RECT SrcRect;
		m_IDirectDrawSurfaceX *lpDDSrcSurfaceX = (!lpDDSrcSurface) ? this : (m_IDirectDrawSurfaceX*)lpDDSrcSurface;
		lpDDSrcSurfaceX->CheckSurfaceRect(&SrcRect, lpSrcRect);

		// Create DestRect
		RECT DestRect = { (LONG)dwX, (LONG)dwY, SrcRect.right - SrcRect.left + (LONG)dwX , SrcRect.bottom - SrcRect.top + (LONG)dwY };

		// Call Blt
		return Blt(&DestRect, lpDDSrcSurface, &SrcRect, Flags, nullptr);
	}

	if (lpDDSrcSurface)
	{
		lpDDSrcSurface = static_cast<m_IDirectDrawSurface7 *>(lpDDSrcSurface)->GetProxyInterface();
	}
	
	return ProxyInterface->BltFast(dwX, dwY, lpDDSrcSurface, lpSrcRect, dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::DeleteAttachedSurface(DWORD dwFlags, LPDIRECTDRAWSURFACE7 lpDDSAttachedSurface)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (lpDDSAttachedSurface)
	{
		lpDDSAttachedSurface = static_cast<m_IDirectDrawSurface7 *>(lpDDSAttachedSurface)->GetProxyInterface();
	}

	return ProxyInterface->DeleteAttachedSurface(dwFlags, lpDDSAttachedSurface);
}

HRESULT m_IDirectDrawSurfaceX::EnumAttachedSurfaces(LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback7)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not fully Implemented.";

		if (!lpEnumSurfacesCallback7)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Game using old DirectX, Convert back to LPDDSURFACEDESC
		if (ConvertSurfaceDescTo2)
		{
			DDSURFACEDESC Desc;
			ConvertSurfaceDesc(Desc, surfaceDesc2);
			lpEnumSurfacesCallback7(this, (LPDDSURFACEDESC2)&Desc, lpContext);
		}
		else
		{
			lpEnumSurfacesCallback7(this, &surfaceDesc2, lpContext);
		}

		return DD_OK;
	}

	ENUMSURFACE CallbackContext;
	CallbackContext.lpContext = lpContext;
	CallbackContext.lpCallback = lpEnumSurfacesCallback7;
	CallbackContext.DirectXVersion = DirectXVersion;
	CallbackContext.ConvertSurfaceDescTo2 = ConvertSurfaceDescTo2;

	return ProxyInterface->EnumAttachedSurfaces(&CallbackContext, m_IDirectDrawEnumSurface::ConvertCallback);
}

HRESULT m_IDirectDrawSurfaceX::EnumOverlayZOrders(DWORD dwFlags, LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpfnCallback)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	ENUMSURFACE CallbackContext;
	CallbackContext.lpContext = lpContext;
	CallbackContext.lpCallback = lpfnCallback;
	CallbackContext.DirectXVersion = DirectXVersion;
	CallbackContext.ConvertSurfaceDescTo2 = ConvertSurfaceDescTo2;

	return ProxyInterface->EnumOverlayZOrders(dwFlags, &CallbackContext, m_IDirectDrawEnumSurface::ConvertCallback);
}

HRESULT m_IDirectDrawSurfaceX::Flip(LPDIRECTDRAWSURFACE7 lpDDSurfaceTargetOverride, DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
			return DDERR_INVALIDOBJECT;
		}

		// Unneeded flags (can be safely ignored?)
		// Note: vsync handled by d3d9 PresentationInterval
		// - DDFLIP_DONOTWAIT
		// - DDFLIP_NOVSYNC
		// - DDFLIP_WAIT

		if ((dwFlags & (DDFLIP_INTERVAL2 | DDFLIP_INTERVAL3 | DDFLIP_INTERVAL4)) && (surfaceDesc2.ddsCaps.dwCaps2 & DDCAPS2_FLIPINTERVAL))
		{
			Logging::Log() << __FUNCTION__ << " Interval flipping not implemented";
		}

		// Check if attached surface is found
		bool FoundAttachedSurface = false;

		// If SurfaceTargetOverride then use that surface
		if (lpDDSurfaceTargetOverride)
		{
			m_IDirectDrawSurfaceX *lpTargetSurface = (m_IDirectDrawSurfaceX*)lpDDSurfaceTargetOverride;

			// Check if target surface exists
			if (!DoesAttachedSurfaceExist(lpTargetSurface) || lpTargetSurface == this)
			{
				Logging::Log() << __FUNCTION__ << " Invalid SurfaceTarget";
				return DDERR_INVALIDPARAMS;
			}

			// Found surface
			FoundAttachedSurface = true;

			// Swap textures
			LPDIRECT3DTEXTURE9 tmpAddr = surfaceTexture;
			surfaceTexture = *lpTargetSurface->GetSurfaceTexture();
			*lpTargetSurface->GetSurfaceTexture() = tmpAddr;
		}
		// Execute flip for all attached surfaces
		else
		{
			if ((dwFlags & DDFLIP_ODD) && (dwFlags & DDFLIP_EVEN))
			{
				return DDERR_INVALIDPARAMS;
			}

			if (dwFlags & DDFLIP_STEREO)
			{
				Logging::Log() << __FUNCTION__ << " Stereo flipping not implemented";
				return E_NOTIMPL;
			}

			if (dwFlags & (DDFLIP_ODD | DDFLIP_EVEN))
			{
				Logging::Log() << __FUNCTION__ << " Even and odd flipping not implemented";
				return E_NOTIMPL;
			}

			// Loop through each surface and swap them
			for (auto it : AttachedSurfaceMap)
			{
				m_IDirectDrawSurfaceX *lpTargetSurface = (m_IDirectDrawSurfaceX*)it.second;

				// Surface does not exist
				if (!ddrawParent->DoesSurfaceExist(lpTargetSurface))
				{
					RemoveAttachedSurfaceFromMap(lpTargetSurface);
				}
				// Found surface
				else
				{
					FoundAttachedSurface = true;

					// Swap textures
					LPDIRECT3DTEXTURE9 tmpAddr = surfaceTexture;
					surfaceTexture = *lpTargetSurface->GetSurfaceTexture();
					*lpTargetSurface->GetSurfaceTexture() = tmpAddr;
				}
			}
		}

		// Return error if no attached surfaces found
		if (!FoundAttachedSurface)
		{
			Logging::Log() << __FUNCTION__ << " No attached surfaces found";
			return DDERR_GENERIC;
		}

		// Set new texture
		HRESULT hr = (*d3d9Device)->SetTexture(0, surfaceTexture);
		if (FAILED(hr))
		{
			Logging::Log() << __FUNCTION__ << " Failed to set texture";
		}

		// Prepare EndScene
		PrepareEndScene();

		// Run EndScene after Flip
		ddrawParent->EndScene();

		return hr;
	}

	if (lpDDSurfaceTargetOverride)
	{
		lpDDSurfaceTargetOverride = static_cast<m_IDirectDrawSurface7 *>(lpDDSurfaceTargetOverride)->GetProxyInterface();
	}

	return ProxyInterface->Flip(lpDDSurfaceTargetOverride, dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::GetAttachedSurface(LPDDSCAPS2 lpDDSCaps, LPDIRECTDRAWSURFACE7 FAR * lplpDDAttachedSurface)
{
	if (!lplpDDAttachedSurface)
	{
		return DDERR_INVALIDPARAMS;
	}

	DDSCAPS2 Caps2;
	// Game using old DirectX, Convert DDSCAPS to DDSCAPS2
	if (lpDDSCaps && ConvertSurfaceDescTo2)
	{
		ConvertCaps(Caps2, *(LPDDSCAPS)lpDDSCaps);
		lpDDSCaps = &Caps2;
	}

	if (Config.Dd7to9)
	{
		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
			return DDERR_INVALIDOBJECT;
		}

		/*ToDo: GetAttachedSurface fails if more than one surface is attached that matches the capabilities requested. 
		In this case, the application must use the IDirectDrawSurface7::EnumAttachedSurfaces method to obtain the attached surfaces.*/

		DDSURFACEDESC2 DDSurfaceDesc2;
		memcpy(&DDSurfaceDesc2, &surfaceDesc2, sizeof(DDSURFACEDESC2));
		if (lpDDSCaps)
		{
			memcpy(&DDSurfaceDesc2.ddsCaps, lpDDSCaps, sizeof(DDSCAPS2));
		}
		DDSurfaceDesc2.ddsCaps.dwCaps &= ~DDSCAPS_PRIMARYSURFACE;		// Remove Primary surface flag

		m_IDirectDrawSurfaceX *attachedSurface = new m_IDirectDrawSurfaceX(d3d9Device, ddrawParent, DirectXVersion, &DDSurfaceDesc2, displayWidth, displayHeight);

		*lplpDDAttachedSurface = attachedSurface;

		AddAttachedSurfaceToMap(attachedSurface);

		ddrawParent->SetHasBackBuffer(true);

		return DD_OK;
	}

	HRESULT hr = ProxyInterface->GetAttachedSurface(lpDDSCaps, lplpDDAttachedSurface);

	if (SUCCEEDED(hr))
	{
		*lplpDDAttachedSurface = ProxyAddressLookupTable.FindAddress<m_IDirectDrawSurface7>(*lplpDDAttachedSurface, DirectXVersion);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::GetBltStatus(DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetBltStatus(dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::GetCaps(LPDDSCAPS2 lpDDSCaps)
{
	if (!lpDDSCaps)
	{
		return DDERR_INVALIDPARAMS;
	}

	LPDDSCAPS2 lpDDSCaps_tmp = lpDDSCaps;
	DDSCAPS2 Caps2;

	// Game using old DirectX, Convert DDSCAPS to DDSCAPS2
	if (ConvertSurfaceDescTo2)
	{
		lpDDSCaps = &Caps2;
	}

	HRESULT hr;

	if (Config.Dd7to9)
	{
		ConvertCaps(*lpDDSCaps, surfaceDesc2.ddsCaps);

		hr = DD_OK;
	}
	else
	{
		hr = ProxyInterface->GetCaps(lpDDSCaps);
	}

	// Convert back to DDSCAPS
	if (SUCCEEDED(hr) && ConvertSurfaceDescTo2)
	{
		lpDDSCaps = lpDDSCaps_tmp;
		ConvertCaps(*(LPDDSCAPS)lpDDSCaps, Caps2);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::GetClipper(LPDIRECTDRAWCLIPPER FAR * lplpDDClipper)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	HRESULT hr = ProxyInterface->GetClipper(lplpDDClipper);

	if (SUCCEEDED(hr) && lplpDDClipper)
	{
		*lplpDDClipper = ProxyAddressLookupTable.FindAddress<m_IDirectDrawClipper>(*lplpDDClipper);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::GetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey)
{
	if (!lpDDColorKey)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (Config.Dd7to9)
	{
		// Get color key index
		int x = (dwFlags & DDCKEY_DESTBLT) ? 0 :
			(dwFlags & DDCKEY_DESTOVERLAY) ? 1 :
			(dwFlags & DDCKEY_SRCBLT) ? 2 :
			(dwFlags & DDCKEY_SRCOVERLAY) ? 3 : -1;

		// Check index
		if (x == -1)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Check if color key is set
		if (!ColorKeys[x].IsSet)
		{
			return DDERR_NOCOLORKEY;
		}

		// Copy color key
		memcpy(lpDDColorKey, &ColorKeys[x].Key, sizeof(DDCOLORKEY));

		// Return
		return DD_OK;
	}

	return ProxyInterface->GetColorKey(dwFlags, lpDDColorKey);
}

HRESULT m_IDirectDrawSurfaceX::GetDC(HDC FAR * lphDC)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not fully Implemented.";

		if (!lphDC)
		{
			return DDERR_INVALIDPARAMS;
		}

		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
			return DDERR_INVALIDOBJECT;
		}

		*lphDC = ddrawParent->GetWindowDC();

		if (!*lphDC)
		{
			return DDERR_GENERIC;
		}

		return DD_OK;
	}

	return ProxyInterface->GetDC(lphDC);
}

HRESULT m_IDirectDrawSurfaceX::GetFlipStatus(DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetFlipStatus(dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::GetOverlayPosition(LPLONG lplX, LPLONG lplY)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not fully Implemented.";

		if (!lplX || !lplY)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Set lplX and lplY to X, Y of this overlay surface
		*lplX = overlayX;
		*lplY = overlayY;

		return DD_OK;
	}

	return ProxyInterface->GetOverlayPosition(lplX, lplY);
}

HRESULT m_IDirectDrawSurfaceX::GetPalette(LPDIRECTDRAWPALETTE FAR * lplpDDPalette)
{
	if (!lplpDDPalette)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (Config.Dd7to9)
	{
		// No palette attached
		if (!attachedPalette)
		{
			return DDERR_NOPALETTEATTACHED;
		}

		// Check exclusive mode

		// Return attached palette
		*lplpDDPalette = (LPDIRECTDRAWPALETTE)attachedPalette;

		// Success
		return DD_OK;
	}

	HRESULT hr = ProxyInterface->GetPalette(lplpDDPalette);

	if (SUCCEEDED(hr) && lplpDDPalette)
	{
		*lplpDDPalette = ProxyAddressLookupTable.FindAddress<m_IDirectDrawPalette>(*lplpDDPalette);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::GetPixelFormat(LPDDPIXELFORMAT lpDDPixelFormat)
{
	if (!lpDDPixelFormat)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (Config.Dd7to9)
	{
		// lpDDPixelFormat receives a detailed description of the current pixel and 
		// color space format of this surface.

		// Copy pixel format to lpDDPixelFormat
		memcpy(lpDDPixelFormat, &surfaceDesc2.ddpfPixelFormat, sizeof(DDPIXELFORMAT));

		return DD_OK;
	}

	return ProxyInterface->GetPixelFormat(lpDDPixelFormat);
}

HRESULT m_IDirectDrawSurfaceX::GetSurfaceDesc(LPDDSURFACEDESC2 lpDDSurfaceDesc2)
{
	if (!lpDDSurfaceDesc2)
	{
		return DDERR_INVALIDPARAMS;
	}

	// Game using old DirectX, Convert to LPDDSURFACEDESC2
	LPDDSURFACEDESC2 lpDDSurfaceDesc_tmp = lpDDSurfaceDesc2;
	DDSURFACEDESC2 Desc2;
	if (ConvertSurfaceDescTo2)
	{
		ConvertSurfaceDesc(Desc2, *(LPDDSURFACEDESC)lpDDSurfaceDesc2);
		lpDDSurfaceDesc2 = &Desc2;
	}

	HRESULT hr;

	if (Config.Dd7to9)
	{
		// Copy surfacedesc to lpDDSurfaceDesc2
		memcpy(lpDDSurfaceDesc2, &surfaceDesc2, sizeof(DDSURFACEDESC2));

		hr = DD_OK;
	}
	else
	{
		hr = ProxyInterface->GetSurfaceDesc(lpDDSurfaceDesc2);
	}

	// Convert back to LPDDSURFACEDESC
	if (SUCCEEDED(hr) && ConvertSurfaceDescTo2)
	{
		lpDDSurfaceDesc2 = lpDDSurfaceDesc_tmp;
		ConvertSurfaceDesc(*(LPDDSURFACEDESC)lpDDSurfaceDesc2, Desc2);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::Initialize(LPDIRECTDRAW lpDD, LPDDSURFACEDESC2 lpDDSurfaceDesc2)
{
	DDSURFACEDESC2 Desc2;
	if (lpDDSurfaceDesc2 && ConvertSurfaceDescTo2)
	{
		ConvertSurfaceDesc(Desc2, *(LPDDSURFACEDESC)lpDDSurfaceDesc2);
		lpDDSurfaceDesc2 = &Desc2;
	}

	if (Config.Dd7to9)
	{
		// Not needed
		return DD_OK;
	}

	if (lpDD)
	{
		lpDD = static_cast<m_IDirectDraw *>(lpDD)->GetProxyInterface();
	}

	return ProxyInterface->Initialize(lpDD, lpDDSurfaceDesc2);
}

HRESULT m_IDirectDrawSurfaceX::IsLost()
{
	if (Config.Dd7to9)
	{
		// You can use this method to determine when you need to reallocate surface memory. 
		// When a DirectDrawSurface object loses its surface memory, most methods return 
		// DDERR_SURFACELOST and perform no other action.

		// Check if surface is lost or not, if not return OK

		// Surface never lost
		return DD_OK;
	}

	return ProxyInterface->IsLost();
}

HRESULT m_IDirectDrawSurfaceX::Lock(LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc2, DWORD dwFlags, HANDLE hEvent)
{
	if (!lpDDSurfaceDesc2)
	{
		return DDERR_INVALIDPARAMS;
	}

	LPDDSURFACEDESC2 lpDDSurfaceDesc_tmp = lpDDSurfaceDesc2;
	DDSURFACEDESC2 Desc2;

	// Game using old DirectX, Convert to LPDDSURFACEDESC2
	if (lpDDSurfaceDesc2 && ConvertSurfaceDescTo2)
	{
		ConvertSurfaceDesc(Desc2, *(LPDDSURFACEDESC)lpDDSurfaceDesc2);
		lpDDSurfaceDesc2 = &Desc2;
	}

	HRESULT hr;

	if (Config.Dd7to9)
	{
		// Save Lock Rect
		if (lpDestRect)
		{
			memcpy(&LockDestRect, lpDestRect, sizeof(RECT));
		}
		else
		{
			LockDestRect.top = 0;
			LockDestRect.left = 0;
			LockDestRect.right = surfaceDesc2.dwWidth;
			LockDestRect.bottom = surfaceDesc2.dwHeight;
		}

		// Make sure surface exists, if not then create it
		if (!surfaceTexture)
		{
			hr = CreateD3d9Surface();
			if (FAILED(hr))
			{
				Logging::Log() << __FUNCTION__ << " could not recreate surface";
			}
		}
		// If surface exists then set hr to DD_OK
		else
		{
			hr = DD_OK;
		}

		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
			return DDERR_INVALIDOBJECT;
		}

		// Try BeginScene (ignore results)
		ddrawParent->BeginScene();

		// Lock rect
		if (SUCCEEDED(hr))
		{
			// Convert flags to d3d9
			DWORD LockFlags = dwFlags & (D3DLOCK_DISCARD | D3DLOCK_DONOTWAIT | D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOOVERWRITE | D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY);

			// Lock surface
			hr = SetLock(lpDestRect, LockFlags);
		}

		// Set desc and video memory
		if (SUCCEEDED(hr))
		{
			// Copy desc to passed in desc
			memcpy(lpDDSurfaceDesc2, &surfaceDesc2, sizeof(DDSURFACEDESC2));

			// Set video memory and pitch
			lpDDSurfaceDesc2->dwFlags |= DDSD_LPSURFACE | DDSD_PITCH;
			if (WriteDirectlyToSurface && d3dlrect.pBits)
			{
				lpDDSurfaceDesc2->lpSurface = d3dlrect.pBits;
				lpDDSurfaceDesc2->lPitch = d3dlrect.Pitch;
			}
			else
			{
				if (!rawVideoBuf)
				{
					AlocateVideoBuffer();
				}
				lpDDSurfaceDesc2->lpSurface = (LPVOID)rawVideoBuf;
				lpDDSurfaceDesc2->lPitch = surfaceDesc2.dwWidth * (GetBitCount(surfaceDesc2.ddpfPixelFormat) / 8);
			}
		}
	}
	else
	{
		hr = ProxyInterface->Lock(lpDestRect, lpDDSurfaceDesc2, dwFlags, hEvent);
	}

	// Convert back to LPDDSURFACEDESC
	if (SUCCEEDED(hr) && ConvertSurfaceDescTo2)
	{
		lpDDSurfaceDesc2 = lpDDSurfaceDesc_tmp;
		ConvertSurfaceDesc(*(LPDDSURFACEDESC)lpDDSurfaceDesc2, Desc2);
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::ReleaseDC(HDC hDC)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not fully Implemented.";

		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
			return DDERR_INVALIDOBJECT;
		}

		if (ddrawParent->ReleaseWindowDC(hDC) == 0)
		{
			return DDERR_GENERIC;
		}

		return DD_OK;
	}

	return ProxyInterface->ReleaseDC(hDC);
}

HRESULT m_IDirectDrawSurfaceX::Restore()
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->Restore();
}

HRESULT m_IDirectDrawSurfaceX::SetClipper(LPDIRECTDRAWCLIPPER lpDDClipper)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (lpDDClipper)
	{
		lpDDClipper = static_cast<m_IDirectDrawClipper *>(lpDDClipper)->GetProxyInterface();
	}

	return ProxyInterface->SetClipper(lpDDClipper);
}

HRESULT m_IDirectDrawSurfaceX::SetColorKey(DWORD dwFlags, LPDDCOLORKEY lpDDColorKey)
{
	if (!lpDDColorKey)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (Config.Dd7to9)
	{
		// Get color key index
		int x = (dwFlags & DDCKEY_DESTBLT) ? 0 :
			(dwFlags & DDCKEY_DESTOVERLAY) ? 1 :
			(dwFlags & DDCKEY_SRCBLT) ? 2 :
			(dwFlags & DDCKEY_SRCOVERLAY) ? 3 : -1;

		// Check index
		if (x == -1)
		{
			return DDERR_INVALIDPARAMS;
		}

		// Set color key
		if (!lpDDColorKey)
		{
			ColorKeys[x].IsSet = false;
		}
		else
		{
			ColorKeys[x].IsSet = true;
			ColorKeys[x].IdColorSpace = ((dwFlags & DDCKEY_COLORSPACE) != 0);
			memcpy(&ColorKeys[x].Key, lpDDColorKey, sizeof(DDCOLORKEY));
		}

		// Return
		return DD_OK;
	}

	return ProxyInterface->SetColorKey(dwFlags, lpDDColorKey);
}

HRESULT m_IDirectDrawSurfaceX::SetOverlayPosition(LONG lX, LONG lY)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not fully Implemented.";

		// Store the new overlay position
		overlayX = lX;
		overlayY = lY;

		return DD_OK;
	}

	return ProxyInterface->SetOverlayPosition(lX, lY);
}

HRESULT m_IDirectDrawSurfaceX::SetPalette(LPDIRECTDRAWPALETTE lpDDPalette)
{
	if (Config.Dd7to9)
	{
		static bool FirstRun = true;

		// If lpDDPalette is nullptr then detach the current palette if it exists
		if (!lpDDPalette)
		{
			if (attachedPalette)
			{
				// Decrement ref count
				attachedPalette->Release();

				// Detach
				attachedPalette = nullptr;
			}

			// Reset FirstRun
			FirstRun = true;

			return DD_OK;
		}

		// Set palette address
		attachedPalette = (m_IDirectDrawPalette *)lpDDPalette;

		// Don't write to surface when using palettes
		WriteDirectlyToSurface = false;
		AlocateVideoBuffer();

		// When you call SetPalette to set a palette to a surface for the first time, 
		// SetPalette increments the palette's reference count; subsequent calls to 
		// SetPalette do not affect the palette's reference count.
		if (FirstRun)
		{
			attachedPalette->AddRef();
			FirstRun = false;
		}

		return DD_OK;
	}

	if (lpDDPalette)
	{
		lpDDPalette = static_cast<m_IDirectDrawPalette *>(lpDDPalette)->GetProxyInterface();
	}

	return ProxyInterface->SetPalette(lpDDPalette);
}

HRESULT m_IDirectDrawSurfaceX::Unlock(LPRECT lpRect)
{
	if (Config.Dd7to9)
	{
		// If no video memory than nothing to do...
		if (!d3dlrect.pBits)
		{
			return DD_OK;
		}

		// Check for video buffer and create it if missing
		if (!WriteDirectlyToSurface && !rawVideoBuf)
		{
			AlocateVideoBuffer();
		}

		// Create raw video memory and rgb buffer varables
		UINT32 *surfaceBuffer = (UINT32*)d3dlrect.pBits;
		DWORD BitCount = GetBitCount(surfaceDesc2.ddpfPixelFormat);

		// Write to surface using a palette
		if (attachedPalette && attachedPalette->rgbPalette && !WriteDirectlyToSurface)
		{
			// Translate palette to rgb video buffer
			switch (BitCount)
			{
			case 8:
			{
				LONG x = 0, z = 0;
				for (LONG j = LockDestRect.top; j < LockDestRect.bottom; j++)
				{
					z = j * (d3dlrect.Pitch / 4);
					for (LONG i = LockDestRect.left; i < LockDestRect.right; i++)
					{
						x = z + i;
						surfaceBuffer[x] = attachedPalette->rgbPalette[rawVideoBuf[x]];
					}
				}
				break;
			}
			default:
				Logging::Log() << __FUNCTION__ << " No support for palette on " << BitCount << "-bit surfaces!";
				break;
			}
		}

		// Write to surface from the memory buffer
		else if (!WriteDirectlyToSurface)
		{
			// Get display format
			D3DFORMAT Format = GetDisplayFormat(surfaceDesc2.ddpfPixelFormat);

			// Translate system memory buffer to rgb video using specified format
			switch (BitCount)
			{
			case 8:
				Logging::Log() << __FUNCTION__ << " No support for non-palette 8-bit surfaces!";
				break;
			case 16:
				switch (Format)
				{
				case D3DFMT_R5G6B5:
				{
					LONG x = 0, z = 0;
					WORD *RawBuffer = (WORD*)&rawVideoBuf[0];
					for (LONG j = LockDestRect.top; j < LockDestRect.bottom; j++)
					{
						z = j * (d3dlrect.Pitch / 4);
						for (LONG i = LockDestRect.left; i < LockDestRect.right; i++)
						{
							x = z + i;

							// More accurate but greater than twice as slow
							/*surfaceBuffer[x] = D3DCOLOR_XRGB(
							((RawBuffer[x] & 0xF800) >> 11) * 255 / 31,				// Red
							((RawBuffer[x] & 0x07E0) >> 5) * 255 / 63,				// Green
							((RawBuffer[x] & 0x001F)) * 255 / 31);					// Blue	*/

							// Fastest but not as accurate
							surfaceBuffer[x] = ((RawBuffer[x] & 0xF800) << 8) +		// Red
								((RawBuffer[x] & 0x07E0) << 5) +					// Green
								((RawBuffer[x] & 0x001F) << 3);						// Blue
						}
					}
					break;
				}
				default:
					Logging::Log() << __FUNCTION__ << " Unsupported 16-bit format type: " << Format;
					break;
				}
				break;
			case 24:
				Logging::Log() << __FUNCTION__ << " 24-bit surfaces not Implemented!";
				break;
			case 32:
				switch (Format)
				{
				case D3DFMT_A8R8G8B8:
				case D3DFMT_X8R8G8B8:
				{
					// Copy raw rgb memory to surface by scanline observing pitch
					const LONG x = LockDestRect.right - LockDestRect.left;
					for (LONG y = LockDestRect.top; y < LockDestRect.bottom; y++)
					{
						memcpy((BYTE *)d3dlrect.pBits + (y * d3dlrect.Pitch), &rawVideoBuf[y * x], x * sizeof(UINT32));
					}
					break;
				}
				default:
					Logging::Log() << __FUNCTION__ << " Unsupported format type: " << Format;
					break;
				}
				break;
			default:
				Logging::Log() << __FUNCTION__ << " Unsupported bit count: " << BitCount;
				break;
			}
		}

		// Unlock surface
		HRESULT hr = SetUnLock();

		// Prepare EndScene (ignore results)
		PrepareEndScene();

		// Return
		return hr;
	}

	return ProxyInterface->Unlock(lpRect);
}

HRESULT m_IDirectDrawSurfaceX::UpdateOverlay(LPRECT lpSrcRect, LPDIRECTDRAWSURFACE7 lpDDDestSurface, LPRECT lpDestRect, DWORD dwFlags, LPDDOVERLAYFX lpDDOverlayFx)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (lpDDDestSurface)
	{
		lpDDDestSurface = static_cast<m_IDirectDrawSurface7 *>(lpDDDestSurface)->GetProxyInterface();
	}

	return ProxyInterface->UpdateOverlay(lpSrcRect, lpDDDestSurface, lpDestRect, dwFlags, lpDDOverlayFx);
}

HRESULT m_IDirectDrawSurfaceX::UpdateOverlayDisplay(DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->UpdateOverlayDisplay(dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::UpdateOverlayZOrder(DWORD dwFlags, LPDIRECTDRAWSURFACE7 lpDDSReference)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	if (lpDDSReference)
	{
		lpDDSReference = static_cast<m_IDirectDrawSurface7 *>(lpDDSReference)->GetProxyInterface();
	}

	return ProxyInterface->UpdateOverlayZOrder(dwFlags, lpDDSReference);
}

/*********************************/
/*** Added in the v2 interface ***/
/*********************************/

HRESULT m_IDirectDrawSurfaceX::GetDDInterface(LPVOID FAR * lplpDD)
{
	if (!lplpDD)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (Config.Dd7to9)
	{
		if (!ddrawParent)
		{
			Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
			return DDERR_INVALIDOBJECT;
		}

		// Set lplpDD to directdraw object that created this surface
		*lplpDD = ddrawParent;

		return DD_OK;
	}

	HRESULT hr = ProxyInterface->GetDDInterface(lplpDD);

	if (SUCCEEDED(hr))
	{
		// Calling the GetDDInterface method from any surface created under DirectDrawEx will return a pointer to the 
		// IUnknown interface instead of a pointer to an IDirectDraw interface. Applications must use the
		// IUnknown::QueryInterface method to retrieve the IDirectDraw, IDirectDraw2, or IDirectDraw3 interfaces.
		IID tmpID = (ProxyDirectXVersion == 1) ? IID_IDirectDraw :
			(ProxyDirectXVersion == 2) ? IID_IDirectDraw2 :
			(ProxyDirectXVersion == 3) ? IID_IDirectDraw3 :
			(ProxyDirectXVersion == 4) ? IID_IDirectDraw4 :
			(ProxyDirectXVersion == 7) ? IID_IDirectDraw7 : IID_IDirectDraw7;

		LPDIRECTDRAW7 lpDD = (LPDIRECTDRAW7)*lplpDD;

		hr = lpDD->QueryInterface(tmpID, lplpDD);
		if (SUCCEEDED(hr))
		{
			*lplpDD = ProxyAddressLookupTable.FindAddress<m_IDirectDraw7>(*lplpDD, DirectXVersion);
			lpDD->Release();
		}
	}

	return hr;
}

HRESULT m_IDirectDrawSurfaceX::PageLock(DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->PageLock(dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::PageUnlock(DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->PageUnlock(dwFlags);
}

/*********************************/
/*** Added in the v3 interface ***/
/*********************************/

HRESULT m_IDirectDrawSurfaceX::SetSurfaceDesc(LPDDSURFACEDESC2 lpDDsd, DWORD dwFlags)
{
	if (!lpDDsd)
	{
		return DDERR_INVALIDPARAMS;
	}

	// Game using old DirectX, Convert to LPDDSURFACEDESC2
	DDSURFACEDESC2 Desc2;
	if (ConvertSurfaceDescTo2)
	{
		ConvertSurfaceDesc(Desc2, *(LPDDSURFACEDESC)lpDDsd);
		lpDDsd = &Desc2;
	}

	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->SetSurfaceDesc(lpDDsd, dwFlags);
}

/*********************************/
/*** Added in the v4 interface ***/
/*********************************/

HRESULT m_IDirectDrawSurfaceX::SetPrivateData(REFGUID guidTag, LPVOID lpData, DWORD cbSize, DWORD dwFlags)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->SetPrivateData(guidTag, lpData, cbSize, dwFlags);
}

HRESULT m_IDirectDrawSurfaceX::GetPrivateData(REFGUID guidTag, LPVOID lpBuffer, LPDWORD lpcbBufferSize)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetPrivateData(guidTag, lpBuffer, lpcbBufferSize);
}

HRESULT m_IDirectDrawSurfaceX::FreePrivateData(REFGUID guidTag)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->FreePrivateData(guidTag);
}

HRESULT m_IDirectDrawSurfaceX::GetUniquenessValue(LPDWORD lpValue)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetUniquenessValue(lpValue);
}

HRESULT m_IDirectDrawSurfaceX::ChangeUniquenessValue()
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->ChangeUniquenessValue();
}

/***********************************/
/*** Moved Texture7 methods here ***/
/***********************************/

HRESULT m_IDirectDrawSurfaceX::SetPriority(DWORD dwPriority)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->SetPriority(dwPriority);
}

HRESULT m_IDirectDrawSurfaceX::GetPriority(LPDWORD lpdwPriority)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetPriority(lpdwPriority);
}

HRESULT m_IDirectDrawSurfaceX::SetLOD(DWORD dwMaxLOD)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->SetLOD(dwMaxLOD);
}

HRESULT m_IDirectDrawSurfaceX::GetLOD(LPDWORD lpdwMaxLOD)
{
	if (Config.Dd7to9)
	{
		Logging::Log() << __FUNCTION__ << " Not Implemented";
		return E_NOTIMPL;
	}

	return ProxyInterface->GetLOD(lpdwMaxLOD);
}

/************************/
/*** Helper functions ***/
/************************/

// Release surface and vertext buffer
void m_IDirectDrawSurfaceX::ReleaseD9Surface()
{
	// Release d3d9 surface
	DWORD x = 0;
	if (surfaceTexture)
	{
		while (surfaceTexture->Release() != 0 && ++x < 100) {}
		surfaceTexture = nullptr;
	}

	// Release d3d9 vertex buffer
	if (vertexBuffer)
	{
		while (vertexBuffer->Release() != 0 && ++x < 100) {}
		vertexBuffer = nullptr;
	}

	// Set surface video buffer to nullptr
	d3dlrect.pBits = nullptr;
}

// Alocate buffer for surface if format is not supported by d3d9 to support backwards compatibility
void m_IDirectDrawSurfaceX::AlocateVideoBuffer()
{
	// Store old temp buffer
	BYTE *tempBuf = rawVideoBuf;

	// No need to create a buffer
	if (WriteDirectlyToSurface)
	{
		return;
	}

	// Buffer size, always support 32bit
	BufferSize = surfaceDesc2.dwWidth * surfaceDesc2.dwHeight * sizeof(INT32);

	// Allocate the raw video buffer
	rawVideoBuf = new BYTE[BufferSize];

	// Clear raw memory
	ZeroMemory(rawVideoBuf, BufferSize * sizeof(BYTE));

	// Free memory in case there was an old one setup
	if (tempBuf)
	{
		delete tempBuf;
		tempBuf = nullptr;
	}
}

// Create surface
HRESULT m_IDirectDrawSurfaceX::CreateD3d9Surface()
{
	// Release existing surface
	ReleaseD9Surface();

	// Check for device
	if (!d3d9Device || !*d3d9Device || !ddrawParent)
	{
		Logging::Log() << __FUNCTION__ << " D3d9 Device not setup.";
		return DDERR_INVALIDOBJECT;
	}

	// Get d3d9Object
	IDirect3D9 *d3d9Object = ddrawParent->GetDirect3D();

	// Get usage
	DWORD Usage = 0;
	if ((surfaceDesc2.ddsCaps.dwCaps2 & DDSCAPS2_HINTSTATIC) == 0 || (surfaceDesc2.ddsCaps.dwCaps2 & DDSCAPS2_HINTDYNAMIC))
	{
		Usage |= D3DUSAGE_DYNAMIC;
	}
	if (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_MIPMAP)
	{
		Usage |= D3DUSAGE_AUTOGENMIPMAP;
	}
	if (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_WRITEONLY)
	{
		Usage |= D3DUSAGE_WRITEONLY;
	}

	// Get pool type
	D3DPOOL Pool = D3DPOOL_DEFAULT;
	if (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
	{
		Pool = D3DPOOL_SYSTEMMEM;
	}
	else if (((surfaceDesc2.ddsCaps.dwCaps & (DDSCAPS_VIDEOMEMORY | DDSCAPS_NONLOCALVIDMEM)) || (surfaceDesc2.ddsCaps.dwCaps2 & (DDSCAPS2_TEXTUREMANAGE | DDSCAPS2_D3DTEXTUREMANAGE))) &&
		((Usage & D3DUSAGE_DYNAMIC) == 0))		// D3DPOOL_MANAGED cannot be used with D3DUSAGE_DYNAMIC
	{
		Pool = D3DPOOL_MANAGED;
	}
	else if (surfaceDesc2.ddsCaps.dwCaps2 & DDSCAPS2_DONOTPERSIST)
	{
		Pool = D3DPOOL_SCRATCH;
	}

	// Get resource type
	// DDSCAPS_BACKBUFFER
	// DDSCAPS_FRONTBUFFER
	// DDSCAPS_OFFSCREENPLAIN
	// DDSCAPS_PRIMARYSURFACE
	// DDSCAPS_TEXTURE
	// DDSCAPS2_CUBEMAP

	// Other flags
	// DDSCAPS_ALPHA
	// DDSCAPS_COMPLEX
	// DDSCAPS_FLIP
	// DDSCAPS_HWCODEC
	// DDSCAPS_LIVEVIDEO
	// DDSCAPS_MODEX
	// DDSCAPS_OVERLAY
	// DDSCAPS_PALETTE
	// DDSCAPS_VIDEOPORT
	// DDSCAPS_VISIBLE
	// DDSCAPS_ZBUFFER
	// DDSCAPS2_CUBEMAP_POSITIVEX
	// DDSCAPS2_CUBEMAP_NEGATIVEX
	// DDSCAPS2_CUBEMAP_POSITIVEY
	// DDSCAPS2_CUBEMAP_NEGATIVEY
	// DDSCAPS2_CUBEMAP_POSITIVEZ
	// DDSCAPS2_CUBEMAP_NEGATIVEZ
	// DDSCAPS2_CUBEMAP_ALLFACES
	// DDSCAPS2_OPAQUE
	// DDSCAPS2_STEREOSURFACELEFT

	// Unused flags (can be safely ignored?)
	// DDSCAPS_3D
	// DDSCAPS_3DDEVICE
	// DDSCAPS_ALLOCONLOAD
	// DDSCAPS_OPTIMIZED
	// DDSCAPS_STANDARDVGAMODE
	// DDSCAPS2_HINTANTIALIASING
	// DDSCAPS2_MIPMAPSUBLEVEL

	// Get format
	D3DFORMAT Format = GetDisplayFormat(surfaceDesc2.ddpfPixelFormat);

	// Test format
	HRESULT hr = d3d9Object->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, Format, D3DUSAGE_QUERY_FILTER, D3DRTYPE_TEXTURE, Format);
	// If test fails or there is a palette then write to memory buffer
	if (FAILED(hr) || attachedPalette)
	{
		WriteDirectlyToSurface = false;
		Format = D3DFMT_X8R8G8B8;
		AlocateVideoBuffer();
	}
	// Write directly to surface
	else
	{
		WriteDirectlyToSurface = true;
	}

	// Create surface
	hr = (*d3d9Device)->CreateTexture(surfaceDesc2.dwWidth, surfaceDesc2.dwHeight, 1, Usage, Format, Pool, &surfaceTexture, nullptr);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Unable to create surface";
		return hr;
	}

	// Reset Locked flag
	IsLocked = false;

	// Only display surface if it is primary for now...
	if ((surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) == 0)
	{
		return DD_OK;
	}

	// Set vertex shader
	hr = (*d3d9Device)->SetVertexShader(nullptr);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Unable to set vertex shader";
		return hr;
	}

	// Set fv format
	hr = (*d3d9Device)->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Unable to set the current vertex stream format";
		return hr;
	}

	// Create vertex buffer
	hr = (*d3d9Device)->CreateVertexBuffer(sizeof(TLVERTEX) * 4, D3DUSAGE_DYNAMIC, (D3DFVF_XYZRHW | D3DFVF_TEX1), D3DPOOL_DEFAULT, &vertexBuffer, nullptr);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Unable to create vertex buffer";
		return hr;
	}

	// Set stream source
	hr = (*d3d9Device)->SetStreamSource(0, vertexBuffer, 0, sizeof(TLVERTEX));
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Unable to set vertex buffer stream source";
		return hr;
	}

	// Set render states(no lighting)
	hr = (*d3d9Device)->SetRenderState(D3DRS_LIGHTING, FALSE);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Unable to set device render state(no lighting)";
		return hr;
	}

	// Set scale mode to linear
	hr = (*d3d9Device)->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Failed to set D3D device to LINEAR sampling";
	}

	// Setup verticies (0,0,currentWidth,currentHeight)
	TLVERTEX* vertices;
	// Lock vertex buffer
	hr = vertexBuffer->Lock(0, 0, (void**)&vertices, 0);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Unable to lock vertex buffer";
		return hr;
	}

	// Set vertex points
	// 0, 0
	vertices[0].x = -0.5f;
	vertices[0].y = -0.5f;
	vertices[0].z = 0.0f;
	vertices[0].rhw = 1.0f;
	vertices[0].u = 0.0f;
	vertices[0].v = 0.0f;

	// currentWidth, 0
	vertices[1].x = (float)displayWidth - 0.5f;
	vertices[1].y = -0.5f;
	vertices[1].z = 0.0f;
	vertices[1].rhw = 1.0f;
	vertices[1].u = 1.0f;
	vertices[1].v = 0.0f;

	// currentWidth, scaledHeight
	vertices[2].x = (float)displayWidth - 0.5f;
	vertices[2].y = (float)displayHeight - 0.5f;
	vertices[2].z = 0.0f;
	vertices[2].rhw = 1.0f;
	vertices[2].u = 1.0f;
	vertices[2].v = 1.0f;

	// 0, currentHeight
	vertices[3].x = -0.5f;
	vertices[3].y = (float)displayHeight - 0.5f;
	vertices[3].z = 0.0f;
	vertices[3].rhw = 1.0f;
	vertices[3].u = 0.0f;
	vertices[3].v = 1.0f;

	// Unlcok vertex buffer
	hr = vertexBuffer->Unlock();
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Unable to unlock vertex buffer";
		return hr;
	}

	// Set texture
	hr = (*d3d9Device)->SetTexture(0, surfaceTexture);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Failed to set texture";
		return hr;
	}

	return hr;
}

// Check surface reck dimensions and copy rect to new rect
bool m_IDirectDrawSurfaceX::CheckSurfaceRect(LPRECT lpOutRect, LPRECT lpInRect)
{
	if (lpInRect)
	{
		memcpy(lpOutRect, lpInRect, sizeof(RECT));
	}
	else
	{
		lpOutRect->top = 0;
		lpOutRect->left = 0;
		lpOutRect->right = surfaceDesc2.dwWidth;
		lpOutRect->bottom = surfaceDesc2.dwHeight;
	}

	return lpOutRect->left < lpOutRect->right && lpOutRect->top < lpOutRect->bottom &&
		lpOutRect->left >= 0 && lpOutRect->top >= 0 &&
		lpOutRect->right <= (LONG)surfaceDesc2.dwWidth && lpOutRect->bottom <= (LONG)surfaceDesc2.dwHeight;
}

// Lock the d3d9 surface
HRESULT m_IDirectDrawSurfaceX::SetLock(LPRECT lpDestRect, DWORD dwFlags)
{
	if (!surfaceTexture)
	{
		return DDERR_GENERIC;
	}

	// Lock surface
	HRESULT hr = surfaceTexture->LockRect(0, &d3dlrect, lpDestRect, dwFlags);
	if (FAILED(hr))
	{
		d3dlrect.pBits = nullptr;
		Logging::Log() << __FUNCTION__ << " Failed to lock surface";
		return DDERR_GENERIC;
	}
	IsLocked = true;

	return hr;
}

// Unlock the d3d9 surface
HRESULT m_IDirectDrawSurfaceX::SetUnLock()
{
	if (!surfaceTexture)
	{
		return DDERR_GENERIC;
	}

	// Lock surface
	HRESULT hr = surfaceTexture->UnlockRect(0);
	if (FAILED(hr))
	{
		Logging::Log() << __FUNCTION__ << " Failed to unlock surface";
		return hr;
	}
	IsLocked = false;
	d3dlrect.pBits = nullptr;

	return hr;
}

// Get LOCKED_RECT, BitCount and Foramt for the surface
HRESULT m_IDirectDrawSurfaceX::GetSurfaceInfo(D3DLOCKED_RECT *pLockRect, DWORD *lpBitCount, D3DFORMAT *lpFormat)
{
	if (pLockRect)
	{
		if (NeedsLock())
		{
			return DDERR_GENERIC;
		}
		if (IsLocked)
		{
			memcpy(pLockRect, &d3dlrect, sizeof(D3DLOCKED_RECT));
		}
		else
		{
			pLockRect->pBits = rawVideoBuf;
			pLockRect->Pitch = surfaceDesc2.dwWidth * (GetBitCount(surfaceDesc2.ddpfPixelFormat) / 8);
		}
	}
	if (lpBitCount)
	{
		*lpBitCount = GetBitCount(surfaceDesc2.ddpfPixelFormat);
	}
	if (lpFormat)
	{
		*lpFormat = GetDisplayFormat(surfaceDesc2.ddpfPixelFormat);
	}
	return DD_OK;
}

// Always get SurfaceDesc2 no matter what DirectXVersion is used
HRESULT m_IDirectDrawSurfaceX::GetSurfaceDesc2(LPDDSURFACEDESC2 lpDDSurfaceDesc2)
{
	// Copy surfacedesc to lpDDSurfaceDesc2
	memcpy(lpDDSurfaceDesc2, &surfaceDesc2, sizeof(DDSURFACEDESC2));

	return DD_OK;
}

// Add attached surface to map
void m_IDirectDrawSurfaceX::AddAttachedSurfaceToMap(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (!lpSurfaceX)
	{
		return;
	}

	// Get map Key
	DWORD Key = 0;
	if (AttachedSurfaceMap.size() != 0)
	{
		Key = AttachedSurfaceMap.end()->first + 1;
	}

	// Store surface
	AttachedSurfaceMap[Key] = lpSurfaceX;
}

// Remove attached surface from map
void m_IDirectDrawSurfaceX::RemoveAttachedSurfaceFromMap(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (!lpSurfaceX)
	{
		return;
	}

	auto it = std::find_if(AttachedSurfaceMap.begin(), AttachedSurfaceMap.end(),
		[=](auto Map) -> bool { return Map.second == lpSurfaceX; });

	if (it != std::end(AttachedSurfaceMap))
	{
		AttachedSurfaceMap.erase(it);
	}
}

// Check if attached surface exists
bool m_IDirectDrawSurfaceX::DoesAttachedSurfaceExist(m_IDirectDrawSurfaceX* lpSurfaceX)
{
	if (!lpSurfaceX)
	{
		return false;
	}

	auto it = std::find_if(AttachedSurfaceMap.begin(), AttachedSurfaceMap.end(),
		[=](auto Map) -> bool { return Map.second == lpSurfaceX; });

	if (it == std::end(AttachedSurfaceMap))
	{
		return false;
	}

	if (!ddrawParent)
	{
		return false;
	}

	if (!ddrawParent->DoesSurfaceExist(lpSurfaceX))
	{
		RemoveAttachedSurfaceFromMap(lpSurfaceX);
		return false;
	}

	return true;
}

HRESULT m_IDirectDrawSurfaceX::ColorFill(RECT *pRect, DWORD dwFillColor)
{
	// Check and copy rect
	RECT DestRect;
	if (!CheckSurfaceRect(&DestRect, pRect))
	{
		Logging::Log() << __FUNCTION__ << " Error, invalid rect size";
		return DDERR_INVALIDRECT;
	}

	// Check if surface is not locked then lock it
	bool UnlockDest = false;
	if (NeedsLock())
	{
		HRESULT hr = SetLock(nullptr, 0);
		if (FAILED(hr))
		{
			Logging::Log() << __FUNCTION__ << " Error, could not lock dest surface";
			return DDERR_GENERIC;
		}
		UnlockDest = true;
	}

	// Get locked rect surface information
	D3DLOCKED_RECT DestLockRect;
	GetSurfaceInfo(&DestLockRect, nullptr, nullptr);

	// Check pBits
	if (!DestLockRect.pBits)
	{
		Logging::Log() << __FUNCTION__ << " Error, could not get pBits";
		return DDERR_GENERIC;
	}

	// Get width and height of blt
	LONG FillWidth = DestRect.right - DestRect.left;
	LONG FillHeight = DestRect.bottom - DestRect.top;

	// Get ByteCount and FillColor
	DWORD ByteCount = DestLockRect.Pitch / surfaceDesc2.dwWidth;
	DWORD FillColor = 0;
	if (attachedPalette && attachedPalette->rgbPalette && !WriteDirectlyToSurface)
	{
		FillColor = attachedPalette->rgbPalette[rawVideoBuf[dwFillColor]];
	}
	else if (ByteCount <= 4)
	{
		FillColor = dwFillColor;
	}
	else
	{
		Logging::Log() << __FUNCTION__ << " Error, could not find correct fill color for ByteCount " << ByteCount;
	}

	// Fill rect
	for (LONG y = 0; y < FillHeight; y++)
	{
		DWORD StartLocation = ((y + DestRect.top) * surfaceDesc2.dwWidth) + DestRect.left;
		if (ByteCount == 4)
		{
			memset((DWORD*)DestLockRect.pBits + StartLocation,	// Video memory address
				FillColor,										// Fill color
				FillWidth);										// Size of bytes to write
		}
		else
		{
			for (LONG x = 0; x < FillWidth; x++)
			{
				memcpy((BYTE*)DestLockRect.pBits + (StartLocation * ByteCount),		// Video memory address
					(BYTE*)&FillColor,												// Fill color
					ByteCount);														// Size of bytes to write
			}
		}
	}

	// Unlock surfaces if needed
	if (UnlockDest)
	{
		SetUnLock();
	}

	return DD_OK;
}

// Set flags to prepare for EndScene
HRESULT m_IDirectDrawSurfaceX::PrepareEndScene()
{
	if (!ddrawParent)
	{
		Logging::Log() << __FUNCTION__ << " Error no ddraw parent!";
		return DDERR_INVALIDOBJECT;
	}

	// Set ReadyToEndScene is this is primary surface
	if (surfaceDesc2.ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
	{
		ddrawParent->SetReadyToEndScene();

		// If no attached surfaces then make sure to reset HasBackBuffer
		if (AttachedSurfaceMap.size() == 0)
		{
			ddrawParent->SetHasBackBuffer(false);
		}
	}

	// Return
	return DD_OK;
}
