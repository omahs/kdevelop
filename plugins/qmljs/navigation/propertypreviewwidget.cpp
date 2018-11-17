/*************************************************************************************
 *  Copyright (C) 2013 by Sven Brauch <svenbrauch@gmail.com>                         *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/


#include "propertypreviewwidget.h"

#include <QQuickWidget>
#include <QQuickItem>
#include <QHBoxLayout>
#include <QLabel>
#include <QStandardPaths>
#include <KLocalizedString>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KDeclarative/KDeclarative>
#include <kdeclarative_version.h>

#include <language/duchain/ducontext.h>
#include <language/duchain/duchainlock.h>

// List of supported properties. The string must be the name of the property,
// which can contain dots if necessary
QHash<QString, SupportedProperty> PropertyPreviewWidget::supportedProperties;

QWidget* PropertyPreviewWidget::constructIfPossible(KTextEditor::Document* doc,
                                                    const KTextEditor::Range& keyRange,
                                                    const KTextEditor::Range& valueRange,
                                                    Declaration* decl,
                                                    const QString& key,
                                                    const QString& value)
{
#define PROP(key, filename, type, class) \
    supportedProperties.insertMulti(key, SupportedProperty(QUrl(base + filename), type, class));

    if ( supportedProperties.isEmpty() ) {
        QString base = QStandardPaths::locate(
            QStandardPaths::GenericDataLocation,
            QStringLiteral("kdevqmljssupport/propertywidgets"),
            QStandardPaths::LocateDirectory
        ) + QLatin1Char('/');

        // Positioning
        PROP(QStringLiteral("width"), QLatin1String("Width.qml"), QString(), QString())
        PROP(QStringLiteral("height"), QLatin1String("Height.qml"), QString(), QString())
        PROP(QStringLiteral("spacing"), QLatin1String("Spacing.qml"), QString(), QString())

        // Margins
        PROP(QStringLiteral("margins"), QLatin1String("Spacing.qml"), QString(), QStringLiteral("QQuickAnchors"));         // matches anchors.margins and anchors { margins: }
        PROP(QStringLiteral("margins"), QLatin1String("Spacing.qml"), QString(), QStringLiteral("QDeclarativeAnchors"));
        PROP(QStringLiteral("leftMargin"), QLatin1String("Spacing.qml"), QString(), QStringLiteral("QQuickAnchors"));
        PROP(QStringLiteral("leftMargin"), QLatin1String("Spacing.qml"), QString(), QStringLiteral("QDeclarativeAnchors"));
        PROP(QStringLiteral("rightMargin"), QLatin1String("Spacing.qml"), QString(), QStringLiteral("QQuickAnchors"));
        PROP(QStringLiteral("rightMargin"), QLatin1String("Spacing.qml"), QString(), QStringLiteral("QDeclarativeAnchors"));
        PROP(QStringLiteral("topMargin"), QLatin1String("Spacing.qml"), QString(), QStringLiteral("QQuickAnchors"));
        PROP(QStringLiteral("topMargin"), QLatin1String("Spacing.qml"), QString(), QStringLiteral("QDeclarativeAnchors"));
        PROP(QStringLiteral("bottomMargin"), QLatin1String("Spacing.qml"), QString(), QStringLiteral("QQuickAnchors"));
        PROP(QStringLiteral("bottomMargin"), QLatin1String("Spacing.qml"), QString(), QStringLiteral("QDeclarativeAnchors"));

        // Animations
        PROP(QStringLiteral("duration"), QLatin1String("Duration.qml"), QString(), QString())

        // Font QDeclarativeFontValueType, QQuickFontValueType
        PROP(QStringLiteral("family"), QLatin1String("FontFamily.qml"), QString(), QStringLiteral("QDeclarativeFontValueType"))
        PROP(QStringLiteral("family"), QLatin1String("FontFamily.qml"), QString(), QStringLiteral("QQuickFontValueType"))
        PROP(QStringLiteral("pointSize"), QLatin1String("FontSize.qml"), QString(), QStringLiteral("QDeclarativeFontValueType"))
        PROP(QStringLiteral("pointSize"), QLatin1String("FontSize.qml"), QString(), QStringLiteral("QQuickFontValueType"))

        // Appearance
        PROP(QStringLiteral("opacity"), QLatin1String("Opacity.qml"), QString(), QString())

        // Type-dependent widgets
        PROP(QString(), QLatin1String("ColorPicker.qml"), QStringLiteral("color"), QString())
    }
#undef PROP

    QList<SupportedProperty> properties;

    properties << supportedProperties.values(key.section(QLatin1Char('.'), -1, -1));
    properties << supportedProperties.values(QString());

    // Explore each possible supported property and return the first supported widget
    DUChainReadLocker lock;

    for (const SupportedProperty& property : qAsConst(properties)) {
        if (!decl || !decl->abstractType() || !decl->context() || !decl->context()->owner()) {
            continue;
        }

        if (!decl->abstractType()->toString().contains(property.typeContains)) {
            continue;
        }

        if (!decl->context()->owner()->toString().contains(property.classContains)) {
            continue;
        }

        return new PropertyPreviewWidget(doc, keyRange, valueRange, property, value);
    }

    return nullptr;
}

void PropertyPreviewWidget::updateValue()
{
    QString newValue = view->rootObject()->property("value").toString();

    // set the cursor to the edited range, otherwise the view will jump if we call doc->endEditing()
    //document->activeView()->setCursorPosition(KTextEditor::Cursor(valueRange.start.line, valueRange.start.column));
    if (valueRange.end().column() - valueRange.start().column() == newValue.size()) {
        document->replaceText(valueRange, newValue);
    }
    else {
        // the length of the text changed so don't replace it but remove the old
        // and insert the new text.
        KTextEditor::Document::EditingTransaction transaction(document);
        document->removeText(valueRange);
        document->insertText(valueRange.start(), newValue);

        valueRange.setRange(
            valueRange.start(),
            KTextEditor::Cursor(valueRange.start().line(), valueRange.start().column() + newValue.size())
        );
    }
}

PropertyPreviewWidget::~PropertyPreviewWidget()
{
}

PropertyPreviewWidget::PropertyPreviewWidget(KTextEditor::Document* doc,
                                             const KTextEditor::Range& keyRange, const KTextEditor::Range& valueRange,
                                             const SupportedProperty& property, const QString& value)
    : QWidget()
    , view(new QQuickWidget)
    , document(doc)
    , keyRange(keyRange)
    , valueRange(valueRange)
    , property(property)
{
    //setup kdeclarative library
    KDeclarative::KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(view->engine());
#if KDECLARATIVE_VERSION >= QT_VERSION_CHECK(5, 45, 0)
    kdeclarative.setupEngine(view->engine());
    kdeclarative.setupContext();
#else
    kdeclarative.setupBindings();        //binds things like kconfig and icons
#endif

    // Configure layout
    auto l = new QHBoxLayout;

    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);

    // see docstring for ILanguageSupport::specialLanguageObjectNavigationWidget
    setProperty("DoNotCloseOnCursorMove", true);

    view->setSource(property.qmlfile);

    if (!view->rootObject()) {
        // don't crash because of a syntax error or missing QML file
        l->addWidget(new QLabel(i18n("Error loading QML file: %1", property.qmlfile.path())));
        delete view;
        return;
    }

    // set the initial value read from the document
    view->rootObject()->setProperty("initialValue", value);

    // connect to the slot which has to be emitted from QML when the value changes
    QObject::connect(view->rootObject(), SIGNAL(valueChanged()),
                     this, SLOT(updateValue()));
    l->addWidget(view);
}
