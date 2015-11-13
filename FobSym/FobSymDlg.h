
// FobSymDlg.h : header file
//

#pragma once


// CFobSymDlg dialog
class CFobSymDlg : public CDialogEx
{
// Construction
public:
	CFobSymDlg(CWnd* pParent = NULL);	// standard constructor
	CBitmap bmp1;

// Dialog Data
	enum { IDD = IDD_FOBSYM_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnStart();
};
