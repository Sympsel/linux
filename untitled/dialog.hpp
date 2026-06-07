#pragma once

#include <QDialog>

class Dialog : public QDialog {
    Q_OBJECT
public:
    explicit Dialog(QWidget* parent = nullptr) : QDialog(parent) {

    }
};