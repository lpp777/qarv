/*
    qarv, a Qt interface to aravis.
    Copyright (C) 2012  Jure Varlec <jure.varlec@ad-vega.si>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "api/qarvdecoder.h"
#include "decoders/swscaledecoder.h"
#include "decoders/graymap.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <QPluginLoader>
#include <QMap>
extern "C" {
  #include <arvenums.h>
}

QImage QArvDecoder::CV2QImage(const cv::Mat& image) {
  if (image.channels() == 1) {
    QImage img(image.cols, image.rows, QImage::Format_Indexed8);
    img.setColorTable(graymap);
    cv::Mat view(image.rows, image.cols, CV_8UC1, img.bits(), img.bytesPerLine());
    image.convertTo(view, CV_8UC1, 1/256.);
    return img;
  } else {
    QImage img(image.cols, image.rows, QImage::Format_RGB888);
    cv::Mat view(image.rows, image.cols, CV_8UC3, img.bits(), img.bytesPerLine());
    image.convertTo(view, CV_8UC3, 1/256.);
    cv::cvtColor(view, view, CV_BGR2RGB);
    return img;
  }
}

QList<QArvPixelFormat*> initPluginFormats() {
  QList<QArvPixelFormat*> list;
  auto plugins = QPluginLoader::staticInstances();
  foreach (auto plugin, plugins) {
    auto fmt = qobject_cast<QArvPixelFormat*>(plugin);
    if (fmt != NULL) list << fmt;
  }
  return list;
}

QMap<ArvPixelFormat, enum PixelFormat> initSwScaleFormats();

// List of formats supported by plugins.
static QList<QArvPixelFormat*> pluginFormats = initPluginFormats();

// List of formats supported by libswscale, with mappings to
// appropriate ffmpeg formats.
QMap<ArvPixelFormat, enum PixelFormat> swScaleFormats = initSwScaleFormats();

QList<ArvPixelFormat> QArvPixelFormat::supportedFormats() {
  QList<ArvPixelFormat> list;
  foreach (auto fmt, pluginFormats) {
    list << fmt->pixelFormat();
  }
  list << swScaleFormats.keys();
  return list;
}

/*!
 * Returns NULL if the format is not supported.
 */
QArvDecoder* QArvPixelFormat::makeDecoder(ArvPixelFormat format, QSize size) {
  foreach (auto fmt, pluginFormats) {
    if (fmt->pixelFormat() == format) return fmt->makeDecoder(size);
  }
  foreach (auto arvfmt, swScaleFormats.keys()) {
    if (arvfmt == format)
      return new QArv::SwScaleDecoder(size, swScaleFormats[arvfmt], arvfmt);
  }
  return NULL;
}

/*!
 * Returns NULL if the format is not supported.
 */
QArvDecoder* QArvPixelFormat::makeSwScaleDecoder(PixelFormat fmt, QSize size) {
  return new QArv::SwScaleDecoder(size, fmt, 0);
}

QMap<ArvPixelFormat, PixelFormat> initSwScaleFormats() {
  QMap<ArvPixelFormat, PixelFormat> m;

  m[ARV_PIXEL_FORMAT_YUV_422_PACKED] = PIX_FMT_UYVY422;
  m[ARV_PIXEL_FORMAT_RGB_8_PACKED] = PIX_FMT_RGB24;
  m[ARV_PIXEL_FORMAT_BGR_8_PACKED] = PIX_FMT_BGR24;
  m[ARV_PIXEL_FORMAT_RGBA_8_PACKED] = PIX_FMT_RGBA;
  m[ARV_PIXEL_FORMAT_BGRA_8_PACKED] = PIX_FMT_BGRA;
  m[ARV_PIXEL_FORMAT_YUV_411_PACKED] = PIX_FMT_UYYVYY411;
  m[ARV_PIXEL_FORMAT_YUV_422_PACKED] = PIX_FMT_UYVY422;
  m[ARV_PIXEL_FORMAT_YUV_422_YUYV_PACKED] = PIX_FMT_YUYV422;

  return m;
}