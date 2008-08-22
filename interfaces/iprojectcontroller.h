/* This file is part of KDevelop
Copyright 2006 Adam Treat <treat@kde.org>
Copyright 2007 Anreas Pakulat <apaku@gmx.de>

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
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#ifndef IPROJECTCONTROLLER_H
#define IPROJECTCONTROLLER_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <kurl.h>
#include "interfacesexport.h"


namespace KDevelop
{

class IProject;
class ProjectModel;
class ProjectBaseItem;

class KDEVPLATFORMINTERFACES_EXPORT IProjectController : public QObject
{
    Q_OBJECT
public:
    IProjectController( QObject *parent = 0 );
    virtual ~IProjectController();

    Q_SCRIPTABLE virtual IProject* projectAt( int ) const = 0;
    Q_SCRIPTABLE virtual int projectCount() const = 0;
    Q_SCRIPTABLE virtual QList<IProject*> projects() const = 0;

    /**
     * Provides access to the model representing the open projects
     * @returns the model containing the projects and their items
     */
    Q_SCRIPTABLE virtual ProjectModel* projectModel() = 0;

    /**
     * Find an open project using the name of the project
     * @param name the name of the project to be found
     * @returns the project or null if no project with that name is open
     */
    Q_SCRIPTABLE virtual IProject* findProjectByName( const QString& name ) = 0;

    /**
     * Finding an open project for a given file
     * @param url the url of a file belonging to an open project
     * @returns the first open project containing the url or null if no such
     * project can be found
     */
    Q_SCRIPTABLE virtual IProject* findProjectForUrl( const KUrl& url ) const = 0;

public Q_SLOTS:
    /**
     * open the project from the given kdev4 project file. This only reads
     * the file and starts creating the project model from it. The opening process
     * is finished once @ref projectOpened signal is emitted.
     * @param url a kdev4 project file top open
     * @returns true if the given project could be opened, false otherwise
     */
    virtual bool openProject( const KUrl & url = KUrl() ) = 0;
    /**
     * close the given project. Closing the project is done in steps and
     * the @ref projectClosing and @ref projectClosed signals are emitted. Only when
     * the latter signal is emitted its guaranteed that the project has been closed.
     * The @ref IProject object will be deleted after the closing has finished.
     * @returns true if the project could be closed, false otherwise
     */
    virtual bool closeProject( IProject* ) = 0;
    virtual bool configureProject( IProject* ) = 0;
//     virtual void changeCurrentProject( KDevelop::ProjectBaseItem* ) = 0;

Q_SIGNALS:
    /**
     * emitted after a project is completely opened and the project model
     * has been populated.
     * @param project the project that has been opened.
     */
    void projectOpened( KDevelop::IProject* project );
    /**
     * emitted when starting to close a project.
     * @param project the project that is going to be closed.
     */
    void projectClosing( KDevelop::IProject* project );
    /**
     * emitted when a project has been closed completely. At this
     * point the project object is still valid, the deletion will be done
     * delayed during the next run of the event loop.
     * @param project the project that has been closed.
     */
    void projectClosed( KDevelop::IProject* project );
};

}
#endif

