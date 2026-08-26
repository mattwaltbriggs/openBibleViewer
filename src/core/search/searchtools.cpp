#include "searchtools.h"
#include "src/core/dbghelper.h"
#include <QTextCodec>

SearchTools::SearchTools()
{
}

QString SearchTools::toQString(const std::string& string)
{
    return QString::fromUtf8(string.c_str(), static_cast<int>(string.size()));
}

std::string SearchTools::toStdString(const QString& string)
{
    return string.toUtf8().constData();
}
