// VocoderGUIDlg.h : header file
//

#if !defined(AFX_VOCODERGUIDLG_H__F93F4248_5661_4180_B396_5929D14B0369__INCLUDED_)
#define AFX_VOCODERGUIDLG_H__F93F4248_5661_4180_B396_5929D14B0369__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CVocoderGUIDlg dialog

class CVocoderGUIDlg : public CDialog
{
// Construction
public:
	void VocodeFinished();
	BOOL UpdateStatus(int frame_no);
	void StartStatus(int num_frames);
	CVocoderGUIDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CVocoderGUIDlg)
	enum { IDD = IDD_VOCODERGUI_DIALOG };
	CString	m_modulatorFile;
	CString	m_carrierFile;
	CString	m_outputFile;
	int		m_bandCount;
	BOOL	m_normalize;
	int		m_outputVolume;
	int		m_windowLength;
	int		m_windowOverlap;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVocoderGUIDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CVocoderGUIDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnModulatorBrowseButton();
	afx_msg void OnCarrierBrowseButton();
	afx_msg void OnOutputBrowseButton();
	virtual void OnOK();
	afx_msg void OnCustomdrawWindowLengthSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawWindowOverlapSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawBandCountSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCustomdrawOutputVolumeSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnStopButton();
	afx_msg void OnModulatorPlayButton();
	afx_msg void OnCarrierPlayButton();
	afx_msg void OnOutputPlayButton();
	afx_msg void OnChangeModulatorFileEdit();
	afx_msg void OnChangeCarrierFileEdit();
	afx_msg void OnChangeOutputFileEdit();
	afx_msg void OnRestoreDefaultsButton();
	afx_msg void OnVocodeListenButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void UpdateSliderValueLabel(int nSliderId, int nLabelId, int nOneValue = 1, char* pszUnits = NULL, int nOffset = 0);
	BOOL m_listenAfterVocode;
	void UpdateSliderLabels();
	void UpdatePlayButtonEnabled(int nEditCntl, int nPlayCntl);
	BOOL m_stopVocode;
	void DoVocode();
	void BrowseFile(BOOL bOpenFileDialog, CWnd* editCtl);
	void UpdatePlayButtonsEnabled();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VOCODERGUIDLG_H__F93F4248_5661_4180_B396_5929D14B0369__INCLUDED_)
