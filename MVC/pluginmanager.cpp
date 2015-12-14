#include "pluginmanager.h"
#include <QStringList>
#include <QPluginLoader>
#include <QString>
#include <QDir>
#include <iostream>
#include <dlfcn.h>

void PluginManager::refreshPlugins()
{
    filters.clear();

    QStringList LibPaths = findLibs();
    loadPlugins(LibPaths);
}

QStringList PluginManager::findLibs()
{
    QStringList nameFilter("*.so");
    QString path("/Users/vadim/Qt_projects/task2/plugins/");
    QDir dir(path);

    QStringList LibPaths = dir.entryList(nameFilter);
    for (auto it=LibPaths.begin(); it != LibPaths.end(); ++it)
    {
        *it = path+(*it);
    }

    return LibPaths;
}

void PluginManager::loadPlugins(QStringList &LibPaths)
{
    for (auto file: LibPaths)
    {
        void *handle;
        handle = dlopen(file.toStdString().c_str(), RTLD_LAZY);

        Filter* (*create)();
        void (*destroy)(Filter*);

        create = (Filter* (*)())dlsym(handle, "create_object");
        destroy = (void (*)(Filter*))dlsym(handle, "destroy_object");

        filters.push_back(create());
    }
}

vector<Filter*> &PluginManager::getFilters()
{
    return filters;
}
