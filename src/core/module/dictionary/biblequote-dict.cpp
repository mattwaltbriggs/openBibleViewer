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
#include "biblequote-dict.h"
#include "src/core/xapian_wrapper.h"
#include "src/core/module/response/stringresponse.h"
#include "src/core/search/searchtools.h"
#include <QStringConverter>
#ifdef Q_OS_WIN
#include <tchar.h>
#endif

BibleQuoteDict::BibleQuoteDict()
{
}


/**
  Reads the ini file and returns the dictionary name.
  */
MetaInfo BibleQuoteDict::readInfo(QFile &file)
{
    const QString encoding = m_settings->encoding;
    QTextCodec *codec = QTextCodec::codecForName(encoding.toStdString().c_str());
    QTextDecoder *decoder = codec->makeDecoder();
    QByteArray byteline = file.readLine();
    QString line = decoder->toUnicode(byteline);
    file.close();
    MetaInfo info;
    info.setName(line.simplified());
    return info;
}
MetaInfo BibleQuoteDict::readInfo(const QString &fileName)
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
        return MetaInfo();
    return readInfo(file);
}

bool BibleQuoteDict::hasIndex()
{
    QDir d;
    if(!d.exists(m_settings->homePath + "index")) {
        return false;
    }
    const QString index = indexPath();
    try {
        Xapian::Database db(index.toStdString());
        db.close();
        return true;
    } catch (const Xapian::Error&) {
        return false;
    }
}
int BibleQuoteDict::buildIndex()
{
    DEBUG_FUNC_NAME

    // parse both and add docs to the indexwriter
    //myDebug() << m_modulePath;
    QFileInfo fileInfo(m_modulePath);
    //myDebug() << fileInfo.absoluteDir();
    QDir moduleDir(fileInfo.absoluteDir());
    moduleDir.setFilter(QDir::Files);
    QFileInfoList list = moduleDir.entryInfoList();

    QFileInfo htmlFileInfo;
    foreach(const QFileInfo & info, list) {
        if((info.suffix().compare("html", Qt::CaseInsensitive) == 0 || info.suffix().compare("htm", Qt::CaseInsensitive) == 0) && info.baseName().compare(fileInfo.baseName(), Qt::CaseInsensitive) == 0) {
            htmlFileInfo = info;
            break;
        }
    }
    //myDebug() << htmlFileInfo.absoluteFilePath();
    if(!htmlFileInfo.isReadable() || !fileInfo.isReadable()) {
        myWarning() << "cannot open file to build index";
        //todo: qmessagebox
        return 1;
    }

    QFile configFile(fileInfo.absoluteFilePath());
    QFile htmlFile(htmlFileInfo.absoluteFilePath());

    const QString encoding = m_settings->encoding;

    if(!configFile.open(QIODevice::ReadOnly | QIODevice::Text) || !htmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        myWarning() << "cannot open file to build index";
        return 1;
    }
    QTextStream configIn(&configFile);
    configIn.setEncoding(QStringConverter::Utf8);
    QTextStream htmlIn(&htmlFile);
    htmlIn.setEncoding(QStringConverter::Utf8);

    const QString index = indexPath();
    QDir dir("/");
    dir.mkpath(index);
    QProgressDialog progress(QObject::tr("Build index"), QObject::tr("Cancel"), 0, 0);
    progress.setWindowModality(Qt::WindowModal);

    Xapian::WritableDatabase db(index.toStdString(), Xapian::DB_CREATE_OR_OVERWRITE);
    Xapian::TermGenerator tg;

    const QString title = configIn.readLine();
    QString id = configIn.readLine();
    long num = configIn.readLine().toLong();
    const QString pre = htmlIn.read(num - 1);
    myDebug() << title << pre;
    while(!configIn.atEnd()) {
        long n = num;
        const QString key = id;
        id = configIn.readLine();
        num = configIn.readLine().toLong();
        const QString data = htmlIn.read(num - n - 1);
        if(key.isEmpty() || data.isEmpty())
            continue;

        Xapian::Document doc;
        doc.set_data(SearchTools::toStdString(data));
        doc.add_value(0, SearchTools::toStdString(key));
        tg.set_document(doc);
        tg.index_text(SearchTools::toStdString(key));
        tg.index_text(SearchTools::toStdString(data));
        db.add_document(doc);

    }

    db.commit();
    return 0;
}

Response* BibleQuoteDict::getEntry(const QString &key)
{
    DEBUG_FUNC_NAME
    if(!hasIndex()) {
        if(buildIndex() != 0) {
            return new StringResponse(QObject::tr("Cannot build index."));
        }
    }
    const QString index = indexPath();
    std::string keyStr = SearchTools::toStdString(key);

    Xapian::Database db(index.toStdString());
    QString ret = "";
    for(auto it = db.postlist_begin(""); it != db.postlist_end(""); ++it) {
        Xapian::Document doc = db.get_document(*it);
        if(doc.get_value(0) == keyStr) {
            if(!ret.isEmpty())
                ret.append("<hr /> ");
            ret.append(SearchTools::toQString(doc.get_data()));
        }
    }
    return ret.isEmpty() ? new StringResponse(QObject::tr("Nothing found for %1").arg(key)) : new StringResponse(ret);
}
QStringList BibleQuoteDict::getAllKeys()
{
    DEBUG_FUNC_NAME
    if(!m_entryList.isEmpty()) {
        return m_entryList;
    }
    if(!hasIndex())
        buildIndex();
    const QString index = indexPath();
    Xapian::Database db(index.toStdString());
    QStringList ret;
    for(auto it = db.postlist_begin(""); it != db.postlist_end(""); ++it) {
        Xapian::Document doc = db.get_document(*it);
        ret.append(SearchTools::toQString(doc.get_value(0)));
    }
    m_entryList = ret;
    return ret;
}

QString BibleQuoteDict::indexPath() const
{
    return m_settings->homePath + "cache/" + m_settings->hash(m_modulePath);
}
Response::ResponseType BibleQuoteDict::responseType() const
{
    return Response::StringReponse;
}
