/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
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

#include "qmakeimporter.h"

#include <QList>
#include <QVector>
// #include <QDomDocument>

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtDesigner/QExtensionFactory>

#include <kurl.h>
#include <kio/job.h>

#include "icore.h"
#include "iproject.h"
#include "kgenericfactory.h"
#include "projectmodel.h"

#include "qmakemodelitems.h"
#include "qmakeprojectscope.h"

typedef KGenericFactory<QMakeImporter> QMakeSupportFactory ;
K_EXPORT_COMPONENT_FACTORY( kdevqmakeimporter,
                            QMakeSupportFactory( "kdevqmakeimporter" ) )

KDEV_ADD_EXTENSION_FACTORY_NS( Koncrete, IBuildManager, QMakeImporter )
KDEV_ADD_EXTENSION_FACTORY_NS( Koncrete, IFileManager, QMakeImporter )

QMakeImporter::QMakeImporter( QObject* parent,
                              const QStringList& )
        : Koncrete::IPlugin( QMakeSupportFactory::componentData(), parent ), Koncrete::IBuildManager(), m_rootItem( 0L )
{
    m_project = 0;

    /*    QMakeSettings* settings = QMakeSettings::self();

        //what do the settings say about our generator?
        QString generator = settings->generator();
        if ( generator.contains( "Unix" ) ) //use make
            m_builder = new Koncrete::MakeBuilder()*/
}

QMakeImporter::~QMakeImporter()
{
    delete m_rootItem;
    m_rootItem = 0;
}

Koncrete::IProject* QMakeImporter::project() const
{
    return m_project;
}

KUrl QMakeImporter::buildDirectory() const
{
    return project()->folder();
}

QList<Koncrete::ProjectFolderItem*> QMakeImporter::parse( Koncrete::ProjectFolderItem* item )
{
    QList<Koncrete::ProjectFolderItem*> folderList;

    kDebug(9024) << k_funcinfo << "Parsing item: " << endl;

    QMakeFolderItem* folderitem = dynamic_cast<QMakeFolderItem*>( item );
    if( !folderitem )
        return folderList;

    kDebug(9024) << k_funcinfo << "Item is a qmakefolder: " << endl;

    foreach( QMakeProjectScope* subproject, folderitem->projectScope()->subProjects() )
    {
        folderList.append( new QMakeFolderItem( subproject, subproject->absoluteDirUrl(), item ) );
    }
    foreach( KUrl u, folderitem->projectScope()->files() )
    {
        new Koncrete::ProjectFileItem( u, item );
    }
    foreach( QString s, folderitem->projectScope()->targets() )
    {
        new QMakeTargetItem( s,  item );
    }
    kDebug(9024) << k_funcinfo << "Added " << folderList.count() << " Elements" << endl;

    return folderList;
}

Koncrete::ProjectItem* QMakeImporter::import( Koncrete::ProjectModel* model,
                                        const KUrl& dirName )
{
    if( !dirName.isLocalFile() )
    {
        //FIXME turn this into a real warning
        kWarning(9025) << "not a local file. QMake support doesn't handle remote projects" << endl;
    }else
    {
        QFileInfo fi( dirName.toLocalFile() );
        QDir dir( dirName.toLocalFile() );
        QStringList l = dir.entryList( QStringList() << "*.pro" );

        QString projectfile;

        if( !l.count() || ( l.count() && l.indexOf( fi.baseName() + ".pro" ) != -1 ) )
        {
            projectfile = fi.baseName() + ".pro";
        }else
        {
            projectfile = l.first();
        }

        KUrl projecturl = dirName;
        projecturl.adjustPath( KUrl::AddTrailingSlash );
        projecturl.setFileName( projectfile );
        m_rootItem = new QMakeFolderItem( new QMakeProjectScope( projecturl ), dirName );
    }

    return m_rootItem;
}

KUrl QMakeImporter::findMakefile( Koncrete::ProjectFolderItem* folder ) const
{

    QMakeFolderItem* qmitem = dynamic_cast<QMakeFolderItem*>( folder );
    if( !qmitem )
    {
        return KUrl();
    }
    return qmitem->projectScope()->absoluteFileUrl();
}

KUrl::List QMakeImporter::findMakefiles( Koncrete::ProjectFolderItem* folder ) const
{
    QMakeFolderItem* qmitem = dynamic_cast<QMakeFolderItem*>( folder );
    if( !qmitem )
    {
        return KUrl::List();
    }
    KUrl::List l;

    l.append( qmitem->projectScope()->absoluteFileUrl() );
    return l;
}

QList<Koncrete::ProjectTargetItem*> QMakeImporter::targets() const
{
    return QList<Koncrete::ProjectTargetItem*>();
}

KUrl::List QMakeImporter::includeDirectories() const
{
    return m_includeDirList;
}

QStringList QMakeImporter::extensions() const
{
    return QStringList() << "IBuildManager" << "IFileManager";
}

Koncrete::ProjectFolderItem * QMakeImporter::top( )
{
    return m_rootItem;
}

Koncrete::ProjectFolderItem * QMakeImporter::addFolder( const KUrl &, Koncrete::ProjectFolderItem* )
{
    return 0;
}

bool QMakeImporter::removeFile( Koncrete::ProjectFileItem * )
{
    return false;
}

bool QMakeImporter::renameFile( Koncrete::ProjectFileItem *, const KUrl & )
{
    return false;
}

bool QMakeImporter::renameFolder( Koncrete::ProjectFolderItem *, const KUrl & )
{
    return false;
}


void QMakeImporter::registerExtensions()
{
    extensionManager()->registerExtensions( new QMakeImporterIFileManagerFactory(
    extensionManager() ), Q_TYPEID( Koncrete::IFileManager ) );
    extensionManager()->registerExtensions( new QMakeImporterIBuildManagerFactory(
    extensionManager() ), Q_TYPEID( Koncrete::IBuildManager ) );
}
void QMakeImporter::unregisterExtensions()
{
    extensionManager()->unregisterExtensions( new QMakeImporterIFileManagerFactory(
    extensionManager() ), Q_TYPEID( Koncrete::IFileManager ) );
    extensionManager()->unregisterExtensions( new QMakeImporterIBuildManagerFactory(
    extensionManager() ), Q_TYPEID( Koncrete::IBuildManager ) );
}

#include "qmakeimporter.moc"
// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
