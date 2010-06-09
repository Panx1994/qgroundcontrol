/*=====================================================================

PIXHAWK Micro Air Vehicle Flying Robotics Toolkit

(c) 2009, 2010 PIXHAWK PROJECT  <http://pixhawk.ethz.ch>

This file is part of the PIXHAWK project

    PIXHAWK is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PIXHAWK is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PIXHAWK. If not, see <http://www.gnu.org/licenses/>.

======================================================================*/

/**
 * @file
 *   @brief Definition of widget controlling one MAV
 *
 *   @author Lorenz Meier <mavteam@student.ethz.ch>
 *
 */

#include <QString>
#include <QTimer>
#include <QLabel>
#include <QFileDialog>
#include <QDebug>
#include <QProcess>

#include <MG.h>
#include "UASControlWidget.h"
#include <UASManager.h>
#include <UAS.h>
//#include <mavlink.h>

#define CONTROL_MODE_LOCKED "MODE LOCKED"
#define CONTROL_MODE_MANUAL "MODE MANUAL"
#define CONTROL_MODE_GUIDED "MODE GUIDED"
#define CONTROL_MODE_AUTO   "MODE AUTO"
#define CONTROL_MODE_TEST1  "MODE TEST1"

#define CONTROL_MODE_LOCKED_INDEX 2
#define CONTROL_MODE_MANUAL_INDEX 3
#define CONTROL_MODE_GUIDED_INDEX 4
#define CONTROL_MODE_AUTO_INDEX   5
#define CONTROL_MODE_TEST1_INDEX  6

UASControlWidget::UASControlWidget(QWidget *parent) : QWidget(parent),
        uas(NULL),
        engineOn(false)
{
    ui.setupUi(this);

    connect(UASManager::instance(), SIGNAL(activeUASSet(UASInterface*)), this, SLOT(setUAS(UASInterface*)));
    ui.modeComboBox->insertItem(0, "Select..");
    ui.modeComboBox->insertItem(CONTROL_MODE_LOCKED_INDEX, CONTROL_MODE_LOCKED);
    ui.modeComboBox->insertItem(CONTROL_MODE_MANUAL_INDEX, CONTROL_MODE_MANUAL);
    ui.modeComboBox->insertItem(CONTROL_MODE_GUIDED_INDEX, CONTROL_MODE_GUIDED);
    ui.modeComboBox->insertItem(CONTROL_MODE_AUTO_INDEX, CONTROL_MODE_AUTO);
    ui.modeComboBox->insertItem(CONTROL_MODE_TEST1_INDEX, CONTROL_MODE_TEST1);

    ui.modeComboBox->setCurrentIndex(0);
}

void UASControlWidget::setUAS(UASInterface* uas)
{
    if (this->uas != NULL)
    {
        disconnect(ui.controlButton, SIGNAL(clicked()), uas, SLOT(enable_motors()));
        disconnect(ui.liftoffButton, SIGNAL(clicked()), uas, SLOT(launch()));
        disconnect(ui.landButton, SIGNAL(clicked()), uas, SLOT(home()));
        disconnect(ui.shutdownButton, SIGNAL(clicked()), uas, SLOT(shutdown()));
        disconnect(ui.modeComboBox, SIGNAL(activated(int)), this, SLOT(setMode(int)));
        disconnect(ui.setModeButton, SIGNAL(clicked()), this, SLOT(transmitMode()));
    }
    else
    {
        // Connect user interface controls
        connect(ui.controlButton, SIGNAL(clicked()), this, SLOT(cycleContextButton()));
        connect(ui.liftoffButton, SIGNAL(clicked()), uas, SLOT(launch()));
        connect(ui.landButton, SIGNAL(clicked()), uas, SLOT(home()));
        connect(ui.shutdownButton, SIGNAL(clicked()), uas, SLOT(shutdown()));
        connect(ui.modeComboBox, SIGNAL(activated(int)), this, SLOT(setMode(int)));
        connect(ui.setModeButton, SIGNAL(clicked()), this, SLOT(transmitMode()));
        ui.modeComboBox->insertItem(0, "Select..");

        ui.controlStatusLabel->setText(tr("Connected to ") + uas->getUASName());

        connect(uas, SIGNAL(modeChanged(int,QString,QString)), this, SLOT(updateMode(int,QString,QString)));
        connect(uas, SIGNAL(statusChanged(int)), this, SLOT(updateState(int)));

        this->uas = uas;
    }
}

UASControlWidget::~UASControlWidget() {

}

void UASControlWidget::updateMode(int uas,QString mode,QString description)
{
    Q_UNUSED(uas);
    Q_UNUSED(mode);
    Q_UNUSED(description);
}

void UASControlWidget::updateState(int state)
{
    switch (state)
    {
    case (int)MAV_STATE_ACTIVE:
        engineOn = true;
        ui.controlButton->setText(tr("Stop Engine"));
        break;
    case (int)MAV_STATE_STANDBY:
        engineOn = false;
        ui.controlButton->setText(tr("Activate Engine"));
        break;
    }
}

void UASControlWidget::setMode(int mode)
{
    // Adapt context button mode
    if (mode == CONTROL_MODE_LOCKED_INDEX)
    {
        uasMode = (unsigned int)MAV_MODE_LOCKED;
        ui.modeComboBox->setCurrentIndex(mode);
    }
    else if (mode == CONTROL_MODE_MANUAL_INDEX)
    {
        uasMode = (unsigned int)MAV_MODE_MANUAL;
        ui.modeComboBox->setCurrentIndex(mode);
    }
    else if (mode == CONTROL_MODE_GUIDED_INDEX)
    {
        uasMode = (unsigned int)MAV_MODE_GUIDED;
        ui.modeComboBox->setCurrentIndex(mode);
    }
    else if (mode == CONTROL_MODE_AUTO_INDEX)
    {
        uasMode = (unsigned int)MAV_MODE_AUTO;
        ui.modeComboBox->setCurrentIndex(mode);
    }
    else if (mode == CONTROL_MODE_TEST1_INDEX)
    {
        uasMode = (unsigned int)MAV_MODE_TEST1;
        ui.modeComboBox->setCurrentIndex(mode);
    }
    else
    {
        qDebug() << "ERROR! MODE NOT FOUND";
        uasMode = 0;
    }


    qDebug() << "SET MODE REQUESTED" << uasMode;
}

void UASControlWidget::transmitMode()
{
    if (uasMode != 0)
    {
        this->uas->setMode(uasMode);
        ui.lastActionLabel->setText(QString("Set new mode for system %1").arg(uas->getUASName()));
    }
}

void UASControlWidget::cycleContextButton()
{
    UAS* mav = dynamic_cast<UAS*>(this->uas);
    if (mav)
    {

        if (engineOn)
        {
            ui.controlButton->setText(tr("Stop Engine"));
            mav->setMode(MAV_MODE_MANUAL);
            mav->enable_motors();
            ui.lastActionLabel->setText(QString("Enabled motors on %1").arg(uas->getUASName()));
        }
        else
        {
            ui.controlButton->setText(tr("Activate Engine"));
            mav->setMode(MAV_MODE_LOCKED);
            mav->disable_motors();
            ui.lastActionLabel->setText(QString("Disabled motors on %1").arg(uas->getUASName()));
        }
            //ui.controlButton->setText(tr("Force Landing"));
            //ui.controlButton->setText(tr("KILL VEHICLE"));
    }

}

