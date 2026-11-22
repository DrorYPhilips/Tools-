/****************************************************************************
 *
 * $Id: main.cpp 27028 2014-06-17 22:10:07Z pzeldin $
 *
 * Copyright (C) 2009-2012 Epiphan Systems Inc. All rights reserved.
 *
 * Application entry point
 *
 ****************************************************************************/

#include <QApplication>
#include "grabmainwindow.h"

int main(int argc, char *argv[])
{
    FrmGrabNet_Init();

    QApplication app(argc, argv);
    app.setOrganizationName("Epiphan Systems Inc.");
    app.setOrganizationDomain("epiphan.com");
    app.setApplicationName("Frame grabber application");

    int ret = 1;
    GrabMainWindow* mainWindow = new GrabMainWindow();
    if (mainWindow) {
        mainWindow->show();
        ret = app.exec();
        delete mainWindow;
    }

    FrmGrabNet_Deinit();
    return ret;
}
