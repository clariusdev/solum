#include "jobbutton.h"

JobButton::JobButton(QWidget* parent) : QPushButton(parent)
{
    connect(this, &QPushButton::clicked, [this]()
    {
        setEnabled(false);
        setText(runningText_);
    });
}

void JobButton::setLabels(const QString& readyText, const QString& runningText)
{
    readyText_ = readyText;
    runningText_ = runningText;
}

void JobButton::ready()
{
    setText(readyText_);
    setEnabled(true);
}
