// VocoderGUIDlg.cpp : implementation file
//

#include <setjmp.h>
#include <sstream>
#include "stdafx.h"
#include "VocoderGUI.h"
#include "VocoderGUIDlg.h"

extern "C"
{
#include "vocode.h"
#include "error.h"
extern int ipow(int, int);
extern int ilog2(int);
}

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int MAX_OUTPUT_VOLUME = 200;
static const int DEFAULT_OUTPUT_VOLUME = 100;
static const int ONE_OUTPUT_VOLUME = 100;
static const int DEFAULT_BAND_COUNT = 16;
static const int MAX_BAND_COUNT = 64;
static const int MAX_WINDOW_OVERLAP = 50;
static const int DEFAULT_WINDOW_OVERLAP = 50;
static const int WHOLE_WINDOW_OVERLAP = 100;
static const int MAX_WINDOW_LENGTH = 500;
static const int DEFAULT_WINDOW_LENGTH = 67;
static const int ONE_SECOND_WINDOW_LENGTH = 1000;
static const BOOL DEFAULT_NORMALIZE = TRUE;

static const char* PROFILE_SECTION = "Settings";
static const char* MODULATOR_FILE_PROFILE_ENTRY = "ModulatorFile";
static const char* CARRIER_FILE_PROFILE_ENTRY = "CarrierFile";
static const char* OUTPUT_FILE_PROFILE_ENTRY = "OutputFile";
static const char* WINDOW_LENGTH_PROFILE_ENTRY = "WindowLength";
static const char* WINDOW_OVERLAP_PROFILE_ENTRY = "WindowOverlap";
static const char* BAND_COUNT_PROFILE_ENTRY = "BandCount";
static const char* OUTPUT_VOLUME_PROFILE_ENTRY = "OutputVolume";
static const char* NORMALIZE_PROFILE_ENTRY = "Normalize";

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVocoderGUIDlg dialog

CVocoderGUIDlg::CVocoderGUIDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVocoderGUIDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CVocoderGUIDlg)
	m_modulatorFile = _T("");
	m_carrierFile = _T("");
	m_outputFile = _T("");
	m_bandCount = 0;
	m_normalize = FALSE;
	m_outputVolume = 0;
	m_windowLength = 0;
	m_windowOverlap = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVocoderGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CVocoderGUIDlg)
	DDX_Text(pDX, IDC_MODULATOR_FILE_EDIT, m_modulatorFile);
	DDX_Text(pDX, IDC_CARRIER_FILE_EDIT, m_carrierFile);
	DDX_Text(pDX, IDC_OUTPUT_FILE_EDIT, m_outputFile);
	DDX_Slider(pDX, IDC_BAND_COUNT_SLIDER, m_bandCount);
	DDX_Check(pDX, IDC_NORMALIZE_CHECK, m_normalize);
	DDX_Slider(pDX, IDC_OUTPUT_VOLUME_SLIDER, m_outputVolume);
	DDX_Slider(pDX, IDC_WINDOW_LENGTH_SLIDER, m_windowLength);
	DDX_Slider(pDX, IDC_WINDOW_OVERLAP_SLIDER, m_windowOverlap);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CVocoderGUIDlg, CDialog)
	//{{AFX_MSG_MAP(CVocoderGUIDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_MODULATOR_BROWSE_BUTTON, OnModulatorBrowseButton)
	ON_BN_CLICKED(IDC_CARRIER_BROWSE_BUTTON, OnCarrierBrowseButton)
	ON_BN_CLICKED(IDC_OUTPUT_BROWSE_BUTTON, OnOutputBrowseButton)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_WINDOW_LENGTH_SLIDER, OnCustomdrawWindowLengthSlider)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_WINDOW_OVERLAP_SLIDER, OnCustomdrawWindowOverlapSlider)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_BAND_COUNT_SLIDER, OnCustomdrawBandCountSlider)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_OUTPUT_VOLUME_SLIDER, OnCustomdrawOutputVolumeSlider)
	ON_BN_CLICKED(IDC_STOP_BUTTON, OnStopButton)
	ON_BN_CLICKED(IDC_MODULATOR_PLAY_BUTTON, OnModulatorPlayButton)
	ON_BN_CLICKED(IDC_CARRIER_PLAY_BUTTON, OnCarrierPlayButton)
	ON_BN_CLICKED(IDC_OUTPUT_PLAY_BUTTON, OnOutputPlayButton)
	ON_EN_CHANGE(IDC_MODULATOR_FILE_EDIT, OnChangeModulatorFileEdit)
	ON_EN_CHANGE(IDC_CARRIER_FILE_EDIT, OnChangeCarrierFileEdit)
	ON_EN_CHANGE(IDC_OUTPUT_FILE_EDIT, OnChangeOutputFileEdit)
	ON_BN_CLICKED(IDC_RESTORE_DEFAULTS_BUTTON, OnRestoreDefaultsButton)
	ON_BN_CLICKED(IDC_VOCODE_LISTEN_BUTTON, OnVocodeListenButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVocoderGUIDlg message handlers

BOOL CVocoderGUIDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(IDC_WINDOW_LENGTH_SLIDER);
	pSlider->SetRange(1, MAX_WINDOW_LENGTH);

	pSlider = (CSliderCtrl*)GetDlgItem(IDC_WINDOW_OVERLAP_SLIDER);
	pSlider->SetRange(0, MAX_WINDOW_OVERLAP);

	pSlider = (CSliderCtrl*)GetDlgItem(IDC_BAND_COUNT_SLIDER);
	pSlider->SetRange(1, MAX_BAND_COUNT);

	pSlider = (CSliderCtrl*)GetDlgItem(IDC_OUTPUT_VOLUME_SLIDER);
	pSlider->SetRange(0, MAX_OUTPUT_VOLUME);
	
	CWinApp* app = AfxGetApp();
	m_modulatorFile = CString(app->GetProfileString(PROFILE_SECTION, MODULATOR_FILE_PROFILE_ENTRY, ""));
	m_carrierFile = CString(app->GetProfileString(PROFILE_SECTION, CARRIER_FILE_PROFILE_ENTRY, ""));
	m_outputFile = CString(app->GetProfileString(PROFILE_SECTION, OUTPUT_FILE_PROFILE_ENTRY, ""));
	m_windowLength = app->GetProfileInt(PROFILE_SECTION, WINDOW_LENGTH_PROFILE_ENTRY, DEFAULT_WINDOW_LENGTH);
	m_windowOverlap = app->GetProfileInt(PROFILE_SECTION, WINDOW_OVERLAP_PROFILE_ENTRY, DEFAULT_WINDOW_OVERLAP);
	m_bandCount = app->GetProfileInt(PROFILE_SECTION, BAND_COUNT_PROFILE_ENTRY, DEFAULT_BAND_COUNT);
	m_outputVolume = app->GetProfileInt(PROFILE_SECTION, OUTPUT_VOLUME_PROFILE_ENTRY, DEFAULT_OUTPUT_VOLUME);
	m_normalize = app->GetProfileInt(PROFILE_SECTION, NORMALIZE_PROFILE_ENTRY, DEFAULT_NORMALIZE);

	UpdateData(FALSE);
	
	UpdatePlayButtonsEnabled();

	return TRUE;  // return TRUE  unless you set the focus to a control
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVocoderGUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CVocoderGUIDlg::OnDestroy()
{
	WinHelp(0L, HELP_QUIT);
	CDialog::OnDestroy();
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVocoderGUIDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVocoderGUIDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CVocoderGUIDlg::UpdatePlayButtonsEnabled()
{
	UpdatePlayButtonEnabled(IDC_MODULATOR_FILE_EDIT, IDC_MODULATOR_PLAY_BUTTON);
	UpdatePlayButtonEnabled(IDC_CARRIER_FILE_EDIT, IDC_CARRIER_PLAY_BUTTON);
	UpdatePlayButtonEnabled(IDC_OUTPUT_FILE_EDIT, IDC_OUTPUT_PLAY_BUTTON);
}

void CVocoderGUIDlg::OnModulatorBrowseButton() 
{
	BrowseFile(TRUE, GetDlgItem(IDC_MODULATOR_FILE_EDIT));	
}

void CVocoderGUIDlg::BrowseFile(BOOL bOpenFileDialog, CWnd *editCtl)
{
	CFileDialog fileDlg(bOpenFileDialog, ".wav", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "Sound Files (*.wav)|*.wav|All Files (*.*)|*.*||", this);
	int iRV = fileDlg.DoModal();
	if (iRV == IDOK)
	{
		editCtl->SetWindowText(fileDlg.GetPathName());
	}
}

void CVocoderGUIDlg::OnCarrierBrowseButton() 
{
	BrowseFile(TRUE, GetDlgItem(IDC_CARRIER_FILE_EDIT));	
}

void CVocoderGUIDlg::OnOutputBrowseButton() 
{
	BrowseFile(FALSE, GetDlgItem(IDC_OUTPUT_FILE_EDIT));	
}

static jmp_buf _vocodeErrorJmpBuf;
static CVocoderGUIDlg* _vocoderDlg;

static void _StartStatusCB(VINT num_frames)
{
	_vocoderDlg->StartStatus(num_frames);
}

static void _FinishStatusCB()
{
}

static VBOOL _UpdateStatusCB(VINT frame_no)
{
	return _vocoderDlg->UpdateStatus(frame_no);
}

static void _ErrorDisplayCB(char* message)
{
	AfxMessageBox(message);
	longjmp(_vocodeErrorJmpBuf, 1);
}

static UINT _VocodeThread(LPVOID pParam)
{
	if (setjmp(_vocodeErrorJmpBuf) == 0)
	{
		vocode();
	}
	vocode_cleanup();
	_vocoderDlg->VocodeFinished();
	return 0;
}


void CVocoderGUIDlg::OnOK() 
{
	m_listenAfterVocode = FALSE;
	DoVocode();
}

void CVocoderGUIDlg::DoVocode()
{
	UpdateData();

	CWinApp* app = AfxGetApp();
	app->WriteProfileString(PROFILE_SECTION, MODULATOR_FILE_PROFILE_ENTRY, m_modulatorFile);
	app->WriteProfileString(PROFILE_SECTION, CARRIER_FILE_PROFILE_ENTRY, m_carrierFile);
	app->WriteProfileString(PROFILE_SECTION, OUTPUT_FILE_PROFILE_ENTRY, m_outputFile);
	app->WriteProfileInt(PROFILE_SECTION, WINDOW_LENGTH_PROFILE_ENTRY, m_windowLength);
	app->WriteProfileInt(PROFILE_SECTION, WINDOW_OVERLAP_PROFILE_ENTRY, m_windowOverlap);
	app->WriteProfileInt(PROFILE_SECTION, BAND_COUNT_PROFILE_ENTRY, m_bandCount);
	app->WriteProfileInt(PROFILE_SECTION, OUTPUT_VOLUME_PROFILE_ENTRY, m_outputVolume);
	app->WriteProfileInt(PROFILE_SECTION, NORMALIZE_PROFILE_ENTRY, m_normalize);

	char* error = NULL;
	if (m_modulatorFile.IsEmpty())
	{
		error = "Modulator file not specified.";
	}
	else if (m_carrierFile.IsEmpty()) 
	{
		error = "Carrier file not specified.";
	}
	else if (m_outputFile.IsEmpty())
	{
		error = "Output file not specified.";
	}
	if (error != NULL)
	{
		AfxMessageBox(error);
	}
	else
	{
		_vocoderDlg = this;
		if (setjmp(_vocodeErrorJmpBuf) == 0)
		{
			vocode_start_status_cb = _StartStatusCB;
			vocode_update_status_cb = _UpdateStatusCB;
			vocode_finish_status_cb = _FinishStatusCB;
			error_display_cb = _ErrorDisplayCB;
			vocode_modulator_filename = m_modulatorFile.GetBuffer(0);
			vocode_carrier_filename = m_carrierFile.GetBuffer(0);
			vocode_output_filename = m_outputFile.GetBuffer(0);
			vocode_normalize = m_normalize;
			vocode_volume = m_outputVolume / (float)ONE_OUTPUT_VOLUME;
			vocode_band_count = m_bandCount;
			vocode_open_files();
			vocode_window_length = ipow(2, ilog2(vocode_modulator_rate * m_windowLength / ONE_SECOND_WINDOW_LENGTH));
			if (vocode_window_length < 2)
			{
				vocode_window_length = 2;
			}
			vocode_window_overlap = vocode_window_length * m_windowOverlap / WHOLE_WINDOW_OVERLAP;
			if (vocode_band_count > vocode_window_length / 2)
			{
				vocode_band_count = vocode_window_length / 2;
			}
			CWnd* pButton = GetDlgItem(IDOK);
			pButton->ModifyStyle(0, WS_DISABLED);
			pButton->Invalidate();
			pButton = GetDlgItem(IDC_VOCODE_LISTEN_BUTTON);
			pButton->ModifyStyle(0, WS_DISABLED);
			pButton->Invalidate();
			pButton = GetDlgItem(IDC_STOP_BUTTON);
			pButton->ModifyStyle(WS_DISABLED, 0);
			pButton->Invalidate();
			m_stopVocode = FALSE;
			AfxBeginThread(_VocodeThread, NULL);
		}
		else
		{
			vocode_cleanup();
		}
	}
}

void CVocoderGUIDlg::OnCustomdrawWindowLengthSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UpdateSliderLabels();
	*pResult = 0;
}

void CVocoderGUIDlg::OnCustomdrawWindowOverlapSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UpdateSliderLabels();
	*pResult = 0;
}

void CVocoderGUIDlg::OnCustomdrawBandCountSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UpdateSliderLabels();
	*pResult = 0;
}

void CVocoderGUIDlg::OnCustomdrawOutputVolumeSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	UpdateSliderLabels();
	*pResult = 0;
}

void CVocoderGUIDlg::StartStatus(int num_frames)
{
	CProgressCtrl* pProgress = (CProgressCtrl*)GetDlgItem(IDC_VOCODE_PROGRESS);
	pProgress->SetRange32(0, num_frames - 1);
	pProgress->SetPos(0);
}

BOOL CVocoderGUIDlg::UpdateStatus(int frame_no)
{
	CProgressCtrl* pProgress = (CProgressCtrl*)GetDlgItem(IDC_VOCODE_PROGRESS);
	pProgress->SetPos(frame_no);
	return m_stopVocode;
}

void CVocoderGUIDlg::VocodeFinished()
{
	CWnd* pButton = GetDlgItem(IDOK);
	pButton->ModifyStyle(WS_DISABLED, 0);
	pButton->Invalidate();
	pButton = GetDlgItem(IDC_VOCODE_LISTEN_BUTTON);
	pButton->ModifyStyle(WS_DISABLED, 0);
	pButton->Invalidate();
	pButton = GetDlgItem(IDC_STOP_BUTTON);
	pButton->ModifyStyle(0, WS_DISABLED);
	pButton->Invalidate();
	UpdatePlayButtonsEnabled();
	if (m_listenAfterVocode && !m_stopVocode)
	{
		PlaySound(m_outputFile, NULL, SND_ASYNC);
	}
}

void CVocoderGUIDlg::OnStopButton() 
{
	m_stopVocode = TRUE;
}

void CVocoderGUIDlg::OnModulatorPlayButton() 
{
	UpdateData();
	PlaySound(m_modulatorFile, NULL, SND_ASYNC);
}

void CVocoderGUIDlg::OnCarrierPlayButton() 
{
	UpdateData();
	PlaySound(m_carrierFile, NULL, SND_ASYNC);
}

void CVocoderGUIDlg::OnOutputPlayButton() 
{
	UpdateData();
	PlaySound(m_outputFile, NULL, SND_ASYNC);
}

void CVocoderGUIDlg::OnChangeModulatorFileEdit() 
{
	UpdatePlayButtonsEnabled();
}

void CVocoderGUIDlg::OnChangeCarrierFileEdit() 
{
	UpdatePlayButtonsEnabled();
}

void CVocoderGUIDlg::OnChangeOutputFileEdit() 
{
	UpdatePlayButtonsEnabled();
}

void CVocoderGUIDlg::UpdatePlayButtonEnabled(int nEditCntl, int nPlayCntl)
{
	CString fileName;
	GetDlgItem(nEditCntl)->GetWindowText(fileName);
	CButton* pButton = (CButton*)GetDlgItem(nPlayCntl);
	TRY
	{
		CFile file(fileName, CFile::modeRead);
		pButton->ModifyStyle(WS_DISABLED, 0);
	}
	CATCH (CFileException, e)
	{
		pButton->ModifyStyle(0, WS_DISABLED);
	}
	END_CATCH
	pButton->Invalidate();
}

void CVocoderGUIDlg::OnRestoreDefaultsButton() 
{
	UpdateData();
	m_windowLength = DEFAULT_WINDOW_LENGTH;
	m_windowOverlap = DEFAULT_WINDOW_OVERLAP;
	m_bandCount = DEFAULT_BAND_COUNT;
	m_outputVolume = DEFAULT_OUTPUT_VOLUME;
	m_normalize = DEFAULT_NORMALIZE;
	UpdateData(FALSE);
	UpdateSliderLabels();	
}

void CVocoderGUIDlg::OnVocodeListenButton() 
{
	m_listenAfterVocode = TRUE;
	DoVocode();	
}

void CVocoderGUIDlg::UpdateSliderLabels()
{
	UpdateSliderValueLabel(IDC_WINDOW_LENGTH_SLIDER, IDC_WINDOW_LENGTH_VALUE_LABEL,
		ONE_SECOND_WINDOW_LENGTH, "s");
	UpdateSliderValueLabel(IDC_WINDOW_OVERLAP_SLIDER, IDC_WINDOW_OVERLAP_VALUE_LABEL,
		WHOLE_WINDOW_OVERLAP / 100, "%");
	UpdateSliderValueLabel(IDC_BAND_COUNT_SLIDER, IDC_BAND_COUNT_VALUE_LABEL);	
	UpdateSliderValueLabel(IDC_OUTPUT_VOLUME_SLIDER, IDC_OUTPUT_VOLUME_VALUE_LABEL,
		ONE_OUTPUT_VOLUME / 100, "%", -ONE_OUTPUT_VOLUME);
}

void CVocoderGUIDlg::UpdateSliderValueLabel(int nSliderId, int nLabelId, int nOneValue, char *pszUnits, int nOffset)
{
	CSliderCtrl* pSlider = (CSliderCtrl*)GetDlgItem(nSliderId);
	CStatic* pLabel = (CStatic*)GetDlgItem(nLabelId);
	ostringstream ss;
	ss << (pSlider->GetPos() + nOffset) / (float)nOneValue;
	if (pszUnits != NULL)
	{
		ss << pszUnits;
	}
	pLabel->SetWindowText(ss.str().c_str());
}
