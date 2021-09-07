/*
 * Copyright <year> Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KDEVPLATFORM_PLUGIN_TEST_QUICKOPEN_H
#define KDEVPLATFORM_PLUGIN_TEST_QUICKOPEN_H

#include "quickopentestbase.h"

class TestQuickOpen
    : public QuickOpenTestBase
{
    Q_OBJECT
public:
    explicit TestQuickOpen(QObject* parent = nullptr);
private Q_SLOTS:
    void testProjectFileSwap();
    void testSorting();
    void testSorting_data();
    void testStableSort();
    void testAbbreviations();
    void testAbbreviations_data();
    void testDuchainFilter();
    void testDuchainFilter_data();

    void testProjectFileFilter();
};

#endif // KDEVPLATFORM_PLUGIN_TEST_QUICKOPEN_H
