#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <vector>
#include <QStringList>
#include "API/API.h"

using std::vector;

class PluginManager
{
    vector <Filter*> filters;

    QStringList findLibs();
    void loadLibs(QStringList &);
    void loadPlugins(QStringList &LibPaths);

public:
    void refreshPlugins();
    vector<Filter*> &getFilters();
};


#endif // PLUGINMANAGER_H
