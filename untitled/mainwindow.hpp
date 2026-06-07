#pragma once

#include <QMainWindow>
#include <QDebug>
#include <QTimerEvent>
#include <QMenuBar>
#include <QPlainTextEdit>
#include <QFileDialog>
#include <QStatusBar>


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr) {
        this->setWindowTitle("你好");
        this->resize(800, 600);

        QMenuBar* menuBar = this->menuBar();

        QMenu* fileMenu = menuBar->addMenu("文件");

        QAction* openAction = fileMenu->addAction("打开");
        QAction* saveAction = fileMenu->addAction("保存");

        QPlainTextEdit* edit = new QPlainTextEdit(this);
        QFont font;
        font.setPixelSize(20);
        edit->setFont(font);
        this->setCentralWidget(edit);

        connect(openAction, &QAction::triggered, [this, edit]() {
            const auto path = QFileDialog::getOpenFileName(this);
            QStatusBar* statusBar = this->statusBar();
            statusBar->showMessage(path);

            QFile file(path);
            if (!file.open(QFile::ReadOnly)) {
                statusBar->showMessage(path + "打开失败");
                return;
            }
            const QString text = file.readAll();
            edit->setPlainText(text);
            file.close();
        });

        connect(saveAction, &QAction::triggered, [this, edit]() {
            const auto path = QFileDialog::getSaveFileName(this);
            QStatusBar *statusBar = this->statusBar();
            statusBar->showMessage(path);

            QFile file(path);
            if (!file.open(QFile::WriteOnly)) {
                statusBar->showMessage(path + "打开失败");
                return;
            }
            const QString& text = edit->toPlainText();
            file.write(text.toUtf8());

            file.close();
        });
    }

    ~MainWindow() override = default;

protected:
    void timerEvent(QTimerEvent *event) override {
        if (event->timerId() != this->timerId) return;
        qDebug() << "滴答";
    }

private:
    int timerId;
};
