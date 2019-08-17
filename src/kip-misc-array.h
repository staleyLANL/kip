
#pragma once

// This file defines the array class template and related functions.

// default
template<unsigned rank, class element>
class array { };



// -----------------------------------------------------------------------------
// is_integer
// -----------------------------------------------------------------------------

namespace internal {
   // is_integer_base: default
   template<bool, class>
   class is_integer_base { };

   // is_integer_base<true,type>
   template<class type>
   class is_integer_base<true,type> { public: using result = type; };

   // is_integer
   template<class integer, class type>
   class is_integer : public is_integer_base<
      std::numeric_limits<integer>::is_integer,
      type
   > { };
}



// -----------------------------------------------------------------------------
// array<0,element>
// -----------------------------------------------------------------------------

template<class element>
class array<0,element> {
   element *ptr;
   size_t num;

   // copy constructor/assignment; deliberately private
   inline array(const array &) { }
   inline array &operator=(const array &) { return *this; }

public:

   // array()
   inline explicit array() : ptr(nullptr), num(0) { }

   // array(n)
   inline explicit array(const size_t n) : ptr(new element[n]), num(n) { }

   // destructor
   inline ~array()
   {
      delete[] ptr;
   }

   // data
   inline       element *data()       { return ptr; }
   inline const element *data() const { return ptr; }

   // size
   inline size_t size() const { return num; }

   // resize
   // Reallocates to size newsize, if different from present size.
   // Data are not copied.
   template<class integer>
   inline typename internal::is_integer<integer,array>::result &
   resize(const integer newsize)
   {
      if (newsize != size()) {
         delete[] ptr;
         ptr = new element[num = size_t(newsize)];
      }
      return *this;
   }

   // upsize
   // Reallocates to size newsize, if larger than present size.
   // Data are not copied.
   template<class integer>
   inline typename internal::is_integer<integer,array>::result &
   upsize(const integer newsize)
   {
      if (newsize > size())
         resize(newsize);
      return *this;
   }

   // operator[]
   template<class integer>
   inline const typename internal::is_integer<integer,element>::result
   &operator[](const integer n) const
   {
      kip_assert_index(n < size());
      return ptr[n];
   }

   template<class integer>
   inline typename internal::is_integer<integer,element>::result
   &operator[](const integer n)
   {
      kip_assert_index(n < size());
      return ptr[n];
   }
};



// -----------------------------------------------------------------------------
// array<1,element>
// -----------------------------------------------------------------------------

template<class element>
class array<1,element> : public array<0,element> {
   inline array(const array &) { }
   inline array &operator=(const array &) { return *this; }
   size_t inum;

public:

   // array()
   inline explicit array() : array<0,element>(), inum(0) { }

   // array(i)
   inline explicit array(const size_t i) : array<0,element>(i), inum(i) { }

   // isize
   inline const size_t &isize() const { return inum; }

   // resize
   template<class integer>
   inline typename internal::is_integer<integer,array>::result
   &resize(const integer i)
      { return array<0,element>::resize((inum = size_t(i))), *this; }

   // upsize
   template<class integer>
   inline typename internal::is_integer<integer,array>::result
   &upsize(const integer i)
      { return array<0,element>::upsize((inum = size_t(i))), *this; }

   // operator()
   inline const element &operator()(const size_t i) const
   {
      kip_assert_index(i < inum);
      return (*this)[i];
   }

   inline element &operator()(const size_t i)
   {
      kip_assert_index(i < inum);
      return (*this)[i];
   }
};



// -----------------------------------------------------------------------------
// array<2,element>
// -----------------------------------------------------------------------------

template<class element>
class array<2,element> : public array<0,element> {
   inline array(const array &) { }
   inline array &operator=(const array &) { return *this; }
   size_t inum, jnum;

public:

   // array()
   inline explicit array() : array<0,element>(), inum(0), jnum(0) { }

   // array(i,j)
   inline explicit array(const size_t i, const size_t j) :
      array<0,element>(i*j), inum(i), jnum(j) { }

   // isize, jsize
   inline const size_t &isize() const { return inum; }
   inline const size_t &jsize() const { return jnum; }

   // resize
   inline array &resize(const size_t i, const size_t j)
      { return array<0,element>::resize((inum = i)*(jnum = j)), *this; }

   // upsize
   inline array &upsize(const size_t i, const size_t j)
      { return array<0,element>::upsize((inum = i)*(jnum = j)), *this; }

   // operator()
   inline const element &operator()(const size_t i, const size_t j) const
   {
      kip_assert_index(i < inum && j < jnum);
      return (*this)[i + inum*j];
   }

   inline element &operator()(const size_t i, const size_t j)
   {
      kip_assert_index(i < inum && j < jnum);
      return (*this)[i + inum*j];
   }
};



// -----------------------------------------------------------------------------
// array<3,element>
// -----------------------------------------------------------------------------

template<class element>
class array<3,element> : public array<0,element> {
   inline array(const array &) { }
   inline array &operator=(const array &) { return *this; }
   size_t inum, jnum, knum;

public:

   // array()
   inline explicit array() :
      array<0,element>(), inum(0), jnum(0), knum(0)
   { }

   // array(i,j,k)
   inline explicit array(const size_t i, const size_t j, const size_t k) :
      array<0,element>(i*j*k), inum(i), jnum(j), knum(k) { }

   // [ijk]size
   inline const size_t &isize() const { return inum; }
   inline const size_t &jsize() const { return jnum; }
   inline const size_t &ksize() const { return knum; }

   // resize
   inline array &resize(const size_t i, const size_t j, const size_t k)
   {
      return array<0,element>::resize((inum = i)*(jnum = j)*(knum = k)), *this;
   }

   // upsize
   inline array &upsize(const size_t i, const size_t j, const size_t k)
   {
      return array<0,element>::upsize((inum = i)*(jnum = j)*(knum = k)), *this;
   }

   // operator()
   inline const element &operator()(
      const size_t i, const size_t j, const size_t k
   ) const {
      kip_assert_index(i < inum && j < jnum && k < knum);
      return (*this)[i + inum*(j + jnum*k)];
   }

   inline element &operator()(
      const size_t i, const size_t j, const size_t k
   ) {
      kip_assert_index(i < inum && j < jnum && k < knum);
      return (*this)[i + inum*(j + jnum*k)];
   }
};
