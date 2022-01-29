/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2022                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#include "sgctedit/display.h"
#include "sgctedit/monitorbox.h"
#include "sgctedit/windowcontrol.h"

WindowControl::WindowControl(unsigned int nMonitors, unsigned int monitorIndex,
                          const unsigned int windowIndex, std::vector<QRect>& monitorDims,
                                                const QString* winColors, QWidget *parent)
    : QWidget(parent)
    , _nMonitors(nMonitors)
    , _monIndex(monitorIndex)
    , _index(windowIndex)
    , _monitorResolutions(monitorDims)
    , _colorsForWindows(winColors)
{
    _windowDims = defaultWindowSizes[windowIndex];
    _size_x = new QLineEdit(
        QString::fromUtf8(std::to_string(int(_windowDims.width())).c_str()), parent);
    _size_y = new QLineEdit(
        QString::fromUtf8(std::to_string(int(_windowDims.height())).c_str()), parent);
    _offset_x = new QLineEdit(
        QString::fromUtf8(std::to_string(int(_windowDims.x())).c_str()), parent);
    _offset_y = new QLineEdit(
        QString::fromUtf8(std::to_string(int(_windowDims.y())).c_str()), parent);
    _validatorSize_x = new QIntValidator(10, _maxWindowSizePixels);
    _validatorSize_y = new QIntValidator(10, _maxWindowSizePixels);
    _validatorOffset_x = new QIntValidator(-_maxWindowSizePixels, _maxWindowSizePixels);
    _validatorOffset_y = new QIntValidator(-_maxWindowSizePixels, _maxWindowSizePixels);
    _size_x->setValidator(_validatorSize_x);
    _size_y->setValidator(_validatorSize_y);
    _offset_x->setValidator(_validatorOffset_x);
    _offset_y->setValidator(_validatorOffset_y);

    _comboMonitorSelect = new QComboBox(this);
    _comboMonitorSelect->addItems(_monitorNames);
    _comboMonitorSelect->setCurrentIndex(_monIndex);

    _fullscreenButton = new QPushButton(this);
    _fullscreenButton->setText("Set to Fullscreen");
    _checkBoxWindowDecor = new QCheckBox("Window Decoration", this);
    _checkBoxWindowDecor->setCheckState(Qt::CheckState::Checked);
    _checkBoxWebGui = new QCheckBox("WebGUI only this window", this);
    _checkBoxSpoutOutput = new QCheckBox("Spout Output", this);
    _comboProjection = new QComboBox(this);
    _comboProjection->addItems(_projectionTypes);

    _comboQuality = new QComboBox(this);
    _comboQuality->addItems(_qualityTypes);

    _lineFovH = new QLineEdit("80.0", parent);
    _validatorFovH = new QDoubleValidator(-180.0, 180.0, 10);
    _lineFovH->setValidator(_validatorFovH);
    _lineFovV = new QLineEdit("50.534", parent);
    _validatorFovV = new QDoubleValidator(-90.0, 90.0, 10);
    _lineFovV->setValidator(_validatorFovV);
    _lineHeightOffset = new QLineEdit("0.0", parent);
    _validatorHeightOffset = new QDoubleValidator(-1000000.0, 1000000.0, 12);
    _lineHeightOffset->setValidator(_validatorHeightOffset);

    connect(_size_x, SIGNAL(textChanged(const QString&)), this,
            SLOT(onSizeXChanged(const QString&)));
    connect(_size_y, SIGNAL(textChanged(const QString&)), this,
            SLOT(onSizeYChanged(const QString&)));
    connect(_offset_x, SIGNAL(textChanged(const QString&)), this,
            SLOT(onOffsetXChanged(const QString&)));
    connect(_offset_y, SIGNAL(textChanged(const QString&)), this,
            SLOT(onOffsetYChanged(const QString&)));
    connect(_comboMonitorSelect, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onMonitorChanged(int)));
    connect(_comboProjection, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onProjectionChanged(int)));
    connect(_checkBoxSpoutOutput, SIGNAL(stateChanged(int)),
            this, SLOT(onSpoutSelection(int)));
    connect(_checkBoxWebGui, SIGNAL(stateChanged(int)),
            this, SLOT(onWebGuiSelection(int)));
    connect(_fullscreenButton, SIGNAL(released()), this, SLOT(onFullscreenClicked()));
}

QVBoxLayout* WindowControl::initializeLayout() {
    _layoutFullWindow = new QVBoxLayout();
    //Window size
    _layoutWindowCtrl = new QVBoxLayout();

    _labelWinNum = new QLabel();
    _labelWinNum->setText("Window " + QString::number(_index + 1));
    QString colorStr = "QLabel { color : ";
    colorStr += _colorsForWindows ? _colorsForWindows[_index] : "#FFFFFF";
    colorStr += "; }";
    _labelWinNum->setStyleSheet(colorStr);

    _layoutWinNum = new QHBoxLayout();
    _layoutWinNum->addStretch(1);
    _layoutWinNum->addWidget(_labelWinNum);
    _layoutWinNum->addStretch(1);
    _layoutWindowCtrl->addLayout(_layoutWinNum);

    _layoutName = new QHBoxLayout();
    _labelName = new QLabel(this);
    _labelName->setText("Name: ");
    _windowName = new QLineEdit(this);
    _windowName->setFixedWidth(160);
    _layoutName->addWidget(_labelName);
    _layoutName->addWidget(_windowName);
    _layoutName->addStretch(1);
    _layoutWindowCtrl->addLayout(_layoutName);

    if (_nMonitors > 1) {
        _layoutMonitorNum = new QHBoxLayout();
        _layoutMonitorNum->addWidget(_comboMonitorSelect);
        _layoutMonitorNum->addStretch(1);
        _layoutWindowCtrl->addLayout(_layoutMonitorNum);
    }
    else {
        _comboMonitorSelect->setVisible(false);
    }
    _size_x->setFixedWidth(_lineEditWidthFixed);
    _size_y->setFixedWidth(_lineEditWidthFixed);
    _labelSize = new QLabel(this);
    _labelDelim = new QLabel(this);
    _layoutSize = new QHBoxLayout();
    _layoutSize->addWidget(_labelSize);
    _labelSize->setText("Size:");
    _labelSize->setFixedWidth(55);
    _layoutSize->addWidget(_size_x);
    _layoutSize->addWidget(_labelDelim);
    _layoutSize->addWidget(_size_y);
    _layoutSize->addStretch(1);
    _labelDelim->setText("x");
    _labelDelim->setFixedWidth(9);
    _layoutWindowCtrl->addLayout(_layoutSize);

    //Window offset
    _offset_x->setFixedWidth(_lineEditWidthFixed);
    _offset_y->setFixedWidth(_lineEditWidthFixed);
    _labelOffset = new QLabel(this);
    _labelComma = new QLabel(this);
    _layoutOffset = new QHBoxLayout();
    _layoutOffset->addWidget(_labelOffset);
    _labelOffset->setText("Offset:");
    _labelOffset->setFixedWidth(55);
    _layoutOffset->addWidget(_offset_x);
    _layoutOffset->addWidget(_labelComma);
    _layoutOffset->addWidget(_offset_y);
    _layoutOffset->addStretch(1);
    _labelComma->setText(",");
    _labelComma->setFixedWidth(9);
    _layoutWindowCtrl->addLayout(_layoutOffset);

    //Window options
    _layoutCheckboxesFull1 = new QHBoxLayout();
    _layoutCheckboxesFull2 = new QVBoxLayout();
    _layoutFullscreenButton = new QHBoxLayout();
    _layoutFullscreenButton->addWidget(_fullscreenButton);
    _layoutFullscreenButton->addStretch(1);
    _layoutCheckboxesFull2->addLayout(_layoutFullscreenButton);
    _layoutCBoxWindowDecor = new QHBoxLayout();
    _layoutCBoxWindowDecor->addWidget(_checkBoxWindowDecor);
    _layoutCBoxWindowDecor->addStretch(1);
    _layoutCheckboxesFull2->addLayout(_layoutCBoxWindowDecor);
    _layoutCBoxWebGui= new QHBoxLayout();
    _layoutCBoxWebGui->addWidget(_checkBoxWebGui);
    _layoutCBoxWebGui->addStretch(1);
    _layoutCheckboxesFull2->addLayout(_layoutCBoxWebGui);
    _layoutProjectionGroup = new QVBoxLayout();
    _layoutComboProjection = new QHBoxLayout();
    _layoutComboProjection->addWidget(_comboProjection);
    _layoutComboProjection->addStretch(1);
    _layoutProjectionGroup->addLayout(_layoutComboProjection);
    _borderProjectionGroup = new QFrame;
    _borderProjectionGroup->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    _borderProjectionGroup->setLayout(_layoutProjectionGroup);
    _borderProjectionGroup->setVisible(true);
    _layoutCBoxSpoutOutput= new QHBoxLayout();
    _layoutCBoxSpoutOutput->addWidget(_checkBoxSpoutOutput);
    _layoutCBoxSpoutOutput->addStretch(1);
    _layoutProjectionGroup->addLayout(_layoutCBoxSpoutOutput);
    _layoutComboQuality = new QHBoxLayout();
    _labelQuality = new QLabel();
    _labelQuality->setText("Quality:");
    _layoutComboQuality->addWidget(_labelQuality);
    _layoutComboQuality->addWidget(_comboQuality);
    _layoutComboQuality->addStretch(1);
    _layoutProjectionGroup->addLayout(_layoutComboQuality);
    _layoutFovH = new QHBoxLayout();
    _labelFovH = new QLabel();
    _labelFovH->setText("Horizontal FOV:");
    _layoutFovH->addWidget(_labelFovH);
    _layoutFovH->addWidget(_lineFovH);
    _layoutFovH->addStretch(1);
    _layoutFovV = new QHBoxLayout();
    _labelFovV = new QLabel();
    _labelFovV->setText("Vertical FOV:");
    _layoutFovV->addWidget(_labelFovV);
    _layoutFovV->addWidget(_lineFovV);
    _layoutFovV->addStretch(1);
    _layoutProjectionGroup->addLayout(_layoutFovH);
    _layoutProjectionGroup->addLayout(_layoutFovV);
    _layoutHeightOffset = new QHBoxLayout();
    _labelHeightOffset = new QLabel();
    _labelHeightOffset->setText("Height Offset:");
    _layoutHeightOffset->addWidget(_labelHeightOffset);
    _layoutHeightOffset->addWidget(_lineHeightOffset);
    _layoutHeightOffset->addStretch(1);
    _layoutProjectionGroup->addLayout(_layoutHeightOffset);
    _layoutCheckboxesFull2->addWidget(_borderProjectionGroup);
    _layoutCheckboxesFull1->addLayout(_layoutCheckboxesFull2);
    _layoutCheckboxesFull1->addStretch(1);
    _layoutWindowCtrl->addLayout(_layoutCheckboxesFull1);
    _layoutWindowCtrl->addStretch(1);
    _layoutFullWindow->addLayout(_layoutWindowCtrl);

    _comboProjection->setCurrentIndex(0);
    onProjectionChanged(0);
    _comboQuality->setCurrentIndex(2);

    return _layoutFullWindow;
}

void WindowControl::showWindowLabel(const bool show) {
    _labelWinNum->setVisible(show);
}

void WindowControl::cleanupLayouts() {
    int labelSize1 = _labelSize->width();
    int labelSize2 = _labelOffset->width();
    int labelWidthStandard = std::max(labelSize1, labelSize2);
    _labelSize->setFixedWidth(labelWidthStandard);
    _labelOffset->setFixedWidth(labelWidthStandard);
}

void WindowControl::onSizeXChanged(const QString& newText) {
    std::string x = newText.toStdString();
    if (!x.empty()) {
        _windowDims.setWidth(std::stoi(x));
    }
    if (_windowChangeCallback) {
        _windowChangeCallback(_monIndex, _index, _windowDims);
    }
}

void WindowControl::onSizeYChanged(const QString& newText) {
    std::string y = newText.toStdString();
    if (!y.empty()) {
        _windowDims.setHeight(std::stoi(y));
    }
    if (_windowChangeCallback) {
        _windowChangeCallback(_monIndex, _index, _windowDims);
    }
}

void WindowControl::onOffsetXChanged(const QString& newText) {
    std::string xOffset = newText.toStdString();
    float prevWidth = _windowDims.width();
    try {
        if (!xOffset.empty()) {
            _windowDims.setX(std::stoi(xOffset));
            _windowDims.setWidth(prevWidth);
        }
        if (_windowChangeCallback) {
            _windowChangeCallback(_monIndex, _index, _windowDims);
        }
    }
    catch (...) {
        //The QIntValidator ensures that the range is a +/- integer
        //However, it's possible to enter only a - character which
        //causes an exception throw, which is ignored here (when user
        //enters an integer after the - then the value will be updated).
    }
}

void WindowControl::onOffsetYChanged(const QString& newText) {
    std::string yOffset = newText.toStdString();
    float prevHeight = _windowDims.height();
    try {
        if (!yOffset.empty()) {
            _windowDims.setY(std::stoi(yOffset));
            _windowDims.setHeight(prevHeight);
        }
        if (_windowChangeCallback) {
            _windowChangeCallback(_monIndex, _index, _windowDims);
        }
    }
    catch (...) {
        //See comment in onOffsetXChanged
    }
}

void WindowControl::onFullscreenClicked() {
    _offset_x->setText("0");
    _offset_y->setText("0");
    _size_x->setText(QString::number(_monitorResolutions[_monIndex].width()));
    _size_y->setText(QString::number(_monitorResolutions[_monIndex].height()));
    _checkBoxWindowDecor->setCheckState(Qt::Unchecked);
}

void WindowControl::enableGuiWindowSelection(bool enabled) {
    _checkBoxWebGui->setEnabled(enabled);
}

void WindowControl::onWebGuiSelection(int selectionState) {
    if (_windowGuiCheckCallback && (selectionState == Qt::Checked)) {
        _windowGuiCheckCallback(_index);
    }
}

void WindowControl::onSpoutSelection(int selectionState) {
    if (selectionState == Qt::Checked) {
        int currentProjectionSelection = _comboProjection->currentIndex();
        if ((currentProjectionSelection != ProjectionIndeces::Equirectangular) &&
            (currentProjectionSelection != ProjectionIndeces::Fisheye))
        {
            _comboProjection->setCurrentIndex(ProjectionIndeces::Equirectangular);
        }
    }
}

template <typename T>
void WindowControl::enableProjectionOption(T* comboModel, int selectionIndex, bool enable)
{
    auto* item = comboModel->item(selectionIndex);
    if (item) {
        item->setEnabled(enable);
    }
}

void WindowControl::onMonitorChanged(int newSelection) {
    _monIndex = newSelection;
    if (_windowChangeCallback) {
        _windowChangeCallback(_monIndex, _index, _windowDims);
    }
}

void WindowControl::onProjectionChanged(int newSelection) {
    _comboQuality->setVisible(newSelection != ProjectionIndeces::Planar);
    _labelQuality->setVisible(newSelection != ProjectionIndeces::Planar);
    _labelFovH->setVisible(newSelection == ProjectionIndeces::Planar);
    _lineFovH->setVisible(newSelection == ProjectionIndeces::Planar);
    _labelFovV->setVisible(newSelection == ProjectionIndeces::Planar);
    _lineFovV->setVisible(newSelection == ProjectionIndeces::Planar);
    _labelHeightOffset->setVisible(newSelection == ProjectionIndeces::Cylindrical);
    _lineHeightOffset->setVisible(newSelection == ProjectionIndeces::Cylindrical);
    _checkBoxSpoutOutput->setVisible(newSelection == ProjectionIndeces::Fisheye
        || newSelection == ProjectionIndeces::Equirectangular);
}

void WindowControl::setDimensions(const QRectF& dimensions) {
    _windowDims = dimensions;
}

void WindowControl::setWindowChangeCallback(
                        std::function<void(int, int, const QRectF&)> cb)
{
    _windowChangeCallback = cb;
}

void WindowControl::setWebGuiChangeCallback(
                        std::function<void(unsigned int)> cb)
{
    _windowGuiCheckCallback = cb;
}

void WindowControl::uncheckWebGuiOption() {
    _checkBoxWebGui->setCheckState(Qt::Unchecked);
}

QRectF& WindowControl::dimensions() {
    return _windowDims;
}

QLineEdit* WindowControl::lineEditSizeWidth() {
    return _size_x;
}

QLineEdit* WindowControl::lineEditSizeHeight() {
    return _size_y;
}

QLineEdit* WindowControl::lineEditSizeOffsetX() {
    return _offset_x;
}

QLineEdit* WindowControl::lineEditSizeOffsetY() {
    return _offset_y;
}

QCheckBox* WindowControl::checkBoxWindowDecor() {
    return _checkBoxWindowDecor;
}

QCheckBox* WindowControl::checkBoxWebGui() {
    return _checkBoxWebGui;
}

QCheckBox* WindowControl::checkBoxSpoutOutput() {
    return _checkBoxSpoutOutput;
}

std::string WindowControl::windowName() {
    return _windowName->text().toStdString();
}

sgct::ivec2 WindowControl::windowSize() {
    return {
        stoi(_size_x->text().toStdString()),
        stoi(_size_y->text().toStdString())
    };
}

sgct::ivec2 WindowControl::windowPos() {
    return {
        stoi(_offset_x->text().toStdString()),
        stoi(_offset_y->text().toStdString())
    };
}

bool WindowControl::isDecorated() {
    return (_checkBoxWindowDecor->checkState() == Qt::Checked);
}

bool WindowControl::isGuiWindow() {
    return (_checkBoxWebGui->checkState() == Qt::Checked);
}

bool WindowControl::isSpoutSelected() {
    return (_checkBoxSpoutOutput->checkState() == Qt::Checked);
}

int WindowControl::projectionSelectedIndex() {
    return _comboProjection->currentIndex();
}

int WindowControl::qualitySelectedIndex() {
    return _comboQuality->currentIndex();
}

int WindowControl::qualitySelectedValue() {
    return QualityValues[_comboQuality->currentIndex()];
}

float WindowControl::fovH() {
    return _lineFovH->text().toFloat();
}

float WindowControl::fovV() {
    return _lineFovV->text().toFloat();
}

float WindowControl::heightOffset() {
    return _lineHeightOffset->text().toFloat();
}

unsigned int WindowControl::monitorNum() {
    return _monIndex;
}

WindowControl::~WindowControl()
{
    delete _size_x;
    delete _size_y;
    delete _validatorSize_x;
    delete _validatorSize_y;
    delete _offset_x;
    delete _offset_y;
    delete _validatorOffset_x;
    delete _validatorOffset_y;
    delete _layoutName;
    delete _labelName;
    delete _windowName;
    delete _labelWinNum;
    delete _labelSize;
    delete _labelDelim;
    delete _layoutSize;
    delete _labelOffset;
    delete _labelComma;
    delete _layoutOffset;
    delete _checkBoxWindowDecor;
    delete _checkBoxWebGui;
    delete _checkBoxSpoutOutput;
    delete _comboProjection;
    delete _comboQuality;
    delete _fullscreenButton;
    delete _labelFovH;
    delete _lineFovH;
    delete _validatorFovH;
    delete _labelFovV;
    delete _lineFovV;
    delete _validatorFovV;
    delete _labelHeightOffset;
    delete _lineHeightOffset;
    delete _validatorHeightOffset;
    delete _labelQuality;
    delete _layoutFullscreenButton;
    delete _layoutCBoxWindowDecor;
    delete _layoutCBoxWebGui;
    delete _layoutCBoxSpoutOutput;
    delete _layoutComboProjection;
    delete _layoutComboQuality;
    delete _layoutFovH;
    delete _layoutFovV;
    delete _layoutHeightOffset;
    delete _layoutProjectionGroup;
    delete _borderProjectionGroup;
    delete _layoutCheckboxesFull2;
    delete _layoutCheckboxesFull1;
    delete _layoutMonitorNum;
    delete _layoutWinNum;
    delete _layoutWindowCtrl;
    delete _layoutFullWindow;
}

