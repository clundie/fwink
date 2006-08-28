#----- Include the PSDK's WIN32.MAK to pick up defines-------------------
!include <win32.mak>

LINK32_OBJS= \
        $(OUTDIR)\amextra.obj       \
        $(OUTDIR)\amfilter.obj      \
        $(OUTDIR)\amvideo.obj       \
        $(OUTDIR)\combase.obj       \
        $(OUTDIR)\cprop.obj         \
        $(OUTDIR)\ctlutil.obj       \
        $(OUTDIR)\ddmm.obj          \
        $(OUTDIR)\dllentry.obj      \
        $(OUTDIR)\dllsetup.obj      \
        $(OUTDIR)\mtype.obj         \
        $(OUTDIR)\outputq.obj       \
        $(OUTDIR)\pstream.obj       \
        $(OUTDIR)\pullpin.obj       \
        $(OUTDIR)\refclock.obj      \
        $(OUTDIR)\renbase.obj       \
        $(OUTDIR)\schedule.obj      \
        $(OUTDIR)\seekpt.obj        \
        $(OUTDIR)\source.obj        \
        $(OUTDIR)\strmctl.obj       \
        $(OUTDIR)\sysclock.obj      \
        $(OUTDIR)\transfrm.obj      \
        $(OUTDIR)\transip.obj       \
        $(OUTDIR)\videoctl.obj      \
        $(OUTDIR)\vtrans.obj        \
        $(OUTDIR)\winctrl.obj       \
        $(OUTDIR)\winutil.obj       \
        $(OUTDIR)\wxdebug.obj       \
        $(OUTDIR)\wxlist.obj        \
        $(OUTDIR)\wxutil.obj      

!IFDEF NODEBUG
STRMBASE=strmbase
!else
STRMBASE=strmbasd
!endif


all:	$(OUTDIR) $(OUTDIR)\$(STRMBASE).lib

$(OUTDIR)\$(STRMBASE).lib:   $(LINK32_OBJS)
	$(link) -lib /OUT:$(OUTDIR)\$(STRMBASE).lib /MACHINE:$(CPU) /NODEFAULTLIB $(LINK32_OBJS) strmiids.lib

$(OUTDIR)\amextra.obj:
	$(cc) $(cflags) $(cdebug) $(cvarsmt) /EHsc /Yc"streams.h" /Fp$(OUTDIR)\streams.pch /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D_UNICODE /DUNICODE /I "." /I"$(DXSDK_DIR)\Include" /D_LIB /D_WIN32_DCOM  amextra.cpp

.cpp{$(OUTDIR)}.obj:
	$(cc) $(cflags) $(cdebug) $(cvarsmt) /EHsc /Yu"streams.h" /Fp$(OUTDIR)\streams.pch /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" /D_UNICODE /DUNICODE /I "." /I"$(DXSDK_DIR)\Include" /D_LIB /D_WIN32_DCOM  $**

#----- If OUTDIR does not exist, then create directory
$(OUTDIR) :
    if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

clean:
        $(CLEANUP)
