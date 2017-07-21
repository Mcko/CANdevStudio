#include "canrawview.h"
#include "canrawview_p.h"
#include "log.hpp"
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QStandardItem>
#include <QtSerialBus/QCanBusFrame>
#include <QtCore/QElapsedTimer>


CanRawView::CanRawView(QWidget* parent)
    : QWidget(parent)
    , d_ptr(new CanRawViewPrivate())
    , timer(std::make_unique<QElapsedTimer>())
    , simStarted(false)
{
    Q_D(CanRawView);

    setLayout(d->ui->layout);
}

CanRawView::~CanRawView() {}

void CanRawView::startSimulation()
{
    timer->restart();
    simStarted = true;
}

void CanRawView::stopSimulation()
{
    simStarted = false;
}

void CanRawView::frameView(const QCanBusFrame& frame, const QString& direction)
{
    Q_D(CanRawView);
    if(!simStarted)
    {
        cds_debug("send/received frame while simulation stopped");
        return;
    }

    auto payHex = frame.payload().toHex();
    for (int ii = payHex.size(); ii >= 2; ii -= 2) {
        payHex.insert(ii, ' ');
    }

    QList<QStandardItem*> list;
    list.append(new QStandardItem(QString::number( (double) timer->elapsed() / 1000)));
    list.append(new QStandardItem("0x" + QString::number(frame.frameId(), 16)));
    list.append(new QStandardItem(direction));
    list.append(new QStandardItem(QString::number(frame.payload().size())));
    list.append(new QStandardItem(QString::fromUtf8(payHex.data(), payHex.size())));

    d->tvModel.appendRow(list);

    if (d->ui->freezeBox->isChecked() == false) {
        d->ui->tv->scrollToBottom();
    }
}

void CanRawView::frameReceived(const QCanBusFrame& frame) { frameView(frame, "RX"); }

void CanRawView::frameSent(bool status, const QCanBusFrame& frame, const QVariant&)
{
    if (status) {
        frameView(frame, "TX");
    }
}
