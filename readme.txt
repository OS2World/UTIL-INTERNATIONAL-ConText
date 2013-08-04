CONTEXT/2

  ConText/2 is a utility for converting text from one character encoding to
  another.  That is to say, it allows you to convert text from any arbitrary
  codepage (amongst those supported by OS/2) into any other codepage.

  Conversion, in this sense, means that the actual byte values of the text are
  changed in order to preserve the _meaning_ of each character in the target
  codepage.  This is distinct from the _rendering_ of characters on-screen,
  which is always determined by the current Presentation Manager codepage.
  This means that the text as you see it will not necessarily look correct if
  the current PM codepage is different from the one selected in the applicable
  list.

  ConText/2 also supports the copying or pasting of Unicode (UCS-2) text to or
  from the clipboard (in the format supported by the Mozilla family of
  products).  Pasted Unicode text will be converted at paste time to either the 
  the currently-selected input encoding (default) or the current codepage, 
  depending on the selected options.  (When copying, Unicode text will always 
  be interpreted use the currently-selected encoding.)


USING CONTEXT/2

  Run CONTEXT.EXE to start the program.  CODEPAGE.LST must be either in same
  directory as CONTEXT.EXE, or else in the current working directory.

  The program window consists of an upper and lower panel.  The combination box
  in the upper panel specifies the input text encoding: this is the encoding
  format of the text in the MLE immediately below (with the white background).

  The combination box in the lower panel specifies the desired output text
  encoding.  The "Convert" button applies the conversion: when pressed, the
  text in the input (upper) MLE will be converted from the input encoding into
  the output encoding, and the resulting text will be placed in the output MLE
  at the bottom (with the pale grey background).

  You can also press Ctrl+Enter to apply the conversion.

  The Tab key can be used to switch the input focus between most of the UI
  controls (Ctrl+Tab may also be used, and is required when the input MLE has
  focus).


THE CODEPAGE LIST

  The list of available codepages is defined in CODEPAGE.LST, which is an ASCII
  text file.  You can edit this file to customize the list to your preferences.
  Lines beginning with '#' are comments; every other non-blank line defines a
  single codepage in the form

    <number>;<description>

  where <number> must be a valid codepage number, and <description> is any
  arbitrary text which describes the codepage.  Valid codepage numbers are those
  with codepage table files located in the \LANGUAGE\CODEPAGE directory on the
  OS/2 boot drive; these files have the name 'IBM*' where * is an integer value.
  (Aliases defined in UCSTBL.LST in the same directory may also be used, as long
  as the alias uses the same naming convention.)

  This file contains a large number of codepages; you may wish to remove some of
  the more obscure ones if you anticipate no need for them.


RELEASE HISTORY

  Version 0.4  (2007-02-02)
    - Added context menus to text controls.
    - Unicode text may now be copied (from either text control) as well as 
      pasted.
    - Clipboard behaviour with respect to Unicode text is now configurable.
    - CODEPAGE.LST is now loaded by default from the executable directory.

  Version 0.3  (2006-11-22)
    - First public release.  (Versions 0.1 & 0.2 were private internal builds.)


NOTICES

  ConText/2 is (C) 2007 Alex Taylor.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
   3. The name of the author may not be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
  EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.


--
Alex Taylor
http://www.cs-club.org/~alex
