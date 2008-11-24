/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "simplerefactoring.h"
#include <language/interfaces/codecontext.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <qaction.h>
#include <klocalizedstring.h>
#include <kicon.h>
#include <kmessagebox.h>
#include <qdialog.h>
#include <qboxlayout.h>
#include <language/duchain/navigation/useswidget.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <QTabWidget>
#include <language/codegen/documentchangeset.h>
#include <qapplication.h>
#include <qdatetime.h>
#include "progressdialogs.h"
#include <language/duchain/navigation/abstractnavigationwidget.h>
#include <language/duchain/navigation/abstractnavigationwidget.h>
#include <limits>
#include <language/duchain/use.h>
#include <language/duchain/classmemberdeclaration.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <project/projectmodel.h>
#include "cppnewclass.h"

using namespace KDevelop;

// #define WARN_BEFORE_REFACTORING

// static Identifier destructorForName(Identifier name) {
//   QString str = name.identifier().str();
//   if(str.startsWith("~"))
//     return Identifier(str);
//   return Identifier("~"+str);
// }

bool doRefactoringWarning() {
#ifndef WARN_BEFORE_REFACTORING
  return true;
#else
   return KMessageBox::Continue == KMessageBox::warningContinueCancel(0, i18n("Refactoring is an experimental feature, it may damage your code. Before using it, make sure to make a backup."));
#endif
}

SimpleRefactoring& SimpleRefactoring::self() {
  static SimpleRefactoring ret;
  return ret;
}

void SimpleRefactoring::doContextMenu(KDevelop::ContextMenuExtension& extension, KDevelop::Context* context) {

  if(DeclarationContext* declContext = dynamic_cast<DeclarationContext*>(context)){
    //Actions on declarations
    qRegisterMetaType<KDevelop::IndexedDeclaration>("KDevelop::IndexedDeclaration");

    DUChainReadLocker lock(DUChain::lock());

    Declaration* declaration = declContext->declaration().data();

    if(declaration) {
      QAction* action = new QAction(i18n("Rename %1", declaration->qualifiedIdentifier().toString()), this);
      action->setData(QVariant::fromValue(IndexedDeclaration(declaration)));
      action->setIcon(KIcon("edit-rename"));
      connect(action, SIGNAL(triggered(bool)), this, SLOT(executeRenameAction()));

      extension.addAction(ContextMenuExtension::RefactorGroup, action);
    }
  }
  if(ProjectItemContext* projectContext = dynamic_cast<ProjectItemContext*>(context)) {
    //Actions on project-items
    foreach(KDevelop::ProjectBaseItem* item, projectContext->items()) {
      if(item->folder()) {
        //Allow creating a class in the folder
        QAction* action = new QAction(i18n("Create Class"), this);
        action->setData(QVariant::fromValue(item->folder()->url()));
//         action->setIcon(KIcon("edit-rename"));
        connect(action, SIGNAL(triggered(bool)), this, SLOT(executeNewClassAction()));

        extension.addAction(ContextMenuExtension::FileGroup, action);
      }
    }
  }
}

void SimpleRefactoring::executeNewClassAction() {
  QAction* action = qobject_cast<QAction*>(sender());
  if(action) {
    KUrl url = action->data().value<KUrl>();
    if(url.isValid()) {
      CppNewClass newClassWizard(qApp->activeWindow(), url);
      newClassWizard.exec();
    }
  }else{
    kWarning() << "strange problem";
  }
}

void SimpleRefactoring::executeRenameAction() {
  QAction* action = qobject_cast<QAction*>(sender());
  if(action) {
    IndexedDeclaration decl = action->data().value<IndexedDeclaration>();
    startInteractiveRename(decl);
  }else{
    kWarning() << "strange problem";
  }
}

class SimpleRefactoringCollector : public KDevelop::UsesWidget::UsesWidgetCollector {
  public:
  SimpleRefactoringCollector(IndexedDeclaration decl) : UsesWidgetCollector(decl) {
  }

  virtual void processUses(KDevelop::ReferencedTopDUContext topContext) {
    m_allUsingContexts << IndexedTopDUContext(topContext.data());
    UsesWidgetCollector::processUses(topContext);
  }

  QVector<IndexedTopDUContext> m_allUsingContexts;
};

DocumentChangeSet::ChangeResult applyChangesToDeclarations(QString oldName, QString newName, DocumentChangeSet& changes, QList<IndexedDeclaration> declarations) {
  foreach(IndexedDeclaration decl, declarations) {
    if(!decl.data())
      continue;
    TopDUContext* top = decl.data()->topContext();
    if(decl.data()->range().isEmpty()) {
      kDebug() << "found empty declaration";
    }
    if(ClassFunctionDeclaration* classFun = dynamic_cast<ClassFunctionDeclaration*>(decl.data())) {
      if(classFun->isDestructor()) {
        //Our C++, destructors only have the range of the text behind the "~"
//         newName = destructorForName(Identifier(newName)).identifier().str();
//         oldName = destructorForName(Identifier(oldName)).identifier().str();
      }
    }
    DocumentChangeSet::ChangeResult result = changes.addChange(DocumentChangePointer(new DocumentChange(top->url(), decl.data()->range(), oldName, newName)));
    if(!result)
      return result;
  }
  return DocumentChangeSet::ChangeResult(true);
}

DocumentChangeSet::ChangeResult applyChanges(QString oldName, QString newName, DocumentChangeSet& changes, DUContext* context, int usedDeclarationIndex) {
   if(usedDeclarationIndex == std::numeric_limits<int>::max())
     return DocumentChangeSet::ChangeResult(true);

   for(int a = 0; a < context->usesCount(); ++a) {
     const Use& use(context->uses()[a]);
     if(use.m_declarationIndex != usedDeclarationIndex)
       continue;
     if(use.m_range.isEmpty()) {
       kDebug() << "found empty use";
       continue;
     }
     DocumentChangeSet::ChangeResult result = changes.addChange(DocumentChangePointer(new DocumentChange(context->url(), use.m_range, oldName, newName)));
     if(!result)
       return result;
   }

   foreach(DUContext* child, context->childContexts()) {
     DocumentChangeSet::ChangeResult result = applyChanges(oldName, newName, changes, child, usedDeclarationIndex);
     if(!result)
       return result;
   }
   return DocumentChangeSet::ChangeResult(true);
}

void SimpleRefactoring::startInteractiveRename(KDevelop::IndexedDeclaration decl) {
  if(!doRefactoringWarning())
    return;

  DUChainReadLocker lock(DUChain::lock());

  Declaration* declaration = decl.data();

  if(!declaration)
    return;

  ///Step 1: Allow the user to specify a replacement name, and allow him to see all uses

  QString originalName = declaration->identifier().identifier().str();
  QString replacementName;

  //Since we don't yet know what the text should be replaced with, we just collect the top-contexts to process
  SimpleRefactoringCollector* collector = new SimpleRefactoringCollector(decl);

  QDialog dialog;

  QTabWidget tabWidget;

  UsesWidget uses(decl, collector);

  QWidget* navigationWidget = declaration->context()->createNavigationWidget(declaration);
  AbstractNavigationWidget* abstractNavigationWidget = dynamic_cast<AbstractNavigationWidget*>(navigationWidget);

  if(abstractNavigationWidget) { //So the context-links work
    connect(&uses, SIGNAL(navigateDeclaration(IndexedDeclaration)), abstractNavigationWidget, SLOT(navigateDeclaration(IndexedDeclaration)));
//     connect(uses, SIGNAL(navigateDeclaration(IndexedDeclaration)), tabWidget, SLOT(setCurrentIndex(...)));
///@todo Switch the tab in the tab-widget, so the user will notice that the declaration is being shown
  }

  QVBoxLayout verticalLayout;
  QHBoxLayout actionsLayout;
  dialog.setLayout(&verticalLayout);
  dialog.setWindowTitle(i18n("Rename %1", declaration->toString()));

  QLabel newNameLabel(i18n("New name:"));
  actionsLayout.addWidget(&newNameLabel);

  QLineEdit edit(declaration->identifier().identifier().str());
  newNameLabel.setBuddy(&edit);

  actionsLayout.addWidget(&edit);
  edit.setText(originalName);
  edit.setFocus();
  edit.selectAll();
  QPushButton goButton(i18n("Rename"));
  goButton.setToolTip(i18n("Note: All overloaded functions, overloads, forward-declarations, etc. will be renamed too"));
  actionsLayout.addWidget(&goButton);
  connect(&goButton, SIGNAL(clicked(bool)), &dialog, SLOT(accept()));

  QPushButton cancelButton(i18n("Cancel"));
  actionsLayout.addWidget(&cancelButton);
  verticalLayout.addLayout(&actionsLayout);

  tabWidget.addTab(&uses, i18n("Uses"));
  if(navigationWidget)
    tabWidget.addTab(navigationWidget, "Declaration Info");

  verticalLayout.addWidget(&tabWidget);

  connect(&cancelButton, SIGNAL(clicked(bool)), &dialog, SLOT(reject()));

  lock.unlock();
  dialog.resize( 750, 550 );

  if(dialog.exec() != QDialog::Accepted) {
    kDebug() << "stopped";
    return;
  }
  //It would be nicer to scope this, but then "uses" would not survive

  replacementName = edit.text();


  if(replacementName == originalName || replacementName.isEmpty())
    return;

  //Now just start doing the actual changes, no matter whether the collector is ready or not
  CollectorProgressDialog collectorProgress(i18n("Searching uses of \"%1\"", originalName), *collector);
  if(!collector->isReady()) {
    collectorProgress.exec();
    if(collectorProgress.result() != QDialog::Accepted) {
      kDebug() << "searching aborted";
      return;
    }
  }

  DocumentChangeSet changes;
  lock.lock();
  foreach(KDevelop::IndexedTopDUContext collected, collector->m_allUsingContexts) {
    QSet<int> hadIndices;
    foreach(IndexedDeclaration decl, collector->declarations()) {
      uint usedDeclarationIndex = collected.data()->indexForUsedDeclaration(collector->declaration().data(), false);
      if(hadIndices.contains(usedDeclarationIndex))
        continue;
      hadIndices.insert(usedDeclarationIndex);
      DocumentChangeSet::ChangeResult result = applyChanges(originalName, replacementName, changes, collected.data(), usedDeclarationIndex);
      if(!result) {
        KMessageBox::error(0, i18n("Applying changes failed: %1").arg(result.m_failureReason));
        return;
      }
    }
  }

  DocumentChangeSet::ChangeResult result = applyChangesToDeclarations(originalName, replacementName, changes, collector->declarations());
  if(!result) {
    KMessageBox::error(0, i18n("Applying changes failed: %1").arg(result.m_failureReason));
    return;
  }

  result = changes.applyAllChanges();
  if(!result) {
      KMessageBox::error(0, i18n("Applying changes failed: %1").arg(result.m_failureReason));
      return;
  }
}
