#ifndef OPT_BITOP_H
#define OPT_BITOP_H

namespace opt {

template<typename B, typename N>
void setBit(B& bits, N n)
{
	bits |= 1U << n;
}

template<typename B, typename N>
void toggleBit(B& bits, N n)
{
	bits ^= 1U << n;
}

template<typename B, typename N>
bool checkBit(const B& bits, N n)
{
	return (bits >> n) & 1U;
}

} // opt

#endif // OPT_BITOP_H