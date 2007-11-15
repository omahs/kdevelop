/*
  Copyright 2007 Roberto Raggi <roberto@kdevelop.org>
  Copyright 2007 Hamish Rodda <rodda@kde.org>

  Permission to use, copy, modify, distribute, and sell this software and its
  documentation for any purpose is hereby granted without fee, provided that
  the above copyright notice appear in all copies and that both that
  copyright notice and this permission notice appear in supporting
  documentation.

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  KDEVELOP TEAM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef SUBLIME_IDEAL_H
#define SUBLIME_IDEAL_H

#include <QtGui>

#include "ideallayout.h"

class KAction;
class KActionCollection;

namespace Sublime {

class MainWindow;

class IdealToolButton: public QToolButton
{
    Q_OBJECT

    enum { DefaultButtonSize = 20 };

public:
    IdealToolButton(Qt::DockWidgetArea area, QWidget *parent = 0);

    Qt::Orientation orientation() const;

    virtual QSize sizeHint() const;

protected:
    virtual void paintEvent(QPaintEvent *event);

private:
    Qt::DockWidgetArea _area;
};

class IdealButtonBarWidget: public QWidget
{
    Q_OBJECT

public:
    IdealButtonBarWidget(Qt::DockWidgetArea area, class IdealMainWidget *parent = 0);

    QWidgetAction *addWidget(const QString& title, QDockWidget *widget);
    void showWidget(QDockWidget* widget);
    void removeAction(QWidgetAction* action);

    IdealMainWidget* parentWidget() const;

    Qt::Orientation orientation() const;

private Q_SLOTS:
    void showWidget(bool checked);
    void anchor(bool anchor);

    void actionToggled(bool state);

protected:
    virtual void resizeEvent(QResizeEvent *event);

    virtual void actionEvent(QActionEvent *event);

private:
    Qt::DockWidgetArea _area;
    QHash<QWidgetAction *, IdealToolButton *> _buttons;
    QActionGroup* _actions;
};

class IdealDockWidgetTitle : public QWidget
{
    Q_OBJECT

public:
    IdealDockWidgetTitle(Qt::Orientation orientation, QDockWidget* parent, QWidgetAction* showAction);
    virtual ~IdealDockWidgetTitle();

    bool isAnchored() const;
    void setAnchored(bool anchored, bool emitSignals);

Q_SIGNALS:
    void anchor(bool anchor);
    void close();

private Q_SLOTS:
    void slotAnchor(bool anchor);
    
private:
    Qt::Orientation m_orientation;
    QToolButton* m_anchor;
};

class IdealCentralWidget : public QWidget
{
    Q_OBJECT

public:
    IdealCentralWidget(IdealMainWidget* parent);
    virtual ~IdealCentralWidget();

    IdealMainLayout* idealLayout() const;

protected:
    virtual void paintEvent(QPaintEvent* event);
};

class View;

class IdealMainWidget : public QWidget
{
    Q_OBJECT

public:
    IdealMainWidget(MainWindow* parent, KActionCollection* ac);

    // Public api
    void setCentralWidget(QWidget* widget);
    QWidgetAction* actionForView(View* view) const;
    void addView(Qt::DockWidgetArea area, View* View);
    void raiseView(View* view);
    void removeView(View* view);

    // Internal api

    // TODO can move the object filter here with judicious focusProxy?
    void centralWidgetFocused();

    void showDockWidget(QDockWidget* widget, bool show);
    void showDock(IdealMainLayout::Role role, bool show);

    void anchorDockWidget(QDockWidget* widget, bool anchor);

    IdealMainLayout* mainLayout() const;
    IdealCentralWidget* internalCentralWidget() const;

    void anchorDockWidget(bool checked, IdealButtonBarWidget* bar);

    QWidget* firstWidget(IdealMainLayout::Role role) const;

    IdealButtonBarWidget* barForRole(IdealMainLayout::Role role) const;

    void setAnchorActionStatus(bool checked);

public Q_SLOTS:
    void showLeftDock(bool show);
    void showRightDock(bool show);
    void showBottomDock(bool show);
    void showTopDock(bool show);
    void hideAllDocks();
    void anchorCurrentDock(bool anchor);

private:
    IdealButtonBarWidget *leftBarWidget;
    IdealButtonBarWidget *rightBarWidget;
    IdealButtonBarWidget *bottomBarWidget;
    IdealButtonBarWidget *topBarWidget;

    KAction* m_anchorCurrentDock;

    IdealCentralWidget* mainWidget;
    class IdealMainLayout* m_mainLayout;

    QMap<QDockWidget*, Qt::DockWidgetArea> docks;
    QMap<View*, QWidgetAction*> views;
    QMap<QDockWidget*, QWidgetAction*> actions;
};

class IdealSplitterHandle : public QWidget
{
    Q_OBJECT

public:
    IdealSplitterHandle(Qt::Orientation orientation, QWidget* parent, IdealMainLayout::Role resizeRole);

Q_SIGNALS:
    void resize(int thickness, IdealMainLayout::Role resizeRole);
    
protected:
    virtual void paintEvent(QPaintEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);

private:
    inline int convert(const QPoint& pos) const { return m_orientation == Qt::Horizontal ? pos.y() : pos.x(); }

    Qt::Orientation m_orientation;
    bool m_hover;
    int m_dragStart;
    IdealMainLayout::Role m_resizeRole;
};

}

#endif
