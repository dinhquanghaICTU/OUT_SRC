#include "UserManagementPage.h"
#include "VirtualKeyboard.h"
#include "ui_UserManagementPage.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

UserManagementPage::UserManagementPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::UserManagementPage)
{
    ui->setupUi(this);
    setStyleSheet(
        "QWidget#UserManagementPage { "
        "  background-color: #06090e; "
        "  color: #e2e8f0; "
        "  font-family: 'Segoe UI', sans-serif; "
        "}"
    );

    setupCustomUI();
}

UserManagementPage::~UserManagementPage()
{
    delete ui;
}

void UserManagementPage::setUsers(const QJsonArray &users)
{
    m_users = users;

    int total = users.size();
    int admins = 0;
    int regularUsers = 0;

    for (const auto &val : users) {
        const auto u = val.toObject();
        if (u.value(QStringLiteral("role")).toString() == QStringLiteral("admin"))
            admins++;
        else
            regularUsers++;
    }

    if (m_filterAllBtn) m_filterAllBtn->setText(QStringLiteral("ALL USERS (%1)").arg(total));
    if (m_filterAdminBtn) m_filterAdminBtn->setText(QStringLiteral("👑 ADMINS (%1)").arg(admins));
    if (m_filterUserBtn) m_filterUserBtn->setText(QStringLiteral("👤 OPERATORS (%1)").arg(regularUsers));

    renderUserGrid();
}

void UserManagementPage::setAdminEnabled(bool enabled)
{
    m_adminEnabled = enabled;
}

void UserManagementPage::setupCustomUI()
{
    while (QLayoutItem *item = ui->verticalLayout->takeAt(0)) {
        if (item->widget()) delete item->widget();
        delete item;
    }

    auto *mainLayout = ui->verticalLayout;
    mainLayout->setContentsMargins(10, 8, 10, 8);
    mainLayout->setSpacing(8);

    // ==========================================
    // TOP COMMAND BAR
    // ==========================================
    auto *topBar = new QHBoxLayout;
    topBar->setSpacing(8);

    auto *titleIcon = new QLabel(QStringLiteral("⬢"));
    titleIcon->setStyleSheet("font-size: 14px; color: #00f0ff; background: transparent;");
    auto *titleLbl = new QLabel(QStringLiteral("SECURITY ACCESS & IDENTITY MATRIX"));
    titleLbl->setStyleSheet("color: #ffffff; font-size: 11px; font-weight: 900; background: transparent; letter-spacing: 0.8px;");

    topBar->addWidget(titleIcon);
    topBar->addWidget(titleLbl);
    topBar->addSpacing(12);

    auto makeFilterBtn = [&](const QString &label, const QString &mode) {
        auto *btn = new QPushButton(label);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setCheckable(true);
        btn->setStyleSheet(
            "QPushButton { "
            "  background: #0c1218; "
            "  color: #64748b; "
            "  border: 1px solid #162430; "
            "  border-radius: 5px; "
            "  font-size: 9px; "
            "  font-weight: 800; "
            "  padding: 4px 10px; "
            "} "
            "QPushButton:hover { background: #131d26; color: #ffffff; } "
            "QPushButton:checked { background: #0284c7; color: #ffffff; border-color: #38bdf8; font-weight: 900; }"
        );
        connect(btn, &QPushButton::clicked, this, [this, mode] {
            m_currentFilter = mode;
            m_filterAllBtn->setChecked(mode == QStringLiteral("all"));
            m_filterAdminBtn->setChecked(mode == QStringLiteral("admin"));
            m_filterUserBtn->setChecked(mode == QStringLiteral("user"));
            renderUserGrid();
        });
        return btn;
    };

    m_filterAllBtn = makeFilterBtn(QStringLiteral("ALL USERS (0)"), QStringLiteral("all"));
    m_filterAdminBtn = makeFilterBtn(QStringLiteral("👑 ADMINS (0)"), QStringLiteral("admin"));
    m_filterUserBtn = makeFilterBtn(QStringLiteral("👤 OPERATORS (0)"), QStringLiteral("user"));
    m_filterAllBtn->setChecked(true);

    topBar->addWidget(m_filterAllBtn);
    topBar->addWidget(m_filterAdminBtn);
    topBar->addWidget(m_filterUserBtn);
    topBar->addStretch();

    // + Add User Button
    auto *addUserBtn = new QPushButton(QStringLiteral("＋ REGISTER IDENTITY"));
    addUserBtn->setCursor(Qt::PointingHandCursor);
    addUserBtn->setStyleSheet(
        "QPushButton { "
        "  background: #0284c7; "
        "  color: #ffffff; "
        "  border: none; "
        "  border-radius: 5px; "
        "  font-size: 10px; "
        "  font-weight: 900; "
        "  padding: 5px 12px; "
        "} "
        "QPushButton:hover { background: #0369a1; } "
        "QPushButton:pressed { background: #075985; }"
    );
    connect(addUserBtn, &QPushButton::clicked, this, [this] { openEditDialog(); });
    topBar->addWidget(addUserBtn);

    mainLayout->addLayout(topBar);

    // ==========================================
    // GRID MATRIX AREA
    // ==========================================
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("background: transparent; border: none;");

    auto *container = new QWidget;
    container->setStyleSheet("background: transparent;");
    m_gridLayout = new QGridLayout(container);
    m_gridLayout->setContentsMargins(2, 2, 2, 2);
    m_gridLayout->setHorizontalSpacing(8);
    m_gridLayout->setVerticalSpacing(8);

    m_emptyLabel = new QLabel(QStringLiteral("Đang đồng bộ cơ sở dữ liệu phân quyền..."));
    m_emptyLabel->setStyleSheet("color: #64748b; font-style: italic; font-size: 11px; padding: 30px;");
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_gridLayout->addWidget(m_emptyLabel, 0, 0, 1, 2);

    scroll->setWidget(container);
    mainLayout->addWidget(scroll, 1);
}

void UserManagementPage::renderUserGrid()
{
    QLayoutItem *item;
    while ((item = m_gridLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            if (item->widget() != m_emptyLabel)
                delete item->widget();
        }
        delete item;
    }

    QVector<QJsonObject> filteredUsers;
    for (const auto &val : m_users) {
        const auto u = val.toObject();
        const QString role = u.value(QStringLiteral("role")).toString(QStringLiteral("user"));

        if (m_currentFilter == QStringLiteral("admin") && role != QStringLiteral("admin"))
            continue;
        if (m_currentFilter == QStringLiteral("user") && role == QStringLiteral("admin"))
            continue;

        filteredUsers.append(u);
    }

    if (filteredUsers.isEmpty()) {
        m_emptyLabel->setText(QStringLiteral("Không tìm thấy tài khoản phù hợp."));
        m_emptyLabel->show();
        m_gridLayout->addWidget(m_emptyLabel, 0, 0, 1, 2);
        return;
    }

    m_emptyLabel->hide();

    int row = 0;
    int col = 0;

    for (const auto &u : filteredUsers) {
        const QString username = u.value(QStringLiteral("username")).toString();
        const QString role = u.value(QStringLiteral("role")).toString(QStringLiteral("user"));
        const bool isAdmin = (role == QStringLiteral("admin"));
        const bool isEnabled = u.value(QStringLiteral("enabled")).toBool(true);

        QStringList devices;
        for (const auto &d : u.value(QStringLiteral("device_ids")).toArray()) {
            devices.append(d.toString());
        }

        auto *card = new QFrame;
        card->setStyleSheet(
            "QFrame { "
            "  background-color: #0c1218; "
            "  border: 1px solid #162430; "
            "  border-radius: 8px; "
            "} "
            "QFrame:hover { "
            "  border-color: #00f0ff; "
            "  background-color: #111a24; "
            "}"
        );

        auto *cLayout = new QVBoxLayout(card);
        cLayout->setContentsMargins(10, 8, 10, 8);
        cLayout->setSpacing(6);

        // Header
        auto *headerRow = new QHBoxLayout;
        auto *avatarLbl = new QLabel(isAdmin ? QStringLiteral("👑") : QStringLiteral("👤"));
        avatarLbl->setFixedSize(28, 28);
        avatarLbl->setAlignment(Qt::AlignCenter);
        avatarLbl->setStyleSheet(QStringLiteral(
            "background: %1; border-radius: 14px; font-size: 14px; border: 1px solid %2;")
            .arg(isAdmin ? QStringLiteral("rgba(2, 132, 199, 0.2)") : QStringLiteral("rgba(16, 185, 129, 0.2)"))
            .arg(isAdmin ? QStringLiteral("#38bdf8") : QStringLiteral("#10b981"))
        );
        headerRow->addWidget(avatarLbl);

        auto *roleBadge = new QLabel(isAdmin ? QStringLiteral("ROOT ADMIN") : QStringLiteral("OPERATOR"));
        roleBadge->setStyleSheet(isAdmin
            ? "color: #38bdf8; font-size: 8px; font-weight: 900; background: rgba(2, 132, 199, 0.15); border-radius: 3px; padding: 2px 5px;"
            : "color: #10b981; font-size: 8px; font-weight: 900; background: rgba(16, 185, 129, 0.15); border-radius: 3px; padding: 2px 5px;");
        headerRow->addWidget(roleBadge);
        headerRow->addStretch();

        auto *statusBadge = new QLabel(isEnabled ? QStringLiteral("🟢 ACTIVE") : QStringLiteral("🔴 LOCKED"));
        statusBadge->setStyleSheet(isEnabled
            ? "color: #10b981; font-size: 8px; font-weight: 900; background: rgba(16, 185, 129, 0.12); border-radius: 3px; padding: 2px 5px;"
            : "color: #ef4444; font-size: 8px; font-weight: 900; background: rgba(239, 68, 68, 0.12); border-radius: 3px; padding: 2px 5px;");
        headerRow->addWidget(statusBadge);
        cLayout->addLayout(headerRow);

        // Username
        auto *uNameLbl = new QLabel(username);
        uNameLbl->setStyleSheet("color: #ffffff; font-size: 14px; font-weight: 900; background: transparent; font-family: monospace;");
        cLayout->addWidget(uNameLbl);

        // Devices Box
        auto *devBox = new QFrame;
        devBox->setStyleSheet("background: #080c10; border: 1px solid #141f29; border-radius: 4px;");
        auto *devBoxLay = new QVBoxLayout(devBox);
        devBoxLay->setContentsMargins(6, 4, 6, 4);
        devBoxLay->setSpacing(2);

        if (devices.isEmpty()) {
            auto *noDevLbl = new QLabel(QStringLiteral("📡 NO HARDWARE LINKED"));
            noDevLbl->setStyleSheet("color: #475569; font-size: 8px; font-weight: 700; background: transparent;");
            devBoxLay->addWidget(noDevLbl);
        } else {
            for (const QString &devId : devices) {
                auto *dRow = new QHBoxLayout;
                auto *dName = new QLabel(QStringLiteral("POD: %1").arg(devId));
                dName->setStyleSheet("color: #00f0ff; font-size: 9px; font-weight: 800; background: transparent;");
                dRow->addWidget(dName);
                dRow->addStretch();

                auto *unBtn = new QPushButton(QStringLiteral("✕ UNLINK"));
                unBtn->setCursor(Qt::PointingHandCursor);
                unBtn->setStyleSheet("QPushButton { background: #2b1118; color: #f87171; border: 1px solid #7f1d1d; border-radius: 3px; font-size: 7px; font-weight: 800; padding: 1px 4px; } QPushButton:hover { background: #dc2626; color: #fff; }");
                connect(unBtn, &QPushButton::clicked, this, [this, username, devId] {
                    emit releaseUserDeviceRequested(username, devId);
                });
                dRow->addWidget(unBtn);
                devBoxLay->addLayout(dRow);
            }
        }
        cLayout->addWidget(devBox);

        // Actions
        auto *actRow = new QHBoxLayout;
        actRow->setSpacing(4);

        auto *editBtn = new QPushButton(QStringLiteral("✏ EDIT"));
        editBtn->setCursor(Qt::PointingHandCursor);
        editBtn->setStyleSheet(
            "QPushButton { "
            "  background: #0284c7; "
            "  color: #ffffff; "
            "  border: none; "
            "  border-radius: 4px; "
            "  font-size: 9px; "
            "  font-weight: 900; "
            "  padding: 4px 10px; "
            "} "
            "QPushButton:hover { background: #0369a1; }"
        );
        connect(editBtn, &QPushButton::clicked, this, [this, u] { openEditDialog(u); });
        actRow->addWidget(editBtn);

        if (username != QStringLiteral("admin")) {
            auto *delBtn = new QPushButton(QStringLiteral("🗑 PURGE"));
            delBtn->setCursor(Qt::PointingHandCursor);
            delBtn->setStyleSheet(
                "QPushButton { "
                "  background: #2b1118; "
                "  color: #fecaca; "
                "  border: 1px solid #7f1d1d; "
                "  border-radius: 4px; "
                "  font-size: 9px; "
                "  font-weight: 900; "
                "  padding: 4px 10px; "
                "} "
                "QPushButton:hover { background: #dc2626; color: #ffffff; }"
            );
            connect(delBtn, &QPushButton::clicked, this, [this, u] { confirmDeleteUser(u); });
            actRow->addWidget(delBtn);
        }

        actRow->addStretch();
        cLayout->addLayout(actRow);

        m_gridLayout->addWidget(card, row, col);
        col++;
        if (col >= 2) {
            col = 0;
            row++;
        }
    }
}

void UserManagementPage::openEditDialog(const QJsonObject &user)
{
    const bool isEdit = !user.isEmpty();
    const QString oldUser = user.value(QStringLiteral("username")).toString();

    QDialog dlg(this);
    dlg.setWindowTitle(isEdit ? QStringLiteral("Cập Nhật Tài Khoản") : QStringLiteral("Đăng Ký Tài Khoản Mới"));
    dlg.resize(440, 360);
    dlg.setStyleSheet(
        "QDialog { background-color: #06090e; color: #e2e8f0; font-family: sans-serif; } "
        "QLabel { color: #94a3b8; font-weight: 700; font-size: 11px; } "
        "QLineEdit, QComboBox { background-color: #0c1218; color: #ffffff; border: 1.5px solid #162430; border-radius: 6px; padding: 5px 8px; font-size: 12px; } "
        "QLineEdit:focus, QComboBox:focus { border: 1.5px solid #00f0ff; background-color: #111a24; }"
    );

    auto *mainLayout = new QVBoxLayout(&dlg);
    mainLayout->setContentsMargins(16, 14, 16, 14);
    mainLayout->setSpacing(10);

    auto *titleLbl = new QLabel(isEdit ? QStringLiteral("✏ Chỉnh Sửa Định Danh: %1").arg(oldUser) : QStringLiteral("＋ Đăng Ký Định Danh Mới"));
    titleLbl->setStyleSheet("color: #00f0ff; font-size: 13px; font-weight: 900;");
    mainLayout->addWidget(titleLbl);

    auto *form = new QFormLayout;
    form->setSpacing(8);

    auto *uInput = new QLineEdit(&dlg);
    uInput->setText(oldUser);
    uInput->setPlaceholderText(QStringLiteral("Nhập tên đăng nhập..."));
    form->addRow(QStringLiteral("Tài khoản:"), uInput);

    auto *pInput = new QLineEdit(&dlg);
    pInput->setEchoMode(QLineEdit::Password);
    pInput->setPlaceholderText(isEdit ? QStringLiteral("Để trống nếu giữ nguyên") : QStringLiteral("Nhập mật khẩu..."));
    form->addRow(QStringLiteral("Mật khẩu:"), pInput);

    auto *rCombo = new QComboBox(&dlg);
    rCombo->addItem(QStringLiteral("Người dùng (User)"), QStringLiteral("user"));
    rCombo->addItem(QStringLiteral("Quản trị viên (Admin)"), QStringLiteral("admin"));
    if (user.value(QStringLiteral("role")).toString() == QStringLiteral("admin")) {
        rCombo->setCurrentIndex(1);
    }
    form->addRow(QStringLiteral("Quyền hạn:"), rCombo);

    auto *enCheck = new QCheckBox(QStringLiteral("Kích hoạt hoạt động tài khoản"), &dlg);
    enCheck->setChecked(user.value(QStringLiteral("enabled")).toBool(true));
    enCheck->setStyleSheet("color: #94a3b8; font-size: 11px; font-weight: 700;");
    form->addRow(QString(), enCheck);

    mainLayout->addLayout(form);

    // Virtual Keyboard with instant focus switching
    auto *kb = new VirtualKeyboard(&dlg);
    kb->attachTo(uInput);

    struct InputFocusFilter : public QObject {
        QLineEdit *input;
        VirtualKeyboard *keyboard;
        InputFocusFilter(QLineEdit *inp, VirtualKeyboard *k, QObject *parent = nullptr)
            : QObject(parent), input(inp), keyboard(k) {}
        bool eventFilter(QObject *watched, QEvent *event) override {
            if (event->type() == QEvent::FocusIn || event->type() == QEvent::MouseButtonPress) {
                if (keyboard && input) {
                    keyboard->attachTo(input);
                }
            }
            return QObject::eventFilter(watched, event);
        }
    };

    uInput->installEventFilter(new InputFocusFilter(uInput, kb, &dlg));
    pInput->installEventFilter(new InputFocusFilter(pInput, kb, &dlg));
    mainLayout->addWidget(kb);

    // Action Buttons
    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();

    auto *cancelBtn = new QPushButton(QStringLiteral("Hủy"), &dlg);
    cancelBtn->setStyleSheet("QPushButton { background: #1e293b; color: #ffffff; border: none; border-radius: 5px; font-size: 10px; font-weight: 800; padding: 6px 14px; }");
    connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);
    btnRow->addWidget(cancelBtn);

    auto *saveBtn = new QPushButton(isEdit ? QStringLiteral("💾 Lưu Thay Đổi") : QStringLiteral("＋ Tạo Tài Khoản"), &dlg);
    saveBtn->setStyleSheet("QPushButton { background: #0284c7; color: #ffffff; border: none; border-radius: 5px; font-size: 10px; font-weight: 900; padding: 6px 16px; } QPushButton:hover { background: #0369a1; }");
    connect(saveBtn, &QPushButton::clicked, &dlg, [&] {
        const QString u = uInput->text().trimmed();
        const QString p = pInput->text();
        const QString r = rCombo->currentData().toString();
        const bool en = enCheck->isChecked();

        if (u.isEmpty()) {
            QMessageBox::warning(&dlg, QStringLiteral("Lỗi"), QStringLiteral("Vui lòng nhập tên tài khoản."));
            return;
        }

        if (isEdit) {
            emit updateUserRequested(oldUser, u, p, r, en);
        } else {
            if (p.isEmpty()) {
                QMessageBox::warning(&dlg, QStringLiteral("Lỗi"), QStringLiteral("Vui lòng nhập mật khẩu cho tài khoản mới."));
                return;
            }
            emit createUserRequested(u, p, r);
        }
        dlg.accept();
    });
    btnRow->addWidget(saveBtn);
    mainLayout->addLayout(btnRow);

    dlg.exec();
}

void UserManagementPage::confirmDeleteUser(const QJsonObject &user)
{
    const QString username = user.value(QStringLiteral("username")).toString();
    auto res = QMessageBox::question(
        this,
        QStringLiteral("Xác Nhận Xóa"),
        QStringLiteral("Bạn có chắc chắn muốn xóa tài khoản <b>%1</b> không?").arg(username),
        QMessageBox::Yes | QMessageBox::No
    );
    if (res == QMessageBox::Yes) {
        emit deleteUserRequested(username);
    }
}
