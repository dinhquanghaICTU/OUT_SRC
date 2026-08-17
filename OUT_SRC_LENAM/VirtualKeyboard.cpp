#include "VirtualKeyboard.h"

#include <QDialog>
#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QSizePolicy>
#include <QVBoxLayout>

VirtualKeyboard::VirtualKeyboard(QWidget *parent) : QWidget(parent)
{
    setObjectName("virtualKeyboard");
    m_grid = new QGridLayout(this);
    m_grid->setSpacing(2);
    m_grid->setContentsMargins(4, 4, 4, 4);
    rebuildLayout();
    setFixedHeight(145);
}

void VirtualKeyboard::attachTo(QLineEdit *target)
{
    m_target = target;
}

void VirtualKeyboard::createKey(int row, int col, int span, const QString &text, const QString &objName)
{
    auto *btn = new QPushButton(text, this);
    btn->setObjectName(objName);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFocusPolicy(Qt::NoFocus);
    btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btn->setMinimumHeight(28);
    m_grid->addWidget(btn, row, col, 1, span);

    if (text == "⇧") {
        m_shiftBtn = btn;
        connect(btn, &QPushButton::clicked, this, &VirtualKeyboard::toggleShift);
    } else if (text == "⌫") {
        connect(btn, &QPushButton::clicked, this, [this] {
            if (m_target) m_target->backspace();
        });
    } else if (text == "↵") {
        connect(btn, &QPushButton::clicked, this, [this] {
            emit enterPressed();
        });
    } else if (text == "123" || text == "ABC") {
        m_modeBtn = btn;
        connect(btn, &QPushButton::clicked, this, &VirtualKeyboard::toggleSymbols);
    } else if (text == "Clr") {
        connect(btn, &QPushButton::clicked, this, [this] {
            if (m_target) m_target->clear();
        });
    } else if (text == "▼") {
        connect(btn, &QPushButton::clicked, this, [this] {
            emit hideRequested();
            this->hide();
        });
    } else {
        if (text.length() == 1 && text[0].isLetter()) {
            m_letterButtons.append(btn);
        }
        connect(btn, &QPushButton::clicked, this, &VirtualKeyboard::onKeyClicked);
    }
}

void VirtualKeyboard::rebuildLayout()
{
    // Clear existing layout items
    while (auto *item = m_grid->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    m_letterButtons.clear();

    if (!m_symbolMode) {
        // === NORMAL / QWERTY MODE ===
        // Row 0: Numbers
        const QStringList r0 = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"};
        for (int i = 0; i < r0.size(); ++i) {
            createKey(0, i * 2, 2, r0[i], "kbKeyNum");
        }
        createKey(0, 20, 2, "-", "kbKeyNum");
        createKey(0, 22, 2, "_", "kbKeyNum");

        // Row 1: Q W E R T Y U I O P
        const QStringList r1 = {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p"};
        for (int i = 0; i < r1.size(); ++i) {
            QString ch = m_shifted ? r1[i].toUpper() : r1[i];
            createKey(1, i * 2 + 1, 2, ch);
        }
        createKey(1, 21, 3, "⌫", "kbKeyAction");

        // Row 2: A S D F G H J K L
        const QStringList r2 = {"a", "s", "d", "f", "g", "h", "j", "k", "l"};
        createKey(2, 0, 2, "@", "kbKeyNum");
        for (int i = 0; i < r2.size(); ++i) {
            QString ch = m_shifted ? r2[i].toUpper() : r2[i];
            createKey(2, i * 2 + 2, 2, ch);
        }
        createKey(2, 20, 4, "↵", "kbKeyEnter");

        // Row 3: Shift, Z X C V B N M, .
        createKey(3, 0, 3, "⇧", m_shifted ? "kbKeyShiftActive" : "kbKeyShift");
        const QStringList r3 = {"z", "x", "c", "v", "b", "n", "m"};
        for (int i = 0; i < r3.size(); ++i) {
            QString ch = m_shifted ? r3[i].toUpper() : r3[i];
            createKey(3, i * 2 + 3, 2, ch);
        }
        createKey(3, 17, 2, ".");
        createKey(3, 19, 2, "/");
        createKey(3, 21, 3, "Clr", "kbKeyAction");

        // Row 4: Mode switch, Space, Hide
        createKey(4, 0, 4, "123", "kbKeyMode");
        createKey(4, 4, 12, " ", "kbKeySpace");
        createKey(4, 16, 4, ".com", "kbKeyAction");
        createKey(4, 20, 4, "▼", "kbKeyAction");
    } else {
        // === SYMBOLS / SPECIAL CHARACTERS MODE ===
        // Row 0: Special symbols
        const QStringList s0 = {"!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "[", "]"};
        for (int i = 0; i < s0.size(); ++i) {
            createKey(0, i * 2, 2, s0[i], "kbKeyNum");
        }

        // Row 1: More symbols
        const QStringList s1 = {"~", "`", "{", "}", "\\", "|", ";", ":", "'", "\""};
        for (int i = 0; i < s1.size(); ++i) {
            createKey(1, i * 2 + 1, 2, s1[i]);
        }
        createKey(1, 21, 3, "⌫", "kbKeyAction");

        // Row 2: Math and punctuation
        const QStringList s2 = {"<", ">", "?", "/", "+", "=", "-", "_", "*"};
        for (int i = 0; i < s2.size(); ++i) {
            createKey(2, i * 2 + 1, 2, s2[i]);
        }
        createKey(2, 19, 5, "↵", "kbKeyEnter");

        // Row 3: Quick tokens
        const QStringList s3 = {".", ",", ":", ";", "=", "!", "?", "%"};
        for (int i = 0; i < s3.size(); ++i) {
            createKey(3, i * 2 + 2, 2, s3[i]);
        }
        createKey(3, 18, 3, "Clr", "kbKeyAction");
        createKey(3, 21, 3, "▼", "kbKeyAction");

        // Row 4: Mode switch, Space
        createKey(4, 0, 4, "ABC", "kbKeyMode");
        createKey(4, 4, 14, " ", "kbKeySpace");
        createKey(4, 18, 6, "127.0.0.1", "kbKeyAction");
    }
}

void VirtualKeyboard::onKeyClicked()
{
    auto *btn = qobject_cast<QPushButton *>(sender());
    if (!btn || !m_target) return;

    QString text = btn->text();
    m_target->insert(text);

    // Auto-unshift after 1 letter typed if in shift mode
    if (m_shifted) {
        m_shifted = false;
        rebuildLayout();
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
    m_shifted = false;
    rebuildLayout();
}

void VirtualKeyboard::setShifted(bool shifted)
{
    if (m_shifted != shifted) {
        m_shifted = shifted;
        rebuildLayout();
    }
}

void VirtualKeyboard::setSymbolMode(bool symbol)
{
    if (m_symbolMode != symbol) {
        m_symbolMode = symbol;
        rebuildLayout();
    }
}

#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPointer>
#include <QVBoxLayout>

VirtualKeyboardDialog::VirtualKeyboardDialog(QLineEdit *target, QWidget *parent, const QString &title)
    : QDialog(parent), m_target(target)
{
    setObjectName(QStringLiteral("virtualKeyboardDialog"));
    setWindowTitle(title.isEmpty() ? tr("Bàn phím ảo") : title);
    setModal(true);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    const int availableWidth = parent ? parent->width() - 8 : 792;
    resize(qBound(540, availableWidth, 792), 220);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(12, 10, 12, 10);
    root->setSpacing(8);

    auto *topLayout = new QHBoxLayout;
    topLayout->setSpacing(10);

    if (!title.isEmpty()) {
        auto *titleLabel = new QLabel(title, this);
        titleLabel->setObjectName(QStringLiteral("kbDialogTitle"));
        topLayout->addWidget(titleLabel);
    }

    m_previewEdit = new QLineEdit(this);
    m_previewEdit->setObjectName(QStringLiteral("kbDialogInput"));
    if (m_target) {
        m_previewEdit->setText(m_target->text());
        m_previewEdit->setEchoMode(m_target->echoMode());
        m_previewEdit->setPlaceholderText(m_target->placeholderText());
    }
    topLayout->addWidget(m_previewEdit, 1);

    auto *doneBtn = new QPushButton(tr("✓ Xong"), this);
    doneBtn->setObjectName(QStringLiteral("kbDialogDoneBtn"));
    doneBtn->setCursor(Qt::PointingHandCursor);
    topLayout->addWidget(doneBtn);

    auto *closeBtn = new QPushButton(tr("✕ Đóng"), this);
    closeBtn->setObjectName(QStringLiteral("kbDialogCloseBtn"));
    closeBtn->setCursor(Qt::PointingHandCursor);
    topLayout->addWidget(closeBtn);

    root->addLayout(topLayout);

    m_keyboard = new VirtualKeyboard(this);
    m_keyboard->setFixedHeight(170);
    m_keyboard->attachTo(m_previewEdit);
    root->addWidget(m_keyboard, 1);

    connect(doneBtn, &QPushButton::clicked, this, [this] {
        if (m_target) {
            m_target->setText(m_previewEdit->text());
            emit m_target->editingFinished();
        }
        accept();
    });

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

    connect(m_keyboard, &VirtualKeyboard::enterPressed, this, [this] {
        if (m_target) {
            m_target->setText(m_previewEdit->text());
            emit m_target->editingFinished();
            emit m_target->returnPressed();
        }
        accept();
    });

    connect(m_previewEdit, &QLineEdit::returnPressed, this, [this] {
        if (m_target) {
            m_target->setText(m_previewEdit->text());
            emit m_target->editingFinished();
            emit m_target->returnPressed();
        }
        accept();
    });
}

void VirtualKeyboardDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    QWidget *topWindow = parentWidget() ? parentWidget()->window() : window();
    if (topWindow) {
        const QRect parentGeo = topWindow->geometry();
        const int kbWidth = qMin(792, qMax(540, parentGeo.width() - 8));
        const int kbHeight = 220;
        const int x = parentGeo.x() + (parentGeo.width() - kbWidth) / 2;
        const int y = parentGeo.y() + parentGeo.height() - kbHeight - 4;
        setGeometry(x, y, kbWidth, kbHeight);
    }
}

void VirtualKeyboardDialog::openFor(QLineEdit *target, QWidget *parent, const QString &title)
{
    if (!target) return;
    VirtualKeyboardDialog dlg(target, parent ? parent : target->window(), title);
    dlg.exec();
}

void VirtualKeyboardDialog::attachToLineEdit(QLineEdit *target, const QString &title)
{
    if (!target) return;

    struct LineEditClickFilter : public QObject {
        QPointer<QLineEdit> edit;
        QString dlgTitle;
        bool opening = false;

        LineEditClickFilter(QLineEdit *targetEdit, const QString &t)
            : QObject(targetEdit), edit(targetEdit), dlgTitle(t) {}

        bool eventFilter(QObject *watched, QEvent *event) override {
            if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) {
                if (!opening && edit && edit->isEnabled() && !edit->isReadOnly()) {
                    opening = true;
                    VirtualKeyboardDialog::openFor(edit.data(), edit->window(), dlgTitle);
                    opening = false;
                    return true;
                }
            }
            return QObject::eventFilter(watched, event);
        }
    };

    target->installEventFilter(new LineEditClickFilter(target, title));
}
