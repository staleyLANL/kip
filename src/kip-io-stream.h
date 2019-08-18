
#pragma once

// This file provides the kip::istream and kip::ostream classes.



// -----------------------------------------------------------------------------
// kip::istream
// -----------------------------------------------------------------------------

class istream {
   static const unsigned default_maxerr = 100;

   using streampos = std::streampos;
   using iostate = std::ios_base::iostate;

   // ------------------------
   // Data
   // ------------------------

   std::ifstream _stream;  // initialized iff this istream is file-based
   std::istream  &stream;  //_stream or std::istream (constructor-dependent)

   // iff file-based...
   std::string file;   // file name
   unsigned    line;   // line number (1-indexed)

   streampos begin;    // position: beginning of line
   streampos themark;  // marked position for '^'

public:
   int level;
   unsigned maxerr;
   mutable unsigned nerr;
   bool bail;

private:
   bool nice;  // true iff file-based



   // ------------------------
   // Constructors
   // ------------------------

public:

   // istream(char *)
   inline explicit istream(const char *const name)
 :_stream(name), stream(_stream),
   file(name), line(1),
   begin(tell()), themark(begin),
   level(0), maxerr(default_maxerr), nerr(0), bail(false), nice(true)
   { }

   // istream(std::string)
   inline explicit istream(const std::string &name)
 :_stream(name.c_str()), stream(_stream),
   file(name), line(1),
   begin(tell()), themark(begin),
   level(0), maxerr(default_maxerr), nerr(0), bail(false), nice(true)
   { }

   // istream(std::istream)
   inline explicit istream(std::istream &s)
 : stream(s),
   begin(tell()), themark(begin),
   level(0), maxerr(default_maxerr), nerr(0), bail(false), nice(false)
   { }



   // ------------------------
   // Miscellaneous
   // ------------------------

private:
   inline void newline() { line++;  begin = tell(); }
   inline void mark() { themark = tell(); }

public:
   // stream-state stuff
   inline bool bad () const { return stream.bad (); }
   inline bool fail() const { return stream.fail(); }
   inline bool eof () const { return stream.eof (); }
   inline bool good() const { return stream.good(); }

   // clear: clear all flags (equivalently, set state to good)
   inline void clear() const
   {
      stream.clear();
   }

   // set: make state equal to the given value
   inline void set(const iostate newstate = std::ios::goodbit) const
   {
      stream.clear(newstate);
   }

   // add: logical-or the given value to the existing state
   inline void add(const iostate newstate) const
   {
      stream.setstate(newstate);
   }

   // state: return the state
   inline iostate state() const
   {
      return stream.rdstate();
   }

   // seek: go to the given position
   inline const istream &seek(const streampos pos) const
   {
      stream.seekg(pos);
      return *this;
   }

   // tell: return the present position
   inline streampos tell() const
   {
      return stream.tellg();
   }



   // input for char
   inline istream &input(char &value)
   {
      stream >> value;
      if (value == '(') level++; else
      if (value == ')') level--;
      return *this;
   }

   // input for T
   template<class T>
   inline istream &input(T &value)
   {
      stream >> value;
      return *this;
   }



   // get, unget, peek, bool, !
   inline int get() const { return stream.get(); }
   inline const istream &unget() const { stream.unget();  return *this; }
   inline int peek() { return stream.peek(); }
   inline operator bool() const { return bool(stream); }
   inline bool operator !() const { return !stream; }



   // ------------------------
   // error, warning, note
   // ------------------------

private:
   template<class unused>
   std::string context(const unused) const;

public:
   template<class MESSAGE>
   inline void error(const MESSAGE &message) const
   {
      if (!maxerr || nerr < maxerr)
         kip::error(context('\0'), message);
      nerr++;
   }

   template<class MESSAGE>
   inline void warning(const MESSAGE &message) const
    { kip::warning(context('\0'), message); }

   template<class MESSAGE>
   inline void note(const MESSAGE &message) const
    { kip::note(context('\0'), message); }



   // ------------------------
   // comments, prefix etc.
   // ------------------------

private:
   template<class unused>
   bool cline  (const unused, const std::string &desc, const bool eof_okay);

   template<class unused>
   bool cblock (const unused, const std::string &desc, const bool eof_okay);

public:
   template<class unused>
   bool prefix (const unused, const std::string &desc, const bool eof_okay);

   template<class unused>
   bool verify (const unused, const std::string &desc, const bool expected);

   template<class unused>
   bool recover(const unused);
};



// -----------------------------------------------------------------------------
// kip::ostream
// -----------------------------------------------------------------------------

class ostream {
   std::ofstream _stream;  // initialized iff this ostream is file-based
public:
   std::ostream  &stream;  //_stream or std::ostream (constructor-dependent)
   int level;

   // Possible user option to put in: user-defined indentation amount

   // ------------------------
   // Constructors
   // ------------------------

   // ostream(char *)
   inline explicit ostream(const char *const name)
 :_stream(name), stream(_stream),
   level(0)
   { }

   // ostream(std::string)
   inline explicit ostream(const std::string &name)
 :_stream(name.c_str()), stream(_stream),
   level(0)
   { }

   // ostream(std::ostream)
   inline explicit ostream(std::ostream &s)
 : stream(s),
   level(0)
   { }


   // ------------------------
   // Miscellaneous
   // ------------------------

   // indent
   inline ostream &indent()
   {
      for (int i = 0;  i < level;  ++i)
         stream << "   ";
      return *this;
   }

   inline operator bool() const { return bool(stream); }
   inline bool operator !() const { return !stream; }

   // endl, flush
   inline void endl () const { stream << std::endl ; }
   inline void flush() const { stream << std::flush; }
};



// -----------------------------------------------------------------------------
// context
// -----------------------------------------------------------------------------

template<class unused>
std::string istream::context(const unused) const
{
   // file and line, standard input, or stream
   std::ostringstream oss;
   oss << "while reading from ";

   if (nice)
      // great - we know file and line number
      oss << '"' << file << "\" line " << line;
   else if (&stream == &std::cin)
      // okay - we know it's standard input
      oss << "standard input";
   else
      // well, it's an input stream
      oss << "input stream";

   if (!bad()) {
      // save state, then clear it
      const iostate oldstate = state();
      clear();

      // save location; then seek to beginning of relevant line
      const streampos location = tell();
      seek(begin);

      // if seek was successful
      if (tell() == begin && long(begin) != -1) {
         // put relevant line and carat into oss
         std::string carat;  int ch;
         oss << '\n';

         while ((ch = get(), good()))
            if (!isspace(ch)) {
               unget();
               break;
            }

         while ((ch = get(), good()) && ch != '\n') {
            oss << char(ch);
            if (long(themark) == -1 || tell() <= themark)
               carat += ch == '\t' ? '\t' : ' ';  // give it a shot :-/
            // don't bother with nonsense like vertical tabs and form feeds
         }
         oss << '\n' << carat << '^';
      } else
         // oh well
         oss << "\n[stream seek unsuccessful; unable to display line]";

      // restore previously-saved location and state
      clear();
      seek(location);
      set(oldstate);
   }

   return oss.str();
}



// -----------------------------------------------------------------------------
// cline
// cblock
// -----------------------------------------------------------------------------

template<class unused>
bool istream::cline(
   const unused, const std::string &description, const bool eof_okay
) {
   themark = long(tell())-1;
   int ch = get();  // get the second / (peeked at previously)

   //   //...
   //     ^
   //   Next character will be read at the location shown by the carat

   while (ch = get(), good())
      if (ch == '\n') {
         newline();
         return true;
      }

   std::ostringstream oss;

   // bad
   if (bad())
      oss << "Bad stream state (unrecoverable) while scanning comment";

   // eof
   else if (eof()) {
      if (eof_okay) return false;
      oss << "End-of-file at termination of comment";

   // fail (shouldn't happen alone)
   } else {
      oss << "get() failure (unrecoverable) while scanning comment";
      bail = true;
   }

   if (description[0])
      oss << "\nExpected " << description << " after comment";
   error(oss);
   return false;
}



// cblock
template<class unused>
bool istream::cblock(
   const unused, const std::string &description, const bool eof_okay
) {
   // for the comment-starting /
   std::vector<unsigned > lines (1, line);
   std::vector<streampos> begins(1, begin);
   std::vector<streampos> marks (1, long(tell())-1);

   int ch = get();  // get the asterisk (peeked at previously)

   //   /*...
   //     ^
   //   Next character will be read at the location shown by the carat

   // Note: nested comments are allowed, and are handled directly. We could
   // have used recursion, but then if multiple in-comment errors occurred,
   // they'd print backward with respect to their line numbers. Note that
   // the only in-comment error that can occur is for end-of-file to occur
   // before termination of the comment.

   for (;;) {
      const char last = char(ch);  // is initially the *
      ch = get();  if (!good()) break;

      if (ch == '\n')
         newline();

      else if (last == '/' && ch == '*') {
         // nested comment
         lines .push_back(line);
         begins.push_back(begin);
         marks .push_back(long(tell())-2);

      } else if (last == '*' && ch == '/') {
         // end of comment
         lines .pop_back();
         begins.pop_back();
         marks .pop_back();
         if (lines.size() == 0) return true;
      }
   }

   std::ostringstream oss;

   // bad
   if (bad()) {
      oss << "Bad stream state (unrecoverable) while scanning comment";
      if (description[0])
         oss << "\nExpected " << description << " after comment";

   // eof
   } else if (eof()) {
      oss << "End-of-file before termination of comment";
      if (!eof_okay)  // then we'll have a description
         oss << "\nExpected " << description << " after comment";

   // fail (shouldn't happen alone)
   } else {
      oss << "get() failure (unrecoverable) while scanning comment";
      if (description[0])
         oss << "\nExpected " << description << " after comment";
      bail = true;
   }

   const unsigned  save_line  = line;
   const streampos save_begin = begin;
   const streampos save_mark  = themark;
   const size_t size = lines.size();

   for (size_t i = 0;  i < size;  ++i) {
      line    = lines [i];
      begin   = begins[i];
      themark = marks [i];
      error(oss);
   }

   line    = save_line;
   begin   = save_begin;
   themark = save_mark;
   return false;
}



// -----------------------------------------------------------------------------
// prefix
// verify
// recover
// -----------------------------------------------------------------------------

// prefix
// Returns true iff we found something other than whitespace or comments,
// and the stream is still good (not bad, fail, or eof). Bad and fail cause
// error messages; eof causes an error message iff !eof_okay.
template<class unused>
bool istream::prefix(
   const unused, const std::string &description, const bool eof_okay
) {
   // skip whitespace and comments
   int ch;
   while (ch = get(), good())
      if (ch == '\n')
         // next line
         newline();
      else if (ch == '/') {
         // possible comment...
         const int next = peek();
         if (next == '/') {
            if (!cline
                ('\0', description,eof_okay)) return false;  // line comment
         } else if (next == '*') {
            if (!cblock
                ('\0', description,eof_okay)) return false;  // block comment
         } else
            { unget();  break; }  // done; '/' is the next non-prefix character
      } else if (!isspace(ch))
         { unget();  break; }  // done; ch is the next non-prefix character

   mark();
   if (good()) return true;
   std::ostringstream oss;

   // bad
   if (bad())
      oss << "Bad stream state (unrecoverable)";

   // eof
   else if (eof()) {
      if (eof_okay) return false;
      oss << "End-of-file";

   // fail (shouldn't happen alone)
   } else {
      oss << "get() failure (unrecoverable)";
      bail = true;
   }

   if (description[0]) oss << "\nExpected " << description;
   error(oss);
   return false;
}



// verify
template<class unused>
bool istream::verify(
   const unused, const std::string &description, const bool expected
) {
   // bad, fail, or !expected
   if (fail() || !expected) {
      std::ostringstream oss;
      if (bad()) oss << "Bad stream state (unrecoverable)\n";
      oss << "Expected " << description;
      error(oss);
      return false;
   }
   return true;
}



// recover
template<class unused>
bool istream::recover(const unused)
{
   if (bad() || eof() || bail)
      return false;  // either way, we can't do anything about it

   // give it a shot
   clear();
   int ch;
   while (prefix('\0',"",true), ch = get(), !fail())  // prefix: true: eof okay
      if (ch == '\n')
         newline();
      else if (ch == '(')
         level++;
      else if (level <= 0 || (ch == ')' && --level <= 0)) {
         if (ch != ')')
            unget();
         if (prefix('\0',"",true))  // advance to recovery position; eof okay
            note("Parsing continues here after previous diagnostic");
         return !bail;
      }
   return false;
}



// -----------------------------------------------------------------------------
// operator<< (kip::ostream, general)
// -----------------------------------------------------------------------------

template<class T>
inline kip::ostream &operator<<(kip::ostream &k, const T &value)
{
   // Note: k.stream is a std::ostream, not a kip::ostream
   k.stream << value;
   return k;
}
