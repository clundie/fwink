# Microsoft Developer Studio Project File - Name="fwink" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=fwink - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fwink.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fwink.mak" CFG="fwink - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fwink - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "fwink - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fwink - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O1 /I "C:\dx90sdk\Samples\C++\DirectShow\BaseClasses" /D "NDEBUG" /D _WIN32_WINNT=0x400 /D "UNICODE" /D "_UNICODE" /D _WIN32_WINNT=0x501 /D WINVER=0x0501 /D _WIN32_WINDOWS=0x410 /D _WIN32_IE=0x0501 /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1009 /d "NDEBUG"
# ADD RSC /l 0x1009 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 unicows.lib strmbase.lib ole32.lib rasapi32.lib comctl32.lib wininet.lib gdiplus.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib uuid.lib strmiids.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"kernel32.lib" /nodefaultlib:"advapi32.lib" /nodefaultlib:"user32.lib" /nodefaultlib:"gdi32.lib" /nodefaultlib:"shell32.lib" /nodefaultlib:"comdlg32.lib" /nodefaultlib:"version.lib" /nodefaultlib:"mpr.lib" /nodefaultlib:"rasapi32.lib" /nodefaultlib:"winmm.lib" /nodefaultlib:"winspool.lib" /nodefaultlib:"vfw32.lib" /nodefaultlib:"secur32.lib" /nodefaultlib:"oleacc.lib" /nodefaultlib:"oledlg.lib" /nodefaultlib:"sensapi.lib" /libpath:"C:\dx90sdk\Samples\C++\DirectShow\BaseClasses\Release_Unicode"

!ELSEIF  "$(CFG)" == "fwink - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "C:\dx90sdk\Samples\C++\DirectShow\BaseClasses" /D "_DEBUG" /D "UNICODE" /D "_UNICODE" /D _WIN32_WINNT=0x501 /D WINVER=0x0501 /D _WIN32_WINDOWS=0x410 /D _WIN32_IE=0x0501 /D "WIN32_LEAN_AND_MEAN" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /Fr /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1009 /d "_DEBUG"
# ADD RSC /l 0x1009 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 strmbasd.lib winmm.lib ole32.lib rasapi32.lib comctl32.lib wininet.lib gdiplus.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib uuid.lib strmiids.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"C:\dx90sdk\Samples\C++\DirectShow\BaseClasses\Debug_Unicode"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "fwink - Win32 Release"
# Name "fwink - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\crossbar.cpp
# End Source File
# Begin Source File

SOURCE=.\dllversion.c
# End Source File
# Begin Source File

SOURCE=.\effectsDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ftpDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\internet.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\mainDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\overlayDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\settings.cpp
# End Source File
# Begin Source File

SOURCE=.\video.cpp
# End Source File
# Begin Source File

SOURCE=.\videoDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\WebCam.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\crossbar.h
# End Source File
# Begin Source File

SOURCE=.\dllversion.h
# End Source File
# Begin Source File

SOURCE=.\overlayDialog.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\WebCam.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\fwink.exe.manifest
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=".\overlay-preview.bmp"
# End Source File
# Begin Source File

SOURCE=.\overlaypreview.bmp
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# Begin Source File

SOURCE=.\think_32_rle.bmp
# End Source File
# End Group
# End Target
# End Project
