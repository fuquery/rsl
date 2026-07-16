#pragma once

#define OPT_      +
#define OPT_true  +
#define OPT_1     +
#define OPT_on    +
#define OPT_ON    +
#define OPT_false -
#define OPT_0     -
#define OPT_off   -
#define OPT_OFF   -

#define OPT_DEFAULT_ON  +
#define OPT_DEFAULT_OFF -

#define OPT_IMPL_EXPAND(x) OPT_##x
#define USES_OPT(x)        ((OPT_IMPL_EXPAND(x) 1) == 1)