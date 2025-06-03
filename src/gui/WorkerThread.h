#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QThread>
#include <QMutex>
#include <QString>
#include <vector>
#include <string>

class WorkerThread : public QThread {
    Q_OBJECT
public:
    enum GenerationMode {
        SINGLE,
        RANGE,
        FULL
    };

    WorkerThread(QObject *parent = nullptr);

    void setup(GenerationMode mode, const std::vector<std::string>& wordlist,
               const std::string& wordlist_base, uint32_t start_ts = 0, uint32_t end_ts = 0);

    void stop();

signals:
    void generationComplete(const QString& message);
    void errorOccurred(const QString& message);
    void singleMnemonicGenerated(const QString& mnemonic);
    void statusUpdated(const QString& status);

protected:
    void run() override;

private:
    GenerationMode m_mode;
    std::vector<std::string> m_wordlist;
    std::string m_wordlist_base;
    uint32_t m_start_timestamp;
    uint32_t m_end_timestamp;
    bool m_stopRequested;
    QMutex m_mutex;
};

#endif // WORKERTHREAD_H
