@echo off
REM -- First make map file from Microsoft Visual C++ generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by VOCODERGUI.HPJ. >"hlp\VocoderGUI.hm"
echo. >>"hlp\VocoderGUI.hm"
echo // Commands (ID_* and IDM_*) >>"hlp\VocoderGUI.hm"
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>"hlp\VocoderGUI.hm"
echo. >>"hlp\VocoderGUI.hm"
echo // Prompts (IDP_*) >>"hlp\VocoderGUI.hm"
makehm IDP_,HIDP_,0x30000 resource.h >>"hlp\VocoderGUI.hm"
echo. >>"hlp\VocoderGUI.hm"
echo // Resources (IDR_*) >>"hlp\VocoderGUI.hm"
makehm IDR_,HIDR_,0x20000 resource.h >>"hlp\VocoderGUI.hm"
echo. >>"hlp\VocoderGUI.hm"
echo // Dialogs (IDD_*) >>"hlp\VocoderGUI.hm"
makehm IDD_,HIDD_,0x20000 resource.h >>"hlp\VocoderGUI.hm"
echo. >>"hlp\VocoderGUI.hm"
echo // Frame Controls (IDW_*) >>"hlp\VocoderGUI.hm"
makehm IDW_,HIDW_,0x50000 resource.h >>"hlp\VocoderGUI.hm"
REM -- Make help for Project VOCODERGUI


echo Building Win32 Help files
start /wait hcw /C /E /M "hlp\VocoderGUI.hpj"
if errorlevel 1 goto :Error
if not exist "hlp\VocoderGUI.hlp" goto :Error
if not exist "hlp\VocoderGUI.cnt" goto :Error
echo.
if exist Debug\nul copy "hlp\VocoderGUI.hlp" Debug
if exist Debug\nul copy "hlp\VocoderGUI.cnt" Debug
if exist Release\nul copy "hlp\VocoderGUI.hlp" Release
if exist Release\nul copy "hlp\VocoderGUI.cnt" Release
echo.
goto :done

:Error
echo hlp\VocoderGUI.hpj(1) : error: Problem encountered creating help file

:done
echo.
