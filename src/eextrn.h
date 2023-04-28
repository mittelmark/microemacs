/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * eextrn.h - External function definitions.
 *
 * Copyright (C) 1988-2009 JASSPA (www.jasspa.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/*
 * Created:     Unknown
 * Synopsis:    External function definitions.
 * Authors:     Jon Green & Steven Phillips
 * Notes:
 *     The external command function declarations were part of efunc.h,
 *     these have all been moved to this header file and all other
 *     exeternally used functions have also been added.
 */

/* abbrev.c externals */
#if MEOPT_ABBREV
extern	int	bufferAbbrev(int f, int n);
extern	int	globalAbbrev(int f, int n);
extern	int	expandAbbrev(int f, int n);
#else
#define bufferAbbrev notAvailable
#define globalAbbrev notAvailable
#define expandAbbrev notAvailable
#endif

/* basic.c externals */
extern	int	windowGotoBol(int f, int n);
extern	int	windowGotoEol(int f, int n);
extern	int	meErrorEob(void) ;
extern	int	meErrorBob(void) ;
extern	int	meWindowBackwardChar(register meWindow *wp, register int n) ;
extern	int	meWindowForwardChar(register meWindow *wp, register int n) ;
extern	int	windowBackwardChar(int f, int n);
extern	int	windowForwardChar(int f, int n);
extern	meUByte meWindowGetChar(meWindow *wp) ;
extern	int	windowGotoLine(int f, int n);
#if MEOPT_EXTENDED || MEOPT_NARROW
extern	int	windowGotoAbsLine(meInt line) ;
#else
#define windowGotoAbsLine(l) windowGotoLine(1,l)
#endif
extern	int	windowGotoBob(int f, int n);
extern	int	windowGotoEob(int f, int n);
extern	int	windowForwardLine(int f, int n);
extern	int	windowBackwardLine(int f, int n);
#if MEOPT_WORDPRO
extern	int	windowBackwardParagraph(int f, int n);
extern	int	windowForwardParagraph(int f, int n);
#else
#define windowBackwardParagraph notAvailable
#define windowForwardParagraph notAvailable
#endif
extern	int	windowSetMark(int f, int n);
extern	int	windowSwapDotAndMark(int f, int n);
extern	void	meBufferStoreLocation(meLine *lp, meUShort lo, meInt ln) ;
extern	void	meBufferUpdateLocation(meBuffer *bp, meUInt noLines, meUShort newOff) ;

/* bind.c externals */
extern  meUInt  cmdHashFunc(register meUByte *cmdName) ;
extern  meUShort  meGetKeyFromString(meUByte **tp) ;
extern  meUShort  meGetKey(int flag) ;
extern  int     meGetStringFromChar(meUShort cc, meUByte *d) ;
extern	void	meGetStringFromKey(meUShort cc, meUByte *seq);
extern	int     decode_fncname(meUByte *fname, int silent);
extern	int	bindkey(meUByte *prom, int f, int n, meUShort *lclNoBinds,
                               meBind **lclBinds) ;
extern	int	unbindkey(meUByte *prom, int n, meUShort *lclNoBinds,
                                 meBind **lclBinds) ;
extern	int	globalBindKey(int f, int n);
extern	int	globalUnbindKey(int f, int n);
#if MEOPT_LOCALBIND
extern	int	bufferBindKey(int f, int n);
extern	int	bufferUnbindKey(int f, int n);
extern	int	mlBind(int f, int n);
extern	int	mlUnbind(int f, int n);
#else
#define bufferBindKey notAvailable
#define bufferUnbindKey notAvailable
#define mlBind notAvailable
#define mlUnbind notAvailable
#endif
extern	int	descBindings(int f, int n);
extern	int	descKey(int f,int n);
#if MEOPT_EXTENDED
extern	int	listCommands(int f, int n);
extern	int	commandApropos(int f, int n);
extern  void    charMaskTblInit(void);
extern	int	setCharMask(int f, int n);
#else
#define listCommands notAvailable
#define commandApropos notAvailable
#define setCharMask notAvailable
#endif

/* buffer.c externals */
extern  int     getBufferName(meUByte *prompt, int opt, int defH, meUByte *buf) ;
#if MEOPT_FILEHOOK
extern int      addFileHook(int f, int n) ;
#else
#define addFileHook notAvailable
#endif
#if MEOPT_EXTENDED
extern	int	nextWndFindBuf(int f, int n);
extern	int	deleteSomeBuffers(int f, int n);
extern	int	changeBufName(int f,int n);
extern	int	namedBufferMode(int f, int n);
#else
#define nextWndFindBuf notAvailable
#define deleteSomeBuffers notAvailable
#define changeBufName notAvailable
#define namedBufferMode notAvailable
#endif
extern	int	findBuffer(int f, int n);
extern	int	nextBuffer(int f, int n);
extern  void    setBufferContext(meBuffer *bp) ;
extern	int	swbuffer(meWindow *wp, meBuffer *bp);
extern	int	bufferDelete(int f, int n);
extern  meBuffer *replacebuffer(meBuffer *oldbuf) ;
extern  void    linkBuffer(meBuffer *bp) ;
extern  void    unlinkBuffer(meBuffer *bp) ;
extern	int	zotbuf(meBuffer *bp, int silent);
extern	int	adjustMode(meBuffer *bp, int nn);
extern	int	bufferMode(int f, int n);
extern	int	globalMode(int f, int n);
extern  int     addLine(register meLine *ilp, meUByte *text) ;
#define addLineToBob(bp,text) (bp->lineCount += addLine(bp->dotLine,text))
#define addLineToEob(bp,text) (bp->lineCount += addLine(bp->baseLine,text))
extern	int	listBuffers(int f, int n);
extern  int     bufferNeedSaving(meBuffer *bp) ;
extern	int	anyChangedBuffer(void);
extern  meBuffer *createBuffer(register meUByte *bname) ;
#define BFND_CREAT  0x01
#define BFND_BINARY 0x02
#define BFND_CRYPT  0x04
#define BFND_RBIN   0x08
#define BFND_NOHOOK 0x10
#define BFND_MKNAM  0x20
#define BFND_CLEAR  0x40

extern	meBuffer *bfind(meUByte *bname, int cflag);
extern	void    resetBufferWindows(meBuffer *bp) ;
extern	int	bclear(meBuffer *bp);
extern  int     getBufferInfo(meInt *,meInt *,meInt *,meInt *) ;

/* crypt.c externals */
#if MEOPT_CRYPT
extern  int	setBufferCryptKey(meBuffer *bp, meUByte *key) ;
extern	int	setCryptKey(int f, int n);
extern	int	meCrypt(meUByte *bptr, meUInt len);
#else
#define setCryptKey notAvailable
#define meCrypt notAvailable
#endif

/* dirlist.c externals */
#if MEOPT_DIRLIST
extern int directoryTree(int f, int n);
#else
#define directoryTree notAvailable
#endif

/* display.c externals */
#if MEOPT_COLOR
extern  int	addColor(int f, int n) ;
extern	int	addColorScheme(int f, int n);
#else
#define addColor notAvailable
#define addColorScheme notAvailable
#endif
extern  int     convertUserScheme(int n, int defaultScheme);
extern  meUByte assessModeLine(meUByte *ml) ;
extern  void    windCurLineOffsetEval(meWindow *wp) ;
extern  void    reframe(meWindow *wp) ;
extern  void    updCursor(register meWindow *wp) ;
extern  int     renderLine(meUByte *s, int len, int wid, meBuffer *bp);
extern	int	screenUpdate(int f, int n);
extern	int	update(int force);
extern	void	updone(meWindow *wp);
extern	void	updall(meWindow *wp);
#if MEOPT_EXTENDED
extern	int     showCursor(int f, int n) ;
extern	int     showRegion(int f, int n) ;
#else
#define showCursor notAvailable
#define showRegion notAvailable
#endif

#define POKE_NOMARK   0x01      /* Don't mark the poke */
#define POKE_NOFLUSH  0x02      /* Don't flush the poke */
#define POKE_COLORS   0x04      /* The fore & back args are color strings */
extern  void    pokeScreen(int flags, int row, int col, meUByte *scheme, meUByte *str) ;
#if MEOPT_EXTENDED
extern	int	screenPoke(int f, int n) ;
#else
#define screenPoke notAvailable
#endif
extern  void    menuline(int flag);

extern  int     renderLine(meUByte *s1, int len, int wid, meBuffer *bp) ;

/* Virtual video interfaces */
extern  int     meVideoAttach(meVideo *vvptr, meWindow *wp);
extern  void    meVideoDetach(meWindow *wp);

extern  int     doRedrawEvent(void);

#define MWCURSOR    0x001
#define MWSPEC      0x002
#define MWABORT     0x004
#define MWPAUSE     0x008
#define MWWAIT      0x010
#define MWCLEXEC    0x020
#define MWCLWAIT    0x040
#define MWUSEMLCOL  0x080
#define MWERASE     0x100
#define MWSTDOUTWRT 0x200
#define MWSTDERRWRT 0x400
#define MWSTDALLWRT 0x600

extern  int     mlwrite(int flags, meUByte *fmt, ...) ;
#ifdef _WIN32
#ifdef _ME_WINDOW
#define mePrintMessage(mm) MessageBox(NULL,(char *) mm,ME_FULLNAME " '" meVERSION,MB_OK);
#else
#define mePrintMessage(mm) do{ int dummyInt; WriteFile(GetStdHandle(STD_ERROR_HANDLE),mm,meStrlen(mm),&dummyInt,NULL); } while(0)
#endif
#else
#define mePrintMessage(mm) write(2,mm,meStrlen(mm))
#endif

/* eval.c externals */
#define meRSTRCMP_ICASE      0x01
#define meRSTRCMP_BEGBUFF    0x02
#define meRSTRCMP_ENDBUFF    0x04
#define meRSTRCMP_MATCHWHOLE 0x08
#define meRSTRCMP_WHOLE      (meRSTRCMP_BEGBUFF|meRSTRCMP_ENDBUFF|meRSTRCMP_MATCHWHOLE)
#define meRSTRCMP_USEMAIN    0x10
extern  int     regexStrCmp(meUByte *str, meUByte *reg, int flags) ;
extern	meUByte *gtfun(meUByte *fname);
extern  meUByte *getUsrLclCmdVar(meUByte *vname, register meVarList *varList) ;
#define getUsrVar(vname) getUsrLclCmdVar(vname,&usrVarList)
extern	meVariable *SetUsrLclCmdVar(meUByte *vname, meUByte *vvalue,
                                           register meVarList *varList) ;
extern	int	setVar(meUByte *vname, meUByte *vvalue, meRegister *regs) ;
extern	int	setVariable(int f, int n);
extern	meUByte *meItoa(int i);
#define mePtos(pp) (((pp) == NULL) ? emptym:(pp))
extern	meUByte *getval(meUByte *token);
extern	int	descVariable(int f, int n);
#if MEOPT_EXTENDED
extern	int	listVariables(int f, int n);
extern	int	unsetVariable(int f, int n);
#else
#define listVariables notAvailable
#define unsetVariable notAvailable
#endif

/* exec.c externals */
extern	int	mePushRegisters(int flags);
extern	int	mePopRegisters(int flags);
extern  int     biChopFindString(register meUByte *ss, register int len, 
                                        register meUByte **tbl, register int size) ;
extern	int	execFunc(int index, int f, int n) ;
extern  void    execFuncHidden(int keyCode, int index, meUInt arg) ;
#define meEBF_ARG_GIVEN   0x01
#define meEBF_HIDDEN      0x02
#define meEBF_USE_B_DOT   0x04
extern  int     execBufferFunc(meBuffer *bp, int index, int flags, int n) ;
extern  int     lineExec(int f, int n, meUByte *cmdstr);
/* Note  tok (the destination token string) must be meTOKENBUF_SIZE_MAX in size, 
 * returning a string no bigger than meBUF_SIZE_MAX with the \0 */
extern	meUByte  *token(meUByte *src, meUByte *tok);
extern	int	macarg(meUByte *tok);
extern  int     meGetString(meUByte *prompt, int option, int defnum,
                               meUByte *buffer, int size);
extern	int	storemac(int f, int n);
extern	int	execFile(meUByte *fname, int f, int n);
extern	int	executeNamedCommand(int f, int n);
extern	int	executeLine(int f, int n);
extern	int	executeBuffer(int f, int n);
extern	int	executeFile(int f, int n);

/* file.c externals */
extern  int fnamecmp(meUByte *f1, meUByte *f2) ;
#define meFILETYPE_NASTY      0
#define meFILETYPE_REGULAR    1
#define meFILETYPE_DIRECTORY  2
#define meFILETYPE_NOTEXIST   3
#define meFILETYPE_HTTP       4
#define meFILETYPE_FTP        5
extern  int getFileStats(meUByte *file, int flag, meStat *stats, meUByte *lname) ;
extern  int mePathAddSearchPath(int index, meUByte *path_name,
                                meUByte *path_base, int *gotUserPath) ;
#define meFL_CHECKDOT    0x01
#define meFL_USESRCHPATH 0x02
#define meFL_USEPATH     0x04
#define meFL_EXEC        0x08
extern	int fileLookup(meUByte *fname, meUByte *ext, meUByte flags, meUByte *outName) ;
extern	int executableLookup(meUByte *fname, meUByte *outName) ;
extern  int bufferOutOfDate(meBuffer *bp) ;
extern	meUByte *gwd(meUByte drive);
extern  meUByte *getFileBaseName(meUByte *fname) ;
extern  void  getFilePath(meUByte *fname, meUByte *path) ;
extern  int inputFileName(meUByte *prompt, meUByte *fn, int corFlag);
extern	int meBufferInsertFile(meBuffer *bp, meUByte *fname, meUInt flags,
                               meUInt hoffest, meUInt loffest, meInt length);
extern	int insertFile(int f, int n);
extern  int findFileList(meUByte *seed, int bflag, meInt lineno, meUShort colno) ;
extern  int findSwapFileList(meUByte *seed, int bflag, meInt lineno, meUShort colno) ;
extern	int findFile(int f, int n);
extern	int readFile(int f, int n);
extern	int viewFile(int f, int n);
extern  void freeFileList(int noStr, meUByte **files) ;
#if MEOPT_CRYPT
extern	int	resetkey(meBuffer *bp);
#endif
extern	int	readin(meBuffer *, meUByte *fname);
extern	void    makename(meUByte *bname, meUByte *fname);
extern  void    autowriteout(register meBuffer *bp) ;
extern  void    autowriteremove(meBuffer *bp) ;
#if MEOPT_EXTENDED
extern	int     nextWndFindFile(int f, int n);
extern	int     fileOp(int f, int n);
extern	int	appendBuffer(int f, int n);
#else
#define nextWndFindFile notAvailable
#define fileOp notAvailable
#define appendBuffer notAvailable
#endif
extern	int	saveBuffer(int f, int n);
extern	int     saveSomeBuffers(int f, int n) ;
extern	int	writeBuffer(int f, int n);
extern  int     writeOut(register meBuffer *bp, meUInt flags, meUByte *fn) ;
extern	void	resetBufferNames(meBuffer *bp, meUByte *fname);
extern	int	changeFileName(int f, int n);
#ifdef _CONVDIR_CHAR
extern  void    fileNameConvertDirChar(meUByte *fname) ;
#else
#define fileNameConvertDirChar(ff)
#endif
extern  void    fileNameSetHome(meUByte *ss) ;
#define PATHNAME_COMPLETE 0
#define PATHNAME_PARTIAL  1
extern  void    pathNameCorrect(meUByte *oldName, int nameType, 
                                       meUByte *newName, meUByte **baseName) ;
#ifdef _WIN32
extern  void    fileNameCorrect(meUByte *oldName, meUByte *newName, meUByte **baseName) ;
#else
#define fileNameCorrect(o,n,b) pathNameCorrect(o,PATHNAME_COMPLETE,n,b)
#endif
extern  void    getDirectoryList(meUByte *pathName, meDirList *dirList) ;

/* fileio.c externals */
#define meRWFLAG_SILENT     0x00001
#define meRWFLAG_READ       0x00002
#define meRWFLAG_INSERT     0x00004
#define meRWFLAG_WRITE      0x00008
#define meRWFLAG_BACKUP     0x00010
#define meRWFLAG_OPENEND    0x00020
#define meRWFLAG_OPENTRUNC  0x00040
#define meRWFLAG_AUTOSAVE   0x00080
#define meRWFLAG_CHKBREAK   0x00100
#define meRWFLAG_IGNRNRRW   0x00200
#define meRWFLAG_CRYPT      0x00400
/* following used in ffFileOp to remove files & create a dir etc */
#define meRWFLAG_DELETE     0x00800
#define meRWFLAG_MKDIR      0x01000
#define meRWFLAG_FTPCLOSE   0x02000
#define meRWFLAG_NOCONSOLE  0x04000
#define meRWFLAG_FTPNLST    0x08000
#define meRWFLAG_NODIRLIST  0x10000
#define meRWFLAG_STAT       0x20000
#define meRWFLAG_PRESRVTS   0x40000
#define meRWFLAG_PRESRVFMOD 0x80000

extern int      ffReadFile(meUByte *fname, meUInt flags, meBuffer *bp, meLine *hlp,
                           meUInt uoffset, meUInt loffset, meInt length) ;
#define meBACKUP_CREATE_PATH 0x0001
extern int      createBackupName(meUByte *filename, meUByte *fn, meUByte backl, int flag) ;


extern int      ffWriteFileOpen(meUByte *fname, meUInt flags, meBuffer *bp) ;
extern int      ffWriteFileWrite(register int len, 
                                        register meUByte *buff, int eolFlag) ;
extern int      ffWriteFileClose(meUByte *fname, meUInt flags, meBuffer *bp) ;
extern int      ffWriteFile(meUByte *fname, meUInt flags, meBuffer *bp) ;
#if MEOPT_EXTENDED
extern int	ffFileOp(meUByte *sfname, meUByte *dfname, meUInt dFlags, meInt fileMode) ;
#else
#define ffFileOp(s,d,f,m) 0
#endif

/* frame.c externals */
extern	int     meFrameChangeWidth(meFrame *frame, int ww);
extern	int     meFrameChangeDepth(meFrame *frame, int hh);
extern	int	frameChangeDepth(int f, int n);
extern	int	frameChangeWidth(int f, int n);
extern	meFrame *meFrameInit(meFrame *sibling) ;
extern	int     meFrameInitWindow(meFrame *frame, meBuffer *buffer) ;
#if MEOPT_FRAME
extern	int	meFrameDelete(meFrame *frame, int flags);
extern	void	meFrameMakeCur(meFrame *frame, int quiet);
extern	int	frameCreate(int f, int n);
extern	int	frameDelete(int f, int n);
extern	int	frameNext(int f, int n);

#define meFrameLoopBegin()                                                   \
do {                                                                         \
    meFrame *loopFrame=frameList;                                            \
    for(loopFrame=frameList ; loopFrame!=NULL ; loopFrame=loopFrame->next)   \
    {

#define meFrameLoopContinue(cond)                                            \
if( cond ) continue

#define meFrameLoopBreak(cond)                                               \
if( cond ) break

#define meFrameLoopEnd()                                                     \
    }                                                                        \
} while(0)

#else
#define frameCreate notAvailable
#define frameDelete notAvailable
#define frameNext notAvailable

#define loopFrame frameCur
#define meFrameLoopBegin()
#define meFrameLoopContinue(cond)
#define meFrameLoopBreak(cond)
#define meFrameLoopEnd()

#endif

/* input.c externals */
#define mlCR_LOWER_CASE     0x01
#define mlCR_QUIT_ON_USER   0x02
#define mlCR_UPDATE_ON_USER 0x04
#define mlCR_CURSOR_IN_MAIN 0x08
#define mlCR_QUOTE_CHAR     0x10
#define mlCR_ALPHANUM_CHAR  0x20
extern	int     mlCharReply(meUByte *prompt, int mask, meUByte *validList, meUByte *helpStr) ;
extern	int	mlyesno(meUByte *prompt);
extern	void	mlDisp(meUByte *prompt, meUByte *buf, meUByte *cont, int cpos) ;
extern	int	getexecCommand(void);
#define meGETKEY_SILENT     0x01
#define meGETKEY_SINGLE     0x02
#define meGETKEY_COMMAND    0x04
extern	meUShort  meGetKeyFromUser(int f, int n, int flag);
extern  int     createBuffList(meUByte ***listPtr, int noHidden) ;
extern  int     createCommList(meUByte ***listPtr, int noHidden) ;
extern  int     createVarList(meUByte ***listPtr) ;
#define MLBUFFER     0x00001		/* entering a buffer name	     */
#define MLCOMMAND    0x00002		/* entering a command		     */
#define MLFILE       0x00004		/* entering a filename		     */
#define MLSEARCH     0x00008		/* entering a search string          */
#define	MLVARBL      0x00010		/* entering a variable               */
#define	MLUSER       0x00020		/* entering a mode                   */
#define MLNOHIST     0x00040            /* Don't use the history (wont store */
#define MLUPPER      0x00080		/* convert chars to uppercase	     */
#define MLLOWER      0x00100		/* convert chars to lowercase	     */
#define	MLNOSPACE    0x00200		/* no spaces allowed		     */
#define	MLNOSTORE    0x00400		/* no history storing		     */
#define	MLISEARCH    0x00800		/* Special incremental search mode   */
#define	MLNORESET    0x01000		/* don't reset the buffer            */
#define	MLMACHIST    0x02000		/* Add output to history even if mac */
#define	MLMACNORT    0x04000		/* dont reset the buffer even if mac */
#define MLINSENSCASE 0x08000            /* ignore letter case in completion  */
#define MLFFZERO     0x10000            /* en/disable \x00 -> \xff encode    */
#define MLHIDEVAL    0x20000            /* Hide the value using '*'s         */
#define MLEXECNOUSER 0x40000            /* Don't get val from user if macro  */

/* setup a #define with the correct flags for getting a filename, this must
 * consider where the platform file system only has one case and where it
 * is case insensitive
 */
#ifdef _SINGLE_CASE
/* Typically dos */
#define MLFILECASE   (MLFILE|MLLOWER)
#define MLBUFFERCASE MLBUFFER
#else
#ifdef _INSENSE_CASE
/* Typically windows */
#define MLFILECASE   (MLFILE|MLINSENSCASE)
#define MLBUFFERCASE (MLBUFFER|MLINSENSCASE)
#else
/* Typically unix */
#define MLFILECASE   MLFILE
#define MLBUFFERCASE MLBUFFER
#endif
#endif
/* Following must be setup when using MLUSER, mlgsStrList must be an
 * array of completion strings, and mlgsStrListSize must be a count
 * of the number of completions. Not the completion array is sorted!
 */
extern meUByte **mlgsStrList ;
extern int mlgsStrListSize ;
extern int meGetStringFromUser(meUByte *prompt, int option, int defnum, meUByte *buf, int nbuf) ;

/* hilight.c externals */
extern	void	mlerase(int flag);
#if MEOPT_HILIGHT
extern	int	hilight(int f, int n);
extern  void    hilightCurLineOffsetEval(meWindow *wp) ;
extern  int     indentLine(int *inComment) ;
extern  meUShort hilightLine(meVideoLine *vp1, meUByte mode) ;
extern  void    hilightLookBack(meWindow *) ;
extern	int	meIndentGetIndent(meUByte indent, meUShort bIndentWidth) ;
extern	int	indent(int f, int n);
#else
#define hilight notAvailable
#define indent notAvailable
#endif

/* history.c externals */
extern  void    initHistory(void) ;
extern  int     setupHistory(int option, meUByte **numPtr, meUByte ***list) ;
extern  void    addHistory(int option, meUByte *str) ;

/* isearch.c externals */
#if MEOPT_ISEARCH
extern	int	isearchBack(int f, int n);
extern	int	isearchForw(int f, int n);
#else
#define isearchBack notAvailable
#define isearchForw notAvailable
#endif

/* key.c externals */
extern  void    count_key_table(void) ;
extern  int     decode_key(register meUShort code, meUInt *arg) ;
extern  int     delete_key(register meUShort code) ;
extern  int	insert_key(register meUShort code, meUShort index, meUInt arg) ;

/* line.c externals */
extern  meLine *meLineMalloc(int length, int editLine) ;
#define meLINEANCHOR_ALWAYS     0x00
#define meLINEANCHOR_IF_LESS    0x01
#define meLINEANCHOR_IF_GREATER 0x02
#define meLINEANCHOR_RETAIN     0x00
#define meLINEANCHOR_FIXED      0x10
#define meLINEANCHOR_RELATIVE   0x20
#define meLINEANCHOR_COMPRESS   0x40
extern  void	meLineResetAnchors(meInt flags, meBuffer *bp, meLine *lp,
                                   meLine *nlp, meUShort offset, meInt adjust) ;
extern	int	bufferSetEdit(void);
extern	void	lineSetChanged(int flag);
extern  meUByte *lineMakeSpace(int n);
extern	int	lineInsertChar(int n, int c);
extern	int	lineInsertString(int n, meUByte *cp);
#define meBUFINSFLAG_UNDOCALL   0x01
#define meBUFINSFLAG_LITERAL    0x02
extern	int	lineInsertNewline(meInt flags);
extern	int     bufferInsertText(meUByte *str, meInt flags) ;
extern	int	bufferInsertSpace(int f, int n);
extern	int	bufferInsertTab(int f, int n);
extern	int	bufferInsertString(int f, int n);
extern	int	bufferInsertNewline(int f, int n);
extern	int	mldelete(meInt n, meUByte *kstring);
extern	int	ldelete(meInt n, int kflag);
extern	int	killSave(void);
extern	meUByte *killAddNode(meInt count);
extern	int	yankfrom(struct meKill *pklist);
extern	int	yank(int f, int n);
extern	int	reyank(int f, int n);
extern  void    meLineLoopFree(meLine *lp, int flag) ;

/* macro.c externals */
extern	int	macroDefine(int f, int n);
#if MEOPT_EXTENDED
extern	int	macroFileDefine(int f, int n);
extern	int	macroHelpDefine(int f, int n);
extern  void    helpBufferReset(meBuffer *bp);
extern	int	help(int f, int n);
extern	int	helpItem(int f, int n);
extern  int	helpCommand(int f, int n) ;
extern  int	helpVariable(int f, int n) ;
extern  int     nameKbdMacro(int f, int n) ;
extern	int	macroQuery(int f, int n);
extern  meMacro *userGetMacro(meUByte *buf, int len) ;
extern  int     insMacro(int f, int n) ;
#else
#define macroFileDefine notAvailable
#define macroHelpDefine notAvailable
#define help notAvailable
#define helpItem notAvailable
#define helpCommand notAvailable
#define helpVariable notAvailable
#define nameKbdMacro notAvailable
#define macroQuery notAvailable
#define insMacro notAvailable
#endif
extern	int	startKbdMacro(int f, int n);
extern	int	endKbdMacro(int f, int n);
extern	int	executeKbdMacro(int f, int n);
extern	int	stringExec(int f, int n, meUByte *macro) ;
extern	int	executeString(int f, int n);

/* main.c externals */
extern  int     insertChar(register int c, register int n) ;
extern  void    doOneKey(void) ;
extern	int	execute(int c, int f, int n);
extern	int	meAbout(int f, int n);
extern	int	exitEmacs(int f, int n);
extern	int	quickExit(int f, int n);
extern  int     promptSaveAll(int f, int n) ;
extern	int	saveExitEmacs(int f, int n);
extern	int	ctrlg(int f, int n);
extern	int	notAvailable(int f, int n);
extern  int     noMarkSet(void) ;
extern	int	rdonly(void);
extern	int     voidFunc(int f, int n) ;
extern	int	resterr(void);
extern	int	prefixFunc(int f, int n);
extern	int	uniArgument(int f, int n);
extern  void    mesetup(int argc, char *argv[]);
#if (defined _UNIX) || (defined _WIN32)
extern  int     meDie(void) ;
#endif
extern  void    autoSaveHandler(void) ;
#if MEOPT_EXTENDED
extern	int     commandWait(int f, int n) ;
#else
#define commandWait notAvailable
#endif

#ifndef NDEBUG
extern  void    _meAssert(char *file, int line);
#define meAssert(x) do{if(!(x)){_meAssert(__FILE__,__LINE__);}}while(0)
#else
#define meAssert(x) /* Nothing (x) */
#endif

/* narrow.c externals */
#if MEOPT_NARROW
extern meNarrow *
meBufferCreateNarrow(meBuffer *bp, meLine *slp, meLine *elp, meInt sln, meInt eln,
                     meInt name, meScheme scheme, meUByte *markupLine, meInt markupCmd, meInt undoCall) ;
extern void
meBufferRemoveNarrow(meBuffer *bp, register meNarrow *nrrw, meUByte *firstLine, meInt undoCall) ;
extern void  meBufferExpandNarrowAll(meBuffer *bp) ;
extern void  meBufferCollapseNarrowAll(meBuffer *bp) ;
extern meInt meBufferRegionExpandNarrow(meBuffer *bp, meLine **startLine, meUShort soffset,
                                        meLine *endLine, meUShort eoffset, meInt remove) ;
extern int   narrowBuffer(int f, int n) ;
#else
#define narrowBuffer notAvailable
#endif

/* next.c externals */
#if MEOPT_FILENEXT
extern int      getNextLine(int f, int n) ;
extern int      addNextLine(int f, int n) ;
#else
#define getNextLine notAvailable
#define addNextLine notAvailable
#endif
#if MEOPT_RCS
extern int      rcsFilePresent(meUByte *fname) ;
extern int      doRcsCommand(meUByte *fname, register meUByte *comStr) ;
extern int      rcsCiCoFile(int f, int n) ;
#else
#define rcsCiCoFile notAvailable
#endif
#if (defined _XTERM) || (defined _DOS) || (defined _WIN32)
extern int      changeFont(int f, int n);
#else
#define changeFont notAvailable
#endif

/* osd.c externals */
#if MEOPT_OSD
extern void     osdStoreAll(void) ;
extern void     osdRestoreAll(int) ;
extern void     osdDisp(meUByte *buf, meUByte *cont, int cpos) ;
extern int      osdDisplayMouseLocate(int leftPick) ;
extern int      osdMainMenuCheckKey(int cc) ;
extern int      osd(int f, int n);
extern void     osdMainMenuUpdate(int force) ;
#if MEOPT_LOCALBIND
extern	int	osdBindKey(int f, int n);
extern	int	osdUnbindKey(int f, int n);
#else
#define osdBindKey notAvailable
#define osdUnbindKey notAvailable
#endif
#else
#define osd notAvailable
#define osdBindKey notAvailable
#define osdUnbindKey notAvailable
#endif

/* print.c externals */
#if MEOPT_PRINT
extern  int     printColor(int f, int n);
extern  int     printScheme(int f, int n);
extern	int	printBuffer(int f, int n) ;
extern	int	printRegion(int f, int n) ;
#ifdef _WIN32
extern  int     WinPrint(meUByte *name, meLine *phead);
#endif /* _WIN32 */
#else
#define printColor notAvailable
#define printScheme notAvailable
#define printBuffer notAvailable
#define printRegion notAvailable
#endif /* PRINT */

/* random.c externals */
#ifdef _ME_USE_STD_MALLOC
#define meMalloc  malloc
#define meRealloc realloc
#define meStrdup  strdup
#else
extern  void   *meMalloc(size_t s) ;
extern  void   *meRealloc(void *, size_t s) ;
extern  void   *meStrdup(const meUByte *s) ;
#endif
extern  void    meStrrep(meUByte **d, const meUByte *s) ;
extern  int     meStricmp(const meUByte *str1, const meUByte *str2) ;
extern  int     meStrnicmp(const meUByte *str1, const meUByte *str2, size_t nn) ;
extern  int     meStridif(const meUByte *str1, const meUByte *str2) ;
extern  void    sortStrings(int noStr, meUByte **strs, int offset, meIFuncSS cmpFunc) ;
extern  int     sortLines(int f, int n) ;
extern	int	bufferInfo(int f, int n);
extern	int	getcline(meWindow *wp);
extern	int	getcol(meUByte *ss, int off, int tabWidth);
#define getccol() getcol(frameCur->windowCur->dotLine->text,\
                         frameCur->windowCur->dotOffset,\
                         frameCur->windowCur->buffer->tabWidth)
extern	int	setccol(int pos);
extern	int	getcwcol(void);
extern	int	setcwcol(int pos);
extern	int	transChars(int f, int n);
extern	int	transLines(int f, int n);
extern	int	quoteKeyToChar(meUShort c) ;
extern	int	quote(int f, int n);
extern	int	meTab(int f, int n);
#if MEOPT_EXTENDED
extern	int	meBacktab(int f, int n);
extern	int	windowDeleteBlankLines(int f, int n);
#else
#define meBacktab notAvailable
#define windowDeleteBlankLines notAvailable
#endif
extern	int	meLineSetIndent(int curInd, int newInd, int undo);
extern	int	meNewline(int f, int n);
extern	int	forwDelChar(int f, int n);
extern	int	backDelChar(int f, int n);
extern	int	killLine(int f, int n);
extern	int	mlWrite(int f, int n);
extern  void    makestrlow(meUByte *str);
#if MEOPT_FENCE
extern  meUByte gotoFrstNonWhite(void) ;
extern	int	gotoFence(int f, int n);
#else
#define gotoFence notAvailable
#endif
#if MEOPT_HILIGHT
extern  int     doCindent(meHilight *indentDef, int *inComment) ;
extern	int	indentInsert(void) ;
#endif
#if MEOPT_WORDPRO
extern  int	winsert(void) ;
#endif

extern  int     meAnchorSet(meBuffer *bp, meInt name,
                            meLine *lp, meUShort off, int silent) ;
extern  int     meAnchorGet(meBuffer *bp, meInt name) ;
extern  int     meAnchorDelete(meBuffer *bp, meInt name) ;
extern	int	setAlphaMark(int f, int n);
extern	int	gotoAlphaMark(int f, int n);
extern  int     insFileName(int f, int n) ;
#if MEOPT_EXTENDED
extern  int     cmpBuffers(int f, int n) ;
#else
#define cmpBuffers notAvailable
#endif
#if MEOPT_CALLBACK
extern  int     createCallback(int f, int n) ;
extern  void    callBackHandler(void) ;
#else
#define createCallback notAvailable
#endif
#if MEOPT_MOUSE
extern  int     setCursorToMouse(int f, int n) ;
#else
#define setCursorToMouse notAvailable
#endif

/* region.c externals */
extern	int	getregion(meRegion *rp);
extern	int	killRegion(int f, int n);
extern	int	copyRegion(int f, int n);
extern	int	lowerRegion(int f, int n);
extern	int	upperRegion(int f, int n);
#if MEOPT_EXTENDED
extern	int	killRectangle(int f, int n);
extern	int	yankRectangle(int f, int n);
#else
#define killRectangle notAvailable
#define yankRectangle notAvailable
#endif

/* registry.c externals */
#if MEOPT_REGISTRY
extern int deleteRegistry(int f, int n);
extern int findRegistry(int f, int n);
extern int listRegistry(int f, int n);
extern int markRegistry(int f, int n);
extern int readRegistry(int f, int n);
extern int saveRegistry(int f, int n);
extern int setRegistry(int f, int n);

/* API registry prototypes made available for export */
extern meRegNode *regFind(meRegNode *root, meUByte *subkey);
#define regGetChild(reg) (reg->child)
#define regGetNext(reg)  (reg->next)
extern meRegNode *regRead(meUByte *rname, meUByte *fname, int mode);
extern meRegNode *regSet(meRegNode *root, meUByte *subkey, meUByte *value);
extern meRegNode *vregFind(meRegNode *root, meUByte *fmt, ...);
extern int  regDelete(meRegNode *root);
extern int  regSave(meRegNode *root, meUByte *fname, int mode);
extern int  anyChangedRegistry(void);
#define regGetName(reg)       (reg->name)
#define regGetValue(reg)      (reg->value)
#define regGetValueLen(reg)   (reg->len)
#define regGetLong(reg,def)   ((reg->value==NULL) ? def:meAtoi(reg->value))
#define regGetString(reg,def) ((reg->value==NULL) ? def:reg->value)

#else
#define deleteRegistry notAvailable
#define findRegistry notAvailable
#define listRegistry notAvailable
#define markRegistry notAvailable
#define readRegistry notAvailable
#define saveRegistry notAvailable
#define setRegistry notAvailable
#endif

/* search.c externals */
#define meEXPAND_BACKSLASH 0x01
#define meEXPAND_FFZERO    0x02
#define meEXPAND_PRINTABLE 0x04
extern  int     expandchar(int c, meUByte *d, int flags) ;
extern  int     expandexp(int slen, meUByte *s, int dlen, int doff,
                                 meUByte *d, int cpos, int *opos, int flags) ;
extern	int	eq(int bc, int pc);
extern	int	searchForw(int f, int n);
extern	int	huntForw(int f, int n);
extern	int	searchBack(int f, int n);
extern	int	huntBack(int f, int n);
extern	int	replaceStr(int f, int n);
extern	int	queryReplaceStr(int f, int n);
extern	int	searchBuffer(int f, int n);

/* spawn.c externals */
#ifdef _UNIX
extern  void    __mkTempName(meUByte *buf, meUByte *name);
#define mkTempName(bb,nn,ee) __mkTempName((bb),(nn))
#else
extern  void    __mkTempName(meUByte *buf, meUByte *name, meUByte *ext);
#define mkTempName(bb,nn,ee) __mkTempName((bb),(nn),(ee))
#endif
#ifdef _WIN32
extern void mkTempCommName(meUByte *filename, meUByte *basename) ;
#else
#define mkTempCommName(filename,basename) mkTempName(filename,basename,NULL)
#endif
#if MEOPT_SPAWN

#define COMMAND_FILE         "stdout.~~~"
#define DUMMY_STDIN_FILE     "stdin.~~~" 

#define LAUNCH_BUFFERNM      0x00001      /* Use *command* buffer      */
#define LAUNCH_WAIT          0x00001      /* shell-com wait -> LAUNCH_NOWAIT */
#define LAUNCH_SILENT        0x00002      /* Hide the output buffer    */
#define LAUNCH_NOCOMSPEC     0x00004      /* Do not use the comspec    */
#define LAUNCH_DETACHED      0x00008      /* Detached process launch   */
#define LAUNCH_LEAVENAMES    0x00010      /* Leave the names untouched */
#define LAUNCH_SHOWWINDOW    0x00020      /* Dont hide the new cmd wdw */
#define LAUNCH_RAW           0x00040      /* Raw pipe output           */
#define LAUNCH_BUFIPIPE      0x00080      /* Ipipe function provided   */
#define LAUNCH_BUFCMDLINE    0x00100      /* cmd to run is 1st line of buf */
#define LAUNCH_NO_WRAP       0x00200      /* Run without wrap mode     */
#define LAUNCH_USER_FLAGS    0x002FE      /* User flags bitmask        */
#define LAUNCH_SHELL         0x01000
#define LAUNCH_SYSTEM        0x02000
#define LAUNCH_FILTER        0x04000
#define LAUNCH_PIPE          0x08000
#define LAUNCH_IPIPE         0x10000
#define LAUNCH_NOWAIT        0x20000
extern	int	meShell(int f, int n);
extern	int	doShellCommand(meUByte *cmdstr, int flags) ;
extern	int	meShellCommand(int f, int n);
extern  int     doPipeCommand(meUByte *comStr, meUByte *path, meUByte *bufName, 
                              int ipipe, int silent) ;
extern	int	pipeCommand(int f, int n);
#if MEOPT_IPIPES
extern	int	ipipeCommand(int f, int n) ;
extern	int	ipipeWrite(int f, int n) ;
extern  void    ipipeRead(meIPipe *ipipe) ;
extern  int     ipipeSetSize(meWindow *wp, meBuffer *bp) ;
extern	void    ipipeRemove(meIPipe *ipipe) ;
#ifdef _UNIX
extern  void    ipipeCheck(void) ;
#endif
extern	int	ipipeKill(int f, int n) ;
extern  int     anyActiveIpipe(void) ;
#else
#define ipipeWriteString voidFunc
#define ipipeCommand  pipeCommand
#define ipipeWrite voidFunc
#define ipipeKill voidFunc
#endif

#if MEOPT_EXTENDED
#define FILTER_IN_FILE    "fltinp.~~~"
#define FILTER_OUT_FILE   "fltout.~~~"
extern	int	meFilter(int f, int n);
#ifdef _UNIX
extern	int	suspendEmacs(int f, int n);
#else
#define suspendEmacs notAvailable
#endif
#else
#define meFilter notAvailable
#define suspendEmacs notAvailable
#endif

#else

#define meShell notAvailable
#define meShellCommand notAvailable
#define pipeCommand notAvailable
#define ipipeWriteString notAvailable
#define ipipeCommand  notAvailable
#define ipipeWrite notAvailable
#define ipipeKill notAvailable
#define meFilter notAvailable
#define suspendEmacs notAvailable

#endif /* MEOPT_SPAWN */

/* spell.c externals */
#if MEOPT_SPELL
extern	int	dictionaryAdd(int f, int n);
extern	int	spellRuleAdd(int f, int n);
extern	int	dictionaryDelete(int f, int n);
extern	int	dictionarySave(int f, int n);
extern	int	spellWord(int f, int n);
extern  int     anyChangedDictionary(void) ;
#else
#define dictionaryAdd notAvailable
#define spellRuleAdd notAvailable
#define dictionaryDelete notAvailable
#define dictionarySave notAvailable
#define spellWord notAvailable
#endif
extern	void	findWordsInit(meUByte *mask);
extern	meUByte  *findWordsNext(void);

/* tag.c externals */
#if MEOPT_TAGS
extern	int	findTag(int f, int n);
#else
#define findTag notAvailable
#endif

/* termio.c externals */
extern	void    TTdoBell(int) ;
extern	void    TTbell(void);
extern  int     charListToShorts(meUShort *sl, meUByte *cl) ;
extern  int     keyListToShorts(meUShort *sl, meUByte *kl) ;
extern  void    translateKeyAdd(meTRANSKEY *tcapKeys, int count, int time,
                                       meUShort *key, meUShort map) ;
extern	int	translateKey(int f, int n);
extern  char   *meTParm(char *str, ...) ;

/* time.c externals */
extern	int	set_timestamp(meBuffer *bp);

/* undo.c externals */
#if MEOPT_UNDO
extern  meUndoNode *meUndoCreateNode(size_t size) ;
extern	void	meUndoAddInsChar(void);
extern	void	meUndoAddDelChar(void);
extern  void    meUndoAddRepChar(void);
extern	void	meUndoAddInsChars(meInt numChars);
extern	void	meUndoAddDelChars(meInt numChars);
extern	void	meUndoAddReplaceBgn(meLine *elinep, meUShort elineo);
extern	void	meUndoAddReplaceEnd(meInt numChars);
extern	void    meUndoAddReplace(meUByte *dstr, meInt count) ;
extern  meInt  *meUndoAddLineSort(meInt lineCount) ;
#if MEOPT_NARROW
extern  void    meUndoAddUnnarrow(meInt sln, meInt eln, meInt name, meScheme scheme,
                                  meInt markupCmd, meLine *markupLine) ;
extern  void    meUndoAddNarrow(meInt sln, meInt name,
                                meInt markupCmd, meLine *firstLine) ;
#endif
extern	void	meUndoRemove(meBuffer *bp) ;
extern	int	meUndo(int f, int n);
#else
#define meUndo notAvailable
#endif

/* window.c externals */
extern  void    meWindowMakeCurrent(meWindow *wp) ;
extern  void    frameAddModeToWindows(int mode) ;
extern  void    meBufferAddModeToWindows(meBuffer *bp, int mode) ;
extern  void    meWindowFixTextSize(meWindow *wp);
extern  void    meWindowFixScrollBars(meWindow *wp);
extern  void    meWindowFixScrollBox(meWindow *wp);
extern  int     frameSetupMenuLine(int n);
extern	int	windowRecenter(int f, int n);
extern	int	windowGotoNext(int f, int n);
extern	int	windowGotoPrevious(int f, int n);
extern	int	windowScrollDown(int f, int n);
extern  int     windowScrollLeft(int f, int n);
extern  int     windowScrollRight(int f, int n);
extern	int	windowScrollUp(int f, int n);
extern	int	windowDeleteOthers(int f, int n);
extern	int	windowDelete(int f, int n);
#define meFRAMERESIZEWIN_WIDTH  0x01
#define meFRAMERESIZEWIN_DEPTH  0x02
extern  int     meFrameResizeWindows(meFrame *frame, int flags);
extern  int     frameResizeWindows(int f, int n);
extern	int	windowSplitDepth(int f, int n);
extern	int	windowChangeDepth(int f, int n);
#if MEOPT_HSPLIT
extern	int	windowSplitWidth(int f, int n);
extern	int	windowChangeWidth(int f, int n);
#else
#define windowSplitWidth notAvailable
#define windowChangeWidth notAvailable
#endif
#if MEOPT_POSITION
extern	int	positionSet(int f, int n);
extern	int	positionGoto(int f, int n);
#else
#define positionSet notAvailable
#define positionGoto notAvailable
#endif
#if MEOPT_MOUSE
extern  int     windowSetScrollWithMouse(int f, int n);
#else
#define windowSetScrollWithMouse notAvailable
#endif
/* uses the bfind flags for finding buffer "name"
 * if not null. Also if WPOP_MKCURR set then 
 * makes the buffer current.
 */
#define WPOP_MKCURR 0x100
#define WPOP_USESTR 0x200
#define WPOP_EXIST  0x400
extern	meWindow *meWindowPopup(meUByte *name, int flags, meBuffer **bufferReplaced);
extern	int	windowPopup(int f, int n);
#if MEOPT_EXTENDED
extern	int	windowScrollNextUp(int f, int n);
extern	int	windowScrollNextDown(int f, int n);
#else
#define windowScrollNextUp notAvailable
#define windowScrollNextDown notAvailable
#endif

/* word.c externals */
extern	int	backWord(int f, int n);
extern	int	forwWord(int f, int n);
extern	int	upperWord(int f, int n);
extern	int	lowerWord(int f, int n);
extern	int	capWord(int f, int n);
extern	int	forwDelWord(int f, int n);
extern	int	backDelWord(int f, int n);
#if MEOPT_WORDPRO
extern	int	wrapWord(int f, int n);
extern	int	justify(int leftMargin, int leftDoto, meUByte jmode);
extern	int	fillPara(int f, int n);
extern	int	killPara(int f, int n);
extern	int	countWords(int f, int n);
#else
#define wrapWord notAvailable
#define fillPara notAvailable
#define killPara notAvailable
#define countWords notAvailable
#endif

/* gettimeofday - Retreives the time of day to millisecond resolution
 *                This is a 4.2 BSD function. IRIX 5.x and Solaris prior to
 *                2.5 provide a single argument version, for backward
 *                compatibility the 2nd argument is ignored. HP-UX 10.x and
 *                versions of Solaris above 2.5 provide a 2 argument version.
 *                Win32 does not provide this function (of course). 
 *                
 *                Jon Green 18th Dec '96.
 */
/* windows has name clashes with timeval, so define an meTimeval */
#ifdef _WIN32

struct meTimeval
{
    int tv_sec;                         /* Time in seconds */
    int tv_usec;                        /* Time in milliseconds */
};

struct meTimezone 
{
    int tz_minuteswest;                 /* Offset (+/-ve) from UTC in minutes */
    int tz_dsttime;                     /* Flag indicating the type of DST correction */
};

extern void gettimeofday (struct meTimeval *tp, struct meTimezone *tz);

#else
#define meTimeval  timeval
#define meTimezone timezone
#endif

#ifdef _WIN32
/* The current win32 rename() function does not cope with long filenames
 * Use the Win32 definition - note that these functions have an oposite
 * boolean sense that the ANSI 'C' definitions, explicitly test to convert
 * to the more familier invocation.
 *
 * Why the hell we need these functions and the ANSI 'C' functions are not
 * valid is beyond belief - typical bloody Microsoft !!
 */
extern int chdir(const char *name) ;
extern int _getdrive(void) ;
#define meChdir(dir)        chdir(dir)
#define meRename(src,dst)   (MoveFile(src,dst)==meFALSE)
#define meUnlink(fn)        (DeleteFile(fn)==meFALSE)
/* Doesn't exist if function returns -1 */
#define meTestExist(fn)     (((int) GetFileAttributes(fn)) < 0)
/* Can't read if doesn't exist or its a directory */
#define meTestRead(fn)      (GetFileAttributes(fn) & FILE_ATTRIBUTE_DIRECTORY)
/* Can't write if exists and its readonly or a directory */
#define meTestWrite(fn)     ((((int) GetFileAttributes(fn)) & 0xffff8001) > 0)
/* File is a directory */
#define meTestDir(fn)       ((GetFileAttributes(fn) & (0xf0000000|FILE_ATTRIBUTE_DIRECTORY)) != FILE_ATTRIBUTE_DIRECTORY)
extern int meTestExecutable(meUByte *fileName) ;
#define meStatTestRead(st)  (((st).stmode & FILE_ATTRIBUTE_DIRECTORY) == 0)
#define meStatTestWrite(st) (((st).stmode & (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_READONLY)) == 0)
#define meStatTestSystem(st) (((st).stmode & FILE_ATTRIBUTE_SYSTEM) == 0)
#define meFileGetAttributes GetFileAttributes
#define meFileSetAttributes SetFileAttributes
extern void WinShutdown (void);
#define meExit(status)      (WinShutdown(), ExitProcess(status))
#endif

#ifdef _DOS
extern int   unlink(const char *file) ;
#define meFILE_ATTRIB_READONLY  0x01
#define meFILE_ATTRIB_HIDDEN    0x02
#define meFILE_ATTRIB_SYSTEM    0x04
#define meFILE_ATTRIB_VOLLABEL  0x08
#define meFILE_ATTRIB_DIRECTORY 0x10
#define meFILE_ATTRIB_ARCHIVE   0x20
extern meInt meFileGetAttributes(meUByte *fn) ;
extern void  _meFileSetAttributes(meUByte *fn, meUShort attr) ;
#define meFileSetAttributes _meFileSetAttributes
extern int   meChdir(meUByte *path) ;
/* Doesn't exist if function returns -1 */
#define meTestExist(fn)     (meFileGetAttributes(fn) < 0)
/* Can't read if doesn't exist or its a directory */
#define meTestRead(fn)      (meFileGetAttributes(fn) & 0x10)
/* Can't write if exists and its readonly or a directory */
#define meTestWrite(fn)     ((meFileGetAttributes(fn) & 0xffff8001) > 0)
/* File is a directory */
#define meTestDir(fn)       ((meFileGetAttributes(fn) & 0xf0000010) != 0x010)
extern int   meTestExecutable(meUByte *fileName) ;
#define meStatTestRead(st)  (((st).stmode & 0x10) == 0)
#define meStatTestWrite(st) (((st).stmode & 0x11) == 0)
#endif

#ifdef _UNIX
/* Define the standard POSIX tests for the file stats */
#ifndef S_IROTH                         /* Other read permission  */
#define S_IROTH  0000004
#endif
#ifndef S_IRGRP                         /* Group read permission */
#define S_IRGRP  0000040
#endif
#ifndef S_IRUSR                         /* User read permission */
#define S_IRUSR  0000400                
#endif
#ifndef S_IWOTH                         /* Other write permission */
#define S_IWOTH  0000002        
#endif
#ifndef S_IWGRP                         /* Group write permission */
#define S_IWGRP  0000020        
#endif
#ifndef S_IWUSR                         /* User write permission */
#define S_IWUSR  0000200                
#endif
#ifndef S_IXOTH                         /* Other exeute permission */
#define S_IXOTH  0000001                
#endif
#ifndef S_IXGRP                         /* Group execute permission */
#define S_IXGRP  0000010                
#endif
#ifndef S_IXUSR                         /* User execute permission */
#define S_IXUSR  0000100
#endif
#ifndef S_IRWXO                         /* Other read/write/execute */
#define S_IRWXO  ((S_IROTH)|(S_IWOTH)|(S_IXOTH))
#endif 
#ifndef S_IRWXU                         /* User read/write/execute */
#define S_IRWXU  ((S_IRUSR)|(S_IWUSR)|(S_IXUSR))
#endif
#ifndef S_IRWXG                         /* Group read/write/exexute */
#define S_IRWXG  ((S_IRGRP)|(S_IWGRP)|(S_IXGRP))
#endif
#ifndef S_ISGID                         /* Set group id on exec */
#define S_ISGID  0002000               
#endif
#ifndef S_ISUID                         /* Set user id on exec */
#define S_ISUID  0004000  
#endif
#ifndef S_IFLNK                         /* Set if symbolic link -- NOT POSIX */
#define S_IFLNK  0120000
#endif
/* Posix test functions */
#ifndef S_ISDIR                         /* Test directory */
#define S_ISDIR(x)  (((x) & 0170000) == 0040000)
#endif
#ifndef S_ISCHR                         /* Test character special file */
#define S_ISCHR(x)  (((x) & 0170000) == 0020000)
#endif
#ifndef S_ISBLK                         /* Test block special file */
#define S_ISBLK(x)  (((x) & 0170000) == 0060000)
#endif
#ifndef S_ISREG                         /* Test regular file */
#define S_ISREG(x)  (((x) & 0170000) == 0100000)
#endif
#ifndef S_ISFIFO                        /* Test FIFO */
#define S_ISFIFO(x) (((x) & 0170000) == 0010000)
#endif
#ifndef S_ISLNK                         /* Test symbolic link -- NOT POSIX */
#define S_ISLNK(x)  (((x) & 0170000) == (S_IFLNK))
#endif

/* File modes defined in terms of POSIX tests */
extern meInt meFileGetAttributes(meUByte *fn) ;
extern int meGidInGidList(gid_t gid) ;
#define meStatTestRead(st)                                                   \
((((st).stuid == meUid) && ((st).stmode & S_IRUSR)) ||                       \
 ((st).stmode & S_IROTH) ||                                                  \
 (((st).stmode & S_IRGRP) &&                                                 \
  (((st).stgid == meGid) || (meGidSize && meGidInGidList((st).stgid))))) 
#define meStatTestWrite(st)                                                  \
((((st).stuid == meUid) && ((st).stmode & S_IWUSR)) ||                       \
 ((st).stmode & S_IWOTH) ||                                                  \
 (((st).stmode & S_IWGRP) &&                                                 \
  (((st).stgid == meGid) || (meGidSize && meGidInGidList((st).stgid))))) 
#define meStatTestExec(st)                                                   \
((((st).stuid == meUid) && ((st).stmode & S_IXUSR)) ||                       \
 ((st).stmode & S_IXOTH) ||                                                  \
 (((st).stmode & S_IXGRP) &&                                                 \
  (((st).stgid == meGid) || (meGidSize && meGidInGidList((st).stgid))))) 
#endif

/* Differentiate between different styles for waiting for a process to finish */
#ifdef _BSD
/* Use wait4 to enable us to wait on a PID. */
#define meWaitpid(pid,status,opt) wait4((pid),(status),(opt),NULL)
#define meWAIT_STATUS union wait
/* Use getwd() to get the current working directory */
#define meGetcwd(buf,size)        getwd((char *)(buf))
/* Use the lighter weight vfork() for forking */
#define meFork  vfork
#else
/* SVr4, POSIX.1 */
#define meWaitpid(pid,status,opt) waitpid((pid),(status),(opt))
#define meWAIT_STATUS int
/* POSIX.1 */
#define meGetcwd(buf,size)        getcwd((char *)(buf),(size))
/* Use fork() always */
#define meFork  fork
#endif /* _BSD */

#ifndef meRename
#define meRename(src,dst) rename((char *)(src),(char *)(dst))
#endif
#ifndef meUnlink
#define meUnlink(fn) unlink((char *)(fn))
#endif
#ifndef meChdir
#define meChdir(fn) chdir((char *)(fn))
#endif
#ifndef meTestExist
#define meTestExist(fn) access((char *)(fn),F_OK)
#endif
#ifndef meTestRead
#define meTestRead(fn) access((char *)(fn),R_OK)
#endif
#ifndef meTestWrite
#define meTestWrite(fn) access((char *)(fn),W_OK)
#endif
#ifndef meTestExec
#define meTestExec(fn) access((char *)(fn),X_OK)
#endif
#ifndef meTestDir
#define meTestDir(fn) (getFileStats(fn,0,NULL,NULL) != meFILETYPE_DIRECTORY)
#endif
#ifndef meFileSetAttributes
#define meFileSetAttributes(fn,attr) chmod((char *)(fn),attr)
#endif
#ifndef meExit
extern void exit(int i) ;
#if MEOPT_CLIENTSERVER
    /* Close & delete the client file */
#define meExit(n) (TTkillClientServer(),exit(n))
#else
#define meExit exit
#endif
#endif

/*
 * Terminal I/O file defintions
 */
#ifdef _NOPUTENV
#define meGetenv(s1)        ((void *) megetenv((const char *)(s1)))
extern char    *megetenv(const char *s);
extern int      putenv(const char *s);
#else
#define meGetenv(s1)        ((void *) getenv((const char *)(s1)))
#endif
#define mePutenv(s1)        (putenv((char *)(s1)))
#define meStrlen(s1)        strlen((const char *)(s1))
#define meStrcmp(s1,s2)     strcmp((const char *)(s1),(const char *)(s2))
#define meStrncmp(s1,s2,n)  strncmp((const char *)(s1),(const char *)(s2),(n))
#define meStrcat(s1,s2)     strcat((char *)(s1),(const char *)(s2))
#define meStrcpy(s1,s2)     strcpy((char *)(s1),(const char *)(s2))
#define meStrncpy(s1,s2,n)  strncpy((char *)(s1),(const char *)(s2),(n))
#define meStrstr(s1,s2)     ((void *) strstr((const char *)(s1),(const char *)(s2)))
#define meStrchr(s1,c1)     ((void *) strchr((const char *)(s1),(c1)))
#define meStrrchr(s1,c1)    ((void *) strrchr((const char *)(s1),(c1)))

#define CHRMSK_MACROTYPE   0x0f
#define CHRMSK_DISPLAYABLE 0x10
#define CHRMSK_POKABLE     0x20
#define CHRMSK_PRINTABLE   0x40
#define CHRMSK_SPACE       0x80

#define CHRMSK_LOWER       0x01
#define CHRMSK_UPPER       0x02
#define CHRMSK_ALPHA       0x03
#define CHRMSK_HEXDIGIT    0x04
#define CHRMSK_ALPHANUM    0x07
#define CHRMSK_SPLLEXT     0x08
#define CHRMSK_USER1       0x10
#define CHRMSK_USER2       0x20
#define CHRMSK_USER3       0x40
#define CHRMSK_USER4       0x80

#define CHRMSK_DEFWORDMSK  CHRMSK_ALPHANUM

#define getMacroType(c)  (charMaskTbl1[((meUByte) (c))] & CHRMSK_MACROTYPE)
#define getMacroTypeS(s) (charMaskTbl1[((meUByte) (*s))] & CHRMSK_MACROTYPE)
#define isDisplayable(c) (charMaskTbl1[((meUByte) (c))] & CHRMSK_DISPLAYABLE)
#define isPokable(c)     (charMaskTbl1[((meUByte) (c))] & CHRMSK_POKABLE)
#define isPrint(c)       (charMaskTbl1[((meUByte) (c))] & CHRMSK_PRINTABLE)
#define isSpace(c)       (charMaskTbl1[((meUByte) (c))] & CHRMSK_SPACE)

#define inWord()         (isWord(meLineGetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset)))
#define inPWord()        ((meLineGetChar(frameCur->windowCur->dotLine, frameCur->windowCur->dotOffset)) > ' ')

#if MEOPT_EXTENDED
/* with extended option enable set-char-mask can be used to configure the char sets */
#define isLower(c)       (charMaskTbl2[((meUByte) (c))] & CHRMSK_LOWER)
#define isUpper(c)       (charMaskTbl2[((meUByte) (c))] & CHRMSK_UPPER)
#define isAlpha(c)       (charMaskTbl2[((meUByte) (c))] & CHRMSK_ALPHA)
#define isDigit(c)       (((c) >= '0') && ((c) <= '9'))
#define isXDigit(c)      (charMaskTbl2[((meUByte) (c))] & CHRMSK_HEXDIGIT)
#define isAlphaNum(c)    (charMaskTbl2[((meUByte) (c))] & CHRMSK_ALPHANUM)
#define isWord(c)        (charMaskTbl2[((meUByte) (c))] & isWordMask)
#define isSpllExt(c)     (charMaskTbl2[((meUByte) (c))] & CHRMSK_SPLLEXT)
#define isSpllWord(c)    (charMaskTbl2[((meUByte) (c))] & (CHRMSK_ALPHANUM|CHRMSK_SPLLEXT))

#define toLower(c)       (isUpper(c) ? (charCaseTbl[((meUByte) (c))]):c)
#define toUpper(c)       (isLower(c) ? (charCaseTbl[((meUByte) (c))]):c)
#define toggleCase(c)    (charCaseTbl[((meUByte) (c))])

#define toUserFont(c)    (charLatinUserTbl[((meUByte) c)])
#define toLatinFont(c)   (charUserLatinTbl[((meUByte) c)])

#else

#define isLower(c)       (((c) >= 'a') && ((c) <= 'z'))
#define isUpper(c)       (((c) >= 'A') && ((c) <= 'Z'))
#define isAlpha(c)       (((c) >= 'A') && ((c) <= 'z') && (((c) <= 'Z') || ((c) >= 'a')))
#define isDigit(c)       (((c) >= '0') && ((c) <= '9'))
#define isXDigit(c)      (isDigit(c) || (((c) >= 'a') && ((c) <= 'f')) || (((c) >= 'A') && ((c) <= 'F')))
#define isAlphaNum(c)    (isDigit(c) || isAlpha(c))
#define isWord(c)        (isAlphaNum(c) || ((c) == '_'))

#define toLower(c)       (isUpper(c) ? ((c) ^ 0x20):c)
#define toUpper(c)       (isLower(c) ? ((c) ^ 0x20):c)
#define toggleCase(c)    (isAlpha(c) ? ((c) ^ 0x20):c)

#endif

#define	hexToNum(c)      ((c <= '9') ? (c^0x30)   : \
                          (c >= 'a') ? (c-'a'+10) : \
                                       (c-'A'+10))

/*	Macro argument token types					*/

#define TKNUL	0x00			/* end-of-string		*/
#define TKARG	0x01			/* interactive argument 	*/
#define TKREG	0x02			/* register variable		*/
#define TKVAR	0x03			/* user variables		*/
#define TKENV	0x04			/* environment variables	*/
#define TKFUN	0x05			/* function.... 		*/
#define TKDIR	0x06			/* directive			*/
#define TKLBL	0x07			/* line label			*/
#define TKLIT	0x08			/* numeric literal		*/
#define TKSTR	0x09			/* quoted string literal	*/
#define TKCMD	0x0A			/* command name 		*/
#define TKLVR	0x0B			/* Local variable 		*/
#define TKCVR	0x0C			/* Command variable 		*/

#define meAtoi(s) strtol((char *)(s),(char **)NULL,0)
#define meAtol(s) (meAtoi(s) != 0)
#define meLtoa(s) ((s) ? truem:falsem)

/* is url file? test */
#define isHttpLink(fn) (!strncmp((char *)fn,"http://",7))
#define isFtpLink(fn)  (!strncmp((char *)fn,"ftp://",6))
#define isUrlLink(fn)  (isHttpLink(fn) || isFtpLink(fn))
#define isUrlFile(fn)  (!strncmp((char *)fn,"file:",5))

/* is built-in file system test */
#define isBfsFile(fn)  ((!strncmp((char *)fn,"bfs://",6))||(!strncmp((char *)fn,"{BFS}",5)))

/* use this with some care */
#define meFree(x) free(x)
#define meNullFree(p) (((p)!=NULL)?(free(p),1):0)

#if MEOPT_OSD
#define mlResetCursor() ((frameCur->mlStatus&MLSTATUS_POSOSD) ? TTmove(osdRow,osdCol):TTmove(frameCur->depth,frameCur->mlColumn))
#else
#define mlResetCursor() TTmove(frameCur->depth,frameCur->mlColumn)
#endif
#define mwResetCursor() TTmove(frameCur->mainRow,frameCur->mainColumn)
#define resetCursor()   ((frameCur->mlStatus&(MLSTATUS_POSOSD|MLSTATUS_POSML)) ? mlResetCursor():mwResetCursor())

/* Get the number of characters to the next TAB position. */
#define get_tab_pos(c,tw)     ((tw-1) - (c)%tw) 
/* Get the next tab position. */
#define next_tab_pos(c,tw)    (tw - ((c)%tw))
/* Get the number of characters to the next TAB position. */
#define	at_tab_pos(c,tw)      ((c)%tw)
/* Get the number of TAB characters to the current position. */
#define get_no_tabs_pos(c,tw) ((c)/tw)                      

#define restoreWindWSet(wp,ws)                                               \
((wp)->vertScroll=(ws)->vertScroll,(wp)->dotLine=(ws)->dotLine,              \
 (wp)->dotOffset=(ws)->dotOffset,(wp)->markLine=(ws)->markLine,              \
 (wp)->markOffset=(ws)->markOffset,(wp)->dotLineNo=(ws)->dotLineNo,          \
 (wp)->markLineNo=(ws)->markLineNo)
#define restoreWindBSet(wp,bp)                                               \
((wp)->vertScroll=(bp)->vertScroll,(wp)->dotLine=(bp)->dotLine,              \
 (wp)->dotOffset=(bp)->dotOffset,(wp)->markLine=(bp)->markLine,              \
 (wp)->markOffset=(bp)->markOffset,(wp)->dotLineNo=(bp)->dotLineNo,          \
 (wp)->markLineNo=(bp)->markLineNo)
#define storeWindBSet(bp,wp)                                                 \
((bp)->vertScroll=(wp)->vertScroll,(bp)->dotLine=(wp)->dotLine,              \
 (bp)->dotOffset=(wp)->dotOffset,(bp)->markLine=(wp)->markLine,              \
 (bp)->markOffset=(wp)->markOffset,(bp)->dotLineNo=(wp)->dotLineNo,          \
 (bp)->markLineNo=(wp)->markLineNo)

#define setShowRegion(sbp,sln,slo,eln,elo)                                   \
(selhilight.bp=sbp,selhilight.markLineNo=sln,selhilight.markOffset=slo,      \
 selhilight.dotLineNo=eln,selhilight.dotOffset=elo,                          \
 selhilight.flags = SELHIL_ACTIVE|SELHIL_FIXED|SELHIL_CHANGED)

#if MEOPT_BINFS
extern unsigned char binfs[1];          /* The built in file system data */
extern bfs_t bfsdev;                    /* Built in file system device */
#endif
