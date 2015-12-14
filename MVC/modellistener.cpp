#include <string>
#include <QTextEdit>
#include <QString>
#include <QTime>

#include "modellistener.h"

using std::string;

void Observed::Notify()
{
    listener->handleEvent(*this);
}

void Observed::setListener(ModelListener *lis)
{
    listener = lis;
}

const string &Observed::getMsg() const
{
    return msg;
}

void Observed::send(const char *s)
{
    msg = string(s);
    Notify();
}

void Observed::send(const QString &s)
{
    msg = s.toStdString();
    Notify();
}

