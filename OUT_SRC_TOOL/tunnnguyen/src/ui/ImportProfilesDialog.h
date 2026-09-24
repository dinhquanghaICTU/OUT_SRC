#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QCheckBox>
#include <QStringList>
#include <QList>
#include "core/ChromeProfileItem.h"

class ImportProfilesDialog : public QDialog {
    Q_OBJECT
public:
    explicit ImportProfilesDialog(const QList<ChromeProfileItem> &existingProfiles, QWidget *parent = nullptr);
    ~ImportProfilesDialog() override = default;

    QList<ChromeProfileItem> getImportedProfiles() const;

    // Helper static function to export standard template CSV file with UTF-8 BOM
    static bool exportTemplateCsv(QWidget *parent = nullptr);

private slots:
    void onSelectFileClicked();
    void onPasteFromClipboardClicked();
    void onParseTextClicked();
    void onExportTemplateClicked();
    void onConfirmImportClicked();

private:
    void setupUi();
    void parseRawContent(const QString &content);
    void updatePreviewTable();
    int findNextAvailablePort(int basePort = 9222);

    QList<ChromeProfileItem> m_existingProfiles;
    QList<ChromeProfileItem> m_parsedProfiles;

    QTableWidget *m_previewTable = nullptr;
    QTextEdit *m_rawTextEdit = nullptr;
    QLabel *m_statusSummaryLabel = nullptr;
    QCheckBox *m_autoRenameDuplicatesCheck = nullptr;
    QPushButton *m_btnImport = nullptr;
    QWidget *m_pasteBoxWidget = nullptr;
    QPushButton *m_btnTogglePasteBox = nullptr;
};
