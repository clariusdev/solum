#include "display.h"
#include <solum/solum.h>

/// default constructor
/// @param[in] parent the parent object
UltrasoundImage::UltrasoundImage(bool overlay, QWidget* parent) : QGraphicsView(parent), depth_(0), overlay_(overlay)
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
/// @param[in] bpp bits per pixel
/// @param[in] format the image format
/// @param[in] sz size of image in bytes
void UltrasoundImage::loadImage(const void* img, int w, int h, int bpp, CusImageFormat format, int sz)
{
    // check for size match
    if (image_.width() != w || image_.height() != h)
        return;

    // ensure the qimage format matches
    if (format == Uncompressed8Bit && image_.format() != QImage::Format_Grayscale8)
        image_.convertTo(QImage::Format_Grayscale8);
    else if (format != Uncompressed8Bit && image_.format() != QImage::Format_ARGB32)
        image_.convertTo(QImage::Format_ARGB32);

    // set the image data
    // check that the size matches the dimensions (uncompressed)
    if (sz >= (w * h * (bpp / 8)))
        std::memcpy(image_.bits(), img, w * h * (bpp / 8));
    // try to load jpeg
    else if (format == Jpeg)
        image_.loadFromData(static_cast<const uchar*>(img), sz, "JPG");
    else if (format == Png)
        image_.loadFromData(static_cast<const uchar*>(img), sz, "PNG");

    // redraw
    scene()->invalidate();
}

/// checks if there's a valid roi that should be drawn
void UltrasoundImage::checkActiveRegion()
{
    // make room for 4 x/y points
    double buf[8];
    // for sector probes, the first 2 and the last 2 points, along with the radius of curvature, should be used to calculate an arc if necessary
    // qt supports a drawArc() function which could be useful for portraying this
    if (solumGetActiveRegion(buf, 4) == 0)
    {
        QPolygonF roi;
        for (auto i = 0u; i < 4; i++)
            roi.push_back(QPointF(buf[i * 2], buf[(i * 2) + 1]));
        activeRoi_ = roi;
    }
}

/// checks if there's a valid roi that should be drawn
void UltrasoundImage::checkRoi()
{
    // make room for 32 x/y points
    double buf[64];
    if (solumGetRoi(buf, 32) == 0)
    {
        QPolygonF roi;
        for (auto i = 0u; i < 32; i++)
            roi.push_back(QPointF(buf[i * 2], buf[(i * 2) + 1]));
        modeRoi_ = roi;
    }
    else
        modeRoi_.clear();
}

/// checks if there's a valid gate that should be drawn
void UltrasoundImage::checkGate()
{
    CusGateLines lines;
    if (solumGetGate(&lines) == 0)
    {
        auto convertLine = [](const CusLineF& line) -> QLineF
        {
            return QLineF(QPointF(line.p1.x, line.p1.y), QPointF(line.p2.x, line.p2.y));
        };
        gate_.clear();
        gate_.push_back(convertLine(lines.active));
        gate_.push_back(convertLine(lines.top));
        gate_.push_back(convertLine(lines.normalTop));
        gate_.push_back(convertLine(lines.normalBottom));
        gate_.push_back(convertLine(lines.bottom));
    }
    else
        gate_.clear();
}

/// handles resizing of the image view
/// @param[in] e the event to parse
void UltrasoundImage::resizeEvent(QResizeEvent* e)
{
    auto w = e->size().width(), h = e->size().height();

    setSceneRect(0, 0, w, h);
    if (!overlay_)
        solumSetOutputSize(w, h);

    image_ = QImage(w, h, QImage::Format_ARGB32);
    image_.fill(Qt::black);

    // update the roi in the case of a resize
    if (!overlay_)
    {
        QTimer::singleShot(250, [this]()
        {
            checkActiveRegion();
            checkRoi();
            checkGate();
        });
    }

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
    if (!image_.isNull())
    {
        painter->drawImage(r, image_);
        if (depth_)
        {
            painter->setPen(Qt::yellow);
            painter->drawText(rect(), Qt::AlignRight | Qt::AlignBottom,
                QStringLiteral("%1 cm").arg(QString::number(depth_, 'f', 1)));
        }
        if (activeRoi_.size())
        {
            painter->setPen(Qt::darkBlue);
            painter->drawPolygon(activeRoi_);
        }
        if (modeRoi_.size())
        {
            painter->setPen(Qt::yellow);
            painter->drawPolygon(modeRoi_);
        }
        if (gate_.size())
        {
            bool active = true;
            for (const auto& l : gate_)
            {
                if (active)
                {
                    painter->setPen(QPen(Qt::yellow, 1, Qt::DotLine));
                    active = false;
                }
                else
                    painter->setPen(QPen(Qt::yellow, 1, Qt::DashLine));
                painter->drawLine(l);
            }
        }
    }
}

/// called on a mouse button release event
/// @param[in] e the mouse event
void UltrasoundImage::mouseReleaseEvent(QMouseEvent* e)
{
    if (overlay_)
        return;

    // if the call to move succeeds, it means an imaging mode supporting an roi is running
    auto pos = e->position();
    auto m = solumGetMode();
    if (m == ColorMode || m == PowerMode || m == Strain || m == RfMode)
    {
        if (solumAdjustRoi(static_cast<int>(pos.x()), static_cast<int>(pos.y()), (e->button() == Qt::LeftButton) ? MoveRoi : SizeRoi) == 0)
            checkRoi();
    }
    else if (m == MMode || m == PwMode)
    {
        if (solumAdjustGate(static_cast<int>(pos.x()), static_cast<int>(pos.y())) == 0)
            checkGate();
    }

    QWidget::mouseReleaseEvent(e);
}

/// default constructor
/// @param[in] parent the parent object
Spectrum::Spectrum(QWidget* parent) : QGraphicsView(parent)
{
    QGraphicsScene* sc = new QGraphicsScene(this);
    setScene(sc);
    setVisible(false);

    // initialize to some arbitrary size
    setSceneRect(0, 0, 400, 100);

    QSizePolicy p(QSizePolicy::Preferred, QSizePolicy::Preferred);
    p.setHeightForWidth(true);
    setSizePolicy(p);
}

/// resets the spectrum
void Spectrum::reset()
{
    spectrum_.fill(Qt::black);
    offset_ = 0;
    scene()->invalidate();
}

/// loads a new image from raw data
/// @param[in] img the new spectrum data
/// @param[in] l # of spectrum lines
/// @param[in] s # of spectrum samples
/// @param[in] bps bits per sample
void Spectrum::loadImage(const void* img, int l, int s, int bps)
{
    auto w = width();
    // recreate the image if the dimensions change
    if (s != spectrum_.width() || w != spectrum_.height())
    {
        spectrum_ = QImage(s, w, QImage::Format_Grayscale8);
        spectrum_.fill(Qt::black);
        offset_ = 0;
    }

    int sz = std::min(w * s, (l * s * (bps / 8)));
    // ensure we don't go beyond the buffer space
    if (offset_ + sz > w * s)
        offset_ = 0;

    std::memcpy(spectrum_.bits() + offset_, img, sz);
    offset_ += sz;

    // redraw
    scene()->invalidate();
}

/// handles resizing of the image view
/// @param[in] e the event to parse
void Spectrum::resizeEvent(QResizeEvent* e)
{
    auto w = e->size().width(), h = e->size().height();

    setSceneRect(0, 0, w, h);
    reset();

    QGraphicsView::resizeEvent(e);
}

/// calculates the ratio of the test image to determine the proper height ratio for width
/// @param[in] w the width of the widget
/// @return the appropriate height
int Spectrum::heightForWidth(int w) const
{
    // keep 2:1 aspect ratio
    double ratio = 1.0 / 2.0;
    return static_cast<int>(w * ratio);
}

/// size hint to keep the test image ratio
/// @return the size hint
QSize Spectrum::sizeHint() const
{
    auto w = width();
    return QSize(w, heightForWidth(w));
}

/// creates a black background
/// @param[in] painter the drawing context
/// @param[in] r the rectangle to fill (the entire view)
void Spectrum::drawBackground(QPainter* painter, const QRectF& r)
{
    painter->fillRect(r, QBrush(Qt::black));
    QGraphicsView::drawBackground(painter, r);
}

/// draws the target image
/// @param[in] painter the drawing context
void Spectrum::drawForeground(QPainter* painter, const QRectF& r)
{
    if (!spectrum_.isNull())
    {
        auto mirrored = spectrum_.flipped(Qt::Horizontal);
        auto rotated = mirrored.transformed(QTransform().rotate(-90));
        painter->drawImage(r, rotated);
    }
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
    signal_.clear();
    const int16_t* buf = static_cast<const int16_t*>(rf) + ((l / 2) * s);
    for (auto i = 0; i < s; i++)
        signal_.push_back(*buf++);

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
    }
}

/// default constructor
/// @param[in] parent the parent object
Prescan::Prescan(QWidget* parent) : QGraphicsView(parent)
{
    QGraphicsScene* sc = new QGraphicsScene(this);
    setScene(sc);
    setVisible(false);

    // initialize to some arbitrary size
    setSceneRect(0, 0, 400, 100);
    image_ = QImage(400, 100, QImage::Format_ARGB32);

    QSizePolicy p(QSizePolicy::Preferred, QSizePolicy::Preferred);
    p.setHeightForWidth(true);
    setSizePolicy(p);
}

/// loads a new image from raw data
/// @param[in] img the new prescan data
/// @param[in] w width of image (aka # of spectrum lines)
/// @param[in] h height of image (aka # of spectrum samples)
/// @param[in] bpp bits per pixel (aka bits per sample)
void Prescan::loadImage(const void* img, int w, int h, int bpp, CusImageFormat format, int sz)
{
    // check for size match
    if (image_.width() != h || image_.height() != w)
    {
        image_ = QImage(h, w, QImage::Format_Grayscale8);
        image_.fill(Qt::black);
    }

    if (format == Jpeg)
        image_.loadFromData(static_cast<const uchar*>(img), sz, "JPG");
    else
        std::memcpy(image_.bits(), img, w * h * (bpp / 8));

    // redraw
    scene()->invalidate();
}

/// handles resizing of the image view
/// @param[in] e the event to parse
void Prescan::resizeEvent(QResizeEvent* e)
{
    auto w = e->size().width(), h = e->size().height();

    setSceneRect(0, 0, w, h);
    image_.fill(Qt::black);

    QGraphicsView::resizeEvent(e);
}

/// calculates the ratio of the test image to determine the proper height ratio for width
/// @param[in] w the width of the widget
/// @return the appropriate height
int Prescan::heightForWidth(int w) const
{
    // keep 2:1 aspect ratio
    double ratio = 1.0 / 2.0;
    return static_cast<int>(w * ratio);
}

/// size hint to keep the test image ratio
/// @return the size hint
QSize Prescan::sizeHint() const
{
    auto w = width();
    return QSize(w, heightForWidth(w));
}

/// creates a black background
/// @param[in] painter the drawing context
/// @param[in] r the rectangle to fill (the entire view)
void Prescan::drawBackground(QPainter* painter, const QRectF& r)
{
    painter->fillRect(r, QBrush(Qt::black));
    QGraphicsView::drawBackground(painter, r);
}

/// draws the target image
/// @param[in] painter the drawing context
void Prescan::drawForeground(QPainter* painter, const QRectF& r)
{
    if (!image_.isNull())
    {
        painter->drawImage(r, image_);
    }
}
