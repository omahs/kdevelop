/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

#include "configwidget.h"

#include <KDebug>

#include "ui_configwidget.h"
#include "projectpathsmodel.h"


ConfigWidget::ConfigWidget( QWidget* parent )
    : QWidget ( parent ), ui( new Ui::ConfigWidget ), pathsModel( new ProjectPathsModel( this ) )
{
    ui->setupUi( this );
    ui->buildAction->insertItem( CustomBuildSystemTool::Build, i18n("Build"), QVariant() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Configure, i18n("Configure"), QVariant() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Install, i18n("Install"), QVariant() );
    ui->buildAction->insertItem( CustomBuildSystemTool::Clean, i18n("Clean"), QVariant() );

    connect( ui->buildAction, SIGNAL(activated(int)), SLOT(changeAction(int)) );

    connect( ui->enableAction, SIGNAL(toggled(bool)), SLOT(toggleActionEnablement(bool)) );
    connect( ui->actionArguments, SIGNAL(textEdited(QString)), SLOT(actionArgumentsEdited(QString)) );
    connect( ui->actionEnvironment, SIGNAL(activated(int)), SLOT(actionEnvironmentChanged(int)) );
    connect( ui->actionExecutable, SIGNAL(urlSelected(KUrl)), SLOT(actionExecutableChanged(KUrl)) );
    connect( ui->actionExecutable, SIGNAL(textChanged(QString)), SLOT(actionExecutableChanged(QString)) );

    ui->projectPaths->setModel( pathsModel );
    connect( ui->projectPaths->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(projectPathSelected(QItemSelection,QItemSelection)) );
}

CustomBuildSystemConfig ConfigWidget::config() const
{
    CustomBuildSystemConfig c;
    return c;
}

void ConfigWidget::loadConfig( CustomBuildSystemConfig cfg )
{
    bool b = blockSignals( true );
    ui->buildDir->setUrl( cfg.buildDir );
    fillTools( cfg.tools );
    pathsModel->setPaths( cfg.projectPaths );
    blockSignals( b );
}

void ConfigWidget::fillTools(const QHash< CustomBuildSystemTool::ActionType, CustomBuildSystemTool>& tools)
{
    for( int i = 0; i < ui->buildAction->count(); i++ ) {
        QHash< CustomBuildSystemTool::ActionType, CustomBuildSystemTool>::const_iterator it = tools.find( CustomBuildSystemTool::ActionType( i ) );
        CustomBuildSystemTool t;
        t.enabled = false;
        if( it != tools.end() ) {
            t = *it;
        }
        ui->buildAction->setItemData( i, QVariant::fromValue<CustomBuildSystemTool>( t ) );
    }
    ui->buildAction->setCurrentIndex( CustomBuildSystemTool::Build );
}

void ConfigWidget::changeAction( int idx )
{
    Q_ASSERT( idx >= 0 && idx < ui->buildAction->count() );

    QVariant data = ui->buildAction->itemData( idx );
    if( data.isValid() && data.canConvert<CustomBuildSystemTool>() ) {
        CustomBuildSystemTool t = data.value<CustomBuildSystemTool>();
        bool b = ui->enableAction->blockSignals( true );
        ui->enableAction->setChecked( t.enabled );
        ui->enableAction->blockSignals( b );

        ui->actionArguments->setText( t.arguments );
        ui->actionExecutable->setUrl( t.executable );
        ui->actionEnvironment->setCurrentIndex( ui->actionEnvironment->findText( t.envGrp ) );
    }
}

void ConfigWidget::toggleActionEnablement( bool enable )
{
    QVariant data = ui->buildAction->itemData( ui->buildAction->currentIndex() );
    if( data.isValid() && data.canConvert<CustomBuildSystemTool>() ) {
        CustomBuildSystemTool t = data.value<CustomBuildSystemTool>();
        t.enabled = enable;
        ui->buildAction->setItemData( ui->buildAction->currentIndex(), QVariant::fromValue( t ) );
    }
    emit changed();
}

void ConfigWidget::actionArgumentsEdited( const QString& txt )
{
    QVariant data = ui->buildAction->itemData( ui->buildAction->currentIndex() );
    if( data.isValid() && data.canConvert<CustomBuildSystemTool>() ) {
        CustomBuildSystemTool t = data.value<CustomBuildSystemTool>();
        t.arguments = txt;
        ui->buildAction->setItemData( ui->buildAction->currentIndex(), QVariant::fromValue( t ) );
    }
    emit changed();
}

void ConfigWidget::actionEnvironmentChanged( int )
{
    QVariant data = ui->buildAction->itemData( ui->buildAction->currentIndex() );
    if( data.isValid() && data.canConvert<CustomBuildSystemTool>() ) {
        CustomBuildSystemTool t = data.value<CustomBuildSystemTool>();
        t.envGrp = ui->actionEnvironment->currentProfile();
        ui->buildAction->setItemData( ui->buildAction->currentIndex(), QVariant::fromValue( t ) );
    }
    emit changed();
}

void ConfigWidget::actionExecutableChanged(const KUrl& url )
{
    QVariant data = ui->buildAction->itemData( ui->buildAction->currentIndex() );
    if( data.isValid() && data.canConvert<CustomBuildSystemTool>() ) {
        CustomBuildSystemTool t = data.value<CustomBuildSystemTool>();
        t.executable = url;
        ui->buildAction->setItemData( ui->buildAction->currentIndex(), QVariant::fromValue( t ) );
    }
    emit changed();
}

void ConfigWidget::actionExecutableChanged(const QString& txt )
{
    actionExecutableChanged( KUrl( txt ) );
}

void ConfigWidget::projectPathSelected( const QItemSelection& selected, const QItemSelection& deselected )
{
    bool enable = !( selected.isEmpty() || selected.at(0).topLeft().row() == pathsModel->rowCount() - 1 );
    ui->includePaths->setEnabled( enable );
    ui->defines->setEnabled( enable );
    ui->switchIncludeDefines->setEnabled( enable );
}

#include "configwidget.moc"

