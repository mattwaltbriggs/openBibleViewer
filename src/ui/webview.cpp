#include "webview.h"
#include <QWebEngineSettings>
#include "src/core/dbghelper.h"
#include <QNetworkReply>

WebView::WebView(QWidget *parent) :
    QWebEngineView(parent), m_doBlocking(false)
{
}
void WebView::contextMenuEvent(QContextMenuEvent * ev)
{
    emit contextMenuRequested(ev);
}

void WebView::scrollToAnchor(const QString &anchor)
{
    page()->runJavaScript(QString("window.location.hash = '#%1';").arg(anchor));
}

void WebView::mouseReleaseEvent(QMouseEvent *event)
{
    QWebEngineView::mouseReleaseEvent(event);
}
void WebView::mousePressEvent(QMouseEvent *event)
{
     m_pressedButtons = event->buttons();
     m_keyboardModifiers = event->modifiers();
     QWebEngineView::mousePressEvent(event);
}

void WebView::setBlockRules(const BlockRules &rules)
{
    Q_UNUSED(rules);
    DEBUG_FUNC_NAME
}

void WebView::applyHidingRules()
{
    DEBUG_FUNC_NAME
}

void WebView::load(const QUrl &url)
{
    QWebEngineView::load(url);
}
