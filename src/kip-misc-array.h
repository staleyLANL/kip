
#pragma once

// array: default
template<unsigned dimension, class element>
class array { };



// -----------------------------------------------------------------------------
// array<0,element>
// -----------------------------------------------------------------------------

template<class element>
class array<0,element> {
   element *ptr;
   ulong num;

   array(const array &) = delete;
   array &operator=(const array &) = delete;

public:

   // array()
   explicit array() : ptr(nullptr), num(0) { }

   // array(n)
   explicit array(const ulong n) : ptr(new element[n]), num(n) { }

   // destructor
  ~array() { delete[] ptr; }

   // data
   element *data() { return ptr; }
   const element *data() const { return ptr; }

   // size
   ulong size() const { return num; }

   // resize
   // Reallocates to size newsize, if different from present size.
   // Data are not copied.
   void resize(const ulong newsize)
   {
      if (newsize != size()) {
         delete[] ptr;
         ptr = new element[num = newsize];
      }
   }

   // upsize
   // Reallocates to size newsize, if larger than present size.
   // Data are not copied.
   void upsize(const ulong newsize)
   {
      if (size() < newsize)
         resize(newsize);
   }

   // operator[]
   const element &operator[](const ulong n) const
   {
      kip_assert_index(n < size());
      return ptr[n];
   }

   element &operator[](const ulong n)
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
   using base = array<0,element>;
   array(const array &) = delete;
   array &operator=(const array &) = delete;
   ulong inum;

public:

   // array()
   explicit array() : inum(0) { }

   // array(i)
   explicit array(const ulong i) : base(i), inum(i) { }

   // isize; intentionally const &
   const ulong &isize() const { return inum; }

   // resize, upsize
   void resize(const ulong i) { base::resize((inum = i)); }
   void upsize(const ulong i) { base::upsize((inum = i)); }

   // operator()
   const element &operator()(const ulong i) const
   {
      kip_assert_index(i < inum);
      return (*this)[i];
   }

   element &operator()(const ulong i)
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
   using base = array<0,element>;
   array(const array &) = delete;
   array &operator=(const array &) = delete;
   ulong inum, jnum;

public:

   // array()
   explicit array() : inum(0), jnum(0) { }

   // array(i,j)
   explicit array(const ulong i, const ulong j) :
      base(i*j), inum(i), jnum(j) { }

   // isize, jsize; intentionally const &
   const ulong &isize() const { return inum; }
   const ulong &jsize() const { return jnum; }

   // resize, upsize
   void resize(const ulong i, const ulong j)
      { base::resize((inum = i)*(jnum = j)); }
   void upsize(const ulong i, const ulong j)
      { base::upsize((inum = i)*(jnum = j)); }

   // operator()
   const element &operator()(const ulong i, const ulong j) const
   {
      kip_assert_index(i < inum && j < jnum);
      return (*this)[i + inum*j];
   }

   element &operator()(const ulong i, const ulong j)
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
   using base = array<0,element>;
   array(const array &) = delete;
   array &operator=(const array &) = delete;
   ulong inum, jnum, knum;

public:

   // array()
   explicit array() : inum(0), jnum(0), knum(0) { }

   // array(i,j,k)
   explicit array(const ulong i, const ulong j, const ulong k) :
      base(i*j*k), inum(i), jnum(j), knum(k) { }

   // isize, jsize, ksize; intentionally const &
   const ulong &isize() const { return inum; }
   const ulong &jsize() const { return jnum; }
   const ulong &ksize() const { return knum; }

   // resize, upsize
   void resize(const ulong i, const ulong j, const ulong k)
      { base::resize((inum = i)*(jnum = j)*(knum = k)); }
   void upsize(const ulong i, const ulong j, const ulong k)
      { base::upsize((inum = i)*(jnum = j)*(knum = k)); }

   // operator()
   const element &operator()(const ulong i, const ulong j, const ulong k) const
   {
      kip_assert_index(i < inum && j < jnum && k < knum);
      return (*this)[i + inum*(j + jnum*k)];
   }

   element &operator()(const ulong i, const ulong j, const ulong k)
   {
      kip_assert_index(i < inum && j < jnum && k < knum);
      return (*this)[i + inum*(j + jnum*k)];
   }
};
