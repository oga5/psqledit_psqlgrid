# Microsoft Developer Studio Project File - Name="psqlgrid" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=psqlgrid - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "psqlgrid.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "psqlgrid.mak" CFG="psqlgrid - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "psqlgrid - Win32 Release" ("Win32 (x86) Application" 用)
!MESSAGE "psqlgrid - Win32 Debug" ("Win32 (x86) Application" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "psqlgrid - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../../exe/Release/psqledit"
# PROP Intermediate_Dir "../../../../obj/Release/psqlgrid"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../../../libs/ostrutil" /I "../../../libs/pglib" /I "../../../libs/octrllib" /I "../../../libs/ofileutil" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 version.lib imm32.lib ostrutil.lib octrllib.lib pglib.lib ws2_32.lib ofileutil.lib /nologo /base:"0x500000" /entry:"wWinMainCRTStartup" /subsystem:windows /machine:I386 /libpath:"../../../../lib/release"

!ELSEIF  "$(CFG)" == "psqlgrid - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../exe/Debug/psqledit"
# PROP Intermediate_Dir "../../../../obj/Debug/psqlgrid"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../../../libs/ostrutil" /I "../../../libs/pglib" /I "../../../libs/octrllib" /I "../../../libs/ofileutil" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 version.lib imm32.lib ofileutil.lib ostrutil.lib octrllib.lib pglib.lib ws2_32.lib /nologo /base:"0x500000" /entry:"wWinMainCRTStartup" /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"../../../../lib/debug"

!ENDIF 

# Begin Target

# Name "psqlgrid - Win32 Release"
# Name "psqlgrid - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AboutDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AcceleratorDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\AccelList.cpp
# End Source File
# Begin Source File

SOURCE=.\CancelDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ConnectInfoDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ConnectList.cpp
# End Source File
# Begin Source File

SOURCE=.\DataEditDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\EditablePgGridData.cpp
# End Source File
# Begin Source File

SOURCE=.\GridOptionPage.cpp
# End Source File
# Begin Source File

SOURCE=.\GridSortDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\InputSequenceDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\InsertRowsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\LoginDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\psqlgrid.cpp
# End Source File
# Begin Source File

SOURCE=.\psqlgrid.rc
# End Source File
# Begin Source File

SOURCE=.\psqlgridDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\psqlgridView.cpp
# End Source File
# Begin Source File

SOURCE=.\ReplaceDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\scr_pass.cpp
# End Source File
# Begin Source File

SOURCE=.\SearchDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SetupPage.cpp
# End Source File
# Begin Source File

SOURCE=.\ShortCutKeyDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sqlstrtoken.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\TableListDlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AboutDlg.h
# End Source File
# Begin Source File

SOURCE=.\AcceleratorDlg.h
# End Source File
# Begin Source File

SOURCE=.\AccelList.h
# End Source File
# Begin Source File

SOURCE=.\CancelDlg.h
# End Source File
# Begin Source File

SOURCE=.\command_list.h
# End Source File
# Begin Source File

SOURCE=.\ConnectInfoDlg.h
# End Source File
# Begin Source File

SOURCE=.\ConnectList.h
# End Source File
# Begin Source File

SOURCE=.\DataEditDlg.h
# End Source File
# Begin Source File

SOURCE=.\default_accel.h
# End Source File
# Begin Source File

SOURCE=.\EditablePgGridData.h
# End Source File
# Begin Source File

SOURCE=.\global.h
# End Source File
# Begin Source File

SOURCE=.\GridOptionPage.h
# End Source File
# Begin Source File

SOURCE=.\GridSortDlg.h
# End Source File
# Begin Source File

SOURCE=.\InputSequenceDlg.h
# End Source File
# Begin Source File

SOURCE=.\InsertRowsDlg.h
# End Source File
# Begin Source File

SOURCE=.\LoginDlg.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\OptionSheet.h
# End Source File
# Begin Source File

SOURCE=.\ProfNameFolder.h
# End Source File
# Begin Source File

SOURCE=.\psqlgrid.h
# End Source File
# Begin Source File

SOURCE=.\psqlgridDoc.h
# End Source File
# Begin Source File

SOURCE=.\psqlgridView.h
# End Source File
# Begin Source File

SOURCE=.\ReplaceDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\scr_pass.h
# End Source File
# Begin Source File

SOURCE=.\SearchDlg.h
# End Source File
# Begin Source File

SOURCE=.\SearchDlgData.h
# End Source File
# Begin Source File

SOURCE=.\SetupPage.h
# End Source File
# Begin Source File

SOURCE=.\ShortCutKeyDlg.h
# End Source File
# Begin Source File

SOURCE=.\sqlstrtoken.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TableListDlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ogrid.ico
# End Source File
# Begin Source File

SOURCE=.\res\psqlgrid.ico
# End Source File
# Begin Source File

SOURCE=.\res\psqlgrid.rc2
# End Source File
# Begin Source File

SOURCE=.\res\psqlgridDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Changelog.txt
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
