/* This file is part of the KDE project
   Copyright (C) 2002 Alexander Dymo <cloudtemple@mksat.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include <qpixmap.h>

#include "plinestyle.h"
#include "propertyeditor.h"

PLineStyle::PLineStyle(const PropertyEditor *editor, const QString pname, const QVariant value, QWidget *parent, const char *name):
    QComboBox(parent, name)
{
    const char *nopen[]={
    "48 16 1 1",
    ". c None",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................"};
    insertItem(QPixmap(nopen));

    const char *solid[]={
    "48 16 2 1",
    ". c None",
    "# c #000000",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    ".###########################################....",
    ".###########################################....",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................"};
    insertItem(QPixmap(solid));

    const char *dash[]={
    "48 16 2 1",
    ". c None",
    "# c #000000",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    ".#########..#########..#########..##########....",
    ".#########..#########..#########..##########....",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................"};
    insertItem(QPixmap(dash));

    const char *dashdot[]={
    "48 16 2 1",
    ". c None",
    "# c #000000",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    ".#########..##..#########..##..#########..##....",
    ".#########..##..#########..##..#########..##....",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................"};
    insertItem(QPixmap(dashdot));

    const char *dashdotdot[]={
    "48 16 2 1",
    ". c None",
    "# c #000000",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    ".#########..##..##..#########..##..##..#####....",
    ".#########..##..##..#########..##..##..#####....",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................",
    "................................................"};
    insertItem(QPixmap(dashdotdot));

    setValue(value, false);
    setPName(pname);
    connect(this, SIGNAL(activated(int)), this, SLOT(updateProperty(int)));
    connect(this, SIGNAL(propertyChanged(QString, QVariant)), editor, SLOT(emitPropertyChange(QString, QVariant)));
}

QVariant PLineStyle::value() const
{
    return QVariant(currentItem());
}

void PLineStyle::setValue(const QVariant value, bool emitChange)
{
    if ( (value.toInt() >= 0) && (value.toInt() <= 5) )
    {
        setCurrentItem(value.toInt());
        if (emitChange)
            emit propertyChanged(pname(), value);
    };
}

void PLineStyle::updateProperty(int /*val*/)
{
    emit propertyChanged(pname(), value());
}

#ifndef PURE_QT
#include "plinestyle.moc"
#endif
