/***************************************************************************
                          jdbcontroller.h  -  description
                             -------------------
    begin                : Sun Aug 8 1999
    copyright            : (C) 1999 by John Birch
    email                : jb.nz@writeme.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _JDBCONTROLLER_H_
#define _JDBCONTROLLER_H_

#include "dbgcontroller.h"
#include <qobject.h>
#include <qlist.h>

class Breakpoint;
class DbgCommand;
class FramestackWidget;
class KProcess;
class QString;
class VarItem;
class VariableTree;
class STTY;

/**
 * A front end implementation to the jdb command line debugger
 * @author jbb
 */

class JDBController : public DbgController
{
    Q_OBJECT

public:
    JDBController(VariableTree *varTree, FramestackWidget *frameStack);
    ~JDBController();
    void reConfig();

protected:
    void queueCmd(DbgCommand *cmd, bool executeNext=false);

private:
    void parseProgramLocation (char *buf);
    void parseBacktraceList   (char *buf);
    void parseBreakpointSet   (char *buf);
    void parseLocals          (char *buf);
    void parseRequestedData   (char *buf);
    void parseLine            (char *buf);
    void parseFrameSelected   (char *buf);
    //  void parseFileStart       (char *buf);

    char *parse               (char *buf);
    char *parseOther          (char *buf);
    char *parseCmdBlock       (char *buf);

    void pauseApp();
    void executeCmd ();
    void destroyCmds();
    void removeInfoRequests();
    void actOnProgramPause(const QString &msg);
    void programNoApp(const QString &msg, bool msgBox);

    void setBreakpoint(const QCString &BPSetCmd, int key);
    void clearBreakpoint(const QCString &BPClearCmd);
    void modifyBreakpoint(Breakpoint *BP);

    void setStateOn(int stateOn)    { state_ |= stateOn; }
    void setStateOff(int stateOff)  { state_ &= ~stateOff; }
    bool stateIsOn(int state)       { return state_  &state; }

public slots:
    void slotStart(const QString &application, const QString &args, const QString &sDbgShell=QString());

    void slotRun();
    void slotStepInto();
    void slotStepOver();
    void slotStepIntoIns();
    void slotStepOutOff();

    void slotBreakInto();
    void slotBPState(Breakpoint *BP);
    void slotClearAllBreakpoints();

    void slotDisassemble(const QString &start, const QString &end);
    void slotExpandItem(VarItem *parent);
    void slotExpandUserItem(VarItem *parent, const QCString &userRequest);
    void slotSelectFrame(int frame);
    void slotSetLocalViewState(bool onOff);

protected slots:
    void slotDbgStdout(KProcess *proc, char *buf, int buflen);
    void slotDbgStderr(KProcess *proc, char *buf, int buflen);
    void slotDbgWroteStdin(KProcess *proc);
    void slotDbgProcessExited(KProcess *proc);
    void slotStepInSource(const QString &fileName, int lineNum);
    void slotDbgStatus(const QString &status, int state);

private slots:
    void slotAbortTimedEvent();

signals:
    void rawData              (const QString &rawData);
    void showStepInSource     (const QString &fileName, int lineNum, const QString &address);
    void rawJDBBreakpointList (char *buf);
    void rawJDBBreakpointSet  (char *buf, int key);
    void ttyStdout            (const char *output);
    void ttyStderr            (const char *output);
    void dbgStatus            (const QString &status, int statusFlag);
    void acceptPendingBPs     ();
    void unableToSetBPNow     (int BPNo);

private:
    FramestackWidget* frameStack_;
    VariableTree*     varTree_;
    int               currentFrame_;

    int               state_;
    int               jdbSizeofBuf_;          // size of the output buffer
    int               jdbOutputLen_;          // amount of data in the output buffer
    char*             jdbOutput_;             // buffer for the output from kprocess

    QList<DbgCommand> cmdList_;
    DbgCommand*       currentCmd_;

    STTY*             tty_;
    bool              programHasExited_;

    // Configuration values
    bool    config_forceBPSet_;
    bool    config_displayStaticMembers_;
    bool    config_dbgTerminal_;
    QString config_jdbPath_;
};

#endif
