/***************************************************************************
openBibleViewer - Bible Study Tool
Copyright (C) 2009-2012 Paul Walger <metaxy@walger.name>
This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3 of the License, or (at your option)
any later version.
This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
this program; if not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/
#include "moduleapi.h"

ModuleApi::ModuleApi(QObject *parent) :
    QObject(parent)
{
}
ModuleApi::~ModuleApi()
{

}

void ModuleApi::activateModule(const int verseTableID)
{
    //DEBUG_FUNC_NAME;
    //myDebug() << verseTableID;

    m_actions->setActiveItem(verseTableID);

    QString js = QString(
        "(function() {"
        "  var elements = document.querySelectorAll('td[class~=verseTableTitle]');"
        "  elements.forEach(function(el) {"
        "    el.classList.remove('active');"
        "    if (el.getAttribute('verseTableID') === '%1') {"
        "      el.classList.add('active');"
        "    }"
        "  });"
        "})();"
    ).arg(verseTableID);
    m_webView->page()->runJavaScript(js);
    m_actions->reloadActive();
}
void ModuleApi::deleteModule(const int verseTableID)
{
    DEBUG_FUNC_NAME
    m_actions->removeModuleFromVerseTable(verseTableID);
}

void ModuleApi::setWebView(QWebEngineView *view)
{
    m_webView = view;
}
int ModuleApi::getModuleIdByName(const QString &name)
{
    foreach(ModuleSettings *set, m_settings->m_moduleSettings) {
        if(set->moduleName == name) return set->moduleID;
    }
    return 0;
}
QString ModuleApi::name() const
{
    return "Module";
}
