/* -*- c -*-
 *
 * JASSPA MicroEmacs - www.jasspa.com
 * spawn.c - Routines for launching external process.
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
 * Synopsis:    Routines for launching external process.
 * Authors:     Unknown, Jon Green & Steven Phillips
 * Description:	
 *      Various system access commands.
 */

#define	__SPAWNC			/* Name the file */

#include "emain.h"

#ifdef _UNIX
#include <fcntl.h>                      /* This should not be required for POSIX !! */
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

/* Definitions for the terimal I/O */
#ifdef _USG
#ifdef _TERMIOS
#include <termios.h>
#else
#include <termio.h>
#endif /* _TERMIOS */
#endif /* _USG */
#ifdef _BSD
#include <sgtty.h>                      /* For stty/gtty functions */
#endif /* _BSD */

#ifdef _SUNOS
#include <stropts.h>
#endif

#ifdef _POSIX_VDISABLE
#define CDISABLE _POSIX_VDISABLE
#else /* not _POSIX_VDISABLE */
#ifdef CDEL
#undef CDISABLE
#define CDISABLE CDEL
#else /* not CDEL */
#define CDISABLE 255
#endif /* not CDEL */
#endif /* not _POSIX_VDISABLE */

#ifndef WEXITSTATUS
#define WEXITSTATUS(status) ((int)(WIFEXITED(status)?(((*((int *)(&status)))>>8)&0xff) : -1))
#endif /* WIFEXITSTATUS */

#endif /* _UNIX */

#ifdef _DOS
#include	<process.h>
#endif


/*
 * Create a temporary name for any command spawn files. On windows & dos
 * concatinate the $TEMP variable with the filename to create the new
 * temporary file name.
 *
 * Under unix create /tmp/me<pid><name>
 * Otherwise create $TEMP/<name>
 * On non-unix systems an extension can be passed in, see macro def in
 * eextrn.h
 */
#ifdef _UNIX
void
__mkTempName (meUByte *buf, meUByte *name)
{
    /* Under UNIX use the process id in order to create the temporary file
     * for this session of me
     */
    if(name != NULL)
        sprintf((char *)buf, "/tmp/me%d%s",(int) getpid(),name);
    else
    {
        strcpy((char *)buf, "/tmp/meXXXXXX") ;
        mkstemp((char *)buf) ;
    }
}

#else

void
__mkTempName (meUByte *buf, meUByte *name, meUByte *ext)
{
#ifdef _CONVDIR_CHAR
#define PIPEDIR_CHAR _CONVDIR_CHAR
#else
#define PIPEDIR_CHAR DIR_CHAR
#endif
    static meUByte *tmpDir=NULL ;
    meUByte *pp ;
    int ii ;

    if(tmpDir == NULL)
    {
        /* Get location of the temporary directory from the environment $TEMP */
        if ((pp = (meUByte *) meGetenv ("TEMP")) != NULL)
            tmpDir = meMalloc((ii=meStrlen(pp))+2) ;
        if(tmpDir != NULL)
        {
            meStrcpy(tmpDir,pp) ;
            if(tmpDir[ii-1] != PIPEDIR_CHAR)
            {
                tmpDir[ii++] = PIPEDIR_CHAR ;
                tmpDir[ii]   = '\0' ;
            }
        }
        else
#if (defined _DOS) || (defined _WIN32)
            /* the C drive is more reliable than ./ as ./ could be on a CD-Rom etc */
            tmpDir = "c:\\" ;
#else
            tmpDir = "./" ;
#endif
    }
    if(ext == NULL)
        ext = "" ;

    if(name != NULL)
        sprintf((char *)buf, "%sme%s%s",tmpDir,name,ext) ;
    else
    {
        for(ii=0 ; ii<999 ; ii++)
        {
            sprintf((char *)buf, "%smetmp%d%s",tmpDir,ii,ext) ;
            if(meTestExist(buf))
                break ;
        }

    }
}
#endif

#if MEOPT_SPAWN

#ifdef _UNIX
static meUByte *
getShellCmd(void)
{
    static meUByte *shellCmd=NULL ;
    if(shellCmd == NULL)
    {
        meUByte *cp, env[meBUF_SIZE_MAX], exe[meBUF_SIZE_MAX] ;
        if(((cp = meGetenv("SHELL")) != NULL) && (cp[0] != '\0') &&
           (meStrcpy(env,cp),(executableLookup(env,exe) != 0)))
            cp = exe ;
        else
            cp = (meUByte *)"/bin/sh" ;
        shellCmd = meStrdup(cp) ;
    }
    return shellCmd ;
}
#endif

/*
 * Create a subjob with a copy of the command intrepreter in it. When the
 * command interpreter exits, mark the screen as garbage so that you do a full
 * repaint. Bound to "^X C". The message at the start in VMS puts out a newline.
 * Under some (unknown) condition, you don't get one free when DCL starts up.
 */
int
meShell(int f, int n)
{
#ifdef _DOS
    register char *cp;
#endif
    meUByte path[meBUF_SIZE_MAX] ;		/* pathfrom where to execute */
    int  cd, ss ;
    
    getFilePath(frameCur->bufferCur->fileName,path) ;
    cd = (meStrcmp(path,curdir) && (meChdir(path) != -1)) ;

#ifdef _WIN32
    ss = WinLaunchProgram (NULL, LAUNCH_SHELL, NULL, NULL,
#if MEOPT_IPIPES
                           NULL, 
#endif
                           NULL) ;
#endif
#ifdef _DOS
    TTclose();
    if ((cp=meGetenv("COMSPEC")) == NULL)
        ss = system("command.com");
    else
        ss = system(cp);
    TTopen();
    sgarbf = meTRUE;
#endif
#ifdef _UNIX
#ifdef _XTERM
    if(!(meSystemCfg & meSYSTEM_CONSOLE))
    {
        switch(fork())
        {
        case 0:
            /* we want the children to die on interrupt */
            execlp("xterm", "xterm", "-sl", "200", "-sb", NULL);
            mlwrite(MWABORT,(meUByte *)"exec failed, %s", strerror(errno));
            meExit(127);
        case -1:
            ss = mlwrite(MWABORT,(meUByte *)"exec failed, %s", strerror(errno));
        default:
            ss = meTRUE ;
        }
    }
    else
#endif
    {
	TTclose();				/* stty to old settings */
	ss = system((char *)getShellCmd()) ;
	sgarbf = meTRUE;
	TTopen();
	ss = (ss < 0) ? meFALSE:meTRUE ;
    }
#endif
    if(cd)
        meChdir(curdir) ;
    return ss ;
}


/* Note: the given string cmdstr must be large enough to strcat
 * " </dev/null" on the end */
int
doShellCommand(meUByte *cmdstr, int flags)
{
    meUByte path[meBUF_SIZE_MAX] ;	/* pathfrom where to execute */
    meInt systemRet ;                   /* return value of last system  */
    int cd, ss ;
#ifdef _UNIX
    meWAIT_STATUS ws;
    meUByte *cmdline, *pp ; 
#endif

    getFilePath(frameCur->bufferCur->fileName,path) ;
    cd = (meStrcmp(path,curdir) && (meChdir(path) != -1)) ;

#ifdef _WIN32
    ss = WinLaunchProgram(cmdstr,LAUNCH_SYSTEM|flags, NULL, NULL, 
#if MEOPT_IPIPES
                          NULL,
#endif
                          &systemRet) ;
#else
#ifdef _UNIX
    /* if no data is piped in then pipe in /dev/null */
    if(((pp=meStrchr(cmdstr,'<')) == NULL) || (flags & LAUNCH_NOWAIT))
    {
        if((cmdline = meMalloc(meStrlen(cmdstr)+16)) == NULL)
            return meFALSE ;
        meStrcpy(cmdline,cmdstr) ;
        if(pp == NULL)
            meStrcat(cmdline," </dev/null") ;
        if(flags & LAUNCH_NOWAIT)
            meStrcat(cmdline," &") ;
    }
    else
        cmdline = cmdstr ;
    ss = system((char *)cmdline) ;
    ws = (meWAIT_STATUS)(ss);
    if(WIFEXITED(ws))
    {
        systemRet = WEXITSTATUS(ws) ;
        ss = meTRUE ;
    }
    else
    {
        systemRet = -1 ;
        ss = meFALSE ;
    }
    if(cmdline != cmdstr)
        meFree(cmdline) ;
#else
    systemRet = system(cmdstr) ;
#ifdef _DOS
    /* dos is naughty with modes, a system call could call a progam that
     * changes the screen stuff under our feet and not restore the current
     * mode! The only thing we can do is call TTopen to ensure we restore
     * our mode.
     * We might check all states, but with hidden things like flashing etc.
     * its not worth the effort - sorry, you do it if you want.
     */
    TTopen() ;
#endif
    ss = (systemRet < 0) ? meFALSE:meTRUE ;
#endif
#endif

    if(cd)
        meChdir(curdir) ;
    meStrcpy(resultStr,meItoa(systemRet)) ;
    return ss ;
}

int
meShellCommand(int f, int n)
{
    meUByte *cmdstr, cmdbuff[meBUF_SIZE_MAX+20];
    meBuffer *bp ;
    
    /* get the line wanted */
    if((meGetString((meUByte *)"System", 0, 0, cmdbuff, meBUF_SIZE_MAX)) <= 0)
        return meABORT ;
    if(n & LAUNCH_BUFCMDLINE)
    {
        if((bp=bfind(cmdbuff,0)) == NULL)
            return mlwrite(MWABORT,(meUByte *)"[%s: no such buffer]",cmdbuff);
        cmdstr = meLineGetText(meLineGetNext(bp->baseLine)) ;
    }
    else
        cmdstr = cmdbuff ;
    f = (n & LAUNCH_USER_FLAGS) ;
    if((n & LAUNCH_WAIT) == 0)
        f |= LAUNCH_NOWAIT ;
    
    return doShellCommand(cmdstr,f) ;
}

#if MEOPT_IPIPES

/*
 *---	Interactive PIPE into the list buffer.
 */
#ifdef _WIN32

static BOOL CALLBACK
ipipeFindChildWindow(HWND hwnd, long lipipe)
{
    DWORD process ;
    meIPipe *ipipe = (meIPipe *)(lipipe);

    GetWindowThreadProcessId (hwnd,&process);
    if (process == ipipe->processId)
    {
        ipipe->childWnd = hwnd ;
        return meFALSE ;
    }
    /* keep looking */
    return meTRUE ;
}

static HWND
ipipeGetChildWindow(meIPipe *ipipe)
{
    ipipe->childWnd = NULL ;
    EnumWindows(ipipeFindChildWindow,(long) ipipe) ;
    return ipipe->childWnd ;
}

#ifdef _WIN32s

#define ipipeKillProcessTree(ppid) meFALSE

#else

#include <tlhelp32.h>

static int
ipipeKillProcessTree(DWORD ppid)
{
    typedef HANDLE (WINAPI *CREATETOOLHELP32SNAPSHOT)(DWORD,DWORD) ;
    typedef BOOL (WINAPI *PROCESS32FIRST)(HANDLE,LPPROCESSENTRY32) ;
    typedef BOOL (WINAPI *PROCESS32NEXT)(HANDLE,LPPROCESSENTRY32) ;

    static int procGetFuncs=0 ;
    static CREATETOOLHELP32SNAPSHOT procCreateSnapshot ;
    static PROCESS32FIRST procGetFirst ;
    static PROCESS32NEXT procGetNext ;
    HANDLE procSnap, procHandle ;
    PROCESSENTRY32 pe ;
    DWORD *pidList ;
    int pidCount, pidCur ;
    
    if(!procGetFuncs)
    {
        HINSTANCE libHandle ; 
        
        procGetFuncs = 1 ;
        if((libHandle = LoadLibrary("kernel32")) != NULL) 
        { 
            procCreateSnapshot = (CREATETOOLHELP32SNAPSHOT) GetProcAddress(libHandle,"CreateToolhelp32Snapshot") ; 
            if(((procGetFirst = (PROCESS32FIRST) GetProcAddress(libHandle,"Process32First")) == NULL) ||
               ((procGetNext = (PROCESS32NEXT) GetProcAddress(libHandle,"Process32Next")) == NULL))
                procCreateSnapshot = NULL ;
        }
    } 
    if(procCreateSnapshot == NULL)
        return meFALSE ;
 
    procSnap = procCreateSnapshot(TH32CS_SNAPPROCESS,0) ; 
    if(procSnap == INVALID_HANDLE_VALUE)
        return meFALSE ;
    
    if((pidList = meMalloc(64*sizeof(DWORD))) == NULL)
    {
        CloseHandle(procSnap) ;
        return meFALSE ;
    }
    pidList[0] = ppid ;
    pidCount = 1 ;
    pidCur = 0 ;
    
    pe.dwSize = sizeof(PROCESSENTRY32) ;
    do {
        ppid = pidList[pidCur] ;
        if(!procGetFirst(procSnap,&pe)) 
            break ;
        do {
            if(pe.th32ParentProcessID == ppid)
            {
                if(((pidCount & 0x3f) == 0) &&
                   ((pidList = meRealloc(pidList,(pidCount+64)*sizeof(DWORD))) == NULL))
                {
                    CloseHandle(procSnap) ;
                    return meFALSE ;
                }
                pidList[pidCount++] = pe.th32ProcessID ;
            }
        } while(procGetNext(procSnap,&pe)) ;
    } while(++pidCur != pidCount) ;
    CloseHandle(procSnap) ;
    
    /* kill from the parent down */
    for(pidCur=0 ; pidCur<pidCount ; pidCur++)
    {
        if((procHandle = OpenProcess(PROCESS_TERMINATE,FALSE,pidList[pidCur])) != NULL)
            TerminateProcess(procHandle,999) ;
    }
    return meTRUE ;
}
#endif
#endif

static void
ipipeWriteString(meIPipe *ipipe, int n, meUByte *str)
{
    while(n--)
    {
#ifdef _WIN32
        DWORD written ;
        WriteFile(ipipe->outWfd,str,meStrlen(str),&written,NULL) ;
#else
        write(ipipe->outWfd,str,meStrlen(str)) ;
#endif
    }
}


static void
ipipeKillBuf(meIPipe *ipipe, int type)
{

    if(ipipe->pid > 0)
    {
        if(type == 2)
        {
#ifdef _WIN32
            {
                /* on windows surprise surprise writing C-c to stdin does not work very
                 * well, it does not have the same effect. There are a few extra things
                 * we can do and hte combination of doing them all does seem to have
                 * the right effect - but this may be very machine dependant.
                 * 
                 * One thing we can do is make the child process window have the 
                 * current input focus and then send the key strokes down to it.
                 * But to do this we must have a foreground window and we must attatch
                 * the thread to the current thread so we can steal the current window.
                 */
                HWND  foreWnd, chldWnd ;
                DWORD foreThread, chldThread ;
                BOOL success=0 ;
                
                if(((foreWnd=GetForegroundWindow()) != NULL) &&
                   ((chldWnd=ipipeGetChildWindow(ipipe)) != NULL))
                {
                    foreThread = GetWindowThreadProcessId(foreWnd,NULL) ;
                    if((GetCurrentThreadId() == foreThread) || 
                       !AttachThreadInput(GetCurrentThreadId(),foreThread,meTRUE))
                        foreThread = 0 ;
                    
                    chldThread = ipipe->processId ;
                    if((GetCurrentThreadId() == chldThread) || 
                       !AttachThreadInput(GetCurrentThreadId(),chldThread,meTRUE))
                        chldThread = 0;
                    
                    /* Set the fore window to the child */
                    if((success=SetForegroundWindow(chldWnd)) != 0)
                    {
                        /* success, now we can start sending the string down */
                        BYTE scanCodeCtrl, scanCodeC ;
                        
                        scanCodeCtrl = (BYTE) MapVirtualKey(VK_CONTROL, 0);
                        scanCodeC = (BYTE) MapVirtualKey('C', 0);
                        keybd_event(VK_CONTROL,scanCodeCtrl,0,0) ;
                        keybd_event('C',scanCodeC,0,0) ;
                        keybd_event('C',scanCodeC,KEYEVENTF_KEYUP,0) ;
                        keybd_event(VK_CONTROL,scanCodeCtrl,KEYEVENTF_KEYUP,0) ;
                        
                        /* call halt here and let the other process get the keys -
                         * the length of the sleep is a fine balance, too small and the
                         * child process will not get the Ctrl-C, but if its to large the
                         * user could press a key, entering a char or changing the state
                         * of the Ctrl or Shift keys - these events should go to the original
                         * foreWnd so that app will get very confused! So the sleep should
                         * be as small as possible
                         */
                        Sleep(50) ;
                        
                        /* Swap back to the original fore window */
                        SetForegroundWindow(foreWnd);
                        
                    }
                    /* Detach the threads */
                    if(foreThread)
                        AttachThreadInput(GetCurrentThreadId(),foreThread,meFALSE);
                    if(chldThread)
                        AttachThreadInput(GetCurrentThreadId(),chldThread,meFALSE);
                }
            
                /* one other thing we can do is to send a Ctrl-C event if we are trying
                 * to write a C-c (0x03), but we will write the string as well */
                GenerateConsoleCtrlEvent(CTRL_C_EVENT,ipipe->processId) ;
                
                /* if we've succeeded in sending the Ctrl-C keys and the event
                 * then quit now - if they don't work, nothing will */
                if(success)
                    return ;
            }
#endif
            /* send a control-C signal */
            ipipeWriteString(ipipe,1,(meUByte *) "\x03") ;
            return ;
        }
#ifdef _WIN32
        if(ipipe->pid > 0)
        {
            if(ipipeKillProcessTree(ipipe->processId) <= 0)
                /* cannot terminate the tree - we can only Terminate this one */
                TerminateProcess(ipipe->process,999) ;
            /* On windows theres no child signal, so flag as killed */
            ipipe->pid = -5 ;
        }
        /* Close the process */
        CloseHandle(ipipe->process);
#else
        kill(0-ipipe->pid,SIGKILL) ;
#endif
    }
}

int
ipipeKill(int f, int n)
{
    meIPipe *ipipe ;

    if(!meModeTest(frameCur->bufferCur->mode,MDPIPE))
    {
        TTbell() ;
        return meFALSE ;
    }
    ipipe = ipipes ;
    while(ipipe->bp != frameCur->bufferCur)
        ipipe = ipipe->next ;
    ipipeKillBuf(ipipe,n) ;
    return meTRUE ;
}

void
ipipeRemove(meIPipe *ipipe)
{
#ifndef _WIN32
    meSigHold() ;
#endif
    if(ipipe->pid > 0)
        ipipeKillBuf(ipipe,1) ;

    if(ipipe == ipipes)
        ipipes = ipipe->next ;
    else
    {
        meIPipe *pp ;

        pp = ipipes ;
            while(pp->next != ipipe)
                pp = pp->next ;
        pp->next = ipipe->next ;
    }
    noIpipes-- ;
    if(ipipe->bp != NULL)
    {
        meModeClear(ipipe->bp->mode,MDPIPE) ;
        meModeClear(ipipe->bp->mode,MDLOCK) ;
    }
#ifdef _WIN32
    /* if we're using a child activity thread the close it down */
    if(ipipe->thread != NULL)
    {
        DWORD exitCode ;

        if(GetExitCodeThread(ipipe->thread,&exitCode) && (exitCode == STILL_ACTIVE))
        {
            /* get the thread going again */
            ipipe->flag |= meIPIPE_CHILD_EXIT ;
            SetEvent(ipipe->threadContinue) ;
            if(WaitForSingleObject(ipipe->thread,200) != WAIT_OBJECT_0)
                TerminateThread(ipipe->thread,0) ;
	}
#ifndef USE_BEGINTHREAD
        CloseHandle (ipipe->thread);
#endif
    }
    if(ipipe->threadContinue != NULL)
        CloseHandle(ipipe->threadContinue) ;
    if(ipipe->childActive != NULL)
        CloseHandle(ipipe->childActive) ;
    CloseHandle(ipipe->rfd) ;
    CloseHandle(ipipe->outWfd) ;
#else
    close(ipipe->rfd) ;
    if(ipipe->rfd != ipipe->outWfd)
        close(ipipe->outWfd) ;
    meSigRelease() ;
#endif
    free(ipipe) ;
}

#ifdef _WIN32

static int
readFromPipe(meIPipe *ipipe, int nbytes, meUByte *buff)
{
    DWORD  bytesRead ;

    /* See if process has ended first */
    if(ipipe->pid < 0)
        return ipipe->pid ;
#if MEOPT_CLIENTSERVER
    if(ipipe->pid == 0)
    {
        if(ttServerToRead == 0)
            return 0 ;
        if(nbytes > ttServerToRead)
            nbytes = ttServerToRead ;
        if(ReadFile(ipipe->rfd,buff,nbytes,&bytesRead,NULL) == 0)
            return -1 ;
        return (int) bytesRead ;
    }
#endif
    if(ipipe->flag & meIPIPE_CHILD_EXIT)
    {
        GetExitCodeProcess(ipipe->process,(LPDWORD) &(ipipe->exitCode)) ;
        CloseHandle(ipipe->process);
        ipipe->pid = -4 ;
        return ipipe->pid ;
    }
    if(ipipe->flag & meIPIPE_NEXT_CHAR)
    {
        buff[0] = ipipe->nextChar ;
        ipipe->flag &= ~meIPIPE_NEXT_CHAR ;
        return 1 ;
    }        
    /* Must peek on a pipe cos if we try to read too many this will fail */
    if((PeekNamedPipe(ipipe->rfd, (LPVOID) NULL, (DWORD) 0,
                      (LPDWORD) NULL, &bytesRead, (LPDWORD) NULL) != meTRUE) ||
       (bytesRead <= 0))
        return 0 ;
    if(bytesRead > (DWORD) nbytes)
        bytesRead = (DWORD) nbytes ;
    if(ReadFile(ipipe->rfd,buff,bytesRead,&bytesRead,NULL) == 0)
        return -1 ;
    return (int) bytesRead ;
}

#else

#if MEOPT_CLIENTSERVER

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netinet/in.h>

static int
readFromPipe(meIPipe *ipipe, int nbytes, meUByte *buff)
{
    int ii ;
    if(ipipe->pid == 0)
    {
        if ((ii = recv(ipipe->rfd,(char *) buff,nbytes,0)) < 0)
            ii = 0 ;
    }
    else
        ii = read(ipipe->rfd,buff,nbytes) ;
#if 0
    if(ii > 0)
    {
        static FILE *fplog=NULL ;
        if(fplog == NULL)
            fplog = fopen("log","wb+") ;
        fwrite(buff,1,ii,fplog) ;
    }
#endif
    return ii ;

}


#else

#define readFromPipe(ipipe,nbytes,buff) read(ipipe->rfd,buff,nbytes)

#endif

#endif

#define getNextCharFromPipe(ipipe,cc,rbuff,curROff,curRRead)                 \
((curROff < curRRead) ?                                                      \
 ((cc=rbuff[curROff++]), 1):                                                 \
 (((curRRead=readFromPipe(ipipe,meBUF_SIZE_MAX,rbuff)) > 0) ?                \
  ((cc=rbuff[0]),curROff=1): 0))


#define ipipeStoreInputPos()                                                 \
do {                                                                         \
    meLine *lp_new ;                                                         \
    noLines += addLine(lp_old,buff) ;                                        \
    lp_new = meLineGetPrev(lp_old) ;                                         \
    if(lp_old != bp->baseLine)                                               \
    {                                                                        \
        noLines-- ;                                                          \
        lp_new->next = lp_old->next ;                                        \
        lp_old->next->prev = lp_new ;                                        \
        if(lp_old->flag & meLINE_ANCHOR)                                     \
            meLineResetAnchors(meLINEANCHOR_ALWAYS|meLINEANCHOR_RETAIN,bp,   \
                               lp_old,lp_new,0,0);                           \
        meFree(lp_old);                                                      \
    }                                                                        \
    else                                                                     \
    {                                                                        \
        bp->dotLineNo-- ;                                                    \
        ipipe->curRow-- ;                                                    \
    }                                                                        \
    bp->dotLineNo += noLines ;                                               \
    bp->lineCount += noLines ;                                               \
    ipipe->curRow = curRow ;                                                 \
    bp->vertScroll = bp->dotLineNo-curRow ;                                  \
    bp->dotLine = lp_new ;                                                   \
    bp->dotOffset = p1 - buff ;                                              \
    meBufferUpdateLocation(bp,noLines,bp->dotOffset) ;                       \
} while(0)


void
ipipeRead(meIPipe *ipipe)
{
    meBuffer *bp=ipipe->bp ;
    meLine   *lp_old ;
    int     len, curOff, maxOff, curRow, ii ;
    meUInt  noLines ;
    meUByte  *p1, cc, buff[1025] ;
    meUByte   rbuff[meBUF_SIZE_MAX] ;
    int     curROff=0, curRRead=0 ;
#if _UNIX
    int     na, nb ;
#endif

    if(meModeTest(bp->mode,MDWRAP))
        maxOff = ipipe->noCols ;
    else
        maxOff = meBUF_SIZE_MAX - 2 ;
#ifdef _UNIX
    meSigHold() ;
#if MEOPT_CLIENTSERVER
    if(ipipe->pid == 0)
    {
        struct sockaddr_un cssa ;	/* for unix socket address */

        ii = sizeof(struct sockaddr_un);
        cssa.sun_family = AF_UNIX;
        ipipe->rfd = accept(ipipe->rfd,(struct sockaddr *)&cssa, (void *)&ii) ;
    }
#endif
#endif
    meAnchorGet(bp,'I') ;
    if(meModeTest(bp->mode,MDLOCK))
    {
        /* Work out which windows are locked to the current cursor position */
        meWindow *wp ;
        
        meFrameLoopBegin() ;
        wp = loopFrame->windowList;
        while(wp != NULL)
        {
            if((wp->buffer == bp) &&
               (wp->dotLine == bp->dotLine) &&
               (wp->dotOffset == bp->dotOffset))
                break ;
            wp = wp->next;
        }
        meFrameLoopBreak(wp != NULL) ;
        meFrameLoopEnd() ;
        if(wp == NULL)
            bp->intFlag &= ~BIFLOCK ;
        else
            bp->intFlag |= BIFLOCK ;
    }
    /* This is a quick sanity check which is needed if the buffer has
     * been changed by something. If curRow becomes greater than dotLineNo
     * the vertScroll becomes negative and things go wrong.
     * Discovered problem when using gdb mode as the gdb input handler
     * kills ^Z^Z lines making curRow > dotLineNo.
     */
    if((curRow=ipipe->curRow) > bp->dotLineNo)
        curRow = bp->dotLineNo ;
    len = bp->dotOffset ;
    lp_old = bp->dotLine ;
    meBufferStoreLocation(lp_old,bp->dotOffset,bp->dotLineNo) ;
    meStrcpy(buff,lp_old->text) ;
    p1 = buff+len ;
    noLines = 0 ;
    curOff = getcol(buff,len,bp->tabWidth) ;
    for(;;)
    {
        if(!getNextCharFromPipe(ipipe,cc,rbuff,curROff,curRRead))
        {

            if(ipipe->pid == -4)
                sprintf((char *) p1,"[EXIT %d]",(int) ipipe->exitCode) ;
            else
            {
                meUByte *ins ;
                if(ipipe->pid == -5)
                    ins = (meUByte *)"[TERMINATED]" ;
                else if(ipipe->pid == -3)
                    ins = (meUByte *)"[KILLED]" ;
                else if(ipipe->pid == -2)
                    ins = (meUByte *)"[CORE DUMP]" ;
                else
                    /* none left to read */
                    break ;
                meStrcpy(p1,ins) ;
            }
            ipipe->pid = -1 ;
            /* Do not annotate the end of the ipipe in raw mode */
            if ((ipipe->flag & meIPIPE_RAW) != 0)
            {
                *p1 = '\0' ;
                cc = 0;
            }
            else
                cc = meCHAR_NL ;
        }
        switch(cc)
        {
        case 0: /* ignore */
            break ;
        case 7:
            TTbell() ;
            break ;
        case 8:
            if(p1 != buff)
            {
                p1-- ;
                len-- ;
                curOff = getcol(buff,len,bp->tabWidth) ;
            }
            break ;
        case '\r':
            p1 = buff ;
            len = curOff = 0 ;
            break ;
        case meCHAR_NL:
#if _UNIX
            if(!(ipipe->flag & meIPIPE_OVERWRITE) && (curRow+1 < ipipe->noRows))
            {
                /* if in over-write mode and not at the bottom, move instead */
                nb = curRow + 1 ;
                na = 0 ;
                goto move_cursor_pos ;
            }
#endif
            ii = addLine(lp_old,buff) ;
            noLines += ii ;
            if(curRow < ipipe->noRows-1)
                curRow += ii ;
            p1 = buff ;
            *p1 = '\0' ;
            len = curOff = 0 ;
            break ;
        case 15: /* ignore */
            break ;
#if _UNIX
        case 27:
            if(getNextCharFromPipe(ipipe,cc,rbuff,curROff,curRRead))
            {
#ifndef NDEBUG
                int gotQ=0 ;
#endif
                int gotN=0 ;

                na=0 ;
                nb=-1 ;
                if(cc == '[')
                {
get_another:
                    if(getNextCharFromPipe(ipipe,cc,rbuff,curROff,curRRead))
                    {
                        if(isDigit(cc))
                        {
                            gotN |= 1 ;
                            na = na*10 + (cc - '0') ;
                            goto get_another ;
                        }
                        switch(cc)
                        {
                        case ';':
                            gotN |= 2 ;
                            nb = na ;
                            na = 0 ;
                            goto get_another ;
                        case '?':
#ifndef NDEBUG
                            gotQ = 1 ;
#endif
                            goto get_another ;
                        case '@':
                            if(!gotN)
                                na = 1 ;
                            nb = meStrlen(p1) ;

                            do
                                p1[nb+na] = p1[nb] ;
                            while(--nb >= 0) ;
                            len += na ;
                            memset(p1,' ',na) ;
                            break ;
                        case 'C':
                            if(!gotN)
                                na = 1 ;
                            if((na + len) >= ipipe->noCols)
                                na = ipipe->noCols - len - 1 ;
                            nb = na - meStrlen(p1) ;
                            p1 += na ;
                            len += na ;
                            if(nb > 0)
                            {
                                memset(p1-nb,' ',nb) ;
                                *p1 = '\0' ;
                            }
                            curOff = getcol(buff,len,bp->tabWidth) ;
                            break ;
                        case 'D':
                            if(!gotN)
                                na = 1 ;
                            if(len < na)
                                na = len ;
                            p1 -= na ;
                            len -= na ;
                            curOff = getcol(buff,len,bp->tabWidth) ;
                            break ;
                        case 'A':
                            if(!gotN)
                                na = 1 ;
                            nb = curRow - na ;
                            na = len ;
                            goto move_cursor_pos ;

                        case 'B':
                            if(!gotN)
                                na = 1 ;
                            nb = curRow + na ;
                            na = len ;
                            goto move_cursor_pos ;

                        case 'H':
                            if(gotN & 2)
                                na-- ;
                            else
                            {
                                nb = na ;
                                na = 0 ;
                            }
                            if(gotN & 1)
                                nb-- ;
                            else
                                nb = 0 ;
move_cursor_pos:
                            if(nb < 0)
                                nb = 0 ;
                            else if(nb >= ipipe->noRows)
                                nb = ipipe->noRows - 1 ;
                            if(na < 0)
                                na = 0 ;
                            else if(na >= ipipe->noCols)
                                na = ipipe->noCols - 1 ;
                            ipipeStoreInputPos() ;
                            bp->dotLineNo += nb - curRow  ;
                            lp_old = bp->dotLine ;
                            if(nb > curRow)
                            {
                                while((curRow != nb) && (lp_old != bp->baseLine))
                                {
                                    curRow++ ;
                                    lp_old = meLineGetNext(lp_old) ;
                                }
                                while(curRow != nb)
                                {
                                    curRow++ ;
                                    addLineToEob(bp,(meUByte *)"") ;
                                }
                            }
                            else
                            {
                                while(curRow != nb)
                                {
                                    curRow-- ;
                                    lp_old = meLineGetPrev(lp_old) ;
                                }
                            }
                            len = na ;
                            bp->dotLine = lp_old ;
                            meStrcpy(buff,lp_old->text) ;
                            meBufferStoreLocation(lp_old,(meUShort)len,bp->dotLineNo) ;
                            na -= meStrlen(buff) ;
                            p1 = buff+len ;
                            if(na > 0)
                            {
                                memset(p1-na,' ',na) ;
                                *p1 = '\0' ;
                            }
                            curOff = getcol(buff,len,bp->tabWidth) ;
                            noLines = 0 ;
                            break ;

                        case 'h':
                            if(na != 4)
                                goto cant_handle_this ;
                            ipipe->flag |= meIPIPE_OVERWRITE ;
                            break ;
                        case 'l':
                            if(na != 4)
                                goto cant_handle_this ;
                            ipipe->flag &= ~meIPIPE_OVERWRITE ;
                            break ;
                        case 'm':
                            /* These are font changes, like bold etc.
                             * We won't do anything with them for now
                             */
                            break ;
                        case 'n':
                            {
                                char outb[20] ;

                                if(na != 6)
                                    goto cant_handle_this ;

                                sprintf(outb,"\033[%d;%dR",curRow,len) ;
                                write(ipipe->outWfd,outb,strlen(outb)) ;
                                break ;
                            }
                        case 'J':
                            {
                                meLine *lp ;

                                lp = lp_old ;
                                if(na == 2)
                                {
                                    for(ii=curRow ; ii>0 ; ii--)
                                        lp = meLineGetPrev(lp) ;
                                    memset(buff,' ',meStrlen(buff)) ;
                                }
                                else if(na == 0)
                                {
                                    memset(buff+len,' ',meStrlen(buff+len)) ;
                                    if(lp != bp->baseLine)
                                        lp = meLineGetNext(lp) ;
                                }
                                else
                                    goto cant_handle_this ;
                                while(lp != bp->baseLine)
                                {
                                    memset(lp->text,' ',meLineGetLength(lp)) ;
                                    lp->flag |= meLINE_CHANGED ;
                                    lp = meLineGetNext(lp) ;
                                }
                                curOff = len ;
                                break ;
                            }
                        case 'K':
                            *p1 = '\0' ;
                            break ;
                        case 'P':
                            if(!gotN)
                                na = 1 ;
                            if(((int) meStrlen(p1)) > na)
                                meStrcpy(p1,p1+na) ;
                            else
                                *p1 = '\0' ;
                            break ;
                        default:
cant_handle_this:
#ifndef NDEBUG
                            if(nb < 0)
                                printf("Don't cope with term code \\E[%s%d%c\n",
                                       (gotQ) ? "?":"",na,cc) ;
                            else
                                printf("Don't cope with term code \\E[%s%d;%d%c\n",
                                       (gotQ) ? "?":"",nb,na,cc) ;
#endif
                            break ;
                        }
                    }
                    break ;
                }
                else if(cc == '7')
                {
                    ipipe->strRow = curRow ;
                    ipipe->strCol = len ;
                }
                else if(cc == '8')
                {
                    nb = ipipe->strRow ;
                    na = ipipe->strCol ;
                    goto move_cursor_pos ;
                }
            }
            /* follow through */
#endif
        default:
            if(curOff >= maxOff)
            {
                meUByte bb[2] ;
                bb[0] = p1[0] ;
                bb[1] = p1[1] ;
                p1[0] = windowChars[WCDISPSPLTLN] ;
                p1[1] = '\0' ;
                ii = addLine(lp_old,buff) ;			/* Add string */
                noLines += ii ;
                if(curRow < ipipe->noRows-1)
                    curRow += ii ;
                p1[0] = bb[0] ;
                p1[1] = bb[1] ;
                meStrcpy(buff,p1) ;
                p1 = buff ;
                len = curOff = 0 ;
            }
            if(ipipe->flag & meIPIPE_OVERWRITE)
            {
                meUByte *ss, *dd ;
                int ll ;

                ll = meStrlen(p1) ;
                ss = p1 + ll ;
                dd = ss+1 ;
                do
                    *dd-- = *ss-- ;
                while(ll--) ;
            }
            else if(*p1 == '\0')
                p1[1] = '\0' ;
            else if((cc == meCHAR_TAB) && (get_tab_pos(curOff,bp->tabWidth) == 0))
            {
                /* theres a strangeness with vt100 tab as it doesn't
                 * seem to erase the next character and seems to be used
                 * (at least by tcsh) to move the cursor one to the right.
                 * So catch this special case of one character move.
                 * NOTE the previous else if checked there is another char.
                 */
                p1++ ;
                curOff++ ;
                break ;
            }
            *p1++ = cc ;
            if(isDisplayable(cc))
                curOff++ ;
            else if(cc == meCHAR_TAB)
                curOff += get_tab_pos(curOff, bp->tabWidth) + 1 ;
            else if (cc  < 0x20)
                curOff += 2 ;
            else
                curOff += 4 ;
            len++ ;
        }
    }
    ipipeStoreInputPos() ;
#ifdef _UNIX
#if MEOPT_CLIENTSERVER
    /* the unix client server trashed the rfd at the top of this function due
     * to the way sockets are handled. But the read handle is the same as the write
     * so its trivial to restore */
    if(ipipe->pid == 0)
        ipipe->rfd = ipipe->outWfd ;
#endif
#endif
    
    if((ii=ipipe->pid) < 0)
        ipipeRemove(ipipe) ;
#ifdef _WIN32
    else if(ipipe->thread != NULL)
        /* get the thread going again */
        SetEvent(ipipe->threadContinue) ;
#endif

#if ((defined (_UNIX)) && (defined (_POSIX_SIGNALS)))
    /* as soon as the BLOCK of sigchld is removed, if the process has finished
     * while reading then it will get registered now, this fact has to be taken
     * into acount and handled carefully. If not typically the [EXIT] line
     * will be missed if signals still blocked during the execution of the ipipe
     * function which leads to side effects like poor cursor blink etc.
     */
    meSigRelease() ;
#endif
    
    meAnchorSet(bp,'I',bp->dotLine,bp->dotOffset,1) ;
    if(bp->ipipeFunc >= 0)
        /* If the process has ended the argument will be 0, else 1 */
        execBufferFunc(bp,bp->ipipeFunc,(meEBF_ARG_GIVEN|meEBF_USE_B_DOT|meEBF_HIDDEN),(ii >= 0)) ;
    else if ((ii < 0) && (bp->intFlag & BIFLOCK))
    {
        meWindow *wp ;
        
        /* The pipe has ended and is not under manual control, a BIFLOCK
         * exists which indidates that one or more windows are tied to the
         * buffer cursor position. For each window that is locked with the
         * buffer then re-center the window so that the last line is at the
         * bottom of the window. Work out which windows are locked to the
         * current buffer position and re-center them. */
        meFrameLoopBegin() ;
        wp = loopFrame->windowList;
        while(wp != NULL)
        {
            /* If the window position matches the buffer then re-center */
            if((wp->buffer == bp) &&
               (wp->dotLine == bp->dotLine) &&
               (wp->dotOffset == bp->dotOffset))
            {
                /* Force a bottom window recenter */
                wp->windowRecenter = -1 ;
                wp->updateFlags |= WFFORCE ;
            }
            wp = wp->next;
        }
        meFrameLoopEnd() ;
    }
    update(meFALSE) ;
}

int
ipipeWrite(int f, int n)
{
    int      ss ;
    meUByte    buff[meBUF_SIZE_MAX];	/* string to add */
    meIPipe *ipipe ;

    if(!meModeTest(frameCur->bufferCur->mode,MDPIPE))
        return mlwrite(MWABORT,(meUByte *)"[Not an ipipe-buffer]") ;
    /* ask for string to insert */
    if((ss=meGetString((meUByte *)"String", 0, 0, buff, meBUF_SIZE_MAX)) <= 0)
        return ss ;
    
    ipipe = ipipes ;
    while(ipipe->bp != frameCur->bufferCur)
        ipipe = ipipe->next ;
    ipipeWriteString(ipipe,n,buff) ;

    return meTRUE ;
}

int
ipipeSetSize(meWindow *wp, meBuffer *bp)
{
    meIPipe *ipipe ;

    ipipe = ipipes ;
    while(ipipe->bp != bp)
        ipipe = ipipe->next ;
    if(((ipipe->noRows != wp->textDepth) ||
        (ipipe->noCols != wp->textWidth-1) ) &&
       (ipipe->pid > 0))
    {
        int ii ;
        ii = wp->textDepth - ipipe->noRows ;
        ipipe->noRows = wp->textDepth ;
        ipipe->noCols = wp->textWidth-1 ;
        if(ii > 0)
        {
            if((ipipe->curRow += ii) > bp->lineCount)
                ipipe->curRow = (meShort) bp->lineCount ;
        }
        else if(ipipe->curRow >= ipipe->noRows)
            ipipe->curRow = ipipe->noRows-1 ;
        /* Check the window is displaying this buffer before we
         * mess with the window settings */
        if((wp->buffer == bp) && meModeTest(bp->mode,MDLOCK))
        {
            if (wp->dotLineNo < ipipe->curRow)
                wp->vertScroll = 0;
            else
                wp->vertScroll = wp->dotLineNo-ipipe->curRow ;
            wp->updateFlags |= WFMOVEL ;
        }
#ifdef _UNIX
#if ((defined TIOCSWINSZ) || (defined TIOCGWINSZ))
        {
            /* BSD-style.  */
            struct winsize size;

            size.ws_col = ipipe->noCols ;
            size.ws_row = ipipe->noRows ;
            size.ws_xpixel = size.ws_ypixel = 0 ;
#ifdef TIOCSWINSZ
            ioctl(ipipe->outWfd,TIOCSWINSZ,&size) ;
#else
            ioctl(ipipe->outWfd,TIOCGWINSZ,&size) ;
#endif
            kill(ipipe->pid,SIGWINCH) ;
        }
#else
#ifdef TIOCGSIZE
        {
            /* SunOS - style.  */
            struct ttysize size;

            size.ts_col = ipipe->noCols ;
            size.ts_lines = ipipe->noRows ;
            ioctl(ipipe->outWfd,TIOCSSIZE,&size) ;
            kill(ipipe->pid,SIGWINCH) ;
        }
#endif /* TIOCGSIZE */
#endif /* TIOCSWINSZ/TIOCGWINSZ */
#endif /* _UNIX */
    }
    return meTRUE ;
}

#ifdef _UNIX
/* allocatePty; Allocate a pty. We use the old BSD method of searching for a
 * pty, once we have aquired one then we look for the tty. Return the name of
 * the tty to the caller so that it may be opened. */

#ifdef  _LINUX26
extern char *ptsname(int) ;
extern int unlockpt(int) ;
extern int grantpt(int) ;
#endif /* _LINUX26 */

static int
allocatePty(meUByte *ptyName)
{
#ifdef _IRIX
    int    fd ;
    struct stat stb ;
    /* struct sigaction ocstat, cstat;*/
    char * name;
    /* sigemptyset(&cstat.sa_mask);*/
    /* cstat.sa_handler = SIG_DFL;*/
    /* cstat.sa_flags = 0;*/
    /* sigaction(SIGCLD, &cstat, &ocstat);*/
    name = _getpty(&fd,O_RDWR,0600,0);
    /* sigaction(SIGCLD, &ocstat, (struct sigaction *)0);*/
    if(name == NULL)
        return -1 ;
    if((fd >=  0) && (fstat (fd, &stb) >= 0))
    {
        /* Return the name of the tty and the file descriptor of the pty */
        meStrcpy (ptyName, name);
        return fd ;
    }
    return -1 ;
#else
#if (defined _SUNOS) || (defined _LINUX26)
    int    fd ;
    /* Sun use their own proporiety PTY system. Refer to the AnswerBook
     * documentation for "Pseudo-TTY Drivers" - ptm(7) and pts(7) */
    fd = open("/dev/ptmx", O_RDWR); /* open master */
    grantpt(fd);                    /* change permission of slave */
    unlockpt(fd);                   /* unlock slave */
    meStrcpy (ptyName,ptsname(fd)); /* Slave name */        
    return fd;
#else
    int    fd ;
    struct stat stb ;
    register int c, ii ;

    /* Some systems name their pseudoterminals so that there are gaps in
       the usual sequence - for example, on HP9000/S700 systems, there
       are no pseudoterminals with names ending in 'f'.  So we wait for
       three failures in a row before deciding that we've reached the
       end of the ptys.  */
    int failed_count = 0;

    for (c ='p' ; c <= 'z' ; c++)
    {
        for (ii=0 ; ii<16 ; ii++)
        {
#ifdef _HPUX
            sprintf((char *)ptyName,"/dev/ptym/pty%c%x", c, ii);
#else
            sprintf((char *)ptyName,"/dev/pty%c%x",c,ii);
#endif
/*            printf("Im trying [%s]\n",ptyName) ;*/
            if(stat((char *)ptyName,&stb) < 0)
            {
                /* Cannot open PTY */
/*                printf("Failed\n") ;*/
                failed_count++;
                if (failed_count >= 3)
                    return -1 ;
            }
            else
            {
                /* Found a potential pty */
                failed_count = 0;
                fd = open((char *)ptyName,O_RDWR,0) ;
                if(fd >= 0)
                {
#if 0
                    /* Jon: POSIX - this is what we should be doing */
                    /* Set up the permissions of the slave */
                    if (grantpt (fd) < 0)
                    {
                        close (fd);
                        continue;
                    }
                    /* Unlock the slave */
                    if (unlockpt(fd) < 0)
                    {
                        close (fd);
                        continue;
                    }
                    else
                    {
                        char *p;
                        if ((p = ptsname (fd)) == NULL)
                        {
                            close (fd);
                            continue;
                        }
                        meStrcpy (ptyName, p);
                    }
#endif
                    /* check to make certain that both sides are available
                       this avoids a nasty yet stupid bug in rlogins */
#ifdef _HPUX
                    sprintf((char *)ptyName,"/dev/pty/tty%c%x",c,ii);
#else
                    sprintf((char *)ptyName,"/dev/tty%c%x",c,ii);
#endif
                    /* If we can read and write the tty then it is not in use. */
                    if(access((char *)ptyName,W_OK|R_OK) != 0)
                    {
                        /* tty in use, close down the pty and try the next one */
                        close (fd);
                        continue;
                    }
                    /* setupPty(fd) ;*/
                    return fd ;
                }
                else
                {
                    /* printf("Couldn't open\n") ;*/
                }
            }
        }
    }
    return -1;
#endif /* _SUNOS or _LINUX26 */
#endif /* _IRIX */
}


/* childSetupTty; Restore the correct terminal settings on the child tty
 * process. We restore the settings from our initial save of the environmet. */
static void
childSetupTty(void)
{
#ifdef _USG
#ifdef _TERMIOS
    extern struct termios otermio ;
    struct termios ntermio ;
#else
    extern struct termio otermio ;
    struct termio ntermio ;
#endif
    ntermio = otermio;

    /* the new ipipe stuff needs the echo to do its terminal output */
    ntermio.c_lflag |= (ECHO|ECHOE|ECHOK) ;

#ifdef _TERMIOS
    tcsetattr (0, TCSAFLUSH, &ntermio);
#else
    ioctl(0, TCSETA, &ntermio); /* and activate them */
#endif /* _TERMIOS */
#endif /* _USG */

#ifdef _BSD
    extern struct sgttyb  osgttyb;      /* Old tty context */
    extern struct tchars  otchars;      /* Old terminal context */
    extern struct ltchars oltchars;     /* Old line discipline context */

    /* Restore the terminal settings */
    stty (0, &osgttyb);
#ifdef _BSD_CBREAK
    ioctl (0, TIOCSETC, &otchars) ;
#endif
    ioctl (0, TIOCSLTC, &oltchars) ;
#endif /* _BSD */
}
#endif

static int
doIpipeCommand(meUByte *comStr, meUByte *path, meUByte *bufName, int ipipeFunc, int flags)
{
    meIPipe  *ipipe ;
    meBuffer *bp ;
    meUByte   line[meBUF_SIZE_MAX] ;
    int       cd ;
#ifdef _UNIX
    int       fds[2], outFds[2], ptyFp ;
    int       pid;                   /* Child process identity */
#endif
#ifdef _WIN32
    int       rr ;
#endif
    /* get or create the command buffer */
    if(((bp=bfind(bufName,0)) != NULL) && meModeTest(bp->mode,MDPIPE))
    {
        sprintf((char *)line,"%s already active, kill",bufName) ;
        if(mlyesno(line) <= 0)
            return meFALSE ;
    }
    if((ipipe = meMalloc(sizeof(meIPipe))) == NULL)
        return meFALSE ;
    cd = (meStrcmp(path,curdir) && (meChdir(path) != -1)) ;

#ifdef _WIN32
    /* Launch the ipipe */
    if((rr=WinLaunchProgram(comStr,(LAUNCH_IPIPE|flags), NULL, NULL, ipipe, NULL)) <= 0)
    {
        if(cd)
            meChdir(curdir) ;
        free(ipipe) ;
        if(rr == meABORT)
            /* returns meABORT when trying to IPIPE a DOS app on win95 (it
             * doesn't work) Try doPipe instead and maintain the same
             * environment as the macros may rely on callbacks etc. */
            return doPipeCommand(comStr,path,bufName,ipipeFunc,flags) ;
        return meFALSE;
    }
#else

    /* Allocate a pseudo terminal to do the work */
    if((ptyFp=allocatePty(line)) >= 0)
    {
        fds[0] = outFds[1] = ptyFp ;
#if ((defined _LINUX_BASE) || (defined _FREEBSD_BASE) || (defined _SUNOS) || (defined _BSD))
        /* On the BSD systems we open the tty prior to the fork. If this is a
         * later POSIX platform then we will expect O_NOCTTY to exist and we
         * open the tty with O_NOCTTY. Do not let this terminal become our
         * controlling tty. This prevents an application from unintentionally
         * aquiring the controlling terminal as a side effect of the open. */
#if (defined O_NOCTTY)
        fds[1] = outFds[0] = open((char *) line,O_RDWR|O_NOCTTY,0) ;
#else
        fds[1] = outFds[0] = open((char *) line,O_RDWR,0) ;
#endif /* O_NOCTTY */
#else
        fds[1] = outFds[0] = -1 ;
#endif /* _LINUX/_FREEBSD/.. */
    }
    else
    {
        /* Could not get a pty use a pipe instead */
        pipe(fds) ;
        pipe(outFds) ;
    }

    /* The master end of the pty must be non-blocking. Under POSIX we can
     * apply these on the open, BSD systems then it is not possible to apply
     * these on the open. For ease of code maintainance then we apply these
     * afterwards usig fnctl.
     *
     * POSIX calls for O_NONBLOCK. BSD systems and earlier UNIX systems are
     * O_NDELAY. Pick whichever is defined - they are typically defined to the
     * same thing if both exist
     *
     * Note that these settings apply irrespective of whether we are dealing
     * with a PTY or PIPE */
#ifdef O_NONBLOCK
    if (fds[0] > 0)
        fcntl(fds[0],F_SETFL,O_NONBLOCK) ;
    if ((fds[1] > 0) && (ptyFp < 0))
        fcntl(fds[1],F_SETFL,O_NONBLOCK) ;
#else
#ifdef O_NDELAY
    if (fds[0] > 0)
        fcntl(fds[0],F_SETFL,O_NDELAY) ;
    if ((fds[1] > 0) && (ptyFp < 0))
        fcntl(fds[1],F_SETFL,O_NDELAY) ;
#endif /* O_NDELAY */
#endif /* O_NONBLOCK */

    /* Hold up the child and alarm signals */
    meSigHold ();

    /* Create the new child process */
    if((pid=meFork()) == 0)
    {
        /******************************************************************
        * CHILD CHILD CHILD CHILD CHILD CHILD CHILD CHILD CHILD CHILD     *
        ******************************************************************/
        char *args[4];		/* command line send to shell */
        meUByte *ss ;

        /* Dissassociate the new process from the controlling terminal */
#if (defined _BSD) && (defined TIOCNOTTY)
        /* Under BSD then we allocate a dummy tty and then immediatly shut it.
         * This has the desired effect of dissassociating the terminal */
        if (ptyFp >= 0)
        {
            /* Under BSD 4.2 then we have to break the tty off. We make a
             * dummy call to open a tty and then immediately close it. This
             * was fixed on BSD4.3, but the same technique works so continue
             * to use it !! */
            int tempFp;

            if ((tempFp = open ("/dev/tty", O_RDWR, 0)) >= 0)
            {
                ioctl (tempFp, TIOCNOTTY);
                close (tempFp);
            }
        }
        
        /* Put the process into parent group 0. Note that setsid() does the
         * same job under SVR4. */
        setpgrp (0,0);                  /* BSD */
#else
        /* Under POSIX.1 environments then simply use setsid() to
         * dissassociate from the terminal. This will also sort out the group
         * ID's groups. */
        setsid ();                      /* Disassociate terminal */

        /* Assign the parent group. Old System V has a setpgrp() with no
         * arguments. Newer programs should use setgpid() instead. It's
         * debatable if we actually need this because setsid() might do it,
         * however no harm will come from re-assigning the parent group. */
#ifdef _SVID   
        setpgrp ();                     /* Old System V */
#else
        setpgid (0,0);                  /* Newer UNIX systems */
#endif        
        /* Not sure what the hell this does, why is it here ?? */
#if (defined TIOCSCTTY) && ((defined _LINUX_BASE) || (defined _FREEBSD_BASE))
        if((ptyFp >= 0) && (outFds[0] >= 0))
            ioctl (outFds[0],TIOCSCTTY,0);
#endif
#endif
        /* On BSD systems then we should try and use the new Barkley line
         * disciplines for communication, especially if we are a pty otherwise
         * we will get some problems with the shell. For simple pipes we do
         * not need to bother. */
#if (defined _BSD) && (defined NTTYDISC) && (defined TIOCSETD)
        if ((ptyFp >= 0) && (outFds[0] >= 0))
        {
            /* Use new line discipline.  */
            int ldisc = NTTYDISC;
            ioctl (outFds[0], TIOCSETD, &ldisc);
        }
#endif /* defined (NTTYDISC) && defined (TIOCSETD) */

        /* The child process has inherited the parent signals. Ensure that all
         * of the signals are reset to their correct default value */
#ifdef _POSIX_SIGNALS
        {
            struct sigaction sa ;

            sigemptyset(&sa.sa_mask) ;
            sa.sa_flags=SA_RESETHAND;
            sa.sa_handler=NULL ;
            sigaction(SIGHUP,&sa,NULL) ;
            sigaction(SIGINT,&sa,NULL) ;
            sigaction(SIGQUIT,&sa,NULL) ;
            sigaction(SIGTERM,&sa,NULL) ;
            sigaction(SIGUSR1,&sa,NULL) ;
            sigaction(SIGALRM,&sa,NULL) ;
            sigaction(SIGCHLD,&sa,NULL) ;

            /* Release any signals that might be blocked */
            sigprocmask(SIG_SETMASK,&sa.sa_mask,NULL);
        }
#else
        signal(SIGHUP,SIG_DFL) ;
        signal(SIGINT,SIG_DFL) ;
        signal(SIGQUIT,SIG_DFL) ;
        signal(SIGTERM,SIG_DFL) ;
        signal(SIGUSR1,SIG_DFL) ;
        signal(SIGALRM,SIG_DFL) ;
        signal(SIGCHLD,SIG_DFL) ;
#ifdef _BSD_SIGNALS
        /* Release any signals that might be blocked */
        sigsetmask (0);
#endif /* _BSD_SIGNALS */
#endif /* _POSIX_SIGNALS  */

#if !((defined _LINUX_BASE) || (defined _FREEBSD_BASE) || (defined _SUNOS) || (defined _BSD))
        /* Some systems the tty is opened late as here */
        if(ptyFp >= 0)
        {
            fds[1] = outFds[0] = open((const char *)line,O_RDWR,0) ;
        }
#endif /* !_LINUX/_FREEBSD/_SUNOS/_BSD */

        /* On solaris (this is POSIX I believe) then push the line emulation
         * modes */
#ifdef _SUNOS
        if(ptyFp >= 0)
        {
            /* Push the hardware emulation mode */
            ioctl (fds[1], I_PUSH, "ptem");
            
            /* Push the line discipline module */
            ioctl (fds[1], I_PUSH, "ldterm");
        }
#endif /* _SUNOS */

        /* Close the existing stdin/out/err */
        close (0);
        close (1);
        close (2);

        /* Duplicate the new descriptors on stdin/out/err */
        dup2 (outFds[0],0);
        dup2 (fds[1],1);
        dup2 (fds[1],2);                /* stdout => stderr */

        /* Dispose of the descriptors */
        close (outFds[0]) ;
        close (fds[1]) ;

        /* Fix up the line disciplines */
        childSetupTty() ;

        /* Set up the arguments for the pipe */
        if((ss=getUsrVar((meUByte *)"ipipe-term")) != NULL)
            mePutenv(meStrdup(ss)) ;
        
        ss = getShellCmd() ;
        args[0] = (char *) ss ;
        if(meStrcmp(ss,comStr))
        {
            args[1] = "-c" ;
            args[2] = (char *) comStr ;
            args[3] = NULL ;
        }
        else
            args[1] = NULL ;

#ifndef _NOPUTENV
        execv(args[0],args) ;
#else
        /* We need to push the environment variable, however in order to do
         * this then we need to supply the absolute pathname of the
         * executable. Search the $PATH for the executable. */
        if (meEnviron != NULL)
        {
            char buf[meBUF_SIZE_MAX];

            if (executableLookup (args[0], buf))
                args[0] = buf;
            execve (args[0], args, meEnviron) ;
        }
        else
            execv(args[0],args) ;
#endif
        exit(1) ;                       /* Should never get here unless we fail */
    }
    ipipe->pid = pid ;
    ipipe->exitCode = 0 ;
    ipipe->rfd = fds[0] ;
    ipipe->outWfd = outFds[1] ;
#endif /* _WIN32 */
    
    if(cd)
        meChdir(curdir) ;
    
    /* Link in the ipipe */
    ipipe->next = ipipes ;
    ipipes = ipipe ;
    noIpipes++ ;

#ifdef _UNIX
    /* Release the signals - we can now cope if the child dies. */
    meSigRelease ();
#endif /*_UNIX */

    /* Create the output buffer */
    {
        meMode sglobMode ;
        meModeCopy(sglobMode,globMode) ;
        if (flags & (LAUNCH_RAW|LAUNCH_NO_WRAP))
            meModeClear(globMode,MDWRAP) ;
        else
            meModeSet(globMode,MDWRAP) ;
        meModeSet(globMode,MDPIPE) ;
        meModeSet(globMode,MDLOCK) ;
        meModeClear(globMode,MDUNDO) ;
        bp=bfind(bufName,BFND_CREAT|BFND_CLEAR) ;
        meModeCopy(globMode,sglobMode) ;
    }
    if((ipipe->bp = bp) == NULL)
    {
        ipipeRemove(ipipe) ;
        return mlwrite(MWABORT,(meUByte *)"[Failed to create %s buffer]",bufName) ;
    }
    /* setup the buffer */
    if (flags & LAUNCH_BUFIPIPE)
        bp->ipipeFunc = ipipeFunc;
    bp->fileName = meStrdup(path) ;
    if ((flags & LAUNCH_RAW) == 0)
    {
        meStrcpy(line,"cd ") ;
        meStrcat(line,path) ;
        addLineToEob(bp,line) ;
        addLineToEob(bp,comStr) ;
        addLineToEob(bp,(meUByte *)"\n") ;
    }
    bp->dotLine = meLineGetPrev(bp->baseLine) ;
    bp->dotOffset = 0 ;
    bp->dotLineNo = bp->lineCount-1 ;
    meAnchorSet(bp,'I',bp->dotLine,bp->dotOffset,1) ;

    /* Set up the window dimensions - default to having auto wrap */
    ipipe->flag = 0 ;
    if ((flags & LAUNCH_RAW) != 0)
        ipipe->flag |= meIPIPE_RAW ;
    ipipe->strRow = 0 ;
    ipipe->strCol = 0 ;
    ipipe->noRows = 0 ;
    ipipe->noCols = frameCur->windowCur->textWidth-1 ;
    ipipe->curRow = (meShort) bp->dotLineNo ;
    /* get a popup window for the command output */
    {
        meWindow *wp ;
        if((flags & LAUNCH_SILENT) || ((wp = meWindowPopup(bp->name,0,NULL)) == NULL))
            wp = frameCur->windowCur ;
        /* while executing the meWindowPopup function the ipipe could have exited so check */
        if(ipipes == ipipe)
        {
            ipipeSetSize(wp,bp) ;
            if(bp->ipipeFunc >= 0)
                /* Give argument of 1 to indicate process has not exited */
                execBufferFunc(bp,bp->ipipeFunc,(meEBF_ARG_GIVEN|meEBF_USE_B_DOT|meEBF_HIDDEN),1) ;
        }
    }
    /* reset again incase there was a delay in the meWindowPopup call */
    bp->dotLine = meLineGetPrev(bp->baseLine) ;
    bp->dotOffset = 0 ;
    bp->dotLineNo = bp->lineCount-1 ;
    resetBufferWindows(bp) ;

    return meTRUE ;
}

int
ipipeCommand(int f, int n)
{
    meBuffer *bp ;
    meUByte lbuf[meBUF_SIZE_MAX], *cl ;	/* command line send to shell */
    meUByte nbuf[meBUF_SIZE_MAX], *bn ;	/* buffer name */
    meUByte pbuf[meBUF_SIZE_MAX] ;
    int ipipeFunc ;                     /* ipipe-buffer function. */
    int ss ;

    if(!(meSystemCfg & meSYSTEM_IPIPES))
        /* No ipipes flagged so just do a normal pipe */
        return pipeCommand(f,n) ;
    
    /* Get the command to pipe in */
    if((ss=meGetString((meUByte *)"Ipipe", 0, 0, lbuf, meBUF_SIZE_MAX)) <= 0)
        return ss ;
    if(n & LAUNCH_BUFCMDLINE)
    {
        if((bp=bfind(lbuf,0)) == NULL)
            return mlwrite(MWABORT,(meUByte *)"[%s: no such buffer]",lbuf);
        cl = meLineGetText(meLineGetNext(bp->baseLine)) ;
    }
    else
        cl = lbuf ;
    
    if((n & LAUNCH_BUFFERNM) == 0)
    {
        /* prompt for and get the new buffer name */
        if((ss = getBufferName((meUByte *)"Buffer", 0, 0, nbuf)) <= 0)
            return ss ;
        bn = nbuf ;
    }
    else
        bn = BicommandN ;

    /* Get the buffer ipipe if requested. */
    if ((n & LAUNCH_BUFIPIPE) != 0)
    {
        /* prompt for the $buffer-ipipe */
        if ((ss = meGetString((meUByte *)"Command", MLCOMMAND, 0,
                              pbuf, meBUF_SIZE_MAX)) <= 0)
            return ss;
        ipipeFunc = decode_fncname(pbuf,1);
    }
    else
        ipipeFunc = -1;
    
    getFilePath(frameCur->bufferCur->fileName,pbuf) ;
    return doIpipeCommand(cl,pbuf,bn,ipipeFunc,(n & LAUNCH_USER_FLAGS)) ;
}

int
anyActiveIpipe(void)
{
    if((ipipes == NULL) ||
#if MEOPT_CLIENTSERVER
       ((ipipes->pid == 0) && (ipipes->next == NULL))
#endif
       )
        return meFALSE ;
    return meTRUE ;
}

#endif


/*
 * Pipe a one line command into a window
 * Bound to ^X @
 */
int
doPipeCommand(meUByte *comStr, meUByte *path, meUByte *bufName, int ipipeFunc, int flags)
{
    register meBuffer *bp;	/* pointer to buffer to zot */
    meUByte buff[meBUF_SIZE_MAX+3] ;
    meInt systemRet ;
    int cd, ret ;
#ifdef _DOS
    static meByte pipeStderr=0 ;
    meUByte cc, *dd, *cl, *ss ;
    int gotPipe=0 ;
#endif
#ifdef _UNIX
    meWAIT_STATUS ws;
    meUByte *cl, *ss ;
    size_t ll ;
#else
    meUByte filnam[meBUF_SIZE_MAX] ;

    mkTempCommName(filnam,COMMAND_FILE) ;
#endif

    /* get rid of the output buffer if it exists and create new */
    if((bp=bfind(bufName,BFND_CREAT|BFND_CLEAR)) == NULL)
        return meFALSE ;
    cd = (meStrcmp(path,curdir) && (meChdir(path) != -1)) ;

#ifdef _DOS
    if(!pipeStderr)
        pipeStderr = (meGetenv("ME_PIPE_STDERR") != NULL) ? 1 : -1 ;
    
    if((cl = meMalloc(meStrlen(comStr) + meStrlen(filnam) + 4)) == NULL)
        return meFALSE ;
        
    dd = cl ;
    ss = comStr ;
    /* convert '/' to '\' in program name */
    while((cc=*ss++) && (cc != ' '))
    {
        if(cc == '/')
            cc = '\\' ;
        *dd++ = cc ;
    }
    if(cc)
    {
        *dd++ = cc ;
        while((cc=*ss++))
        {
            if((cc == ' ') && (*ss == '>'))
                gotPipe = 1 ;
            *dd++ = cc ;
        }
    }
    *dd = '\0' ;
    if(!gotPipe)
    {
        *dd++ = ' ' ;
        *dd++ = '>' ;
        if(pipeStderr > 0)
            *dd++ = '&' ;
        strcpy(dd,filnam);
    }
    if((flags & LAUNCH_SILENT) == 0)
        mlerase(MWERASE|MWCURSOR);
    systemRet = system(cl) ;
    if(cd)
        meChdir(curdir) ;
    /* Call TTopen as we can't guarantee whats happend to the terminal */
    TTopen();
    meFree(cl) ;
    if(meTestExist(filnam))
        return meFALSE;
#endif
#ifdef _WIN32
    
    ret = WinLaunchProgram(comStr,(LAUNCH_PIPE|flags), NULL, filnam,
#if MEOPT_IPIPES
                           NULL, 
#endif
                           &systemRet) ;
    if(cd)
        meChdir(curdir) ;
    if(ret == meFALSE)
        return meFALSE ;

#endif
#ifdef _UNIX
    ll = meStrlen(comStr) ;
    if((cl = meMalloc(ll + 17)) == NULL)
        return meFALSE ;
    
    if((ss=meStrchr(comStr,'|')) == NULL)
        ss = comStr + ll ;
    else
        ll = (size_t)(ss - comStr) ;
    
    meStrncpy(cl,comStr,ll) ;
    cl[ll] = '\0' ;
    /* if no data is piped in then pipe in /dev/null */
    if(meStrchr(cl,'<') == NULL)
    {
        memcpy(cl+ll," </dev/null",11) ;
        ll += 11 ;
    }
    /* merge stderr and stdout */
    memcpy(cl+ll," 2>&1",5) ;
    ll += 5 ;
    meStrcpy(cl+ll,ss) ;
    
    TTclose();				/* stty to old modes    */
    /* Must flag to our sigchild handler that we are running a piped command
     * otherwise it will call waitpid with -1 and loose the exit status of
     * this process */
    alarmState |= meALARM_PIPE_COMMAND ;
    meio.rp = popen((char *) cl, "r") ;
    if(cd)
        meChdir(curdir) ;
    TTopen();
    TTflush();
    meFree(cl) ;
#endif
    if ((flags & LAUNCH_RAW) == 0)
    {
        meStrcpy(buff,"cd ") ;
        meStrcpy(buff+3,path) ;
        addLineToEob(bp,buff) ;
        addLineToEob(bp,comStr) ;
        addLineToEob(bp,(meUByte *)"") ;
    }
    /* If this is an ipipe launch then call the callback. */
    if ((flags & LAUNCH_BUFIPIPE) && (ipipeFunc >= 0))
    {
#if MEOPT_IPIPES
        bp->ipipeFunc = ipipeFunc;
#endif
        execBufferFunc(bp,ipipeFunc,(meEBF_ARG_GIVEN|meEBF_USE_B_DOT|meEBF_HIDDEN),1) ;
    }
    /* and read the stuff in */
#ifdef _UNIX
    ret = meBufferInsertFile(bp,NULL,meRWFLAG_SILENT|meRWFLAG_PRESRVFMOD,0,0,0) ;
    /* close the pipe and get exit status */
    ws = (meWAIT_STATUS) pclose(meio.rp) ;
    if(WIFEXITED(ws))
        systemRet = WEXITSTATUS(ws) ;
    else
        systemRet = -1 ;
    alarmState &= ~meALARM_PIPE_COMMAND ;
#else
    ret = meBufferInsertFile(bp,filnam,meRWFLAG_SILENT|meRWFLAG_PRESRVFMOD,0,0,0) ;
    /* and get rid of the temporary file */
    meUnlink(filnam);
#endif
    
    /* give it the path as a filename */
    bp->fileName = meStrdup(path) ;
    /* make this window in VIEW mode, update all mode lines */
    meModeClear(bp->mode,MDEDIT) ;
    meModeSet(bp->mode,MDVIEW) ;
    bp->dotLine = meLineGetNext(bp->baseLine) ;
    bp->dotLineNo = 0 ;
    resetBufferWindows(bp) ;

    if((flags & LAUNCH_SILENT) == 0)
        meWindowPopup(bp->name,WPOP_MKCURR,NULL) ;

    /* Issue the callback if required. */
#if MEOPT_IPIPES
    ipipeFunc = bp->ipipeFunc;
    if ((flags & LAUNCH_BUFIPIPE) && (ipipeFunc >= 0))
        execBufferFunc(bp,ipipeFunc,(meEBF_ARG_GIVEN|meEBF_USE_B_DOT|meEBF_HIDDEN),0) ;
#endif 
    meStrcpy(resultStr,meItoa(systemRet)) ;
    return ret ;
}

/*
 * Pipe a one line command into a window
 * Bound to ^X @
 */
int
pipeCommand(int f, int n)
{
    register int ss ;
    meBuffer *bp ;
    meUByte lbuf[meBUF_SIZE_MAX], *cl ; /* command line send to shell */
    meUByte nbuf[meBUF_SIZE_MAX], *bn ;	/* buffer name */
    meUByte pbuf[meBUF_SIZE_MAX] ;

    /* get the command to pipe in */
    if((ss=meGetString((meUByte *)"Pipe", 0, 0, lbuf, meBUF_SIZE_MAX)) <= 0)
        return ss ;
    if(n & LAUNCH_BUFCMDLINE)
    {
        if((bp=bfind(lbuf,0)) == NULL)
            return mlwrite(MWABORT,(meUByte *)"[%s: no such buffer]",lbuf);
        cl = meLineGetText(meLineGetNext(bp->baseLine)) ;
    }
    else
        cl = lbuf ;
    
    if((n & LAUNCH_BUFFERNM) == 0)
    {
        /* prompt for and get the new buffer name */
        if((ss = getBufferName((meUByte *)"Buffer", 0, 0, nbuf)) <= 0)
            return ss ;
        bn = nbuf ;
    }
    else
        bn = BcommandN ;

    getFilePath(frameCur->bufferCur->fileName,pbuf) ;

    return doPipeCommand(cl,pbuf,bn,-1,(n & LAUNCH_USER_FLAGS)) ;
}

#if MEOPT_EXTENDED
/*
 * filter a buffer through an external DOS program. This needs to be rewritten
 * under UNIX to use pipes.
 *
 * Bound to ^X #
 */
int
meFilter(int f, int n)
{
    register int     s;			/* return status from CLI */
    register meBuffer *bp;		/* pointer to buffer to zot */
    meUByte            line[meBUF_SIZE_MAX];	/* command line send to shell */
    meUByte           *tmpnam ;		/* place to store real file name */
#ifdef _UNIX
    int	             exitstatus;	/* exit status of command */
#endif
    meUByte filnam1[meBUF_SIZE_MAX];
    meUByte filnam2[meBUF_SIZE_MAX];

    /* Construct the filter names */
    mkTempName (filnam1,(meUByte *)FILTER_IN_FILE,NULL);
    mkTempName (filnam2,(meUByte *)FILTER_OUT_FILE,NULL);

    /* get the filter name and its args */
    if ((s=meGetString((meUByte *)"Filter", 0, 0, line, meBUF_SIZE_MAX)) <= 0)
        return s ;

    if((s=bufferSetEdit()) <= 0)               /* Check we can change the buffer */
        return s ;

    /* setup the proper file names */
    bp = frameCur->bufferCur;
    tmpnam = bp->fileName ;	/* save the original name */
    bp->fileName = NULL ;	/* set it to NULL         */

    /* write it out, checking for errors */
    if(writeOut(bp,meRWFLAG_SILENT,filnam1) <= 0)
    {
        bp->fileName = tmpnam ;
        return mlwrite(MWABORT,(meUByte *)"[Cannot write filter file]");
    }

#ifdef _DOS
    strcat(line," <");
    strcat(line, filnam1);
    strcat(line," >");
    strcat(line, filnam2);
    mlerase(MWERASE|MWCURSOR);
    system(line);
    sgarbf = meTRUE;
    s = meTRUE;
#endif
#ifdef _WIN32
    s = WinLaunchProgram(line,LAUNCH_FILTER,filnam1,filnam2,
#if MEOPT_IPIPES
                         NULL,
#endif
                         NULL);
    sgarbf = meTRUE;
#endif
#ifdef _UNIX
    TTclose();			/* stty to old modes	*/
    meStrcat(line," <");
    meStrcat(line, filnam1);
    meStrcat(line," >");
    meStrcat(line, filnam2);
    errno = 0;			/* clear errno before call */
    if((exitstatus = system((char *)line)) != 0)
    {
        if(errno)
            mlwrite(MWCURSOR|MWWAIT,(meUByte *)"exit status %d ", exitstatus);
        else
            mlwrite(MWCURSOR|MWWAIT,(meUByte *)"exit status %d, errno %d ",
                    exitstatus, errno);
    }
    TTopen();
    sgarbf = meTRUE;
    s = meTRUE;
#endif

    /* on failure, escape gracefully */
    if(s > 0)
    {
        bp->fileName = filnam2 ;
        if((bclear(bp) <= 0) ||
           ((frameCur->bufferCur->intFlag |= BIFFILE),(swbuffer(frameCur->windowCur,frameCur->bufferCur) <= 0)))
            s = meFALSE ;
    }
    /* reset file name */
    bp->fileName = tmpnam ;
    /* and get rid of the temporary file */
    meUnlink(filnam1);
    meUnlink(filnam2);

    if(s <= 0)
        mlwrite(0,(meUByte *)"[Execution failed]");
    else
        meModeSet(bp->mode,MDEDIT) ;		/* flag it as changed */

    return s ;
}
#endif

#ifdef _UNIX
#if MEOPT_EXTENDED
int
suspendEmacs(int f, int n)		/* suspend MicroEMACS and wait to wake up */
{
    /*
    ** Note that we might have got here by hitting the wrong keys. If you've
    ** ever tried suspending something when you havent got job control in your
    ** shell, its painful.
    **
    ** Confirm with the user that they want to suspend if the basename of the
    ** SHELL environment variable is NOT "ksh" or "csh" and it hasnt got a "j"
    ** in it.
    */
    if((n & 0x01) && (mlyesno((meUByte *)"Suspend") <= 0))
        return meFALSE ;

    TTclose();				/* stty to old settings */
    kill(getpid(), SIGTSTP);
    //printf("\033[?47l\n");
    TTopen();
    sgarbf = meTRUE;

    return meTRUE ;
}
#endif
#endif

#endif /* MEOPT_SPAWN */
