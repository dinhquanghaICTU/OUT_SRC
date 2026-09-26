#ifndef LICENSEKEYPAGE_H
#define LICENSEKEYPAGE_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFrame>
#include <QList>
#include <QNetworkAccessManager>

struct LicenseSubscriptionPlan {
    QString title;
    QString durationDesc;
    int months = 1;
    qint64 priceVnd = 150000;
    QString badge;
    bool isPopular = false;
};

class LicenseKeyPage : public QWidget {
    Q_OBJECT

public:
    explicit LicenseKeyPage(QWidget *parent = nullptr);
    ~LicenseKeyPage() override = default;

    void refreshLicenseStatus();

private slots:
    void onCopyHwidClicked();
    void onActivateKeyClicked();
    void onSelectPlan(int planIndex);
    void onCheckPaymentClicked();
    void onQrLoaded();

private:
    void setupUi();
    void updateQrCode();
    QString getTransferSyntax() const;

    QNetworkAccessManager *m_nam = nullptr;
    QList<LicenseSubscriptionPlan> m_plans;
    int m_selectedPlanIndex = 2; // Default 1 year

    // UI Widgets
    QLabel *m_lblHwid = nullptr;
    QLabel *m_lblStatusBadge = nullptr;
    QLabel *m_lblDaysLeft = nullptr;
    QLabel *m_lblExpiryDate = nullptr;
    QLabel *m_lblCurrentPlan = nullptr;

    QLineEdit *m_txtKeyInput = nullptr;
    QPushButton *m_btnActivate = nullptr;

    // VietQR Payment UI
    QLabel *m_lblQrImage = nullptr;
    QLabel *m_lblBankName = nullptr;
    QLabel *m_lblAccountNo = nullptr;
    QLabel *m_lblAccountHolder = nullptr;
    QLabel *m_lblTransferAmount = nullptr;
    QLabel *m_lblTransferSyntax = nullptr;
    QList<QFrame*> m_planCardFrames;
};

#endif // LICENSEKEYPAGE_H
