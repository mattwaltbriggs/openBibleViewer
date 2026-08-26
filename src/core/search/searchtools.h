#ifndef SEARCHTOOLS_H
#define SEARCHTOOLS_H
#include <QString>
#include <string>

class SearchTools
{
public:
    SearchTools();

    static std::string toStdString(const QString& string);
    static QString toQString(const std::string& string);
};

#endif // SEARCHTOOLS_H
