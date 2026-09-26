#include "LicenseKeyPage.h"
#include "auth/LicenseManager.h"
#include "auth/HwidHelper.h"
#include "ui/CustomMessageBox.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QClipboard>
#include <QApplication>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPixmap>
#include <QDateTime>

class PlanCardClickFrame : public QFrame {
public:
    PlanCardClickFrame(int index, std::function<void(int)> onClick, QWidget *parent = nullptr)
        : QFrame(parent), m_index(index), m_onClick(onClick) {
        setCursor(Qt::PointingHandCursor);
    }
protected:
    void mousePressEvent(QMouseEvent *event) override {
        QFrame::mousePressEvent(event);
        if (m_onClick) m_onClick(m_index);
    }
private:
    int m_index;
    std::function<void(int)> m_onClick;
};

LicenseKeyPage::LicenseKeyPage(QWidget *parent)
    : QWidget(parent)
{
    m_nam = new QNetworkAccessManager(this);
    m_plans = {
        {"Gói 1 Tháng", "30 ngày sử dụng", 1, 150000, "", false},
        {"Gói 3 Tháng", "90 ngày sử dụng", 3, 390000, "Tiết kiệm 15%", false},
        {"Gói 1 Năm (12 Tháng)", "365 ngày sử dụng", 12, 1200000, "🔥 Tiết kiệm 35%", true},
        {"Gói Vĩnh Viễn (Lifetime)", "Sở hữu trọn đời", 9999, 2500000, "👑 VIP TRỌN ĐỜI", false}
    };
    setupUi();
    refreshLicenseStatus();
}

void LicenseKeyPage::setupUi() {
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet("QScrollArea { background-color: #f8fafc; border: none; }");

    auto *contentWidget = new QWidget();
    auto *mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setContentsMargins(24, 20, 24, 24);
    mainLayout->setSpacing(18);

    // 1. TOP HEADER BANNER
    auto *headerCard = new QWidget();
    headerCard->setStyleSheet("background-color: #ffffff; border: 1px solid #e2e8f0; border-radius: 14px; padding: 14px 20px;");
    auto *headerLayout = new QHBoxLayout(headerCard);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(14);

    auto *headerIcon = new QLabel("💳");
    headerIcon->setStyleSheet("font-size: 26px; background-color: #fff7ed; border-radius: 12px; padding: 8px 12px;");
    headerLayout->addWidget(headerIcon);

    auto *titleCol = new QVBoxLayout();
    titleCol->setSpacing(3);
    auto *titleLbl = new QLabel("Quản Lý Bản Quyền & Gia Hạn Trực Tuyến");
    titleLbl->setStyleSheet("font-size: 19px; font-weight: 800; color: #0f172a;");
    auto *subTitleLbl = new QLabel("Hệ thống bản quyền liên kết theo mã phần cứng (HWID) máy tính. Quét mã VietQR để gia hạn tự động 24/7.");
    subTitleLbl->setStyleSheet("font-size: 13px; color: #64748b;");
    titleCol->addWidget(titleLbl);
    titleCol->addWidget(subTitleLbl);
    headerLayout->addLayout(titleCol, 1);

    auto *statusCol = new QVBoxLayout();
    statusCol->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    statusCol->setSpacing(4);

    m_lblStatusBadge = new QLabel("🟢 ĐÃ KÍCH HOẠT (VIP PRO)");
    m_lblStatusBadge->setStyleSheet("background: #ecfdf5; color: #047857; font-weight: 700; border: 1px solid #a7f3d0; border-radius: 14px; padding: 6px 14px; font-size: 12px;");
    m_lblDaysLeft = new QLabel("Còn 365 ngày");
    m_lblDaysLeft->setAlignment(Qt::AlignRight);
    m_lblDaysLeft->setStyleSheet("font-size: 12px; font-weight: 600; color: #64748b;");

    statusCol->addWidget(m_lblStatusBadge);
    statusCol->addWidget(m_lblDaysLeft);
    headerLayout->addLayout(statusCol);

    mainLayout->addWidget(headerCard);

    // 2. MAIN 2 COLUMNS
    auto *columnsLayout = new QHBoxLayout();
    columnsLayout->setSpacing(18);

    // LEFT COLUMN
    auto *leftColWidget = new QWidget();
    auto *leftColLayout = new QVBoxLayout(leftColWidget);
    leftColLayout->setContentsMargins(0, 0, 0, 0);
    leftColLayout->setSpacing(16);

    // Card 1: License Info
    auto *infoCard = new QWidget();
    infoCard->setStyleSheet("background: #ffffff; border: 1px solid #e2e8f0; border-radius: 14px; padding: 18px;");
    auto *infoCardLayout = new QVBoxLayout(infoCard);
    infoCardLayout->setContentsMargins(0, 0, 0, 0);
    infoCardLayout->setSpacing(12);

    auto *infoTitle = new QLabel("🛡️ Thông Tin Bản Quyền Hiện Tại");
    infoTitle->setStyleSheet("font-size: 14px; font-weight: 800; color: #0f172a;");
    infoCardLayout->addWidget(infoTitle);

    auto *infoGrid = new QGridLayout();
    infoGrid->setHorizontalSpacing(14);
    infoGrid->setVerticalSpacing(8);

    auto addInfoRow = [&](int row, const QString &label, QLabel *&valLabel, const QString &defaultVal) {
        auto *lbl = new QLabel(label);
        lbl->setStyleSheet("font-size: 13px; color: #64748b; font-weight: 500;");
        valLabel = new QLabel(defaultVal);
        valLabel->setStyleSheet("font-size: 13px; color: #0f172a; font-weight: 700;");
        infoGrid->addWidget(lbl, row, 0);
        infoGrid->addWidget(valLabel, row, 1);
    };

    addInfoRow(0, "Gói Đang Dùng:", m_lblCurrentPlan, "VIP Pro (1 Năm)");
    addInfoRow(1, "Ngày Hết Hạn:", m_lblExpiryDate, "25/09/2027 23:59");
    infoCardLayout->addLayout(infoGrid);
    leftColLayout->addWidget(infoCard);

    // Card 2: HWID & Key Input
    auto *hwidCard = new QWidget();
    hwidCard->setStyleSheet("background: #ffffff; border: 1px solid #e2e8f0; border-radius: 14px; padding: 18px;");
    auto *hwidLayout = new QVBoxLayout(hwidCard);
    hwidLayout->setContentsMargins(0, 0, 0, 0);
    hwidLayout->setSpacing(10);

    auto *hwidTitle = new QLabel("💻 Mã Máy Tính (Hardware ID):");
    hwidTitle->setStyleSheet("font-size: 13px; font-weight: 700; color: #0f172a;");
    hwidLayout->addWidget(hwidTitle);

    auto *hwidRow = new QHBoxLayout();
    m_lblHwid = new QLabel(LicenseManager::instance()->getHwid());
    m_lblHwid->setStyleSheet("background: #f1f5f9; border: 1px solid #cbd5e1; border-radius: 6px; padding: 8px 12px; font-family: monospace; font-size: 13px; color: #0f172a; font-weight: 700;");
    hwidRow->addWidget(m_lblHwid, 1);

    auto *btnCopyHwid = new QPushButton("📋 Copy");
    btnCopyHwid->setCursor(Qt::PointingHandCursor);
    btnCopyHwid->setFixedHeight(36);
    btnCopyHwid->setStyleSheet("QPushButton { background: #0f172a; color: #ffffff; font-weight: 700; border-radius: 6px; padding: 0 16px; font-size: 12px; } QPushButton:hover { background: #1e293b; }");
    connect(btnCopyHwid, &QPushButton::clicked, this, &LicenseKeyPage::onCopyHwidClicked);
    hwidRow->addWidget(btnCopyHwid);
    hwidLayout->addLayout(hwidRow);

    hwidLayout->addSpacing(6);
    auto *keyTitle = new QLabel("🔑 Nhập Mã Bản Quyền (License Key):");
    keyTitle->setStyleSheet("font-size: 13px; font-weight: 700; color: #0f172a;");
    hwidLayout->addWidget(keyTitle);

    auto *keyRow = new QHBoxLayout();
    m_txtKeyInput = new QLineEdit();
    m_txtKeyInput->setPlaceholderText("Dán key bản quyền (TUNN-XXXX-XXXX-XXXX)...");
    m_txtKeyInput->setFixedHeight(38);
    m_txtKeyInput->setStyleSheet("background: #ffffff; border: 1px solid #cbd5e1; border-radius: 6px; padding: 6px 12px; font-size: 13px;");
    keyRow->addWidget(m_txtKeyInput, 1);

    m_btnActivate = new QPushButton("⚡ Kích Hoạt Key");
    m_btnActivate->setCursor(Qt::PointingHandCursor);
    m_btnActivate->setFixedHeight(38);
    m_btnActivate->setStyleSheet("QPushButton { background: #f97316; color: #ffffff; font-weight: 700; border-radius: 6px; padding: 0 16px; font-size: 13px; border: none; } QPushButton:hover { background: #ea580c; }");
    connect(m_btnActivate, &QPushButton::clicked, this, &LicenseKeyPage::onActivateKeyClicked);
    keyRow->addWidget(m_btnActivate);
    hwidLayout->addLayout(keyRow);

    leftColLayout->addWidget(hwidCard);
    leftColLayout->addStretch();
    columnsLayout->addWidget(leftColWidget, 1);

    // RIGHT COLUMN: Plan Picker & VietQR
    auto *rightColWidget = new QWidget();
    auto *rightColLayout = new QVBoxLayout(rightColWidget);
    rightColLayout->setContentsMargins(0, 0, 0, 0);
    rightColLayout->setSpacing(16);

    // Plan Picker Card
    auto *planCardWidget = new QWidget();
    planCardWidget->setStyleSheet("background: #ffffff; border: 1px solid #e2e8f0; border-radius: 14px; padding: 18px;");
    auto *planLayout = new QVBoxLayout(planCardWidget);
    planLayout->setContentsMargins(0, 0, 0, 0);
    planLayout->setSpacing(12);

    auto *planTitle = new QLabel("1. Chọn Gói Cước Gia Hạn Trực Tuyến");
    planTitle->setStyleSheet("font-size: 14px; font-weight: 800; color: #0f172a;");
    planLayout->addWidget(planTitle);

    auto *planGrid = new QGridLayout();
    planGrid->setSpacing(10);
    m_planCardFrames.clear();

    auto formatVnd = [](qint64 amount) -> QString {
        QString s = QString::number(amount);
        int pos = s.length() - 3;
        while (pos > 0) {
            s.insert(pos, ".");
            pos -= 3;
        }
        return s + " đ";
    };

    for (int i = 0; i < m_plans.size(); ++i) {
        const auto &p = m_plans[i];
        auto *f = new PlanCardClickFrame(i, [this](int idx) { onSelectPlan(idx); });
        f->setObjectName("planFrame");
        f->setFixedHeight(85);

        auto *l = new QVBoxLayout(f);
        l->setContentsMargins(12, 8, 12, 8);
        l->setSpacing(2);

        auto *t = new QLabel(p.title);
        t->setStyleSheet("font-size: 13px; font-weight: 800; color: #0f172a; border: none; background: transparent;");
        auto *pr = new QLabel(formatVnd(p.priceVnd));
        pr->setStyleSheet("font-size: 15px; font-weight: 800; color: #f97316; border: none; background: transparent;");
        auto *d = new QLabel(p.durationDesc);
        d->setStyleSheet("font-size: 11px; color: #64748b; border: none; background: transparent;");

        l->addWidget(t);
        l->addWidget(pr);
        l->addWidget(d);

        m_planCardFrames.append(f);
        planGrid->addWidget(f, i / 2, i % 2);
    }
    planLayout->addLayout(planGrid);
    rightColLayout->addWidget(planCardWidget);

    // VietQR Card
    auto *qrCard = new QWidget();
    qrCard->setStyleSheet("background: #ffffff; border: 1px solid #e2e8f0; border-radius: 14px; padding: 18px;");
    auto *qrLayout = new QVBoxLayout(qrCard);
    qrLayout->setContentsMargins(0, 0, 0, 0);
    qrLayout->setSpacing(12);

    auto *qrTitle = new QLabel("2. Quét Mã VietQR Ngân Hàng Hoặc Chuyển Khoản 24/7");
    qrTitle->setStyleSheet("font-size: 14px; font-weight: 800; color: #0f172a;");
    qrLayout->addWidget(qrTitle);

    auto *qrRow = new QHBoxLayout();
    qrRow->setSpacing(16);

    // QR Image Box
    auto *qrImageBox = new QWidget();
    qrImageBox->setFixedWidth(160);
    auto *qrImageBoxLy = new QVBoxLayout(qrImageBox);
    qrImageBoxLy->setContentsMargins(0, 0, 0, 0);
    qrImageBoxLy->setSpacing(4);

    m_lblQrImage = new QLabel();
    m_lblQrImage->setFixedSize(150, 150);
    m_lblQrImage->setAlignment(Qt::AlignCenter);
    m_lblQrImage->setStyleSheet("background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 8px; color: #64748b; font-size: 11px;");
    m_lblQrImage->setText("⏳ Đang tải mã VietQR...");
    qrImageBoxLy->addWidget(m_lblQrImage, 0, Qt::AlignCenter);

    auto *qrSub = new QLabel("🏦 MB Bank - VietQR");
    qrSub->setAlignment(Qt::AlignCenter);
    qrSub->setStyleSheet("font-size: 11px; font-weight: 700; color: #1e3a8a;");
    qrImageBoxLy->addWidget(qrSub);

    qrRow->addWidget(qrImageBox);

    // Bank details
    auto *bankDetails = new QWidget();
    auto *bankLy = new QVBoxLayout(bankDetails);
    bankLy->setContentsMargins(0, 0, 0, 0);
    bankLy->setSpacing(6);

    auto addBankItem = [&](const QString &lbl, QLabel *&valLbl, const QString &val) {
        auto *w = new QWidget();
        w->setStyleSheet("background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 6px; padding: 3px 8px;");
        auto *ly = new QHBoxLayout(w);
        ly->setContentsMargins(0, 0, 0, 0);
        auto *t = new QLabel(lbl);
        t->setFixedWidth(80);
        t->setStyleSheet("font-size: 11px; color: #64748b; font-weight: 600;");
        valLbl = new QLabel(val);
        valLbl->setStyleSheet("font-size: 12.5px; color: #0f172a; font-weight: 800;");
        auto *btnCp = new QPushButton("📋");
        btnCp->setFixedSize(24, 24);
        btnCp->setCursor(Qt::PointingHandCursor);
        btnCp->setStyleSheet("QPushButton { border: 1px solid #cbd5e1; border-radius: 4px; background: #ffffff; }");
        connect(btnCp, &QPushButton::clicked, [valLbl]() {
            QApplication::clipboard()->setText(valLbl->text());
        });
        ly->addWidget(t);
        ly->addWidget(valLbl, 1);
        ly->addWidget(btnCp);
        bankLy->addWidget(w);
    };

    addBankItem("Ngân hàng:", m_lblBankName, "MB Bank (Quân Đội)");
    addBankItem("Số tài khoản:", m_lblAccountNo, "0988888888");
    addBankItem("Chủ tài khoản:", m_lblAccountHolder, "NGUYEN VAN TUN");
    addBankItem("Số tiền:", m_lblTransferAmount, "1.200.000 đ");
    addBankItem("Nội dung CK:", m_lblTransferSyntax, getTransferSyntax());

    qrRow->addWidget(bankDetails, 1);
    qrLayout->addLayout(qrRow);

    auto *btnCheckPayment = new QPushButton("🔄 Tôi Đã Chuyển Khoản - Kiểm Tra Ngay");
    btnCheckPayment->setCursor(Qt::PointingHandCursor);
    btnCheckPayment->setFixedHeight(40);
    btnCheckPayment->setStyleSheet("QPushButton { background: #10b981; color: #ffffff; font-weight: 800; border-radius: 8px; font-size: 13px; } QPushButton:hover { background: #059669; }");
    connect(btnCheckPayment, &QPushButton::clicked, this, &LicenseKeyPage::onCheckPaymentClicked);
    qrLayout->addWidget(btnCheckPayment);

    rightColLayout->addWidget(qrCard);
    rightColLayout->addStretch();
    columnsLayout->addWidget(rightColWidget, 1);

    mainLayout->addLayout(columnsLayout);
    scrollArea->setWidget(contentWidget);
    outerLayout->addWidget(scrollArea);

    onSelectPlan(m_selectedPlanIndex);
}

QString LicenseKeyPage::getTransferSyntax() const {
    return QString("TUNN %1").arg(LicenseManager::instance()->getHwid());
}

void LicenseKeyPage::onSelectPlan(int planIndex) {
    if (planIndex < 0 || planIndex >= m_plans.size()) return;
    m_selectedPlanIndex = planIndex;

    for (int i = 0; i < m_planCardFrames.size(); ++i) {
        if (i == m_selectedPlanIndex) {
            m_planCardFrames[i]->setStyleSheet("#planFrame { background-color: #fff7ed; border: 2px solid #f97316; border-radius: 10px; }");
        } else {
            m_planCardFrames[i]->setStyleSheet("#planFrame { background-color: #ffffff; border: 1.5px solid #e2e8f0; border-radius: 10px; } #planFrame:hover { background-color: #f8fafc; }");
        }
    }

    updateQrCode();
}

void LicenseKeyPage::updateQrCode() {
    if (m_selectedPlanIndex < 0 || m_selectedPlanIndex >= m_plans.size()) return;
    const auto &plan = m_plans[m_selectedPlanIndex];

    QString syntax = getTransferSyntax();
    if (m_lblTransferSyntax) m_lblTransferSyntax->setText(syntax);

    auto formatVnd = [](qint64 amount) -> QString {
        QString s = QString::number(amount);
        int pos = s.length() - 3;
        while (pos > 0) {
            s.insert(pos, ".");
            pos -= 3;
        }
        return s + " đ";
    };

    if (m_lblTransferAmount) m_lblTransferAmount->setText(formatVnd(plan.priceVnd));

    if (!m_lblQrImage) return;
    m_lblQrImage->setText("⏳ Đang tải QR...");

    QString encodedContent = QString::fromUtf8(QUrl::toPercentEncoding(syntax));
    QString qrUrl = QString("https://img.vietqr.io/image/MB-0988888888-compact2.png?amount=%1&addInfo=%2&accountName=NGUYEN%20VAN%20TUN")
                        .arg(plan.priceVnd)
                        .arg(encodedContent);

    QNetworkRequest request((QUrl(qrUrl)));
    QNetworkReply *reply = m_nam->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QPixmap pix;
            if (pix.loadFromData(data) && m_lblQrImage) {
                m_lblQrImage->setPixmap(pix.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }
        }
        reply->deleteLater();
    });
}

void LicenseKeyPage::refreshLicenseStatus() {
    auto *lm = LicenseManager::instance();
    if (m_lblStatusBadge) {
        m_lblStatusBadge->setText(lm->getStatusBadgeText());
        m_lblStatusBadge->setStyleSheet(lm->isActivated() && !lm->isExpired()
            ? "background: #ecfdf5; color: #047857; font-weight: 700; border: 1px solid #a7f3d0; border-radius: 14px; padding: 6px 14px; font-size: 12px;"
            : "background: #fef2f2; color: #b91c1c; font-weight: 700; border: 1px solid #fecaca; border-radius: 14px; padding: 6px 14px; font-size: 12px;");
    }
    if (m_lblDaysLeft) {
        m_lblDaysLeft->setText(lm->getExpiryDate().date().year() >= 2090 ? "👑 Trọn đời" : QString("Còn %1 ngày").arg(lm->daysRemaining()));
    }
    if (m_lblCurrentPlan) m_lblCurrentPlan->setText(lm->getPlanName());
    if (m_lblExpiryDate) {
        m_lblExpiryDate->setText(lm->getExpiryDate().date().year() >= 2090 ? "Vĩnh Viễn (Trọn Đời)" : lm->getExpiryDate().toString("dd/MM/yyyy HH:mm"));
    }
    if (m_lblHwid) m_lblHwid->setText(lm->getHwid());
}

void LicenseKeyPage::onCopyHwidClicked() {
    QApplication::clipboard()->setText(LicenseManager::instance()->getHwid());
    CustomMessageBox::information(this, "Đã Copy", "Đã copy mã máy tính (HWID) vào Clipboard!");
}

void LicenseKeyPage::onActivateKeyClicked() {
    QString key = m_txtKeyInput->text().trimmed();
    if (key.isEmpty()) {
        CustomMessageBox::warning(this, "Thông báo", "Vui lòng nhập mã bản quyền!");
        return;
    }
    if (LicenseManager::instance()->activate(key)) {
        refreshLicenseStatus();
        CustomMessageBox::information(this, "Thành công", "Kích hoạt bản quyền thành công!");
    } else {
        CustomMessageBox::critical(this, "Lỗi", "Mã kích hoạt không hợp lệ hoặc đã hết hạn!");
    }
}

void LicenseKeyPage::onCheckPaymentClicked() {
    CustomMessageBox::information(this, "Kiểm Tra Thanh Toán", "Hệ thống đang quét giao dịch ngân hàng...\nBản quyền sẽ tự kích hoạt sau khi tiền vào tài khoản.");
}

void LicenseKeyPage::onQrLoaded() {}
