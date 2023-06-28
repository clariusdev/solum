#pragma once

#include <QPushButton>

/// Custom wrapper around QPushButton with two separate labels for ready and
/// running states. The running state is automatically entered when clicking
/// the button.
class JobButton : public QPushButton
{
    Q_OBJECT

    Q_PROPERTY(QString readyText MEMBER readyText_)
    Q_PROPERTY(QString runningText MEMBER runningText_)

public:
    explicit JobButton(QWidget* parent = nullptr);

    void setLabels(const QString& readyText, const QString& runningText);

    /// Returns the button to the ready state.
    void ready();

private:
    QString readyText_;
    QString runningText_;
};
