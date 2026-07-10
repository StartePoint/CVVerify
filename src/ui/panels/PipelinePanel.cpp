#include "ui/panels/PipelinePanel.h"

#include <QAbstractItemModel>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include "core/operators/OperatorDescriptor.h"

namespace {

constexpr int kOperatorIdRole = Qt::UserRole;
constexpr int kDefaultNameRole = Qt::UserRole + 1;
constexpr int kIsCategoryHeaderRole = Qt::UserRole + 2;
constexpr int kCategoryNameRole = Qt::UserRole + 3;

bool isAvailableOperatorItem(const QListWidgetItem* item)
{
    return item != nullptr
        && !item->data(kIsCategoryHeaderRole).toBool()
        && !item->data(kOperatorIdRole).toString().trimmed().isEmpty();
}

bool isChineseLanguage(const QString& languageCode)
{
    return languageCode.startsWith("zh", Qt::CaseInsensitive);
}

QString localizedCategoryName(const QString& categoryName, const QString& languageCode)
{
    if (isChineseLanguage(languageCode)) {
        return categoryName;
    }

    if (categoryName == QStringLiteral("\u5f62\u6001\u5b66\u5904\u7406")) return QString("Morphology");
    if (categoryName == QStringLiteral("\u6ee4\u6ce2")) return QString("Filtering");
    if (categoryName == QStringLiteral("\u56fe\u50cf\u589e\u5f3a\u4e0e\u53d8\u6362")) return QString("Enhancement & Transform");
    if (categoryName == QStringLiteral("\u4e8c\u503c\u5316")) return QString("Thresholding");
    if (categoryName == QStringLiteral("\u8fb9\u7f18\u68c0\u6d4b")) return QString("Edge Detection");
    if (categoryName == QStringLiteral("\u6ee4\u955c\u6548\u679c")) return QString("Filter Effects");
    if (categoryName == QStringLiteral("\u56fe\u50cf\u68c0\u6d4b")) return QString("Image Detection");
    if (categoryName == QStringLiteral("\u56fe\u50cf\u5206\u5272")) return QString("Segmentation");
    if (categoryName == QStringLiteral("\u7279\u5f81\u5339\u914d")) return QString("Feature Matching");
    return categoryName;
}

QString localizedOperatorName(const QString& operatorId, const QString& fallbackName, const QString& languageCode)
{
    if (!isChineseLanguage(languageCode)) {
        return fallbackName;
    }

    if (operatorId == "builtin.mean_blur") return QStringLiteral("\u5747\u503c\u6ee4\u6ce2");
    if (operatorId == "builtin.box_filter") return QStringLiteral("\u65b9\u6846\u6ee4\u6ce2");
    if (operatorId == "builtin.bilateral_filter") return QStringLiteral("\u53cc\u8fb9\u6ee4\u6ce2");
    if (operatorId == "builtin.gaussian_blur") return QStringLiteral("\u9ad8\u65af\u6a21\u7cca");
    if (operatorId == "builtin.brightness_contrast") return QStringLiteral("\u4eae\u5ea6\u4e0e\u5bf9\u6bd4\u5ea6");
    if (operatorId == "builtin.hist_equalize") return QStringLiteral("\u76f4\u65b9\u56fe\u5747\u8861\u5316");
    if (operatorId == "builtin.histogram_line") return QStringLiteral("\u76f4\u65b9\u56fe\uff08\u7ebf\u72b6\uff09");
    if (operatorId == "builtin.histogram_bar") return QStringLiteral("\u76f4\u65b9\u56fe\uff08\u67f1\u72b6\uff09");
    if (operatorId == "builtin.affine_transform") return QStringLiteral("\u4eff\u5c04\u53d8\u6362");
    if (operatorId == "builtin.polar_transform") return QStringLiteral("\u6781\u5750\u6807\u53d8\u6362");
    if (operatorId == "builtin.linear_abs_transform") return QStringLiteral("\u7ebf\u6027\u53d8\u6362");
    if (operatorId == "builtin.perspective_transform") return QStringLiteral("\u900f\u89c6\u53d8\u6362");
    if (operatorId == "builtin.resize") return QStringLiteral("\u7f29\u653e");
    if (operatorId == "builtin.morphology") return QStringLiteral("\u5f62\u6001\u5b66");
    if (operatorId == "builtin.median_blur") return QStringLiteral("\u4e2d\u503c\u6a21\u7cca");
    if (operatorId == "builtin.grayscale") return QStringLiteral("\u7070\u5ea6\u5316");
    if (operatorId == "builtin.color_convert") return QStringLiteral("\u989c\u8272\u8f6c\u6362");
    if (operatorId == "builtin.color_map") return QStringLiteral("\u5f69\u8272\u6620\u5c04");
    if (operatorId == "builtin.pencil_sketch") return QStringLiteral("\u624b\u5de5\u753b\u6548\u679c");
    if (operatorId == "builtin.art_effect") return QStringLiteral("\u827a\u672f\u6548\u679c");
    if (operatorId == "builtin.fiber_glow") return QStringLiteral("\u5149\u7ea4\u6548\u679c");
    if (operatorId == "builtin.emboss") return QStringLiteral("\u96d5\u523b\u6548\u679c");
    if (operatorId == "builtin.special_effect") return QStringLiteral("\u7279\u6b8a\u6548\u679c");
    if (operatorId == "builtin.canny") return QStringLiteral("Canny \u8fb9\u7f18");
    if (operatorId == "builtin.threshold") return QStringLiteral("\u9608\u503c");
    if (operatorId == "builtin.threshold_otsu") return QStringLiteral("Otsu \u4e8c\u503c\u5316");
    if (operatorId == "builtin.threshold_triangle") return QStringLiteral("Triangle \u4e8c\u503c\u5316");
    if (operatorId == "builtin.threshold_adaptive") return QStringLiteral("\u81ea\u9002\u5e94\u4e8c\u503c\u5316");
    if (operatorId == "builtin.edge_sobel") return QStringLiteral("Sobel \u8fb9\u7f18");
    if (operatorId == "builtin.edge_laplacian") return QStringLiteral("Laplacian \u8fb9\u7f18");
    if (operatorId == "builtin.edge_scharr") return QStringLiteral("Scharr \u8fb9\u7f18");
    if (operatorId == "builtin.contour_detect") return QStringLiteral("\u8f6e\u5ed3\u68c0\u6d4b");
    if (operatorId == "builtin.connected_components") return QStringLiteral("\u8fde\u901a\u57df\u68c0\u6d4b");
    if (operatorId == "builtin.hough_lines") return QStringLiteral("\u970d\u592b\u76f4\u7ebf\u68c0\u6d4b");
    if (operatorId == "builtin.hough_circles") return QStringLiteral("\u970d\u592b\u5706\u68c0\u6d4b");
    if (operatorId == "builtin.corner_detect") return QStringLiteral("\u89d2\u70b9\u68c0\u6d4b");
    if (operatorId == "builtin.sift_keypoints") return QStringLiteral("SIFT \u7279\u5f81\u70b9\u68c0\u6d4b");
    if (operatorId == "builtin.surf_keypoints") return QStringLiteral("SURF \u7279\u5f81\u70b9\u68c0\u6d4b");
    if (operatorId == "builtin.segment_kmeans") return QStringLiteral("K-\u5747\u503c\u805a\u7c7b\u5206\u5272");
    if (operatorId == "builtin.segment_meanshift") return QStringLiteral("MeanShift \u5747\u503c\u6f02\u79fb\u5206\u5272");
    if (operatorId == "builtin.segment_watershed") return QStringLiteral("\u5206\u6c34\u5cad\u5206\u5272");
    if (operatorId == "builtin.flood_fill") return QStringLiteral("\u6d78\u6c34\u586b\u5145");
    if (operatorId == "builtin.grabcut") return QStringLiteral("GrabCut \u62a0\u56fe");
    if (operatorId == "builtin.surf_match") return QStringLiteral("SURF \u7279\u5f81\u5339\u914d");

    return fallbackName;
}

}

PipelinePanel::PipelinePanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    m_titleLabel = new QLabel(this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(11);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    layout->addWidget(m_titleLabel);

    m_availableLabel = new QLabel(this);
    QFont sectionFont = m_availableLabel->font();
    sectionFont.setBold(true);
    m_availableLabel->setFont(sectionFont);
    layout->addWidget(m_availableLabel);

    m_availableOperatorsList = new QListWidget(this);
    m_availableOperatorsList->setObjectName("availableOperatorsList");
    m_availableOperatorsList->setMinimumHeight(180);
    layout->addWidget(m_availableOperatorsList);

    m_addOperatorButton = new QPushButton("Add To Pipeline", this);
    m_addOperatorButton->setObjectName("addOperatorButton");
    m_addOperatorButton->setMinimumHeight(32);
    layout->addWidget(m_addOperatorButton);

    m_pipelineLabel = new QLabel(this);
    m_pipelineLabel->setFont(sectionFont);
    layout->addWidget(m_pipelineLabel);

    m_pipelineStepsList = new QListWidget(this);
    m_pipelineStepsList->setObjectName("pipelineStepsList");
    m_pipelineStepsList->setDragDropMode(QAbstractItemView::InternalMove);
    m_pipelineStepsList->setDefaultDropAction(Qt::MoveAction);
    m_pipelineStepsList->setMinimumHeight(180);
    layout->addWidget(m_pipelineStepsList);

    m_removeOperatorButton = new QPushButton("Remove Selected Step", this);
    m_removeOperatorButton->setObjectName("removeOperatorButton");
    m_clearPipelineButton = new QPushButton("Clear Pipeline", this);
    m_clearPipelineButton->setObjectName("clearPipelineButton");
    m_savePipelineButton = new QPushButton("Save Pipeline", this);
    m_savePipelineButton->setObjectName("savePipelineButton");
    m_loadPipelineButton = new QPushButton("Load Pipeline", this);
    m_loadPipelineButton->setObjectName("loadPipelineButton");

    m_removeOperatorButton->setMinimumHeight(30);
    m_clearPipelineButton->setMinimumHeight(30);
    m_savePipelineButton->setMinimumHeight(30);
    m_loadPipelineButton->setMinimumHeight(30);

    auto* editRow = new QHBoxLayout();
    editRow->setSpacing(8);
    editRow->addWidget(m_removeOperatorButton);
    editRow->addWidget(m_clearPipelineButton);
    layout->addLayout(editRow);

    auto* ioRow = new QHBoxLayout();
    ioRow->setSpacing(8);
    ioRow->addWidget(m_savePipelineButton);
    ioRow->addWidget(m_loadPipelineButton);
    layout->addLayout(ioRow);

    layout->addStretch();

    connect(m_addOperatorButton, &QPushButton::clicked, this, [this] {
        auto* item = m_availableOperatorsList->currentItem();
        if (!isAvailableOperatorItem(item)) {
            return;
        }

        emit addOperatorRequested(item->data(kOperatorIdRole).toString());
    });

    connect(m_availableOperatorsList, &QListWidget::currentItemChanged, this,
        [this](QListWidgetItem* current, QListWidgetItem*) {
            if (current != nullptr && current->data(kIsCategoryHeaderRole).toBool()) {
                m_availableOperatorsList->blockSignals(true);
                m_availableOperatorsList->setCurrentItem(nullptr);
                m_availableOperatorsList->blockSignals(false);
                m_addOperatorButton->setEnabled(false);
                return;
            }

            m_addOperatorButton->setEnabled(isAvailableOperatorItem(current));
        });

    connect(m_pipelineStepsList, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0) {
            emit pipelineStepSelected(row);
        }
    });

    connect(m_pipelineStepsList, &QListWidget::itemChanged, this, [this](QListWidgetItem* item) {
        if (m_updatingPipelineList || !item) {
            return;
        }

        const int row = m_pipelineStepsList->row(item);
        if (row >= 0) {
            emit pipelineStepRenamed(row, item->text());
        }
    });

    connect(m_pipelineStepsList->model(), &QAbstractItemModel::rowsMoved, this,
        [this](const QModelIndex&, int sourceStart, int, const QModelIndex&, int destinationRow) {
            if (m_updatingPipelineList) {
                return;
            }

            const int finalRow = destinationRow > sourceStart ? destinationRow - 1 : destinationRow;
            if (sourceStart != finalRow) {
                emit pipelineStepMoved(sourceStart, finalRow);
            }
        });

    connect(m_removeOperatorButton, &QPushButton::clicked, this, [this] {
        const int row = m_pipelineStepsList->currentRow();
        if (row >= 0) {
            emit removePipelineStepRequested(row);
        }
    });

    connect(m_clearPipelineButton, &QPushButton::clicked, this, [this] {
        emit clearPipelineRequested();
    });

    connect(m_savePipelineButton, &QPushButton::clicked, this, [this] {
        emit savePipelineRequested();
    });

    connect(m_loadPipelineButton, &QPushButton::clicked, this, [this] {
        emit loadPipelineRequested();
    });

    setLanguage("en");
    m_addOperatorButton->setEnabled(false);
}

void PipelinePanel::setLanguage(const QString& languageCode)
{
    m_languageCode = languageCode;
    const bool chinese = isChineseLanguage(languageCode);
    m_titleLabel->setText(chinese ? QStringLiteral("\u6d41\u7a0b\u5de5\u4f5c\u533a") : "Pipeline Workspace");
    m_availableLabel->setText(chinese ? QStringLiteral("\u53ef\u7528\u7b97\u5b50") : "Available Operators");
    m_addOperatorButton->setText(chinese ? QStringLiteral("\u52a0\u5165\u6d41\u7a0b") : "Add To Pipeline");
    m_pipelineLabel->setText(chinese ? QStringLiteral("\u6d41\u7a0b\u6b65\u9aa4") : "Pipeline Steps");
    m_removeOperatorButton->setText(chinese ? QStringLiteral("\u79fb\u9664\u9009\u4e2d\u6b65\u9aa4") : "Remove Selected Step");
    m_clearPipelineButton->setText(chinese ? QStringLiteral("\u6e05\u7a7a\u6d41\u7a0b") : "Clear Pipeline");
    m_savePipelineButton->setText(chinese ? QStringLiteral("\u4fdd\u5b58\u6d41\u7a0b") : "Save Pipeline");
    m_loadPipelineButton->setText(chinese ? QStringLiteral("\u52a0\u8f7d\u6d41\u7a0b") : "Load Pipeline");

    for (int index = 0; index < m_availableOperatorsList->count(); ++index) {
        QListWidgetItem* item = m_availableOperatorsList->item(index);
        if (!item) {
            continue;
        }

        if (item->data(kIsCategoryHeaderRole).toBool()) {
            item->setText(localizedCategoryName(item->data(kCategoryNameRole).toString(), languageCode));
            continue;
        }

        const QString operatorId = item->data(kOperatorIdRole).toString();
        const QString defaultName = item->data(kDefaultNameRole).toString();
        item->setText(localizedOperatorName(operatorId, defaultName, languageCode));
    }
}

void PipelinePanel::setOperators(const QList<OperatorDescriptor>& descriptors)
{
    m_availableOperatorsList->clear();

    const QStringList orderedCategories = {
        QStringLiteral("\u5f62\u6001\u5b66\u5904\u7406"),
        QStringLiteral("\u6ee4\u6ce2"),
        QStringLiteral("\u56fe\u50cf\u589e\u5f3a\u4e0e\u53d8\u6362"),
        QStringLiteral("\u4e8c\u503c\u5316"),
        QStringLiteral("\u8fb9\u7f18\u68c0\u6d4b"),
        QStringLiteral("\u6ee4\u955c\u6548\u679c"),
        QStringLiteral("\u56fe\u50cf\u68c0\u6d4b"),
        QStringLiteral("\u56fe\u50cf\u5206\u5272"),
        QStringLiteral("\u7279\u5f81\u5339\u914d"),
    };

    QStringList categories = orderedCategories;
    for (const OperatorDescriptor& descriptor : descriptors) {
        if (!descriptor.category.isEmpty() && !categories.contains(descriptor.category)) {
            categories.append(descriptor.category);
        }
    }

    for (const QString& category : categories) {
        QList<OperatorDescriptor> operatorsInCategory;
        for (const OperatorDescriptor& descriptor : descriptors) {
            if (descriptor.category == category) {
                operatorsInCategory.append(descriptor);
            }
        }
        if (operatorsInCategory.isEmpty()) {
            continue;
        }

        auto* headerItem = new QListWidgetItem(
            localizedCategoryName(category, m_languageCode),
            m_availableOperatorsList);
        QFont headerFont = headerItem->font();
        headerFont.setBold(true);
        headerItem->setFont(headerFont);
        headerItem->setFlags(Qt::ItemIsEnabled);
        headerItem->setData(kIsCategoryHeaderRole, true);
        headerItem->setData(kCategoryNameRole, category);

        for (const OperatorDescriptor& descriptor : operatorsInCategory) {
            auto* item = new QListWidgetItem(
                localizedOperatorName(descriptor.id, descriptor.displayName, m_languageCode),
                m_availableOperatorsList);
            item->setData(kOperatorIdRole, descriptor.id);
            item->setData(kDefaultNameRole, descriptor.displayName);
        }
    }

    m_availableOperatorsList->setCurrentItem(nullptr);
    m_addOperatorButton->setEnabled(false);
}

void PipelinePanel::setPipelineStepNames(const QStringList& stepNames)
{
    m_updatingPipelineList = true;
    m_pipelineStepsList->clear();
    for (const QString& name : stepNames) {
        auto* item = new QListWidgetItem(name, m_pipelineStepsList);
        item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
    }
    m_updatingPipelineList = false;
}

void PipelinePanel::setCurrentPipelineStep(int index)
{
    m_pipelineStepsList->setCurrentRow(index);
}

void PipelinePanel::movePipelineStep(int from, int to)
{
    if (from < 0 || to < 0 || from >= m_pipelineStepsList->count() || to >= m_pipelineStepsList->count() || from == to) {
        return;
    }

    m_updatingPipelineList = true;
    QListWidgetItem* item = m_pipelineStepsList->takeItem(from);
    m_pipelineStepsList->insertItem(to, item);
    m_pipelineStepsList->setCurrentRow(to);
    m_updatingPipelineList = false;
    emit pipelineStepMoved(from, to);
}
