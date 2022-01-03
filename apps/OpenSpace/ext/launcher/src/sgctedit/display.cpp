#include <QApplication>
#include <QMainWindow>
#include <string>

#include "include/monitorbox.h"
#include "include/windowcontrol.h"
#include "include/display.h"


Display::Display()
{
    _toggleNumMonitorsButton = new QPushButton("Add 2nd Window", this);
    _toggleNumMonitorsButton->setObjectName("toggleNumMonitors");

    _monBox = new MonitorBox(_widgetDims, _monitorRes, this);
    //Add 2 window controls
    addWindowControl();
    addWindowControl();
    initializeLayout();

    connect(_toggleNumMonitorsButton, SIGNAL(released()), this,
            SLOT(toggleWindows()));
}

Display::~Display() {
    delete _toggleNumMonitorsButton;
    delete _monBox;
    delete _layoutMonBox;
    delete _layoutMonButton;
    delete _layoutWindows;
    delete _layout;
}

void Display::initializeLayout() {
    _layout = new QVBoxLayout(this);
    _layoutMonBox = new QHBoxLayout(this);
    _layoutMonBox->addStretch(1);
    //_layout->addWidget(_monBox);
    _layoutMonBox->addWidget(_monBox);
    _layoutMonBox->addStretch(1);
    _layout->addLayout(_layoutMonBox);
    _monBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _monBox->setFixedSize(400, 400);
    _layoutMonButton = new QHBoxLayout(this);
    _layoutMonButton->addStretch(1);
    _layoutMonButton->addWidget(_toggleNumMonitorsButton);
    _layoutMonButton->addStretch(1);
    _layout->addLayout(_layoutMonButton);
    _layoutWindows = new QHBoxLayout(this);

    _winCtrlLayouts.push_back(_windowControl[0]->initializeLayout(this));
    _layoutWindowWrappers.push_back(new QWidget());
    _layoutWindowWrappers.back()->setLayout(_winCtrlLayouts.back());
    _layoutWindows->addWidget(_layoutWindowWrappers.back());
    _borderFrame = new QFrame;
    _borderFrame->setFrameShape(QFrame::VLine);
    _layoutWindows->addWidget(_borderFrame);
    _winCtrlLayouts.push_back(_windowControl[1]->initializeLayout(this));
    _layoutWindowWrappers.push_back(new QWidget());
    _layoutWindowWrappers.back()->setLayout(_winCtrlLayouts.back());
    _layoutWindows->addWidget(_layoutWindowWrappers.back());
    hideSecondWindow();
    _layout->addLayout(_layoutWindows);

    this->setLayout(_layout);

    QRect defaultMonitorResolution(_monitorResolution[0], _monitorResolution[1], 0, 0);
    _monBox->setResolution(defaultMonitorResolution);

    for (WindowControl* w : _windowControl) {
        w->cleanupLayouts();
    }
}

void Display::toggleWindows() {
    if (_nWindowsDisplayed == 1) {
        _toggleNumMonitorsButton->setText("Remove 2nd window");
        showSecondWindow();
    }
    else if (_nWindowsDisplayed == 2) {
        _toggleNumMonitorsButton->setText("Add 2nd window");
        hideSecondWindow();
        int minWidth = minimumWidth();
    }
}

void Display::hideSecondWindow() {
    _borderFrame->setVisible(false);
    _layoutWindowWrappers[1]->setVisible(false);
    _nWindowsDisplayed = 1;
    _monBox->setNumWindowsDisplayed(_nWindowsDisplayed);
}

void Display::showSecondWindow() {
    _borderFrame->setVisible(true);
    _layoutWindowWrappers[1]->setVisible(true);
    _nWindowsDisplayed = 2;
    _monBox->setNumWindowsDisplayed(_nWindowsDisplayed);
}

void Display::addWindowControl() {
    if (_nWindowsAllocated < 2) {
        _windowControl.push_back(
            new WindowControl(
                _nWindowsAllocated,
                _widgetDims,
                _monitorRes,
                this
            )
        );
        _windowControl.back()->setWindowChangeCallback(
            [this](unsigned int windowIndex, const QRectF& newDims) {
                _monBox->windowDimensionsChanged(windowIndex, newDims);
            }
        );
        _monBox->mapWindowResolutionToWidgetCoordinates(_nWindowsAllocated,
            _windowControl.back()->dimensions());
        _nWindowsAllocated++;
    }
}
