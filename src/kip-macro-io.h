
// kip::istream >> shape
template<class real, class tag>
inline kip::istream &operator>>(kip::istream &k, kip::kip_class<real,tag> &obj)
{
   read_value(k,obj);
   return k;
}

// std::istream >> shape
template<class real, class tag>
inline std::istream &operator>>(std::istream &s, kip::kip_class<real,tag> &obj)
{
   kip::istream k(s);
   k >> obj;
   return s;
}

// std::ostream << shape
template<class real, class tag>
inline std::ostream &operator<<(
   std::ostream &s,
   const kip::kip_class<real,tag> &obj
) {
   kip::ostream k(s);
   k << obj;
   return s;
}

#undef kip_brief
#undef kip_class
