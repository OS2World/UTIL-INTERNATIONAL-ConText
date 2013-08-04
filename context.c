/* ------------------------------------------------------------------------- *
 * (C) 2006 Alex Taylor                                                      *
 *                                                                           *
 * Redistribution and use in source and binary forms, with or without        *
 * modification, are permitted provided that the following conditions are    *
 * met:                                                                      *
 *                                                                           *
 *  1. Redistributions of source code must retain the above copyright        *
 *     notice, this list of conditions and the following disclaimer.         *
 *  2. Redistributions in binary form must reproduce the above copyright     *
 *     notice, this list of conditions and the following disclaimer in the   *
 *     documentation and/or other materials provided with the distribution.  *
 *  3. The name of the author may not be used to endorse or promote products *
 *     derived from this software without specific prior written permission. *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR      *
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES *
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.   *
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,          *
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT  *
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, *
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY     *
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT       *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF  *
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.         *
 * ------------------------------------------------------------------------- */
#include "context.h"

// ----------------------------------------------------------------------------
// GLOBALS

CTGLOBAL global;            // global parameters
UCHAR    szDebug[ 512 ];    // used for generating debug output


/* ------------------------------------------------------------------------- *
 * main program                                                              *
 * ------------------------------------------------------------------------- */
int main( void )
{
    QMSG     qmsg;                      // message queue
    CHAR     szClass[] = "ConText";     // window class name
    HWND     hwndFrame;                // handle to the client window area
    HATOMTBL hSATbl;                    // handle to system atom table
    ULONG    flStyle = FCF_STANDARD;


    global.hab = WinInitialize( 0 );
    global.hmq = WinCreateMsgQueue( global.hab, 0 );

    global.fPasteUni       = TRUE;
    global.fPasteAsCurrent = FALSE;
    global.fCopyUniSource  = TRUE;
    global.fCopyUniTarget  = TRUE;

    if ( ! WinRegisterClass( global.hab, szClass, ClientWndProc, CS_SIZEREDRAW, 0 )) return ( 1 );
    hwndFrame = WinCreateStdWindow( HWND_DESKTOP, WS_VISIBLE, &flStyle, szClass,
                                    "Convert Text Encoding",
                                     0L, NULLHANDLE, ID_MAINPROGRAM, &(global.hwndClient) );
    if ( ! hwndFrame ) return ( 1 );


    // Set up the window contents
    WinSetPresParam( global.hwndClient, PP_FONTNAMESIZE, strlen("9.WarpSans")+1, "9.WarpSans");
    CreateControls( global.hwndClient );

    // Populate the codepage lists
    PopulateCodepages( global.hwndClient );

    // Make sure the Unicode clipboard format is registered
    hSATbl = WinQuerySystemAtomTable();
    global.cf_UniText = WinAddAtom( hSATbl, "text/unicode");

    while ( WinGetMsg( global.hab, &qmsg, 0, 0, 0 )) WinDispatchMsg( global.hab, &qmsg );

    WinDeleteAtom( hSATbl, global.cf_UniText );

    WinDestroyWindow( hwndFrame );
    WinDestroyMsgQueue( global.hmq );
    WinTerminate( global.hab );

    return ( 0 );
}


/* ------------------------------------------------------------------------- *
 * CreateControls                                                            *
 *                                                                           *
 * Creates the various UI controls on the client window.                     *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwndClient: Handle of the client window.                           *
 *                                                                           *
 * RETURNS: USHORT                                                           *
 *   1 if an error occured, 0 otherwise.                                     *
 * ------------------------------------------------------------------------- */
USHORT CreateControls( HWND hwndClient )
{
    HWND  hwnd;
    LONG  lColor = SYSCLR_DIALOGBACKGROUND;
    RGB   rgb    = {228, 228, 228};
    SHORT sMState;


    // Source text MLE
    global.hwndSource = WinCreateWindow( hwndClient, WC_MLE, "",
                                         MLS_BORDER | MLS_VSCROLL | MLS_WORDWRAP | WS_GROUP,
                                         0, 0, 0, 0, hwndClient, HWND_TOP, IDD_SOURCE, NULL, NULL );
    if ( global.hwndSource == NULLHANDLE ) return ( 1 );
    global.pfnMLEProc1 = WinSubclassWindow( global.hwndSource, InputMLEProc );

    WinSetPresParam( global.hwndSource, PP_FONTNAMESIZE,
                     strlen(SZ_DEFAULTFONT)+1, SZ_DEFAULTFONT );

    // Target text MLE
    global.hwndTarget = WinCreateWindow( hwndClient, WC_MLE, "",
                                         MLS_BORDER | MLS_VSCROLL | MLS_WORDWRAP | MLS_READONLY | WS_GROUP,
                                         0, 0, 0, 0, hwndClient, HWND_TOP, IDD_TARGET, NULL, NULL );
    if ( global.hwndTarget == NULLHANDLE ) return ( 1 );
    global.pfnMLEProc2 = WinSubclassWindow( global.hwndTarget, OutputMLEProc );

    WinSetPresParam( global.hwndTarget, PP_FONTNAMESIZE,
                     strlen(SZ_DEFAULTFONT)+1, SZ_DEFAULTFONT );
    WinSetPresParam( global.hwndTarget, PP_BACKGROUNDCOLOR, sizeof(rgb), &rgb );

    // Context menus
    global.hwndSContext = WinLoadMenu( global.hwndSource, NULLHANDLE, IDM_ICONTEXT );
    global.hwndTContext = WinLoadMenu( global.hwndTarget, NULLHANDLE, IDM_OCONTEXT );

    sMState = global.fPasteUni ? MIA_CHECKED : 0;
    WinSendMsg( global.hwndSContext, MM_SETITEMATTR,
                MPFROM2SHORT( ID_IPASTEUNI, TRUE ), MPFROM2SHORT( MIA_CHECKED, sMState ));
    sMState = global.fPasteAsCurrent ? 0 : MIA_CHECKED;
    WinSendMsg( global.hwndSContext, MM_SETITEMATTR,
                MPFROM2SHORT( ID_IPASTECP, TRUE ), MPFROM2SHORT( MIA_CHECKED, sMState ));
    sMState = global.fCopyUniSource ? MIA_CHECKED : 0;
    WinSendMsg( global.hwndSContext, MM_SETITEMATTR,
                MPFROM2SHORT( ID_ICOPYUNI, TRUE ), MPFROM2SHORT( MIA_CHECKED, sMState ));
    sMState = global.fCopyUniTarget ? MIA_CHECKED : 0;
    WinSendMsg( global.hwndTContext, MM_SETITEMATTR,
                MPFROM2SHORT( ID_OCOPYUNI, TRUE ), MPFROM2SHORT( MIA_CHECKED, sMState ));

    // Text labels
    hwnd = WinCreateWindow( hwndClient, WC_STATIC, "Input text:",
                            SS_TEXT | DT_LEFT | DT_VCENTER,
                            0, 0, 0, 0, hwndClient, HWND_TOP, IDD_SOURCETX, NULL, NULL );
    if ( hwnd == NULLHANDLE ) return ( 1 );
    WinSetPresParam( hwnd, PP_BACKGROUNDCOLORINDEX, sizeof(lColor), &lColor );
    hwnd = WinCreateWindow( hwndClient, WC_STATIC, "Output text:",
                            SS_TEXT | DT_LEFT | DT_VCENTER,
                            0, 0, 0, 0, hwndClient, HWND_TOP, IDD_TARGETTX, NULL, NULL );
    if ( hwnd == NULLHANDLE ) return ( 1 );
    WinSetPresParam( hwnd, PP_BACKGROUNDCOLORINDEX, sizeof(lColor), &lColor );

    // Codepage list comboboxes
    global.hwndSourceCP = WinCreateWindow( hwndClient, WC_COMBOBOX, "",
                                           CBS_DROPDOWNLIST | LS_HORZSCROLL | WS_GROUP,
                                           0, 0, 0, 0, hwndClient, HWND_TOP, IDD_SOURCECP, NULL, NULL );
    if ( global.hwndSourceCP == NULLHANDLE ) return ( 1 );
    global.hwndTargetCP = WinCreateWindow( hwndClient, WC_COMBOBOX, "",
                                           CBS_DROPDOWNLIST | LS_HORZSCROLL | WS_GROUP,
                                           0, 0, 0, 0, hwndClient, HWND_TOP, IDD_TARGETCP, NULL, NULL );
    if ( global.hwndTargetCP == NULLHANDLE ) return ( 1 );

    // Convert button
    global.hwndButton = WinCreateWindow( hwndClient, WC_BUTTON, "~Convert", BS_PUSHBUTTON | WS_GROUP,
                                         0, 0, 0, 0, hwndClient, HWND_TOP, ID_CONVERT, NULL, NULL );
    if ( global.hwndButton == NULLHANDLE ) return ( 1 );

    WinSetFocus( HWND_DESKTOP, WinWindowFromID( global.hwndClient, IDD_SOURCE ));
    return ( 0 );
}


/* ------------------------------------------------------------------------- *
 * ClientWndProc                                                             *
 *                                                                           *
 * Client window procedure.  Refer to the PM documentation for argument and  *
 * return value descriptions.                                                *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY ClientWndProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    CLIPOPTIONS cbopts;     // clipboard options struct
    USHORT usFlags,         // flags from WM_CHAR
           usVK;            // virtual-key value from WM_CHAR
    SHORT  sSourceIdx,      // index position in the source codepage list
           sTargetIdx,      // index position in the target codepage list
           sMState;         // menu attribute state
    ULONG  ulSourceCP,      // selected source codepage
           ulTargetCP;      // selected target codepage
    HWND   hwndMenu,        // main menubar
           hwndFocus,       // current focus window
           hwndNext;        // window to switch focus to


    switch( msg ) {

        case WM_CREATE:
            break;

        case WM_PAINT:
            PaintClient( hwnd );
            break;

        case WM_COMMAND:
            switch( SHORT1FROMMP( mp1 )) {

                case ID_CONVERT:                // Apply the conversion
                    sSourceIdx = (SHORT) WinSendDlgItemMsg( hwnd, IDD_SOURCECP, LM_QUERYSELECTION,
                                                            MPFROMSHORT( LIT_FIRST ), MPFROMLONG( 0 ));
                    if ( sSourceIdx == LIT_NONE ) break;
                    sTargetIdx = (SHORT) WinSendDlgItemMsg( hwnd, IDD_TARGETCP, LM_QUERYSELECTION,
                                                            MPFROMSHORT( LIT_FIRST ), MPFROMLONG( 0 ));
                    if ( sTargetIdx == LIT_NONE ) break;

                    ulSourceCP = global.encodings[ sSourceIdx ].ulCP;
                    ulTargetCP = global.encodings[ sTargetIdx ].ulCP;
                    if (( ulSourceCP > 0 ) && ( ulTargetCP > 0 ))
                        ConvertText( hwnd, ulSourceCP, ulTargetCP );
                    return (MRESULT) 0;

                // EDIT MENU

                case ID_ICUT:
                    WinSendMsg( global.hwndSource, MLM_CUT, 0, 0 );
                    return (MRESULT) 0;

                case ID_ICOPY:
                    WinSendMsg( global.hwndSource, MLM_COPY, 0, 0 );
                    return (MRESULT) 0;

                case ID_IPASTE:
                    WinSendMsg( global.hwndSource, MLM_PASTE, "", 0 );
                    return (MRESULT) 0;

                case ID_OCOPY:
                    WinSendMsg( global.hwndTarget, MLM_COPY, 0, 0 );
                    return (MRESULT) 0;

                // OPTIONS MENU

                case ID_IPASTEUNI:
                    global.fPasteUni = !global.fPasteUni;
                    sMState = global.fPasteUni ? MIA_CHECKED : 0;
                    WinSendMsg( global.hwndSContext, MM_SETITEMATTR,
                                MPFROM2SHORT( ID_IPASTEUNI, TRUE ),
                                MPFROM2SHORT( MIA_CHECKED, sMState ));
                    return (MRESULT) 0;

                case ID_IPASTECP:
                    global.fPasteAsCurrent = !global.fPasteAsCurrent;
                    sMState = global.fPasteAsCurrent ? 0 : MIA_CHECKED;
                    WinSendMsg( global.hwndSContext, MM_SETITEMATTR,
                                MPFROM2SHORT( ID_IPASTECP, TRUE ),
                                MPFROM2SHORT( MIA_CHECKED, sMState ));
                    return (MRESULT) 0;

                case ID_ICOPYUNI:
                    global.fCopyUniSource = !global.fCopyUniSource;
                    sMState = global.fCopyUniSource ? MIA_CHECKED : 0;
                    WinSendMsg( global.hwndSContext, MM_SETITEMATTR,
                                MPFROM2SHORT( ID_ICOPYUNI, TRUE ),
                                MPFROM2SHORT( MIA_CHECKED, sMState ));
                    return (MRESULT) 0;

                case ID_OCOPYUNI:
                    global.fCopyUniTarget = !global.fCopyUniTarget;
                    sMState = global.fCopyUniTarget ? MIA_CHECKED : 0;
                    WinSendMsg( global.hwndTContext, MM_SETITEMATTR,
                                MPFROM2SHORT( ID_OCOPYUNI, TRUE ),
                                MPFROM2SHORT( MIA_CHECKED, sMState ));
                    return (MRESULT) 0;

                case ID_OPTCLIP:                // Clipboard options dialog
                    cbopts.fChanged        = FALSE;
                    cbopts.fPasteUni       = global.fPasteUni;
                    cbopts.fPasteAsCurrent = global.fPasteAsCurrent;
                    cbopts.fCopyUniSource  = global.fCopyUniSource;
                    cbopts.fCopyUniTarget  = global.fCopyUniTarget;
                    WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP) OptionDlgProc, 0, IDD_CLIPBOARD, &cbopts );
                    if ( cbopts.fChanged ) {
                        global.fPasteUni = cbopts.fPasteUni;
                        sMState = global.fPasteUni ? MIA_CHECKED : 0;
                        WinSendMsg( global.hwndSContext, MM_SETITEMATTR,
                                    MPFROM2SHORT( ID_IPASTEUNI, TRUE ),
                                    MPFROM2SHORT( MIA_CHECKED, sMState ));

                        global.fPasteAsCurrent = cbopts.fPasteAsCurrent;
                        sMState = global.fPasteAsCurrent ? 0 : MIA_CHECKED;
                        WinSendMsg( global.hwndSContext, MM_SETITEMATTR,
                                    MPFROM2SHORT( ID_IPASTECP, TRUE ),
                                    MPFROM2SHORT( MIA_CHECKED, sMState ));

                        global.fCopyUniSource  = cbopts.fCopyUniSource;
                        sMState = global.fCopyUniSource ? MIA_CHECKED : 0;
                        WinSendMsg( global.hwndSContext, MM_SETITEMATTR,
                                    MPFROM2SHORT( ID_ICOPYUNI, TRUE ),
                                    MPFROM2SHORT( MIA_CHECKED, sMState ));

                        global.fCopyUniTarget  = cbopts.fCopyUniTarget;
                        sMState = global.fCopyUniTarget ? MIA_CHECKED : 0;
                        WinSendMsg( global.hwndTContext, MM_SETITEMATTR,
                                    MPFROM2SHORT( ID_OCOPYUNI, TRUE ),
                                    MPFROM2SHORT( MIA_CHECKED, sMState ));
                    }
                    break;

                case ID_ABOUT:                  // Product information dialog
                    WinDlgBox( HWND_DESKTOP, hwnd, (PFNWP) AboutDlgProc, 0, IDD_ABOUT, NULL );
                    break;

                case ID_QUIT:                   // Exit the program
                    WinPostMsg( hwnd, WM_CLOSE, 0, 0 );
                    return (MRESULT) 0;

                default: break;
            }
            break;

        case WM_CHAR:

            // Handle tabbing between controls
            usFlags = SHORT1FROMMP( mp1 );
            usVK    = SHORT2FROMMP( mp2 );

            // forwards tab
            if (( usFlags & KC_VIRTUALKEY ) && ( ! (usFlags & KC_KEYUP) ) && ( usVK == VK_TAB ))
            {
                hwndFocus = WinQueryFocus( HWND_DESKTOP );
                if ( WinIsChild( hwndFocus, global.hwndSourceCP ))
                    hwndNext = global.hwndSource;
                else if ( WinIsChild( hwndFocus, global.hwndSource ) && ( usFlags & KC_CTRL ))
                    hwndNext = global.hwndTargetCP;
                else if ( WinIsChild( hwndFocus, global.hwndTargetCP ))
                    hwndNext = global.hwndButton;
                else if ( WinIsChild( hwndFocus, global.hwndButton ))
                    hwndNext = global.hwndTarget;
                else if ( WinIsChild( hwndFocus, global.hwndTarget ))
                    hwndNext = global.hwndSourceCP;
                else break;

                WinSetFocus( HWND_DESKTOP, hwndNext );
                return (MRESULT) TRUE;
            }
            // backwards tab
            if (( usFlags & KC_VIRTUALKEY ) && ( ! (usFlags & KC_KEYUP) ) && ( usVK == VK_BACKTAB ))
            {
                hwndFocus = WinQueryFocus( HWND_DESKTOP );
                if ( WinIsChild( hwndFocus, global.hwndSourceCP ))
                    hwndNext = global.hwndTarget;
                else if ( WinIsChild( hwndFocus, global.hwndSource ))
                    hwndNext = global.hwndSourceCP;
                else if ( WinIsChild( hwndFocus, global.hwndTargetCP ))
                    hwndNext = global.hwndSource;
                else if ( WinIsChild( hwndFocus, global.hwndButton ))
                    hwndNext = global.hwndTargetCP;
                else if ( WinIsChild( hwndFocus, global.hwndTarget ))
                    hwndNext = global.hwndButton;
                else break;

                WinSetFocus( HWND_DESKTOP, hwndNext );
                return (MRESULT) TRUE;
            }
            break;

    }

    return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * InputMLEProc                                                              *
 *                                                                           *
 * A subclass window procedure for the input (source) MLE control.  We use   *
 * this to intercept the default clipboard keyboard triggers, so that we can *
 * call our own functions for handling Unicode text.                         *
 *                                                                           *
 * Refer to the PM documentation for argument and return value descriptions. *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY InputMLEProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    USHORT  usFlags,
            usVK,
            usX, usY;
    ULONG   ulCopied = 0;
    POINTL  pts;


    switch( msg ) {

        case WM_CHAR:
            usFlags = SHORT1FROMMP( mp1 );
            usVK    = SHORT2FROMMP( mp2 );
            if (( usFlags & KC_VIRTUALKEY ) && ( ! (usFlags & KC_KEYUP) )) {
                // Shift+Ins (paste)
                if (( usFlags & KC_SHIFT ) && ( usVK == VK_INSERT )) {
                    ulCopied = DoPaste();
                    return (MRESULT) TRUE;
                }
                // Ctrl+Ins (copy)
                if (( usFlags & KC_CTRL ) && ( usVK == VK_INSERT )) {
                    ulCopied = DoCopyCut( hwnd, global.fCopyUniSource, FALSE );
                    return (MRESULT) TRUE;
                }
                // Shift+Del (cut)
                if (( usFlags & KC_SHIFT ) && ( usVK == VK_DELETE )) {
                    ulCopied = DoCopyCut( hwnd, global.fCopyUniSource, TRUE );
                    return (MRESULT) TRUE;
                }
            }
            break;

        case WM_CONTEXTMENU:
            WinQueryPointerPos( HWND_DESKTOP, &pts );
            WinMapWindowPoints( HWND_DESKTOP, hwnd, &pts, 1 );
            WinPopupMenu( hwnd, global.hwndClient, global.hwndSContext, pts.x, pts.y,
                          0, PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1 );
            return (MRESULT) TRUE;

        case MLM_CUT:
            ulCopied = DoCopyCut( hwnd, global.fCopyUniSource, TRUE );
            return (MRESULT) ulCopied;

        case MLM_COPY:
            ulCopied = DoCopyCut( hwnd, global.fCopyUniSource, FALSE );
            return (MRESULT) ulCopied;

        case MLM_PASTE:
            ulCopied = DoPaste();
            return (MRESULT) ulCopied;

        default: break;
    }

    return (MRESULT) global.pfnMLEProc1( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * OutputMLEProc                                                             *
 *                                                                           *
 * A subclass window procedure for the output (target) MLE control.  We use  *
 * this to intercept the default clipboard keyboard triggers, so that we can *
 * call our own functions for handling Unicode text.                         *
 *                                                                           *
 * Refer to the PM documentation for argument and return value descriptions. *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY OutputMLEProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    USHORT  usFlags,
            usVK,
            usX, usY;
    ULONG   ulCopied = 0;
    POINTL  pts;


    switch( msg ) {

        case WM_CHAR:
            usFlags = SHORT1FROMMP( mp1 );
            usVK    = SHORT2FROMMP( mp2 );
            if (( usFlags & KC_VIRTUALKEY ) && ( ! (usFlags & KC_KEYUP) )) {
                // Ctrl+Ins (copy)
                if (( usFlags & KC_CTRL ) && ( usVK == VK_INSERT )) {
                    ulCopied = DoCopyCut( hwnd, global.fCopyUniTarget, FALSE );
                    return (MRESULT) TRUE;
                }
            }
            break;

        case WM_CONTEXTMENU:
            WinQueryPointerPos( HWND_DESKTOP, &pts );
            WinMapWindowPoints( HWND_DESKTOP, hwnd, &pts, 1 );
            WinPopupMenu( hwnd, global.hwndClient, global.hwndTContext, pts.x, pts.y,
                          0, PU_HCONSTRAIN | PU_VCONSTRAIN | PU_KEYBOARD | PU_MOUSEBUTTON1 );
            return (MRESULT) TRUE;

        case MLM_COPY:
            ulCopied = DoCopyCut( hwnd, global.fCopyUniTarget, FALSE );
            return (MRESULT) ulCopied;

        default: break;
    }

    return (MRESULT) global.pfnMLEProc2( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * CentreWindow                                                              *
 *                                                                           *
 * Centres the given window on the screen.                                   *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd    : handle of the window to be centred.                      *
 *                                                                           *
 * RETURNS: n/a                                                              *
 * ------------------------------------------------------------------------- */
void CentreWindow( HWND hwnd )
{
    LONG scr_width, scr_height,
         app_width, app_height;
    LONG x, y;
    SWP wp;

    scr_width  = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
    scr_height = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );

    if ( WinQueryWindowPos( hwnd, &wp )) {
        x = ( scr_width - wp.cx ) / 2;
        y = ( scr_height - wp.cy ) / 2;
        WinSetWindowPos( hwnd, HWND_TOP, x, y, wp.cx, wp.cy, SWP_MOVE | SWP_ACTIVATE );
    }

}


/* ------------------------------------------------------------------------- *
 * PaintClient                                                               *
 *                                                                           *
 * Repaints the client window area, making sure all controls are properly    *
 * positioned and sized.                                                     *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd: Client window handle.                                        *
 *                                                                           *
 * RETURNS: (MRESULT) 0                                                      *
 * ------------------------------------------------------------------------- */
MRESULT PaintClient( HWND hwnd )
{
    HPS         hps;            // client presentation space
    RECTL       rcl,            // dimensions of client
                rclCtrl;        // dimensions of a single control
    POINTL      ptclient[ 2 ],  // client origin and size points
                ptorigin;       // current positioning offset
    LONG        lPanelHeight,   // height of a single grouping of controls
                x, y, cx, cy;

    hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );
    WinQueryWindowRect( hwnd, &rcl );
    WinFillRect( hps, &rcl, SYSCLR_DIALOGBACKGROUND );

    // Get some basic measurements
    ptclient[0].x = rcl.xLeft;
    ptclient[0].y = rcl.yBottom;
    ptclient[1].x = rcl.xRight;
    ptclient[1].y = rcl.yTop;
    WinMapDlgPoints( hwnd, ptclient, 2, FALSE );
    lPanelHeight = ptclient[1].y / 2;

    // Now resize all our window contents, working from the bottom up

    // The bottom panel: target (output) text controls
    ptorigin.x = 1;
    ptorigin.y = 1;
    LayoutControls( hwnd, ptorigin, ptclient, lPanelHeight, TRUE );

    // The top panel: source (input) text controls
    ptorigin.x = 1;
    ptorigin.y = lPanelHeight + 1;
    LayoutControls( hwnd, ptorigin, ptclient, lPanelHeight, FALSE );

    WinEndPaint( hps );
    return ( 0 );
}


/* ------------------------------------------------------------------------- *
 * LayoutControls                                                            *
 *                                                                           *
 * Handles resizing of the controls within a single "panel" (i.e. the top or *
 * bottom grouping of controls for input and output, respectively).          *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND   hwnd        : Handle to client window                            *
 *   POINTL ptorigin    : Origin coordinates for the current "panel"         *
 *   POINTL ptclient[2] : Origin and size coordinates of the client window   *
 *   LONG   lPanelHeight: Height of the current "panel" (in DU).             *
 *   BOOL   fTarget     : Indicates bottom or top panel (TRUE=bottom)        *
 *                                                                           *
 * RETURNS: n/a                                                              *
 * ------------------------------------------------------------------------- */
void LayoutControls( HWND hwnd, POINTL ptorigin, POINTL ptclient[2], LONG lPanelHeight, BOOL fTarget )
{
    POINTL ptmle[ 2 ],      // MLE control origin and size points
           pttx[ 2 ],       // static text label origin and size points
           ptcombo[ 2 ],    // combo-box origin and size points
           ptbtn[ 2 ];      // pushbutton origin and size points
    HWND   hwndCtrl;        // handle to current control


    ptmle[0].x = ptorigin.x;
    ptmle[0].y = ptorigin.y;
    ptmle[1].x = ptclient[1].x - (ptorigin.x * 2);
    ptmle[1].y = lPanelHeight - 16;

    pttx[0].x = ptorigin.x;
    pttx[0].y = ptorigin.y + ptmle[1].y + 2;
    pttx[1].x = 50;
    pttx[1].y = 8;

    ptcombo[0].x = ptorigin.x + pttx[1].x + 1;
    ptcombo[0].y = ptorigin.y;
    ptcombo[1].x = ptclient[1].x - pttx[1].x - 64 - (ptorigin.x * 2);
    ptcombo[1].y = lPanelHeight - 5;

    if ( fTarget ) hwndCtrl = global.hwndTarget;
    else           hwndCtrl = global.hwndSource;
    WinMapDlgPoints( hwnd, ptmle, 2, TRUE );
    WinSetWindowPos( hwndCtrl, HWND_TOP, ptmle[0].x, ptmle[0].y, ptmle[1].x, ptmle[1].y, SWP_SIZE | SWP_MOVE | SWP_SHOW );

    if ( fTarget ) hwndCtrl = WinWindowFromID(hwnd, IDD_TARGETTX);
    else           hwndCtrl = WinWindowFromID(hwnd, IDD_SOURCETX);
    WinMapDlgPoints( hwnd, pttx, 2, TRUE );
    WinSetWindowPos( hwndCtrl, HWND_TOP, pttx[0].x, pttx[0].y, pttx[1].x, pttx[1].y, SWP_SIZE | SWP_MOVE | SWP_SHOW );

    if ( fTarget ) hwndCtrl = WinWindowFromID(hwnd, IDD_TARGETCP);
    else           hwndCtrl = WinWindowFromID(hwnd, IDD_SOURCECP);
    WinMapDlgPoints( hwnd, ptcombo, 2, TRUE );
    WinSetWindowPos( hwndCtrl, HWND_TOP, ptcombo[0].x, ptcombo[0].y, ptcombo[1].x, ptcombo[1].y, SWP_SIZE | SWP_MOVE | SWP_SHOW );

    if ( fTarget ) {
        ptbtn[0].x = ptclient[1].x - 60 - ptorigin.x;
        ptbtn[0].y = lPanelHeight - 14;
        ptbtn[1].x = 60;
        ptbtn[1].y = 12;
        WinMapDlgPoints( hwnd, ptbtn, 2, TRUE );
        WinSetWindowPos( WinWindowFromID(hwnd, ID_CONVERT), HWND_TOP,
                         ptbtn[0].x, ptbtn[0].y, ptbtn[1].x, ptbtn[1].y, SWP_SIZE | SWP_MOVE | SWP_SHOW );
    }

}


/* ------------------------------------------------------------------------- *
 * PopulateCodepages                                                         *
 *                                                                           *
 * Populate the two drop-down codepage lists with the defined codepages read *
 * from "codepage.lst".  The codepages will appear in the list in the same   *
 * order in which they are defined in this file.                             *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd: Handle to the application client window.                     *
 *                                                                           *
 * RETURNS: n/a                                                              *
 * ------------------------------------------------------------------------- */
void PopulateCodepages( HWND hwnd )
{
    ULONG     ulEntries,            // size of the pszEncodings structure
              ulCount,              // number of codepages defined
              ulCP,                 // parsed codepage number
              ulMyCP,               // current (PM) codepage
              i, j;                 // counters
    UCHAR     szBuf[ MAX_CP_DEF ],  // input string (from file)
              szEntry[ MAX_ENTRY ], // string entry for codepage list(s)
              szPath[ CCHMAXPATH ]; // path to codepage list file
    PSZ       token,                // string token from strtok
              c;                    // character pointer from strrchr
    SHORT     sIdx,                 // current list index
              sDefault;             // default list index
    PTIB      ptib;                 // required by DosGetInfoBlocks
    PPIB      ppib;                 // ...
    FILE      *pfCPList = NULL;     // codepage list file


    ulMyCP   = WinQueryCp( global.hmq );
    sDefault = 0;
    memset( global.encodings, 0, sizeof(global.encodings) );

    // TODO: get list of codepages in "?:\language\codepage\ibm"* and
    //       aliases in "?:\language\codepage\ucstbl.lst".

    // Open the file containing the list of codepages
    DosGetInfoBlocks( &ptib, &ppib );
    // (look in the program directory first)
    if ( DosQueryModuleName( ppib->pib_hmte, CCHMAXPATH, szPath ) == NO_ERROR ) {
        if (( c = strrchr( szPath, '\\')) != NULL ) {
            memset( c, 0, strlen(c) );
            strncat( szPath, "\\codepage.lst", CCHMAXPATH - 1 );
            pfCPList = fopen( szPath, "r");
        }
    }
    // (if that failed, try the current working directory)
    if ( pfCPList == NULL ) {
        if (( pfCPList = fopen("codepage.lst", "r")) == NULL ) return;
    }

    // Parse the list of codepages
    ulCount = 0;
    while ( ! feof(pfCPList) ) {
        if ( fgets( szBuf, MAX_CP_DEF, pfCPList ) == NULL ) break;
        strstrip( szBuf );
        // skip blank or commented lines
        if (( strlen(szBuf) == 0 ) || ( szBuf[0] == '#')) continue;
        if (( token = strtok( szBuf, ";")) == NULL ) continue;
        if ( ! sscanf( token, "%d", &ulCP )) continue;
        token = strtok( NULL, ";");

        // TODO: verify codepage is in list of existing codepages (above)

        // Save codepage to the master list
        global.encodings[ ulCount ].ulCP = ulCP;
        if ( token != NULL )
            strncpy( global.encodings[ ulCount ].szDesc, token, MAX_DESC-1 );

        ulCount++;
    }
    fclose( pfCPList );

    // Populate the comboboxes with our list of codepages
    for ( i = 0; i < ulCount; i++ ) {
        sprintf( szEntry, "[%d]  %s", global.encodings[i].ulCP, global.encodings[i].szDesc );
        sIdx = (SHORT) WinSendDlgItemMsg( hwnd, IDD_SOURCECP, LM_INSERTITEM,
                                          MPFROMSHORT(LIT_END), MPFROMP(szEntry) );
        global.encodings[i].sIndex = sIdx;
        sIdx = (SHORT) WinSendDlgItemMsg( hwnd, IDD_TARGETCP, LM_INSERTITEM,
                                          MPFROMSHORT(LIT_END), MPFROMP(szEntry) );

        // Check to see if this is the current codepage
        if ( global.encodings[i].ulCP == ulMyCP )
            sDefault = global.encodings[i].sIndex;
    }
    // ?TODO: Add special encodings afterwards?

    // Select the current codepage in each list by default
    WinSendDlgItemMsg( hwnd, IDD_SOURCECP, LM_SELECTITEM,
                       MPFROMSHORT( sDefault ), MPFROMSHORT( TRUE ));
    WinSendDlgItemMsg( hwnd, IDD_TARGETCP, LM_SELECTITEM,
                       MPFROMSHORT( sDefault ), MPFROMSHORT( TRUE ));

}


/* ------------------------------------------------------------------------- *
 * strstrip                                                                  *
 *                                                                           *
 * A rather quick-and-dirty function to strip leading and trailing white     *
 * space from a string.                                                      *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PSZ s: The string to be stripped.  This parameter will contain the      *
 *          stripped string when the function returns.                       *
 *                                                                           *
 * RETURNS: PSZ                                                              *
 *   A pointer to the string s.                                              *
 * ------------------------------------------------------------------------- */
PSZ strstrip( PSZ s )
{
    int  next,
         last,
         i,
         len,
         newlen;
    PSZ  s2;

    len  = strlen( s );
    next = strspn( s, " \t\n\r");
    for ( i = len - 1; i >= 0; i-- ) {
        if ( s[i] != ' ' && s[i] != '\t' && s[i] != '\r' && s[i] != '\n') break;
    }
    last = i;
    if (( next >= len ) || ( next > last )) {
        memset( s, 0, len+1 );
        return s;
    }

    newlen = last - next + 1;
    s2 = (char *) malloc( newlen + 1 );
    i = 0;
    while ( next <= last )
        s2[i++] = s[next++];
    s2[i] = 0;

    memset( s, 0, len+1 );
    strncpy( s, s2, newlen );
    free( s2 );

    return ( s );
}


/* ------------------------------------------------------------------------- *
 * DoPaste                                                                   *
 *                                                                           *
 * Pastes text from the clipboard into the source MLE.  Preference is given  *
 * to clipboard data in the "text/unicode" format; if not available, we use  *
 * plain text (CF_TEXT) instead.                                             *
 *                                                                           *
 * ARGUMENTS: none                                                           *
 *                                                                           *
 * RETURNS: ULONG                                                            *
 *   Number of bytes pasted.                                                 *
 * ------------------------------------------------------------------------- */
ULONG DoPaste( void )
{
    UconvObject uconv;
    UniChar suCodepage[ MAX_CP_SPEC ],  // conversion specifier
            *psuClipText,               // Unicode text in clipboard
            *puniC;                     // pointer into psuClipText
    CHAR    szError[ MAX_ERROR ];       // buffer for error messages
    PSZ     pszClipText,                // plain text in clipboard
            pszLocalText,               // imported text
            s;                          // pointer into pszLocalText
    ULONG   ulCP,                       // codepage to be used
            ulBufLen,                   // length of output buffer
            ulCopied = 0,               // number of characters copied
            ulRC;                       // return code
    SHORT   sIdx;                       // index of selected list item


    if ( WinOpenClipbrd(global.hab) ) {

        // Import as Unicode text if available
        if ( global.fPasteUni && (( psuClipText = (UniChar *) WinQueryClipbrdData( global.hab, global.cf_UniText )) != NULL )) {

            // Determine which codepage it will be converted to
            sIdx = (SHORT) WinSendDlgItemMsg( global.hwndClient, IDD_SOURCECP, LM_QUERYSELECTION,
                                              MPFROMSHORT(LIT_FIRST), MPFROMLONG(0) );
            if ( sIdx == LIT_NONE || global.fPasteAsCurrent )
                ulCP = WinQueryCp( global.hmq );
            else
                ulCP = global.encodings[ sIdx ].ulCP;

            // Create the conversion object
            UniMapCpToUcsCp( ulCP, suCodepage, MAX_CP_NAME );
            UniStrcat( suCodepage, (UniChar *) L"@map=cdra,path=no");

            if (( ulRC = UniCreateUconvObject( suCodepage, &uconv )) == ULS_SUCCESS ) {

                // Make sure dotless-i characters don't get converted to euro signs
                if ( ulCP == 850 )
                    while (( puniC = UniStrchr( psuClipText, 0x0131 )) != NULL ) *puniC = 0xFFFD;

                // Now do the conversion
                ulBufLen = ( UniStrlen(psuClipText) * 4 ) + 1;
                pszLocalText = (PSZ) malloc( ulBufLen );
                if (( ulRC = UniStrFromUcs( uconv, pszLocalText,
                                            psuClipText, ulBufLen )) == ULS_SUCCESS )
                {
                    // (some codepages use 0x1A for substitutions; replace with ?)
                    while (( s = strchr( pszLocalText, 0x1A )) != NULL ) *s = '?';
                    // Output the converted text
                    WinSendMsg( global.hwndSource, MLM_INSERT, MPFROMP(pszLocalText), 0 );
                    ulCopied = strlen( pszLocalText );
                } else {
                    sprintf( szError, "Error pasting Unicode text:\nUniStrFromUcs() = %08X", ulRC );
                    ErrorPopup( szError );
                }
                UniFreeUconvObject( uconv );
                free( pszLocalText );

            } else {
                sprintf( szError, "Error pasting Unicode text:\nUniCreateUconvObject() = %08X", ulRC );
                ErrorPopup( szError );
            }
        }

        // Plain text otherwise
        else if (( pszClipText = (PSZ) WinQueryClipbrdData( global.hab, CF_TEXT )) != NULL ) {
            ulBufLen = strlen(pszClipText) + 1;
            pszLocalText = (PSZ) malloc( ulBufLen );
            strcpy( pszLocalText, pszClipText );
            WinSendMsg( global.hwndSource, MLM_INSERT, MPFROMP(pszLocalText), 0 );
            ulCopied = strlen( pszLocalText );
            free( pszLocalText );
        }

        WinCloseClipbrd(global.hab);
    }

    return ( ulCopied );
}


/* ------------------------------------------------------------------------- *
 * DoCopyCut                                                                 *
 *                                                                           *
 * Copies or cuts text to the clipboard from either MLE.  The text is copied *
 * in both plain text (CF_TEXT) format, and optionally in Unicode format     *
 * ("text/unicode") as well.                                                 *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwndMLE : Handle of the MLE that text is being copied from         *
 *   BOOL fUnicode: Indicates whether to copy text in Unicode format         *
 *   BOOL fCut    : Indicates if this is a cut rather than a copy operation  *
 *                                                                           *
 * RETURNS: ULONG                                                            *
 *   Number of bytes copied.                                                 *
 * ------------------------------------------------------------------------- */
ULONG DoCopyCut( HWND hwndMLE, BOOL fUnicode, BOOL fCut )
{
    UconvObject uconv;
    UniChar suCodepage[ MAX_CP_SPEC ],  // conversion specifier
            *psuCopyText,               // Unicode text to be copied
            *psuShareMem,               // Unicode text in clipboard
            *puniC;                     // pointer into psuCopyText
    HWND    hwndCP;                     // handle of codepage list combobox
    CHAR    szError[ MAX_ERROR ];       // buffer for error messages
    PSZ     pszCopyText,                // exported text
            pszShareMem,                // plain text in clipboard
            s;                          // pointer into pszLocalText
    ULONG   ulCP,                       // codepage to be used
            ulBufLen,                   // length of output buffer
            ulCopied = 0,               // number of characters copied
            ulRC;                       // return code
    SHORT   sSelected,                  // number of selected characters in MLE
            sIdx;                       // index of selected list item
    BOOL    fRC,                        // boolean return code
            fUniCopyFailed = FALSE,     // Unicode copy failed
            fTxtCopyFailed = FALSE;     // plain text copy failed
    IPT     ipt1, ipt2;                 // MLE insertion points (for querying selection)


    if ( hwndMLE == global.hwndTarget )
        hwndCP = global.hwndTargetCP;
    else
        hwndCP = global.hwndSourceCP;

    // Get the selected text
    ipt1 = (IPT) WinSendMsg( hwndMLE, MLM_QUERYSEL, MPFROMSHORT(MLFQS_MINSEL), 0 );
    ipt2 = (IPT) WinSendMsg( hwndMLE, MLM_QUERYSEL, MPFROMSHORT(MLFQS_MAXSEL), 0 );
//  WinSendMsg( hwndMLE, MLM_FORMAT, MPFROMSHORT(MLFIE_CFTEXT), 0 );
    sSelected   = (ULONG) WinSendMsg( hwndMLE, MLM_QUERYFORMATTEXTLENGTH, MPFROMLONG(ipt1), MPFROMLONG(ipt2 - ipt1) );
    pszCopyText = (PSZ) malloc( sSelected + 1 );
    ulCopied    = (ULONG) WinSendMsg( hwndMLE, MLM_QUERYSELTEXT, MPFROMP(pszCopyText), 0 );

    if ( WinOpenClipbrd(global.hab) ) {

        ulBufLen = ulCopied + 1;
        WinEmptyClipbrd( global.hab );

        // Copy as Unicode text
        if ( fUnicode ) {

            // Determine which codepage we are converting from
            sIdx = (SHORT) WinSendMsg( hwndCP, LM_QUERYSELECTION,
                                   MPFROMSHORT(LIT_FIRST), MPFROMLONG(0) );
            if ( sIdx == LIT_NONE )
                ulCP = WinQueryCp( global.hmq );
            else
                ulCP = global.encodings[ sIdx ].ulCP;

            // Create the conversion object
            UniMapCpToUcsCp( ulCP, suCodepage, MAX_CP_NAME );
            UniStrcat( suCodepage, (UniChar *) L"@map=cdra,path=no");

            if (( ulRC = UniCreateUconvObject( suCodepage, &uconv )) == ULS_SUCCESS ) {

                // Do the conversion
                psuCopyText = (UniChar *) calloc( ulBufLen, sizeof(UniChar) );
                if (( ulRC = UniStrToUcs( uconv, psuCopyText,
                                          pszCopyText, ulBufLen )) == ULS_SUCCESS )
                {
                    // Make sure dotless-i characters don't get converted to euro signs
                    if ( ulCP == 850 )
                        while (( puniC = UniStrchr( psuCopyText, 0x0131 )) != NULL ) *puniC = 0xFFFD;

                    // Place the UCS-2 string on the clipboard as "text/unicode"
                    ulRC = DosAllocSharedMem( (PVOID) &psuShareMem, NULL, ulBufLen,
                                              PAG_WRITE | PAG_COMMIT | OBJ_GIVEABLE );
                    if ( ulRC == 0 ) {
                        UniStrncpy( psuShareMem, psuCopyText, ulBufLen - 1 );
                        if ( ! WinSetClipbrdData( global.hab, (ULONG) psuShareMem,
                                                  global.cf_UniText, CFI_POINTER  ))
                        {
                            sprintf( szError, "Error copying Unicode text: WinSetClipbrdData() failed.\nError code: 0x%X\n", WinGetLastError(global.hab) );
                            ErrorPopup( szError );
                            fUniCopyFailed = TRUE;
                        }
                    } else {
                        sprintf( szError, "Error copying Unicode text.\nDosAllocSharedMem: 0x%X\n", ulRC );
                        ErrorPopup( szError );
                        fUniCopyFailed = TRUE;
                    }
                } else {
                    sprintf( szError, "Error copying Unicode text:\nUniStrToUcs() = %08X", ulRC );
                    ErrorPopup( szError );
                    fUniCopyFailed = TRUE;
                }

                UniFreeUconvObject( uconv );
                free( psuCopyText );

            } else {
                sprintf( szError, "Error copying Unicode text:\nUniCreateUconvObject() = %08X", ulRC );
                ErrorPopup( szError );
                fUniCopyFailed = TRUE;
            }
        }

        // Copy as plain text
        ulRC = DosAllocSharedMem( (PVOID) &pszShareMem, NULL, ulBufLen,
                                  PAG_WRITE | PAG_COMMIT | OBJ_GIVEABLE );
        if ( ulRC == 0 ) {
            strncpy( pszShareMem, pszCopyText, ulBufLen - 1 );
            if ( ! WinSetClipbrdData( global.hab, (ULONG) pszShareMem, CF_TEXT, CFI_POINTER ))
            {
                sprintf( szError, "Error copying plain text: WinSetClipbrdData() failed.\nError code: 0x%X\n", WinGetLastError(global.hab) );
                ErrorPopup( szError );
                fTxtCopyFailed = TRUE;
            }
        } else {
            sprintf( szError, "Error copying plain text.\nDosAllocSharedMem: 0x%X\n", ulRC );
            ErrorPopup( szError );
            fTxtCopyFailed = TRUE;
        }

        // Done copying, now finish up
        if (( fCut ) && ( !fUniCopyFailed ) && ( !fTxtCopyFailed ))
            WinSendMsg( hwndMLE, MLM_CLEAR, 0, 0 );

        WinCloseClipbrd(global.hab);
    }

    free( pszCopyText );
    if ( fTxtCopyFailed ) ulCopied = 0;
    return ( ulCopied );
}


/* ------------------------------------------------------------------------- *
 * ConvertText                                                               *
 *                                                                           *
 * This function performs the actual text conversion.  Text is read from the *
 * source MLE, converted from the specified source codepage to UCS-2, and    *
 * thence to the requested target codepage; the converted text is then       *
 * placed in the (read-only) target MLE.                                     *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND hwnd  : Client window handle.                                      *
 *   ULONG ulCP1: The source codepage.                                       *
 *   ULONG ulCP2: The target codepage.                                       *
 *                                                                           *
 * RETURNS: (ULONG) 0                                                        *
 * ------------------------------------------------------------------------- */
ULONG ConvertText( HWND hwnd, ULONG ulCP1, ULONG ulCP2 )
{
    UconvObject uoSource = NULL,         // conversion object for source codepage
                uoTarget = NULL;         // conversion object for target codepage
    UniChar     suSource[ MAX_CP_SPEC ], // source codepage specifier
                suTarget[ MAX_CP_SPEC ], // target codepage specifier
                *psuBuffer,              // UCS-2 buffer for text conversion
                *puniC;                  // pointer into buffer (for UniStrchr)
    CHAR        szError[ MAX_ERROR ];    // buffer for error messages
    PSZ         pszSourceText,           // original text (input)
                pszTargetText,           // converted text (output)
                s;                       // pointer into buffer (for strchr)
    LONG        lBytes;                  // number of bytes read from MLE
    ULONG       ulBufSize,               // length of string being converted
                ulOutputSize,            // length of output buffer
                ulRC;                    // return code


    // Read the input text from the MLE
    lBytes = WinQueryWindowTextLength( global.hwndSource );
    if ( lBytes == 0 ) {
        WinSetWindowText( global.hwndTarget, "");
        return ( 0 );
    }
    pszSourceText = (PSZ) malloc( lBytes + 1 );
    WinQueryWindowText( global.hwndSource, lBytes + 1, pszSourceText );
    ulBufSize = strlen( pszSourceText ) + 1;
    ulOutputSize = ulBufSize * 4;

    // Create ULS conversion objects for the source and target codepages
    UniMapCpToUcsCp( ulCP1, suSource, MAX_CP_NAME );
    UniStrcat( suSource, (UniChar *) L"@map=cdra,path=no");
    if (( ulRC = UniCreateUconvObject( suSource, &uoSource )) == ULS_SUCCESS ) {;

        UniMapCpToUcsCp( ulCP2, suTarget, MAX_CP_NAME );
        UniStrcat( suTarget, (UniChar *) L"@map=cdra,path=no");

        if (( ulRC = UniCreateUconvObject( suTarget, &uoTarget )) == ULS_SUCCESS ) {

            // Now convert the text
            psuBuffer = (UniChar *) calloc( ulBufSize, sizeof(UniChar) );
            memset( psuBuffer, 0, sizeof(psuBuffer) );
            pszTargetText = (PSZ) malloc( ulOutputSize );
            memset( pszTargetText, 0, ulOutputSize );

            if (( ulRC = UniStrToUcs( uoSource, psuBuffer,
                                      pszSourceText, ulBufSize )) == ULS_SUCCESS )
            {
                if ( ulCP2 == 850 ) {
                    // Make sure dotless-i characters don't get rendered as euro signs
                    while (( puniC = UniStrchr( psuBuffer, 0x0131 )) != NULL ) *puniC = 0xFFFD;
                }
                if (( ulRC = UniStrFromUcs( uoTarget, pszTargetText,
                                            psuBuffer, ulOutputSize )) == ULS_SUCCESS )
                {
                    // Success: output the converted text

                    // (some codepages use 0x1A for substitutions; replace with ?)
                    while (( s = strchr( pszTargetText, 0x1A )) != NULL ) *s = '?';

                    WinSetWindowText( global.hwndTarget, pszTargetText );

                } else {
                    sprintf( szError, "UniStrFromUcs() failed: 0x%08X", ulRC );
                    ErrorPopup( szError );
                }
            } else {
                sprintf( szError, "UniStrToUcs() failed: 0x%08X", ulRC );
                ErrorPopup( szError );
            }

            UniFreeUconvObject( uoTarget );
            free( psuBuffer );
            free( pszTargetText );

        } else {
            sprintf( szError, "UniCreateUconvObject() for target codepage %d failed: 0x%08X", ulCP2, ulRC );
            ErrorPopup( szError );
        }

        UniFreeUconvObject( uoSource );

    } else {
        sprintf( szError, "UniCreateUconvObject() for source codepage %d failed: 0x%08X", ulCP1, ulRC );
        ErrorPopup( szError );
    }

    free( pszSourceText );
    return ( 0 );
}


/* ------------------------------------------------------------------------- *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY OptionDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    static PCLIPOPTIONS pOptions;

    switch( msg ) {

        case WM_INITDLG:
            pOptions = (PCLIPOPTIONS) mp2;
            WinSendDlgItemMsg( hwnd, IDD_IPASTEUCS,    BM_SETCHECK,
                               MPFROMSHORT( (SHORT) pOptions->fPasteUni ), 0 );
            WinSendDlgItemMsg( hwnd, IDD_PASTETARGET,  BM_SETCHECK,
                               MPFROMSHORT( (SHORT) !pOptions->fPasteAsCurrent ), 0 );
            WinSendDlgItemMsg( hwnd, IDD_PASTECURRENT, BM_SETCHECK,
                               MPFROMSHORT( (SHORT) pOptions->fPasteAsCurrent ), 0 );
            WinSendDlgItemMsg( hwnd, IDD_ICOPYUCS,     BM_SETCHECK,
                               MPFROMSHORT( (SHORT) pOptions->fCopyUniSource ), 0 );
            WinSendDlgItemMsg( hwnd, IDD_OCOPYUCS,     BM_SETCHECK,
                               MPFROMSHORT( (SHORT) pOptions->fCopyUniTarget ), 0 );
            CentreWindow( hwnd );
            return (MRESULT) FALSE;


        case WM_COMMAND:
            switch( SHORT1FROMMP( mp1 )) {

                case DID_OK:
                    pOptions->fPasteUni       = (BOOL) WinQueryButtonCheckstate( hwnd, IDD_IPASTEUCS );
                    pOptions->fPasteAsCurrent = (BOOL) WinQueryButtonCheckstate( hwnd, IDD_PASTECURRENT );
                    pOptions->fCopyUniSource  = (BOOL) WinQueryButtonCheckstate( hwnd, IDD_ICOPYUCS );
                    pOptions->fCopyUniTarget  = (BOOL) WinQueryButtonCheckstate( hwnd, IDD_OCOPYUCS );
                    pOptions->fChanged        = TRUE;
                    break;

                default: break;
            }
            break;

        default: break;
    }

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * AboutDlgProc                                                              *
 *                                                                           *
 * Dialog procedure for the "product information" dialog.                    *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY AboutDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    switch( msg ) {
        case WM_INITDLG:
            CentreWindow( hwnd );
            return (MRESULT) FALSE;

        default: break;
    }
    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}


