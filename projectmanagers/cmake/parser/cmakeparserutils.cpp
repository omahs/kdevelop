/* KDevelop CMake Support
 *
 * Copyright 2008 Matt Rogers <mattr@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "cmakeparserutils.h"

#include <QStringList>
#include <qfileinfo.h>
#include <qdir.h>
#include <kdebug.h>

namespace CMakeParserUtils
{
    QList<int> parseVersion(const QString& version, bool* ok)
    {
        QList<int> versionNumList;
        *ok = false;
        QStringList versionStringList = version.split('.', QString::SkipEmptyParts);
        if (versionStringList.isEmpty())
            return versionNumList;

        foreach(const QString& part, versionStringList)
        {
            int i = part.toInt(ok);
            if (!*ok)
            {
                versionNumList.clear();
                return versionNumList;
            }
            else
                versionNumList.append(i);
        }
        return versionNumList;
    }

    QString valueFromSystemInfo(const QString& variable, const QString& systeminfo)
    {
        int idx = systeminfo.indexOf( variable );
        if( idx != -1 ) {
            // 2 == ' "'
            idx += variable.count() + 2;
            int end = systeminfo.indexOf( "\"", idx );
            if( end != -1 ) {
                return systeminfo.mid( idx, end-idx );
            }
        }
        return QString();
    }

    
}
    
