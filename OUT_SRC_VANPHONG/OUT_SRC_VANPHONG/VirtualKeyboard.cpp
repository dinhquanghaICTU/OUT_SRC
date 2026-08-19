#include "VirtualKeyboard.h"

#include <QApplication>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

static const char *row1_lower[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"};
static const char *row1_upper[] = {"!", "@", "#", "$", "%", "^", "&", "*", "(", ")"};

static const char *row2_lower[] = {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p"};
static const char *row2_upper[] = {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"};

static const char *row3_lower[] = {"a", "s", "d", "f", "g", "h", "j", "k", "l", "_"};
static const char *row3_upper[] = {"A", "S", "D", "F", "G", "H", "J", "K", "L", "-"};

static const char *row4_lower[] = {"z", "x", "c", "v", "b", "n", "m", ".", "/", "@"};
static const char *row4_upper[] = {"Z", "X", "C", "V", "B", "N", "M", ",", "?", ":"};

static const char *sym_row1[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"};
static const char *sym_row2[] = {"~", "`", "!", "@", "#", "$", "%", "^", "&", "*"};
static const char *sym_row3[] = {"(", ")", "-", "_", "=", "+", "[", "]", "{", "}"};
static const char *sym_row4[] = {"\\", "|", ";", ":", "'", "\"", "<", ">", ",", "."};

VirtualKeyboard::VirtualKeyboard(QWidget *parent)
    : QWidget(parent), m_grid(new QGridLayout(this))
{
    setObjectName("virtualKeyboard");
    m_grid->setSpacing(4);
    m_grid->setContentsMargins(6, 6, 6, 6);
    rebuildLayout();
}

void VirtualKeyboard::attachTo(QLineEdit *target)
{
    m_target = target;
}

void VirtualKeyboard::setShifted(bool shifted)
{
    m_shifted = shifted;
    rebuildLayout();
}

void VirtualKeyboard::setSymbolMode(bool symbol)
{
    m_symbolMode = symbol;
    rebuildLayout();
}

void VirtualKeyboard::createKey(int row, int col, int span, const QString &text, const QString &objName)
{
    auto *btn = new QPushButton(text, this);
    btn->setObjectName(objName);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    btn->setMinimumHeight(34);
    btn->setFocusPolicy(Qt::NoFocus);
    connect(btn, &QPushButton::clicked, this, &VirtualKeyboard::onKeyClicked);
    m_grid->addWidget(btn, row, col, 1, span);

    if (objName == "kbKey")
        m_letterButtons.append(btn);
    else if (objName == "kbShift")
        m_shiftBtn = btn;
    else if (objName == "kbMode")
        m_modeBtn = btn;
}

void VirtualKeyboard::rebuildLayout()
{
    m_letterButtons.clear();
    QLayoutItem *item;
    while ((item = m_grid->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    const char **r1 = m_symbolMode ? sym_row1 : (m_shifted ? row1_upper : row1_lower);
    const char **r2 = m_symbolMode ? sym_row2 : (m_shifted ? row2_upper : row2_lower);
    const char **r3 = m_symbolMode ? sym_row3 : (m_shifted ? row3_upper : row3_lower);
    const char **r4 = m_symbolMode ? sym_row4 : (m_shifted ? row4_upper : row4_lower);

    for (int i = 0; i < 10; ++i)
        createKey(0, i * 2, 2, QString::fromUtf8(r1[i]));
    createKey(0, 20, 3, "⌫", "kbBackspace");

    for (int i = 0; i < 10; ++i)
        createKey(1, i * 2, 2, QString::fromUtf8(r2[i]));
    createKey(1, 20, 3, "Tab", "kbTab");

    createKey(2, 0, 2, m_shifted ? "⇧ ON" : "⇧", "kbShift");
    for (int i = 0; i < 10; ++i)
        createKey(2, 2 + i * 2, 2, QString::fromUtf8(r3[i]));
    createKey(2, 22, 2, "Enter", "kbEnter");

    createKey(3, 0, 2, m_symbolMode ? "ABC" : "?123", "kbMode");
    for (int i = 0; i < 10; ++i)
        createKey(3, 2 + i * 2, 2, QString::fromUtf8(r4[i]));
    createKey(3, 22, 2, "Ẩn", "kbHide");

    createKey(4, 0, 3, "Clear", "kbClear");
    createKey(4, 3, 2, "◄", "kbLeft");
    createKey(4, 5, 10, "Space", "kbSpace");
    createKey(4, 15, 2, "►", "kbRight");
    createKey(4, 17, 3, ".com", "kbKey");
    createKey(4, 20, 4, "OK", "kbDone");
}

void VirtualKeyboard::onKeyClicked()
{
    auto *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    const QString text = btn->text();
    const QString name = btn->objectName();

    if (name == "kbShift") {
        toggleShift();
        return;
    }
    if (name == "kbMode") {
        toggleSymbols();
        return;
    }
    if (name == "kbHide") {
        emit hideRequested();
        hide();
        return;
    }
    if (name == "kbDone") {
        emit enterPressed();
        return;
    }

    if (!m_target) return;

    if (name == "kbBackspace") {
        m_target->backspace();
    } else if (name == "kbClear") {
        m_target->clear();
    } else if (name == "kbSpace") {
        m_target->insert(" ");
    } else if (name == "kbTab") {
        m_target->insert("\t");
    } else if (name == "kbLeft") {
        m_target->setCursorPosition(qMax(0, m_target->cursorPosition() - 1));
    } else if (name == "kbRight") {
        m_target->setCursorPosition(qMin(m_target->text().length(), m_target->cursorPosition() + 1));
    } else if (name == "kbEnter") {
        emit enterPressed();
    } else {
        m_target->insert(text);
        if (m_shifted && !m_capsLock) {
            m_shifted = false;
            rebuildLayout();
        }
    }
}

void VirtualKeyboard::toggleShift()
{
    m_shifted = !m_shifted;
    rebuildLayout();
}

void VirtualKeyboard::toggleSymbols()
{
    m_symbolMode = !m_symbolMode;
    rebuildLayout();
}

VirtualKeyboardDialog::VirtualKeyboardDialog(QLineEdit *target, QWidget *parent, const QString &title)
    : QDialog(parent), m_target(target)
{
    setWindowTitle(title.isEmpty() ? tr("Bàn phím ảo") : title);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setModal(true);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    if (!title.isEmpty()) {
        auto *titleLabel = new QLabel(title, this);
        titleLabel->setObjectName("kbDialogTitle");
        mainLayout->addWidget(titleLabel);
    }

    m_previewEdit = new QLineEdit(this);
    m_previewEdit->setObjectName("kbPreviewEdit");
    if (m_target) {
        m_previewEdit->setText(m_target->text());
        m_previewEdit->setEchoMode(m_target->echoMode());
        m_previewEdit->setPlaceholderText(m_target->placeholderText());
    }
    m_previewEdit->setMinimumHeight(40);
    mainLayout->addWidget(m_previewEdit);

    m_keyboard = new VirtualKeyboard(this);
    m_keyboard->attachTo(m_previewEdit);
    mainLayout->addWidget(m_keyboard);

    connect(m_keyboard, &VirtualKeyboard::enterPressed, this, [this] {
        if (m_target)
            m_target->setText(m_previewEdit->text());
        accept();
    });
    connect(m_keyboard, &VirtualKeyboard::hideRequested, this, &QDialog::reject);

    resize(520, 280);
}

void VirtualKeyboardDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (m_previewEdit) {
        m_previewEdit->setFocus();
        m_previewEdit->selectAll();
    }
}

void VirtualKeyboardDialog::openFor(QLineEdit *target, QWidget *parent, const QString &title)
{
    VirtualKeyboardDialog dlg(target, parent, title);
    if (dlg.exec() == QDialog::Accepted && target) {
        emit target->editingFinished();
        emit target->returnPressed();
    }
}

void VirtualKeyboardDialog::attachToLineEdit(QLineEdit *target, const QString &title)
{
    if (!target) return;
    struct KbFilter : public QObject {
        QString dlgTitle;
        KbFilter(QObject *parent, QString t) : QObject(parent), dlgTitle(std::move(t)) {}
        bool eventFilter(QObject *watched, QEvent *event) override {
            if (event->type() == QEvent::MouseButtonPress) {
                auto *le = qobject_cast<QLineEdit*>(watched);
                if (le && le->isEnabled() && !le->isReadOnly()) {
                    VirtualKeyboardDialog::openFor(le, le->window(), dlgTitle);
                    return true;
                }
            }
            return QObject::eventFilter(watched, event);
        }
    };
    target->installEventFilter(new KbFilter(target, title));
}
