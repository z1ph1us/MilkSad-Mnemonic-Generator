#include "helpers.h"
#include "constants.h"
#include "MainWindow.h"
#include "WorkerThread.h"
#include <QMessageBox>
#include <QDate>
#include <QDateTime>
#include <QDebug> // For qWarning, qInfo if you want to use them
#include <QPainter> // For background image transparency
#include <QPixmap> // For background image

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Milk Sad Generator - by z1ph1us");
    setMinimumSize(600, 500);

    QWidget *centralWidget = new QWidget(this);
    QGridLayout *gridLayout = new QGridLayout(centralWidget);
    gridLayout->setContentsMargins(0, 0, 0, 0); // Remove margins for background

    // --- Background Image Setup with Half Transparency ---
    QLabel *backgroundLabel = new QLabel(this);
    QPixmap originalImage("Pic/1_0LOPQwRdahE_ABkF8idXgg.png");

    if (originalImage.isNull()) {
        qWarning() << "Failed to load background image: Pic/1_0LOPQwRdahE_ABkF8idXgg.png";
        centralWidget->setStyleSheet("background-color: #f0f0f0; color: black;"); // Light fallback background
    } else {
        QPixmap transparentImage(originalImage.size());
        transparentImage.fill(Qt::transparent);
        QPainter painter(&transparentImage);
        painter.setOpacity(0.5); // 50% transparent
        painter.drawPixmap(0, 0, originalImage);
        painter.end();

        backgroundLabel->setPixmap(transparentImage.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        backgroundLabel->setScaledContents(true);
        gridLayout->addWidget(backgroundLabel, 0, 0, 1, 1);
        gridLayout->setRowStretch(0, 1);
        gridLayout->setColumnStretch(0, 1);
    }

    // --- Foreground Layout ---
    QVBoxLayout *foregroundLayout = new QVBoxLayout();
    foregroundLayout->setContentsMargins(10, 10, 10, 10);

    // --- Help Button in Top Right Corner ---
    QPushButton *helpButton = new QPushButton("?", this);
    helpButton->setFixedSize(30, 30); // Small, circular-ish button
    helpButton->setStyleSheet(
        "QPushButton {"
        "   background-color: rgba(200, 200, 200, 180);"
        "   color: black;"
        "   border-radius: 15px;" // Makes it circular
        "   border: 1px solid #777;"
        "   font-weight: normal;" // Ensure normal weight
        "}"
        "QPushButton:hover {"
        "   background-color: #ccc;"
        "}"
    );
    QHBoxLayout *topBarLayout = new QHBoxLayout();
    topBarLayout->addStretch(); // Pushes button to the right
    topBarLayout->addWidget(helpButton);
    foregroundLayout->addLayout(topBarLayout); // Add to the main foreground layout

    // --- Create a QFont object for bolding headers ---
    QFont boldFont;
    boldFont.setBold(true);
    // Note: The base font for the application is set in main(), so we don't set family here
    // If you wanted a different font size for the header, you could set it here, e.g., boldFont.setPointSize(12);


    // Wordlist selection
    QGroupBox *wordlistGroup = new QGroupBox("Wordlist Selection", this);
    wordlistGroup->setFont(boldFont); // Apply bold font directly to the QGroupBox
    wordlistGroup->setStyleSheet(
        "QGroupBox {"
        "   background-color: rgba(255, 255, 255, 120);" // Semi-transparent white
        "   border: 1px solid #bbb;"
        "   border-radius: 5px;"
        "   margin-top: 2ex;" // Space for title
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top left;"
        "   padding: 0 3px;"
        "   color: black;"
        "}" // Removed font-weight: bold; from here as font is set by setFont()
    );
    QVBoxLayout *wordlistLayout = new QVBoxLayout(wordlistGroup);

    m_wordlistCombo = new QComboBox(this);
    m_wordlistCombo->setStyleSheet(
        "QComboBox {"
        "   background-color: rgba(255, 255, 255, 200);" // More opaque white
        "   color: black;"
        "   border: 1px solid #888;"
        "   border-radius: 3px;"
        "   padding: 5px;"
        "   font-weight: normal;" // Ensure normal weight, Open Sans applied by app font
        "}"
        "QComboBox::drop-down {" // Styling the dropdown arrow part
        "   subcontrol-origin: padding;"
        "   subcontrol-position: top right;"
        "   width: 20px;"
        "   border-left: 1px solid #888;"
        "}"
        "QComboBox::down-arrow {" // This is the actual arrow icon
        "   image: url(:/qt-project.org/styles/commonstyle/images/down-arrow.png);" // Default Qt arrow icon
        "   width: 12px;"
        "   height: 12px;"
        "}"
        "QComboBox QAbstractItemView {" // Style for the dropdown list itself
        "   background-color: rgba(240, 240, 240, 220);" // Light gray, slightly transparent
        "   color: black;"
        "   selection-background-color: #4CAF50;" // Green selection
        "   selection-color: white;"
        "   border: 1px solid #888;"
        "   font-weight: normal;" // Ensure normal weight, Open Sans applied by app font
        "}"
    );
    for (const auto& wordlist : WORDLIST_FILES) {
        m_wordlistCombo->addItem(QString::fromStdString(wordlist));
    }
    wordlistLayout->addWidget(m_wordlistCombo);
    foregroundLayout->addWidget(wordlistGroup);

    // Generation options
    QGroupBox *optionsGroup = new QGroupBox("Generation Options", this);
    optionsGroup->setFont(boldFont); // Apply bold font directly to the QGroupBox
    optionsGroup->setStyleSheet(
        "QGroupBox {"
        "   background-color: rgba(255, 255, 255, 120);"
        "   border: 1px solid #bbb;"
        "   border-radius: 5px;"
        "   margin-top: 2ex;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top left;"
        "   padding: 0 3px;"
        "   color: black;"
        "}" // Removed font-weight: bold; from here
        "QRadioButton { color: black; padding: 3px; font-weight: normal;}" // Ensure normal weight, Open Sans applied by app font
    );
    QVBoxLayout *optionsLayout = new QVBoxLayout(optionsGroup);

    m_singleRadio = new QRadioButton("Single Mnemonic for Specific Date/Time", this);
    m_rangeRadio = new QRadioButton("Mnemonics for Date Range", this);
    m_fullRadio = new QRadioButton("Full Unix Timestamp Range (Warning: Large Output)", this);

    m_singleRadio->setChecked(true);

    optionsLayout->addWidget(m_singleRadio);
    optionsLayout->addWidget(m_rangeRadio);
    optionsLayout->addWidget(m_fullRadio);
    foregroundLayout->addWidget(optionsGroup);

    // Date/time inputs
    QGroupBox *dateGroup = new QGroupBox("Date/Time Parameters", this);
    dateGroup->setFont(boldFont); // Apply bold font directly to the QGroupBox
    dateGroup->setStyleSheet(
        "QGroupBox {"
        "   background-color: rgba(255, 255, 255, 120);"
        "   border: 1px solid #bbb;"
        "   border-radius: 5px;"
        "   margin-top: 2ex;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top left;"
        "   padding: 0 3px;"
        "   color: black;"
        "}" // Removed font-weight: bold; from here
        "QLabel { color: black; font-weight: normal; }" // Ensure normal weight, Open Sans applied by app font
        "QDateEdit, QDateTimeEdit {"
        "   background-color: rgba(255, 255, 255, 200);" // Opaque white background
        "   color: black;"
        "   border: 1px solid #888;"
        "   border-radius: 3px;"
        "   padding: 3px;"
        "   font-weight: normal;" // Ensure normal weight, Open Sans applied by app font
        "}"
        "QDateEdit::drop-down, QDateTimeEdit::drop-down {"
        "   subcontrol-origin: padding;"
        "   subcontrol-position: top right;"
        "   width: 20px;"
        "   border-left: 1px solid #888;"
        "}"
        "QDateEdit::down-arrow, QDateTimeEdit::down-arrow {"
        "   image: url(:/qt-project.org/styles/commonstyle/images/down-arrow.png);"
        "   width: 12px;"
        "   height: 12px;"
        "}"
    );
    QGridLayout *dateLayout = new QGridLayout(dateGroup);

    m_singleDateTimeLabel = new QLabel("Date/Time (YYYY-MM-DD HH:MM:SS):", this);
    m_singleDateTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    m_singleDateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    dateLayout->addWidget(m_singleDateTimeLabel, 0, 0);
    dateLayout->addWidget(m_singleDateTimeEdit, 0, 1);

    m_startDateLabel = new QLabel("Start Date:", this);
    m_startDateEdit = new QDateEdit(QDate::currentDate(), this);
    m_startDateEdit->setDisplayFormat("yyyy-MM-dd");
    m_endDateLabel = new QLabel("End Date:", this);
    m_endDateEdit = new QDateEdit(QDate::currentDate(), this);
    m_endDateEdit->setDisplayFormat("yyyy-MM-dd");

    dateLayout->addWidget(m_startDateLabel, 1, 0);
    dateLayout->addWidget(m_startDateEdit, 1, 1);
    dateLayout->addWidget(m_endDateLabel, 2, 0);
    dateLayout->addWidget(m_endDateEdit, 2, 1);

    foregroundLayout->addWidget(dateGroup);

    // Output display (Log Window)
    m_outputText = new QTextEdit(this);
    m_outputText->setReadOnly(true);
    m_outputText->setStyleSheet(
        "QTextEdit {"
        "   background-color: rgba(255, 255, 255, 150);" // Semi-transparent white
        "   color: black;" // Black text
        "   border: 1px solid #888;"
        "   border-radius: 5px;"
        "   padding: 5px;"
        "   font-weight: normal;" // Ensure normal weight, Open Sans applied by app font
        "}"
    );
    foregroundLayout->addWidget(m_outputText);

    // Status label
    m_statusLabel = new QLabel("Ready", this);
    m_statusLabel->setStyleSheet("QLabel { color: black; font-weight: normal; padding: 5px; }"); // Ensure normal weight, Open Sans applied by app font
    foregroundLayout->addWidget(m_statusLabel);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_generateButton = new QPushButton("Generate", this);
    m_generateButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #4CAF50;" // Green
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   padding: 10px 20px;"
        "   font-weight: normal;" // Ensure normal weight, Open Sans applied by app font
        "}"
        "QPushButton:hover { background-color: #45a049; }" // Darker green on hover
        "QPushButton:pressed { background-color: #3e8e41; }"
    );
    m_stopButton = new QPushButton("Stop", this);
    m_stopButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #f44336;" // Red
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   padding: 10px 20px;"
        "   font-weight: normal;" // Ensure normal weight, Open Sans applied by app font
        "}"
        "QPushButton:hover { background-color: #da190b; }" // Darker red on hover
        "QPushButton:pressed { background-color: #c40c00; }"
    );
    m_stopButton->setEnabled(false);
    buttonLayout->addWidget(m_generateButton);
    buttonLayout->addWidget(m_stopButton);
    foregroundLayout->addLayout(buttonLayout);

    // Add the foreground layout to the grid
    gridLayout->addLayout(foregroundLayout, 0, 0, 1, 1);

    setCentralWidget(centralWidget);

    // Connect signals and slots
    connect(m_singleRadio, &QRadioButton::toggled, this, &MainWindow::updateUi);
    connect(m_rangeRadio, &QRadioButton::toggled, this, &MainWindow::updateUi); // Connect range radio button
    connect(m_fullRadio, &QRadioButton::toggled, this, &MainWindow::updateUi); // Connect full radio button
    connect(m_generateButton, &QPushButton::clicked, this, &MainWindow::startGeneration);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::stopGeneration);
    connect(helpButton, &QPushButton::clicked, this, &MainWindow::showHelp); // Connect help button

    // Set initial UI state
    updateUi();
}

void MainWindow::updateUi() {
    bool singleSelected = m_singleRadio->isChecked();
    bool rangeSelected = m_rangeRadio->isChecked();

    m_singleDateTimeLabel->setVisible(singleSelected);
    m_singleDateTimeEdit->setVisible(singleSelected);

    m_startDateLabel->setVisible(rangeSelected);
    m_startDateEdit->setVisible(rangeSelected);
    m_endDateLabel->setVisible(rangeSelected);
    m_endDateEdit->setVisible(rangeSelected);
}

void MainWindow::startGeneration() {
    try {
        int wordlistIndex = m_wordlistCombo->currentIndex();
        if (wordlistIndex < 0 || wordlistIndex >= static_cast<int>(WORDLIST_FILES.size())) {
            QMessageBox::critical(this, "Error", "Invalid wordlist selection");
            return;
        }

        std::string wordlist_name = WORDLIST_FILES[wordlistIndex];
        std::string wordlist_base = get_filename_base(wordlist_name);

        m_outputText->append("Loading " + QString::fromStdString(wordlist_name) + " wordlist...");
        m_statusLabel->setText("Loading wordlist...");
        std::vector<std::string> wordlist = load_wordlist(wordlist_name);
        m_outputText->append("Wordlist loaded successfully.");
        m_statusLabel->setText("Wordlist loaded.");

        // Clean up previous thread if it exists
        if (m_workerThread && m_workerThread->isRunning()) {
            m_workerThread->stop();
            m_workerThread->wait(100); // Wait a bit for it to stop cleanly
            if (m_workerThread->isRunning()) { // If it still hasn't stopped
                m_workerThread->terminate(); // Force terminate
                m_workerThread->wait();
            }
            m_workerThread->deleteLater();
            m_workerThread = nullptr;
        }

        m_workerThread = new WorkerThread(this);

        if (m_singleRadio->isChecked()) {
            QDateTime dateTime = m_singleDateTimeEdit->dateTime();
            uint32_t timestamp = get_unix_timestamp(
                dateTime.date().year(), dateTime.date().month(), dateTime.date().day(),
                dateTime.time().hour(), dateTime.time().minute(), dateTime.time().second()
            );

            m_outputText->append("Generating mnemonic for timestamp: " + QString::number(timestamp));
            m_statusLabel->setText("Generating single mnemonic...");
            m_workerThread->setup(WorkerThread::SINGLE, wordlist, wordlist_base, timestamp);
        } else if (m_rangeRadio->isChecked()) {
            QDate startDate = m_startDateEdit->date();
            QDate endDate = m_endDateEdit->date();

            if (startDate > endDate) {
                QMessageBox::critical(this, "Error", "Start date must be before or equal to end date");
                return;
            }

            uint32_t start_timestamp = get_unix_timestamp(
                startDate.year(), startDate.month(), startDate.day(), 0, 0, 0);
            uint32_t end_timestamp = get_unix_timestamp(
                endDate.year(), endDate.month(), endDate.day(), 0, 0, 0);

            m_outputText->append("Generating mnemonics for timestamps between " +
                                 QString::number(start_timestamp) + " and " +
                                 QString::number(end_timestamp));
            m_statusLabel->setText("Generating mnemonics for range...");
            m_workerThread->setup(WorkerThread::RANGE, wordlist, wordlist_base, start_timestamp, end_timestamp);
        } else if (m_fullRadio->isChecked()) {
            if (QMessageBox::question(this, "Warning",
                                     "This will generate mnemonics for the full 32-bit Unix timestamp range.\n"
                                     "The output file could be hundreds of GB in size.\n"
                                     "Are you sure you want to continue?",
                                     QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
                return;
            }

            m_outputText->append("Generating mnemonics for the full 32-bit Unix timestamp range.");
            m_statusLabel->setText("Generating full range mnemonics (long process)...");
            m_workerThread->setup(WorkerThread::FULL, wordlist, wordlist_base);
        }

        connect(m_workerThread, &WorkerThread::generationComplete, this, &MainWindow::handleGenerationComplete);
        connect(m_workerThread, &WorkerThread::errorOccurred, this, &MainWindow::handleError);
        connect(m_workerThread, &WorkerThread::singleMnemonicGenerated, this, &MainWindow::handleSingleMnemonic);
        connect(m_workerThread, &WorkerThread::statusUpdated, m_statusLabel, &QLabel::setText);
        connect(m_workerThread, &WorkerThread::finished, m_workerThread, &QObject::deleteLater); // Clean up thread object

        m_generateButton->setEnabled(false);
        m_stopButton->setEnabled(true);
        m_workerThread->start();

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Error: ") + e.what());
        m_statusLabel->setText("Error: " + QString(e.what()));
        m_generateButton->setEnabled(true);
        m_stopButton->setEnabled(false);
        // Ensure worker thread is cleaned up on error too
        if (m_workerThread) {
            m_workerThread->deleteLater();
            m_workerThread = nullptr;
        }
    }
}

void MainWindow::stopGeneration() {
    if (m_workerThread && m_workerThread->isRunning()) {
        m_workerThread->stop();
        m_outputText->append("Stopping generation...");
        m_statusLabel->setText("Stopping...");
    }
}

void MainWindow::handleGenerationComplete(const QString& message) {
    m_outputText->append(message);
    m_statusLabel->setText("Complete!");
    m_generateButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    // m_workerThread is cleaned up by the connect(m_workerThread, &QThread::finished, ...)
    m_workerThread = nullptr;
}

void MainWindow::handleError(const QString& message) {
    m_outputText->append(message);
    m_statusLabel->setText("Error!");
    m_generateButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    // m_workerThread is cleaned up by the connect(m_workerThread, &QThread::finished, ...)
    m_workerThread = nullptr;
}

void MainWindow::handleSingleMnemonic(const QString& mnemonic) {
    m_outputText->append("Generated mnemonic:\n" + mnemonic);
}

void MainWindow::showHelp() {
    QMessageBox::information(this, "How to Use",
        "<h3>Milk Sad Mnemonic Generator Usage:</h3>"
        "<p>1. <b>Select Wordlist:</b> Choose your desired language from the dropdown menu.</p>"
        "<p>2. <b>Choose Generation Mode:</b></p>"
        "<ul>"
        "<li><b>Single Mnemonic:</b> Generates one mnemonic for a specific date/time (Unix timestamp).</li>"
        "<li><b>Mnemonics for Date Range:</b> Generates mnemonics for every second within a specified date range.</li>"
        "<li><b>Full Unix Timestamp Range:</b> Generates mnemonics for the entire 32-bit Unix timestamp range (EXTREMELY LARGE OUTPUT!).</li>"
        "</ul>"
        "<p>3. <b>Set Parameters:</b> Enter the required date/time or date range based on your selected mode.</p>"
        "<p>4. <b>Generate:</b> Click the 'Generate' button to start. The output will appear in the log window.</p>"
        "<p>5. <b>Stop:</b> Use the 'Stop' button to halt ongoing generation.</p>"
        "<br>"
        "<p><i>Note: Generated mnemonics are saved to text files in the application directory.</i></p>"
        "<p>Made by z1ph1us</p>"
    );
}
