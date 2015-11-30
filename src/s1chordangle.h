// Copyright 2013 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS-IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// Author: ericv@google.com (Eric Veach)

#ifndef S2_GEOMETRY_S1CHORDANGLE_H_
#define S2_GEOMETRY_S1CHORDANGLE_H_

#include <math.h>

#include <limits>
#include <iosfwd>   // No longer needed
#include <ostream>
#include <type_traits>

#include "fpcontractoff.h"
#include "s1angle.h"
#include "s2.h"

// S1ChordAngle represents the angle subtended by a chord (i.e., the straight
// line segment connecting two points on the sphere).  Its representation
// makes it very efficient for computing and comparing distances, but unlike
// S1Angle it is only capable of representing angles between 0 and Pi radians.
// Generally, S1ChordAngle should only be used in loops where many angles need
// to be calculated and compared.  Otherwise it is simpler to use S1Angle.
//
// S1ChordAngle also loses some accuracy as the angle approaches Pi radians.
// Specifically, the representation of (Pi - x) radians can be expected to
// have an error of about (1e-15 / x), with a maximum error of about 1e-7.
//
// This class is intended to be copied by value as desired.  It uses
// the default copy constructor and assignment operator.
class S1ChordAngle {
 public:
  // The default constructor yields a zero angle.  This is useful for STL
  // containers and class methods with output arguments.
  S1ChordAngle() : length2_(0) {}

  // Construct the S1ChordAngle corresponding to the distance between the two
  // given points.  The points must be unit length.
  S1ChordAngle(S2Point const& x, S2Point const& y);

  // Return the zero chord angle.
  static S1ChordAngle Zero();

  // Return a chord angle of 180 degrees (a "straight angle").  This is the
  // maximum finite chord angle.
  static S1ChordAngle Straight();

  // Return a chord angle larger than any finite chord angle.  The only valid
  // operations on Infinity() are comparisons and S1Angle conversions.
  static S1ChordAngle Infinity();

  // Return a chord angle smaller than Zero().  The only valid operations on
  // Negative() are comparisons and S1Angle conversions.
  static S1ChordAngle Negative();

  // Conversion from an S1Angle.  Angles outside the range [0, Pi] are handled
  // as follows: Infinity() is mapped to Infinity(), negative angles are
  // mapped to Negative(), and finite angles larger than Pi are mapped to
  // Straight().
  //
  // Note that this operation is relatively expensive and should be avoided.
  // To use S1ChordAngle effectively, you should structure your code so that
  // input arguments are converted to S1ChordAngles at the beginning of your
  // algorithm, and results are converted back to S1Angles only at the end.
  explicit S1ChordAngle(S1Angle angle);

  // Construct an S1ChordAngle that is an upper bound on the given S1Angle,
  // i.e. such that FastUpperBoundFrom(x).ToAngle() >= x.  Unlike the S1Angle
  // constructor above, this method is very fast, and the bound is accurate to
  // within 1% for distances up to about 3100km on the Earth's surface.
  static S1ChordAngle FastUpperBoundFrom(S1Angle angle);

  // Construct an S1ChordAngle from the squared chord length.  Note that the
  // argument is automatically clamped to a maximum of 4.0 to handle possible
  // roundoff errors.  The argument must be non-negative.
  static S1ChordAngle FromLength2(double length2);

  // Convert the chord angle to an S1Angle.  Infinity() is converted to
  // S1Angle::Infinity(), and Negative() is converted to a negative S1Angle.
  // This operation is relatively expensive.
  S1Angle ToAngle() const;

  // All operators and functions are declared here so that we can put them all
  // in one place.  (The compound assignment operators must be put here.)

  // Comparison operators.
  friend bool operator==(S1ChordAngle x, S1ChordAngle y);
  friend bool operator!=(S1ChordAngle x, S1ChordAngle y);
  friend bool operator<(S1ChordAngle x, S1ChordAngle y);
  friend bool operator>(S1ChordAngle x, S1ChordAngle y);
  friend bool operator<=(S1ChordAngle x, S1ChordAngle y);
  friend bool operator>=(S1ChordAngle x, S1ChordAngle y);

  // Comparison predicates.
  bool is_zero() const;
  bool is_negative() const;
  bool is_infinity() const;
  bool is_special() const;  // Negative or infinity.

  // Only addition and subtraction of S1ChordAngles is supported.  These
  // methods add or subtract the corresponding S1Angles.
  friend S1ChordAngle operator+(S1ChordAngle a, S1ChordAngle b);
  friend S1ChordAngle operator-(S1ChordAngle a, S1ChordAngle b);
  S1ChordAngle& operator+=(S1ChordAngle a);
  S1ChordAngle& operator-=(S1ChordAngle a);

  // Trigonmetric functions.  It is more accurate and efficient to call these
  // rather than first converting to an S1Angle.
  friend double sin(S1ChordAngle a);
  friend double cos(S1ChordAngle a);
  friend double tan(S1ChordAngle a);

  // The squared length of the chord.  (Most clients will not need this.)
  double length2() const { return length2_; }

  // Return true if the internal representation is valid.  Negative() and
  // Infinity() are both considered valid.
  bool is_valid() const;

  // When S1ChordAngle is used as a key in one of the btree container types
  // (util/btree), indicate that linear rather than binary search should be
  // used.  This is much faster when the comparison function is cheap.
  typedef std::true_type goog_btree_prefer_linear_node_search;

 private:
  // S1ChordAngles are represented by the squared chord length, which can
  // range from 0 to 4.  Infinity() uses an infinite squared length.
  explicit S1ChordAngle(double length2) : length2_(length2) {
    DCHECK(is_valid());
  }
  double length2_;
};


//////////////////   Implementation details follow   ////////////////////


inline S1ChordAngle S1ChordAngle::FromLength2(double length2) {
  return S1ChordAngle(std::min(4.0, length2));
}

inline S1ChordAngle S1ChordAngle::Zero() {
  return S1ChordAngle(0);
}

inline S1ChordAngle S1ChordAngle::Straight() {
  return S1ChordAngle(4);
}

inline S1ChordAngle S1ChordAngle::Infinity() {
  return S1ChordAngle(std::numeric_limits<double>::infinity());
}

inline S1ChordAngle S1ChordAngle::Negative() {
  return S1ChordAngle(-1);
}

inline S1ChordAngle S1ChordAngle::FastUpperBoundFrom(S1Angle angle) {
  // This method uses the distance along the surface of the sphere as an upper
  // bound on the distance through the sphere's interior.
  return S1ChordAngle::FromLength2(angle.radians() * angle.radians());
}

inline bool S1ChordAngle::is_zero() const {
  return length2_ == 0;
}

inline bool S1ChordAngle::is_negative() const {
  return length2_ < 0;
}

inline bool S1ChordAngle::is_infinity() const {
  return length2_ == std::numeric_limits<double>::infinity();
}

inline bool S1ChordAngle::is_special() const {
  return is_negative() || is_infinity();
}

inline bool operator==(S1ChordAngle x, S1ChordAngle y) {
  return x.length2() == y.length2();
}

inline bool operator!=(S1ChordAngle x, S1ChordAngle y) {
  return x.length2() != y.length2();
}

inline bool operator<(S1ChordAngle x, S1ChordAngle y) {
  return x.length2() < y.length2();
}

inline bool operator>(S1ChordAngle x, S1ChordAngle y) {
  return x.length2() > y.length2();
}

inline bool operator<=(S1ChordAngle x, S1ChordAngle y) {
  return x.length2() <= y.length2();
}

inline bool operator>=(S1ChordAngle x, S1ChordAngle y) {
  return x.length2() >= y.length2();
}

inline S1ChordAngle& S1ChordAngle::operator+=(S1ChordAngle a) {
  return (*this = *this + a);
}

inline S1ChordAngle& S1ChordAngle::operator-=(S1ChordAngle a) {
  return (*this = *this - a);
}

// Outputs the chord angle as the equivalent S1Angle.
std::ostream& operator<<(std::ostream& os, S1ChordAngle a);

#endif  // S2_GEOMETRY_S1CHORDANGLE_H_