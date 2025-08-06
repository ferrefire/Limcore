#pragma once

class Bitmask
{
	public:
		template <typename M, typename F>
		static constexpr bool HasFlag(M mask, F flag) { return ((mask & flag) == flag); }
		template <typename M, typename F>
		static constexpr M SetFlag(M mask, F flag) { return (mask | flag); }
};
