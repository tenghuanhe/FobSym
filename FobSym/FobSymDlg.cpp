
// FobSymDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FobSym.h"
#include "FobSymDlg.h"
#include "afxdialogex.h"
#include <afxtoolbarimages.h>
#include "EEGAMP.h"
#include "SerialPort.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UINT Thread_Target(LPVOID pParam);
UINT Thread_EEG_Data(LPVOID pParam);
UINT Thread_Fob_Data(LPVOID pParam);

HANDLE g_EventStopall;
HANDLE g_EventFobready;
HANDLE g_EventEEGready;
HANDLE g_EventStartall;

void convert(char data[12], double pos[3]);

// variables for SYMTOP
HANDLE g_hDevice;
STRU_DEVICE_INFO g_struDeviceInfo;
int g_nTimes = 0;
int g_nFlag = 0;
int g_nIndex = 0;
long g_lData = 0;
BOOL g_bWatch; 
DWORD g_dwNullTimes;
const int spleFreq = 1000;
const int channelNumber = 8;
short g_pBufferTmp[90];

// variables for Fob
CSerialPort port1;
CSerialPort port4;

// variables for Display
const int radius = 20;

// CFobSymDlg dialog

CFobSymDlg::CFobSymDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFobSymDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFobSymDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CFobSymDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CFobSymDlg::OnStart)
END_MESSAGE_MAP()


// CFobSymDlg message handlers

BOOL CFobSymDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Init for display interface
	CRect TempRect(0, 0, 1920, 1080);
	CWnd::SetWindowPos(NULL, 0, 0, TempRect.Width(), TempRect.Height(), SWP_NOZORDER | SWP_NOMOVE);
	bmp1.LoadBitmap(IDB_BITMAP1);

	// Init for SYMTOP EEG amplifier

	g_EventStopall = CreateEvent(NULL, TRUE, FALSE, NULL);
	g_EventFobready = CreateEvent(NULL, TRUE, FALSE, NULL);
	g_EventEEGready = CreateEvent(NULL, TRUE, FALSE, NULL);
	g_EventStartall = CreateEvent(NULL, TRUE, FALSE, NULL);

	// Init for Flock of birds motion tracker

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFobSymDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CRect rc;
		CPaintDC dc(this);
		GetClientRect(&rc);
		dc.FillSolidRect(&rc, RGB(255, 255, 255));

		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFobSymDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFobSymDlg::OnStart()
{
	AfxBeginThread(&Thread_Target, AfxGetApp()->GetMainWnd());
	AfxBeginThread(&Thread_Fob_Data, AfxGetApp()->GetMainWnd());
	AfxBeginThread(&Thread_EEG_Data, AfxGetApp()->GetMainWnd());
}

UINT Thread_Target(LPVOID pParam)
{
	CFobSymDlg* pDlg = (CFobSymDlg*)pParam;

	LARGE_INTEGER lTemp;
	LONGLONG QPart1, QPart2;
	double dfMinus, dfFreq, dfTim;
	QueryPerformanceFrequency(&lTemp);
	dfFreq = (double)lTemp.QuadPart;
	CWnd* pWnd = pDlg->GetDlgItem(IDC_PICTURE1);

	WaitForSingleObject(g_EventFobready, INFINITE);
	WaitForSingleObject(g_EventEEGready, INFINITE);
	SetEvent(g_EventStartall);

	for (int i = 0; i < 2000; i++)
	{
		QueryPerformanceCounter(&lTemp);
		QPart1 = lTemp.QuadPart;		
		do 
		{
			QueryPerformanceCounter(&lTemp);
			QPart2 = lTemp.QuadPart;
			dfMinus = (double)(QPart2 - QPart1);
			dfTim = dfMinus / dfFreq;
		} while (dfTim < 0.01);
		pWnd->SetWindowPos(AfxGetApp()->GetMainWnd(), i%1940-20, 520, 20, 20, SWP_NOZORDER);
	}

	SetEvent(g_EventStopall);	// Stop collecting fob and eeg data

	return 0;
}

UINT Thread_EEG_Data(LPVOID pParam)
{
	CFobSymDlg* pDlg = (CFobSymDlg*) AfxGetApp()->GetMainWnd();
	g_hDevice = OpenDevice();
	if (g_hDevice == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(_T("Failing to open EEG amplifier"));
	}
	g_struDeviceInfo = ReadDeviceInfo(g_hDevice);
	g_bWatch = TRUE;
	g_nTimes = 0;
	g_lData = 0;
	long lData = 0;
	g_dwNullTimes = 0;
	CString cs;

	SetEvent(g_EventEEGready);
	WaitForSingleObject(g_EventStartall, INFINITE);
	pDlg->GetDlgItem(IDC_EEG)->SetWindowTextW(_T("Hello EEG"));

	while (WAIT_OBJECT_0 != WaitForSingleObject(g_EventStopall, 0))
	{
		g_dwNullTimes++;
		
		if (g_dwNullTimes >= 20)
		{
			g_dwNullTimes = 0;
			CloseDevice(g_hDevice);
			Sleep(500);
			g_hDevice = OpenDevice();
			if (g_hDevice == INVALID_HANDLE_VALUE)
				continue;
		}

		ULONG nCounts;
		if (!ReadData(g_hDevice, &g_pBufferTmp[0], &nCounts))
			continue;
		cs.Format(_T("%d %d %d %ld"), g_pBufferTmp[23], g_pBufferTmp[34], g_pBufferTmp[45], lData);
		pDlg->GetDlgItem(IDC_EEG)->SetWindowTextW(cs);
		lData++;
		if (lData == 1)
		{
			// Start producing EEG data
		}
	}

	pDlg->GetDlgItem(IDC_EEG)->SetWindowTextW(_T("EEG bye"));

	return 0;
}

UINT Thread_Fob_Data(LPVOID pParam)
{
	CFobSymDlg *pDlg = (CFobSymDlg *) AfxGetApp()->GetMainWnd();

	port1.Open(1, 115200, CSerialPort::NoParity, 8, CSerialPort::OneStopBit,CSerialPort::XonXoffFlowControl);
	port1.SetRTS();
	Sleep(500);
	port1.ClearRTS();
	Sleep(500);

	port4.Open(5, 115200, CSerialPort::NoParity, 8, CSerialPort::OneStopBit,CSerialPort::XonXoffFlowControl);
	port4.SetRTS();
	Sleep(500);
	port4.ClearRTS();
	Sleep(500);

	CString cs;
	cs.Format(_T("%d bytes waiting in port1, %d bytes waiting in port4"), port1.BytesWaiting(), port4.BytesWaiting());
	pDlg->GetDlgItem(IDC_POS)->SetWindowTextW(cs);
	if (port1.BytesWaiting() > 0)
		port1.ClearReadBuffer();
	if (port4.BytesWaiting() > 0)
		port4.ClearReadBuffer();

	cs.Format(_T("%d bytes waiting in port1, %d bytes waiting in port4"), port1.BytesWaiting(), port4.BytesWaiting());
	pDlg->GetDlgItem(IDC_POS)->SetWindowTextW(cs);

	port1.TransmitChar('P');
	port1.TransmitChar(50);
	port1.TransmitChar(2);
	Sleep(1000);

	port4.TransmitChar('Y');
	Sleep(1000);

	SetEvent(g_EventFobready);
	WaitForSingleObject(g_EventStartall, INFINITE);
	pDlg->GetDlgItem(IDC_POS)->SetWindowTextW(_T("Hello Fob"));

	char sBuf[12];
	double pos[3];
	char bbuf[1] = {'B'};

	LARGE_INTEGER lTemp;
	LONGLONG QPart1, QPart2;
	double dfMinus, dfFreq, dfTim;
	QueryPerformanceFrequency(&lTemp);
	dfFreq = (double)lTemp.QuadPart;

	int i = 0;
	while (WAIT_OBJECT_0 != WaitForSingleObject(g_EventStopall, 0)) {
		QueryPerformanceCounter(&lTemp);
		QPart1 = lTemp.QuadPart;

		port4.ClearReadBuffer();
		port4.ClearWriteBuffer();
		port4.Write(bbuf, 1);
		do {
			QueryPerformanceCounter(&lTemp);
			QPart2 = lTemp.QuadPart;
			dfMinus = (double)(QPart2 - QPart1);
			dfTim = dfMinus / dfFreq;
		} while (dfTim < 0.0075);

		// Why can't use while {continue}?

		port4.Read(sBuf, 12);
		convert(sBuf, pos);
		cs.Format(_T("%.2lf, %.2lf, %.2lf, %d"), pos[0], pos[1], pos[2], i++);
		pDlg->GetDlgItem(IDC_POS)->SetWindowTextW(cs);
	}

	port1.Close();
	port4.Close();

	pDlg->GetDlgItem(IDC_POS)->SetWindowTextW(_T("Fob bye"));

	return 0;
}

void convert(char data[12], double pos[3])
{
	int x = (((data[0] - 128) << 1) + data[1] * 256) << 1;
	int y = ((data[2] << 1) + data[3] * 256) << 1;
	int z = ((data[4] << 1) + data[5] * 256) << 1;

	x = (x > 32767) ? (x - 65535) : x;
	y = (y > 32767) ? (y - 65535) : y;
	z = (z > 32767) ? (z - 65535) : z;

	pos[0] = x * 144.0 / 32768.0;
	pos[1] = y * 144.0 / 32768.0;
	pos[2] = z * 144.0 / 32768.0;
}
