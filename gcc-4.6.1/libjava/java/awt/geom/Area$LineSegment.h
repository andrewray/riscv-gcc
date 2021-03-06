
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_awt_geom_Area$LineSegment__
#define __java_awt_geom_Area$LineSegment__

#pragma interface

#include <java/awt/geom/Area$Segment.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace awt
    {
      namespace geom
      {
          class AffineTransform;
          class Area;
          class Area$LineSegment;
          class Area$Segment;
          class Point2D;
          class Rectangle2D;
      }
    }
  }
}

class java::awt::geom::Area$LineSegment : public ::java::awt::geom::Area$Segment
{

public:
  Area$LineSegment(::java::awt::geom::Area *, jdouble, jdouble, jdouble, jdouble);
  Area$LineSegment(::java::awt::geom::Area *, ::java::awt::geom::Point2D *, ::java::awt::geom::Point2D *);
  virtual ::java::lang::Object * clone();
public: // actually package-private
  virtual void transform(::java::awt::geom::AffineTransform *);
  virtual void reverseCoords();
  virtual ::java::awt::geom::Point2D * getMidPoint();
  virtual jdouble curveArea();
  virtual jint getType();
  virtual void subdivideInsert(jdouble);
  virtual jboolean isCoLinear(::java::awt::geom::Area$LineSegment *);
  virtual ::java::awt::geom::Area$Segment * lastCoLinear();
  virtual jboolean equals(::java::awt::geom::Area$Segment *);
  virtual jint pathIteratorFormat(JArray< jdouble > *);
  virtual jboolean hasIntersections(::java::awt::geom::Area$Segment *);
  virtual jint splitIntersections(::java::awt::geom::Area$Segment *);
  virtual ::java::awt::geom::Rectangle2D * getBounds();
  virtual jint rayCrossing(jdouble, jdouble);
  ::java::awt::geom::Area * __attribute__((aligned(__alignof__( ::java::awt::geom::Area$Segment)))) this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __java_awt_geom_Area$LineSegment__
