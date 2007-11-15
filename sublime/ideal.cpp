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

#include "ideal.h"

#include <KIcon>
#include <kdebug.h>
#include <klocale.h>
#include <KActionCollection>
#include <KAction>
#include <KAcceleratorManager>

#include "view.h"
#include "document.h"
#include "mainwindow.h"

using namespace Sublime;

IdealToolButton::IdealToolButton(Qt::DockWidgetArea area, QWidget *parent)
    : QToolButton(parent), _area(area)
{
    setCheckable(true);
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    if (orientation() == Qt::Vertical)
        setFixedWidth(DefaultButtonSize);
    else
        setFixedHeight(DefaultButtonSize);
}

Qt::Orientation IdealToolButton::orientation() const
{
    if (_area == Qt::LeftDockWidgetArea || _area == Qt::RightDockWidgetArea)
        return Qt::Vertical;
    
    return Qt::Horizontal;
}

QSize IdealToolButton::sizeHint() const
{
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    QFontMetrics fm = fontMetrics();
    int w = fm.width(text()) + (10 + fm.width(QLatin1Char('x'))) * 2;
    w += opt.iconSize.width();
    if (orientation() == Qt::Vertical)
        return QSize(width(), w);
    return QSize(w, height());
}

void IdealToolButton::paintEvent(QPaintEvent *event)
{
    if (_area == Qt::TopDockWidgetArea || _area == Qt::BottomDockWidgetArea) {
        QToolButton::paintEvent(event);
    } else {
        QStyleOptionToolButton opt;
        initStyleOption(&opt);
        opt.rect.setSize(QSize(opt.rect.height(), opt.rect.width()));
        
        QPixmap pix(opt.rect.width(), opt.rect.height());
        
        QPainter painter(&pix);
        painter.fillRect(pix.rect(), opt.palette.color(backgroundRole()));
        style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &painter, this);
        painter.end();    
        
        QPainter p(this);
    
        if (_area == Qt::LeftDockWidgetArea) {
            p.translate(0, height());
            p.rotate(-90);
        } else {
            p.translate(width(), 0);
            p.rotate(90);
        }
    
        p.drawPixmap(0, 0, pix);    
    }
}

IdealButtonBarWidget::IdealButtonBarWidget(Qt::DockWidgetArea area, IdealMainWidget *parent)
    : QWidget(parent)
    , _area(area)
    , _actions(new QActionGroup(this))
{
    // TODO Only for now...
    _actions->setExclusive(true);

    (void) new IdealButtonBarLayout(orientation(), this);
}

QWidgetAction *IdealButtonBarWidget::addWidget(const QString& title, QDockWidget *dock)
{
    QWidgetAction *action = new QWidgetAction(this);
    action->setCheckable(true);
    action->setText(title);
    action->setIcon(dock->widget()->windowIcon());

    dock->setAutoFillBackground(true);
    //dock->setFocusProxy(widget);

    if (!dock->titleBarWidget()) {
        IdealDockWidgetTitle* title = new IdealDockWidgetTitle(orientation(), dock, action);
        dock->setTitleBarWidget(title);
        connect(title, SIGNAL(anchor(bool)), SLOT(anchor(bool)));
        //dock->setWindowOpacity(0.8);
    }

    action->setDefaultWidget(dock);
    connect(action, SIGNAL(toggled(bool)), this, SLOT(showWidget(bool)));

    addAction(action);
    _actions->addAction(action);

    return action;
}

void IdealButtonBarWidget::removeAction(QWidgetAction * action)
{
    delete _buttons.take(action);
}

Qt::Orientation IdealButtonBarWidget::orientation() const
{
    if (_area == Qt::LeftDockWidgetArea || _area == Qt::RightDockWidgetArea)
        return Qt::Vertical;

    return Qt::Horizontal;
}

void IdealButtonBarWidget::showWidget(bool checked)
{
    Q_ASSERT(parentWidget() != 0);

    QWidgetAction *action = qobject_cast<QWidgetAction *>(sender());
    Q_ASSERT(action);

    QDockWidget *widget = qobject_cast<QDockWidget*>(action->defaultWidget());
    Q_ASSERT(widget);

    parentWidget()->showDockWidget(widget, checked);
}

void IdealButtonBarWidget::resizeEvent(QResizeEvent *event)
{
    if (layout() != 0 && orientation() == Qt::Vertical)
        static_cast<IdealButtonBarLayout *>(layout())->setHeight(event->size().height());
}

void IdealButtonBarWidget::anchor(bool anchor)
{
    parentWidget()->anchorDockWidget(anchor, this);
}

void IdealButtonBarWidget::actionEvent(QActionEvent *event)
{
    QWidgetAction *action = qobject_cast<QWidgetAction *>(event->action());
    if (! action)
      return;

    switch (event->type()) {
    case QEvent::ActionAdded: {
        if (! _buttons.contains(action)) {
            IdealToolButton *button = new IdealToolButton(_area);
            _buttons.insert(action, button);
        
            button->setText(action->text());
            button->setIcon(action->icon());
            button->setChecked(action->isChecked());
            layout()->addWidget(button);
            connect(action, SIGNAL(toggled(bool)), SLOT(actionToggled(bool)));
            connect(button, SIGNAL(toggled(bool)), action, SLOT(setChecked(bool)));
        }
    } break;

    case QEvent::ActionRemoved: {
        if (IdealToolButton *button = _buttons.value(action)) {
            for (int index = 0; index < layout()->count(); ++index) {
                if (QLayoutItem *item = layout()->itemAt(index)) {
                    if (item->widget() == button) {
                        action->disconnect(this);
                        delete layout()->takeAt(index);
                        layout()->invalidate();
                        break;
                    }
                }
            }
        }
    } break;

    case QEvent::ActionChanged: {
      if (IdealToolButton *button = _buttons.value(action)) {
            button->setText(action->text());
            button->setIcon(action->icon());
            Q_ASSERT(action->defaultWidget() != 0);
            action->defaultWidget()->setWindowTitle(action->text());
        }
    } break;

    default:
        break;
    }
}

void IdealButtonBarWidget::actionToggled(bool state)
{
    QWidgetAction* action = qobject_cast<QWidgetAction*>(sender());
    Q_ASSERT(action);

    IdealToolButton* button = _buttons[action];
    Q_ASSERT(button);

    button->blockSignals(true);
    button->setChecked(state);
    button->blockSignals(false);
}

IdealDockWidgetTitle::IdealDockWidgetTitle(Qt::Orientation orientation, QDockWidget * parent, QWidgetAction* showAction)
    : QWidget(parent)
    , m_orientation(orientation)
{
    QBoxLayout* layout = 0;
    switch (m_orientation) {
        case Qt::Horizontal:
            layout = new QBoxLayout(QBoxLayout::BottomToTop, this);
            break;
        case Qt::Vertical:
            layout = new QHBoxLayout(this);
            break;
    }

    setLayout(layout);

    QLabel* title = new QLabel(this);
    title->setText(parent->windowTitle());
    layout->addWidget(title);

    layout->addStretch();

    m_anchor = new QToolButton(this);
    m_anchor->setFocusPolicy(Qt::NoFocus);
    m_anchor->setIcon(KIcon("document-decrypt"));
    m_anchor->setCheckable(true);
    connect(m_anchor, SIGNAL(toggled(bool)), SLOT(slotAnchor(bool)));
    layout->addWidget(m_anchor);

    /*QToolButton* floatb = new QToolButton(this);
    floatb->setFocusPolicy(Qt::NoFocus);
    floatb->setIcon(KIcon("exec"));
    layout->addWidget(floatb);*/

    QToolButton* close = new QToolButton(this);
    close->setFocusPolicy(Qt::NoFocus);
    close->setIcon(KIcon("dialog-close"));
    connect(close, SIGNAL(clicked(bool)), showAction, SLOT(toggle()));
    layout->addWidget(close);
}

IdealDockWidgetTitle::~ IdealDockWidgetTitle()
{
}

bool IdealDockWidgetTitle::isAnchored() const
{
    return m_anchor->isChecked();
}

void IdealDockWidgetTitle::setAnchored(bool anchored, bool emitSignals)
{
    if (!emitSignals)
        m_anchor->blockSignals(true);

    m_anchor->setChecked(anchored);
    if (anchored)
        m_anchor->setIcon(KIcon("document-encrypt"));
    else
        m_anchor->setIcon(KIcon("document-decrypt"));

    if (!emitSignals)
        m_anchor->blockSignals(false);
}

void IdealDockWidgetTitle::slotAnchor(bool anchored)
{
    if (anchored)
        m_anchor->setIcon(KIcon("document-encrypt"));
    else
        m_anchor->setIcon(KIcon("document-decrypt"));

    emit anchor(anchored);
}

IdealMainWidget::IdealMainWidget(MainWindow* parent, KActionCollection* ac)
    : QWidget(parent)
{
    leftBarWidget = new IdealButtonBarWidget(Qt::LeftDockWidgetArea);
    leftBarWidget->hide();

    rightBarWidget = new IdealButtonBarWidget(Qt::RightDockWidgetArea);
    rightBarWidget->hide();

    bottomBarWidget = new IdealButtonBarWidget(Qt::BottomDockWidgetArea);
    bottomBarWidget->hide();

    topBarWidget = new IdealButtonBarWidget(Qt::TopDockWidgetArea);
    topBarWidget->hide();

    mainWidget = new IdealCentralWidget(this);

    m_mainLayout = new IdealMainLayout(mainWidget);
    mainWidget->setLayout(m_mainLayout);

    connect(parent, SIGNAL(settingsLoaded()), m_mainLayout, SLOT(loadSettings()));

    QGridLayout *grid = new QGridLayout(this);
    grid->setMargin(0);
    grid->setSpacing(0);
    grid->addWidget(leftBarWidget, 1, 0);
    grid->addWidget(mainWidget, 1, 1);
    grid->addWidget(rightBarWidget, 1, 2);
    grid->addWidget(bottomBarWidget, 2, 0, 1, 3);
    grid->addWidget(topBarWidget, 0, 0, 1, 3);
    setLayout(grid);

    KAction* action = new KAction(i18n("Show Left Dock"), this);
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_L);
    action->setCheckable(true);
    connect(action, SIGNAL(triggered(bool)), SLOT(showLeftDock(bool)));
    ac->addAction("show_left_dock", action);

    action = new KAction(i18n("Show Right Dock"), this);
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_R);
    action->setCheckable(true);
    connect(action, SIGNAL(triggered(bool)), SLOT(showRightDock(bool)));
    ac->addAction("show_right_dock", action);

    action = new KAction(i18n("Show Bottom Dock"), this);
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_B);
    action->setCheckable(true);
    connect(action, SIGNAL(triggered(bool)), SLOT(showBottomDock(bool)));
    ac->addAction("show_bottom_dock", action);

    action = new KAction(i18n("Show Top Dock"), this);
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_T);
    action->setCheckable(true);
    connect(action, SIGNAL(triggered(bool)), SLOT(showTopDock(bool)));
    ac->addAction("show_top_dock", action);

    action = new KAction(i18n("Hide All Docks"), this);
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_H);
    connect(action, SIGNAL(triggered(bool)), SLOT(hideAllDocks()));
    ac->addAction("hide_all_docks", action);

    m_anchorCurrentDock = action = new KAction(i18n("Anchor Current Dock"), this);
    action->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_A);
    action->setCheckable(true);
    connect(action, SIGNAL(triggered(bool)), SLOT(anchorCurrentDock(bool)));
    ac->addAction("anchor_current_dock", action);
}

void IdealMainWidget::addView(Qt::DockWidgetArea area, View* view)
{
    QDockWidget *dock = new QDockWidget(mainWidget);

    KAcceleratorManager::setNoAccel(dock);

    dock->setWidget(view->widget());
    dock->setWindowTitle(view->widget()->windowTitle());

    if (IdealButtonBarWidget* bar = barForRole(roleForArea(area))) {
        actions[dock] = views[view] = bar->addWidget(view->document()->title(), dock);
        bar->show();
    }

    docks[dock] = area;
}

void IdealMainWidget::centralWidgetFocused()
{
    for (IdealMainLayout::Role role = IdealMainLayout::Left; role <= IdealMainLayout::Top; role = static_cast<IdealMainLayout::Role>(role + 1))
        if (!m_mainLayout->isAreaAnchored(role))
            foreach (QAction* action, barForRole(role)->actions())
                if (action->isChecked())
                    action->toggle();
}

void IdealMainWidget::hideAllDocks()
{
    for (IdealMainLayout::Role role = IdealMainLayout::Left; role <= IdealMainLayout::Top; role = static_cast<IdealMainLayout::Role>(role + 1))
        foreach (QAction* action, barForRole(role)->actions())
            if (action->isChecked())
                action->toggle();
}

void IdealMainWidget::raiseView(View * view)
{
    QWidgetAction* action = views[view];
    Q_ASSERT(action);

    action->setChecked(true);
}

void IdealMainWidget::removeView(View* view)
{
    Q_ASSERT(views.contains(view));

    QWidgetAction* action = views[view];

    QDockWidget* dock = qobject_cast<QDockWidget*>(action->defaultWidget());
    Q_ASSERT(dock);

    Q_ASSERT(docks.contains(dock));

    if (IdealButtonBarWidget* bar = barForRole(roleForArea(docks[dock])))
        bar->removeAction(action);

    views.remove(view);
    actions.remove(dock);
    delete dock;
}

void IdealMainWidget::setCentralWidget(QWidget * widget)
{
    m_mainLayout->addWidget(widget, IdealMainLayout::Central);
}

void IdealMainWidget::anchorCurrentDock(bool anchor)
{
    if (QDockWidget* dw = m_mainLayout->lastDockWidget()) {
        IdealDockWidgetTitle* title = static_cast<IdealDockWidgetTitle*>(dw->titleBarWidget());
        title->setAnchored(anchor, true);
    }
}

void IdealMainWidget::anchorDockWidget(bool checked, IdealButtonBarWidget * bar)
{
    if (bar == leftBarWidget)
        m_mainLayout->anchorWidget(checked, IdealMainLayout::Left);
    else if (bar == rightBarWidget)
        m_mainLayout->anchorWidget(checked, IdealMainLayout::Right);
    else if (bar == topBarWidget)
        m_mainLayout->anchorWidget(checked, IdealMainLayout::Top);
    else if (bar == bottomBarWidget)
        m_mainLayout->anchorWidget(checked, IdealMainLayout::Bottom);
}

void IdealMainWidget::anchorDockWidget(QDockWidget * dock, bool anchor)
{
    Q_ASSERT(docks.contains(dock));

    m_mainLayout->anchorWidget(anchor, roleForArea(docks[dock]));
}

void IdealMainWidget::showDockWidget(QDockWidget * dock, bool show)
{
    Q_ASSERT(docks.contains(dock));

    IdealMainLayout::Role role = roleForArea(docks[dock]);

    static_cast<IdealDockWidgetTitle*>(dock->titleBarWidget())->setAnchored(m_mainLayout->isAreaAnchored(role), false);

    if (show)
        m_mainLayout->addWidget(dock, role);
    else
        m_mainLayout->removeWidget(role);
}

IdealCentralWidget::IdealCentralWidget(IdealMainWidget * parent)
    : QWidget(parent)
{
}

IdealCentralWidget::~ IdealCentralWidget()
{
}

void IdealCentralWidget::paintEvent(QPaintEvent * event)
{
    return;
    
    Q_UNUSED(event);
    
    QPainter p(this);
    //p.fillRect(event->rect(), Qt::red);

    if (false && layout())
        for (int i = 0; i < layout()->count(); ++i)
            p.fillRect(layout()->itemAt(i)->geometry(), Qt::red);
}

IdealMainLayout * IdealCentralWidget::idealLayout() const
{
    return static_cast<IdealMainLayout*>(layout());
}

IdealSplitterHandle::IdealSplitterHandle(Qt::Orientation orientation, QWidget* parent, IdealMainLayout::Role resizeRole)
    : QWidget(parent)
    , m_orientation(orientation)
    , m_hover(false)
    , m_resizeRole(resizeRole)
{
    setCursor(orientation == Qt::Horizontal ? Qt::SplitVCursor : Qt::SplitHCursor);
    setMouseTracking(true);
}

void IdealSplitterHandle::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event)

    QPainter p(this);
    QStyleOption so(0);
    so.rect = rect();
    so.palette = palette();
    so.state = (m_orientation == Qt::Horizontal ? QStyle::State_None : QStyle::State_Horizontal);
    if (m_hover)
        so.state |= QStyle::State_MouseOver;
    so.state |= QStyle::State_Enabled;

    style()->drawControl(QStyle::CE_Splitter, &so, &p, this);
}

void IdealSplitterHandle::mouseMoveEvent(QMouseEvent * event)
{
    bool hover = rect().contains(event->pos());
    if (m_hover != hover) {
        m_hover = hover;
        update();
    }

    if (!(event->buttons() & Qt::LeftButton))
        return;

    int thickness = convert(parentWidget()->mapFromGlobal(event->globalPos())) - m_dragStart;

    switch (m_resizeRole) {
        case IdealMainLayout::Right:
            thickness = parentWidget()->size().width() - thickness;
            break;
        case IdealMainLayout::Bottom:
            thickness = parentWidget()->size().height() - thickness;
            break;
        default:
            break;
    }

    emit resize(thickness, m_resizeRole);
}

void IdealSplitterHandle::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton)
        m_dragStart = convert(event->pos());
}

IdealMainWidget * IdealButtonBarWidget::parentWidget() const
{
    return static_cast<IdealMainWidget *>(QWidget::parentWidget());
}

IdealMainLayout * IdealMainWidget::mainLayout() const
{
    return m_mainLayout;
}

IdealCentralWidget * IdealMainWidget::internalCentralWidget() const
{
    return mainWidget;
}

void IdealMainWidget::showDock(IdealMainLayout::Role role, bool show)
{
    if (QDockWidget* widget = m_mainLayout->lastDockWidget(role))
        if (actions.contains(widget))
            return actions[widget]->setChecked(show);

    if (show && barForRole(role)->actions().count())
        barForRole(role)->actions().first()->setChecked(show);
}

void IdealMainWidget::showLeftDock(bool show)
{
    showDock(IdealMainLayout::Left, show);
}

void IdealMainWidget::showBottomDock(bool show)
{
    showDock(IdealMainLayout::Bottom, show);
}

void IdealMainWidget::showTopDock(bool show)
{
    showDock(IdealMainLayout::Top, show);
}

void IdealMainWidget::showRightDock(bool show)
{
    showDock(IdealMainLayout::Right, show);
}

QWidget * IdealMainWidget::firstWidget(IdealMainLayout::Role role) const
{
    if (IdealButtonBarWidget* button = barForRole(role))
        if (!button->actions().isEmpty())
            return static_cast<QWidgetAction*>(button->actions().first())->defaultWidget();

    return 0;
}

IdealButtonBarWidget* IdealMainWidget::barForRole(IdealMainLayout::Role role) const
{
    switch (role) {
        case IdealMainLayout::Left:
            return leftBarWidget;

        case IdealMainLayout::Top:
            return topBarWidget;

        case IdealMainLayout::Right:
            return rightBarWidget;

        case IdealMainLayout::Bottom:
            return bottomBarWidget;

        default:
            return 0;
    }
}

QWidgetAction * IdealMainWidget::actionForView(View * view) const
{
    if (views.contains(view))
        return views[view];

    return 0;
}

void IdealMainWidget::setAnchorActionStatus(bool checked)
{
    m_anchorCurrentDock->blockSignals(true);
    m_anchorCurrentDock->setChecked(checked);
    m_anchorCurrentDock->blockSignals(false);
}

#include "ideal.moc"
