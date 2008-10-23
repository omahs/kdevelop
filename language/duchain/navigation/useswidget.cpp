/*
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "useswidget.h"
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/uses.h>
#include <QVBoxLayout>
#include <QLabel>
#include <QCompleter>
#include <KComboBox>
#include <qpushbutton.h>
#include <limits>
#include <klocalizedstring.h>
#include <qabstractitemview.h>

using namespace KDevelop;

TopContextUsesWidget::TopContextUsesWidget(IndexedDeclaration declaration, IndexedTopDUContext topContext) : m_topContext(topContext), m_declaration(declaration) {
    DUChainReadLocker lock(DUChain::lock());
    QLabel* label = new QLabel;
    QHBoxLayout* layout = new QHBoxLayout;
    label->setText(KUrl(topContext.url().str()).fileName());
    layout->addWidget(label);
    setLayout(layout);
}

UsesWidget::UsesWidget(IndexedDeclaration declaration) : m_declaration(declaration), m_showingUses(false), m_usingFiles(0) {
    DUChainReadLocker lock(DUChain::lock());
    if(Declaration* decl = declaration.data()) {

        QStringList files;
        
        QList<IndexedTopDUContext> contexts = allUsingContexts();
        
        FOREACH_ARRAY(IndexedTopDUContext context, contexts)
            files << context.url().str();
        
        if(files.isEmpty()) {
            //Hide
            setMinimumHeight(0);
            setMaximumHeight(0);
            hide();
            kDebug() << "hiding";
        }else{
            QLayout* layout = new QHBoxLayout;
//             QLabel* label = new QLabel(i18n("Used in:"));
//             layout->addWidget(label);

//             m_usingFiles = new QComboBox(this);
//             m_usingFiles->setFocusPolicy(Qt::NoFocus);
//             m_usingFiles->view()->setFocusPolicy(Qt::NoFocus);
//             layout->addWidget(m_usingFiles);
            layout->setAlignment(Qt::AlignLeft);
            
            layout->setMargin(0);
            
            
            
            foreach(QString file, files) {
//               KUrl u(file);
//               m_usingFiles->addItem(u.fileName());
/*//                 if(file.isEmpty()) {
//                     kDebug() << "got empty file-name in list";
//                     continue;
//                 }
                QLabel* fileLabel = new QLabel(this);
                KUrl u(file);
                fileLabel->setText(QString("<a href=\"%1\">%2</a>").arg(u.url()).arg(u.fileName()));
                
                layout->addWidget(fileLabel);
                layout->setAlignment(Qt::AlignLeft);
                connect(fileLabel, SIGNAL(linkActivated(const QString&)),
                        this, SLOT(showUsesForButton()));*/
            }
            setLayout(layout);
        }
    }
}

QList<IndexedTopDUContext> UsesWidget::allUsingContexts() {
    Declaration* decl = m_declaration.data();
    QList<IndexedTopDUContext> ret;
    KDevVarLengthArray<IndexedTopDUContext> uses = DUChain::uses()->uses(decl->id());
    FOREACH_ARRAY(IndexedTopDUContext context, uses)
    ret << context;
    
    if(decl && decl->topContext()->indexForUsedDeclaration(decl, false) != std::numeric_limits<int>::max())
        if(ret.indexOf(decl->topContext()) == -1)
            ret.push_front(decl->topContext());

    return ret;
}

void UsesWidget::showUsesForButton() {
    m_showingUses = !m_showingUses;
    emit showingUses(m_showingUses);
    QLabel* button = qobject_cast<QLabel*>(sender());

    if(button) {
        int num = layout()->indexOf(button);
        Q_ASSERT(num != -1);
        DUChainReadLocker lock(DUChain::lock());
        QList<IndexedTopDUContext> usingContexts = allUsingContexts();

        if(usingContexts.size() > (num - 1)) {
            //Show all the uses of the declaration within this context
            usingContexts.value(num - 1).data();
        }else{
            kWarning() << "Wrong count of using contexts";
        }
    }
}

#include "useswidget.moc"
