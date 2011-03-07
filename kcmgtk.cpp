/***************************************************************************
 *   Copyright (C) 2008 David Sansome <me@davidsansome.com>                *
 *   Copyright (C) 2009 Jonathan Thoams <echidnaman@kubuntu.org>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include "kcmgtk.h"
#include "gtkrcfile.h"
#include "searchpaths.h"

#include <kgenericfactory.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kfontdialog.h>

#include <QDir>
#include <QSettings>
#include <QMessageBox>

#include <sys/stat.h>


const QString KcmGtk::k_gtkRcFileName(QDir::homePath() + "/.gtkrc-2.0-kde4");
const QString KcmGtk::k_qtThemeName("Qt4");
const QString KcmGtk::k_qtcurveThemeName("QtCurve");

K_PLUGIN_FACTORY(KcmGtkFactory, registerPlugin<KcmGtk>();)
K_EXPORT_PLUGIN(KcmGtkFactory("kcm_gtk"))


KcmGtk::KcmGtk(QWidget* parent, const QVariantList&)
	: KCModule(KcmGtkFactory::componentData(), parent)
{
	m_ui.setupUi(this);
	connect(m_ui.fontChange, SIGNAL(clicked()), SLOT(fontChangeClicked()));
	connect(m_ui.fontKde, SIGNAL(clicked(bool)), SLOT(fontKdeClicked()));
	connect(m_ui.styleBox, SIGNAL(activated(int)), SLOT(styleChanged()));
	
	m_gtkRc = new GtkRcFile(k_gtkRcFileName);
	
	m_searchPaths = new SearchPaths(this);
	connect(m_searchPaths, SIGNAL(accepted()), SLOT(getInstalledThemes()));
	connect(m_ui.warning3, SIGNAL(clicked()), m_searchPaths, SLOT(exec()));
	
	// Load icons
	KIconLoader* loader = KIconLoader::global();
	m_ui.styleIcon->setPixmap(loader->loadIcon("preferences-desktop-theme", KIconLoader::Desktop));
	m_ui.fontIcon->setPixmap(loader->loadIcon("preferences-desktop-font", KIconLoader::Desktop));
	
	// Set KCM stuff
	setAboutData(new KAboutData(I18N_NOOP("kcm_gtk"), 0,
                                    ki18n("GTK Styles and Fonts"),0,KLocalizedString(),KAboutData::License_GPL,
                                    ki18n("(C) 2008 David Sansome"),
                                    ki18n("(C) 2009 Jonathan Thomas")));
	setQuickHelp(i18n("Change the appearance of GTK+ applications in KDE"));
	
	// Load GTK settings
	getInstalledThemes();
	load();
	setButtons(Apply);
}

KcmGtk::~KcmGtk()
{
	delete m_gtkRc;
}

void KcmGtk::load()
{
	m_gtkRc->load();
	m_ui.qtcurveFontLabel->hide();

	checkQtCurve();

	m_ui.styleBox->setCurrentIndex(m_themes.keys().indexOf(m_gtkRc->themeName()));
	
	QFont defaultFont;
	bool usingKdeFont = (m_gtkRc->font().family() == defaultFont.family() &&
	                     m_gtkRc->font().pointSize() == defaultFont.pointSize() &&
	                     m_gtkRc->font().bold() == defaultFont.bold() &&
	                     m_gtkRc->font().italic() == defaultFont.italic());
	m_ui.fontKde->setChecked(usingKdeFont);
	m_ui.fontOther->setChecked(!usingKdeFont);
	
	updateFontPreview();
}

void KcmGtk::updateFontPreview()
{
	m_ui.fontPreview->setFont(m_gtkRc->font());
	m_ui.fontPreview->setText(i18n("%1 (size %2)", m_gtkRc->font().family(), QString::number(m_gtkRc->font().pointSize())));
}

void KcmGtk::save()
{
	// Write ~/.gtkrc-2.0-kde4
	m_gtkRc->save();
}

void KcmGtk::defaults()
{
}

void KcmGtk::getInstalledThemes()
{
	m_themes.clear();
	Q_FOREACH (QString path, m_searchPaths->paths())
	{
		path += "/share/themes/";
		Q_FOREACH (QString subdir, QDir(path).entryList(QDir::Dirs, QDir::Unsorted))
		{
			if (subdir.startsWith('.'))
				continue;
			if (m_themes.contains(subdir))
				continue;
			if (!QFile::exists(path + subdir + "/gtk-2.0/gtkrc"))
				continue;
			m_themes[subdir] = path + subdir + "/gtk-2.0/gtkrc";
		}
	}

	m_ui.styleBox->clear();
	m_ui.styleBox->addItems(m_themes.keys());
}

void KcmGtk::fontChangeClicked()
{
	QFont font(m_gtkRc->font());
	if (KFontDialog::getFont(font) != KFontDialog::Accepted)
		return;
	
	m_gtkRc->setFont(font);
	updateFontPreview();
	m_ui.fontOther->setChecked(true);
	changed(true);
}

void KcmGtk::fontKdeClicked()
{
	m_gtkRc->setFont(QFont());
	updateFontPreview();
	changed(true);
}

void KcmGtk::styleChanged()
{
	m_gtkRc->setTheme(m_themes[m_ui.styleBox->currentText()]);
	checkQtCurve();
	changed(true);
}

void KcmGtk::checkQtCurve()
{
	if (m_gtkRc->themeName() == k_qtcurveThemeName)
	{
		m_ui.qtcurveFontLabel->show();
	}
	else
	{
		m_ui.qtcurveFontLabel->hide();
	}
	m_ui.fontOther->setEnabled(m_gtkRc->themeName() != k_qtcurveThemeName);
	m_ui.fontChange->setEnabled(m_gtkRc->themeName() != k_qtcurveThemeName);
	m_ui.fontPreview->setEnabled(m_gtkRc->themeName() != k_qtcurveThemeName);
}
