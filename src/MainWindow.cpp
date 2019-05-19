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

#include <QtSerialPort/QSerialPortInfo>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QDialogButtonBox>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QFileDialog>
#include <QDebug>
#include <QtCore/QProcess>
#include <QScrollBar>
#include <QtWidgets/QMessageBox>
#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    ui.setupUi(this);

    // Create firmware table model
    firmwareModel = new QStandardItemModel(0, 2);
    firmwareModel->setHeaderData(0, Qt::Horizontal, "Address");
    firmwareModel->setHeaderData(1, Qt::Horizontal, "Firmware file");
    ui.tableView->setModel(firmwareModel);

    // Stretch firmware column to fill width
    QHeaderView *headerView = ui.tableView->horizontalHeader();
    headerView->setSectionResizeMode(1, QHeaderView::Stretch);

    // Connect signals
    connectSignals();

    // Find serial ports and fill dropdown box
    refreshSerialPorts();
}

void MainWindow::connectSignals() {
    // Refresh ports button
    connect(
            ui.btnRefresh, &QPushButton::clicked,
            this, &MainWindow::refreshSerialPorts
    );

    // Add/remove/info firmware buttons
    connect(
            ui.btnAddFirmware, &QPushButton::clicked,
            this, &MainWindow::addFirmware
    );
    connect(
            ui.btnRmFirmware, &QPushButton::clicked,
            this, &MainWindow::removeFirmware
    );
    connect(
            ui.btnFirmwareInfo, &QPushButton::clicked,
            this, &MainWindow::firmwareInfo
    );

    // Firmware selection/change
    connect(
            ui.tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::tableSelectionChanged
    );
    connect(
            firmwareModel, &QStandardItemModel::rowsRemoved,
            this, &MainWindow::rowsChanged
    );
    connect(
            firmwareModel, &QStandardItemModel::rowsInserted,
            this, &MainWindow::rowsChanged
    );

    // Flash firmware button
    connect(
            ui.btnFlash, &QPushButton::clicked,
            this, &MainWindow::flashFirmware
    );

    // Menu -> Help actions
    connect(
            ui.actionAbout_Qt, &QAction::triggered,
            QApplication::aboutQt
    );
    connect(
            ui.actionAbout_CuteESP, &QAction::triggered,
            this, &MainWindow::about
    );
}

void MainWindow::refreshSerialPorts() {
    // Grab all ports
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();

    // Clear box
    ui.cboxSerialPort->clear();

    // For each one
    for (int i = 0; i < ports.size(); i++) {
        // Add to combo box
        QSerialPortInfo port = ports[i];
        ui.cboxSerialPort->addItem(port.systemLocation());
    }
}

void MainWindow::addFirmware() {
    // Grab path from a dialog
    QString path = QFileDialog::getOpenFileName(
            this, // Parent
            "Add firmware file", // Dialog title
            "", // Default file (none)
            "Binary files (*.bin)" // File filters
    );

    // If path is empty don't add anything
    if (path.length() == 0) return;

    // Add to firmware table
    QList<QStandardItem *> row;
    row.append(new QStandardItem(tr("0x00")));
    row.append(new QStandardItem(path));
    firmwareModel->appendRow(row);
}

void MainWindow::removeFirmware() {
    // Grab selection
    QItemSelection selection = ui.tableView
            ->selectionModel()
            ->selection();

    // Remove every row
    // @formatter:off
    QList<int> rows;
    foreach (const QModelIndex &index, selection.indexes()) {
        rows.append(index.row());
    }
    qSort(rows);
    // @formatter:on

    int previousRow = -1;
    for (int i = rows.count() - 1; i >= 0; i -= 1) {
        int curRow = rows[i];
        if (curRow != previousRow) {
            firmwareModel->removeRows(curRow, 1);
            previousRow = curRow;
        }
    }
}

void MainWindow::firmwareInfo() {
    // Get selection
    QItemSelection selection = ui.tableView->selectionModel()->selection();
    QString firmwarePath = firmwareModel->item(selection.indexes()[0].row(), 1)->text();

    // Prepare arguments
    QStringList arguments;
    arguments << "image_info" << firmwarePath;

    // Call esptool
    QString program = QCoreApplication::applicationDirPath() + QString("/esptool.py");
    esptool = new QProcess(this);
    esptool->start(program, arguments);

    // Get all output
    esptool->waitForFinished();
    QString output = QString(
            esptool->readAll()
    );
    output.replace("\n", "<br>");

    // Display a dialog
    showMessage(tr("Firmware Info"),
                tr("Info for") +
                QString("<code>") +
                firmwarePath +
                QString("</code>:<br><br><code>") +
                output +
                QString("</code>")
    );

    // Clean some memory
    delete esptool;
}

void MainWindow::flashFirmware() {
    // Collect parameters
    QString serialPort = ui.cboxSerialPort->itemText(
            ui.cboxSerialPort->currentIndex()
    );
    QString flashMode = ui.cboxSerialMode->itemText(
            ui.cboxSerialMode->currentIndex()
    ).toLower();
//    QString flashSize = ui.cboxFlashSize->itemText(
//            ui.cboxFlashSize->currentIndex()
//    );
    QString flashSize = "detect";
    QString flashFreq = "40m";

    // Prepare arguments
    QStringList arguments;
    arguments << "--port"
              << serialPort
              << "write_flash"
              << "--flash_mode"
              << flashMode
              << "--flash_size"
              << flashSize
              << "--flash_freq"
              << flashFreq;

    // Disable compression if asked to
    if (!ui.chkboxCompression->isChecked()) {
        arguments << "--no-compress";
    }

    // Add each entry from the firmware table
    for (int i = 0; i < firmwareModel->rowCount(); i++) {
        QString address = firmwareModel->item(i, 0)->text();
        QString path = firmwareModel->item(i, 1)->text();

        arguments << address;
        arguments << path;
    }

    // Create process and connect signals
    QString program = QCoreApplication::applicationDirPath() + QString("/esptool.py");
    esptool = new QProcess(this);
    esptool->setProcessChannelMode(QProcess::MergedChannels);

    connect(
            esptool, SIGNAL(readyReadStandardOutput()),
            this, SLOT(handleESPToolOutput())
    );
    connect(
            esptool, SIGNAL(readyReadStandardError()),
            this, SLOT(handleESPToolOutput())
    );
    connect(
            esptool, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            [=](int exitCode, QProcess::ExitStatus exitStatus) {
                disconnect(
                        esptool, SIGNAL(readyReadStandardOutput()),
                        this, SLOT(handleESPToolOutput())
                );
                setUiEnabled(true);
            }
    );

    esptool->start(program, arguments);
    if (!esptool->waitForStarted()) {
        // TODO Complain about not finding esptool.py
    } else {
        setUiEnabled(false);
    }
}

void MainWindow::handleESPToolOutput() {
    QTextEdit *txt = ui.txtOutput;
    QScrollBar *scrollBar = txt->verticalScrollBar();

    // Grab data from the output
    char data[1024];
    esptool->read(data, 1024);
    QString string = QString(data);

    // We only print messages that don't include a percentage (%)
    // Those that do are status lines and are used to update the
    // progress bar
    QRegularExpression regex("\\d+ \\%");
    QRegularExpressionMatch match = regex.match(string);
    if (match.hasMatch()) {
        // Remove % symbol from match
        QString perString = match.captured();
        perString.remove(perString.length() - 1, perString.length() - 1);

        // Convert to integer
        bool success;
        int value = perString.toInt(&success, 10);
        if (success) {
            // Update progress bar
            ui.progressBar->setValue(value);
        }
    } else {
        // No percentage found, this is not a status message
        txt->append(string.trimmed());
        scrollBar->setValue(scrollBar->maximum()); // Scroll to bottom
    }
}

void MainWindow::setUiEnabled(bool state) {
    ui.cboxSerialPort->setEnabled(state);
    ui.btnRefresh->setEnabled(state);
    ui.spinBaud->setEnabled(state);
    ui.tableView->setEnabled(state);
    ui.btnAddFirmware->setEnabled(state);
    ui.btnRmFirmware->setEnabled(state);
    ui.btnFirmwareInfo->setEnabled(state);
    ui.cboxSerialMode->setEnabled(state);
    ui.cboxSerialPort->setEnabled(state);
    ui.cboxFlashSize->setEnabled(state);
    ui.cboxFlashFreq->setEnabled(state);
    ui.chkboxCompression->setEnabled(state);
    ui.btnFlash->setEnabled(state);
    ui.btnVerify->setEnabled(state);
    ui.btnDelete->setEnabled(state);
}

int MainWindow::showMessage(QString title, QString message) {
    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.setWindowTitle(title);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);

    return msgBox.exec();
}

void MainWindow::about() {
    QMessageBox::about(this,
                       tr("About CuteESP"),
                       tr("<b>CuteESP</b> is a frontend to the <code>esptool.py</code> "
                                  "tool for flashing ESP32/ESP8266 devices.<br><br>"
                                  "By Thanasis Georgiou "
                                  "<a href=\"mailto:contact@thgeorgiou.com\">"
                                  "(contact@thgeorgiou.com)"
                                  "</a><br>"
                                  "Version 0.1.0"));
}

void MainWindow::tableSelectionChanged() {
    bool buttonState = ui.tableView->selectionModel()->selection().count() == 1;
    ui.btnFirmwareInfo->setEnabled(buttonState);
    ui.btnRmFirmware->setEnabled(buttonState);
}

void MainWindow::rowsChanged() {
    bool firmwareExists = firmwareModel->rowCount() > 0;
    ui.btnFlash->setEnabled(firmwareExists);
    ui.btnVerify->setEnabled(firmwareExists);
}
