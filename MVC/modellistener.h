#ifndef MODELLISTENER_H
#define MODELLISTENER_H

#include <string>
#include <QString>

using std::string;

class Observed;

class ModelListener
{
public:
    virtual void handleEvent(const Observed &) = 0;
};


class Observed
{
    string msg;
    ModelListener *listener;

    void Notify();

public:
    void setListener(ModelListener *lis);

    const string &getMsg() const;

    void send(const char *s);
    void send(const QString &s);
};


#endif // MODELLISTENER_H
