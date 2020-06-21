/***************************************************************************
 *   Copyright (C) 2017 by Popov Alexey                                    *
 *   hovercraft@yandex.ru                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/
 
#ifndef SIMUAPI_APPPATH_H
#define SIMUAPI_APPPATH_H

#include <qdir.h>

class MAINMODULE_EXPORT SIMUAPI_AppPath
{
    public:
        static SIMUAPI_AppPath* self();

        QDir ROExamFolder() const;
        void setROExamFolder(const QDir &ROExamFolder);

        QDir RODataFolder() const;
        void setRODataFolder(const QDir &RODataFolder);

        QDir RWDataFolder() const;
        void setRWDataFolder(const QDir &RWDataFolder);

        QString availableDataFilePath(QString fileRelPath);
        QString availableDataDirPath(QString dirRelPath);

private:
        SIMUAPI_AppPath();
        static SIMUAPI_AppPath* m_pSelf;

        QDir m_ROExamFolder;
        QDir m_RODataFolder;
        QDir m_RWDataFolder;
};

#endif // SIMUAPI_APPPATH_H
