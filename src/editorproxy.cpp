// Ewww... need this to access KParts::Part::setWidget(), so that kdevelop
// doesn't need to be rearchitected for multiple views before the lazy view
// creation can go in
#define protected public
#include <kparts/part.h>
#undef protected

#include <qwidget.h>
#include <qpopupmenu.h>
#include <qtimer.h>

#include <kdeversion.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kmdidefines.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/viewcursorinterface.h>
#include <ktexteditor/popupmenuinterface.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/selectioninterface.h>
#include <ktexteditor/view.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>
#include <kmainwindow.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kstdaccel.h>

#include "toplevel.h"
#include "partcontroller.h"
#include "core.h"
#include "newmainwindow.h"


#include "editorproxy.h"


using namespace KTextEditor;

EditorProxy *EditorProxy::s_instance = 0;


EditorProxy::EditorProxy()
  : QObject()
{
	KConfig *config = kapp->config();
	config->setGroup("UI");
	int mdimode = config->readNumEntry("MDIMode", KMdi::IDEAlMode);

	m_delayedViewCreationCompatibleUI = (mdimode == KMdi::IDEAlMode || mdimode == KMdi::TabPageMode);

	KAction *ac = new KAction( i18n("Show Context Menu"), 0, this,
		SLOT(showPopup()), TopLevel::getInstance()->main()->actionCollection(), "show_popup" );
    KShortcut cut ;/*= KStdAccel::shortcut(KStdAccel::PopupMenuContext);*/
    cut.append(KKey(CTRL+Key_Return));
    ac->setShortcut(cut);
}


EditorProxy *EditorProxy::getInstance()
{
  if (!s_instance)
    s_instance = new EditorProxy;

  return s_instance;
}


void EditorProxy::setLineNumber(KParts::Part *part, int lineNum, int col)
{
  if (!part || !part->inherits("KTextEditor::Document"))
    return;

  if ( lineNum < 0 )
    return;

  ViewCursorInterface *iface = dynamic_cast<ViewCursorInterface*>(part->widget());
  if (iface)
    iface->setCursorPositionReal(lineNum, col == -1 ? 0 : col);
  else {
    // Save the position for a rainy day (or when the view gets activated and wants its position)
    for (QValueList<EditorWrapper*>::ConstIterator it = m_editorParts.begin(); it != m_editorParts.end(); ++it)
      if ((*it)->document() == part) {
        (*it)->setLine(lineNum);
        (*it)->setCol(col);
        return;
      }

    // Shouldn't hit this?
    Q_ASSERT(false);
  }
}

void EditorProxy::installPopup( KParts::Part * part )
{

	if ( part->inherits("KTextEditor::Document") && part->widget())
	{
		PopupMenuInterface *iface = dynamic_cast<PopupMenuInterface*>(part->widget());
		if (iface)
		{
			KTextEditor::View * view = static_cast<KTextEditor::View*>( part->widget() );

			QPopupMenu * popup = static_cast<QPopupMenu*>( part->factory()->container("ktexteditor_popup", view ) );

			if (!popup)
			{
				kdWarning() << k_funcinfo << "Popup not found!" << endl;
				return;
			}

			KAction * action = NULL;
			//If there is a tab for this file, we don't need to plug the closing menu entries here
			switch (dynamic_cast<NewMainWindow*>(TopLevel::getInstance())->getTabWidgetVisibility())
			{
			case KMdi::AlwaysShowTabs:
				break;
			case KMdi::ShowWhenMoreThanOneTab:
				if(PartController::getInstance()->parts()->count() > 1)
					break;
			case KMdi::NeverShowTabs:
				// I'm not sure if this is papering over a bug in xmlgui or not, but this test is
				// needed in order to avoid multiple close actions in the popup menu in some cases
				action = TopLevel::getInstance()->main()->actionCollection()->action( "file_close" );
				if ( action && !action->isPlugged( popup ) )
				{
					popup->insertSeparator( 0 );
					action->plug( popup, 0 );
				}
				action = TopLevel::getInstance()->main()->actionCollection()->action( "file_closeother" );
				if ( action && !action->isPlugged( popup ) )
					action->plug( popup, 1 );
				break;
			default:
				break;
			}


			iface->installPopup( popup );

			connect(popup, SIGNAL(aboutToShow()), this, SLOT(popupAboutToShow()));

			// ugly hack: mark the "original" items
			m_popupIds.resize(popup->count());
			for (uint index=0; index < popup->count(); ++index)
				m_popupIds[index] = popup->idAt(index);
		}
	}
}

void EditorProxy::popupAboutToShow()
{
  QPopupMenu *popup = (QPopupMenu*)sender();
  if (!popup)
    return;

  // ugly hack: remove all but the "original" items
  for (int index=popup->count()-1; index >= 0; --index)
  {
    int id = popup->idAt(index);
    if (m_popupIds.contains(id) == 0)
    {
      QMenuItem *item = popup->findItem(id);
      if (item->popup())
	delete item->popup();
      else
          popup->removeItemAt(index);
//      kdDebug(9000) << "removed id " << id << " at index " << index << endl;
    } else {
//        kdDebug(9000) << "leaving id " << id << endl;
    }
  }
/*	// why twice !?!?
  // ugly hack: mark the "original" items
  m_popupIds.resize(popup->count());
  for (uint index=0; index < popup->count(); ++index)
    m_popupIds[index] = popup->idAt(index);
*/

  KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(PartController::getInstance()->activePart());
  if (!ro_part)
    return;
/*	// I disagree.. the EditorContext shouldn't emit the filecontext event
  // fill the menu in the file context
  FileContext context(ro_part->url().path(), false);
  Core::getInstance()->fillContextMenu(popup, &context);
*/
  // fill the menu in the editor context
  if (!ro_part->widget())
    return;

  SelectionInterface *selectIface = dynamic_cast<SelectionInterface*>(ro_part);
  ViewCursorInterface *cursorIface = dynamic_cast<ViewCursorInterface*>(ro_part->widget());
  EditInterface *editIface = dynamic_cast<EditInterface*>(ro_part);

  QString wordstr, linestr;
  bool hasMultilineSelection = false;
  if( selectIface && selectIface->hasSelection() )
  {
    hasMultilineSelection = ( selectIface->selection().contains('\n') != 0 );
    if ( !hasMultilineSelection )
    {
      wordstr = selectIface->selection();
    }
  }
  if( cursorIface && editIface )
  {
    uint line, col;
    line = col = 0;
    cursorIface->cursorPositionReal(&line, &col);
    linestr = editIface->textLine(line);
    if( wordstr.isEmpty() && !hasMultilineSelection ) {
      int startPos = QMAX(QMIN((int)col, (int)linestr.length()-1), 0);
      int endPos = startPos;
      while (startPos >= 0 && ( linestr[startPos].isLetterOrNumber() || linestr[startPos] == '_' ) )
          startPos--;
      while (endPos < (int)linestr.length() && ( linestr[endPos].isLetterOrNumber() || linestr[endPos] == '_' ) )
          endPos++;
      wordstr = (startPos==endPos)?
          QString() : linestr.mid(startPos+1, endPos-startPos-1);
    }
    kdDebug(9000) << "Word:" << wordstr << ":" << endl;
    EditorContext context(ro_part->url(), line, col, linestr, wordstr);
    Core::getInstance()->fillContextMenu(popup, &context);
  } else {
    Core::getInstance()->fillContextMenu(popup, 0);
  }

  // Remove redundant separators (any that are first, last, or doubled)
  bool lastWasSeparator = true;
  for( uint i = 0; i < popup->count(); ) {
    int id = popup->idAt( i );
    if( lastWasSeparator && popup->findItem( id )->isSeparator() ) {
      popup->removeItem( id );
      // Since we removed an item, don't increment i
    } else {
      lastWasSeparator = false;
      i++;
    }
  }
  if( lastWasSeparator && popup->count() > 0 )
    popup->removeItem( popup->idAt( popup->count() - 1 ) );
}

void EditorProxy::showPopup( )
{
	kdDebug(9000) << k_funcinfo << endl;

	if ( KParts::Part * part = PartController::getInstance()->activePart() )
	{
		ViewCursorInterface *iface = dynamic_cast<ViewCursorInterface*>( part->widget() );
		if ( iface )
		{
			KTextEditor::View * view = static_cast<KTextEditor::View*>( part->widget() );
			QPopupMenu * popup = static_cast<QPopupMenu*>( view->factory()->container("ktexteditor_popup", view ) );

			popup->exec( view->mapToGlobal( iface->cursorCoordinates() ) );
		}
	}

}

void EditorProxy::registerEditor(EditorWrapper* wrapper)
{
  m_editorParts.append(wrapper);
}

void EditorProxy::deregisterEditor(EditorWrapper* wrapper)
{
  m_editorParts.remove(wrapper);
}

EditorWrapper::EditorWrapper(KTextEditor::Document* editor, bool activate, QWidget* parent, const char* name)
  : QWidgetStack(parent, name)
  , m_doc(editor)
  , m_view(0L)
  , m_line(0)
  , m_col(0)
  , m_first(!activate && EditorProxy::getInstance()->isDelayedViewCapable())
{
  EditorProxy::getInstance()->registerEditor(this);
}

EditorWrapper::~EditorWrapper()
{
  kdDebug() << k_funcinfo << this << endl;
  EditorProxy::getInstance()->deregisterEditor(this);
}

KTextEditor::Document* EditorWrapper::document() const
{
  return m_doc;
}

void EditorWrapper::setLine(int line)
{
  m_line = line;
}

void EditorWrapper::setCol(int col)
{
  m_col = col;
}

void EditorWrapper::show()
{
  if ( !m_doc ) {
    QWidgetStack::show();
    return;
  }

  if (m_first) {
    m_first = false;
    QWidgetStack::show();
    return;
  }

  if (m_doc->widget()) {
    QWidgetStack::show();
    return;
  }

  m_view = m_doc->createView(this);

  addWidget(m_view);

  m_doc->setWidget(m_view);
  // FIXME assumption check
  // We're managing the view deletion by being its parent, don't let the part self-destruct
  disconnect( m_view, SIGNAL( destroyed() ), m_doc, SLOT( slotWidgetDestroyed() ) );
  //connect( m_view, SIGNAL( destroyed() ), this, SLOT( deleteLater() ) );

  m_doc->insertChildClient(m_view);

  PartController::getInstance()->integrateTextEditorPart(m_doc);

  ViewCursorInterface *iface = dynamic_cast<ViewCursorInterface*>(static_cast<KTextEditor::View*>(m_view));
  if (iface) {
    iface->setCursorPositionReal(m_line, m_col == -1 ? 0 : m_col);

  } else {
    // Shouldn't get here
    Q_ASSERT(false);
  }

  QWidgetStack::show();
}

QWidget * EditorProxy::widgetForPart( KParts::Part * part )
{
	if ( !part ) return 0;

	if (part->widget())
		return part->widget();

	for (QValueList<EditorWrapper*>::ConstIterator it = m_editorParts.begin(); it != m_editorParts.end(); ++it)
		if ((*it)->document() == part)
			return *it;

	return 0L;
}

QWidget * EditorProxy::topWidgetForPart( KParts::Part * part )
{
	if ( !part ) return 0;

	for (QValueList<EditorWrapper*>::ConstIterator it = m_editorParts.begin(); it != m_editorParts.end(); ++it)
		if ((*it)->document() == part)
			return *it;

	if (part->widget())
		return part->widget();

	return 0L;
}

bool EditorProxy::isDelayedViewCapable( )
{
	return m_delayedViewCreationCompatibleUI;
}

#include "editorproxy.moc"
