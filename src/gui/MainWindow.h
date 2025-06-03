#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QRadioButton>
#include <QLabel>
#include <QDateTimeEdit>
#include <QDateEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QMessageBox> // For QMessageBox
#include <QGroupBox>   // NEW: For QGroupBox
#include <QGridLayout> // NEW: For QGridLayout
#include <QVBoxLayout> // NEW: For QVBoxLayout
#include <QHBoxLayout> // NEW: For QHBoxLayout

// Forward declaration for WorkerThread if not included directly in .h (preferred for dependency)
// #include "WorkerThread.h" // If you need full WorkerThread definition in MainWindow.h
class WorkerThread; // Forward declaration if only pointer is used in .h

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void updateUi();
    void startGeneration();
    void stopGeneration();
    void handleGenerationComplete(const QString& message);
    void handleError(const QString& message);
    void handleSingleMnemonic(const QString& mnemonic);
    void showHelp();

private:
    QComboBox *m_wordlistCombo;
    QRadioButton *m_singleRadio;
    QRadioButton *m_rangeRadio;
    QRadioButton *m_fullRadio;
    QLabel *m_singleDateTimeLabel;
    QDateTimeEdit *m_singleDateTimeEdit;
    QLabel *m_startDateLabel;
    QDateEdit *m_startDateEdit;
    QLabel *m_endDateLabel;
    QDateEdit *m_endDateEdit;
    QTextEdit *m_outputText;
    QLabel *m_statusLabel;
    QPushButton *m_generateButton;
    QPushButton *m_stopButton;
    WorkerThread *m_workerThread = nullptr;
};

#endif // MAINWINDOW_H