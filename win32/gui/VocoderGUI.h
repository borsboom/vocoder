// VocoderGUI.h : main header file for the VOCODERGUI application
//

#if !defined(AFX_VOCODERGUI_H__B2E4805C_F3FF_401D_91BF_4874C34BF245__INCLUDED_)
#define AFX_VOCODERGUI_H__B2E4805C_F3FF_401D_91BF_4874C34BF245__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CVocoderGUIApp:
// See VocoderGUI.cpp for the implementation of this class
//

class CVocoderGUIApp : public CWinApp
{
public:
	CVocoderGUIApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVocoderGUIApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CVocoderGUIApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VOCODERGUI_H__B2E4805C_F3FF_401D_91BF_4874C34BF245__INCLUDED_)
