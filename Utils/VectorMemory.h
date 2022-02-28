#ifndef OPT_VECTORMEMORY_H
#define OPT_VECTORMEMORY_H

namespace opt {

template<typename V>
inline void freeVectMem(V& v)
{
	V().swap(v);
}

} // opt

#endif // OPT_VECTORMEMORY_H