// ======================================================================
//
// Copyright (c) 2003 The CGAL Consortium
//
// This software and related documentation is part of an INTERNAL release
// of the Computational Geometry Algorithms Library (CGAL). It is not
// intended for general use.
//
// ----------------------------------------------------------------------
//
// release       : 
// release_date  : 
//
// file          : include/CGAL/Parabola_segment_2.h
// package       : Apollonius_graph_2
// source        : $RCSfile$
// revision      : $Revision$
// revision_date : $Date$
// author(s)     : Menelaos Karavelas <mkaravel@cse.nd.edu>
//
// coordinator   :
//
// ======================================================================



#ifndef CGAL_PARABOLA_SEGMENT_2_H
#define CGAL_PARABOLA_SEGMENT_2_H

#include <CGAL/Parabola_2.h>

#ifdef CGAL_USE_QT
#include <CGAL/IO/Qt_widget.h>
#endif

CGAL_BEGIN_NAMESPACE

template < class Gt >
class Parabola_segment_2 : public Parabola_2< Gt >
{
  typedef CGAL::Parabola_2<Gt>            Base;
  typedef typename Base::Site_2           Site_2;
  typedef typename Base::FT               FT;
  typedef typename Base::Point_2          Point_2;
  typedef typename Base::Segment_2        Segment_2;
  typedef typename Base::Line_2           Line_2;

protected:
  Point_2 p1, p2;

public:
  Parabola_segment_2() : Parabola_2< Gt >() {}

  template<class ApolloniusSite>
  Parabola_segment_2(const ApolloniusSite &p, const Line_2 &l,
		     const Point_2 &p1, const Point_2 &p2)
    : Parabola_2< Gt >(p, l)
  {
    this->p1 = p1;
    this->p2 = p2;
  }

  Parabola_segment_2(const Point_2 &p, const Line_2 &l,
		     const Point_2 &p1, const Point_2 &p2)
    : Parabola_2< Gt >(p, l)
  {
    this->p1 = p1;
    this->p2 = p2;
  }

  void generate_points(std::vector<Point_2>& p) const
  {
    FT s[2];

    s[0] = t(p1);
    s[1] = t(p2);

    if (CGAL_NTS compare(s[0], s[1]) == LARGER) {
#if defined(__GNUC__) && (__GNUC__ < 3)
      FT tmp = s[0];
      s[0] = s[1];
      s[1] = tmp;
#else
      std::swap< FT >(s[0], s[1]);
#endif
    }

    p.clear();

    if ( !(CGAL_NTS is_positive(s[0])) &&
	 !(CGAL_NTS is_negative(s[1])) ) {
      FT tt;
      int k;

      p.push_back( this->o );
      k = 1;
      tt = -this->STEP;
      while ( CGAL_NTS compare(tt, s[0]) == LARGER ) {
	p.insert( p.begin(), f(tt) );
	k--;
	tt = -FT(k * k) * this->STEP;
      }
      p.insert( p.begin(), f(s[0]) );

      k = 1;
      tt = this->STEP;
      while ( CGAL_NTS compare(tt, s[1]) == SMALLER ) {
	p.push_back( f(tt) );
	k++;
	tt = FT(k * k) * this->STEP;
      }
      p.push_back( f(s[1]) );
    } else if ( !(CGAL_NTS is_negative(s[0])) &&
		!(CGAL_NTS is_negative(s[1])) ) {
      FT tt;
      int k;


      p.push_back( f(s[0]) );

      tt = s[0];
      k = int(CGAL_NTS to_double(CGAL_NTS sqrt(tt / this->STEP)));

      while ( CGAL_NTS compare(tt, s[1]) == SMALLER ) {
	if ( CGAL_NTS compare(tt, s[0]) != SMALLER )
	  p.push_back( f(tt) );
	k++;
	tt = FT(k * k) * this->STEP;
      }
      p.push_back( f(s[1]) );
    } else {
      FT tt;
      int k;

      p.push_back( f(s[1]) );

      tt = s[1];
      k = int(CGAL_NTS to_double(-CGAL_NTS sqrt(-tt / this->STEP)));

      while ( CGAL_NTS compare(tt, s[0]) == LARGER ) {
	if ( CGAL_NTS compare(tt, s[1]) != LARGER )
	  p.push_back( f(tt) );
	k--;
	tt = -FT(k * k) * this->STEP;
      }
      p.push_back( f(s[0]) );
    }
  }


  template< class Stream >
  void draw(Stream& W) const
  {
    std::vector< Point_2 > p;
    generate_points(p);

    for (unsigned int i = 0; i < p.size() - 1; i++) {
      W << Segment_2(p[i], p[i+1]);
    }
  }
};



template< class Stream, class Gt >
inline
Stream& operator<<(Stream &s, const Parabola_segment_2<Gt> &P)
{
  P.draw(s);
  return s;
}

CGAL_END_NAMESPACE

#endif // CGAL_PARABOLA_SEGMENT_2_H
