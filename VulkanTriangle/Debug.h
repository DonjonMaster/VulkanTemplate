#pragma once

class Debug
{
public:
// Are validation layers active ?
#ifdef _DEBUG
    static constexpr bool enable_debug = true;
#else
    static constexpr bool enable_debug = false;
#endif
};
