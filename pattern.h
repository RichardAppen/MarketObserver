#include <QMainWindow>
#include <QApplication>

#ifndef PATTERN_H
#define PATTERN_H


class Pattern
{
public:
    Pattern();
    ~Pattern();

    QString name;
    QString firstTrigger;
    QString secondTrigger;
    QString thirdTrigger;
    int amountOfFirst;
    int amountOfSecond;
    int amountOfThird;
};

#endif // PATTERN_H
