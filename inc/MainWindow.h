/*
 * CuteESP
 * Copyright (C) 2017 Thanasis Georgiou
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESP_FLASHING_UTILITY_MAINWINDOW_H
#define ESP_FLASHING_UTILITY_MAINWINDOW_H

#include <QtCore/QArgument>
#include <QtWidgets/QWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QAbstractButton>
#include <QtGui/QStandardItemModel>
#include <QtCore/QProcess>
#include "ui_MainWindow.h"

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

    /**
     * Display an error message box
     * @param message   Message to display
     */
    int showMessage(QString title, QString message);

private slots:

    /**
     * Refreshes the serial ports in cboxSerial
     */
    void refreshSerialPorts();

    /**
     * Show a file-open dialog for the firmware file
     */
    void addFirmware();

    /**
     * Remove the currently selected firmware file
     */
    void removeFirmware();

    /**
     * Display some info for the selected firmware
     */
    void firmwareInfo();

    /**
     * Check if there are any rows remaining so the flash/verify
     * buttons are only enabled when it makes sense
     */
    void rowsChanged();

    /**
     * Call ESPTool to flash the firmware to the uController
     */
    void flashFirmware();

    /**
     * This slot parses the output of the esptool to
     * update the progress bar and the flashing log.
     */
    void handleESPToolOutput();

    /**
     * Display a dialog with info about the app
     */
    void about();

    /**
     * Checks if a table row is selected to enable the remove/info buttons
     */
    void tableSelectionChanged();

private:
    // Generated UI
    Ui::MainWindow ui;
    QStandardItemModel *firmwareModel;
    QProcess *esptool;

    /**
     * Enable or disable most of the UI elements. Use when flashing so the user
     * can't click stuff.
     * @param state     True for enabled, false for disabled
     */
    void setUiEnabled(bool state);

    /**
     * Connect signals for the UI elements
     */
    void connectSignals();
};

#endif //ESP_FLASHING_UTILITY_MAINWINDOW_H
