#define INCL_DOSMODULEMGR
#define INCL_DOSPROCESS
#define INCL_GPI
#define INCL_WIN
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uconv.h>
#include "resource.h"


// ----------------------------------------------------------------------------
// MACROS

#define ErrorPopup( text ) \
    WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, text, "Error", 0, MB_OK | MB_ERROR )


// ----------------------------------------------------------------------------
// CONSTANTS

// These constants are used for reading and populating the codepage list
#define MAX_CPS         256     // maximum number of codepages
#define MAX_CP_DEF      256     // maximum length of a line in codepages.lst (plus null)
#define MAX_DESC        244     // maximum length of a codepage description (plus null)
#define MAX_ENTRY       260     // maximum length of a codepage list item (plus null)

// These constants are used in the Unicode API logic
#define MAX_CP_NAME     12      // maximum length of a codepage name
#define MAX_CP_SPEC     64      // maximum length of a UconvObject codepage specifier

#define MAX_ERROR       256     // maximum length of an error popup message

#define SZ_DEFAULTFONT  "6.System VIO"


// ----------------------------------------------------------------------------
// TYPES

typedef struct _encoding {
    ULONG ulCP;                     // codepage number
    SHORT sIndex;                   // list index
    UCHAR szDesc[ MAX_DESC ];       // description
} ENCODING, *PENCODING;

typedef struct _appglobal {
    HAB       hab;                  // anchor-block handle
    HMQ       hmq;                  // message queue handle
    HWND      hwndClient,           // handle to the program client window
              hwndMenu,             // handle to the main menubar
              hwndSource,           // handle to the source MLE
              hwndTarget,           // handle to the target MLE
              hwndSourceCP,         // handle to source combobox
              hwndTargetCP,         // handle to target combobox
              hwndButton,           // handle to convert button
              hwndSContext,         // popup menu for source MLE
              hwndTContext;         // popup menu for target MLE
    ATOM      cf_UniText;           // atom for "text/unicode" clipboard format
    BOOL      fPasteUni,            // Unicode paste flag
              fPasteAsCurrent,      // Unicode paste-conversion flag
              fCopyUniSource,       // Unicode copy flag for source MLE
              fCopyUniTarget;       // Unicode copy flag for target MLE
    PFNWP     pfnMLEProc1,          // default MLE window procedures
              pfnMLEProc2;
    ENCODING  encodings[ MAX_CPS ]; // list of available encodings
} CTGLOBAL, *PCTGLOBAL;

typedef struct _clipopts {
    BOOL fChanged,              // user changed settings?
         fPasteUni,             // Unicode paste flag
         fPasteAsCurrent,       // Unicode paste-conversion flag
         fCopyUniSource,        // Unicode copy flag for source MLE
         fCopyUniTarget;        // Unicode copy flag for target MLE
} CLIPOPTIONS, *PCLIPOPTIONS;


// ----------------------------------------------------------------------------
// FUNCTIONS

USHORT           CreateControls( HWND hwndClient );
MRESULT EXPENTRY ClientWndProc( HWND, ULONG, MPARAM, MPARAM );
MRESULT EXPENTRY InputMLEProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY OutputMLEProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
void             CentreWindow( HWND hwnd );
MRESULT          PaintClient( HWND );
void             LayoutControls( HWND hwnd, POINTL ptorigin, POINTL ptclient[2], LONG lPanelHeight, BOOL fTarget );
void             PopulateCodepages( HWND hwnd );
PSZ              strstrip( PSZ s );
ULONG            DoPaste( void );
ULONG            DoCopyCut( HWND hwndMLE, BOOL fUnicode, BOOL fCut );
ULONG            ConvertText( HWND hwnd, ULONG ulCP1, ULONG ulCP2 );
MRESULT EXPENTRY OptionDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY AboutDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );


