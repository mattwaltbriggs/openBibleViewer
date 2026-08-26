#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QWebEngineView>
#include <QWebEnginePage>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QUrl>
#include "src/core/blockrules.h"

class WebView : public QWebEngineView
{
    Q_OBJECT
public:
    explicit WebView(QWidget *parent = nullptr);

    void scrollToAnchor(const QString &anchor);

    void setBlockRules(const BlockRules &rules);

    void load(const QUrl &url);
protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent * ev) override;
signals:
    void contextMenuRequested(QContextMenuEvent * ev);
    void linkMiddleOrCtrlClicked(const QUrl &url);
    void linkShiftClicked(const QUrl &url);
private slots:
    void applyHidingRules();

private:
    Qt::KeyboardModifiers m_keyboardModifiers;
    Qt::MouseButtons m_pressedButtons;

    bool m_doBlocking;
};

#endif // WEBVIEW_H
