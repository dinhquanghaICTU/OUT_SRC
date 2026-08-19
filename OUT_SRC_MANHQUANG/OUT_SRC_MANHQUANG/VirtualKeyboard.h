#ifndef VIRTUALKEYBOARD_H
#define VIRTUALKEYBOARD_H

#include <QWidget>
#include <QDialog>
#include <QVector>
#include <QString>

class QGridLayout;
class QLineEdit;
class QPushButton;

class VirtualKeyboard : public QWidget
{
    Q_OBJECT
public:
    explicit VirtualKeyboard(QWidget *parent = nullptr);

    /// Attach keyboard to a target QLineEdit
    void attachTo(QLineEdit *target);
    QLineEdit *currentTarget() const { return m_target; }

    void setShifted(bool shifted);
    void setSymbolMode(bool symbol);

signals:
    void enterPressed();
    void hideRequested();

private slots:
    void onKeyClicked();
    void toggleShift();
    void toggleSymbols();

private:
    void rebuildLayout();
    void createKey(int row, int col, int span, const QString &text, const QString &objName = "kbKey");

    QLineEdit *m_target = nullptr;
    QGridLayout *m_grid = nullptr;
    bool m_shifted = false;
    bool m_capsLock = false;
    bool m_symbolMode = false;

    QVector<QPushButton*> m_letterButtons;
    QPushButton *m_shiftBtn = nullptr;
    QPushButton *m_modeBtn = nullptr;
};

class QDialog;
class QLabel;

class VirtualKeyboardDialog : public QDialog
{
    Q_OBJECT
public:
    explicit VirtualKeyboardDialog(QLineEdit *target, QWidget *parent = nullptr, const QString &title = QString());
    static void openFor(QLineEdit *target, QWidget *parent = nullptr, const QString &title = QString());
    static void attachToLineEdit(QLineEdit *target, const QString &title = QString());

protected:
    void showEvent(QShowEvent *event) override;

private:
    QLineEdit *m_target = nullptr;
    QLineEdit *m_previewEdit = nullptr;
    VirtualKeyboard *m_keyboard = nullptr;
};

#endif // VIRTUALKEYBOARD_H
