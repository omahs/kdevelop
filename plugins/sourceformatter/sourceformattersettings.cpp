/* This file is part of KDevelop
*  Copyright (C) 2008 Cédric Pasteur <cedric.pasteur@free.fr>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.

*/
#include "sourceformattersettings.h"

#include <QVBoxLayout>
#include <QList>
#include <QListWidgetItem>
#include <QInputDialog>
#include <KMessageBox>
#include <KIconLoader>
#include <KGenericFactory>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/editorchooser.h>
#include <ktexteditor/configinterface.h>

#include <interfaces/iplugin.h>
#include <util/interfaces/isourceformatter.h>
#include <util/sourceformattermanager.h>

#include "editstyledialog.h"

#define STYLE_ROLE (Qt::UserRole+1)

K_PLUGIN_FACTORY(SourceFormatterSettingsFactory, registerPlugin<SourceFormatterSettings>();)
K_EXPORT_PLUGIN(SourceFormatterSettingsFactory("kcm_kdevsourceformattersettings"))

SourceFormatterSettings::SourceFormatterSettings(QWidget *parent, const QVariantList &args)
		: KCModule(SourceFormatterSettingsFactory::componentData(), parent, args)
{
	setupUi(this);

	init();
}

SourceFormatterSettings::~SourceFormatterSettings()
{
}

void SourceFormatterSettings::init()
{
	// add texteditor preview
	KTextEditor::Editor *editor = KTextEditor::EditorChooser::editor();
	if (!editor)
		KMessageBox::error(this, i18n("A KDE text-editor component could not be found.\n"
		        "Please check your KDE installation."));

	m_document = editor->createDocument(this);

	m_view = qobject_cast<KTextEditor::View*>(m_document->createView(textEditor));
	QVBoxLayout *layout = new QVBoxLayout(textEditor);
	layout->addWidget(m_view);
	textEditor->setLayout(layout);
	m_view->show();

	KTextEditor::ConfigInterface *iface =
	    qobject_cast<KTextEditor::ConfigInterface*>(m_view);
	if (iface) {
		iface->setConfigValue("dynamic-word-wrap", false);
		iface->setConfigValue("icon-bar", false);
	}

	//init language combo box
	SourceFormatterManager *manager = SourceFormatterManager::self();
	QStringList l = manager->languages();
	foreach(QString s, l) {
		//! todo add real icons to support
		QString mime = manager->mimeTypeForLanguage(s).replace('/', '-');
		KIcon icon(mime);
		cbLanguagesStyle->addItem(icon, s);
		cbLanguagesFormatters->addItem(icon, s);
	}

	// set buttons icons
	btnNewStyle->setIcon(KIcon("list-add"));
	btnDelStyle->setIcon(KIcon("list-remove"));
	btnEditStyle->setIcon(KIcon("configure"));

	//connect: first tab
	connect(tabWidget, SIGNAL(currentChanged(QWidget*)), this, SLOT(currentTabChanged()));
	connect(cbLanguagesStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(languagesStylesChanged(int)));
	connect(listStyles, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
	        this, SLOT(currentStyleChanged(QListWidgetItem*, QListWidgetItem*)));
	connect(listStyles, SIGNAL(itemChanged(QListWidgetItem*)),
	        this, SLOT(styleRenamed(QListWidgetItem*)));
	connect(btnDelStyle, SIGNAL(clicked()), this, SLOT(deleteStyle()));
	connect(btnNewStyle, SIGNAL(clicked()), this, SLOT(addStyle()));
	connect(btnEditStyle, SIGNAL(clicked()), this, SLOT(editStyle()));
	connect(chkKateModelines, SIGNAL(stateChanged(int)), this, SLOT(modelineChanged()));

	// connect: second tab
	connect(cbLanguagesFormatters, SIGNAL(currentIndexChanged(int)), this, SLOT(languagesFormattersChanged(int)));
	connect(cbFormatters, SIGNAL(currentIndexChanged(int)), this, SLOT(formattersChanged(int)));
}

void SourceFormatterSettings::addItemInStyleList(const QString &caption, const QString &name, bool editable)
{
	QListWidgetItem *item = new QListWidgetItem(caption);
	item->setData(STYLE_ROLE, name);
	if (editable)
		item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	listStyles->addItem(item);
}

void SourceFormatterSettings::updatePreviewText()
{
	if (m_currentFormatter) {
		m_document->setReadWrite(true);
		m_document->setText(m_currentFormatter->formatSource(m_previewText, m_currentMimeType));
		m_document->setReadWrite(false);
	}
}

void SourceFormatterSettings::load()
{
	SourceFormatterManager::self()->loadConfig(); //reload
	cbLanguagesStyle->setCurrentIndex(0); // select the first item to fill the rest
	cbLanguagesFormatters->setCurrentIndex(0);
	languagesStylesChanged(0);
	languagesFormattersChanged(0);
	//update kate modeline
	chkKateModelines->setChecked(SourceFormatterManager::self()->modelinesEnabled());

	updatePreviewText();
}

void SourceFormatterSettings::save()
{
	SourceFormatterManager::self()->setModelinesEnabled(chkKateModelines->isChecked());
	SourceFormatterManager::self()->saveConfig();
}

void SourceFormatterSettings::currentTabChanged()
{
	int idx = tabWidget->currentIndex();
	if (idx == 0) //style tab shown
		cbLanguagesStyle->setCurrentIndex(cbLanguagesFormatters->currentIndex()); // trigger reload of style list
	else
		cbLanguagesFormatters->setCurrentIndex(cbLanguagesStyle->currentIndex());
}

void SourceFormatterSettings::languagesStylesChanged(int idx)
{
	if (idx < 0) // no selection
		return;
	//update source formatter
	setActiveLanguage(cbLanguagesStyle->currentText(), QString());
	populateStyleList();

	// reload current style
	QString currentStyle = SourceFormatterManager::self()->currentStyle();
	kDebug() << "Trying to select " << currentStyle << endl;
	int selectedRow = 0;
	for (int i = 0; i < listStyles->count(); ++i) {
		QListWidgetItem *item = listStyles->item(i);
		if (item->data(STYLE_ROLE).toString() == currentStyle)
			selectedRow = i;
	}
	kDebug() << "Selected index is " << selectedRow << endl;
	listStyles->setCurrentRow(selectedRow);

	updatePreviewText();
}

void SourceFormatterSettings::populateStyleList()
{
	listStyles->clear();
	//add predefined styles
	QMap<QString, QString> map = m_currentFormatter->predefinedStyles(m_currentMimeType);
	QMap<QString, QString>::const_iterator it = map.constBegin();
	for (; it != map.constEnd(); ++it)
		addItemInStyleList(it.value(), it.key());
	m_numberOfPredefinedStyles = map.count();

	//load custom styles
	KConfigGroup pluginGroup = SourceFormatterManager::self()->configGroup();
	QStringList keyList = pluginGroup.keyList();
	foreach(QString key, keyList) {
		if (key.startsWith("User")) { // style definition
			QString caption = pluginGroup.readEntry("Caption" + key.mid(4));
			addItemInStyleList(caption, key);
			kDebug() << "Adding item in list: user" << key.mid(4)
			<< "  " << caption << endl;
		}
	}
}

void SourceFormatterSettings::currentStyleChanged(QListWidgetItem *current, QListWidgetItem *)
{
	if (!current)
		return;

	QString styleName = current->data(STYLE_ROLE).toString();
	SourceFormatterManager::self()->setCurrentStyle(styleName);
	if (listStyles->currentRow() < m_numberOfPredefinedStyles) {
		// it is a predefined style
//         m_currentFormatter->setStyle(styleName);
		btnDelStyle->setEnabled(false);
		btnEditStyle->setEnabled(false);
	} else {
//         QString content = SourceFormatterManager::self()->loadStyle(styleName);
//         m_currentFormatter->setStyle(QString(), content);
		btnDelStyle->setEnabled(true);
		btnEditStyle->setEnabled(true);
	}
	changed();
	updatePreviewText();
}

void SourceFormatterSettings::styleRenamed(QListWidgetItem *item)
{
	changed();
}

void SourceFormatterSettings::deleteStyle()
{

	int res = KMessageBox::questionYesNo(this, i18n("Are you sure you"
	        " want to delete this style?", i18n("Delete style")));
	if (res == KMessageBox::No)
		return;

	//remove list item
	int idx = listStyles->currentRow();
	QListWidgetItem *item = listStyles->takeItem(idx);
	if (!item)
		return;

	QString styleName = item->data(STYLE_ROLE).toString();
	SourceFormatterManager::self()->deleteStyle(styleName);
	listStyles->setCurrentRow(idx - 1);
}

void SourceFormatterSettings::addStyle()
{
	//ask for caption
	bool ok;
	QString caption = QInputDialog::getText(this,
	        i18n("New style"), i18n("Please enter a name for the new style"),
	        QLineEdit::Normal, i18n("Custom Style"), &ok);
	if (!ok) // dialog aborted
		return;

	EditStyleDialog dialog(m_currentFormatter, m_currentMimeType);
	if (dialog.exec() == QDialog::Accepted) {
		SourceFormatterManager *manager = SourceFormatterManager::self();
		QString name = manager->nameForNewStyle();
		manager->saveStyle(name, dialog.content());
		manager->renameStyle(name, caption);
		// add item in list and select it
		addItemInStyleList(caption, name, true);
		listStyles->setCurrentRow(listStyles->count() - 1);
	}
	changed();
}

void SourceFormatterSettings::editStyle()
{
	QListWidgetItem *item = listStyles->currentItem();
	if (!item)
		return;
	QString styleName = item->data(STYLE_ROLE).toString();

	QString content = SourceFormatterManager::self()->configGroup().readEntry(styleName);
	EditStyleDialog dialog(m_currentFormatter, m_currentMimeType, content);
	if (dialog.exec() == QDialog::Accepted)
		SourceFormatterManager::self()->saveStyle(styleName, dialog.content());
}

void SourceFormatterSettings::modelineChanged()
{
	changed();
}

void SourceFormatterSettings::languagesFormattersChanged(int idx)
{
	if (idx < 0) // no selection
		return;
	//update source formatter
    setActiveLanguage(cbLanguagesFormatters->currentText(), QString());

	poulateFormattersList(); // will call setActiveLanguage
//     updatePreviewText();
}

void SourceFormatterSettings::poulateFormattersList()
{
	cbFormatters->blockSignals(true);
	cbFormatters->clear();

	SourceFormatterManager *manager = SourceFormatterManager::self();
	QList<KDevelop::IPlugin*> list = manager->pluginListForLanguage(cbLanguagesFormatters->currentText());

	int rowToSelect = 0;
	foreach(KDevelop::IPlugin *plugin, list) {
		ISourceFormatter *formatter = plugin->extension<ISourceFormatter>();
		cbFormatters->addItem(formatter->caption(), formatter->name());
		if (m_currentFormatter && (formatter->name() == m_currentFormatter->name())) {
			rowToSelect = cbFormatters->count() - 1;
			kDebug() << "Selecting " << rowToSelect << formatter->name() << endl;
		}
	}
	cbFormatters->blockSignals(false);
	cbFormatters->setCurrentIndex(rowToSelect);
}

void SourceFormatterSettings::formattersChanged(int idx)
{
	// update formatter for this language
	QString lang = cbLanguagesFormatters->currentText();
	QString name = cbFormatters->itemData(idx).toString();
	setActiveLanguage(lang, name);
	populateStyleList();

	//update description label
	lblDescription->setText(m_currentFormatter->description());
	updatePreviewText();
	changed();
}

void SourceFormatterSettings::setActiveLanguage(const QString &lang, const QString &plugin)
{
	kDebug() << "lang = " << lang << " plugin = " << plugin << endl;
	SourceFormatterManager *manager = SourceFormatterManager::self();
	manager->setActiveLanguage(lang, plugin);
	m_currentFormatter = manager->activeFormatter();
	m_currentMimeType = KMimeType::mimeType(manager->mimeTypeForLanguage(lang));
	m_previewText = m_currentFormatter->previewText(m_currentMimeType);
	QString mode = m_currentFormatter->highlightModeForMime(m_currentMimeType);
	m_document->setHighlightingMode(mode);
	m_currentLang = lang;
}

#include "sourceformattersettings.moc"
// kate: indent-mode cstyle; space-indent off; tab-width 4; 
