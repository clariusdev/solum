#include "display.h"
#include <oem/oem.h>

/// default constructor
/// @param[in] parent the parent object
UltrasoundImage::UltrasoundImage(QWidget* parent) : QGraphicsView(parent), depth_(0)
{
    QGraphicsScene* sc = new QGraphicsScene(this);
    setScene(sc);

    // initialize image to some arbitrary size
    image_ = QImage(320, 240, QImage::Format_ARGB32);
    image_.fill(Qt::black);
    setSceneRect(0, 0, image_.width(), image_.height());

    QSizePolicy p(QSizePolicy::Preferred, QSizePolicy::Preferred);
    p.setHeightForWidth(true);
    setSizePolicy(p);
}

/// loads a new image from raw data
/// @param[in] img the new image data
/// @param[in] w the image width
/// @param[in] h the image height
void UltrasoundImage::loadImage(const void* img, int w, int h, int bpp)
{
    // check for size match
    if (image_.width() != w || image_.height() != h)
        return;

    // set the image data
    lock_.lock();
    memcpy(image_.bits(), img, w * h * (bpp / 8));
    lock_.unlock();

    // redraw
    scene()->invalidate();
}

/// checks if there's a valid roi that should be drawn
void UltrasoundImage::checkRoi()
{
    lock_.lock();
    // make room for 32 x/y points
    double buf[64];
    if (cusOemGetRoi(buf, 32) == 0)
    {
        QPolygonF roi;
        for (auto i = 0u; i < 32; i++)
            roi.push_back(QPointF(buf[i * 2], buf[(i * 2) + 1]));
        roi_ = roi;
    }
    else
        roi_.clear();
    lock_.unlock();
}

/// handles resizing of the image view
/// @param[in] e the event to parse
void UltrasoundImage::resizeEvent(QResizeEvent* e)
{
    auto w = e->size().width(), h = e->size().height();

    setSceneRect(0, 0, w, h);
    cusOemSetOutputSize(w, h);

    lock_.lock();
    image_ = QImage(w, h, QImage::Format_ARGB32);
    image_.fill(Qt::black);
    lock_.unlock();

    // update the roi in the case of a resize
    QTimer::singleShot(250, [this]()
    {
        checkRoi();
    });

    QGraphicsView::resizeEvent(e);
}

/// calculates the ratio of the test image to determine the proper height ratio for width
/// @param[in] w the width of the widget
/// @return the appropriate height
int UltrasoundImage::heightForWidth(int w) const
{
    // keep 4:3 aspect ratio
    double ratio = 3.0 / 4.0;
    return static_cast<int>(w * ratio);
}

/// size hint to keep the test image ratio
/// @return the size hint
QSize UltrasoundImage::sizeHint() const
{
    auto w = width();
    return QSize(w, heightForWidth(w));
}

/// creates a black background
/// @param[in] painter the drawing context
/// @param[in] r the rectangle to fill (the entire view)
void UltrasoundImage::drawBackground(QPainter* painter, const QRectF& r)
{
    painter->fillRect(r, QBrush(Qt::black));
    QGraphicsView::drawBackground(painter, r);
}

/// draws the target image
/// @param[in] painter the drawing context
void UltrasoundImage::drawForeground(QPainter* painter, const QRectF& r)
{
    lock_.lock();
    if (!image_.isNull())
    {
        painter->drawImage(r, image_);
        if (depth_)
        {
            painter->setPen(Qt::yellow);
            painter->drawText(rect(), Qt::AlignRight | Qt::AlignBottom,
                QStringLiteral("%1 cm").arg(QString::number(depth_, 'f', 1)));
        }
        if (roi_.size())
        {
            painter->setPen(Qt::yellow);
            painter->drawPolygon(roi_);
        }
    }
    lock_.unlock();
}

/// called on a mouse button release event
/// @param[in] e the mouse event
void UltrasoundImage::mouseReleaseEvent(QMouseEvent* e)
{
    // if the call to move succeeds, it means an imaging mode supporting an roi is running
    auto pos = e->localPos();
    if (cusOemAdjustRoi(static_cast<int>(pos.x()), static_cast<int>(pos.y()), (e->button() == Qt::LeftButton) ? ROI_MOVE : ROI_SIZE) == 0)
        checkRoi();

    QWidget::mouseReleaseEvent(e);
}

/// default constructor
/// @param[in] parent the parent object
RfSignal::RfSignal(QWidget* parent) : QGraphicsView(parent), zoom_(0.1)
{
    QGraphicsScene* sc = new QGraphicsScene(this);
    setScene(sc);
    setVisible(false);

    setSceneRect(0, 0, width(), height());

    QSizePolicy p(QSizePolicy::Preferred, QSizePolicy::Preferred);
    p.setHeightForWidth(true);
    setSizePolicy(p);
}

/// loads new rf signal
/// @param[in] rf the new rf data
/// @param[in] l # of rf lines
/// @param[in] s # of samples per line
/// @param[in] ss sample size in bytes
void RfSignal::loadSignal(const void* rf, int l, int s, int ss)
{
    if (!rf || !l || !s || ss != 2)
        return;

    // pick the center line to display
    lock_.lock();
    signal_.clear();
    const int16_t* buf = static_cast<const int16_t*>(rf) + ((l / 2) * s);
    for (auto i = 0; i < s; i++)
        signal_.push_back(*buf++);
    lock_.unlock();

    // redraw
    scene()->invalidate();
}

/// sets the zoom scaling to display the rf signal
/// @param[in] zoom the zoom percentage
void RfSignal::setZoom(int zoom)
{
    zoom_ = (static_cast<qreal>(zoom) / 100.0);
}

/// handles resizing of the image view
/// @param[in] e the event to parse
void RfSignal::resizeEvent(QResizeEvent* e)
{
    auto w = e->size().width(), h = e->size().height();
    setSceneRect(0, 0, w, h);
    QGraphicsView::resizeEvent(e);
}

/// calculates the ratio of the test image to determine the proper height ratio for width
/// @param[in] w the width of the widget
/// @return the appropriate height
int RfSignal::heightForWidth(int w) const
{
    // keep 4:1 aspect ratio
    double ratio = 1.0 / 4.0;
    return static_cast<int>(w * ratio);
}

/// size hint to keep the test image ratio
/// @return the size hint
QSize RfSignal::sizeHint() const
{
    auto w = width();
    return QSize(w, heightForWidth(w));
}

/// creates a black background
/// @param[in] painter the drawing context
/// @param[in] r the rectangle to fill (the entire view)
void RfSignal::drawBackground(QPainter* painter, const QRectF& r)
{
    painter->fillRect(r, QBrush(Qt::black));
    QGraphicsView::drawBackground(painter, r);
}

/// draws the rf signal
/// @param[in] painter the drawing context
/// @param[in] r the view rectangle
void RfSignal::drawForeground(QPainter* painter, const QRectF& r)
{
    if (!signal_.isEmpty())
    {
        lock_.lock();
        painter->setPen(QColor(96, 96, 0));
        qreal x = 0, baseline = r.height() / 2;
        double sampleSize = static_cast<double>(r.width()) / static_cast<double>(signal_.size());
        QPointF p(x, baseline);
        for (auto s : signal_)
        {
            qreal y = s * zoom_;
            QPointF pt(x + sampleSize, baseline + y);
            painter->drawLine(p, pt);
            p = pt;
            x = x + sampleSize;
        }
        lock_.unlock();
    }
}
