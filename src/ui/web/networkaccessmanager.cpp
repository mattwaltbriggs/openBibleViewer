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
#include "networkaccessmanager.h"

#include <QNetworkReply>
#include <QTimer>
#include <QWidget>
#include <QWebEnginePage>
#include "src/core/dbghelper.h"

class NullNetworkReply : public QNetworkReply
{
public:
    NullNetworkReply(const QNetworkRequest &req, QObject* parent = 0)
        : QNetworkReply(parent)
    {
        setRequest(req);
        setUrl(req.url());
        setHeader(QNetworkRequest::ContentLengthHeader, 0);
        setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");
        setError(QNetworkReply::ContentAccessDenied, "Blocked by ad filter");
        setAttribute(QNetworkRequest::User, QNetworkReply::ContentAccessDenied);
        QTimer::singleShot(0, this, SIGNAL(finished()));
    }

    virtual void abort() {}
    virtual qint64 bytesAvailable() const
    {
        return 0;
    }

protected:
    virtual qint64 readData(char*, qint64)
    {
        return -1;
    }
};


// ----------------------------------------------------------------------------------------------


#define     HIDABLE_ELEMENTS    "audio,img,embed,object,iframe,frame,video"


static void hideBlockedElements(const QUrl& url, QWebEnginePage* page)
{
    QString js = QString(
        "var elements = document.querySelectorAll('%1');"
        "elements.forEach(function(el) {"
        "  var src = el.getAttribute('src');"
        "  if (!src) src = el.src;"
        "  if (src) {"
        "    var resolved = new URL(src, document.baseURI).href;"
        "    if (resolved === '%2') el.remove();"
        "  }"
        "});"
    ).arg(HIDABLE_ELEMENTS, url.toString());
    page->runJavaScript(js);
}


// ----------------------------------------------------------------------------------------------


NetworkAccessManager::NetworkAccessManager(QObject *parent)
    : QNetworkAccessManager(parent), m_doBlock(false)
{
}

void NetworkAccessManager::setBlockRules(const BlockRules &rules)
{
    m_blockRules = rules;
    m_doBlock = true;
}

QNetworkReply *NetworkAccessManager::createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData)
{
    if(!m_doBlock)  return QNetworkAccessManager::createRequest(op, req, outgoingData);

    bool blocked = false;

    // Handle GET operations with AdBlock
    if (op == QNetworkAccessManager::GetOperation) {
        const QString url = req.url().toString().toLower();
        foreach(const QString &m, m_blockRules.m_blackListUrl) {
            if(url.contains(m)) blocked = true;
        }
    }

    if (!blocked)
    {
        return QNetworkAccessManager::createRequest(op, req, outgoingData);
    }

    // if we blocked something then we should hide it
    QWebEnginePage* page = qobject_cast<QWebEnginePage*>(req.originatingObject());
    if (page)
    {
        if (!m_blockedRequests.contains(page))
            connect(page, SIGNAL(loadFinished(bool)), this, SLOT(applyHidingBlockedElements(bool)));
        m_blockedRequests.insert(page, req.url());
    }
    myDebug() << "blocked " << req.url();
    return new NullNetworkReply(req, this);
}


void NetworkAccessManager::applyHidingBlockedElements(bool ok)
{
    if (!ok)
        return;

    QWebEnginePage* page = qobject_cast<QWebEnginePage*>(sender());
    if (!page)
        return;

    QList<QUrl> urls = m_blockedRequests.values(page);
    if (urls.isEmpty())
        return;

    Q_FOREACH(const QUrl & url, urls)
        hideBlockedElements(url, page);
}
