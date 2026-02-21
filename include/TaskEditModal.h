#ifndef TASKEDITMODAL_H
#define TASKEDITMODAL_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>

class TaskEditModal : public QWidget
{
    Q_OBJECT
public:
    explicit TaskEditModal(const QString& text, QWidget *parent = nullptr);

signals:
    void saveRequested(const QString& newText);
    void cancelRequested();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QTextEdit* m_textEdit;
    QPushButton* m_saveBtn;
    QPushButton* m_cancelBtn;
};

#endif // TASKEDITMODAL_H
