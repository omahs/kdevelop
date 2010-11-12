/***************************************************************************
 *   Copyright 1999-2001 Bernd Gehrmann and the KDevelop Team              *
 *   bernd@kdevelop.org                                                    *
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "grepoutputmodel.h"
#include "grepviewplugin.h"
#include <QModelIndex>
#include <QTextDocument>
#include <ktexteditor/cursor.h>
#include <ktexteditor/document.h>
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <klocale.h>

using namespace KDevelop;

GrepOutputItem::GrepOutputItem(DocumentChangePointer change, const QString &text, bool replace)
    : QStandardItem(), m_change(change)
{
    setText(text);
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    setCheckState(Qt::Checked);
    if(replace)
    {
        QString replacement = Qt::escape(text.left(change->m_range.start.column)) + // start of line
                            "<b>" + Qt::escape(change->m_newText) + "</b>" +        // replaced part
                            Qt::escape(text.right(text.length() - change->m_range.end.column)); // rest of line
        setToolTip(replacement.trimmed());
    }
}

GrepOutputItem::GrepOutputItem(const QString& filename, const QString& text)
    : QStandardItem(), m_change(new DocumentChange(IndexedString(filename), SimpleRange::invalid(), QString(), QString()))
{
    setText(text);
    setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsTristate);
    setCheckState(Qt::Checked);
}

int GrepOutputItem::lineNumber() const 
{
    // line starts at 0 for cursor but we want to start at 1
    return m_change->m_range.start.line + 1;
}

QString GrepOutputItem::filename() const 
{
    return m_change->m_document.str();
}

DocumentChangePointer GrepOutputItem::change() const
{
    return m_change;
}

bool GrepOutputItem::isText() const
{
    return m_change->m_range.isValid();
}

GrepOutputItem::~GrepOutputItem()
{}

///////////////////////////////////////////////////////////////

GrepOutputModel::GrepOutputModel( QObject *parent )
    : QStandardItemModel( parent ), m_regExp("")
{
    connect(this, SIGNAL(itemChanged(QStandardItem*)),
              this, SLOT(updateCheckState(QStandardItem*)));
}

GrepOutputModel::~GrepOutputModel()
{}

void GrepOutputModel::setRegExp(const QRegExp& re)
{
    m_regExp = re;
}

void GrepOutputModel::activate( const QModelIndex &idx )
{
    QStandardItem *stditem = itemFromIndex(idx);
    GrepOutputItem *grepitem = dynamic_cast<GrepOutputItem*>(stditem);
    if( !grepitem || !grepitem->isText() )
        return;

    KUrl url(grepitem->filename());

    int line = grepitem->lineNumber() - 1;
    KTextEditor::Range range( line, 0, line+1, 0);

    // Try to find the actual text range we found during the grep
    IDocument* doc = ICore::self()->documentController()->documentForUrl( url );
    if(!doc)
        doc = ICore::self()->documentController()->openDocument( url, range );
    if(!doc)
        return;
    if (KTextEditor::Document* tdoc = doc->textDocument()) {
        QString text = tdoc->line(line);
        int index = m_regExp.indexIn(text);
        if (index!=-1) {
            range.setBothLines(line);
            range.start().setColumn(index);
            range.end().setColumn(index+m_regExp.matchedLength());
            doc->setTextSelection( range );
        }
    }

    ICore::self()->documentController()->activateDocument( doc, range );
}

bool GrepOutputModel::isValidIndex( const QModelIndex& idx ) const
{
    return ( idx.isValid() && idx.row() >= 0 && idx.row() < rowCount() && idx.column() == 0 );
}

QModelIndex GrepOutputModel::nextHighlightIndex( const QModelIndex &currentIdx )
{
    int startrow = isValidIndex(currentIdx) ? currentIdx.row() + 1 : 0;
    
    for (int row = 0; row < rowCount(); ++row) {
        int currow = (startrow + row) % rowCount();
        if (GrepOutputItem* grep_item = dynamic_cast<GrepOutputItem*>(item(currow)))
            if (grep_item->isText())
                return index(currow, 0);
    }
    return QModelIndex();
}

QModelIndex GrepOutputModel::previousHighlightIndex( const QModelIndex &currentIdx )
{
    //We have to ensure that startrow is >= rowCount - 1 to get a positive value from the % operation.
    int startrow = rowCount() + (isValidIndex(currentIdx) ? currentIdx.row() : rowCount()) - 1;
    
    for (int row = 0; row < rowCount(); ++row)
    {
        int currow = (startrow - row) % rowCount();
        if (GrepOutputItem* grep_item = dynamic_cast<GrepOutputItem*>(item(currow)))
            if (grep_item->isText())
                return index(currow, 0);
    }
    return QModelIndex();
}

void GrepOutputModel::appendOutputs( const QString &filename, const GrepOutputItem::List &items )
{
    QString fnString = i18np("%2 (one match)", "%2 (%1 matches)", items.length(), filename);

    GrepOutputItem *fileItem = new GrepOutputItem(filename, fnString);
    appendRow(fileItem);
    //m_tracker.addUrl(KUrl(filename));
    foreach( const GrepOutputItem& item, items )
    {
        fileItem->appendRow(new GrepOutputItem(item));
    }
}

void GrepOutputModel::updateCheckState(QStandardItem* item)
{
    // if we don't disconnect the SIGNAL, the setCheckState will call it in loop
    disconnect(SIGNAL(itemChanged(QStandardItem*)));

    if(item->parent() == 0)
    {
        int idx = item->rowCount() - 1;

        if(item->checkState() == Qt::Unchecked)
        {
            while(idx >= 0)
            {
                item->child(idx)->setCheckState(Qt::Unchecked);
                idx--;
            }
        } else if(item->checkState() == Qt::Checked)
        {
            while(idx >= 0)
            {
                item->child(idx)->setCheckState(Qt::Checked);
                idx--;
            }
        }
    }
    else
    {
        QStandardItem *parent = item->parent();
        bool checked = false;
        bool unchecked = false;
        int idx = parent->rowCount() - 1;
        while(idx >= 0)
        {
            if(parent->child(idx)->checkState() == Qt::Checked) checked = true;
            else unchecked = true;

            idx--;
        }

        if(checked) unchecked ? parent->setCheckState(Qt::PartiallyChecked) : parent->setCheckState(Qt::Checked);
        else parent->setCheckState(Qt::Unchecked);
    }

    connect(this, SIGNAL(itemChanged(QStandardItem*)),
              this, SLOT(updateCheckState(QStandardItem*)));
}

#include "grepoutputmodel.moc"

