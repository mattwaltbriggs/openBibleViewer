#include "src/core/dbghelper.h"
#include "src/ui/dialog/insertlinkdialog.h"
#include "src/core/moduletools.h"

#include "ui_noteseditor.h"
#include "noteseditor.h"
#include <QFileDialog>
#include <QFontDatabase>
#include <QInputDialog>
#include <QColorDialog>
#include "ui_inserthtmldialog.h"
#include "highlighter.h"
#include <QWebEnginePage>
#include <QWebEngineProfile>

#define FORWARD_ACTION(action1, action2) \
    connect(action1, &QAction::triggered, \
            ui->webView->page(), &QWebEnginePage::action); \

NotesEditor::NotesEditor(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::NotesEditor)
    , m_sourceDirty(true)
    , m_highlighter(0)
    , ui_dialog(0)
    , m_insertHtmlDialog(0)
{
    ui->setupUi(this);
    ui->tabWidget->setTabText(0, tr("Normal View"));
    ui->tabWidget->setTabText(1, tr("HTML Source"));
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &NotesEditor::changeTab);

    m_highlighter = new Highlighter(ui->plainTextEdit->document());
    m_simpleNotes = new SimpleNotes();

    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    ui->standardToolBar->insertWidget(ui->actionZoomOut, spacer);

    m_zoomLabel = new QLabel(this);
    ui->standardToolBar->insertWidget(ui->actionZoomOut, m_zoomLabel);

    m_zoomSlider = new QSlider(this);
    m_zoomSlider->setOrientation(Qt::Horizontal);
    m_zoomSlider->setMaximumWidth(150);
    m_zoomSlider->setRange(25, 500);
    m_zoomSlider->setSingleStep(25);
    m_zoomSlider->setPageStep(100);
    connect(m_zoomSlider, &QSlider::valueChanged, this, &NotesEditor::changeZoom);
    ui->standardToolBar->insertWidget(ui->actionZoomIn, m_zoomSlider);

    connect(ui->actionFileSave, &QAction::triggered, this, &NotesEditor::fileSave);
    connect(ui->actionNewFile, &QAction::triggered, m_simpleNotes, &SimpleNotes::newTextNote);
    connect(ui->actionFileSaveAs, &QAction::triggered, this, &NotesEditor::fileSaveAs);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionInsertImage, &QAction::triggered, this, &NotesEditor::insertImage);
    connect(ui->actionCreateLink, &QAction::triggered, this, qOverload<>(&NotesEditor::createLink));
    connect(ui->actionInsertHtml, &QAction::triggered, this, &NotesEditor::insertHtml);
    connect(ui->actionZoomOut, &QAction::triggered, this, &NotesEditor::zoomOut);
    connect(ui->actionZoomIn, &QAction::triggered, this, &NotesEditor::zoomIn);

    connect(ui->actionEditUndo, &QAction::triggered, this, [this]() {
        ui->webView->page()->triggerAction(QWebEnginePage::Undo);
    });
    connect(ui->actionEditRedo, &QAction::triggered, this, [this]() {
        ui->webView->page()->triggerAction(QWebEnginePage::Redo);
    });
    connect(ui->actionEditCut, &QAction::triggered, this, [this]() {
        ui->webView->page()->triggerAction(QWebEnginePage::Cut);
    });
    connect(ui->actionEditCopy, &QAction::triggered, this, [this]() {
        ui->webView->page()->triggerAction(QWebEnginePage::Copy);
    });
    connect(ui->actionEditPaste, &QAction::triggered, this, [this]() {
        ui->webView->page()->triggerAction(QWebEnginePage::Paste);
    });
    connect(ui->actionFormatBold, &QAction::triggered, this, [this]() {
        ui->webView->page()->triggerAction(QWebEnginePage::ToggleBold);
    });
    connect(ui->actionFormatItalic, &QAction::triggered, this, [this]() {
        ui->webView->page()->triggerAction(QWebEnginePage::ToggleItalic);
    });
    connect(ui->actionFormatUnderline, &QAction::triggered, this, [this]() {
        ui->webView->page()->triggerAction(QWebEnginePage::ToggleUnderline);
    });

    connect(ui->actionEditSelectAll, &QAction::triggered, this, &NotesEditor::editSelectAll);

    connect(ui->actionStyleParagraph, &QAction::triggered, this, &NotesEditor::styleParagraph);
    connect(ui->actionStyleHeading1, &QAction::triggered, this, &NotesEditor::styleHeading1);
    connect(ui->actionStyleHeading2, &QAction::triggered, this, &NotesEditor::styleHeading2);
    connect(ui->actionStyleHeading3, &QAction::triggered, this, &NotesEditor::styleHeading3);
    connect(ui->actionStyleHeading4, &QAction::triggered, this, &NotesEditor::styleHeading4);
    connect(ui->actionStyleHeading5, &QAction::triggered, this, &NotesEditor::styleHeading5);
    connect(ui->actionStyleHeading6, &QAction::triggered, this, &NotesEditor::styleHeading6);
    connect(ui->actionStylePreformatted, &QAction::triggered, this, &NotesEditor::stylePreformatted);
    connect(ui->actionStyleAddress, &QAction::triggered, this, &NotesEditor::styleAddress);
    connect(ui->actionFormatFontName, &QAction::triggered, this, &NotesEditor::formatFontName);
    connect(ui->actionFormatFontSize, &QAction::triggered, this, &NotesEditor::formatFontSize);
    connect(ui->actionFormatTextColor, &QAction::triggered, this, &NotesEditor::formatTextColor);
    connect(ui->actionFormatBackgroundColor, &QAction::triggered, this, &NotesEditor::formatBackgroundColor);

    connect(ui->actionFormatStrikethrough, &QAction::triggered, this, &NotesEditor::formatStrikeThrough);
    connect(ui->actionFormatAlignLeft, &QAction::triggered, this, &NotesEditor::formatAlignLeft);
    connect(ui->actionFormatAlignCenter, &QAction::triggered, this, &NotesEditor::formatAlignCenter);
    connect(ui->actionFormatAlignRight, &QAction::triggered, this, &NotesEditor::formatAlignRight);
    connect(ui->actionFormatAlignJustify, &QAction::triggered, this, &NotesEditor::formatAlignJustify);
    connect(ui->actionFormatDecreaseIndent, &QAction::triggered, this, &NotesEditor::formatDecreaseIndent);
    connect(ui->actionFormatIncreaseIndent, &QAction::triggered, this, &NotesEditor::formatIncreaseIndent);
    connect(ui->actionFormatNumberedList, &QAction::triggered, this, &NotesEditor::formatNumberedList);
    connect(ui->actionFormatBulletedList, &QAction::triggered, this, &NotesEditor::formatBulletedList);

    connect(ui->webView->page(), &QWebEnginePage::selectionChanged, this, &NotesEditor::adjustActions);
    connect(ui->webView->page(), &QWebEnginePage::loadFinished, this, &NotesEditor::adjustSource);
    connect(m_simpleNotes, &SimpleNotes::contentChanged, this, &NotesEditor::adjustSource);
    ui->webView->setFocus();

    fileNew();

    adjustActions();
    adjustSource();
    changeZoom(100);
}
void NotesEditor::init()
{
    setAll(m_simpleNotes);
    m_simpleNotes->setWebView(ui->webView);
    m_simpleNotes->setViewWidget(ui->treeView);
    m_simpleNotes->setTitleWidget(ui->lineEdit_noteTitle);
    m_simpleNotes->setLinkButtonWidget(ui->pushButton_editNoteLink);
    m_simpleNotes->setLinkWidget(ui->label_noteLink);

    m_simpleNotes->init();
    connect(ui->webView->page(), &QWebEnginePage::urlChanged, this, &NotesEditor::parseUrl);
}

NotesEditor::~NotesEditor()
{
    delete m_simpleNotes;
    delete m_highlighter;

    delete ui;
    delete ui_dialog;
}
void NotesEditor::adjustSource()
{
    DEBUG_FUNC_NAME
    myDebug() << "current index = " << ui->tabWidget->currentIndex();
    m_sourceDirty = true;

    if(ui->tabWidget->currentIndex() == 1)
        changeTab(1);
}
void NotesEditor::adjustHtml()
{
    DEBUG_FUNC_NAME;
    ui->webView->page()->setHtml(ui->plainTextEdit->toPlainText());
}

void NotesEditor::changeTab(int index)
{
    DEBUG_FUNC_NAME;
    myDebug() << "dirty = " << m_sourceDirty << " index = " << index;
    if(m_sourceDirty && (index == 1)) {
        ui->webView->page()->toHtml([this](const QString &html) {
            ui->plainTextEdit->setPlainText(html);
        });
        m_sourceDirty = false;
    } else if(index == 0) {
        adjustHtml();
    }
}

void NotesEditor::fileNew()
{
    ui->tabWidget->setCurrentIndex(0);
    ui->webView->setHtml("<p></p>");
    ui->webView->setFocus();
}
bool NotesEditor::fileSave()
{
    if(ui->tabWidget->currentIndex() == 1)
        adjustHtml();
    m_simpleNotes->saveNote();
    return true;
}

bool NotesEditor::fileSaveAs()
{
    if(ui->tabWidget->currentIndex() == 1)
        adjustHtml();

    QFileDialog dialog(this);

    dialog.setAcceptMode(QFileDialog::AcceptSave);

    QString fn = dialog.getSaveFileName(this, tr("Save as..."),
                                        QString(), tr("HTML-Files (*.html *.htm);;Text-Files (*.txt);;All Files (*)"));
    if(fn.isEmpty())
        return false;
    QFile data(fn);
    if(data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);
        if(fn.endsWith(".html", Qt::CaseInsensitive) || fn.endsWith(".htm", Qt::CaseInsensitive)) {
            ui->webView->page()->toHtml([&out](const QString &html) {
                out << html;
            });
        } else {
            ui->webView->page()->toPlainText([&out](const QString &text) {
                out << text;
            });
        }
        data.close();
    }

    return fileSave();
}

void NotesEditor::insertImage()
{
    QString filters;
    filters += tr("Common Graphics (*.png *.jpg *.jpeg *.gif);;");
    filters += tr("Portable Network Graphics (PNG) (*.png);;");
    filters += tr("JPEG (*.jpg *.jpeg);;");
    filters += tr("Graphics Interchange Format (*.gif);;");
    filters += tr("All Files (*)");

    QString fn = QFileDialog::getOpenFileName(this, tr("Open image..."),
                 QString(), filters);
    if(fn.isEmpty())
        return;
    if(!QFile::exists(fn))
        return;

    QUrl url = QUrl::fromLocalFile(fn);
    execCommand("insertImage", url.toString());
}

static QUrl guessUrlFromString(const QString &string)
{
    QString urlStr = string.trimmed();
    QRegularExpression test(QLatin1String("^[a-zA-Z]+\\:.*"));

    bool hasSchema = test.match(urlStr).hasMatch();
    if(hasSchema) {
        QUrl url(urlStr, QUrl::TolerantMode);
        if(url.isValid())
            return url;
    }

    if(QFile::exists(urlStr))
        return QUrl::fromLocalFile(urlStr);

    if(!hasSchema) {
        int dotIndex = urlStr.indexOf(QLatin1Char('.'));
        if(dotIndex != -1) {
            QString prefix = urlStr.left(dotIndex).toLower();
            QString schema = (prefix == QLatin1String("ftp")) ? prefix : QLatin1String("http");
            QUrl url(schema + QLatin1String("://") + urlStr, QUrl::TolerantMode);
            if(url.isValid())
                return url;
        }
    }

    return QUrl(string, QUrl::TolerantMode);
}

void NotesEditor::createLink()
{
    InsertLinkDialog *insertLinkDialog = new InsertLinkDialog(this);
    connect(insertLinkDialog, &InsertLinkDialog::newLink, this, qOverload<QString>(&NotesEditor::createLink));
    setAll(insertLinkDialog);

    insertLinkDialog->init();
    insertLinkDialog->show();
    insertLinkDialog->exec();
}
void NotesEditor::createLink(QString link)
{
    if(link.startsWith("verse:") || link.startsWith("note:")) {
        execCommand("createLink", link);
    } else {
        QUrl url = guessUrlFromString(link);
        if(url.isValid())
            execCommand("createLink", url.toString());
    }
}
void NotesEditor::parseUrl(QUrl url)
{
    DEBUG_FUNC_NAME;
    QString link = url.toString();
    const QString note = "note://";
    if(link.startsWith(note)) {
        link = link.remove(0, note.size());
        m_simpleNotes->showNote(link, true);
    } else {
        m_actions->get(url);
    }
}

void NotesEditor::insertHtml()
{
    if(!m_insertHtmlDialog) {
        m_insertHtmlDialog = new QDialog(this);
        if(!ui_dialog)
            ui_dialog = new Ui::InsertHtmlDialog;
        ui_dialog->setupUi(m_insertHtmlDialog);
        connect(ui_dialog->buttonBox, &QDialogButtonBox::accepted,
                m_insertHtmlDialog, &QDialog::accept);
        connect(ui_dialog->buttonBox, &QDialogButtonBox::rejected,
                m_insertHtmlDialog, &QDialog::reject);
    }

    ui_dialog->plainTextEdit->clear();
    ui_dialog->plainTextEdit->setFocus();
    Highlighter *hilite = new Highlighter(ui_dialog->plainTextEdit->document());

    if(m_insertHtmlDialog->exec() == QDialog::Accepted)
        execCommand("insertHTML", ui_dialog->plainTextEdit->toPlainText());

    delete hilite;
}

void NotesEditor::zoomOut()
{
    int percent = static_cast<int>(ui->webView->zoomFactor() * 100);
    if(percent > 25) {
        percent -= 25;
        percent = 25 * (int((percent + 25 - 1) / 25));
        qreal factor = static_cast<qreal>(percent) / 100;
        ui->webView->setZoomFactor(factor);
        ui->actionZoomOut->setEnabled(percent > 25);
        ui->actionZoomIn->setEnabled(true);
        m_zoomSlider->setValue(percent);
    }
}

void NotesEditor::zoomIn()
{
    int percent = static_cast<int>(ui->webView->zoomFactor() * 100);
    if(percent < 400) {
        percent += 25;
        percent = 25 * (int(percent / 25));
        qreal factor = static_cast<qreal>(percent) / 100;
        ui->webView->setZoomFactor(factor);
        ui->actionZoomIn->setEnabled(percent < 400);
        ui->actionZoomOut->setEnabled(true);
        m_zoomSlider->setValue(percent);
    }
}

void NotesEditor::editSelectAll()
{
    ui->webView->page()->triggerAction(QWebEnginePage::SelectAll);
}

void NotesEditor::execCommand(const QString &cmd)
{
    QString js = QString("document.execCommand(\"%1\", false, null)").arg(cmd);
    ui->webView->page()->runJavaScript(js);
}

void NotesEditor::execCommand(const QString &cmd, const QString &arg)
{
    QString js = QString("document.execCommand(\"%1\", false, \"%2\")").arg(cmd).arg(arg);
    ui->webView->page()->runJavaScript(js);
}

bool NotesEditor::queryCommandState(const QString &cmd)
{
    QString js = QString("document.queryCommandState(\"%1\", false, null)").arg(cmd);
    bool result = false;
    QEventLoop loop;
    ui->webView->page()->runJavaScript(js, [&result, &loop](const QVariant &v) {
        result = v.toString().simplified().toLower() == "true";
        loop.quit();
    });
    loop.exec();
    return result;
}

void NotesEditor::styleParagraph()
{
    execCommand("formatBlock", "p");
}

void NotesEditor::styleHeading1()
{
    execCommand("formatBlock", "h1");
}

void NotesEditor::styleHeading2()
{
    execCommand("formatBlock", "h2");
}

void NotesEditor::styleHeading3()
{
    execCommand("formatBlock", "h3");
}

void NotesEditor::styleHeading4()
{
    execCommand("formatBlock", "h4");
}

void NotesEditor::styleHeading5()
{
    execCommand("formatBlock", "h5");
}

void NotesEditor::styleHeading6()
{
    execCommand("formatBlock", "h6");
}

void NotesEditor::stylePreformatted()
{
    execCommand("formatBlock", "pre");
}

void NotesEditor::styleAddress()
{
    execCommand("formatBlock", "address");
}

void NotesEditor::formatStrikeThrough()
{
    execCommand("strikeThrough");
}

void NotesEditor::formatAlignLeft()
{
    execCommand("justifyLeft");
}

void NotesEditor::formatAlignCenter()
{
    execCommand("justifyCenter");
}

void NotesEditor::formatAlignRight()
{
    execCommand("justifyRight");
}

void NotesEditor::formatAlignJustify()
{
    execCommand("justifyFull");
}

void NotesEditor::formatIncreaseIndent()
{
    execCommand("indent");
}

void NotesEditor::formatDecreaseIndent()
{
    execCommand("outdent");
}

void NotesEditor::formatNumberedList()
{
    execCommand("insertOrderedList");
}

void NotesEditor::formatBulletedList()
{
    execCommand("insertUnorderedList");
}

void NotesEditor::formatFontName()
{
    QStringList families = QFontDatabase().families();
    bool ok = false;
    QString family = QInputDialog::getItem(this, tr("Font"), tr("Select font:"),
                                           families, 0, false, &ok);

    if(ok)
        execCommand("fontName", family);
}

void NotesEditor::formatFontSize()
{
    QStringList sizes;
    sizes << "xx-small";
    sizes << "x-small";
    sizes << "small";
    sizes << "medium";
    sizes << "large";
    sizes << "x-large";
    sizes << "xx-large";

    bool ok = false;
    QString size = QInputDialog::getItem(this, tr("Font Size"), tr("Select font size:"),
                                         sizes, sizes.indexOf("medium"), false, &ok);

    if(ok)
        execCommand("fontSize", QString::number(sizes.indexOf(size)));
}

void NotesEditor::formatTextColor()
{
    QColor color = QColorDialog::getColor(Qt::black, this);
    if(color.isValid())
        execCommand("foreColor", color.name());
}

void NotesEditor::formatBackgroundColor()
{
    QColor color = QColorDialog::getColor(Qt::white, this);
    if(color.isValid())
        execCommand("hiliteColor", color.name());
}

void NotesEditor::adjustActions()
{
    ui->actionEditUndo->setEnabled(ui->webView->page()->action(QWebEnginePage::Undo)->isEnabled());
    ui->actionEditRedo->setEnabled(ui->webView->page()->action(QWebEnginePage::Redo)->isEnabled());
    ui->actionEditCut->setEnabled(ui->webView->page()->action(QWebEnginePage::Cut)->isEnabled());
    ui->actionEditCopy->setEnabled(ui->webView->page()->action(QWebEnginePage::Copy)->isEnabled());
    ui->actionEditPaste->setEnabled(ui->webView->page()->action(QWebEnginePage::Paste)->isEnabled());

    ui->actionFormatStrikethrough->setChecked(queryCommandState("strikeThrough"));
    ui->actionFormatNumberedList->setChecked(queryCommandState("insertOrderedList"));
    ui->actionFormatBulletedList->setChecked(queryCommandState("insertUnorderedList"));
}

void NotesEditor::changeZoom(int percent)
{
    ui->actionZoomOut->setEnabled(percent > 25);
    ui->actionZoomIn->setEnabled(percent < 500);
    qreal factor = static_cast<qreal>(percent) / 100;
    ui->webView->setZoomFactor(factor);

    m_zoomLabel->setText(tr(" Zoom: %1% ").arg(percent));
    m_zoomSlider->setValue(percent);
}

void NotesEditor::closeEvent(QCloseEvent *e)
{
    Q_UNUSED(e);
    m_simpleNotes->saveNote();
}
