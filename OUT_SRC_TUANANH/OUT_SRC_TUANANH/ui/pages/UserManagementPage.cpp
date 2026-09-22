#include "UserManagementPage.h"
#include "VirtualKeyboard.h"
#include "ui_UserManagementPage.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDialog>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

UserManagementPage::UserManagementPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::UserManagementPage)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QStringLiteral(
        "UserManagementPage { background-color: #060b17; }"
        "QLabel { color: #e2e8f0; font-family: 'Segoe UI', 'Roboto', sans-serif; }"));

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

    if (m_statTotal) m_statTotal->setText(QString::number(total));
    if (m_statAdmin) m_statAdmin->setText(QString::number(admins));
    if (m_statRegular) m_statRegular->setText(QString::number(regularUsers));

    if (m_filterAllBtn) m_filterAllBtn->setText(tr("Tất cả (%1)").arg(total));
    if (m_filterAdminBtn) m_filterAdminBtn->setText(tr("Admin (%1)").arg(admins));
    if (m_filterUserBtn) m_filterUserBtn->setText(tr("User (%1)").arg(regularUsers));

    renderUserTable();
}

void UserManagementPage::setAdminEnabled(bool enabled)
{
    m_adminEnabled = enabled;
    if (m_addUserBtn)
        m_addUserBtn->setEnabled(enabled);
}

void UserManagementPage::setupCustomUI()
{
    while (QLayoutItem *item = ui->verticalLayout->takeAt(0)) {
        if (item->widget()) delete item->widget();
        delete item;
    }

    auto *mainLayout = ui->verticalLayout;
    mainLayout->setContentsMargins(12, 10, 12, 10);
    mainLayout->setSpacing(8);

    // ==========================================
    // TOP HEADER BAR
    // ==========================================
    auto *topBar = new QHBoxLayout;
    topBar->setSpacing(8);

    auto *titles = new QVBoxLayout;
    titles->setSpacing(2);
    auto *titleLbl = new QLabel(tr("Quản Lý Tài Khoản Người Dùng"));
    titleLbl->setStyleSheet(QStringLiteral("color: #f8fafc; font-size: 14px; font-weight: 800;"));
    auto *subtitleLbl = new QLabel(tr("Phân quyền tài khoản truy cập và giám sát điều khiển đèn phòng theo ca/khu vực."));
    subtitleLbl->setStyleSheet(QStringLiteral("color: #94a3b8; font-size: 10px;"));
    titles->addWidget(titleLbl);
    titles->addWidget(subtitleLbl);
    topBar->addLayout(titles, 1);

    auto *refreshBtn = new QPushButton(tr("Làm mới"), this);
    refreshBtn->setCursor(Qt::PointingHandCursor);
    refreshBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #1e293b; color: #cbd5e1; border: 1px solid #334155; "
        "border-radius: 6px; font-size: 10px; font-weight: 700; padding: 5px 12px; }"
        "QPushButton:hover { background: #334155; color: #ffffff; }"
    ));
    connect(refreshBtn, &QPushButton::clicked, this, &UserManagementPage::refreshRequested);
    topBar->addWidget(refreshBtn);

    m_addUserBtn = new QPushButton(tr("+ Thêm Người Dùng"), this);
    m_addUserBtn->setCursor(Qt::PointingHandCursor);
    m_addUserBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background: #f59e0b; color: #020617; border: none; "
        "border-radius: 6px; font-size: 10px; font-weight: 800; padding: 5px 14px; }"
        "QPushButton:hover { background: #fbbf24; }"
        "QPushButton:disabled { background: #334155; color: #64748b; }"
    ));
    connect(m_addUserBtn, &QPushButton::clicked, this, [this] {
        openEditDialog();
    });
    topBar->addWidget(m_addUserBtn);

    mainLayout->addLayout(topBar);

    // ==========================================
    // STAT KPI CARDS (3 Cards)
    // ==========================================
    auto *kpiRow = new QHBoxLayout;
    kpiRow->setSpacing(8);

    auto makeKpiCard = [](const QString &tag, const QString &label, QLabel *&valOut, const QString &accent) {
        auto *frame = new QFrame;
        frame->setStyleSheet(QStringLiteral(
            "QFrame { background: #0f172a; border: 1px solid #1e293b; border-radius: 8px; }"));
        auto *l = new QHBoxLayout(frame);
        l->setContentsMargins(12, 8, 12, 8);
        l->setSpacing(8);

        auto *ic = new QLabel(tag);
        ic->setStyleSheet(QStringLiteral("font-size: 10px; font-weight: 900; color: #94a3b8; background: #1e293b; border-radius: 4px; padding: 4px 6px;"));
        l->addWidget(ic);

        auto *txt = new QVBoxLayout;
        txt->setSpacing(0);
        auto *lbl = new QLabel(label);
        lbl->setStyleSheet(QStringLiteral("font-size: 9px; color: #94a3b8; font-weight: 700;"));
        valOut = new QLabel(QStringLiteral("0"));
        valOut->setStyleSheet(QStringLiteral("font-size: 15px; font-weight: 900; color: %1;").arg(accent));
        txt->addWidget(lbl);
        txt->addWidget(valOut);
        l->addLayout(txt);
        l->addStretch();
        return frame;
    };

    kpiRow->addWidget(makeKpiCard(QStringLiteral("ALL"), tr("TỔNG TÀI KHOẢN"), m_statTotal, QStringLiteral("#38bdf8")), 1);
    kpiRow->addWidget(makeKpiCard(QStringLiteral("ADM"), tr("QUẢN TRỊ VIÊN"), m_statAdmin, QStringLiteral("#f59e0b")), 1);
    kpiRow->addWidget(makeKpiCard(QStringLiteral("USR"), tr("NGƯỜI DÙNG THƯỜNG"), m_statRegular, QStringLiteral("#10b981")), 1);
    mainLayout->addLayout(kpiRow);

    // ==========================================
    // TOOLBAR: Filters & Search
    // ==========================================
    auto *filterBar = new QHBoxLayout;
    filterBar->setSpacing(8);

    auto makeFilterBtn = [&](const QString &label, const QString &mode) {
        auto *btn = new QPushButton(label, this);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setCheckable(true);
        btn->setStyleSheet(QStringLiteral(
            "QPushButton { background: #0f172a; color: #64748b; border: 1px solid #1e293b; "
            "border-radius: 6px; font-size: 10px; font-weight: 700; padding: 4px 12px; }"
            "QPushButton:hover { background: #1e293b; color: #ffffff; }"
            "QPushButton:checked { background: #1e293b; color: #f59e0b; border-color: #f59e0b; }"
        ));
        connect(btn, &QPushButton::clicked, this, [this, mode] {
            m_currentFilter = mode;
            m_filterAllBtn->setChecked(mode == QStringLiteral("all"));
            m_filterAdminBtn->setChecked(mode == QStringLiteral("admin"));
            m_filterUserBtn->setChecked(mode == QStringLiteral("user"));
            renderUserTable();
        });
        return btn;
    };

    m_filterAllBtn = makeFilterBtn(tr("Tất cả (0)"), QStringLiteral("all"));
    m_filterAdminBtn = makeFilterBtn(tr("Admin (0)"), QStringLiteral("admin"));
    m_filterUserBtn = makeFilterBtn(tr("User (0)"), QStringLiteral("user"));
    m_filterAllBtn->setChecked(true);

    filterBar->addWidget(m_filterAllBtn);
    filterBar->addWidget(m_filterAdminBtn);
    filterBar->addWidget(m_filterUserBtn);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("🔍 Tìm theo tên tài khoản..."));
    m_searchEdit->setStyleSheet(QStringLiteral(
        "QLineEdit { background: #0f172a; color: #f8fafc; border: 1px solid #1e293b; "
        "border-radius: 6px; padding: 4px 10px; font-size: 10px; }"
        "QLineEdit:focus { border-color: #f59e0b; }"
    ));
    VirtualKeyboardDialog::attachToLineEdit(m_searchEdit, tr("Tìm tài khoản"));
    connect(m_searchEdit, &QLineEdit::textChanged, this, &UserManagementPage::renderUserTable);
    filterBar->addWidget(m_searchEdit, 1);

    mainLayout->addLayout(filterBar);

    // ==========================================
    // USER TABLE
    // ==========================================
    m_userTable = new QTableWidget(this);
    m_userTable->setColumnCount(5);
    m_userTable->setHorizontalHeaderLabels({
        tr("Tên Đăng Nhập"), tr("Vai Trò"), tr("Trạng Thái"),
        tr("Thiết Bị Liên Kết"), tr("Thao Tác")
    });
    m_userTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_userTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_userTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_userTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_userTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_userTable->verticalHeader()->hide();
    m_userTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_userTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_userTable->setStyleSheet(QStringLiteral(
        "QTableWidget { background-color: #0b1329; alternate-background-color: #111c38; border: 1px solid #1e293b; border-radius: 6px; "
        "gridline-color: #1e293b; color: #f8fafc; font-size: 11px; }"
        "QHeaderView::section { background-color: #080d1a; color: #94a3b8; font-weight: 800; "
        "padding: 6px 8px; border: none; border-bottom: 1.5px solid #1e293b; font-size: 10px; }"
        "QTableWidget::item { padding: 5px 8px; color: #f8fafc; }"
        "QTableWidget::item:alternate { background-color: #111c38; color: #f8fafc; }"
        "QTableWidget::item:selected { background-color: #0284c7; color: #ffffff; }"
    ));
    mainLayout->addWidget(m_userTable, 1);

    m_emptyLabel = new QLabel(tr("Không tìm thấy tài khoản phù hợp."), this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet(QStringLiteral("color: #64748b; font-style: italic; font-size: 11px; padding: 20px;"));
    m_emptyLabel->hide();
    mainLayout->addWidget(m_emptyLabel);
}

void UserManagementPage::renderUserTable()
{
    if (!m_userTable)
        return;

    const QString search = m_searchEdit ? m_searchEdit->text().trimmed().toLower() : QString();
    m_userTable->setRowCount(0);

    int rowIdx = 0;
    for (const auto &val : m_users) {
        const auto u = val.toObject();
        const QString username = u.value(QStringLiteral("username")).toString();
        const QString role = u.value(QStringLiteral("role")).toString();
        const bool enabled = u.value(QStringLiteral("enabled")).toBool(true);
        const QJsonArray devices = u.value(QStringLiteral("devices")).toArray();

        // Filters
        if (m_currentFilter == QStringLiteral("admin") && role != QStringLiteral("admin"))
            continue;
        if (m_currentFilter == QStringLiteral("user") && role == QStringLiteral("admin"))
            continue;
        if (!search.isEmpty() && !username.toLower().contains(search))
            continue;

        m_userTable->insertRow(rowIdx);

        // 0: Username
        auto *nameItem = new QTableWidgetItem(username);
        nameItem->setFont(QFont(QStringLiteral("Segoe UI"), 10, QFont::Bold));
        m_userTable->setItem(rowIdx, 0, nameItem);

        // 1: Role
        const bool isAdmin = (role == QStringLiteral("admin"));
        auto *roleItem = new QTableWidgetItem(isAdmin ? tr("Quản trị viên") : tr("Người dùng"));
        roleItem->setForeground(isAdmin ? QColor("#f59e0b") : QColor("#38bdf8"));
        roleItem->setTextAlignment(Qt::AlignCenter);
        m_userTable->setItem(rowIdx, 1, roleItem);

        // 2: Enabled
        auto *statusItem = new QTableWidgetItem(enabled ? tr("● Hoạt động") : tr("○ Đã khóa"));
        statusItem->setForeground(enabled ? QColor("#10b981") : QColor("#ef4444"));
        statusItem->setTextAlignment(Qt::AlignCenter);
        m_userTable->setItem(rowIdx, 2, statusItem);

        // 3: Devices
        QStringList devNames;
        for (const auto &d : devices) {
            const auto devObj = d.toObject();
            devNames << devObj.value(QStringLiteral("name")).toString(
                devObj.value(QStringLiteral("device_id")).toString());
        }
        const QString devSummary = devNames.isEmpty() ? tr("Chưa liên kết thiết bị") : devNames.join(QStringLiteral(", "));
        auto *devItem = new QTableWidgetItem(devSummary);
        m_userTable->setItem(rowIdx, 3, devItem);

        // 4: Actions (Edit, Delete)
        auto *actionWidget = new QWidget(m_userTable);
        auto *actLayout = new QHBoxLayout(actionWidget);
        actLayout->setContentsMargins(4, 2, 4, 2);
        actLayout->setSpacing(6);

        auto *editBtn = new QPushButton(tr("Sửa"), actionWidget);
        editBtn->setCursor(Qt::PointingHandCursor);
        editBtn->setStyleSheet(QStringLiteral(
            "QPushButton { background: #1e293b; color: #38bdf8; border: 1px solid #0284c7; "
            "border-radius: 4px; font-size: 9px; font-weight: 700; padding: 2px 8px; }"
            "QPushButton:hover { background: #0284c7; color: #ffffff; }"
        ));
        editBtn->setEnabled(m_adminEnabled);
        connect(editBtn, &QPushButton::clicked, this, [this, u] {
            openEditDialog(u);
        });
        actLayout->addWidget(editBtn);

        auto *delBtn = new QPushButton(tr("Xóa"), actionWidget);
        delBtn->setCursor(Qt::PointingHandCursor);
        delBtn->setStyleSheet(QStringLiteral(
            "QPushButton { background: #1e293b; color: #f43f5e; border: 1px solid #e11d48; "
            "border-radius: 4px; font-size: 9px; font-weight: 700; padding: 2px 8px; }"
            "QPushButton:hover { background: #e11d48; color: #ffffff; }"
            "QPushButton:disabled { color: #475569; border-color: #334155; }"
        ));
        delBtn->setEnabled(m_adminEnabled && username != QStringLiteral("admin"));
        connect(delBtn, &QPushButton::clicked, this, [this, u] {
            confirmDeleteUser(u);
        });
        actLayout->addWidget(delBtn);

        m_userTable->setCellWidget(rowIdx, 4, actionWidget);
        rowIdx++;
    }

    m_emptyLabel->setVisible(rowIdx == 0);
    m_userTable->setVisible(rowIdx > 0);
}

void UserManagementPage::openEditDialog(const QJsonObject &user)
{
    const bool isEdit = !user.isEmpty();
    const QString oldUsername = user.value(QStringLiteral("username")).toString();
    const QString oldRole = user.value(QStringLiteral("role")).toString(QStringLiteral("user"));
    const bool oldEnabled = user.value(QStringLiteral("enabled")).toBool(true);
    const QJsonArray devices = user.value(QStringLiteral("devices")).toArray();

    QDialog dlg(this);
    dlg.setWindowTitle(isEdit ? tr("Chỉnh Sửa Tài Khoản") : tr("Tạo Tài Khoản Mới"));
    dlg.setModal(true);
    dlg.setFixedWidth(420);
    dlg.setStyleSheet(QStringLiteral(
        "QDialog { background-color: #0b1329; color: #f8fafc; font-family: 'Segoe UI', sans-serif; }"
        "QLabel { color: #e2e8f0; font-size: 10px; font-weight: 600; }"
        "QLineEdit, QComboBox { background: #1e293b; color: #f8fafc; border: 1px solid #334155; "
        "border-radius: 6px; padding: 5px 8px; font-size: 11px; }"
        "QCheckBox { color: #f8fafc; font-size: 10px; font-weight: 700; }"
    ));

    auto *root = new QVBoxLayout(&dlg);
    root->setContentsMargins(18, 16, 18, 16);
    root->setSpacing(12);

    auto *head = new QLabel(isEdit ? tr("CẬP NHẬT TÀI KHOẢN: %1").arg(oldUsername)
                                   : tr("THÊM TÀI KHOẢN MỚI"), &dlg);
    head->setStyleSheet(QStringLiteral("font-size: 12px; font-weight: 800; color: #f59e0b;"));
    root->addWidget(head);

    auto *form = new QFormLayout;
    form->setSpacing(8);

    auto *userEdit = new QLineEdit(&dlg);
    userEdit->setText(oldUsername);
    form->addRow(tr("Tên đăng nhập:"), userEdit);
    VirtualKeyboardDialog::attachToLineEdit(userEdit, tr("Tên đăng nhập"));

    auto *passEdit = new QLineEdit(&dlg);
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setPlaceholderText(isEdit ? tr("(Để trống nếu không đổi mật khẩu)") : tr("Mật khẩu"));
    form->addRow(tr("Mật khẩu:"), passEdit);
    VirtualKeyboardDialog::attachToLineEdit(passEdit, tr("Mật khẩu"));

    auto *roleCombo = new QComboBox(&dlg);
    roleCombo->addItem(tr("Người dùng (User)"), QStringLiteral("user"));
    roleCombo->addItem(tr("Quản trị viên (Admin)"), QStringLiteral("admin"));
    roleCombo->setCurrentIndex(oldRole == QStringLiteral("admin") ? 1 : 0);
    form->addRow(tr("Vai trò:"), roleCombo);

    auto *enabledCheck = new QCheckBox(tr("Kích hoạt tài khoản này"), &dlg);
    enabledCheck->setChecked(oldEnabled);
    form->addRow(enabledCheck);

    root->addLayout(form);

    // Linked Devices section if editing
    if (isEdit && !devices.isEmpty()) {
        auto *devTitle = new QLabel(tr("Thiết bị ánh sáng đã gán cho tài khoản:"), &dlg);
        devTitle->setStyleSheet(QStringLiteral("font-size: 10px; font-weight: 700; color: #38bdf8; margin-top: 4px;"));
        root->addWidget(devTitle);

        for (const auto &d : devices) {
            const auto dObj = d.toObject();
            const QString devId = dObj.value(QStringLiteral("device_id")).toString();
            const QString devName = dObj.value(QStringLiteral("name")).toString(devId);

            auto *dRow = new QHBoxLayout;
            auto *dLbl = new QLabel(tr("[LIGHT] %1 (%2)").arg(devName, devId), &dlg);
            dLbl->setStyleSheet(QStringLiteral("font-size: 10px; color: #cbd5e1;"));
            dRow->addWidget(dLbl, 1);

            auto *relBtn = new QPushButton(tr("Gỡ thiết bị"), &dlg);
            relBtn->setStyleSheet(QStringLiteral(
                "background: #1e293b; color: #f43f5e; border: 1px solid #e11d48; "
                "border-radius: 4px; font-size: 8px; font-weight: 700; padding: 2px 6px;"));
            connect(relBtn, &QPushButton::clicked, this, [this, oldUsername, devId, &dlg] {
                emit releaseUserDeviceRequested(oldUsername, devId);
                dlg.accept();
            });
            dRow->addWidget(relBtn);
            root->addLayout(dRow);
        }
    }

    // Buttons
    auto *btns = new QHBoxLayout;
    btns->setSpacing(8);
    auto *cancel = new QPushButton(tr("Hủy"), &dlg);
    cancel->setStyleSheet(QStringLiteral(
        "background: #1e293b; color: #94a3b8; border-radius: 6px; padding: 6px 12px; font-size: 10px;"));
    auto *save = new QPushButton(tr("Lưu Thông Tin"), &dlg);
    save->setStyleSheet(QStringLiteral(
        "background: #f59e0b; color: #020617; font-weight: 800; border-radius: 6px; padding: 6px 16px; font-size: 10px;"));
    btns->addStretch();
    btns->addWidget(cancel);
    btns->addWidget(save);
    root->addLayout(btns);

    connect(cancel, &QPushButton::clicked, &dlg, &QDialog::reject);
    connect(save, &QPushButton::clicked, &dlg, [this, isEdit, oldUsername, userEdit, passEdit, roleCombo, enabledCheck, &dlg] {
        const QString uName = userEdit->text().trimmed();
        const QString pass = passEdit->text();
        const QString role = roleCombo->currentData().toString();
        const bool en = enabledCheck->isChecked();

        if (uName.isEmpty()) {
            QMessageBox::warning(&dlg, tr("Thiếu thông tin"), tr("Vui lòng nhập tên đăng nhập."));
            return;
        }

        if (isEdit) {
            emit updateUserRequested(oldUsername, uName, pass, role, en);
        } else {
            if (pass.isEmpty()) {
                QMessageBox::warning(&dlg, tr("Thiếu thông tin"), tr("Vui lòng nhập mật khẩu cho tài khoản mới."));
                return;
            }
            emit createUserRequested(uName, pass, role);
        }
        dlg.accept();
    });

    dlg.exec();
}

void UserManagementPage::confirmDeleteUser(const QJsonObject &user)
{
    const QString username = user.value(QStringLiteral("username")).toString();
    if (username.isEmpty() || username == QStringLiteral("admin"))
        return;

    if (QMessageBox::question(this, tr("Xác nhận xóa tài khoản"),
                              tr("Bạn có chắc chắn muốn xóa tài khoản '%1'?\nCác thiết bị liên kết với tài khoản này sẽ được giải phóng.")
                              .arg(username)) == QMessageBox::Yes) {
        emit deleteUserRequested(username);
    }
}
