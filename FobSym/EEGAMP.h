// EEGAMP.h : main header file for the EEGAMP DLL
//

#if !defined(AFX_EEGAMP_H__10A9D1B8_19AE_41DE_AE91_791C3A9A3FA5__INCLUDED_)
#define AFX_EEGAMP_H__10A9D1B8_19AE_41DE_AE91_791C3A9A3FA5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

/////////////////////////////////////////////////////////////////////////////
typedef struct _PARAM
{
	unsigned short		nSenseDegree;
	unsigned short		nHightFre;
	unsigned short		nTimeConstant;
	unsigned short		nWorkFre;
	unsigned short		nModeOfSign;
	unsigned short		nGateOfJam;
	unsigned short		nHold1;
	unsigned short		nHold2;

}STRU_PARAM,*P_STRU_PARAM;


typedef struct _DEVICEINFO_
{
	unsigned short		nRouteNum;
	unsigned short		nType;
	unsigned short		nDeviceID;
	unsigned short		nSwitchNo;

}STRU_DEVICE_INFO,*P_STRU_DEVICE_INFO;


extern "C" __declspec(dllexport) HANDLE __stdcall OpenDevice();
extern "C" __declspec(dllexport) BOOL __stdcall CloseDevice(HANDLE hDevice);
extern "C" __declspec(dllexport) BOOL __stdcall ValidateClientID(HANDLE hDevice,unsigned short nClientID);
extern "C" __declspec(dllexport) BOOL __stdcall ReadData(HANDLE hDevice,short* pBuffer,ULONG *nCounts);
extern "C" __declspec(dllexport) STRU_PARAM __stdcall ReadParam(HANDLE hDevice);
extern "C" __declspec(dllexport) STRU_DEVICE_INFO __stdcall ReadDeviceInfo(HANDLE hDevice);
extern "C" __declspec(dllexport) BOOL __stdcall WriteParam(HANDLE hDevice,STRU_PARAM struParam);

/*
	nParamType		Type
		1			DeviceInfo
		2			DeviceParam
		3			ImpedanceLED
		4			Sampling rate
*/
extern "C" __declspec(dllexport) BOOL __stdcall ReadParamEx(HANDLE hDevice,short nParamType,short* pBuffer,short* nSize);
extern "C" __declspec(dllexport) BOOL __stdcall WriteParamEx(HANDLE hDevice,short nParamType,short* pBuffer,short nSize);

#endif 
