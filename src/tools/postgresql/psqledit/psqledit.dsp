# Microsoft Developer Studio Project File - Name="psqledit" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=psqledit - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "psqledit.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "psqledit.mak" CFG="psqledit - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "psqledit - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "psqledit - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "psqledit - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../../exe/Release/psqledit"
# PROP Intermediate_Dir "../../../../obj/Release/psqledit"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /O2 /I "../../../libs/ostrutil" /I "../../../libs/pglib" /I "../../../libs/octrllib" /I "../../../libs/ofileutil" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 imm32.lib version.lib ostrutil.lib octrllib.lib ofileutil.lib pglib.lib ws2_32.lib /nologo /base:"0x500000" /entry:"wWinMainCRTStartup" /subsystem:windows /machine:I386 /libpath:"../../../../lib/release"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=mkdir ..\..\..\..\exe\Release\psqledit\data	copy data\*.txt          ..\..\..\..\exe\Release\psqledit\data	mkdir   ..\..\..\..\exe\Release\psqledit\doc	copy doc\*.txt ..\..\..\..\exe\Release\psqledit\doc
# End Special Build Tool

!ELSEIF  "$(CFG)" == "psqledit - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../exe/Debug/psqledit"
# PROP Intermediate_Dir "../../../../obj/Debug/psqledit"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /ZI /Od /I "../../../libs/ostrutil" /I "../../../libs/pglib" /I "../../../libs/octrllib" /I "../../../libs/ofileutil" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 imm32.lib version.lib ostrutil.lib octrllib.lib ofileutil.lib pglib.lib ws2_32.lib /nologo /entry:"wWinMainCRTStartup" /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"../../../../lib/debug"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=mkdir ..\..\..\..\exe\Debug\psqledit\data	copy data\*.txt          ..\..\..\..\exe\Debug\psqledit\data	mkdir   ..\..\..\..\exe\Debug\psqledit\doc	copy doc\*.txt ..\..\..\..\exe\Debug\psqledit\doc
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "psqledit - Win32 Release"
# Name "psqledit - Win32 Debug"
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

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\octrllib\ComboBoxUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\common.cpp
# End Source File
# Begin Source File

SOURCE=.\completion_util.cpp
# End Source File
# Begin Source File

SOURCE=.\ConnectInfoDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ConnectList.cpp
# End Source File
# Begin Source File

SOURCE=.\DownloadDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorOptionPage.cpp
# End Source File
# Begin Source File

SOURCE=.\EditorOptionPage2.cpp
# End Source File
# Begin Source File

SOURCE=.\FuncOptionPage.cpp
# End Source File
# Begin Source File

SOURCE=.\FuncOptionPage2.cpp
# End Source File
# Begin Source File

SOURCE=.\getsrc.cpp
# End Source File
# Begin Source File

SOURCE=.\getsrc2.cpp
# End Source File
# Begin Source File

SOURCE=.\GridOptionPage.cpp
# End Source File
# Begin Source File

SOURCE=.\GridSortDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\GridView.cpp
# End Source File
# Begin Source File

SOURCE=.\LineJumpDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\lintsql.cpp
# End Source File
# Begin Source File

SOURCE=.\localsql.cpp
# End Source File
# Begin Source File

SOURCE=.\LogEditData.cpp
# End Source File
# Begin Source File

SOURCE=.\LoginDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectBar.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectDetailBar.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionSheet.cpp
# End Source File
# Begin Source File

SOURCE=.\pggriddata.cpp
# End Source File
# Begin Source File

SOURCE=.\PrintDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\psql_util.cpp
# End Source File
# Begin Source File

SOURCE=.\psqledit.cpp
# End Source File
# Begin Source File

SOURCE=.\psqledit.rc
# End Source File
# Begin Source File

SOURCE=.\PSqlEditCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\psqleditDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\QueryCloseDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ReplaceDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\scr_pass.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\libs\octrllib\ScrollWnd.cpp
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

SOURCE=.\shortcutsql.cpp
# End Source File
# Begin Source File

SOURCE=.\ShortCutSqlDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ShortCutSqlListDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sizecbar.cpp
# End Source File
# Begin Source File

SOURCE=.\SQLEditView.cpp
# End Source File
# Begin Source File

SOURCE=.\SQLExplainView.cpp
# End Source File
# Begin Source File

SOURCE=.\SqlLibraryDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SqlListMaker.cpp
# End Source File
# Begin Source File

SOURCE=.\sqllog.cpp
# End Source File
# Begin Source File

SOURCE=.\SqlLogDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SqlLogFileDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\sqlparse.cpp
# End Source File
# Begin Source File

SOURCE=.\sqlparser.cpp
# End Source File
# Begin Source File

SOURCE=.\sqlstrtoken.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\TabBar.cpp
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

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\command_list.h
# End Source File
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\completion_util.h
# End Source File
# Begin Source File

SOURCE=.\ConnectInfoDlg.h
# End Source File
# Begin Source File

SOURCE=.\ConnectList.h
# End Source File
# Begin Source File

SOURCE=.\default_accel.h
# End Source File
# Begin Source File

SOURCE=.\DownloadDlg.h
# End Source File
# Begin Source File

SOURCE=.\EditorOptionPage.h
# End Source File
# Begin Source File

SOURCE=.\EditorOptionPage2.h
# End Source File
# Begin Source File

SOURCE=.\FuncOptionPage.h
# End Source File
# Begin Source File

SOURCE=.\FuncOptionPage2.h
# End Source File
# Begin Source File

SOURCE=.\getsrc.h
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

SOURCE=.\GridView.h
# End Source File
# Begin Source File

SOURCE=.\LineJumpDlg.h
# End Source File
# Begin Source File

SOURCE=.\lintsql.h
# End Source File
# Begin Source File

SOURCE=.\localsql.h
# End Source File
# Begin Source File

SOURCE=.\LogEditData.h
# End Source File
# Begin Source File

SOURCE=.\LoginDlg.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\ObjectBar.h
# End Source File
# Begin Source File

SOURCE=.\ObjectDetailBar.h
# End Source File
# Begin Source File

SOURCE=.\OptionSheet.h
# End Source File
# Begin Source File

SOURCE=.\pggriddata.h
# End Source File
# Begin Source File

SOURCE=.\PrintDlg.h
# End Source File
# Begin Source File

SOURCE=.\psql_util.h
# End Source File
# Begin Source File

SOURCE=.\psqledit.h
# End Source File
# Begin Source File

SOURCE=.\PSqlEditCtrl.h
# End Source File
# Begin Source File

SOURCE=.\psqleditDoc.h
# End Source File
# Begin Source File

SOURCE=.\QueryCloseDlg.h
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

SOURCE=.\shortcutsql.h
# End Source File
# Begin Source File

SOURCE=.\ShortCutSqlDlg.h
# End Source File
# Begin Source File

SOURCE=.\ShortCutSqlListDlg.h
# End Source File
# Begin Source File

SOURCE=.\sizecbar.h
# End Source File
# Begin Source File

SOURCE=.\SQLEditView.h
# End Source File
# Begin Source File

SOURCE=.\SQLExplainView.h
# End Source File
# Begin Source File

SOURCE=.\SqlLibraryDlg.h
# End Source File
# Begin Source File

SOURCE=.\SqlListMaker.h
# End Source File
# Begin Source File

SOURCE=.\sqllog.h
# End Source File
# Begin Source File

SOURCE=.\SqlLogDlg.h
# End Source File
# Begin Source File

SOURCE=.\SqlLogFileDlg.h
# End Source File
# Begin Source File

SOURCE=.\sqlparse.h
# End Source File
# Begin Source File

SOURCE=.\sqlparser.h
# End Source File
# Begin Source File

SOURCE=.\sqlstrtoken.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TabBar.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\folder1.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\psqledit.ico
# End Source File
# Begin Source File

SOURCE=.\res\psqledit.rc2
# End Source File
# Begin Source File

SOURCE=.\res\psqleditDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\sqltune.ico
# End Source File
# Begin Source File

SOURCE=.\res\sqltuneDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\changelog.txt
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
