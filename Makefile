CC     = icc.exe
RC     = rc.exe
LINK   = ilink.exe
CFLAGS = /Gm /Ss /Q
LFLAGS = /NOE /PMTYPE:PM /NOLOGO
RFLAGS = -n
NAME   = context
LIBS   = libconv.lib libuls.lib


$(NAME).exe : $(NAME).obj $(NAME).res $(NAME).h
                $(LINK) $(LFLAGS) $(LIBS) $(NAME).obj
                $(RC) $(RFLAGS) $(NAME).res $@

$(NAME).res : resource.rc resource.dlg resource.h
                $(RC) -r resource.rc $@


clean       :
              @if exist $(NAME).exe del $(NAME).exe
              @if exist $(NAME).res del $(NAME).res
              @if exist $(NAME).obj del $(NAME).obj

