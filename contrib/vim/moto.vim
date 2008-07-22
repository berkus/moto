" Vim syntax file
" Language:     Moto
" Author:       Stefano Corsi <scorsi@projectmoto.org>, Shay Harding <sharding@ccbill.com>
" Maintainer:   Stefano Corsi <scorsi@projectmoto.org>, Shay Harding <sharding@ccbill.com>
" URL:          http://www.projectmoto.org
" Last Change:  2003 Mar 25

" Place this file in your VIMHOME/syntax/ directory
" Modify VIMHOME/filetype.vim and add:
"
" au BufNewFile,BufRead *.moto        setf moto 
"

" Quit when a syntax file was already loaded
if !exists("main_syntax")
  if version < 600
    syntax clear
  elseif exists("b:current_syntax")
    finish
  endif
  " we define it here so that included files can test for it
  let main_syntax='moto'
endif

" don't use standard HiLink, it will not work with included syntax files
if version < 508
  command! -nargs=+ MotoHiLink hi link <args>
else
  command! -nargs=+ MotoHiLink hi def link <args>
endif

" To unload html highlighting simply specify:
" :let moto_no_html=1
" :set syntax=moto
if !exists("moto_no_html")
   let html_no_rendering = 1
   syn include @html <sfile>:p:h/html.vim

   " Remove this since it's really not an error in Moto
   syn cluster html remove=htmlError
endif

" Highlight trailing white space
if !exists("moto_no_space_errors")
   syn match motoSpaceError display excludenl "\s\+$" contained
   syn match motoSpaceError display " \+\t"me=e-1 contained
   MotoHiLink motoSpaceError Error
endif

"Highlight SQL queries
if exists("moto_show_sql")
   syn include @sql <sfile>:p:h/sql.vim
   syn cluster sql remove=sqlString,sqlComment
endif

syntax case match

if exists("moto_fold_page")
   syn region motoPage start=".[^\$]" end="${" contains=@html,motoEmbedded,motoMultiLineComment1,@motoText,@motoMacros,@motoPreProcessor,motoSpaceError transparent fold keepend

   let moto_folded=1
else
   syn region motoPage start=".[^\$]" end="${" contains=@html,motoEmbedded,motoMultiLineComment1,@motoText,@motoMacros,@motoPreProcessor,motoSpaceError
endif

if exists("moto_fold_embed")
   syn region motoEmbedded matchgroup=MotoCEmbed start="${" end="}\$" contained contains=motoError,@motoComments,@motoText,@motoAll,motoRegex,motoCast,motoSpaceError transparent fold keepend

   let moto_folded=1
else
   syn region motoEmbedded matchgroup=MotoCEmbed start="${" end="}\$" contained contains=motoError,@motoComments,@motoText,@motoAll,motoRegex,motoCast,motoSpaceError
endif

if exists("moto_folded")
   syn sync fromstart
   setlocal foldmethod=syntax
endif


" Characters that cannot be in a moto program (outside a string)
" Some of these can be on a Regex // so we declare that below
syn match motoError "<<<\|\.\.\|=>\|<>\|||=\|&&=\|[^-]->\|\*\/\|[\\@`]\|$\w*(" contained

" Regex... used to supress false errors
" This must appear before the comments
syn match motoRegex "/.\+/" contained

" Comments
syn keyword motoTodo contained TODO FIXME XXX
syn region motoMultiLineComment1 start="\s*\$\*" end="\*\$" contains=motoTodo contained
syn region motoMultiLineComment2 start="\s*/\*" end="\*/" contains=motoTodo contained
syn region motoLineComment start="//" end="$" contains=motoTodo contained
syn cluster motoComments contains=motoMultiLineComment1,motoMultiLineComment2,motoLineComment

" Cast
syn region motoCast matchgroup=Special start="<\s*\w\+\s*>"rs=s+1 end=">" end="$" contained contains=@motoTypes

" Macros and directives
syn region motoMacro1 start="\$\w\+" end="[(\t ]"he=e-1 contains=@motoDirectives contained
syn match motoMacro2 "$("he=e-1 contained
syn cluster motoMacros contains=motoMacro1,motoMacro2,@motoTypes,motoObjectOps

" Strings
syn region motoString start=/"/ skip=/\\"/ end=/"/ contains=motoStringEscape,@sql contained
syn region motoString start="'" skip="\\'" end="'" contained
syn match motoStringEscape "\\." contained

" Numbers
syn match motoNumber "\<\(0[0-7]*\|0[xX]\x\+\|\d\+\)[lL]\=\>" contained display 
syn match motoNumber "\(\<\d\+\.\d*\|\.\d\+\)\([eE][-+]\=\d\+\)\=[fFdD]\=" contained display
syn match motoNumber "\<\d\+[eE][-+]\=\d\+[fFdD]\=\>" contained display
syn match motoNumber "\<\d\+\([eE][-+]\=\d\+\)\=[fFdD]\>" contained display
syn cluster motoText contains=motoString,motoNumber

" Pre-processor
syn match motoPP display "\s*#\(include\|readdefs\)" contained display
syn cluster motoPreProcessor contains=motoPP

syn keyword motoPrimitiveTypes contained        double float long int byte boolean char void
syn keyword motoObjectTypes    contained        Array Integer Long Float Double Boolean Object
syn keyword motoObjectTypes    contained        Character Regex Match String Date Enumeration
syn keyword motoObjectTypes    contained        FileIO Hashset Hashtable IntEnumeration IntHashtable
syn keyword motoObjectTypes    contained        IntSet IntStack ItolHashtable KMPString Sharedmem
syn keyword motoObjectTypes    contained        Stack StringBuffer Stringset SymbolTable TabularData 
syn keyword motoObjectTypes    contained        Tokenizer Vector Cookie Form HTTPRequest ModuleConfig 
syn keyword motoObjectTypes    contained        HTTPResonse MIMEEntity State Session Context 
syn keyword motoObjectTypes    contained        MySQLResultSet MySQLConnection PGResultSet PGConnection 
syn keyword motoObjectTypes    contained        PosixFileSystem 
syn keyword motoStatement      contained        declare define use enddef
syn keyword motoObjectStore    contained        class endclass
syn keyword motoObjectOps      contained        new delete
syn keyword motoBranch         contained        break return continue case
syn keyword motoConstant       contained        null true false
syn keyword motoConditional    contained        if else switch endif endswitch elseif
syn keyword motoRepeat         contained        for while do endwhile endfor
syn keyword motoLabel          contained        default
syn keyword motoException      contained        try catch finally throw endtry
syn keyword motoExceptionType  contained        Exception AllocationFailureException ArrayBoundsException
syn keyword motoExceptionType  contained        ConnectionRefusedException DLCloseException
syn keyword motoExceptionType  contained        DLException DLOpenException DLSymbolException
syn keyword motoExceptionType  contained        DefaultException EmptyQueueException EmptyStackException
syn keyword motoExceptionType  contained        ExecFailedException FileNotFoundException ForkFailedException
syn keyword motoExceptionType  contained        IOException IllegalArgumentException MathException NetException
syn keyword motoExceptionType  contained        NoSuchElementException NullPointerException NumberFormatException
syn keyword motoExceptionType  contained        SecurityException SemaphoreException UnexpectedEOFException
syn keyword motoExceptionType  contained        UnsupportedOperationException SignalException RegexParseException
syn keyword motoExceptionType  contained        RuntimeException SQLException IllegalConversionException

MotoHiLink motoDeclare motoCast
syn region motoDeclare matchgroup=Type start="global[ \t]\+" end="[ \t]"he=e-1 end="$" contains=@motoTypes contained

syn cluster motoTypes contains=motoPrimitiveTypes,motoObjectTypes,motoDeclare
syn cluster motoDirectives contains=motoBranch,motoConditional,motoRepeat,motoException,motoStatement,motoObjectStore

syn cluster motoAll contains=@motoTypes,@motoDirectives,motoConstant,motoLabel,motoExceptionType,motoObjectOps

" Custom colors 
hi def MotoCComment term=bold ctermfg=lightgrey guifg=lightgrey
hi def MotoCTodo term=bold,underline cterm=bold,underline ctermfg=darkred gui=bold,underline
hi def MotoCEmbed ctermfg=darkgrey
hi def MotoCType ctermfg=darkcyan

" The default highlighting.
if version >= 508 || !exists("did_moto_syn_inits")
  if version < 508
    let did_moto_syn_inits = 1
  endif
   MotoHiLink motoStringEscape         Special
   MotoHiLink motoString               Constant
   MotoHiLink motoStatement            Statement 
   MotoHiLink motoRepeat               Repeat
   MotoHiLink motoConditional          Conditional
   MotoHiLink motoBranch               Statement
   MotoHiLink motoException            Exception
   MotoHiLink motoExceptionType        Type 
   MotoHiLink motoObjectStore          Statement
   MotoHiLink motoObjectOps            Statement
   MotoHiLink motoConstant             Constant
   MotoHiLink motoLabel                Label
   MotoHiLink motoPrimitiveTypes       Type
   MotoHiLink motoObjectTypes          Type
   MotoHiLink motoTodo                 MotoCTodo
   MotoHiLink motoLineComment          MotoCComment
   MotoHiLink motoMultiLineComment1    MotoCComment
   MotoHiLink motoMultiLineComment2    MotoCComment
   MotoHiLink motoError                Error
   MotoHiLink motoMacro1               Identifier
   MotoHiLink motoMacro2               Identifier
   MotoHiLink motoNumber               Number
   MotoHiLink motoPP                   Include
   MotoHiLink motoCast                 MotoCType
endif

syn sync match motoSync grouphere motoEmbedded "${"
syn sync ccomment motoComment minlines=50 maxlines=300

delcommand MotoHiLink
let b:current_syntax = "moto"

if main_syntax == 'moto'
  unlet main_syntax
endif

let b:spell_options="contained"

